/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * Copyright (c) 1991 Sequent Computer Systems
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND SEQUENT COMPUTER SYSTEMS ALLOW FREE USE OF
 * THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * SEQUENT COMPUTER SYSTEMS DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	setconf.c,v $
 * Revision 2.4  93/05/18  11:42:50  rvb
 * 	Lint
 * 
 * Revision 2.3  91/07/31  18:03:39  dbg
 * 	Changed copyright.
 * 	Moved root_name string out of text segment.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  12:58:47  dbg
 * 	Adapted for pure Mach kernel.
 * 	[90/10/09            dbg]
 * 
 */

/*
 * $Header: setconf.c,v 2.4 93/05/18 11:42:50 rvb Exp $
 */

/*
 * Revision 1.1  89/11/14  18:35:23  root
 * Initial revision
 * 
 * Revision 1.2  89/08/16  15:17:31  root
 * balance -> sqt
 * 
 * Revision 1.1  89/07/05  13:17:37  kak
 * Initial revision
 * 
 * Revision 2.2  88/03/20  17:32:11  bak
 * removed xp entry from genericconf structure. xp not supported in symmetry.
 * 
 */
#include <mach/boolean.h>

#include <sqt/cfg.h>
#include <sqt/vm_defs.h>

char	root_name_string[16] = "\0\0\0\0\0\0\0";	/* at least ddNNp */
char	*root_name = root_name_string;

#define	MINARGLEN	5

/*
 * The generic rootdev is passed into the kernel as the
 * 1st argument in the boot name. The argument specifies the device and
 * unit number to use for the rootdev. The actual partitions to be used for
 * a given device is specified in ../conf/conf_generic.c.
 * The argument string is in the form:
 *		XXds		(eg. sd0a)
 *
 *	where	XX is the device type (e.g. sd for scsi disk),
 *		d  is the Dynix device unit number,
 *		s is the Dynix device slice (a-g)
 */

setconf()
{
	register struct genericconf *gc;
	register char *boot_name;
	register char *name;

	/*
	 * Find Generic rootdev string.
	 *
	 * Skip past bootname, then till argument string.
	 * If argument string missing, use boot device name.
	 */

	boot_name = PHYSTOKV(va_CD_LOC->c_boot_name, char *);

	for (name = boot_name; *name != '\0'; name++)
		continue;
	while ((*name == '\0') && (name < &boot_name[BNAMESIZE]))
		name++;
	if (name > &boot_name[BNAMESIZE-MINARGLEN]) {
		boot_name[BNAMESIZE-1] = '\0';

		/*
		 * Use boot name prefix.
		 * zd(0,0) becomes zd0a.
		 */
		name = boot_name;
		root_name[0] = name[0];
		root_name[1] = name[1];
		if (name[2] != '(')
			goto error;
		root_name[2] = name[3];
		if (name[4] != ',')
			goto error;
		root_name[3] = name[5] - '0' + 'a';
	}
	else {
		strncpy(root_name, name, 5);
	}
	root_name[4] = '\0';

	printf("Root on %s.\n", root_name);
	return;

error:
	printf("Generic root device ");
	if (*name == '0')
		printf("not specified.\n");
	else
		printf("\"%s\" is incorrect.\n", name);

	printf("Returning to firmware\n");
	return_fw(FALSE);
}

