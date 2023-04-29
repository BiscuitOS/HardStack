// SPDX-License-Identifier: GPL-2.0
/*
 * DMA-BUF Export: Video Capture
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
#include <linux/kallsyms.h>
#include <linux/dma-map-ops.h>
#include <linux/poll.h>
#include <linux/dma-buf.h>
#include <linux/uaccess.h>

#define DEV_NAME		"BiscuitOS-DMABUF-Export"
#define IO_BAR			0x00

/* DMA Register Maps */
#define PCI_ADDR_REG		0x00
#define DDR_ADDR_REG		0x04
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
#define BISCUITOS_DMABUF_EXP	_IO(BISCUITOS_IO, 0x02)

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
static wait_queue_head_t BiscuitOS_wait;
static int BiscuitOS_dma_ready;

static irqreturn_t Broiler_msix_handler(int irq, void *dev)
{
	BiscuitOS_dma_ready = 1;
	wake_up_interruptible(&BiscuitOS_wait);
	return IRQ_HANDLED;
}

/*
 * DMA: Between PCI and DDR
 *  @pci_addr: Offset address of PCI
 *  @ddr_addr: The address of DDR
 *  @len: Translation length
 */
static void Broiler_dma_ops(u64 pci_addr, u64 ddr_addr, u64 len, u64 direct)
{
	iowrite32(pci_addr, bpdev->io + PCI_ADDR_REG);
	iowrite32(ddr_addr, bpdev->io + DDR_ADDR_REG);
	iowrite32(len, bpdev->io + DMA_LEN_REG);
	iowrite32(direct, bpdev->io + DMA_DIRT_REG);
	/* Doorball: kick off */
	iowrite16(0x1, bpdev->io + DOORBALL_REG);
}

static long BiscuitOS_ioctl(struct file *filp,
                        unsigned int ioctl, unsigned long arg)
{
	int fd;

	switch (ioctl) {
	case BISCUITOS_PCI_TO_MEM:
		BiscuitOS_dma_ready = 0;
		/* DMA: PCI to DDR -> Memory Write TLP */
		Broiler_dma_ops(0x00, bpdev->dma_addr, PAGE_SIZE, PCI_TO_DDR);
		break;
	case BISCUITOS_MEM_TO_PCI:
		/* DMA: DDR to PCI -> Memory Write TLP */
		Broiler_dma_ops(bpdev->dma_addr, 0x00, PAGE_SIZE, DDR_TO_PCI);
		break;
	case BISCUITOS_DMABUF_EXP:
		fd = dma_buf_fd(bpdev->dmabuf, O_CLOEXEC);

		return copy_to_user((int __user *)arg, &fd, sizeof(fd));
	default:
		break;
	}
	return 0;
}

static
__poll_t BiscuitOS_poll(struct file *filp, struct poll_table_struct *wait)
{
	unsigned long mask = 0;

	poll_wait(filp, &BiscuitOS_wait, wait);
	if (BiscuitOS_dma_ready)
		mask = POLLIN;
	else
		mask = 0;

	BiscuitOS_dma_ready = 0;
	return mask;
}

static struct file_operations BiscuitOS_fops = {
	.owner          = THIS_MODULE,
	.poll		= BiscuitOS_poll,
	.unlocked_ioctl	= BiscuitOS_ioctl,
};

static struct miscdevice BiscuitOS_drv = {
	.minor  = MISC_DYNAMIC_MINOR,
	.name   = DEV_NAME,
	.fops   = &BiscuitOS_fops,
};

/** DMA BUF **/
static
struct sg_table *BiscuitOS_map_dma_buf(struct dma_buf_attachment *attach,
						enum dma_data_direction dir)
{
	struct sg_table *table;

	table = kmalloc(sizeof(*table), GFP_KERNEL);
	if (!table) {
		printk("System Error: %s table memory.\n", DEV_NAME);
		return NULL;
	}

	sg_alloc_table(table, 1, GFP_KERNEL);
	sg_dma_len(table->sgl) = PAGE_SIZE;
	sg_dma_address(table->sgl) = bpdev->dma_addr;

	return table;
}

static void BiscuitOS_unmap_dma_buf(struct dma_buf_attachment *at,
			struct sg_table *sg, enum dma_data_direction dir)
{
	sg_free_table(sg);
	kfree(sg);
}

static void BiscuitOS_dmabuf_release(struct dma_buf *buf) { }

static int BiscuitOS_dmabuf_mmap(struct dma_buf *dmabuf,
					struct vm_area_struct *vma)
{
	struct Broiler_pci_device *bpdev = dmabuf->priv;

	return io_remap_pfn_range(vma, vma->vm_start,
			bpdev->dma_addr >> PAGE_SHIFT,
			vma->vm_end - vma->vm_start, vma->vm_page_prot);
}

static const struct dma_buf_ops BiscuitOS_exp_dmabuf_ops = {
	.map_dma_buf	= BiscuitOS_map_dma_buf, 
	.unmap_dma_buf	= BiscuitOS_unmap_dma_buf,
	.release	= BiscuitOS_dmabuf_release,
	.mmap		= BiscuitOS_dmabuf_mmap,
};

static int Broiler_pci_init(struct pci_dev *pdev, 
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

	r = misc_register(&BiscuitOS_drv);
	if (r < 0) {
		printk("Regisrter MISC Failed.\n");
		goto err_misc;
	}

	/* INIT QUEUE FOR EPOLL */
	init_waitqueue_head(&BiscuitOS_wait);

	return 0;

err_misc:
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

static void Broiler_pci_uninit(struct pci_dev *pdev)
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

static void Broiler_pci_remove(struct pci_dev *pdev)
{
	dma_free_coherent(&pdev->dev, DMA_BUFFER_LEN,
			bpdev->buffer, bpdev->dma_addr);
	Broiler_pci_uninit(pdev);
}

static int Broiler_pci_probe(struct pci_dev *pdev, 
				const struct pci_device_id *id)
{
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	int r;

	r = Broiler_pci_init(pdev, id);
	if (r < 0) {
		printk("System Error: PCI Init failed.\n");
		goto err;
	}

	/* DMA Remapping */
	bpdev->buffer = dma_alloc_coherent(&pdev->dev,
				DMA_BUFFER_LEN, &bpdev->dma_addr, GFP_KERNEL);
	if (!bpdev->buffer) {
		r = -ENOMEM;
		printk("%s ERROR: DMA Alloc and Remapping failed.\n", DEV_NAME);
		goto err_dma;
	}
	printk("Default DMA Physical Address %#llx", bpdev->dma_addr);

	/* DMA BUF */
	exp_info.ops = &BiscuitOS_exp_dmabuf_ops;
	exp_info.size = PAGE_SIZE;
	exp_info.flags = O_CLOEXEC;
	exp_info.priv = bpdev;

	bpdev->dmabuf = dma_buf_export(&exp_info);
	if (!bpdev->dmabuf) {
		r = -ENOMEM;
		printk("System Error: DMABUF export failed\n");
		goto err_dmabuf;
	}

	printk("%s  is Ready\n", DEV_NAME);
	return 0;

err_dmabuf:
	dma_free_coherent(&pdev->dev, DMA_BUFFER_LEN,
			bpdev->buffer, bpdev->dma_addr);
err_dma:
	Broiler_pci_remove(pdev);
err:
	return r;
}

static const struct pci_device_id Broiler_pci_ids[] = {
	{ PCI_DEVICE(0x1018, 0x1991), },
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
MODULE_DESCRIPTION("DMA-BUF Export: Video Capture");
