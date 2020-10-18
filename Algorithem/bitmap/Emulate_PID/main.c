/*
 * Bitmap: Emulate PID allocating and Releasing on BiscuitOS
 *
 * (C) 2020.10.11  Meijusan <20741602@qq.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include<linux/slab.h>
#include <linux/err.h>
#include <linux/file.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/atomic.h>
#include <linux/mm.h>
#include <linux/export.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/spinlock.h>
#include <linux/rcupdate.h>
#include <linux/mutex.h>
#include <linux/gfp.h>
#include <linux/pid.h>
#include <linux/delay.h>
#include <linux/random.h>

#define PID_BITMAP_MAX 4096
/*
 * pid range 0~4096,this is a example
 */

static DECLARE_BITMAP( pid_bitmap_mask, PID_BITMAP_MAX);
static DEFINE_MUTEX(pid_bitmap_lock);

int pid_produce_thread(void *arg)
{
	while (!kthread_should_stop())  {
		int pid;
		
		mutex_lock(&pid_bitmap_lock);
		if( bitmap_full(pid_bitmap_mask, PID_BITMAP_MAX) ) {
			mutex_unlock(&pid_bitmap_lock);
			msleep(1000);
			continue;
		}
		
		pid = find_first_zero_bit(pid_bitmap_mask, PID_BITMAP_MAX);
		if ( pid >= PID_BITMAP_MAX) {
			printk("bitmap is full\n");
			mutex_unlock(&pid_bitmap_lock);
			msleep(1000);
			continue;
		}
		printk("threadid %d create  pid: %d \n",current->pid,  pid);	
		set_bit(pid, pid_bitmap_mask);		
		mutex_unlock(&pid_bitmap_lock);
	}
	return 0;
}

static int pid_consume_thread(void *dummy)
{
	while (!kthread_should_stop()) {
		
		/*range 0 ~PID_BITMAP_MAX*/
		int pid = prandom_u32()%(PID_BITMAP_MAX + 1) ;	
		
		mutex_lock(&pid_bitmap_lock);
		clear_bit(pid, pid_bitmap_mask);		
		mutex_unlock(&pid_bitmap_lock);
		printk("threadid %d remove  pid: %d \n",current->pid,  pid);		
		msleep(1000);		
	}		
	return 0;
}

static struct task_struct *pid_produce_task = NULL;
static struct task_struct *pid_consume_task = NULL;


/* Module initialize entry */
static int __init pid_bitmap_demo_init(void)
{
	/* default all pid already exist */
	bitmap_fill(pid_bitmap_mask, PID_BITMAP_MAX);
	
	pid_produce_task = kthread_run(pid_produce_thread, NULL, "pidconsujme");
	if (IS_ERR(pid_produce_task)) {
		int err = PTR_ERR(pid_produce_task);
		printk("failed to start the kauditd thread (%d)\n", err);
	}
	
	pid_consume_task = kthread_run(pid_consume_thread, NULL, "testbitmap");
	if (IS_ERR(pid_consume_task)) {
		int err = PTR_ERR(pid_consume_task);
		printk("failed to start the kauditd thread (%d)\n", err);
	}
	
	return 0;
}

/* Module exit entry */
static void __exit pid_bitmap_demo_exit(void)
{
	if(pid_produce_task)
		kthread_stop(pid_produce_task);	
	if(pid_consume_task)
		kthread_stop(pid_consume_task);	
}

module_init(pid_bitmap_demo_init);
module_exit(pid_bitmap_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Meijusan <20741602@qq.com>");
MODULE_DESCRIPTION("Emulate PID allocating and Releasing on BiscuitOS");
