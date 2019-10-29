#ifndef _COMMAND_H_
#define _COMMAND_H_

#define	CHEST_READ_HEX		0x00000001
#define CHEST_READ_DEC		0x00000002
#define CHEST_READ_OCT		0x00000004
#define CHEST_SCALE_MASK	(CHEST_READ_HEX | CHEST_READ_DEC | CHEST_READ_OCT)
#define CHEST_DISP_ONE		0x00000010
#define CHEST_DISP_SER		0x00000020
#define CHEST_DISP_MASK		(CHEST_DISP_ONE | CHEST_DISP_SER)
#define CHEST_SECT		0x80000000

enum {
	CMD_TYPE_MAC,
	CMD_TYPE_IP,
	CMD_TYPE_BOOLEAN,
	CMD_TYPE_PASSWD,
	CMD_TYPE_STRING,
	CMD_TYPE_INT,
	CMD_TYPE_ID,
	CMD_TYPE_VERSION,
	CMD_TYPE_PORT,
};

struct chest_cmd_struct
{
	unsigned int type;
	const char *defaults;
	unsigned int flags;
	unsigned char eeprom_addr;
	unsigned char len;
	const char *cmdline;
	const char *usage;
	const char *desc;
};

extern struct chest_cmd_struct cmd_lists[];
static struct chest_cmd_struct *parse_chest_cmd(const char *string);
#endif
