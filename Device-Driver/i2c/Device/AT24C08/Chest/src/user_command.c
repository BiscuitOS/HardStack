/*
 * User command
 *
 * (C) 2019.10.28 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include "command.h"

struct chest_cmd_struct cmd_lists[] = {
	/* Chip ID Definition */
	{
		.type		= CMD_TYPE_ID,
		.eeprom_addr	= 0x00,
		.len		= 4,
		.flags		= CHEST_READ_HEX,
		.cmdline	= "chip_id",
		.usage		= "%s <-r/-w> chip_id",
		.desc		= "Machine ID",
	},
	/* Version Definition */
	{
		.type		= CMD_TYPE_VERSION,
		.eeprom_addr	= 0x04,
		.len		= 4,
		.flags		= CHEST_READ_HEX,
		.cmdline	= "version",
		.usage		= "%s <-r/-w> version",
		.desc		= "Machine version",
	},
	/* String Definition */
	{
		.type		= CMD_TYPE_STRING,
		.eeprom_addr	= 0x08,
		.len		= 20,
		.cmdline	= "SN",
		.usage		= "%s <-r/-w> SN",
		.desc		= "SN string"
	},
	/* Integer Definition */
	{
		.type		= CMD_TYPE_INT,
		.eeprom_addr	= 0x0A,
		.len		= 4,
		.flags		= CHEST_READ_DEC | CHEST_DISP_SER,
		.cmdline	= "number",
		.usage		= "%s <-r/-w> number",
		.desc		= "Integer number",
	},
	/* Boolean Definition */
	{
		.type		= CMD_TYPE_BOOLEAN,
		.eeprom_addr	= 0x0A,
		.len		= 1,
		.flags		= CHEST_READ_DEC | CHEST_DISP_SER,
		.cmdline	= "bool",
		.usage		= "%s <-r/-w> bool",
		.desc		= "Boolean number",
	},
	/* Password Definition */
	{
		.type		= CMD_TYPE_INT,
		.eeprom_addr	= 0x1A,
		.len		= 1,
		.flags		= CHEST_READ_DEC | CHEST_DISP_SER | CHEST_SECT,
		.cmdline	= "psdlen",
		.desc		= "Password Length",
	},
	{
		.type		= CMD_TYPE_PASSWD,
		.eeprom_addr	= 0x1B,
		.cmdline	= "password",
		.usage		= "%s <-r/-w> password",
		.desc		= "Password",
	},
	/* MAC Address Definition */
	{
		.type		= CMD_TYPE_MAC,
		.eeprom_addr	= 0x20,
		.len		= 6,
		.cmdline	= "mac",
		.usage		= "%s <-r/-w> mac",
		.desc		= "MAC address",
	},
	/* IP Address Definition */
	{
		.type		= CMD_TYPE_IP,
		.eeprom_addr	= 0x28,
		.len		= 4,
		.cmdline	= "ip",
		.usage		= "%s <-r/-w> ip",
		.desc		= "IP address",
	},
	/* PORT definition */
	{
		.type		= CMD_TYPE_PORT,
		.eeprom_addr	= 0x2c,
		.len		= 1,
		.flags		= CHEST_READ_DEC | CHEST_DISP_SER,
		.cmdline	= "port",
		.usage		= "%s <-r/-w> port",
		.desc		= "Port information",
	},
	/* Must setup empty node */
	{ },
};
