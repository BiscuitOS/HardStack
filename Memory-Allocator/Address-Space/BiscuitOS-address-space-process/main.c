/*
 * Process Address Space
 *
 * (C) 2021.05.01 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main()
{
	char *cmd[] = { "ls", "-l", NULL};

	execv("/bin/ls", cmd);

	return 0;
}
