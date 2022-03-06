/*
 * ASAN: Stack use after scope
 *
 * (C) 2022.02.28 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int *ptr;

void func(void)
{
	int array[10];

	ptr = &array[6];
}

int main()
{
	/* BOOM: scope */
	func();	

	return *ptr;
}
