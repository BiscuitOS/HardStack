/*  mtrr-show.c

    Source file for mtrr-show (example program to show MTRRs using ioctl()'s)

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

    This program will use an ioctl() on /proc/mtrr to show the current MTRR
    settings. This is an alternative to reading /proc/mtrr.


    Written by      Richard Gooch   17-DEC-1997

    Last updated by Richard Gooch   2-MAY-1998
    Running BiscuitOS		    2023-03-05
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int main()
{
	struct mtrr_gentry gentry;
	int fd;

	if ((fd = open ("/proc/mtrr", O_RDONLY, 0)) == -1) {
		if (errno == ENOENT) {
			fputs ("/proc/mtrr not found: not supported or "
					"you don't have a PPro?\n", stderr);
			exit (1);
		}
		fprintf(stderr, "Error opening /proc/mtrr\t%s\n", ERRSTRING);
		exit(2);
	}

	for (gentry.regnum = 0; ioctl(fd, MTRRIOC_GET_ENTRY, &gentry) == 0;
							++gentry.regnum) {
		if (gentry.size < 1) {
			printf("Register: %u disabled\n", gentry.regnum);
			continue;
		}
		printf ("Register: %u base: 0x%llx size: 0x%x type: %s\n",
				gentry.regnum, gentry.base, gentry.size,
				mtrr_strings[gentry.type]);
	}

	if (errno == EINVAL)
		exit(0);

	printf ("Error doing ioctl(2) on /dev/mtrr\t%s\n", ERRSTRING);
	exit(3);
}   /*  End Function main  */
