/*
 * Usage for anonymous interval tree on BiscuitOS
 *
 * (C) 2022.02.09 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/rmap.h>
#include "anon_interval_tree.h"

#define VADDR		0x600000000000
#define PAGE_NR		4

/* Module initialize entry */
static int __init BiscuitOS_init(void)
{
	struct anon_vma *anon_vma, *tanon_vma;
	struct anon_vma_chain *avc, *tavc;
	struct page *pages[PAGE_NR], *page;
	struct vm_area_struct *vma;
	unsigned long start, end;
	int ret = 0, i;

	/*
	 * VMA contain 4 memory regions and mapping 4 physical page
	 *
	 *   1st region: 0x600000000000 - 0x600000001000 --> 1st page
	 *   2nd region: 0x600000001000 - 0x600000002000 --> 2nd page
	 *   3rd region: 0x600000002000 - 0x600000003000 --> 3rd page
	 *   4th region: 0x600000003000 - 0x600000004000 --> 4th page
	 */
	vma = kmalloc(sizeof(*vma), GFP_KERNEL);
	if (!vma) {
		printk("VMA create failed.\n");
		ret = -ENOMEM;
		goto err;
	}
	vma->vm_pgoff = VADDR >> PAGE_SHIFT;
	vma->vm_start = VADDR;
	vma->vm_end   = VADDR + PAGE_NR * PAGE_SIZE;
	INIT_LIST_HEAD(&vma->anon_vma_chain);

	/* anon_vma_chain for vma */
	avc = kmalloc(sizeof(*avc), GFP_KERNEL);
	if (!avc) {
		printk("AVC create failed.\n");
		ret = -ENOMEM;
		goto err_avc;
	}
	INIT_LIST_HEAD(&avc->same_vma);

	/* Root anon_vma */
	anon_vma = kzalloc(sizeof(*anon_vma), GFP_KERNEL);
	if (!anon_vma) {
		printk("AV create failed.\n");
		ret = -ENOMEM;
		goto err_av;
	}
	atomic_set(&anon_vma->refcount, 1);
	anon_vma->degree = 1;
	anon_vma->parent = anon_vma;
	anon_vma->root   = anon_vma;

	/* Link VMA/AV/AVC */
	vma->anon_vma = anon_vma;
	avc->vma = vma;
	avc->anon_vma = anon_vma;
	list_add(&avc->same_vma, &vma->anon_vma_chain);
	BiscuitOS_anon_vma_interval_tree_insert(avc, &anon_vma->rb_root);
	anon_vma->degree++;

	/* Emulate mapping pysical page */
	for (i = 0; i < PAGE_NR; i++) {
		unsigned long vaddr = i * PAGE_SIZE + VADDR;

		/* Assume alloc page success */
		pages[i] = alloc_page(GFP_KERNEL);
		/* rmap */
		atomic_set(&pages[i]->_mapcount, 0);
		anon_vma = (void *) anon_vma + PAGE_MAPPING_ANON;
		pages[i]->mapping = (struct address_space *) anon_vma;
		pages[i]->index =
			((vaddr - vma->vm_start) >> PAGE_SHIFT) + vma->vm_pgoff;
		printk("Page %#lx index %#lx\n", 
				page_to_pfn(pages[i]), pages[i]->index);
	}

	/* traversal interval tree  */
	page = pages[2];
	tanon_vma = (struct anon_vma *)((unsigned long)page->mapping &
							~PAGE_MAPPING_FLAGS);
	start = page->index;
	end   = start + 1;
	BiscuitOS_anon_vma_interval_tree_foreach(tavc, 
					&tanon_vma->rb_root, start, end) {
		struct vm_area_struct *tvma = tavc->vma;
		unsigned long address =
			((page->index - tvma->vm_pgoff) << PAGE_SHIFT) + 
								tvma->vm_start;

		printk("Page %#lx with VA: %#lx and VMA %#lx - %#lx\n",
					page_to_pfn(page), address,
					tvma->vm_start, tvma->vm_end);
	}

	/* remove */
	list_for_each_entry(tavc, &vma->anon_vma_chain, same_vma)
		BiscuitOS_anon_vma_interval_tree_remove(tavc, 
					&tavc->anon_vma->rb_root);

	for (i = 0; i < PAGE_NR; i++)
		__free_page(pages[i]);

	kfree(anon_vma);
err_av:
	kfree(avc);
err_avc:
	kfree(vma);
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
MODULE_DESCRIPTION("Anonymous interval tree on BiscuitOS");
