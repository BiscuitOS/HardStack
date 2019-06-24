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
	unsigned long bitmap[2] = {0};
	u64 map = 0x123456789abcdef;
	
	/* Cover u64 to bitmap */
	bitmap_from_u64(bitmap, map);
	printf("%#llx cover to [0]%#lx [1]%#lx\n", map, bitmap[0], bitmap[1]);

	return 0;
}
