/*
 * Bitmap Manual.
 *
 * (C) 2019.06.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>

/* bitmap header */
#include <bitmap.h>

int main()
{
	unsigned long bitmap = 0x00100000;

	printf("Last bit: %#lx\n", __builtin_clzl(bitmap));	

	return 0;
}
