/*
 * IDA Manual.
 *
 * (C) 2019.06.03 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>

/* IDR/IDA */
#include <ida.h>

/* Root of IDA */
static DEFINE_IDA(BiscuitOS_ida);

int main()
{
	unsigned long i;
	int id;

	for (i = 0; i < 1000; i++) {
		/* Allocate an unused ID */
		id = ida_alloc(&BiscuitOS_ida, GFP_KERNEL);
		printf("IDA-ID: %d\n", id);
	}


	/* Release an allocated ID */
	ida_free(&BiscuitOS_ida, id);

	return 0;
}
