/*
 * PCIe Device Driver
 *
 * (C) 2019.10.24 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/version.h>
#include <linux/slab.h>

#define DEV_NAME		"PCIe_demo"
#define PCIE_DEMO_BAR_NUM	6
#define PCIE_DEMO_BAR_SIZE	(0x8000UL)
#define PCIE_DEMO_OFS_CONFIG	(0x3000UL)
#define CONFIG_BLOCK_ID		0x1fc30000UL

struct PCIe_demo_dev
{
	struct pci_dev *pdev;
	/* PCIe BAR management */
	void *__iomem bar[PCIE_DEMO_BAR_NUM];
	int user_bar_idx;	/* BAR index of user logic */
	int config_bar_idx;	/* BAR index of config logic */
	int bypass_bar_idx;	/* BAR index of bypass logic */
	int regions_in_use;	/* dev was in use during probe() */
	int got_regions;	/* probe() obtained the region */
};

struct config_regs
{
	u32 identifier;
	u32 reserved_1[4];
	u32 msi_enable;
};

static const struct pci_device_id PCIe_ids[] = {
	{ PCI_DEVICE(0x8868, 0x1188), },
};

inline u32 read_register(void *iomem)
{
	return ioread32(iomem);
}

/*
 * Unmap the BAR regions that had been mapped earlier using map_bars()
 */
static void unmap_bars(struct PCIe_demo_dev *xdev, struct pci_dev *pdev)
{
	int i;

	for (i = 0; i < PCIE_DEMO_BAR_NUM; i++) {
		/* is this BAR mapped */
		if (xdev->bar[i]) {
			/* unmap BAR */
			pci_iounmap(pdev, xdev->bar[i]);
			/* mark as unmapped */
			xdev->bar[i] = NULL;
		}
	}
}

static int map_single_bar(struct PCIe_demo_dev *xdev, 
					struct pci_dev *pdev, int idx)
{
	resource_size_t bar_start;
	resource_size_t bar_len;
	resource_size_t map_len;

	bar_start = pci_resource_start(pdev, idx);
	bar_len = pci_resource_len(pdev, idx);
	map_len = bar_len;

	xdev->bar[idx] = NULL;

	/* do not map BARs with length 0. Note that start MAY be 0! */
	if (!bar_len)
		return 0;

	if (bar_len > INT_MAX)
		map_len = (resource_size_t)INT_MAX;

	xdev->bar[idx] = pci_iomap(pdev, idx, map_len);
	if (!xdev->bar[idx]) {
		printk("Could not map BAR %d.\n", idx);
		return -1;
	}

	printk("BAR%d at %#llx mapped at %#llx, length=%#llx\n", idx,
		(u64)bar_start, (u64)xdev->bar[idx], (u64)map_len);

	return (int)map_len;
}

static int is_config_bar(struct PCIe_demo_dev *xdev, int idx)
{
	u32 cfg_id = 0;
	u32 mask = 0xffff0000; /* Compare only ID's not Version number */
	struct config_regs *cfg_regs =
		(struct config_regs *)(xdev->bar[idx] + PCIE_DEMO_OFS_CONFIG);

	cfg_id = read_register(&cfg_regs->identifier);

	if ((cfg_id & mask) == CONFIG_BLOCK_ID)
		return 1; /* Configure BAR */
	else
		return 0;
}

static void identify_bars(struct PCIe_demo_dev *xdev, int *bar_id_list,
					int num_bars, int config_bar_pos)
{
	/*
	 * The following logic identifies which BARs contain what functionality
	 * based on the position of the config BAR and the number of BARs
	 * detected. The rules are that the user logic and bypass logic BARs
	 * are optional.  When both are present, the config BAR will be the
	 * 2nd BAR detected (config_bar_pos = 1), with the user logic being
	 * detected first and the bypass being detected last. When one is
	 * omitted, the type of BAR present can be identified by whether the
	 * config BAR is detected first or last.  When both are omitted,
	 * only the config BAR is present.  This somewhat convoluted
	 * approach is used instead of relying on BAR numbers in order to work
	 * correctly with both 32-bit and 64-bit BARs.
	 */
	switch (num_bars) {
	case 1:
		/* Only one BAR present - no extra work necessary */
		break;
	case 2:
		if (config_bar_pos == 0) {
			xdev->bypass_bar_idx = bar_id_list[1];
		} else if (config_bar_pos == 1) {
			xdev->user_bar_idx = bar_id_list[0];
		} else {
			printk("2. config BAR unexpected %d\n",
							config_bar_pos);
		}
		break;
	case 3:
	case 4:
		if ((config_bar_pos == 1) || (config_bar_pos == 2)) {
			/* user bar at bar #0 */
			xdev->user_bar_idx = bar_id_list[0];
			/* bypass bar at the last bar */
			xdev->bypass_bar_idx = bar_id_list[num_bars - 1];
		} else {
			printk("3/4, config BAR unexpected %d\n",
							config_bar_pos);
		}
		break;
	default:
		/* Should not occur - warn user bar safe to continue */
		printk("Unexpected # BARs (%d), config BAR only.\n",
							num_bars);
	}
	printk("%d BARs: config %d, user %d, bypass %d\n",
		num_bars, config_bar_pos, xdev->user_bar_idx,
		xdev->bypass_bar_idx);
}

/* map_bars() -- map device regions into kernel virtual address space.
 *
 * Map the device memory regions into kernel virtual address space after
 * verifying their sizes respect the minimum sizes needed.
 */
static int map_bars(struct PCIe_demo_dev *xdev, struct pci_dev *pdev)
{
	int bar_id_list[PCIE_DEMO_BAR_NUM];
	int config_bar_pos = 0;
	int bar_id_idx = 0;
	int idx, rv;

	/* Iterate through all the BARs */
	for (idx = 0; idx < PCIE_DEMO_BAR_NUM; idx++) {
		int bar_len;

		bar_len = map_single_bar(xdev, pdev, idx);
		if (bar_len == 0) {
			continue;
		} else if (bar_len < 0) {
			rv = -EINVAL;
			goto fail;
		}

		/* Try to identify BAR as control BAR */
		if ((bar_len >= PCIE_DEMO_BAR_SIZE) && 
				(xdev->config_bar_idx < 0)) {
			if (is_config_bar(xdev, idx)) {
				xdev->config_bar_idx = idx;
				config_bar_pos = bar_id_idx;
			}
		}
		bar_id_list[bar_id_idx] = idx;
		bar_id_idx++;
	}

	/* The config BAR must always be present */
	if (xdev->config_bar_idx < 0) {
		printk("Failed to detect Config BAR\n");
		rv = -EINVAL;
		goto fail;
	}

	identify_bars(xdev, bar_id_list, bar_id_idx, config_bar_pos);

fail:
	/* unwind: unmap any BARs that we did map */
	unmap_bars(xdev, pdev);
	return rv;
}

/* PCIe device probe */
static int PCIe_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct PCIe_demo_dev *xdev;
	int rv;
	u16 v;

	xdev = kzalloc(sizeof(*xdev), GFP_KERNEL);
	if (!xdev) {
		printk("No free memory for xdma\n");
		rv = -ENOMEM;
		goto err_all;
	}
	xdev->pdev = pdev;
	xdev->user_bar_idx = -1;
	xdev->config_bar_idx = -1;
	xdev->bypass_bar_idx = -1;

	/* Enable PCIe Device */
	rv = pci_enable_device(pdev);
	if (rv) {
		printk("PCIe Enable faild\n");
		goto err_enable;
	}

	/* Keep INTx enable */
	pci_read_config_word(pdev, PCI_STATUS, &v);
	if (v & PCI_STATUS_INTERRUPT) {
		printk("%s PCI STATUS Interrupt pending 0x%x\n",
			dev_name(&pdev->dev), v);
		pci_write_config_word(pdev, PCI_STATUS, PCI_STATUS_INTERRUPT);
	}

	/* enable relaxed ordering */
	pcie_capability_set_word(pdev, PCI_EXP_DEVCTL, PCI_EXP_DEVCTL_RELAX_EN);

	/* Force MRRS to be 512 */
	rv = pcie_set_readrq(pdev, 512);
	if (rv)
		printk("PCI_EXP_DEVCTL_READRQ error\n");

	/* Enable bus master capability */
	pci_set_master(pdev);

	/* Request region */
	rv = pci_request_regions(pdev, DEV_NAME);
	if (rv) {
		printk("PCIe region in use?\n");
		xdev->regions_in_use = 1;
		goto err_region;
	} else {
		xdev->got_regions = 1;
	}

	/* map bar */
	map_bars(xdev, pdev);

	dev_set_drvdata(&pdev->dev, xdev);

	printk("PCIe Device: %#x:%#x Probe Ok....\n", id->vendor, id->device);
	return 0;

err_region:
	if (xdev->regions_in_use)
		pci_disable_device(pdev);
err_enable:
	kfree(xdev);
err_all:
	return rv;
}

/* PCIe device remove */
static void PCIe_remove(struct pci_dev *pdev)
{
	struct PCIe_demo_dev *xdev = dev_get_drvdata(&pdev->dev);

	/* unmap region */
	unmap_bars(xdev, pdev);

	if (xdev->got_regions)
		pci_release_regions(pdev);

	/* Disable pci device */
	if (!xdev->regions_in_use)
		pci_disable_device(pdev);

	kfree(xdev);
}

static struct pci_driver PCIe_demo_driver = {
	.name		= DEV_NAME,
	.id_table	= PCIe_ids,
	.probe		= PCIe_probe,
	.remove		= PCIe_remove,
};

static int __init PCIe_demo_init(void)
{
	return pci_register_driver(&PCIe_demo_driver);
}

static void __exit PCIe_demo_exit(void)
{
	pci_unregister_driver(&PCIe_demo_driver);
}

module_init(PCIe_demo_init);
module_exit(PCIe_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("PCIe Device Driver Module");
