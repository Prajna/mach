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
 * $Log:	test.c,v $
 * Revision 2.3  93/08/31  15:15:50  mrt
 * 	main --> _main_
 * 	[93/08/26  17:33:09  af]
 * 
 * Revision 2.2  93/02/05  08:01:31  danner
 * 	Created.
 * 	[93/02/04            af]
 * 
 */

#include <mach/std_types.h>
#include "prom_routines.h"

_main_(arg0)
	long arg0;
{
	puts("Hi mom!\n");
	putnum(arg0);
	puts(" == arg0\n");

	{
		char			devname[128];
		prom_return_t		ret;

		ret.bits = prom_getenv( PROM_E_BOOTED_DEV, devname, sizeof devname);

		prom_puts(console, devname, ret.u.retval);
		puts(" is BOOTED_DEV\n");

		ret.bits = prom_getenv( PROM_E_BOOTED_FILE, devname, sizeof devname);

		prom_puts(console, devname, ret.u.retval);
		puts(" is BOOTED_FILE\r\n");
	}
}
