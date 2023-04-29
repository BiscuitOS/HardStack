// SPDX-License-Identifier: GPL-2.0
/*
 * DMA-BUF Import: GPUA
 *
 * (C) 2022.08.09 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/pci.h>
#include <linux/dma-buf.h>
#include <linux/dma-direction.h>

#define DEV_NAME		"BiscuitOS-DMABUF-ImportA"
#define IO_BAR			0x00

/* DMA Register Maps */
#define DMA_SRC_REG		0x00
#define DMA_DST_REG		0x04
#define DMA_DIRT_REG		0x08
#define DMA_LEN_REG		0x0C
#define DOORBALL_REG		0x10

#define DMA_BUFFER_LEN		4096
#define PCI_TO_DDR		0
#define DDR_TO_PCI		1

/* IOCTL CMD */
#define BISCUITOS_IO		0xBD
#define BISCUITOS_PCI_TO_MEM	_IO(BISCUITOS_IO, 0x00)
#define BISCUITOS_MEM_TO_PCI	_IO(BISCUITOS_IO, 0x01)
#define BISCUITOS_DMABUF_FD	_IO(BISCUITOS_IO, 0x02)

struct Broiler_pci_device {
	struct pci_dev *pdev;
	/* IO BAR */
	void __iomem *io;
	/* DMA Physical address */
	dma_addr_t dma_addr;
	/* DMA Remapping Virtual address */
	char *buffer;
	/* DMA-BUF */
	struct dma_buf *dmabuf;
};
static struct Broiler_pci_device *bpdev;

static irqreturn_t Broiler_msix_handler(int irq, void *dev)
{
	printk("GPU-A Receive Data From DDR\n");
	return IRQ_HANDLED;
}

static void Broiler_dma_ops(u64 src, u64 dst, u64 len, u64 direct)
{
	iowrite32(src, bpdev->io + DMA_SRC_REG);
	iowrite32(dst, bpdev->io + DMA_DST_REG);
	iowrite32(len, bpdev->io + DMA_LEN_REG);
	iowrite32(direct, bpdev->io + DMA_DIRT_REG);
	/* Doorball: kick off */
	iowrite16(0x1, bpdev->io + DOORBALL_REG);
}

static long BiscuitOS_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	int fd, r;

	switch (ioctl) {
	case BISCUITOS_PCI_TO_MEM:
		/* DMA: PCI to DDR -> Memory Write TLP */
		Broiler_dma_ops(0x00, bpdev->dma_addr, PAGE_SIZE, PCI_TO_DDR);
		break;
	case BISCUITOS_MEM_TO_PCI: {
		struct dma_buf_attachment *attachment;
		struct sg_table *table;
		unsigned long addr;

		attachment = dma_buf_attach(bpdev->dmabuf, &bpdev->pdev->dev);
		if (!attachment) {
			printk("System Error: %s attach failed\n", DEV_NAME);
			return -EINVAL;
		}
		table = dma_buf_map_attachment(attachment, DMA_TO_DEVICE);
		if (!table) {
			printk("System Error: %s table failed.\n", DEV_NAME);
			dma_buf_detach(bpdev->dmabuf, attachment);
			return -EINVAL;
		}

		addr = sg_dma_address(table->sgl);
		/* DMA: DDR to PCI -> Memory Write TLP */
		Broiler_dma_ops(addr, 0x00, PAGE_SIZE, DDR_TO_PCI);

		dma_buf_unmap_attachment(attachment, table, DMA_TO_DEVICE);
		dma_buf_detach(bpdev->dmabuf, attachment);
		break;
	}
	case BISCUITOS_DMABUF_FD:
		r = copy_from_user(&fd, (void __user *)arg, sizeof(int));
		if (r < 0)
			return -EINVAL;

		bpdev->dmabuf = dma_buf_get(fd);
		break;
	default:
		break;
	}
	return 0;
}

static int BiscuitOS_release(struct inode * inode, struct file *filp)
{
	dma_buf_put(bpdev->dmabuf);
	return 0;
}

static int BiscuitOS_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return dma_buf_mmap(bpdev->dmabuf, vma, vma->vm_pgoff);
}

static struct file_operations BiscuitOS_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= BiscuitOS_ioctl,
	.release	= BiscuitOS_release,
	.mmap		= BiscuitOS_mmap,
};

static struct miscdevice BiscuitOS_drv = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEV_NAME,
	.fops	= &BiscuitOS_fops,
};

static int Broiler_pci_probe(struct pci_dev *pdev, 
				const struct pci_device_id *id)
{
	struct msix_entry msix;
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
	r = pci_request_region(pdev, IO_BAR, "Broiler-PCIe-INTX-IO");
	if (r < 0) {
		printk("%s ERROR: Request IO-BAR Failed.\n", DEV_NAME);
		goto err_request_io;
	}

	/* Remapping IO-BAR */
	bpdev->io = pci_iomap(pdev, IO_BAR, pci_resource_len(pdev, IO_BAR));
	if (!bpdev->io) {
		r = -EBUSY;
		printk("%s ERROR: Remapping IO Failed\n", DEV_NAME);
		goto err_iomap;
	}

	/* Reqeust MSIX Interrupt */
	msix = (struct msix_entry) {
		.vector = 0,
		.entry  = 0, /* 1st msix interrupt */
	};
	r = pci_enable_msix_range(pdev, &msix, 1, 1);
	if (r != 1) { /* Only request 1 MSIX Interrupt */
		printk("%s ERROR: Enable MSIX failed.\n", DEV_NAME);
		goto err_msix;
	}

	r = request_irq(msix.vector, Broiler_msix_handler,
				IRQF_SHARED | IRQF_TRIGGER_HIGH,
						DEV_NAME, (void *)bpdev);
	if (r < 0) {
		printk("%s ERROR: Request IRQ failed.\n", DEV_NAME);
		goto err_irq;
	}

	/* Set master */
	pci_set_master(pdev);

	/* MSIC Interface */
	misc_register(&BiscuitOS_drv);

	printk("%s is Ready.\n", DEV_NAME);

	return 0;

err_irq:
	pci_disable_msix(pdev);
err_msix:
	pci_iounmap(pdev, bpdev->io);
err_iomap:
	pci_release_region(pdev, IO_BAR);
err_request_io:
	pci_disable_device(pdev);
err_enable_pci:
	kfree(bpdev);
	bpdev = NULL;
err_alloc:
	return r;
}

static void Broiler_pci_remove(struct pci_dev *pdev)
{
	misc_deregister(&BiscuitOS_drv);
	free_irq(pdev->irq, NULL);
	pci_disable_msix(pdev);
	pci_iounmap(pdev, bpdev->io);
	pci_release_region(pdev, IO_BAR);
	pci_disable_device(pdev);
	kfree(bpdev);
	bpdev = NULL;
}

static const struct pci_device_id Broiler_pci_ids[] = {
	{ PCI_DEVICE(0x1019, 0x1991), },
};

static struct pci_driver Broiler_PCIe_driver = {
	.name		= DEV_NAME,
	.id_table	= Broiler_pci_ids,
	.probe		= Broiler_pci_probe,
	.remove		= Broiler_pci_remove,
};

static int __init Broiler_init(void)
{
	return pci_register_driver(&Broiler_PCIe_driver);
}

static void __exit Broiler_exit(void)
{
	pci_unregister_driver(&Broiler_PCIe_driver);
}

module_init(Broiler_init);
module_exit(Broiler_exit);

MODULE_IMPORT_NS(DMA_BUF);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("DMA-BUF Import: GPUA");
