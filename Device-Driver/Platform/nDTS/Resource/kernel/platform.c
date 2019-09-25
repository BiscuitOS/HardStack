/*
 * Platform Bus (Normal without DTS)
 *
 * (C) 2019.10.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

/* DDL Platform Name */
#define DEV_NAME "Platform_Resource"

/* Platform Device data */
struct Platform_demo_data {
	unsigned long index;
};

/* Platform Device private data */
struct Platform_demo_pdata {
	struct device *dev;
	void __iomem *base;
	unsigned int irq;
};

/* IRQ handler */
static irqreturn_t Platform_demo_irqhandler(int irq, void *param)
{
	return IRQ_HANDLED;
}

/* Probe: (DDL) Initialize Device */
static int Platform_demo_probe(struct platform_device *pdev)
{
	struct Platform_demo_pdata *private_data;
	struct Platform_demo_data *device_data;
	struct resource *mem_res;
	struct resource *irq_res;
	void __iomem *mem_base;
	int rvl;

	/* Platform Pirvate data */
	device_data = pdev->dev.platform_data;
	if (!device_data) {
		dev_err(&pdev->dev, "Unable to get drvdata!");
		return -EINVAL;
	}
	printk("Device platform_data: %#lx\n", device_data->index);

	/* Private Platform: allocate memory */
	private_data = devm_kzalloc(&pdev->dev, sizeof(*private_data),
								GFP_KERNEL);
	if (!private_data) {
		dev_err(&pdev->dev, "Private Data no free memory.\n");
		rvl = -ENOMEM;
		goto pdata_mem;
	}

	/* Resource: Memory Region
	 *   Platform get memory region resource and remap to kernel
	 *   address space. The default route is:
	 *
	 *   1) get resource from platform device
	 *      -> platform_get_reousrce()
	 *   2) request memory region
	 *      -> devm_request_mem_region()
	 *   3) remap memory region
	 *      -> devm_ioremap()
	 *   4) store to private data
	 *
	 *   Note! start address and end address on memory region must
	 *   out of RAM address region. for example:
	 *
	 *       struct resource memory_res = {
	 *               .start = 0x20000000,
	 *               .end   = 0x20080000,
	 *               .flags  = IORESOURCE_MEM,
	 *       };
	 *
	 *   Then, The RAM range is from 0x60000000 to 0x70000000, so
	 *   The device memory region must out of range from 0x60000000
	 *   to 0x70000000.
	 */

	/* First memory region */
	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem_res) {
		dev_err(&pdev->dev, "IORESOURCE_MEM 0 unavailable");
		rvl = -ENODEV;
		goto mem_0;
	}
	printk("Resource: %s\n", mem_res->name);
	printk("Region: %#lx - %#lx\n", 
		   		(unsigned long)mem_res->start,
				(unsigned long)mem_res->end);
	/* Second memory region */
	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!mem_res) {
		dev_err(&pdev->dev, "IORESOURCE_MEM 1 unavailable");
		rvl = -ENODEV;
		goto mem_1;
	}
	printk("Resource: %s\n", mem_res->name);
	printk("Region: %#lx - %#lx\n", 
		   		(unsigned long)mem_res->start,
				(unsigned long)mem_res->end);
	/* Request Memory Region */
	if (!devm_request_mem_region(&pdev->dev, mem_res->start,
				resource_size(mem_res), mem_res->name)) {
		printk("Unmap Memory Region: %#lx - %#lx\n",
		   		(unsigned long)mem_res->start,
				(unsigned long)mem_res->end);
		rvl = -EBUSY;
		goto mem_req;
	}
	/* Remap Memory Region */
	mem_base = devm_ioremap(&pdev->dev, mem_res->start, 
						resource_size(mem_res));
	if (IS_ERR(mem_base)) {
		printk(KERN_ERR "Unable remap memory region: %#lx - %#lx\n",
		   		(unsigned long)mem_res->start,
				(unsigned long)mem_res->end);
		rvl = PTR_ERR(mem_base);
		goto mem_remap;
	}
	/* Store on private data */
	private_data->base = mem_base;
	/* Memory Region OK */
	*(unsigned long *)mem_base = 0x902989;
	/* Store to private data */
	printk("Vaddr: %#lx value: %#lx\n", (unsigned long)mem_base, 
					   *(unsigned long *)mem_base);

	/* Resource: IRQ
	 *   Get and request an IRQ from platform device, and 
	 *   register a IRQ into Interrupt core. The default route:
	 *
	 *   1) Get IRQ resource information from platform
	 *      -> platform_get_resource(pdev, IORESOURCE_IRQ, index)
	 *   2) Reqeust IRQ resource from platform bus, and register
	 *      IRQ into Interrupt core. and bind a interrupt handler.
	 *      -> devm_request_irq()
	 *
	 *   The IRQ number is 'start' on structure 'resource', such as:
	 *
	 *      struct resrouce irq_res {
	 *              .start = IRQ_num,
	 *              .end   = IRQ_num,
	 *              .flags = IORESOURCE_IRQ,
	 *      };
	 *
	 *   The value of 'start' is same as 'end'.
	 */

	/* First IRQ */
	irq_res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!irq_res) {
		dev_err(&pdev->dev, "IRQRESOURCE unavailable.\n");
		rvl = -ENODEV;
		goto irq_0;
	}
	/* Second IRQ */
	irq_res = platform_get_resource(pdev, IORESOURCE_IRQ, 1);
	if (!irq_res) {
		dev_err(&pdev->dev, "IRQRESOURCE unabailable.\n");
		rvl = -ENODEV;
		goto irq_1;
	}
	/* Reqeust IRQ resource */
	rvl = devm_request_irq(&pdev->dev, irq_res->start,
			Platform_demo_irqhandler, 0, "Platform", NULL);
	if (rvl) {
		dev_err(&pdev->dev, "Unable to request IRQ.\n");
		goto irq_req;
	}
	/* Store on private data */
	private_data->irq = irq_res->start;
	printk("Register IRQ: %d handler: Platform_demo_irqhandler()\n",
					irq_res->start);

	/* Bind private data into platform device */
	private_data->dev = &pdev->dev;
	platform_set_drvdata(pdev, private_data);
	
	return 0;

irq_req:
irq_1:
irq_0:
	devm_iounmap(&pdev->dev, mem_base);
mem_remap:
mem_req:
mem_1:
mem_0:
	devm_kfree(&pdev->dev, private_data);
pdata_mem:
	return rvl;
}

/* Remove: (DDL) Remove Device (Module) */
static int Platform_demo_remove(struct platform_device *pdev)
{
	struct Platform_demo_pdata *pdata = platform_get_drvdata(pdev);

	/* Remove IRQ */
	devm_free_irq(&pdev->dev, pdata->irq, NULL);

	/* Remove ioremap */
	devm_iounmap(&pdev->dev, pdata->base);

	/* Remove private data */
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, pdata);
	
	return 0;
}

/* Shutdown: (DDL) Power-off/Shutdown */
static void Platform_demo_shutdown(struct platform_device *pdev)
{
}

/* Suspend: (DDL) Suspend (schedule) Sleep */
static int Platform_demo_suspend(struct platform_device *pdev, 
							pm_message_t state)
{
	return 0;
}

/* Resume: (DDL) (schedule) From Suspend/Sleep */
static int Platform_demo_resume(struct platform_device *pdev)
{
	return 0;
}

/* Platform Driver Information */
static struct platform_driver Platform_demo_driver = {
	.probe    = Platform_demo_probe,
	.remove   = Platform_demo_remove,
	.shutdown = Platform_demo_shutdown,
	.suspend  = Platform_demo_suspend,
	.resume   = Platform_demo_resume,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DEV_NAME,
	},
};

/* Platform Resource */
static struct resource Platform_demo_resources[] = {
	[0] = { /* Memory Region 0 */
		.name	= "Platform Memory 0",
		.start	= 0x22002000,
		.end	= 0x22002008,
		.flags	= IORESOURCE_MEM,
	},
	[1] = { /* Memory Region 1 */
		.name	= "Platform Memory 1",
		.start	= 0x24003000,
		.end	= 0x24003000,
		.flags	= IORESOURCE_MEM,
	},
	[2] = { /* IRQ 0 */
		.name	= "Platform IRQ 0",
		.start	= 12,
		.end	= 12,
		.flags	= IORESOURCE_IRQ,
	},
	[3] = { /* IRQ 1 */
		.name	= "Platform IRQ 1",
		.start	= 38,
		.end	= 38,
		.flags	= IORESOURCE_IRQ,
	},
};

/* Platform data release */
static void Platform_demo_data_release(struct device *dev)
{
	dev->parent = NULL;
}

/* Platform Device Private data */
static struct Platform_demo_data Platform_demo_pdata = {
	.index = 0x10,
};

/* Platform Device Information */
static struct platform_device Platform_demo_device = {
	.id = -1,
	.name = DEV_NAME,
	.dev = {
		.platform_data = &Platform_demo_pdata,
		.release       =  Platform_demo_data_release,
	},
	.resource              = Platform_demo_resources,
	.num_resources         = ARRAY_SIZE(Platform_demo_resources),
};

/* Module initialize entry */
static int __init Platform_demo_init(void)
{
	int ret;

	/* Register platform driver */
	ret = platform_driver_register(&Platform_demo_driver);
	if (ret) {
		printk("Unable register Platform driver.\n");
		return -EBUSY;
	}

	/* Register platform device */
	ret = platform_device_register(&Platform_demo_device);
	if (ret) {
		printk("Unable register Platform device.\n");
		return -EBUSY;
	}

	return 0;
}

/* Module exit entry */
static void __exit Platform_demo_exit(void)
{
	platform_device_unregister(&Platform_demo_device);
	platform_driver_unregister(&Platform_demo_driver);
}

module_init(Platform_demo_init);
module_exit(Platform_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Platform Resource without DTS");
