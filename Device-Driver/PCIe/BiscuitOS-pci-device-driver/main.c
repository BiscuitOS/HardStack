/*
 * BiscuitOS PCIe QEMU Device Driver
 *
 * (C) 2022.03.26 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2022.04.02 BiscuitOS <https://biscuitos.github.io/blog/BiscuitOS_Catalogue/>
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

#define DEV_NAME		"BiscuitOS-PCIe"
#define IO_BAR			0
#define MMIO_BAR		1
#define BAR_ADDR_REG		0x04
#define BAR_VER_REG		0x08

struct BiscuitOS_PCIe_state {
	struct pci_dev *pdev;
	/* IRQ */
	int irq;
	/* IO BAR */
	void __iomem *io;
	/* MMIO BAR */
	void __iomem *mmio;
};
static struct BiscuitOS_PCIe_state *bds;

/* Interrup handler */
static irqreturn_t BiscuitOS_PCIe_irq_handler(int irq, void *dev)
{
	/* TODO */
	return IRQ_HANDLED;
}

/* Device Probe interface */
static int BiscuitOS_PCIe_probe(struct pci_dev *pdev, const struct pci_device_id *id)
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

	/* Request IO BAR resource */
	ret = pci_request_region(pdev, IO_BAR, "BiscuitOS-PCIe-IO");
	if (ret < 0) {
		printk("%s ERROR: Request IO BAR Failed.\n", DEV_NAME);
		goto err_request_io;
	}

	/* Mapping IO BAR */
	bds->io = pci_iomap(pdev, IO_BAR, pci_resource_len(pdev, IO_BAR));
	if (!bds->io) {
		ret = -EBUSY;
		printk("%s ERROR: Mapping IO Failedn\n", DEV_NAME);
		goto err_io;
	}

	/* Request MMIO BAR resource */
	ret = pci_request_region(pdev, MMIO_BAR, "BiscuitOS-PCIe-MMIO");
	if (ret < 0) {
		printk("%s ERROR: Request MMIO BAR Failed.\n", DEV_NAME);
		goto err_request_mmio;
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
	ret = request_irq(bds->irq, BiscuitOS_PCIe_irq_handler,
					IRQF_SHARED, DEV_NAME, (void *)bds);
	if (ret < 0) {
		printk("%s ERROR: register irq failed.\n", DEV_NAME);
		goto err_irq;
	}

	/* Set master */
	pci_set_master(pdev);

	printk("%s Success Register PCIe Device.\n", DEV_NAME);

	/* Read Data From IO BAR */
	iowrite8(0x12, bds->io + BAR_ADDR_REG);
	printk("[IO-BAR] Address-Reg: %#hhx\n", ioread8(bds->io + BAR_ADDR_REG));

	/* Read Data From MMIO BAR */
	printk("[MMIO-BAR] VerionReg: %c\n", *(uint8_t *)(bds->mmio + BAR_VER_REG));

	return 0;

err_irq:
	pci_disable_msi(pdev);
err_msi:
	pci_iounmap(pdev, bds->io);
err_mmio:
	pci_release_region(pdev, MMIO_BAR);
err_request_mmio:
	pci_iounmap(pdev, bds->mmio);
err_io:
	pci_release_region(pdev, IO_BAR);
err_request_io:
	pci_disable_device(pdev);
err_enable_pci:
	kfree(bds);
	bds = NULL;
err_alloc:
	return ret;
}

static void BiscuitOS_PCIe_remove(struct pci_dev *pdev)
{

	free_irq(bds->irq, (void *)bds);
	pci_disable_msi(pdev);
	pci_iounmap(pdev, bds->io);
	pci_release_region(pdev, IO_BAR);
	pci_iounmap(pdev, bds->mmio);
	pci_release_region(pdev, MMIO_BAR);
	pci_disable_device(pdev);
	kfree(bds);
	bds = NULL;
}

static const struct pci_device_id BiscuitOS_PCIe_ids[] = {
	{ PCI_DEVICE(0x1016, 0x1413), },
};

static struct pci_driver BiscuitOS_PCIe_driver = {
	.name		= DEV_NAME,
	.id_table	= BiscuitOS_PCIe_ids,
	.probe		= BiscuitOS_PCIe_probe,
	.remove		= BiscuitOS_PCIe_remove,
};

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	/* Register pci device */
	return pci_register_driver(&BiscuitOS_PCIe_driver);
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
	/* Un-Register pci device */
	pci_unregister_driver(&BiscuitOS_PCIe_driver);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("BiscuitOS PCIe Device Driver");
