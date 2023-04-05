// SPDX-License-Identifier: GPL-2.0
/*
 * IOMMU with PCIe
 *  - Add "iomm=on intel_iommu=on"
 *
 * (C) 2023.04.05 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/pci.h>
#include <linux/iommu.h>
#include <linux/mm.h>

#define DEV_NAME		"BiscuitOS-IOMMU"
#define PAGE_NR			4
#define VBASE			0x00000000

struct BiscuitOS_device {
	/* PCI Device */
	struct pci_dev *pdev;
	/* IOMMU Domain */
	struct iommu_domain *domain;
	/* Physical Buffer */
	struct page *pages[PAGE_NR];
};
static struct BiscuitOS_device bpdev;

static int BiscuitOS_iommu_pf(struct iommu_domain *domain,
		struct device *dev, unsigned long iova, int flags, void *token)
{
	return -ENOSYS;
}

static int BiscuitOS_PCIe_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	int r, i;

	/* Enable PCI device */
	r = pci_enable_device(pdev);
	if (r < 0) {
		printk("%s ERROR: PCI Device Enable failed.\n", DEV_NAME);
		goto err_enable_pci;
	}
	pci_set_master(pdev);
	bpdev.pdev = pdev;

	/* Page Buffer */
	for (i = 0; i < PAGE_NR; i++) {
		bpdev.pages[i] = alloc_page(GFP_KERNEL);
		if (!bpdev.pages[i])
			goto err_page;
	}

	/* IOMMU Domain */
	bpdev.domain = iommu_domain_alloc(&pci_bus_type);
	if (!bpdev.domain) {
		printk("System Error: Domain failed.\n");
		r = -ENOMEM;
		goto err_domain;
	}

	/* IOMMU PageFault */
	iommu_set_fault_handler(bpdev.domain, BiscuitOS_iommu_pf, &bpdev);	

	/* Attach Device */
	r = iommu_attach_device(bpdev.domain, &pdev->dev);
	if (r) {
		printk("System Error: Can't attach iommu device.\n");
		goto err_attach;
	}

	/* IOMMU Mapping */
	for (i = 0; i < PAGE_NR; i++) {
		unsigned long pfn = page_to_pfn(bpdev.pages[i]);

		r = iommu_map(bpdev.domain, VBASE + i * PAGE_SIZE, 
				pfn << PAGE_SHIFT, PAGE_SIZE, IOMMU_READ);
		if (r) {
			printk("System Error: Faild to iommu map.\n");
			goto err_iommu_map;
		}
	}

	printk("%s Success: Mapping IOMMU.\n", DEV_NAME);

	return 0;

err_iommu_map:
	while (i--)
		iommu_unmap(bpdev.domain, VBASE + i * PAGE_SIZE, PAGE_SIZE);
err_attach:
	iommu_domain_free(bpdev.domain);
err_domain:
	while (i--)
		__free_page(bpdev.pages[i]);
err_page:
	pci_disable_device(pdev);
err_enable_pci:

	return r;
}

static void BiscuitOS_PCIe_remove(struct pci_dev *pdev)
{
	int i;

	for (i = 0; i < PAGE_NR; i++) {
		iommu_unmap(bpdev.domain, VBASE + i * PAGE_SIZE, PAGE_SIZE);
		__free_page(bpdev.pages[i]);
	}

	iommu_domain_free(bpdev.domain);
	pci_disable_device(pdev);
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
MODULE_DESCRIPTION("IOMMU with PCIe Device");
