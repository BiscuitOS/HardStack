/*
 * Broiler Asychronous PIO with PCI
 *
 * (C) 2022.08.07 BuddyZhang1 <buddy.zhang@aliyun.com>
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

#define DEV_NAME		"Broiler-Asynchronous-PIO-PCI"
#define IO_BAR			0x00
#define DOORBALL_REG		0x10

struct Broiler_pci_device {
	struct pci_dev *pdev;
	/* MEM BAR */
	void __iomem *pio;
};
static struct Broiler_pci_device *bpdev;

static irqreturn_t Broiler_pci_irq_handler(int irq, void *dev)
{
	/* TODO */
	printk("%s Receive IRQ %d\n", DEV_NAME, irq);
	return IRQ_HANDLED;
}

/* Device Probe interface */
static int Broiler_pci_probe(struct pci_dev *pdev, 
				const struct pci_device_id *id)
{
	int r = 0;

	bpdev = kzalloc(sizeof(*bpdev), GFP_KERNEL);
	if (!bpdev) {
		r = -ENOMEM;
		printk("%s ERROR: Broiler PCI allocate failed.\n", DEV_NAME);
		goto err_alloc;
	}
	bpdev->pdev = pdev;

	/* Enable PCI device */
	r = pci_enable_device(pdev);
	if (r < 0) {
		printk("%s ERROR: PCI Device Enable failed.\n", DEV_NAME);
		goto err_enable_pci;
	}

	/* Request IO BAR resource */
	r = pci_request_region(pdev, IO_BAR, DEV_NAME);
	if (r < 0) {
		printk("%s ERROR: Request IO-BAR Failed.\n", DEV_NAME);
		goto err_request_pio;
	}

        /* Mapping IO-BAR */
        bpdev->pio = pci_iomap(pdev, IO_BAR,
				pci_resource_len(pdev, IO_BAR));
        if (!bpdev->pio) {
                r = -EBUSY;
                printk("%s ERROR: Mapping IO Failedn\n", DEV_NAME);
                goto err_pio;
        }

	/* Reqeust INTX IRQ */
	pci_intx(pdev, 1);
	r = request_irq(pdev->irq, Broiler_pci_irq_handler,
				IRQF_SHARED | IRQF_TRIGGER_HIGH,
						DEV_NAME, (void *)bpdev);
	if (r < 0) {
		printk("%s ERROR: Request IRQ failed.\n", DEV_NAME);
		goto err_irq;
	}

	/* Set master */
	pci_set_master(pdev);

	printk("%s Success Register PCIe Device.\n", DEV_NAME);
	/* kick off */
	iowrite32(0x01, bpdev->pio + DOORBALL_REG);

	return 0;

err_irq:
	pci_iounmap(pdev, bpdev->pio);
err_pio:
	pci_release_region(pdev, IO_BAR);
err_request_pio:
	pci_disable_device(pdev);
err_enable_pci:
	kfree(bpdev);
	bpdev = NULL;
err_alloc:
	return r;
}

static void Broiler_pci_remove(struct pci_dev *pdev)
{
	free_irq(pdev->irq, NULL);
	pci_intx(pdev, 0);	
	pci_iounmap(pdev, bpdev->pio);
	pci_release_region(pdev, IO_BAR);
	pci_disable_device(pdev);
	kfree(bpdev);
	bpdev = NULL;
}

static const struct pci_device_id Broiler_pci_ids[] = {
	{ PCI_DEVICE(0x1009, 0x1991), },
};

static struct pci_driver Broiler_PCIe_driver = {
	.name		= DEV_NAME,
	.id_table	= Broiler_pci_ids,
	.probe		= Broiler_pci_probe,
	.remove		= Broiler_pci_remove,
};

/* Module initialize entry */
static int __init Broiler_init(void)
{
	/* Register pci device */
	return pci_register_driver(&Broiler_PCIe_driver);
}

/* Module exit entry */
static void __exit Broiler_exit(void)
{
	/* Un-Register pci device */
	pci_unregister_driver(&Broiler_PCIe_driver);
}

module_init(Broiler_init);
module_exit(Broiler_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Broiler Asynchronous PIO with PCI");
