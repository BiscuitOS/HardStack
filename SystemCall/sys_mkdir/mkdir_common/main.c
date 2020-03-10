/*
 * sys_mkdir in C
 *
 * (C) 2020.03.10 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

int main()
{
	/* mkdir on current dirent */
	mkdir("BiscuitOS_bus", 0777);

	return 0;
}
