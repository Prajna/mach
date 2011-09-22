/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 * $Log:	setroot.c,v $
 * Revision 2.11  92/04/01  19:32:17  rpd
 * 	Renamed gets to safe_gets.
 * 	Fixed get_root_device.
 * 	[92/03/31            rpd]
 * 
 * Revision 2.10  92/02/19  16:29:33  elf
 * 	Make atleast a half hearted attempt to use the info provide by
 * 	the bootstrap to choose the boot device.
 * 	[92/02/07            rvb]
 * 
 * Revision 2.9  91/06/19  11:55:36  rvb
 * 	cputypes.h->platforms.h
 * 	[91/06/12  13:45:24  rvb]
 * 
 * Revision 2.8  91/05/14  16:16:26  mrt
 * 	Correcting copyright
 * 
 * Revision 2.7  91/05/08  12:42:00  dbg
 * 	Include platforms.h to get CPU names.
 * 	[91/03/21            dbg]
 * 
 * Revision 2.6  91/02/05  17:14:38  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:37:59  mrt]
 * 
 * Revision 2.5  90/12/04  14:46:35  jsb
 * 	iPSC2 -> iPSC386.
 * 	[90/12/04  11:19:36  jsb]
 * 
 * Revision 2.4  90/11/24  15:14:53  jsb
 * 	Added missing newline.
 * 	[90/11/24  11:45:51  jsb]
 * 
 * Revision 2.3  90/09/23  16:47:18  jsb
 * 	If iPSC386, root device is sd (scsi), not hd.
 * 
 * Revision 2.2  90/05/03  15:37:31  dbg
 * 	Created.
 * 	[90/02/20            dbg]
 * 
 */

#include <platforms.h>
#include <sys/reboot.h>

extern int boottype;


/*
 * Get root device.  Temporarily hard-coded.
 */
char root_string[5];

#if	iPSC386
char *root_name = "sd0a";
get_root_device()
{
	printf("root on %s\n", root_name);
}
#else	iPSC386
char *root_name = root_string;

/*
 *	 (4) (4) (4) (4)  (8)     (8)
 *	--------------------------------
 *	|MA | AD| CT| UN| PART  | TYPE |
 *	--------------------------------
 */

char *maj_devs[4] = { "hd", "fd", 0, "sd"};

get_root_device()
{
	int type = boottype & 0xff;
	int part = (boottype >> 8) & 0xff;
	int unit = (boottype >> 16) & 0x7;

	if ( (type > sizeof (maj_devs) / sizeof (char *)) ||
	     maj_devs[type] == 0) {
	}

	root_string[0] = maj_devs[type][0];
	root_string[1] = maj_devs[type][1];
	root_string[2] = '0' + unit;
	root_string[3] = 'a' + part;
	root_string[4] = 0;

	if (boothowto & RB_ASKNAME) {
		char name[5];

		printf("root device [%s]? ", root_name);
		safe_gets(name, sizeof name);
		if (*name != 0)
			strcpy(root_name, name);
	}
	printf("root on %s\n", root_name);
}
#endif	iPSC386
