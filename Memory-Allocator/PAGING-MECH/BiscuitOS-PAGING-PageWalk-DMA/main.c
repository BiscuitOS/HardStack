// SPDX-License-Identifier: GPL-2.0
/*
 * PageWalk with PCIe DMA
 *
 * (C) 2023.08.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/pci.h>
#include <linux/pagewalk.h>

#define DEV_NAME		"Broiler-PCI-DMA-MSIX"
#define IO_BAR			0x00

/* DMA Register Maps */
#define DMA_SRC_REG		0x00
#define DMA_DST_REG		0x04
#define DMA_DIRT_REG		0x08
#define DMA_LEN_REG		0x0C
#define DOORBALL_REG		0x10

#define DMA_BUFFER_LEN		(16 * 1024 * 1024)
#define PCI_TO_DDR		0
#define DDR_TO_PCI		1

struct Broiler_pci_device {
	struct pci_dev *pdev;
	/* IO BAR */
	void __iomem *io;
	/* DMA Physical address */
	dma_addr_t dma_addr;
	/* DMA Remapping Virtual address */
	char *dma_buffer;
};
static struct Broiler_pci_device *bpdev;

static int BiscuitOS_pmd_entry(pmd_t *pmd, unsigned long addr,
                        unsigned long next, struct mm_walk *walk)
{
	if (pmd_none(*pmd))
		return 0;

	printk("BS Virtual Addr: %#lx\n", addr);
	printk("BS PageTB PDE:   %#lx\n", pmd_val(*pmd));
	printk("BS Phys:         %#lx\n", pmd_pfn(*pmd) << PAGE_SHIFT);
	printk("BS DMA Phys:     %#lx\n", (unsigned long)bpdev->dma_addr);

	return 0;
}

static const struct mm_walk_ops BiscuitOS_pwalk_ops = {
	.pmd_entry	= BiscuitOS_pmd_entry,
};

static irqreturn_t Broiler_msix_handler(int irq, void *dev)
{
	/* TODO */
	printk("DMA Buffer[%s]\n", bpdev->dma_buffer);

	/* PageWalk */
	mmap_write_lock_killable(&init_mm);
	walk_page_range_novma(&init_mm, (unsigned long)bpdev->dma_buffer,
		(unsigned long)bpdev->dma_buffer + PAGE_SIZE,
		&BiscuitOS_pwalk_ops, init_mm.pgd, NULL);

	mmap_write_unlock(&init_mm);

	return IRQ_HANDLED;
}

static void dma_ops(u64 src, u64 dst, u64 len, u64 direct)
{
	iowrite32(src, bpdev->io + DMA_SRC_REG);
	iowrite32(dst, bpdev->io + DMA_DST_REG);
	iowrite32(len, bpdev->io + DMA_LEN_REG);
	iowrite32(direct, bpdev->io + DMA_DIRT_REG);
	/* Doorball: kick off */
	iowrite16(0x1, bpdev->io + DOORBALL_REG);
}

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

	printk("%s Success Register PCIe Device.\n", DEV_NAME);

	/* DMA Remapping */
	bpdev->dma_buffer = dma_alloc_coherent(&pdev->dev,
				DMA_BUFFER_LEN, &bpdev->dma_addr, GFP_KERNEL);
	if (!bpdev->dma_buffer) {
		r = -ENOMEM;
		printk("%s ERROR: DMA Alloc and Remapping failed.\n", DEV_NAME);
		goto err_dma;
	}

	/* DMA: PCI to DDR -> Memory Write TLP */
	dma_ops(0x00, bpdev->dma_addr, 0x20, PCI_TO_DDR);

	return 0;

err_dma:
	free_irq(msix.vector, NULL);
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
	dma_free_coherent(&pdev->dev, DMA_BUFFER_LEN,
			bpdev->dma_buffer, bpdev->dma_addr);
	free_irq(pdev->irq, NULL);
	pci_disable_msix(pdev);
	pci_iounmap(pdev, bpdev->io);
	pci_release_region(pdev, IO_BAR);
	pci_disable_device(pdev);
	kfree(bpdev);
	bpdev = NULL;
}

static const struct pci_device_id Broiler_pci_ids[] = {
	{ PCI_DEVICE(0x1026, 0x1991), },
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
device_initcall(Broiler_init);
