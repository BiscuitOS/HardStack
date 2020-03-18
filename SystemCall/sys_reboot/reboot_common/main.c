/*
 * sys_reboot in C
 *
 * (C) 2020.03.11 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
/* __NR_reboot */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>

/* Architecture defined */
#ifndef __NR_open
#define __NR_open	5
#endif
#ifndef __NR_close
#define __NR_close	6
#endif

/* Architecture flags */
/*
 * Magic values required to use _reboot() system call.
 */

#define LINUX_REBOOT_MAGIC1	0xfee1dead
#define LINUX_REBOOT_MAGIC2	672274793
#define LINUX_REBOOT_MAGIC2A	85072278
#define LINUX_REBOOT_MAGIC2B	369367448
#define LINUX_REBOOT_MAGIC2C	537993216


/*
 * Commands accepted by the _reboot() system call.
 *
 * RESTART     Restart system using default command and mode.
 * HALT        Stop OS and give system control to ROM monitor, if any.
 * CAD_ON      Ctrl-Alt-Del sequence causes RESTART command.
 * CAD_OFF     Ctrl-Alt-Del sequence sends SIGINT to init task.
 * POWER_OFF   Stop OS and remove all power from system, if possible.
 * RESTART2    Restart system using given command string.
 * SW_SUSPEND  Suspend system using software suspend if compiled in.
 * KEXEC       Restart system using a previously loaded Linux kernel
 */
        
#define LINUX_REBOOT_CMD_RESTART	0x01234567
#define LINUX_REBOOT_CMD_HALT		0xCDEF0123
#define LINUX_REBOOT_CMD_CAD_ON		0x89ABCDEF
#define LINUX_REBOOT_CMD_CAD_OFF	0x00000000
#define LINUX_REBOOT_CMD_POWER_OFF	0x4321FEDC
#define LINUX_REBOOT_CMD_RESTART2	0xA1B2C3D4
#define LINUX_REBOOT_CMD_SW_SUSPEND	0xD000FCE2
#define LINUX_REBOOT_CMD_KEXEC		0x45584543

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_reboot helper\n");
	printf("Usage:\n");
	printf("      %s <-M MAGIC1> <-m MAJIC2> <-c cmd>\n", program_name);
	printf("\n");
	printf("\t-M\t--MAGIC1\tThe magic1 for reboot.\n");
	printf("\t-n\t--MAGIC2\tThe magic2 for reboot.\n");
	printf("\t\t\tLINUX_REBOOT_MAGIC1\n");
	printf("\t\t\tLINUX_REBOOT_MAGIC2\n");
	printf("\t\t\tLINUX_REBOOT_MAGIC2A\n");
	printf("\t\t\tLINUX_REBOOT_MAGIC2B\n");
	printf("\t\t\tLINUX_REBOOT_MAGIC2C\n");
	printf("\t-c\t--cmd\tThe command accepted by reboot.\n");
	printf("\t\t\tRESTART      Restart system using default command and mode\n");
	printf("\t\t\tHALT         Stop OS and give system control to ROM monitor, if any\n");
	printf("\t\t\tCAD_ON       Ctrl-Alt-Del sequence causes RESTART command\n");
	printf("\t\t\tCAD_OFF      Ctrl-Alt-Del sequence sends SIGINT to init task\n");
	printf("\t\t\tPOWER_OFF    Stop OS and remove all power from system, if possible\n");
	printf("\t\t\tRESTART2     Restart system using given command string\n");
	printf("\t\t\tSW_SUSPEND   Suspend system using software suspend if compiled in\n");
	printf("\t\t\tKEXEC        Restart system using a previously loaded Linux kernel\n");
	printf("\ne.g:\n");
	printf("%s -M LINUX_REBOOT_MAGIC1 -m LINUX_REBOOT_MAGIC2 "
			"-c POWER_OFF\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *m1 = NULL;
	char *m2 = NULL;
	char *cmd = NULL;
	int mag1, mag2;
	unsigned int ocmd;
	int c, hflags = 0;
	opterr = 0;

	/* options */
	const char *short_opts = "hM:m:c:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "Magic1", required_argument, NULL, 'M'},
		{ "Magic2", required_argument, NULL, 'm'},
		{ "command", required_argument, NULL, 'c'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 'M': /* MAGIC1 */
			m1 = optarg;
			break;
		case 'm': /* MAGIC2 */
			m2 = optarg;
			break;
		case 'c': /* command */
			cmd = optarg;
			break;
		default:
			abort();
		}
	}

	if (hflags || !m1 || !m2 || !cmd) {
		usage(argv[0]);
		return 0;
	}

	/* parse MAGIC1 */
	if (strstr(m1, "LINUX_REBOOT_MAGIC1"))
		mag1 = LINUX_REBOOT_MAGIC1;
	else if (strstr(m1, "LINUX_REBOOT_MAGIC2"))
		mag1 = LINUX_REBOOT_MAGIC2;
	else if (strstr(m1, "LINUX_REBOOT_MAGIC2A"))
		mag1 = LINUX_REBOOT_MAGIC2A;
	else if (strstr(m1, "LINUX_REBOOT_MAGIC2B"))
		mag1 = LINUX_REBOOT_MAGIC2B;
	else if (strstr(m1, "LINUX_REBOOT_MAGIC2C"))
		mag1 = LINUX_REBOOT_MAGIC2C;

	/* parse MAGIC2 */
	if (strstr(m2, "LINUX_REBOOT_MAGIC1"))
		mag2 = LINUX_REBOOT_MAGIC1;
	else if (strstr(m2, "LINUX_REBOOT_MAGIC2"))
		mag2 = LINUX_REBOOT_MAGIC2;
	else if (strstr(m2, "LINUX_REBOOT_MAGIC2A"))
		mag2 = LINUX_REBOOT_MAGIC2A;
	else if (strstr(m2, "LINUX_REBOOT_MAGIC2B"))
		mag2 = LINUX_REBOOT_MAGIC2B;
	else if (strstr(m2, "LINUX_REBOOT_MAGIC2C"))
		mag2 = LINUX_REBOOT_MAGIC2C;

	/* parse Command */
	if (strstr(cmd, "LINUX_REBOOT_CMD_RESTART"))
		ocmd = LINUX_REBOOT_CMD_RESTART;
	else if (strstr(cmd, "LINUX_REBOOT_CMD_HALT"))
		ocmd = LINUX_REBOOT_CMD_HALT;
	else if (strstr(cmd, "LINUX_REBOOT_CMD_CAD_ON"))
		ocmd = LINUX_REBOOT_CMD_CAD_ON;
	else if (strstr(cmd, "LINUX_REBOOT_CMD_CAD_OFF"))
		ocmd = LINUX_REBOOT_CMD_CAD_OFF;
	else if (strstr(cmd, "LINUX_REBOOT_CMD_POWER_OFF"))
		ocmd = LINUX_REBOOT_CMD_POWER_OFF;
	else if (strstr(cmd, "LINUX_REBOOT_CMD_RESTART2"))
		ocmd = LINUX_REBOOT_CMD_RESTART2;
	else if (strstr(cmd, "LINUX_REBOOT_CMD_SW_SUSPEND"))
		ocmd = LINUX_REBOOT_CMD_SW_SUSPEND;
	else if (strstr(cmd, "LINUX_REBOOT_CMD_KEXEC"))
		ocmd = LINUX_REBOOT_CMD_KEXEC;

	/*
	 * sys_reboot
	 *
	 *    SYSCALL_DEFINE4(reboot,
	 *                    int, magic1,
	 *                    int, magic2,
	 *                    unsigned int, cmd,
	 *                    void __user *, arg)
	 */
	syscall(__NR_reboot, mag1, mag2, ocmd, NULL);

	return 0;
}
