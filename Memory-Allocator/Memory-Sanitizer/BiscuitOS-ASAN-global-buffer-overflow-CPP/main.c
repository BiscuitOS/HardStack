/*
 * ASAN: Global-buffer-overflow on C++
 *
 * (C) 2022.02.28 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <iostream>

using namespace std;

int array[10] = { 0 };

int main()
{
	/* BOOM: Global buffer overflow */
	array[10] = 0x88;
	return 0;
}
