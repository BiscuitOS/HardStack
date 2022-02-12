/*
 * Usage for file interval tree on BiscuitOS
 *
 * (C) 2021.02.11 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include "file_interval_tree.h"

#define VADDR			0x600000000000
#define PAGE_NR			4

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	struct address_space *mapping, *tmapping;
	struct page *pages[PAGE_NR], *page;
	struct vm_area_struct *vma, *tvma;
	unsigned long start, end;
	struct inode *inode;
	struct file *file;
	int ret = 0, i;

	/* pseudo inode and file */
	inode = kzalloc(sizeof(*inode), GFP_KERNEL);
	if (!inode) {
		printk("inode create failed.\n");
		ret = -ENOMEM;
		goto err;
	}
	inode->i_mapping = &inode->i_data;

	file = kzalloc(sizeof(*file), GFP_KERNEL);
	if (!file) {
		printk("File create failed.\n");
		ret = -ENOMEM;
		goto err_file;
	}
	file->f_inode   = inode;
	file->f_mapping = inode->i_mapping;

	/*
	 * VMA contain 4 memory regions and mapping 4 physical page
	 *
	 *   1st region: 0x600000000000 - 0x600000001000 --> 1st page
	 *   2nd region: 0x600000001000 - 0x600000002000 --> 2nd page
	 *   3rd region: 0x600000002000 - 0x600000003000 --> 3rd page
	 *   4th region: 0x600000003000 - 0x600000004000 --> 4th page
	 */
	vma = kzalloc(sizeof(*vma), GFP_KERNEL);
	if (!vma) {
		printk("VMA create failed.\n");
		ret = -ENOMEM;
		goto err_vma;
	}
	/* mapping from head of file */
	vma->vm_pgoff = 0;
	vma->vm_start = VADDR;
	vma->vm_end   = VADDR + PAGE_NR * PAGE_SIZE;
	vma->vm_file  = get_file(file);

	/* insert into interval tree */
	mapping = file->f_mapping;
	BiscuitOS_vma_interval_tree_insert(vma, &mapping->i_mmap);

	/* Emulate mapping physical page */
	for (i = 0; i < PAGE_NR; i++) {
		unsigned long vaddr = i * PAGE_SIZE + VADDR;
		unsigned long index =
			((vaddr - vma->vm_start) >> PAGE_SHIFT) + vma->vm_pgoff;
		XA_STATE_ORDER(xas, &mapping->i_pages, index, 0);

		/* Assume alloc page success */
		pages[i] = alloc_page(GFP_KERNEL);
		/* rmap */
		pages[i]->mapping = mapping;
		pages[i]->index   = index;
		xas_store(&xas, pages[i]);
		printk("Page %#lx index %#lx\n",
				page_to_pfn(pages[i]), pages[i]->index);
	}

	/* Traversal interal tree */
	page  = pages[2];
	tmapping = (void *)((unsigned long)page->mapping & ~PAGE_MAPPING_FLAGS);

	start = page->index;
	end   = start + 1;
	BiscuitOS_vma_interval_tree_foreach(tvma, 
					&tmapping->i_mmap, start, end) {
		unsigned long address =
			((page->index - tvma->vm_pgoff) << PAGE_SHIFT) +
							tvma->vm_start;

		printk("Page %#lx with VA: %#lx and VMA %#lx - %#lx\n",
				page_to_pfn(page), address,
				tvma->vm_start, tvma->vm_end);
	}

	/* remove */
	BiscuitOS_vma_interval_tree_remove(vma, &mapping->i_mmap);

	for (i = 0; i < PAGE_NR; i++)
		__free_page(pages[i]);

	kfree(vma);
err_vma:
	kfree(file);
err_file:
	kfree(inode);
err:
	return ret;
}

/* Module exit entry */
static void __exit BiscuitOS_exit(void)
{
}

module_init(BiscuitOS_init);
module_exit(BiscuitOS_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BiscuitOS <buddy.zhang@aliyun.com>");
MODULE_DESCRIPTION("File interval tree on BiscuitOS");
