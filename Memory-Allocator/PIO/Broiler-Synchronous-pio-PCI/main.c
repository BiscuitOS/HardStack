/*
 * Broiler Synchronous IO on PCI Device Driver
 *
 * (C) 2022.07.22 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2022.07.22 BiscuitOS <https://biscuitos.github.io/>
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

#define DEV_NAME		"Broiler Synchronous IO on PCI"
#define IO_BAR			0
#define MMIO_BAR		1
/* BAR0 and BAR1 bitmap */
#define SLOT_NUM_REG		0x00
#define SLOT_SEL_REG		0x04
#define MIN_FREQ_REG		0x08
#define MAX_FREQ_REG		0x0C

struct BiscuitOS_PCIe_state {
	struct pci_dev *pdev;
	/* IO BAR */
	void __iomem *io;
};
static struct BiscuitOS_PCIe_state *bds;

/* Device Probe interface */
static int BiscuitOS_PCIe_probe(struct pci_dev *pdev, 
				const struct pci_device_id *id)
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
	ret = pci_request_region(pdev, IO_BAR, DEV_NAME);
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

	/* Set master */
	pci_set_master(pdev);

	printk("%s Success Register PCIe Device.\n", DEV_NAME);

	/* Slot information */
	printk("Slot Number: %#x\n", ioread32(bds->io + SLOT_NUM_REG));
	iowrite32(0x02, bds->io + SLOT_SEL_REG);
	printk("Slot Select: %#x\n", ioread32(bds->io + SLOT_SEL_REG));

	/* Read Frequency From IO/MM BAR */
	printk("Frequency Range: %#x - %#x\n",
			ioread32(bds->io + MIN_FREQ_REG),
			ioread32(bds->io + MAX_FREQ_REG));

	return 0;

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

	pci_iounmap(pdev, bds->io);
	pci_release_region(pdev, IO_BAR);
	pci_disable_device(pdev);
	kfree(bds);
	bds = NULL;
}

static const struct pci_device_id BiscuitOS_PCIe_ids[] = {
	{ PCI_DEVICE(0x1003, 0x1991), },
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
MODULE_DESCRIPTION("Broiler Sychronous IO on PCI Device Driver");
