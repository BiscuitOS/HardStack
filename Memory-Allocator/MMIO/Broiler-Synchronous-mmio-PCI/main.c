/*
 * Broiler Synchronous MMIO with PCI Device Driver
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

#define DEV_NAME		"BiscuitOS-Synchronous-MMIO-PCI"
#define MMIO_BAR		0
/* BAR0 and BAR1 bitmap */
#define SLOT_NUM_REG		0x00
#define SLOT_SEL_REG		0x04
#define MIN_FREQ_REG		0x08
#define MAX_FREQ_REG		0x0C

struct BiscuitOS_PCIe_state {
	struct pci_dev *pdev;
	/* MMIO BAR */
	void __iomem *mmio;
};
static struct BiscuitOS_PCIe_state *bds;

/* Device Probe interface */
static int BiscuitOS_PCIe_probe(struct pci_dev *pdev, 
				const struct pci_device_id *id)
{
	u32 __iomem *addr;
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

	/* Set master */
	pci_set_master(pdev);

	printk("%s Success Register PCIe Device.\n", DEV_NAME);

	/* Slot information */
	printk("Slot Number: %#x\n", *(uint8_t *)(bds->mmio + SLOT_NUM_REG));
	addr = (u32 *)((uint8_t *)bds->mmio + SLOT_SEL_REG);
	*addr = 0x02;
	printk("Slot Select: %#x\n", *(uint8_t *)(bds->mmio + SLOT_SEL_REG));

	/* Read Frequency From IO/MM BAR */
	printk("Frequency Range: %#x - %#x\n",
			*(uint8_t *)(bds->mmio + MIN_FREQ_REG),
			*(uint8_t *)(bds->mmio + MAX_FREQ_REG));

	return 0;

err_mmio:
	pci_release_region(pdev, MMIO_BAR);
err_request_mmio:
	pci_disable_device(pdev);
err_enable_pci:
	kfree(bds);
	bds = NULL;
err_alloc:
	return ret;
}

static void BiscuitOS_PCIe_remove(struct pci_dev *pdev)
{

	pci_iounmap(pdev, bds->mmio);
	pci_release_region(pdev, MMIO_BAR);
	pci_disable_device(pdev);
	kfree(bds);
	bds = NULL;
}

static const struct pci_device_id BiscuitOS_PCIe_ids[] = {
	{ PCI_DEVICE(0x1006, 0x1991), },
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
MODULE_DESCRIPTION("Broiler Synchronous MMIO with PCI Device Driver");
