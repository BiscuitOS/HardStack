/*
 * Xarray.
 *
 * (C) 2019.06.06 <buddy.zhang@aliyun.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#include <stdio.h>
#include <stdlib.h>

/* Header of XArray */
#include <xarray.h>

/* Declare and implement XArray */
static DEFINE_XARRAY(BiscuitOS_xa);

int main()
{
	printf("Hello BiscuitOS.\n");

	return 0;
}
