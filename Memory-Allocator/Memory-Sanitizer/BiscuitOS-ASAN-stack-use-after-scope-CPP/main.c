/*
 * ASAN: stack-use-after-scope on C++
 *
 * (C) 2022.02.28 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <iostream>

using namespace std;

int *ptr;

void func(void)
{
	int array[10];

	ptr = &array[8];
}

int main()
{
	func();

	return *ptr;
}
