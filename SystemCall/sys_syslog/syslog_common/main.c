/*
 * sys_syslog in C
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
/* __NR_syslog */
#include <asm/unistd.h>
/* syscall() */
#include <unistd.h>
#include <sys/syslog.h>

/* Architecture defined */
#ifndef __NR_syslog
#define __NR_syslog	103
#endif

static void usage(const char *program_name)
{
	printf("BiscuitOS: sys_syslog helper\n");
	printf("Usage:\n");
	printf("      %s <-t log_type> <-m message>\n", program_name);
	printf("\n");
	printf("\t-t\t--log_type\tThe log type.\n");
	printf("\t\t\tLOG_EMERG       system is unusable\n");
	printf("\t\t\tLOG_ALERT       action must be taken immediately\n");
	printf("\t\t\tLOG_CRIT        critical conditions\n");
	printf("\t\t\tLOG_ERR         error conditions\n");
	printf("\t\t\tLOG_WARNING     warning conditions\n");
	printf("\t\t\tLOG_NOTICE      normal but significant condition\n");
	printf("\t\t\tLOG_INFO        informational\n");
	printf("\t\t\tLOG_DEBUG       debug-level messages\n");
	printf("\ne.g:\n");
	printf("%s -t LOG_INFO -m Hello-BiscuitOS\n\n", program_name);
}

int main(int argc, char *argv[])
{
	char *type = NULL;
	char *message = NULL;
	int c, hflags = 0;
	int log_type = 0;
	opterr = 0;

	/* options */
	const char *short_opts = "ht:m:";
	const struct option long_opts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "type", required_argument, NULL, 't'},
		{ "message", required_argument, NULL, 'm'},
		{ 0, 0, 0, 0 }
	};

	while ((c = getopt_long(argc, argv, short_opts, 
						long_opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			hflags = 1;
			break;
		case 't': /* type */
			type = optarg;
			break;
		case 'm': /* message */
			message = optarg;
			break;
		default:
			abort();
		}
	}

	if (hflags || !type || !message) {
		usage(argv[0]);
		return 0;
	}

	/* parse log-type argument */
	if (strstr(type, "LOG_EMERG"))
		log_type |= LOG_EMERG;
	if (strstr(type, "LOG_ALERT"))
		log_type |= LOG_ALERT;
	if (strstr(type, "LOG_CRIT"))
		log_type |= LOG_CRIT;
	if (strstr(type, "LOG_ERR"))
		log_type |= LOG_ERR;
	if (strstr(type, "LOG_WARNING"))
		log_type |= LOG_WARNING;
	if (strstr(type, "LOG_NOTICE"))
		log_type |= LOG_NOTICE;
	if (strstr(type, "LOG_INFO"))
		log_type |= LOG_INFO;
	if (strstr(type, "LOG_DEBUG"))
		log_type |= LOG_DEBUG;

	/*
	 * sys_syslog
	 *
	 *    SYSCALL_DEFINE3(syslog,
	 *                    int, type,
	 *                    char __user *, buf,
	 *                    int, len)
	 */
	syscall(__NR_syslog, log_type, message, strlen(message));

	return 0;
}
