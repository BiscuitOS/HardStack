/*
 * Copyright (C) 2017 buddy.zhang@aliyun.com
 *
 * Misc device driver demo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

#define DEV_NAME "misc_demo"

/*
 * open operation
 */
static int misc_demo_open(struct inode *inode,struct file *filp)
{
    printk(KERN_INFO "Open device\n");
    return 0;
}
/*
 * release opertion 
 */
static int misc_demo_release(struct inode *inode,struct file *filp)
{
    printk(KERN_INFO "Close device\n");
    return 0;
}
/*
 * read operation
 */
static ssize_t misc_demo_read(struct file *filp,char __user *buffer,size_t count,
		loff_t *offset)
{
    printk(KERN_INFO "read device\n");
    return 0;
}
/*
 * write operation
 */
static ssize_t misc_demo_write(struct file *filp,const char __user *buf,
		size_t count,loff_t *offset)
{
    printk(KERN_INFO "Write device\n");
    return 0;
}
/*
 * file_operations
 */
static struct file_operations misc_demo_fops = {
    .owner     = THIS_MODULE,
    .open      = misc_demo_open,
    .release   = misc_demo_release,
    .write     = misc_demo_write,
    .read      = misc_demo_read,
};
/*
 * misc struct 
 */

static struct miscdevice misc_demo_misc = {
    .minor    = MISC_DYNAMIC_MINOR,
    .name     = DEV_NAME,
    .fops     = &misc_demo_fops,
};
/*
 * Init module
 */
static __init int misc_demo_init(void)
{
    misc_register(&misc_demo_misc);
    printk("misc demo initialize.\n");
    return 0;
}
/*
 * Exit module
 */
static __exit void misc_demo_exit(void)
{
    misc_deregister(&misc_demo_misc);
}
/*
 * module information
 */
module_init(misc_demo_init);
module_exit(misc_demo_exit);

MODULE_LICENSE("GPL");
