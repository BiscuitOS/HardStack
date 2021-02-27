/*
 * BiscuitOS KVM on 2M HugePage
 *
 * (C) 2020.10.24 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
/* KVM Header */
#include <linux/kvm.h>

int main()
{

	printf("Hello BiscuitOS.\n");

	return 0;
}
