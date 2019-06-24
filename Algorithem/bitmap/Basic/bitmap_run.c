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
	unsigned long bitmap = 0x123;
	unsigned long pos;	

	for_each_clear_bit(pos, &bitmap, 32)
		printf("bitmap: %d\n", pos);

	return 0;
}
