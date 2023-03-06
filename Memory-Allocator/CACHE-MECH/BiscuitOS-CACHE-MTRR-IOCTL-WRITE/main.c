/*  mtrr-add.c

    Source file for mtrr-add (example programme to add an MTRRs using ioctl())

    Copyright (C) 1997-1998  Richard Gooch

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Richard Gooch may be reached by email at  rgooch@atnf.csiro.au
    The postal address is:
      Richard Gooch, c/o ATNF, P. O. Box 76, Epping, N.S.W., 2121, Australia.

    This programme will use an ioctl() on /proc/mtrr to add an entry. The first
    available mtrr is used. This is an alternative to writing /proc/mtrr.


    Written by      Richard Gooch   17-DEC-1997

    Last updated by Richard Gooch   2-MAY-1998
    BiscuitOS Running               2023-03-05
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <asm/mtrr.h>

#define TRUE 1
#define FALSE 0
#define ERRSTRING strerror (errno)

static char *mtrr_strings[MTRR_NUM_TYPES] =
{
	"uncachable",               /* 0 */
	"write-combining",          /* 1 */
	"?",                        /* 2 */
	"?",                        /* 3 */
	"write-through",            /* 4 */
	"write-protect",            /* 5 */
	"write-back",               /* 6 */
};

int main (int argc, char **argv)
{
	int fd;
	struct mtrr_sentry sentry;

	if (argc != 4) {
		fprintf (stderr, "Usage:\tmtrr-add base size type\n");
		exit(1);
	}
	sentry.base = strtoul(argv[1], NULL, 0);
	sentry.size = strtoul(argv[2], NULL, 0);

	for (sentry.type = 0; sentry.type < MTRR_NUM_TYPES; ++sentry.type) {
		if (strcmp(argv[3], mtrr_strings[sentry.type]) == 0)
			break;
	}
    
	if (sentry.type >= MTRR_NUM_TYPES) {
		fprintf(stderr, "Illegal type: \"%s\"\n", argv[3]);
		exit(2);
	}

	if((fd = open ("/proc/mtrr", O_WRONLY, 0)) == -1) {
		if (errno == ENOENT) {
			fputs ("/proc/mtrr not found: not supported or "
					"you don't have a PPro?\n", stderr);
			exit(3);
		}
  		printf("Error opening /proc/mtrr\t%s\n", ERRSTRING);
		exit(4);
	}

	if (ioctl(fd, MTRRIOC_ADD_ENTRY, &sentry) == -1) {
		printf("Error doing ioctl(2) on /dev/mtrr\t%s\n", ERRSTRING);
		exit(5);
	}

	printf("Sleeping for 5 seconds so you can see the new entry\n");
	/* Just for debug */
	sleep(-1);
	close(fd);
	fputs ("I've just closed /proc/mtrr so now the new "
					"entry should be gone\n", stderr);
} /*  End Function main  */
