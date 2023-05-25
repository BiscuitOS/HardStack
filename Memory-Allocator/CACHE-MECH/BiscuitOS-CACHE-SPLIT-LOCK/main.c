// SPDX-License-Identifier: GPL-2.0
/*
 * Split lock
 *
 * (C) 2023.05.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#define SMP_CACHE_BYTES		64

#pragma pack(push, 2)
struct BiscuitOS_node
{
	char node[SMP_CACHE_BYTES -2];
	long data;
};
#pragma pack(pop)

int main()
{
	struct BiscuitOS_node *np;

	np = (struct BiscuitOS_node *)mmap(NULL, sizeof(*np),
					PROT_READ | PROT_WRITE,
					MAP_PRIVATE | MAP_ANONYMOUS,
					-1,
					0);
	if (np == MAP_FAILED) {
		printf("Error: NoFree Memory.\n");
		exit (-1);
	}

	printf("NP:\n node: %#lx\n data: %#lx\n", (unsigned long)np->node,
					(unsigned long)&np->data);

	while (1)
		__sync_fetch_and_add(&np->data, 1);

	munmap(np, sizeof(*np));
	return 0;
}
