/*
 * CMA driver
 *
 * (C) 2018.11.29 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/debugfs.h>
#include <linux/mempolicy.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/dma-mapping.h>
#include <linux/export.h>
#include <linux/gfp.h>
#include <linux/acpi.h>
#include <linux/bootmem.h>
#include <linux/cache.h>
#include <linux/export.h>
#include <linux/slab.h>
#include <linux/genalloc.h>
#include <linux/dma-mapping.h>
#include <linux/dma-contiguous.h>
#include <linux/cma.h>
#include <asm/io.h>

#define CMA_MEM_VERSION         3
#define DEVICE_NAME             "CMA" 
#define CMEM_IOCTL_MAGIC        'm'
#define CMEM_ALLOCATE           _IOW(CMEM_IOCTL_MAGIC, 1, unsigned int)
#define CMEM_RELEASE            _IOW(CMEM_IOCTL_MAGIC, 2, unsigned int)

struct cmamem_info {
    unsigned int version;
    unsigned int len;
    unsigned int offset;
    unsigned long mem_base;
    unsigned long phy_base;
};

struct cmamem_dev {
    struct miscdevice dev;
    struct mutex cmamem_lock;
    struct list_head info_list;
};

struct current_status {
    int status;
    dma_addr_t phy_base;
};

enum cma_status {
    UNKNOW_STATUS = 0, 
    HAVE_ALLOCED = 1,
};

static struct current_status cmamem_status;
static struct cmamem_dev cmamem_dev;
static struct cmamem_info cma_info;

static long cmamem_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
{
    unsigned long nr_pages;
    struct page *page;
    unsigned int pool_size_order;

    switch (cmd) {
    case CMEM_ALLOCATE:
        mutex_lock(&cmamem_dev.cmamem_lock);
        if(cmamem_status.status != HAVE_ALLOCED) {		
            if (copy_from_user(&cma_info, (void __user *)arg, 
                                         sizeof(struct cmamem_info))) {
                printk(KERN_ERR "CMEM_ALLOCATE: copy_from_user error\n");
                goto CMA_FAIL;
            }
            if(cma_info.version != CMA_MEM_VERSION) {
                printk(KERN_ERR "CMEM_ALLOCATE: kernel module version "
                              "check fail, version % d\n", cma_info.version);
                goto CMA_FAIL;
            }

            nr_pages = cma_info.len >> PAGE_SHIFT;
            pool_size_order = get_order(cma_info.len);
            page = dma_alloc_from_contiguous(NULL, nr_pages, 
                                       pool_size_order, GFP_KERNEL);		

            if(!page) {
                printk(KERN_ERR "CMEM_ALLOCATE: dma_alloc_from_contiguous "
                                "fail, len 0x%x\n", cma_info.len);
                goto CMA_FAIL;
            }

            cma_info.mem_base = (dma_addr_t)page_to_virt(page);
            cma_info.phy_base = (dma_addr_t)page_to_phys(page);
            cmamem_status.phy_base = cma_info.phy_base;
            cmamem_status.status = HAVE_ALLOCED;
        }		
        if (copy_to_user((void __user *)arg, &cma_info, 
                                     sizeof(struct cmamem_info))) {
            printk(KERN_ERR "CMEM_ALLOCATE: copy_to_user error\n");
            goto CMA_FAIL;
        }
        mutex_unlock(&cmamem_dev.cmamem_lock);
        return 0;
    case CMEM_RELEASE:
        mutex_lock(&cmamem_dev.cmamem_lock);
        if(cmamem_status.status != HAVE_ALLOCED) {
            printk(KERN_ERR "CMEM_RELEASE: %s, not allocted memory\n", 
                                    __func__);
            goto CMA_FAIL;
        }
        if (copy_from_user(&cma_info, (void __user *)arg, 
                                    sizeof(struct cmamem_info))) {
            printk(KERN_ERR "CMEM_RELEASE: copy_from_user error\n");
            goto CMA_FAIL;
        }		
        if(cma_info.version != CMA_MEM_VERSION) {
            printk(KERN_ERR "CMEM_RELEASE: kernel module version check fail, "
                       "version % d\n", cma_info.version);
            goto CMA_FAIL;
        }		
        if(cma_info.phy_base != cmamem_status.phy_base) {
            printk(KERN_ERR "CMEM_RELEASE: unknown CMA 0x%lx\n", 
                     cma_info.phy_base);
            goto CMA_FAIL;
        }
                
        page = phys_to_page(cmamem_status.phy_base);
        nr_pages = cma_info.len	>> PAGE_SHIFT;
        dma_release_from_contiguous(NULL, page, nr_pages);
        memset(&cma_info, 0, sizeof(cma_info));
        memset(&cmamem_status, 0, sizeof(cmamem_status));
        mutex_unlock(&cmamem_dev.cmamem_lock);
        return 0;
    default:
        printk(KERN_INFO "cma mem not support command\n");
        return -EFAULT;
    }
CMA_FAIL:
    mutex_unlock(&cmamem_dev.cmamem_lock);
    return -EFAULT;
}

static int cmamem_mmap(struct file *filp, struct vm_area_struct *vma) {
    unsigned long start = vma->vm_start;
    unsigned long size = vma->vm_end - vma->vm_start;
    unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
    unsigned long page, pos;

    if (cmamem_status.status != HAVE_ALLOCED) {
        printk(KERN_ERR "cmamem_mmap: %s, you should allocted memory "
                             "firstly\n", __func__);
        return -EINVAL;
    }

    pos = (unsigned long) cmamem_status.phy_base + offset;
    page = pos >> PAGE_SHIFT;
    if (remap_pfn_range(vma, start, page, size, PAGE_SHARED))
        return -EAGAIN;

    vma->vm_flags &= ~VM_IO;
    vma->vm_flags |= (VM_DONTEXPAND | VM_DONTDUMP);

    return 0;
}

static struct file_operations dev_fops = { 
    .owner = THIS_MODULE,
    .unlocked_ioctl = cmamem_ioctl, 
    .mmap = cmamem_mmap,
};

static int __init cmamem_init(void)
{
    mutex_init(&cmamem_dev.cmamem_lock);
    INIT_LIST_HEAD(&cmamem_dev.info_list);
    cmamem_dev.dev.name = DEVICE_NAME;
    cmamem_dev.dev.minor = MISC_DYNAMIC_MINOR;
    cmamem_dev.dev.fops = &dev_fops;

    cmamem_status.status = UNKNOW_STATUS;
    cmamem_status.phy_base = 0;

    return misc_register(&cmamem_dev.dev);
}

static void __exit cmamem_exit(void)
{
    unsigned long nr_pages;
    struct page *page;
	
    printk(KERN_ERR "%s\n", __func__);
    mutex_lock(&cmamem_dev.cmamem_lock);
    if(cmamem_status.status == HAVE_ALLOCED) {
        page = phys_to_page(cma_info.mem_base);		
        nr_pages = cma_info.len >> PAGE_SHIFT;
        dma_release_from_contiguous(NULL, page, nr_pages);
        memset(&cmamem_status, 0, sizeof(cmamem_status));
    }
    mutex_unlock(&cmamem_dev.cmamem_lock);
    misc_deregister(&cmamem_dev.dev);
}

module_init (cmamem_init);
module_exit (cmamem_exit);
MODULE_LICENSE("GPL");
