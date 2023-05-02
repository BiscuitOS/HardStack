// SPDX-License-Identifier: GPL-2.0
/*
 * CACHE with DMA IOMMU
 *
 * (C) 2023.05.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/pci.h>
#include <linux/iommu.h>
#include <linux/delay.h>

#define DEV_NAME		"BiscuitOS-DMA-IOMMU"
#define IO_BAR			0x00

/* DMA Register Maps */
#define DMA_SRC_REG		0x00
#define DMA_DST_REG		0x04
#define DMA_DIRT_REG		0x08
#define DMA_LEN_REG		0x0C
#define DOORBALL_REG		0x10

/* IOMMU */
#define IOMMU_VBASE		0x00000000
#define PAGE_NR			0x10
#define PCI_TO_DDR		0
#define DDR_TO_PCI		1

struct Broiler_pci_device {
	struct pci_dev *pdev;
	/* IO BAR */
	void __iomem *io;
	/* IOMMU Domain */
	struct iommu_domain *domain;
	/* Page Buffer */
	struct page *pages[PAGE_NR];
	/* IOVA */
	unsigned long iova_base;
	/* DMA Buffer */
	char *dma_buffer[PAGE_NR];
	/* Completion */
	struct completion irq_raised;
};
static struct Broiler_pci_device bpdev;

static irqreturn_t Broiler_msix_handler(int irq, void *dev)
{
	/* TODO */
	complete(&bpdev.irq_raised);
	return IRQ_HANDLED;
}

static void dma_ops(u64 src, u64 dst, u64 len, u64 direct)
{
	iowrite32(src, bpdev.io + DMA_SRC_REG);
	iowrite32(dst, bpdev.io + DMA_DST_REG);
	iowrite32(len, bpdev.io + DMA_LEN_REG);
	iowrite32(direct, bpdev.io + DMA_DIRT_REG);
	/* Doorball: kick off */
	iowrite16(0x1, bpdev.io + DOORBALL_REG);
}

static int Broiler_pci_init(struct pci_dev *pdev, 
				const struct pci_device_id *id)
{
	struct msix_entry msix;
	int r = 0;

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
	bpdev.io = pci_iomap(pdev, IO_BAR, pci_resource_len(pdev, IO_BAR));
	if (!bpdev.io) {
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
						DEV_NAME, (void *)&bpdev);
	if (r < 0) {
		printk("%s ERROR: Request IRQ failed.\n", DEV_NAME);
		goto err_irq;
	}

	/* Set master */
	pci_set_master(pdev);
	bpdev.pdev = pdev;
	init_completion(&bpdev.irq_raised);

	return 0;

err_irq:
	pci_disable_msix(pdev);
err_msix:
	pci_iounmap(pdev, bpdev.io);
err_iomap:
	pci_release_region(pdev, IO_BAR);
err_request_io:
	pci_disable_device(pdev);
err_enable_pci:
	return r;
}

static void BiscuitOS_pci_exit(struct pci_dev *pdev)
{
	free_irq(pdev->irq, NULL);
	pci_disable_msix(pdev);
	pci_iounmap(pdev, bpdev.io);
	pci_release_region(pdev, IO_BAR);
	pci_disable_device(pdev);
}

static int Broiler_pci_probe(struct pci_dev *pdev, 
				const struct pci_device_id *id)
{
	int r, i;

	r = Broiler_pci_init(pdev, id);
	if (r) {
		printk("PCIe Error: Init error.\n");
		goto err_pci;
	}

	/* IOMMU Domain */
	bpdev.domain = iommu_domain_alloc(&pci_bus_type);
	if (!bpdev.domain) {
		printk("System Error: Domain alloc.\n");
		r = -ENOMEM;
		goto err_domain;
	}

	/* Attach Device */
	r = iommu_attach_device(bpdev.domain, &pdev->dev);
	if (r) {
		printk("System Error: Can't attach IOMMU Device.\n");
		goto err_attach;
	}
	
	/* Alloc Discontinuous Physical Memory */
	for (i = 0; i < PAGE_NR; i++) {
		bpdev.pages[i] = alloc_page(GFP_KERNEL);
		if (!bpdev.pages[i]) {
			printk("System Error: No Memory for page.\n");
			goto err_page;
		}
		bpdev.dma_buffer[i] = page_address(bpdev.pages[i]);
	}

	/* IOMMU Mapping: IOVA to PFN */
	bpdev.iova_base = IOMMU_VBASE; /* Start for IOVA */
	for (i = 0; i < PAGE_NR; i++) {
		unsigned long pfn = page_to_pfn(bpdev.pages[i]);

		r = iommu_map(bpdev.domain, 
			      bpdev.iova_base + i * PAGE_SIZE,
			      pfn << PAGE_SHIFT, PAGE_SIZE,
			      IOMMU_READ | IOMMU_WRITE);
		if (r) {
			printk("System Error: IOMMU Mapping failed.\n");
			goto err_iommu_map;
		}
	}

	/* DAM With IOMMU */
	/* 1. DMA From PCI Device, and IOVA=0 */
	reinit_completion(&bpdev.irq_raised);
	memset(bpdev.dma_buffer[0], 0x00, PAGE_SIZE);
	/* 1.1 Invalidate CACHE */
	dma_sync_single_for_cpu(&bpdev.pdev->dev,
				page_to_pfn(bpdev.pages[0]) << PAGE_SHIFT,
						PAGE_SIZE, DMA_FROM_DEVICE);
	/* 1.2 DMA FROM DEVICE */
	dma_ops(bpdev.iova_base, 0x00, 0x20, PCI_TO_DDR);
	/* 1.3 Wait DMA Finish */
	wait_for_completion(&bpdev.irq_raised);
	/* 1.4 Read Data From Memory */
	printk("DMA[0]: %s\n", (char *)bpdev.dma_buffer[0]);

	/* 2. DMA To PCI Device, and IOVA=PAGE_SIZE, CACHE KVA */
	reinit_completion(&bpdev.irq_raised);
	/* 2.1 Write TO CACHE */
	sprintf(bpdev.dma_buffer[1], "Hello BiscuitOS");
	/* 2.2 FLUSH CACHE */
	dma_sync_single_for_cpu(&bpdev.pdev->dev,
				page_to_pfn(bpdev.pages[1]) << PAGE_SHIFT,
						PAGE_SIZE, DMA_TO_DEVICE);
	/* 2.3 Write TLP */
	dma_ops(PAGE_SIZE, bpdev.iova_base + PAGE_SIZE, 0x20, DDR_TO_PCI);
	/* 2.4 Wait DMA Finish */
	wait_for_completion(&bpdev.irq_raised);

	/* 3. Read TLP */
	reinit_completion(&bpdev.irq_raised);
	memset(bpdev.dma_buffer[1], 0x00, PAGE_SIZE);
	/* 3.1 Invalidate CACHE */
	dma_sync_single_for_cpu(&bpdev.pdev->dev,
				page_to_pfn(bpdev.pages[1]) << PAGE_SHIFT,
						PAGE_SIZE, DMA_FROM_DEVICE);
	/* 3.2 DMA FROM DEVICE */
	dma_ops(bpdev.iova_base + PAGE_SIZE, PAGE_SIZE, 0x20, PCI_TO_DDR);
	/* 3.3 Wait DMA Finish */
	wait_for_completion(&bpdev.irq_raised);
	/* 3.4 READ From Memory */
	printk("DMA[1]: %s\n", (char *)bpdev.dma_buffer[1]);

	return 0;

err_iommu_map:
	while (i--)
		iommu_unmap(bpdev.domain,
			bpdev.iova_base + i * PAGE_SIZE, PAGE_SIZE);
err_page:
	while (i--)
		__free_page(bpdev.pages[i]);
	iommu_detach_device(bpdev.domain, &pdev->dev);
err_attach:
	iommu_domain_free(bpdev.domain);
err_domain:
	BiscuitOS_pci_exit(pdev);
err_pci:
	return r;
}

static void Broiler_pci_remove(struct pci_dev *pdev)
{
	int i;

	for (i = 0; i < PAGE_NR; i++) {
		iommu_unmap(bpdev.domain,
			bpdev.iova_base + i * PAGE_SIZE, PAGE_SIZE);
		__free_page(bpdev.pages[i]);
	}
	iommu_detach_device(bpdev.domain, &pdev->dev);
	iommu_domain_free(bpdev.domain);
	BiscuitOS_pci_exit(pdev);
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

static void __exit Broiler_exit(void)
{
	pci_unregister_driver(&Broiler_PCIe_driver);
}

module_init(Broiler_init);
module_exit(Broiler_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("CACHE with DMA IOMMU");
