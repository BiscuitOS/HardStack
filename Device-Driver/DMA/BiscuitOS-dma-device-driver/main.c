/*
 * BiscuitOS DMA QEMU Device Driver
 *
 * (C) 2022.03.26 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2022.04.02 BiscuitOS
 *          <https://biscuitos.github.io/blog/BiscuitOS_Catalogue/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/pci.h>
#include <linux/delay.h>

/* DMA device name */
#define DEV_NAME			"BiscuitOS-DMA"
/* IOCTL CMD */
#define BISCUITOS_IO			0xAE
#define BISCUITOS_IOCTL_DMA_FROM_PCI	_IO(BISCUITOS_IO, 0x00)
#define BISCUITOS_IOCTL_DMA_TO_PCI	_IO(BISCUITOS_IO, 0x01)
#define BISCUITOS_IOCTL_DMA_TO_USER	_IO(BISCUITOS_IO, 0x02)

/* DMA */
#define MMIO_BAR		0x00
#define DMA_BUFFER_SIZE		0x100000
#define DMA_RUN_CMD		0x01
#define DMA_DIR(direct)		(((direct) & 0x1) << 1)
#define DMA_DIR_EXT(cmd)	(((cmd) & 0x2) >> 1)
#define REG_PCI_BASE		0x60
#define REG_PCI_SIZE		0x64
#define REG_TRANS_SRC		0x68
#define REG_TRANS_DST		0x6c
#define REG_TRANS_CNT		0x70
#define REG_TRANS_CMD		0x74
#define REG_TRANS_RUN		0x78
#define BISCUITOS_DMA_TO_PCI	0x0
#define BISCUITOS_DMA_FROM_PCI	0x1
#define BISCUITOS_DMA_IRQ	0x8

struct dma_desc_userspace {
	unsigned int dma_base;
	unsigned int pci_base;
	unsigned int size;
	int dma_finish;
	char data[128];
};

struct BiscuitOS_DMA_state {
	struct pci_dev *pdev;
	/* IRQ */
	int irq;
	/* MMIO BAR */
	void __iomem *mmio;
	/* DMA Physcial address */
	dma_addr_t dma_addr;
	/* DMA mapping virtual address */
	char *buffer;
	unsigned long buffer_size;
	int dma_finish;
};
static struct BiscuitOS_DMA_state *bds;

static inline int within(unsigned int base0, unsigned int size0,
				unsigned int base1, unsigned int size1)
{
	unsigned int end0 = base0 + size0;
	unsigned int end1 = base1 + size1;

	if (base0 >= base1 && end0 <= end1 && base0 < end0)
		return 0;
	return -EINVAL;
}

static int BiscuitOS_DMA_transmit(struct dma_desc_userspace *data, int direct)
{
	int pci_base, pci_size;
	int dma_base, dma_size;

	/* Detect PCI Memory Range */
	pci_base = ioread32(bds->mmio + REG_PCI_BASE);
	pci_size = ioread32(bds->mmio + REG_PCI_SIZE);
	/* Detect DMA Memory Range */
	dma_base = data->dma_base + bds->dma_addr;
	dma_size = data->size;

	/* Check PCI Memory Request */
	if (within(data->pci_base, data->size, pci_base, pci_size)) {
		printk("ERROR: Outof PCI Memory range.\n");
		return -EINVAL;
	}

	/* Check DMA Request */
	if (within(dma_base, dma_size, bds->dma_addr, bds->buffer_size)) {
		printk("ERROR: Outof DMA Memory range.\n");
		return -EINVAL;
	}

	/* Init DMA */
	iowrite32(data->size, bds->mmio + REG_TRANS_CNT);
	if (direct == BISCUITOS_DMA_TO_PCI) { /* MOV Data from DMA to PCI */
		/* Copy user data into DMA */
		sprintf(bds->buffer, "%s", data->data);
		iowrite32(dma_base, bds->mmio + REG_TRANS_SRC);
		iowrite32(pci_base, bds->mmio + REG_TRANS_DST);
		/* Transmit begin */
		iowrite32(DMA_DIR(direct) | DMA_RUN_CMD, 
						bds->mmio + REG_TRANS_RUN);
	} else /* MOV Data from PCI to DMA */ {
		iowrite32(data->pci_base, bds->mmio + REG_TRANS_SRC);
		iowrite32(dma_base, bds->mmio + REG_TRANS_DST);
		/* Transmit begin: Need Interrupt  */
		iowrite32(DMA_DIR(direct) | DMA_RUN_CMD | BISCUITOS_DMA_IRQ, 
						bds->mmio + REG_TRANS_RUN);
	}

	printk("PCI-MEM [%#x - %#x] Request-Range [%#x - %#x]\n",
			pci_base, pci_base + pci_size, data->pci_base,
			data->pci_base + data->size);
	printk("DMA-MEM [%#lx - %#lx] Request-Range [%#x - %#x]\n",
			(unsigned long)bds->dma_addr,
			(unsigned long)(bds->dma_addr + bds->buffer_size),
			dma_base, dma_base + dma_size);

	return 0;
}

static int BiscuitOS_DMA_dump(struct dma_desc_userspace *data, void __user *argp)
{
	int src, dst, cnt, cmd, offset;

	memset(data, 0x00, sizeof(*data));

	/* Dump DMA transmit args */
	src = ioread32(bds->mmio + REG_TRANS_SRC);
	dst = ioread32(bds->mmio + REG_TRANS_DST);
	cnt = ioread32(bds->mmio + REG_TRANS_CNT);
	cmd = ioread32(bds->mmio + REG_TRANS_CMD);

	if (DMA_DIR_EXT(cmd) == BISCUITOS_DMA_TO_PCI) { /* DMA TO PCI */
		data->dma_base = src;
		data->pci_base = dst;
		offset = src - bds->dma_addr;
	} else { /* PCI TO DMA */
		data->dma_base = dst;
		data->pci_base = src;
		offset = dst - bds->dma_addr;
	}
	data->size = cnt;
	data->dma_finish = bds->dma_finish;
	if (bds->dma_finish) {
		sprintf(data->data, "%s", bds->buffer + offset);
		bds->dma_finish = 0;
	}
	if (copy_to_user(argp, data, sizeof(*data)))
		return -EINVAL;
	return 0;
}

/* ioctl */
static long BiscuitOS_interface_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	struct dma_desc_userspace data;
	int ret;

	if (copy_from_user(&data, argp, sizeof(data)))
		goto out;

	switch (ioctl) {
	case BISCUITOS_IOCTL_DMA_FROM_PCI:
		ret = BiscuitOS_DMA_transmit(&data, BISCUITOS_DMA_FROM_PCI);
		break;
	case BISCUITOS_IOCTL_DMA_TO_PCI:
		ret = BiscuitOS_DMA_transmit(&data, BISCUITOS_DMA_TO_PCI);
		break;
	case BISCUITOS_IOCTL_DMA_TO_USER:
		ret = BiscuitOS_DMA_dump(&data, argp);
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;

out:
	return -EINVAL;
}

/* file operations */
static struct file_operations BiscuitOS_interface_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= BiscuitOS_interface_ioctl,
};

/* Misc device driver */
static struct miscdevice BiscuitOS_interface = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEV_NAME,
	.fops	= &BiscuitOS_interface_fops,
};

/* Interrup handler */
static irqreturn_t BiscuitOS_DMA_irq_handler(int irq, void *dev)
{
	/* DMA complete  */
	bds->dma_finish = 1;

	return IRQ_HANDLED;
}

/* Device Probe interface */
static int BiscuitOS_DMA_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	int ret = 0;

	/* DMA status */
	bds = kzalloc(sizeof(*bds), GFP_KERNEL);
	if (!bds) {
		ret = -ENOMEM;
		printk("%s ERROR: DMA Status allocate failed.\n", DEV_NAME);
		goto err_alloc;
	}
	bds->pdev = pdev;

	/* Enable PCI device */
	ret = pci_enable_device(pdev);
	if (ret < 0) {
		printk("%s ERROR: PCI Device Enable failed.\n", DEV_NAME);
		goto err_enable_pci;
	}

	/* Request MMIO BAR resource */
	ret = pci_request_region(pdev, MMIO_BAR, "BiscuitOS-DMA-MMIO");
	if (ret < 0) {
		printk("%s ERROR: Request MMIO BAR Failed.\n", DEV_NAME);
		goto err_request;
	}

	/* Mapping MMIO BAR */
	bds->mmio = pci_iomap(pdev, MMIO_BAR, pci_resource_len(pdev, MMIO_BAR));
	if (!bds->mmio) {
		ret = -EBUSY;
		printk("%s ERROR: Mapping MMIO Failedn\n", DEV_NAME);
		goto err_mmio;
	}

	/* Request MSI IRQ */
	ret = pci_enable_msi(pdev);
	if (ret < 0) {
		printk("%s ERROR: Enable MSI Interrupt failed.\n", DEV_NAME);
		goto err_msi;
	}
	bds->irq = pdev->irq;

	/* Register Interrupt */
	ret = request_irq(bds->irq, BiscuitOS_DMA_irq_handler,
					IRQF_SHARED, DEV_NAME, (void *)bds);
	if (ret < 0) {
		printk("%s ERROR: register irq failed.\n", DEV_NAME);
		goto err_irq;
	}

	/* Allocate DMA coherent memory */
	bds->buffer_size = DMA_BUFFER_SIZE;
	bds->buffer = dma_alloc_coherent(&pdev->dev, DMA_BUFFER_SIZE,
				&bds->dma_addr, GFP_KERNEL | GFP_DMA32);

	if (!bds->buffer) {
		printk("%s ERROR: DMA Memory allocate failed.\n", DEV_NAME);
		goto err_dma_buffer;
	}

	/* Set PCI master */
	pci_set_master(pdev);

	/* Userland interface */
	ret = misc_register(&BiscuitOS_interface);
	if (ret < 0) {
		printk("%s ERROR: MISC Interface failed.\n", DEV_NAME);
		goto err_misc;
	}

	printk("%s Success Register DMA Device.\n", DEV_NAME);
	return 0;

err_misc:
	dma_free_coherent(&pdev->dev, DMA_BUFFER_SIZE,
				bds->buffer, bds->dma_addr);
err_dma_buffer:
	free_irq(bds->irq, (void *)bds);
err_irq:
	pci_disable_msi(pdev);
err_msi:
	pci_iounmap(pdev, bds->mmio);
err_mmio:
	pci_release_region(pdev, MMIO_BAR);
err_request:
	pci_disable_device(pdev);
err_enable_pci:
	kfree(bds);
	bds = NULL;
err_alloc:
	return ret;
}

static void BiscuitOS_DMA_remove(struct pci_dev *pdev)
{
	misc_deregister(&BiscuitOS_interface);
	dma_free_coherent(&pdev->dev, DMA_BUFFER_SIZE,
				bds->buffer, bds->dma_addr);
	free_irq(bds->irq, (void *)bds);
	pci_disable_msi(pdev);
	pci_iounmap(pdev, bds->mmio);
	pci_release_region(pdev, MMIO_BAR);
	pci_disable_device(pdev);
	kfree(bds);
	bds = NULL;
}

static const struct pci_device_id BiscuitOS_DMA_ids[] = {
	{ PCI_DEVICE(0x1016, 0x1314), },
};

static struct pci_driver BiscuitOS_DMA_driver = {
	.name		= DEV_NAME,
	.id_table	= BiscuitOS_DMA_ids,
	.probe		= BiscuitOS_DMA_probe,
	.remove		= BiscuitOS_DMA_remove,
};

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	/* Register pci device */
	return pci_register_driver(&BiscuitOS_DMA_driver);
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
	/* Un-Register pci device */
	pci_unregister_driver(&BiscuitOS_DMA_driver);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS DMA Device Driver");
