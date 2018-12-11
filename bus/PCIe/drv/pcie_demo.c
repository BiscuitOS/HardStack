/*
 * Copyright (C) 2018 <buddy.zhang@aliyun.com>
 *
 * PCIe driver demo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

#include <linux/pci.h>

#define DEV_NAME                 "pcie_demo"
#define PCIE_DEMO_VENDOR_ID      0x10ee
#define PCIE_DEMO_DEVICE_ID      0x9034
#define BAR_NUM                  6

struct bar_node {
    unsigned long phys;
    unsigned long virt;
    unsigned long length;
    unsigned long used;
};

/* Barn(n=0,1 or 0,1,2,3,4,5) physical address, length, virtual address */
struct bar_node bars[BAR_NUM] = {
    /* Bar0 */
    {
        .phys = 0,
        .virt = 0,
        .length = 0,
        .used = 1,
    },
    /* Bar1 */
    {
        .phys = 0,
        .virt = 0,
        .length = 0,
        .used = 0,
    },
    /* Bar2 */
    {
        .phys = 0,
        .virt = 0,
        .length = 0,
        .used = 1,
    },
    /* Bar3 */
    {
        .phys = 0,
        .virt = 0,
        .length = 0,
        .used = 0,
    },
    /* Bar4 */
    {
        .phys = 0,
        .virt = 0,
        .length = 0,
        .used = 0,
    },
    /* Bar5 */
    {
        .phys = 0,
        .virt = 0,
        .length = 0,
        .used = 0,
    },
};

/*
 * open operation
 */
static int pcie_demo_open(struct inode *inode,struct file *filp)
{
    printk(KERN_INFO "Open device\n");
    return 0;
}
/*
 * release opertion 
 */
static int pcie_demo_release(struct inode *inode,struct file *filp)
{
    return 0;
}
/*
 * read operation
 */
static ssize_t pcie_demo_read(struct file *filp,char __user *buffer,size_t count,
		loff_t *offset)
{
    return 0;
}
/*
 * write operation
 */
static ssize_t pcie_demo_write(struct file *filp,const char __user *buf,
		size_t count,loff_t *offset)
{
    return 0;
}

static int pcie_demo_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
    int result;
    int i;

    printk("PCIe probe starting....Vendor %#x DeviceID %#x\n", 
                                              id->vendor, id->device);

    /* Enable PCIe */
    if (pci_enable_device(pdev)) {
        result = -EIO;
        goto end;
    }

    pci_set_master(pdev);

    if (unlikely(pci_request_regions(pdev, DEV_NAME))) {
        printk("Failed: pci_request_regions\n");
        result = -EIO;
        goto enable_device_err;
    }

    /* Obtain bar0 to bar5 information */
    for (i = 0; i < BAR_NUM; i++) {
        if (!bars[i].used)
            continue;
        /* Obtain bar physical address */
        bars[i].phys = pci_resource_start(pdev, i);
            if (bars[i].phys < 0) {
                printk("Failed: Bar%d pci_resource_start\n", i);
                result = -EIO;
                goto request_regions_err;
        }

        /* Obtain the length for Bar */
        bars[i].length = pci_resource_len(pdev, 0);
        if (bars[i].length != 0)
            bars[i].virt = (unsigned long)ioremap(bars[i].phys,
                                         bars[i].length);
        

        printk("Bar%d=> phys: %#lx virt: %#lx length: %#lx\n",
                     i, bars[i].phys, bars[i].virt, bars[i].length);
    }

    return 0;

request_regions_err:

    pci_release_regions(pdev);

enable_device_err:
    pci_disable_device(pdev);

end:

    return result;
}

static void pcie_demo_remove(struct pci_dev *pdev)
{
    int i;

    for (i = 0; i < BAR_NUM; i++)
        if (bars[i].virt > 0)
            iounmap((void *)bars[i].virt);

    pci_release_regions(pdev);

    pci_disable_device(pdev);
}

/*
 * file_operations
 */
static struct file_operations pcie_demo_fops = {
    .owner     = THIS_MODULE,
    .open      = pcie_demo_open,
    .release   = pcie_demo_release,
    .write     = pcie_demo_write,
    .read      = pcie_demo_read,
};
/*
 * misc struct 
 */

static struct miscdevice pcie_demo_misc = {
    .minor    = MISC_DYNAMIC_MINOR,
    .name     = DEV_NAME,
    .fops     = &pcie_demo_fops,
};

static struct pci_device_id pcie_demo_ids[] = {
    { PCIE_DEMO_VENDOR_ID, PCIE_DEMO_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
    { 0, }
};
MODULE_DEVICE_TABLE(pci, pcie_demo_ids);

static struct pci_driver pci_drivers = {
    .name = DEV_NAME,
    .id_table = pcie_demo_ids,
    .probe = pcie_demo_probe,
    .remove = pcie_demo_remove,
};


/*
 * Init module
 */
static __init int pcie_demo_init(void)
{
    int ret;

    /* Register PCIe driver */
    ret = pci_register_driver(&pci_drivers);

    /* Register Misc layer interface. */
    misc_register(&pcie_demo_misc);

    return 0;
}
/*
 * Exit module
 */
static __exit void pcie_demo_exit(void)
{
    /* Unregister PCIe driver */
    pci_unregister_driver(&pci_drivers);

    /* Unregister misc driver */
    misc_deregister(&pcie_demo_misc);
}
/*
 * module information
 */
module_init(pcie_demo_init);
module_exit(pcie_demo_exit);

MODULE_LICENSE("GPL");
