/*
 * QEMU-PCI Device BiscuitOS Code on BiscuitOS
 *
 * (C) 2020.10.02 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/pci.h>
#include <linux/uaccess.h>

#define BISCUITOS_BAR		0
#define BISCUITOS_NAME		"BiscuitOS_PCI"
#define BISCUITOS_DEVICE_ID	0x11e8
#define IO_IRQ_ACK		0x64
#define IO_IRQ_STATUS		0x24
#define QEMU_VENDOR_ID		0x1234

/* Interrupt Number */
static int BiscuitOS_irq;
/* MMIO */
static void __iomem *BiscuitOS_mmio;

static struct pci_device_id BiscuitOS_pci_ids[] = {
	{ PCI_DEVICE(QEMU_VENDOR_ID, BISCUITOS_DEVICE_ID), }, 
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, BiscuitOS_pci_ids);

static ssize_t BiscuitOS_write(struct file *filp, 
			const char __user *buf, size_t len, loff_t *off)
{
	size_t ret;
	u32 buffer;

	ret = len;
	if (!(*off % 4)) {
		if (copy_from_user((void *)&buffer, buf, 4) || len != 4)
			return -EFAULT;
		else
			iowrite32(buffer, BiscuitOS_mmio + *off);
	}
	return ret;
}

static ssize_t BiscuitOS_read(struct file *filp, 
			char __user *buf, size_t len, loff_t *off)
{
	size_t ret;
	u32 buffer;

	if (*off % 4 || len == 0)
		return 0;

	buffer = ioread32(BiscuitOS_mmio + *off);
	if (copy_to_user(buf, (void *)&buffer, 4)) {
		return -EFAULT;
	} else {
		ret = 4;
		(*off)++;
	}

	return ret;
}

static loff_t BiscuitOS_llseek(struct file *filp, loff_t off, int whence)
{
	filp->f_pos = off;
	return off;
}

/* Char Device interface */
static struct file_operations BiscuitOS_fops = {
	.owner	= THIS_MODULE,
	.llseek	= BiscuitOS_llseek,
	.read	= BiscuitOS_read,
	.write	= BiscuitOS_write,
};

/* MISC Device */
static struct miscdevice BiscuitOS_misc = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= BISCUITOS_NAME,
	.fops	= &BiscuitOS_fops,
};

/* BiscuitOS IRQ Handler */
static irqreturn_t BiscuitOS_irq_handler(int irq, void *dev)
{
	irqreturn_t ret;
	u32 irq_status;
	
	irq_status = ioread32(BiscuitOS_mmio + IO_IRQ_STATUS);
	printk("Interrupt irq %d irq_status %#llx\n",
				irq, (u64)irq_status);
	/* Must do this ACK, or else the interrupts just keep firing. */
	iowrite32(irq_status, BiscuitOS_mmio + IO_IRQ_ACK);
	ret = IRQ_HANDLED;

	return ret;
}

/* Driver Probe PCI device */
static int BiscuitOS_pci_probe(struct pci_dev *dev, 
				const struct pci_device_id *id)
{
	resource_size_t start, end;
	int idx;
	u8 val;

	/* Register Misc interface */
	misc_register(&BiscuitOS_misc);

	/* Enable PCI Device */
	if (pci_enable_device(dev) < 0) {
		printk("ERROR: pci_enable_device()\n");
		goto error;
	}

	/* Register BAR */
	if (pci_request_region(dev, BISCUITOS_BAR, 
						"BiscuitOS_Region")) {
		printk("ERROR: pci_request_region()\n");
		goto error;
	}
	BiscuitOS_mmio = pci_iomap(dev, BISCUITOS_BAR, 
			pci_resource_len(dev, BISCUITOS_BAR));

	/* IRQ setup */
	pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &val);
	BiscuitOS_irq = val;
	if (request_irq(BiscuitOS_irq, BiscuitOS_irq_handler, 
		IRQF_SHARED, "BiscuitOS_irq_handler0", &BiscuitOS_misc.minor) < 0) {
		printk("ERROR: request_irq.\n");
		goto error;
	}
	printk("BiscuitOS PCI Device IRQ %d\n", BiscuitOS_irq);

	/* PCI Snity Check 
	 *
	 * In QEMU, the type is defined by either:
	 * 
	 * - PCI_BASE_ADDRESS_SPACE_IO
	 * - PCI_BASE_ADDRESS_SPACE_MEMORY
	 */
	if ((pci_resource_flags(dev, BISCUITOS_BAR) &
					IORESOURCE_MEM) != IORESOURCE_MEM) {
		printk("ERROR: pci_resource_flags.\n");
		goto error_irq;
	}

	/* 1MiB, as defined by the '1 << 20' in QEMU's memory_region_init_io.
	 * Same as pci_resource_len. 
	 */
	start = pci_resource_start(dev, BISCUITOS_BAR);
	end = pci_resource_end(dev, BISCUITOS_BAR);
	printk("BiscuitOS PCI Memory Range: %#llx - %#llx\n", start, end);

	/* The PCI standardized 64 bytes of the configuration space. */
	for (idx = 0; idx < 64; idx++) {
		pci_read_config_byte(dev, 1, &val);
		printk("BiscuitOS PCI Config %d: %x\n", idx, val);
	}

	/* Initial value of the IO Memory */
	for (idx = 0; idx < 0x30; idx++)
		printk("BiscuitOS IO %#x: %x\n", idx, 
				ioread32((void *)(BiscuitOS_mmio + idx)));

	return 0;

error_irq:
	free_irq(BiscuitOS_irq, NULL);
error:
	return -EINVAL;
}

/* BiscuitOS Device remove */
static void BiscuitOS_pci_remove(struct pci_dev *dev)
{
	free_irq(BiscuitOS_irq, NULL);
	pci_release_region(dev, BISCUITOS_BAR);
	misc_deregister(&BiscuitOS_misc);
}

/* BiscuitOS PCI Driver */
static struct pci_driver BiscuitOS_pci_driver = {
	.name	= "BiscuitOS_pci",
	.id_table = BiscuitOS_pci_ids,
	.probe	= BiscuitOS_pci_probe,
	.remove	= BiscuitOS_pci_remove,
};

static int __init BiscuitOS_init(void)
{
	if (pci_register_driver(&BiscuitOS_pci_driver) < 0)
		return -EINVAL;

	printk("Hello modules on BiscuitOS\n");

	return 0;
}

static void __exit BiscuitOS_exit(void)
{
	pci_unregister_driver(&BiscuitOS_pci_driver);
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Common Device driver on BiscuitOS");
