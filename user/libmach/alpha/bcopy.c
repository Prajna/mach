/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
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
 *  $Log:	bcopy.c,v $
 * Revision 2.2  93/01/14  18:02:36  danner
 * 	Created.
 * 	[92/12/10            af]
 * 
 */

#include <mach/std_types.h>

/*
 *	Object:
 *		bcopy				EXPORTED function
 *
 *		Memory copy
 *
 */
void
bcopy(from, to, bcount)
	register vm_offset_t from;
	register vm_offset_t to;
	register unsigned bcount;
{
	register int    i;

	if ((from & 3) != (to & 3)) {
		/* wont align easily */
		while (bcount--)
			*((char *) to++) = *((char *) from++);
		return;
	}
	switch (to & 3) {
	    case 1:
		*((char *) to++) = *((char *) from++);
		if (--bcount == 0)
			return;
	    case 2:
		*((char *) to++) = *((char *) from++);
		if (--bcount == 0)
			return;
	    case 3:
		*((char *) to++) = *((char *) from++);
		if (--bcount == 0)
			return;
	    default:
		break;
	}

	for (i = bcount >> 2; i; i--, to += 4, from += 4)
		*((int *) to) = *((int *) from);

	switch (bcount & 3) {
	    case 3:
		*((char *) to++) = *((char *) from++);
	    case 2:
		*((char *) to++) = *((char *) from++);
	    case 1:
		*((char *) to++) = *((char *) from++);
	    default:
		break;
	}
}

