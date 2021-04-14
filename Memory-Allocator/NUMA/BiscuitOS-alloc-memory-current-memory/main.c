/*
 * NUMA: Allocate memory on current NODE.
 *
 * (C) 2021.04.01 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
/* NUMA header with -lnuma */
#include <numa.h>
#include <numaif.h>

/* Memory size */
#define BISCUITOS_AREA_SIZE	1000

int main()
{
	int nid, rc;
	char *man;

	/*
	 * Allocates size bytes of memory with the current NUMA policy.
	 * The size argument with be rounded up to a multiple of the 
	 * system page size. This function is relatively slow compare
	 * to the malloc() family of functions. The memory must be freed
	 * with numa_free(). On errors NULL is retured.
	 */
	man = numa_alloc(BISCUITOS_AREA_SIZE);
	*man = 1;

	rc = get_mempolicy(&nid, NULL, 0, man, MPOL_F_NODE | MPOL_F_ADDR);
	if (rc < 0)
		perror("get_mempolicy() function.");
	else
		printf("Current NUMA NODE %d\n", nid);

	printf("%#lx => %#hhx\n", (unsigned long)man, *man);

	/*
	 * Frees size bytes of memory starting at start, allocated by the
	 * numa_alloc_* functions above. The size argument will be rounded
	 * up to a multiple of the system page size.
	 */
	numa_free(man, BISCUITOS_AREA_SIZE);

	return 0;
}
