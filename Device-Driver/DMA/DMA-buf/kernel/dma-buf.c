/*
 * DMA-buffer
 *
 * (C) 2019.12.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * DTS descripts:
 *
 * / {
 *        BiscuitOS_demo {
 *                compatible = "dma-buf, BiscuitOS";
 *                status = "okay";
 *        };
 * };
 *
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/dma-direct.h>

/* LDD Platform Name */
#define DEV_NAME	"BiscuitOS_demo"
/* IOCTL */
#define GET_DMA_BUF	_IOWR('q', 13, struct demo_dma_info)

/* Char device */
static int demo_major;
static int demo_minor;
static struct class *demo_class;

struct demo_desc {
	struct device *device;
	struct cdev cdev;
	int index;
};

struct demo_dma_info {
	__u32 fd;
	__u32 size;
	__u32 phy_addr;
};

struct demo_dma_buffer {
	u32 size;
	dma_addr_t dma_handle;
	void *cpu_handle;

	/* DMABUF related */
	struct device *dev;
	struct sg_table *sgt_base;
	enum dma_data_direction dma_dir;

	/* real buf */
	struct dma_buf *dbuf;
	int dma_fd;
	u32 dma_address;
};

static const struct file_operations demo_fops;
static const struct dma_buf_ops demo_dmabuf_ops;

/** DMA-buffer Operations **/

struct demo_dmabuf_attachment {
	struct sg_table sgt;
	enum dma_data_direction dma_dir;
};

static int demo_dmabuf_attach(struct dma_buf *buf, 
			      struct dma_buf_attachment *dbuf_attach)
{
	struct demo_dma_buffer *dbuf = buf->priv;
	struct demo_dmabuf_attachment *attach;
	struct scatterlist *rd, *wr;
	struct sg_table *sgt;
	int ret, i;

	attach = kzalloc(sizeof(*attach), GFP_KERNEL);
	if (!attach)
		return -ENOMEM;
	sgt = &attach->sgt;

	ret = sg_alloc_table(sgt, dbuf->sgt_base->orig_nents, GFP_KERNEL);
	if (ret) {
		kfree(sgt);
		return -ENOMEM;
	}

	rd = dbuf->sgt_base->sgl;
	wr = sgt->sgl;

	for (i = 0; i < sgt->orig_nents; i++) {
		sg_set_page(wr, sg_page(rd), rd->length, rd->offset);
		rd = sg_next(rd);
		wr = sg_next(wr);
	}

	attach->dma_dir = DMA_NONE;
	dbuf_attach->priv = attach;

	return 0;
}

static void demo_dmabuf_detach(struct dma_buf *dbuf,
			       struct dma_buf_attachment *db_attach)
{
	struct demo_dmabuf_attachment *attach = db_attach->priv;
	struct sg_table *sgt;

	if (!attach)
		return;

	sgt = &attach->sgt;

	/* release the scatterlist cache */
	if (attach->dma_dir != DMA_NONE)
		dma_unmap_sg(db_attach->dev, sgt->sgl, sgt->orig_nents,
				attach->dma_dir);

	sg_free_table(sgt);
	kfree(attach);
	db_attach->priv = NULL;
}

static struct sg_table *demo_dmabuf_map(struct dma_buf_attachment *db_attach,
				enum dma_data_direction dma_dir)
{
	struct demo_dmabuf_attachment *attach = db_attach->priv;
	struct sg_table *sgt;
	struct mutex *lock = &db_attach->dmabuf->lock;

	mutex_lock(lock);

	sgt = &attach->sgt;

	if (attach->dma_dir == dma_dir) {
		mutex_unlock(lock);
		return sgt;
	}

	if (attach->dma_dir != DMA_NONE) {
		dma_unmap_sg(db_attach->dev, sgt->sgl, sgt->orig_nents,
					attach->dma_dir);
		attach->dma_dir = DMA_NONE;
	}

	sgt->nents = dma_map_sg(db_attach->dev, sgt->sgl, sgt->orig_nents,
					dma_dir);

	if (!sgt->nents) {
		printk("Failed to map scatterlist\n");
		mutex_unlock(lock);
		return ERR_PTR(-EIO);
	}

	attach->dma_dir = dma_dir;
	mutex_unlock(lock);

	return sgt;
}

static void demo_dmabuf_unmap(struct dma_buf_attachment *at,
			struct sg_table *sg, enum dma_data_direction dir)
{
}

static void *demo_dmabuf_kmap(struct dma_buf *dmabuf, unsigned long page_num)
{
	struct demo_dma_buffer *dbuf = dmabuf->priv;
	void *vaddr = dbuf->cpu_handle;

	return vaddr + page_num * PAGE_SIZE;
}

static void *demo_dmabuf_vmap(struct dma_buf *buf)
{
	struct demo_dma_buffer *dbuf = buf->priv;
	void *vaddr = dbuf->cpu_handle;

	return vaddr;
}

static int demo_dmabuf_mmap(struct dma_buf *buf, struct vm_area_struct *vma)
{
	struct demo_dma_buffer *dbuf = buf->priv;
	unsigned long start = vma->vm_start;
	unsigned long vsize = vma->vm_end - start;
	int ret;

	if (!dbuf) {
		printk("No buffer to map\n");
		return -EINVAL;
	}

	vma->vm_pgoff = 0;

	ret = dma_mmap_coherent(dbuf->dev, vma, dbuf->cpu_handle,
				dbuf->dma_handle, vsize);
	if (ret < 0) {
		printk("Remapping memory failed, error: %d\n", ret);
		return ret;
	}

	vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;

	return 0;
}

static void demo_dmabuf_release(struct dma_buf *buf)
{
	struct demo_dma_buffer *dbuf = buf->priv;

	if (dbuf->sgt_base) {
		sg_free_table(dbuf->sgt_base);
		kfree(dbuf->sgt_base);
	}

	dma_free_coherent(dbuf->dev, dbuf->size, dbuf->cpu_handle,
				dbuf->dma_handle);
	put_device(dbuf->dev);
	kfree(dbuf);
}

/** DMA-buffer interface **/

/* DMA-buffer get address */
static int demo_dma_get_address(struct demo_dma_buffer *dbuf)
{
	struct dma_buf *buf;
	struct dma_buf_attachment *attach;
	struct sg_table *sgt;
	int err = 0;

	buf = dma_buf_get(dbuf->dma_fd);
	if (IS_ERR(dbuf))
		return -EINVAL;

	attach = dma_buf_attach(buf, dbuf->dev);
	if (IS_ERR(attach)) {
		err = -EINVAL;
		goto err_attach;
	}

	sgt = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(sgt)) {
		err = -EINVAL;
		goto err_map;
	}

	dbuf->dma_address = sg_dma_address(sgt->sgl);

	dma_buf_unmap_attachment(attach, sgt, DMA_BIDIRECTIONAL);
err_map:
	dma_buf_detach(buf, attach);
err_attach:
	dma_buf_put(buf);
	return err;
}

/* DMA-buffer ioctl get fd */
static int demo_ioctl_get_dmabuf(struct device *dev, unsigned long arg)
{
	struct dma_buf_export_info exp_info;
	struct demo_dma_info info;
	struct demo_dma_buffer *dbuf;
	struct sg_table *sgt;
	int ret;

	if (copy_from_user(&info, (struct demo_dma_info *)arg, sizeof(info))) {
		ret = -EFAULT;
		goto err_user;
	}

	/* allocate structure dma_buf */
	dbuf = kzalloc(sizeof(struct demo_dma_buffer), GFP_KERNEL);
	if (!dbuf) {
		ret = -ENOMEM;
		goto err_user;
	}
	dbuf->size = info.size;

	/* Allocate DMA Memory */
	dbuf->cpu_handle = dma_alloc_coherent(dev,
					dbuf->size,
					&dbuf->dma_handle,
					GFP_KERNEL | GFP_DMA);
	if (!dbuf->cpu_handle) {
		ret = -EFAULT;
		goto err_handler;
	}
	/* DMA memory information */
	printk("DMA-Memory Vir-addr: %#lx\n", (unsigned long)dbuf->cpu_handle);
	printk("DMA-Memory DMA-addr: %#lx\n", (unsigned long)dbuf->dma_handle);
	printk("DMA-Memory Phy-addr: %#lx\n", 
			(unsigned long)dma_to_phys(dev, dbuf->dma_handle));

	/* sg_table setup */
	sgt = kzalloc(sizeof(*sgt), GFP_KERNEL);
	if (!sgt) {
		ret = -ENOMEM;
		goto err_sgt;
	}
	dbuf->dev = get_device(dev);
	dbuf->dma_dir = DMA_BIDIRECTIONAL;
	ret = dma_get_sgtable(dev, sgt, dbuf->cpu_handle, 
					dbuf->dma_handle, dbuf->size);
	if (ret < 0) {
		ret = -EINVAL;
		goto err_sgtable;
	}
	dbuf->sgt_base = sgt;

	/* DMA-Buf exp_info */
	exp_info.owner = THIS_MODULE;
	exp_info.exp_name = KBUILD_MODNAME;
	exp_info.ops = &demo_dmabuf_ops;
	exp_info.flags = O_RDWR;
	exp_info.resv = NULL;
	exp_info.size = dbuf->size;
	exp_info.priv = dbuf;
	dbuf->dbuf = dma_buf_export(&exp_info);
	if (IS_ERR(dbuf->dbuf)) {
		ret = -EINVAL;
		goto err_dbuf;
	}
	/* Get DMA-Buffer FD */
	dbuf->dma_fd = dma_buf_fd(dbuf->dbuf, O_RDWR);

	/* DMA get address */
	ret = demo_dma_get_address(dbuf);
	if (ret)
		goto err_dbuf;

	info.phy_addr = dbuf->dma_address;
	/* Copy to user */
	if (copy_to_user((void *)arg, &info, sizeof(info)))
		goto err_dbuf;
	return 0;

err_dbuf:
	sg_free_table(dbuf->sgt_base);
err_sgtable:
	kfree(sgt);
err_sgt:
	dma_free_coherent(dev, dbuf->size, dbuf->cpu_handle, dbuf->dma_handle);	
err_handler:
	kfree(dbuf);
err_user:
	return ret;
}

/** DMA-buf file operations **/

static int demo_open(struct inode *inode, struct file *filp)
{
	struct demo_desc *desc;

	desc = container_of(inode->i_cdev, struct demo_desc, cdev);
	filp->private_data = desc;
	return 0;
}

static int demo_release(struct inode *inode, struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}

static long demo_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct demo_desc *desc = filp->private_data;
	int ret;

	switch (cmd) {
	case GET_DMA_BUF:
		ret = demo_ioctl_get_dmabuf(desc->device, arg);
		return ret;
	}

	return 0;
}

/* cdev interface create */
static int demo_cdev(struct demo_desc *desc)
{
	struct device *device;
	dev_t dev = 0;

	alloc_chrdev_region(&dev, demo_minor, 1, "BiscuitOS");
	demo_major = MAJOR(dev);
	cdev_init(&desc->cdev, &demo_fops);
	desc->cdev.owner = THIS_MODULE;
	cdev_add(&desc->cdev, dev, 1);

	/* class creete */
	demo_class = class_create(THIS_MODULE, "BiscuitOS_class");
	if (IS_ERR(demo_class)) {
		printk("BiscuitOS class not created\n");
		cdev_del(&desc->cdev);
		return PTR_ERR(demo_class);
	}

	/* device create */
	device = device_create(demo_class, NULL, dev, NULL, "BiscuitOS");
	if (IS_ERR(device)) {
		printk("BiscuitOS device not created\n");
		class_destroy(demo_class);
		cdev_del(&desc->cdev);
	}
	return 0;
}

/* Probe: (LDD) Initialize Device */
static int demo_probe(struct platform_device *pdev)
{
	struct demo_desc *desc;
	int err;

	desc = devm_kzalloc(&pdev->dev, sizeof(struct demo_desc), GFP_KERNEL);
	if (!desc)
		return -ENOMEM;

	err = demo_cdev(desc);
	desc->device = &pdev->dev;
	platform_set_drvdata(pdev, desc);

	return 0;
}

/* Remove: (LDD) Remove Device (Module) */
static int demo_remove(struct platform_device *pdev)
{
	struct demo_desc *desc;
	dev_t dev = MKDEV(demo_major, demo_minor);

	desc = platform_get_drvdata(pdev);
	device_destroy(demo_class, dev);
	cdev_del(&desc->cdev);
	kfree(desc);

	return 0;
}

static const struct dma_buf_ops demo_dmabuf_ops = {
	.attach		= demo_dmabuf_attach,
	.detach		= demo_dmabuf_detach,
	.map_dma_buf	= demo_dmabuf_map,
	.unmap_dma_buf	= demo_dmabuf_unmap,
	.map		= demo_dmabuf_kmap,
	.vmap		= demo_dmabuf_vmap,
	.mmap		= demo_dmabuf_mmap,
	.release	= demo_dmabuf_release,
};

static const struct file_operations demo_fops = {
	.owner		= THIS_MODULE,
	.open		= demo_open,
	.release	= demo_release,
	.unlocked_ioctl	= demo_ioctl,
};

static const struct of_device_id demo_of_match[] = {
	{ .compatible = "dma-buf, BiscuitOS", },
	{ },
};
MODULE_DEVICE_TABLE(of, demo_of_match);

/* Platform Driver Information */
static struct platform_driver demo_driver = {
	.probe    = demo_probe,
	.remove   = demo_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
		.of_match_table	= demo_of_match,
	},
};
module_platform_driver(demo_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS DMA-buf");
