/*
 * BiscuitOS DMA QEMU Device Driver
 *
 * (C) 2022.03.26 BuddyZhang1 <buddy.zhang@aliyun.com>
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

/* DD Platform Name */
#define DEV_NAME		"BiscuitOS-DMA"
/* IOCTL CMD */
#define BISCUITOS_IO		0xAE
#define BISCUITOS_SET		_IO(BISCUITOS_IO, 0x00)
#define BISCUITOS_GET		_IO(BISCUITOS_IO, 0x01)

/* DMA */
#define MMIO_BAR		0x00
#define DMA_BUFFER_SIZE		0x100000

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
};
static struct BiscuitOS_DMA_state *bds;

/* ioctl */
static long BiscuitOS_interface_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	switch (ioctl) {
	case BISCUITOS_SET:
		printk("IOCTL: BISCUITOS_SET.\n");
		break;
	case BISCUITOS_GET:
		printk("IOCTL: BISCUITOS_GET.\n");
		break;
	default:
		break;
	}
	return 0;
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

static const struct pci_device_id BiscuitOS_DMA_ids[] = {
	{ PCI_DEVICE(0x1016, 0x1413), },
};

static struct pci_driver BiscuitOS_DMA_driver = {
	.name		= DEV_NAME,
	.id_table	= BiscuitOS_DMA_ids,
	.probe		= BiscuitOS_DMA_probe,
};

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	/* Register pci device */
	return pci_register_driver(&BiscuitOS_DMA_driver);
	//misc_register(&BiscuitOS_interface);
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
	/* Un-Register pci device */
	pci_unregister_driver(&BiscuitOS_DMA_driver);
	/* Un-Register Misc device */
	//misc_deregister(&BiscuitOS_interface);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS DMA Device Driver");
