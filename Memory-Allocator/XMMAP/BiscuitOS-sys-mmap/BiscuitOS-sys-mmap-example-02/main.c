/*
 * NULL: 0 virtual address
 *
 * (C) 2021.05.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>

int main()
{
	int *p = NULL;

	*p = 88520;

	return 0;
}
