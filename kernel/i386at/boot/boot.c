/*
 * Mach Operating System
 * Copyright (c) 1992, 1991 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

/*
 * HISTORY
 * $Log:	boot.c,v $
 * Revision 2.4.1.3  94/05/21  13:28:06  rvb
 * 	Allow a default boot string to be specified and used.
 * 
 * Revision 2.4.1.1  94/02/03  13:28:47  rvb
 * 	New banner date.  Initialize fd head for asm to 0.
 * 	[94/02/03            rvb]
 * 
 * Revision 2.4  93/08/10  15:56:46  mrt
 * 	DEBUG -> LABEL_DEBUG
 * 	[93/08/02            rvb]
 * 	Under #ifdef DEBUG allow for FILE, LABEL and parition printout
 * 	[93/07/09  15:14:39  rvb]
 * 
 * 	Don't print out "Insert file system" message for 3.0
 * 	[93/06/29            rvb]
 * 
 * Revision 2.3  93/05/10  17:46:58  rvb
 * 	Leave sector #1 empty so 386bsd can store its label there.
 * 	Fix typo in handling "ha" device.
 * 	New banner.
 * 	[93/04/01            rvb]
 * 
 * Revision 2.2  92/04/04  11:34:37  rpd
 * 	Change date in banner.
 * 	[92/04/03  16:51:14  rvb]
 * 
 * 	Fix Intel Copyright as per B. Davies authorization.
 * 	[92/04/03            rvb]
 * 	From 2.5 version.
 * 	[92/03/30            mg32]
 * 
 */

/*
  Copyright 1988, 1989, 1990, 1991, 1992 
   by Intel Corporation, Santa Clara, California.

                All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Intel
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <boot.h>
#include <a.out.h>
#include <sys/reboot.h>

#define	KERNEL_BOOT_ADDR	0x100000	/* load at 1 Megabyte */

struct exec head;
int argv[10], esym;
#ifdef	LABEL_DEBUG
int iflag = 0;
#endif	/* LABEL_DEBUG */
int pflag = -1;
char *name;
char *names[] = {
	"/mach", "/RFS/.LOCALROOT/mach",
	"/vmunix", "/RFS/.LOCALROOT/vmunix",
	"/RFS/.LOCALROOT/mach.old"
};
#define NUMNAMES	(sizeof(names)/sizeof(char *))

boot(drive)
     int drive;
{
	int loadflags, currname = 0;
	part = unit = 0;
#ifdef	SD
	maj = (drive&0x80 ? 3 : 1);
#else	SD
	maj = (drive&0x80 ? 0 : 1);
#endif	SD
	printf("\n>> MACH 3.0 BOOT w BSD LABEL w DOS: %dK/%dK mem  [4/4/94]\n",
	       argv[7] = memsize(0),
	       argv[8] = memsize(1));
	gateA20();
loadstart:
	loadflags = KERNEL_BOOT_ADDR;
	name = names[currname++];
	if (currname == NUMNAMES)
		currname = 0;
	getbootdev(&loadflags);
	if (pflag != -1) {
		loadpart(pflag);
		retry();
	}
	if (openrd()) {
		printf("Can't find %s\n", name);
		goto loadstart;
	}
	if (inode.i_mode&IEXEC)
		loadflags |= RB_KDB;
	loadprog(loadflags);
	goto loadstart;
}

loadprog(howto)
	int		howto;
{
	int i;
	char *addr;
	static int (*x_entry)() = 0;

	argv[3] = 0;
	argv[4] = 0;
	read(&head, sizeof(head));
	if (head.a_magic == 0407)
		poff = 32;
	else if (head.a_magic == 0413 ) {
		poff = 0;
		head.a_text += sizeof(struct exec);
	} else {
		printf("Invalid format!\n");
		return;
	}

	addr = (char *) (howto & 0xffff0000);
	printf("Booting %s(%d,%c)%s\n", devs[maj], unit, 'a'+part, name);
	printf("%d", head.a_text);
	xread(addr, head.a_text);
	addr += head.a_text;
	printf("+%d", head.a_data);
	xread(addr, head.a_data);
	addr += head.a_data;
	printf("+%d", head.a_bss);

	argv[3] = (int)(addr += head.a_bss);
	pcpy(&head.a_syms, addr, sizeof(head.a_syms));
	addr += sizeof(head.a_syms);
	printf("[+%d", head.a_syms);
	xread(addr, head.a_syms);
	addr += head.a_syms;
	read(&i, sizeof(int));
	pcpy(&i, addr, sizeof(int));
	if (i) {
		i -= sizeof(int);
		addr += sizeof(int);
		xread(addr, i);
		addr += i;
	}
	printf("+%d]", i);
	argv[4] = ((int)(addr+sizeof(int)-1))&~(sizeof(int)-1);

	printf("\n");

	/*
	 *  We now pass the various bootstrap parameters to the loaded
	 *  image via the argument list
	 *
	 *  arg1 = boot flags
	 *  arg2 = boot device
	 *  arg3 = start of symbol table (0 if not loaded)
	 *  arg4 = end of symbol table (0 if not loaded)
	 *  arg5 = transfer address from image
	 *  arg6 = transfer address for next image pointer
	 */
	argv[1] = howto;
	switch(maj) {
	case 1:
		if (! find("/mach_servers/startup")) {
			printf("\n\nInsert file system \n");
			getchar();
		}
		break;
	case 4:
		maj = 3;
		unit = 0;
		break;
	}
	argv[2] = (maj << B_TYPESHIFT) |
		(unit << B_UNITSHIFT) |
		(part << B_PARTITIONSHIFT);
	argv[5] = (head.a_entry &= 0xfffffff);
	argv[6] = (int) &x_entry;
	argv[0] = 8;
	startprog(head.a_entry,argv);
}

char namebuf[100];
getbootdev(howto)
     int *howto;
{
	extern char Default[];
	char c, *ptr = namebuf, *df = Default;

	printf("\nboot [%s]: ", df);
	gets(namebuf);
	if (! *namebuf) {
		while (*ptr++ = *df++);
		ptr = namebuf;
	}
	if (*namebuf) {
		while (c=*ptr) {
			while (c==' ')
				c = *++ptr;
			if (!c)
				return;
			if (c=='-')
				while ((c = *++ptr) && c!=' ')
					switch (c) {
					      case 'a':
						*howto |= RB_ASKNAME; continue;
					      case 's':
						*howto |= RB_SINGLE; continue;
					      case 'd':
						*howto |= RB_KDB; continue;
					      case 'b':
						*howto |= RB_HALT; continue;
#ifdef	LABEL_DEBUG
					      case 'F':
					         iflag |= I_FILE; continue;
					      case 'L':
					         iflag |= I_LABEL; continue;
					      case 'D':
					         iflag |= I_DOS; continue;
#endif	/* LABEL_DEBUG */
					      case '0': case '1': case '2': case '3':
						 pflag = c - '0'; continue;
					}
			else {
				name = ptr;
				while ((c = *++ptr) && c!=' ');
				if (c)
					*ptr++ = 0;
			}
		}
	}
}
