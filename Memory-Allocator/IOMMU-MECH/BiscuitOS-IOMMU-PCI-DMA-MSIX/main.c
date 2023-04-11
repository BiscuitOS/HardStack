/*
 * Broiler PCI DMA with IOMMU
 *
 * (C) 2022.08.09 BuddyZhang1 <buddy.zhang@aliyun.com>
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
#include <linux/iommu.h>
#include <linux/delay.h>

#define DEV_NAME		"Broiler-PCI-DMA-MSIX"
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
};
static struct Broiler_pci_device bpdev;

static irqreturn_t Broiler_msix_handler(int irq, void *dev)
{
	int i;

	printk("PCI Receive Interrupt.\n");
	for (i = 0; i < 2; i++)
		printk("BUFFER-%d: %s\n", i, bpdev.dma_buffer[i]);
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
	dma_ops(bpdev.iova_base, 0x00, 0x20, PCI_TO_DDR);
	mdelay(100);
	/* 2. DMA To PCI Device, and IOVA=PAGE_SIZE */
	sprintf(bpdev.dma_buffer[1], "Hello BiscuitOS");
	/* 2.1 Write TLP */
	dma_ops(PAGE_SIZE, bpdev.iova_base + PAGE_SIZE, 0x20, DDR_TO_PCI);
	mdelay(100);
	memset(bpdev.dma_buffer[1], 0x00, PAGE_SIZE);
	/* 2.2 Read TLP */
	dma_ops(bpdev.iova_base + PAGE_SIZE, PAGE_SIZE, 0x20, PCI_TO_DDR);

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
MODULE_DESCRIPTION("Broiler PCI DMA with IOMMU");
