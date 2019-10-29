/*
 * Chest base interface
 *
 * (C) 2019.10.28 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "command.h"
#include "chest.h"
#include "at24c08_lib.h"

int fd_at24c08;

/* Probe procedure */
void chest_probe(void)
{
	int ret;

	/* open at24c08 eeprom */
	fd_at24c08 = open(I2C_BUS, O_RDWR);
	if (fd_at24c08 < 0) {
		printf("Unable to open %s\n", I2C_BUS);
		exit (-1);
	}
}

/* Remove procedure */
void chest_remove(void)
{
	close(fd_at24c08);
}

/* Interger read */
static int64_t chest_integer_read(struct chest_cmd_struct *cmd, char *argv[])
{
	unsigned char *buf;
	int idx;

	buf = malloc(cmd->len);
	memset(buf, 0, cmd->len);

	at24c08_packet_read(fd_at24c08, I2C_ADDR, cmd->eeprom_addr,
							buf, cmd->len);
	if ((cmd->flags & CHEST_DISP_MASK) == CHEST_DISP_ONE) {
		/* Display one byte */
		for (idx = 0; idx < cmd->len; idx++) {
			if ((cmd->flags & CHEST_SCALE_MASK) == CHEST_READ_HEX)
				printf("%02x", buf[idx]);
			else if ((cmd->flags & CHEST_SCALE_MASK) == 
								CHEST_READ_OCT)
				printf("%02o", buf[idx]);
			else
				printf("%02d", buf[idx]);
		}
	} else {
		/* Integer: 64bit */
		int64_t value = 0;

		for (idx = 0; idx < cmd->len; idx++)
			value |= buf[idx] << (idx * 8);
		if (!(cmd->flags & CHEST_SECT))
			printf("%lld", (long long int)(unsigned long)value);
		free(buf);
		return value;
	}

	free(buf);
	return 0;
}

/* Interger write */
static int chest_integer_write(struct chest_cmd_struct *cmd, 
						char *argv[], int optind)
{
	unsigned char *buf;
	int idx;

	buf = malloc(cmd->len);
	memset(buf, 0, cmd->len);

	if ((cmd->flags & CHEST_DISP_MASK) == CHEST_DISP_ONE) {
		for (idx = 0; idx < cmd->len; idx++) {
			char *tmp = argv[optind] + idx;
			if (tmp)
				break;
			if ((cmd->flags & CHEST_SCALE_MASK) == CHEST_READ_HEX)
				sscanf(tmp, "%hhx", &buf[idx]);
			else if ((cmd->flags & CHEST_SCALE_MASK) == 
								CHEST_READ_OCT)
				sscanf(tmp, "%hhd", &buf[idx]);
			else
				sscanf(tmp, "%hho", &buf[idx]);
		}
	} else {
		int64_t value = 0;

		sscanf(argv[optind], "%lld", 
				(long long int *)(unsigned long)&value);
		for (idx = 0; idx < cmd->len; idx++) {
			buf[idx] = (value >> (idx * 8)) & 0xff;
		}
	}

	at24c08_packet_write(fd_at24c08, I2C_ADDR, cmd->eeprom_addr,
							buf, cmd->len);

	free(buf);
	return 0;
}

/* String Read */
static int chest_string_read(struct chest_cmd_struct *cmd, char *argv[])
{
	unsigned char *buf;
	
	buf = malloc(cmd->len + 1);
	memset(buf, 0, cmd->len + 1);

	at24c08_packet_read(fd_at24c08, I2C_ADDR, cmd->eeprom_addr,
							buf, cmd->len);
	
	buf[cmd->len] = '\0';
	printf("%s", buf);
	free(buf);
	return 0;
}

/* String write */
static int chest_string_write(struct chest_cmd_struct *cmd,
						char *argv[], int optind)
{
	unsigned char *buf;
	int len = strlen(argv[optind]);

	/* String len */

	buf = malloc(cmd->len + 1);
	memset(buf, 0, cmd->len + 1);
	if (len > cmd->len)
		len = cmd->len;

	strncpy(buf, argv[optind], len);
	buf[len + 1] = '\0';

	at24c08_packet_write(fd_at24c08, I2C_ADDR, cmd->eeprom_addr,
							buf, cmd->len);
	free(buf);
}

/* Passwd Read */
static int chest_passwd_read(struct chest_cmd_struct *cmd, char *argv[])
{
	struct chest_cmd_struct *psdlen_cmd;
	struct chest_cmd_struct tmp;

	/* Read passwd length */
	psdlen_cmd = parse_chest_cmd("psdlen");

	tmp.len = chest_integer_read(psdlen_cmd, argv);
	tmp.eeprom_addr = cmd->eeprom_addr;
	chest_string_read(&tmp, argv);

	return 0;
}

/* Passwd Write */
static int chest_passwd_write(struct chest_cmd_struct *cmd, 
						char *argv[], int optind)
{
	struct chest_cmd_struct *psdlen_cmd;
	char len = (char)(unsigned long)strlen(argv[optind]);
	char *buf;

	/* Read passwd length */
	psdlen_cmd = parse_chest_cmd("psdlen");
	at24c08_packet_write(fd_at24c08, I2C_ADDR, psdlen_cmd->eeprom_addr,
						&len, psdlen_cmd->len);
	buf = malloc(len + 1);
	memset(buf, 0, len + 1);
	strcpy(buf, argv[optind]);
	
	at24c08_packet_write(fd_at24c08, I2C_ADDR, cmd->eeprom_addr,
							buf, len);
	free(buf);
	return 0;
}

/* MAC/IP Address Read */
static int chest_addr_read(struct chest_cmd_struct *cmd, char *argv[])
{
	unsigned char *buf;
	int idx;

	buf = malloc(cmd->len);

	at24c08_packet_read(fd_at24c08, I2C_ADDR, cmd->eeprom_addr,
							buf, cmd->len);
	for (idx = 0; idx < cmd->len; idx++) {
		if (cmd->type == CMD_TYPE_MAC)
			printf("%02x", buf[idx]);
		else if (cmd->type == CMD_TYPE_IP)
			printf("%d", buf[idx]);
		fflush(stdout);
		if (idx < (cmd->len - 1)) {
			if (cmd->type == CMD_TYPE_MAC)
				printf(":");
			else if (cmd->type == CMD_TYPE_IP)
				printf(".");
		}
	}
	free(buf);
	return 0;
}

/* MAC/IP Address Write */
static int chest_addr_write(struct chest_cmd_struct *cmd, 
					char *argv[], int optind)
{
	unsigned char *buf, tmp[8];
	char *prev = argv[optind];
	char *next;
	int idx;

	buf = malloc(cmd->len);

	for (idx = 0; idx < cmd->len; idx++) {
		memset(tmp, 0, 8);
		if (cmd->type == CMD_TYPE_MAC)
			next = strchr(prev, ':');
		else if (cmd->type == CMD_TYPE_IP)
			next = strchr(prev, '.');
		if (!next)
			strncpy(tmp, prev, strlen(prev));
		else
			strncpy(tmp, prev, next - prev);
		if (cmd->type == CMD_TYPE_MAC)
			sscanf(tmp, "%hhx", &buf[idx]);
		else if (cmd->type == CMD_TYPE_IP)
			sscanf(tmp, "%hhd", &buf[idx]);
		prev = next + 1;
	}
	at24c08_packet_write(fd_at24c08, I2C_ADDR, cmd->eeprom_addr,
							buf, cmd->len);
	free(buf);
	return 0;
}

static struct chest_cmd_struct *parse_chest_cmd(const char *string)
{
        int i;

        for (i = 0; cmd_lists[i].cmdline; i++) {
                if (strcmp(string, cmd_lists[i].cmdline) == 0) {
                        /* Obtain special command structure */
                        return &cmd_lists[i];
                }
        }
        printf("Invalid command line: %s\n", string);
        return NULL;
}

/* Read interface */
int chest_read_interface(char *optarg, char *argv[])
{
	struct chest_cmd_struct *cmd;

	cmd = parse_chest_cmd(optarg);
	if (!cmd)
		return -1;

	switch (cmd->type) {
	case CMD_TYPE_ID:
	case CMD_TYPE_VERSION:
	case CMD_TYPE_INT:
	case CMD_TYPE_BOOLEAN:
	case CMD_TYPE_PORT:
		return chest_integer_read(cmd, argv);
	case CMD_TYPE_STRING:
		return chest_string_read(cmd, argv);
	case CMD_TYPE_PASSWD:
		return chest_passwd_read(cmd, argv);
	case CMD_TYPE_MAC:
	case CMD_TYPE_IP:
		return chest_addr_read(cmd, argv);
	defau1t:
		return -1;
	}
}

/* Write interface */
int chest_write_interface(char *optarg, char *argv[], int optind)
{
	struct chest_cmd_struct *cmd;

	cmd = parse_chest_cmd(optarg);
	if (!cmd)
		return -1;

	switch (cmd->type) {
	case CMD_TYPE_INT:
	case CMD_TYPE_PORT:
	case CMD_TYPE_BOOLEAN:
	case CMD_TYPE_ID:
	case CMD_TYPE_VERSION:
		return chest_integer_write(cmd, argv, optind);
	case CMD_TYPE_STRING:
		return chest_string_write(cmd, argv, optind);
	case CMD_TYPE_PASSWD:
		return chest_passwd_write(cmd, argv, optind);
	case CMD_TYPE_MAC:
	case CMD_TYPE_IP:
		return chest_addr_write(cmd, argv, optind);
	defau1t:
		return -1;
	}
}
