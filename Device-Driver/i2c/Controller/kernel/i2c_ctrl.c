/*
 * I2C Controller on rpi 4B
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
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#include <asm/io.h>

/* BCM2835 I2C Controller 
 *
 *   The Broadcom Serial Controller (BSC) controller is a master, fast-mode
 *   (400Kb/s) BSC controller. The Broadcom Serial Control bus is a 
 *   proprietary bus compliant with the Philips I2C bus/interface version 2.1
 *   January 2000.
 *
 *   * I2C single master only operation (support clock stretching wait states)
 *   * Both 7-bit and 10-bit addressing is supported.
 *   * Timing completely software controller via registers.
 *
 * Register View
 *
 *   The BSC controller has eight memory-mapped registers. All accesses are
 *   assumed to be 32bit. Note that the BSC2 master is used dedicated with 
 *   the HDMI interface and should not be accessed by user programs.
 *
 *   The are three BSC master inside BCM. The register addresses start from:
 *
 *   * BSC0: 0x7E205000
 *   * BSC1: 0x7E804000
 *   * BSC2: 0x7E805000
 *
 *   The table below shows the addrss of I2C interface where the address is
 *   an offset from one of the three base address listed above.
 *
 *   +----------------------------------------------------------------------+
 *   |                          I2C Address Map                             |
 *   +----------------+---------------+------------------------------+------+
 *   | Address Offset | Register Name | Description                  | Size |
 *   +----------------+---------------+------------------------------+------+
 *   | 0x00           | C             | Control                      | 32   |
 *   +----------------+---------------+------------------------------+------+
 *   | 0x04           | S             | Status                       | 32   |
 *   +----------------+---------------+------------------------------+------+
 *   | 0x08           | DLEN          | Data Length                  | 32   |
 *   +----------------+---------------+------------------------------+------+
 *   | 0x0C           | A             | Slave Address                | 32   |
 *   +----------------+---------------+------------------------------+------+
 *   | 0x10           | FIFO          | Data FIFO                    | 32   |
 *   +----------------+---------------+------------------------------+------+
 *   | 0x14           | DIV           | Clock Divider                | 32   |
 *   +----------------+---------------+------------------------------+------+
 *   | 0x18           | DEL           | Data Delay                   | 32   |
 *   +----------------+---------------+------------------------------+------+
 *   | 0x1C           | CLKT          | Clock Stretch Timeout        | 32   |
 *   +----------------+---------------+------------------------------+------+
 */

/* DDL Platform Name */
#define DEV_NAME "I2C_ctrl"

#define I2C_REG_CTRL		0x0

struct i2c_dev {
	void __iomem *regs;
};

/* Controller Register
 *
 *   The control register is used to enable interrupts, clear the FIFO,
 *   define a read or write operation and start a transfer. 
 */
static int i2c_control_register(struct i2c_dev *i2c_dev)
{
	u32 val;

	val = readl(i2c_dev->regs + I2C_REG_CTRL);

	/* READ-bit: BIT[0]
	 *
	 *   The READ field specifies the type of transfer.
	 *   ---> 0: Write Packet Transfer.
	 *   ---> 1: Read Packet Transfer.
	 */
	if (val & 0x1)
		printk(KERN_INFO "Read Packet Transfer.\n");
	else
		printk(KERN_INFO "Write Packet Transfer.\n");

	/* Reserved: BIT[3:1]
	 *
	 *   Write as 0, read as don't care
	 */

	/* CLEAR-bit: BIT[5:4]
	 *
	 *   The CLEAR field is used to clear the FIFO. Writing to this
	 *   field is a one-shot operation which will always read back as
	 *   zero. The CLEAR bit can set at the same time as the start
	 *   transfer bit,
	 */
	
	
	return 0;
}

/* Probe: (DDL) Initialize Device */
static int I2C_ctrl_probe(struct platform_device *pdev)
{
	struct i2c_dev *i2c_dev;
	struct resource *mem;
	int ret;

	i2c_dev = kzalloc(sizeof(*i2c_dev), GFP_KERNEL);
	if (!i2c_dev) {
		printk(KERN_ERR "No free memory!\n");
		ret = -ENOMEM;
		goto err_alloc;
	}
	/* Setup platform driver data */
	platform_set_drvdata(pdev, i2c_dev);

	/* Remap memory */
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	i2c_dev->regs = devm_ioremap_resource(&pdev->dev, mem);
	if (IS_ERR(i2c_dev->regs)) {
		ret = PTR_ERR(i2c_dev->regs);
		goto err_io;
	}

	printk("Region Range: %#lx - %#lx\n", (unsigned long)mem->start, 
						(unsigned long)mem->end);
	/* Contrl Register */
	i2c_control_register(i2c_dev);
	
	
	return 0;

err_io:
	kfree(i2c_dev);
err_alloc:

	return ret;
}

/* Remove: (DDL) Remove Device (Module) */
static int I2C_ctrl_remove(struct platform_device *pdev)
{
	struct i2c_dev *i2c_dev = platform_get_drvdata(pdev);

	kfree(i2c_dev);

	return 0;
}

static const struct of_device_id I2C_ctrl_of_match[] = {
	{ .compatible = "BiscuitOS,I2C_ctrl", },
	{}
};

static struct platform_driver I2C_ctrl_driver = {
	.driver = {
		.name = DEV_NAME,
		.of_match_table = I2C_ctrl_of_match,
	},
	.probe = I2C_ctrl_probe,
	.remove = I2C_ctrl_remove,
};

static int I2C_ctrl_init(void)
{
	return platform_driver_register(&I2C_ctrl_driver);
}

static void I2C_ctrl_exit(void)
{
	platform_driver_unregister(&I2C_ctrl_driver);
}

module_init(I2C_ctrl_init);
module_exit(I2C_ctrl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("Platform Device Driver without DTS");
