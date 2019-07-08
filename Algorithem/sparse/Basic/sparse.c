/*
 * Sparse.
 *
 * (C) 2019.07.01 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>

/*  little-endian/big-endian */
#include <arpa/inet.h>

#define __force		__attribute__((force))
#define __bitwise	__attribute__((bitwise))
/* bitwise: big-endian, little-endian */
typedef unsigned int __bitwise bs_t;

int main()
{
	bs_t a = (__force bs_t)0x12345678;
	bs_t b;
	unsigned int c = 100;

	/* Cove same type */
	b = a;

	/* Cove different type */
	b = c;

	/* Force different type */
	b = (__force bs_t)c;

	return 0;
}
