/*
 * kasprintf
 *
 * (C) 2020.02.06 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "linux/buddy.h"

/* Simplified asprintf */
char *kvasprintf(gfp_t gfp, const char *fmt, va_list ap)
{
	unsigned int first, second;
	char *p;
	va_list aq;

	va_copy(aq, ap);
	p = va_arg(aq, char *);
	va_end(aq);
	printk("SS %s\n", p);
}

char *kasprintf(gfp_t gfp, const char *fmt, ...)
{
	va_list ap;
	char *p;

	va_start(ap, fmt);
	p = kvasprintf(gfp, fmt, ap);
	va_end(ap);

	return p;
}
