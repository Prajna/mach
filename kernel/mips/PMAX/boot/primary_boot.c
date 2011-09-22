/*  
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	primary_boot.c,v $
 * Revision 2.7  91/08/24  12:20:45  af
 * 	Pass along all 4 arguments, needed on 3min.
 * 	[91/08/22  11:17:49  af]
 * 
 * Revision 2.6  91/05/14  17:18:26  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:39:08  mrt
 * 	Added author notices
 * 	[91/02/04  11:10:57  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:06:23  mrt]
 * 
 * Revision 2.4  91/01/09  18:51:49  rpd
 * 	For new proms, zero envp because it is bogus.  This lets
 * 	me boot 2.5 on 3maxen again.
 * 	[91/01/09  16:33:38  af]
 * 
 * Revision 2.3  90/12/05  23:30:11  af
 * 	Created.
 * 	[90/12/02            af]
 * 
 */
/*
 *	File: primary_boot.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	12/90
 *
 *	Primary (and only) bootstrap.
 */

#include <asm_misc.h>

#define	RETRIES	20

main(argc,argv,arg2,arg3)
	char	**argv;
{
	static char	*whatsthat = "?%s?\n";
	char		buf[256];
	register int	i, file;

	/*
	 *	New proms ?
	 */
	if (strcmp(*argv, "boot") == 0) {
		argv++,argc--;
	}

	/*
	 *	Need to retry many times, broken
	 *	SCSI driver does not reset properly
	 */
retry:
	for (i = 0; i < RETRIES; i++)
		if ((file = open(*argv, 0)) >= 0)
			break;
	if (file < 0) {
		prom_printf(whatsthat, *argv);
askuser:
		prom_printf("Boot file ? ");
		prom_gets(buf);
		if (*buf == 0)
			goto askuser;
		*argv = buf;
		goto retry;
	}

	/*
	 *	Try to exec the file, will only
	 *	return in case of errors
	 */
	exec(file, argc, argv, arg2, arg3);
	close(file);

	/* just loop forever, user can ^C out of it */
	goto askuser;
}
