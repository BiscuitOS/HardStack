/*
 * C++ Language on BiscuitOS
 *
 * (C) 2021.09.09 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <iostream>

using namespace std;

class BiscuitOS {
private:
	int number;
	int byte;
public:
	BiscuitOS(int n, int b);
	~BiscuitOS();
	int getnumber();
	void setnumber(int n);
	int getbyte();
};

BiscuitOS::BiscuitOS(int a, int b) : number(a), byte(b) {
	// TODO:
}

BiscuitOS::~BiscuitOS() {
	cout << "Hello Bye" << endl;
}

int BiscuitOS::getnumber() {
	cout << number << endl;
}

void BiscuitOS::setnumber(int n) {
	number = n;
}

int BiscuitOS::getbyte() {
	cout << byte << endl;
}

int main()
{
	BiscuitOS *bs = new BiscuitOS(20, 30);

	bs->setnumber(50);
	bs->getnumber();
	bs->getbyte();

	printf("Hello C++ Language on BiscuitOS.\n");

	delete bs;
	return 0;
}
