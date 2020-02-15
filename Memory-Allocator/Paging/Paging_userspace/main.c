/*
 * Page Table
 *
 * (C) 2020.02.14 (Valentine's Day) BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "linux/biscuitos.h"
#include "linux/memblock.h"

int main()
{
	memory_init();

	page_table_init();
	
	dup_pgdir();

	memory_exit();

	return 0;
}
