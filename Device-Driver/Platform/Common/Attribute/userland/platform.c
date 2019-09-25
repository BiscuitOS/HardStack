/*
 * Platform Device
 *
 * (C) 2019.09.24 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define PLATFORM_PATH		"/sys/bus/platform/devices/Platform_attr.1"
#define HEXADECIMAL_PATH	PLATFORM_PATH"/Hexadecimal"
#define INTEGER_PATH		PLATFORM_PATH"/Integer"
#define STRING_PATH		PLATFORM_PATH"/String"

int main()
{
	unsigned long Hexadecimal = 0x88;
	unsigned long Integer = 1234;
	char String[128];
	char buf[128];
	int nr_read;
	int fd;

	/* Hexadecimal
	 *   If you want to exchange an Hexadecimal between kernel 
	 *   and userspace, follow steps:
	 *
	 *   1. open path
	 *      --> fd = open(path, O_RDWR)
	 *   2. transfer hexadecimal to buffer
	 *      --> sprintf(buf, "%lx", Hexadecimal)
	 *   3. write buffer
	 *      --> write(fd, buf, sizeof(Hexadecimal))
	 *   4. read buffer
	 *      --> read(fd, buf, sizeof(Hexadecimal))
	 *   5. format Hexadeciamal
	 *      --> sscanf(buf, "%lx", &Hexadecimal)
	 *   6. close path
	 *      --> close(fd) 
	 */
	
	/* open */
	fd = open(HEXADECIMAL_PATH, O_RDWR);
	if (fd < 0) {
		printf("%s can't open.\n", HEXADECIMAL_PATH);
	}

	/* Write */
	sprintf(buf, "%lx", Hexadecimal);
	write(fd, buf, sizeof(Hexadecimal));
	Hexadecimal = 0;
	/* Read */
	read(fd, buf, sizeof(Hexadecimal));
	sscanf(buf, "%lx", &Hexadecimal);
	printf("Hexadecimal: %#lx\n", Hexadecimal);
	/* close */
	close(fd);

	/* Integer
	 *   If you want to exchange an Integer between kernel 
	 *   and userspace, follow steps:
	 *
	 *   1. open path
	 *      --> fd = open(path, O_RDWR)
	 *   2. transfer Integer to buffer
	 *      --> sprintf(buf, "%lx", Integer)
	 *   3. write buffer
	 *      --> write(fd, buf, sizeof(Integer))
	 *   4. read buffer
	 *      --> read(fd, buf, sizeof(Integer))
	 *   5. format Integer
	 *      --> sscanf(buf, "%lx", &Integer)
	 *   6. close path
	 *      --> close(fd) 
	 */
	
	/* open */
	fd = open(INTEGER_PATH, O_RDWR);
	if (fd < 0) {
		printf("%s can't open.\n", INTEGER_PATH);
	}

	/* Write */
	sprintf(buf, "%ld", Integer);
	write(fd, buf, sizeof(Integer));
	Integer = 0;
	/* Read */
	read(fd, buf, sizeof(Integer));
	sscanf(buf, "%ld", &Integer);
	printf("Integer:     %#ld\n", Integer);
	/* close */
	close(fd);

	/* String
	 *   If you want to exchange an String between kernel 
	 *   and userspace, follow steps:
	 *
	 *   1. open path
	 *      --> fd = open(path, O_RDWR)
	 *   2. transfer String to buffer
	 *      --> sprintf(buf, "%s", String)
	 *   3. write buffer
	 *      --> write(fd, buf, strlen(String))
	 *   4. read buffer
	 *      --> read(fd, buf, strlen(String))
	 *   5. format Integer
	 *      --> sscanf(buf, "%s", String)
	 *   6. close path
	 *      --> close(fd) 
	 */
	
	/* open */
	fd = open(STRING_PATH, O_RDWR);
	if (fd < 0) {
		printf("%s can't open.\n", STRING_PATH);
	}

	strcpy(String, "Hello-BiscuitOS\n");
	nr_read = strlen(String);
	sprintf(buf, "%s", String);
	/* Write */
	write(fd, buf, nr_read);
	memset(String, 0, nr_read);
	/* Read */
	read(fd, buf, nr_read);
	sscanf(buf, "%s", String);
	printf("String:      %s\n", String);
	/* close */
	close(fd);

	return 0;
}
