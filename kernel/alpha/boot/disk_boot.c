/*  
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
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
 * $Log:	disk_boot.c,v $
 * Revision 2.4  93/08/31  15:15:48  mrt
 * 	Contrary to SRM specs, the proms on the Flamingo
 * 	do not have a BOOT_DEF_FILE variable.  So try
 * 	for "mach" if we do not know what to do.
 * 	[93/08/26  17:34:37  af]
 * 
 * Revision 2.3  93/03/09  10:49:39  danner
 * 	main --> _main_ to spare GCC wastes.
 * 	Interactively asking for file works.
 * 	[93/03/05            af]
 * 
 * Revision 2.2  93/02/05  08:00:44  danner
 * 	Rewritten for alpha.
 * 	[93/02/04            af]
 * 
 */
/*
 *	File: disk_boot.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	12/90
 *
 *	Primary (and only) bootstrap.
 */

#include <mach/std_types.h>
#include "prom_routines.h"
#include "min_io.h"

_main_(arg0)
	long	arg0;
{
	open_file_t	file;
	prom_return_t	ret;
	char		buf[128];

	ret.bits = prom_getenv( PROM_E_BOOTED_FILE, buf, 128);
	if (ret.u.retval)
		buf[ret.u.retval] = 0;	
	else
		bcopy("mach", buf, sizeof "mach");

retry:
	if (open(buf, 0, &file) != 0) {
		prom_puts(console, buf, ret.u.retval);
		puts(" not found.\r\n");
#ifdef	INTERACTIVE
		puts("Boot file ? ");
		if ((ret.u.retval = gets(buf)) != 0)
			goto retry;
#endif
	} else {
		/*
		 *	Try to exec the file, will only
		 *	return in case of errors
		 */
		exec(file, arg0);
		close(file);
	}
}
