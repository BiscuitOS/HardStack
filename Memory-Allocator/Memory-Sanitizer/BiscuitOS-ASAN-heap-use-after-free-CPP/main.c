/*
 * ASAN: heap-use-after-free on C++
 *
 * (C) 2022.02.28 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <iostream>

using namespace std;

int main()
{
	/* alloc from heap */
	int *array = new int[100];

	/* free heap memory */
	delete [] array;

	return array[10]; /* BOOM */
}
