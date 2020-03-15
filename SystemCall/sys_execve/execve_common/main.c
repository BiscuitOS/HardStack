/*
 * sys_execve in C
 *
 * (C) 2020.03.11 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
/* __NR_execve */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_execve
#define __NR_execve	11
#endif

int main(void)
{
	const char *argv[] = { "ls", "-al", "/mnt", NULL};
	const char *envp[] = { 0, NULL};
	const char *filename = "/bin/ls";

	/*
	 * sys_execve
	 *
	 *    SYSCALL_DEFINE3(execve,
	 *                    const char __user *, filename,
	 *                    const char __user *const __user *, argv,
	 *                    const char __user *const __user *, envp) 
	 */
	syscall(__NR_execve, filename, argv, envp);

	return 0;
}
