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
 * $Log:	sscanf.c,v $
 * Revision 2.3  93/03/26  17:54:53  mrt
 * 	Lint.
 * 	[93/03/23            af]
 * 
 * Revision 2.2  93/03/09  10:55:32  danner
 * 	Created.
 * 	[93/03/06  14:35:41  af]
 * 
 */
/*
 *	File: sscanf.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	3/93
 *
 *	Parse trivially simple strings
 */

#include <mach/std_types.h>
#if later
#include <sys/stdargs.h>
#else
#include <sys/varargs.h>
#endif

/*
 * All I need is a miracle ...
 * to keep this from growing like all the other sscanf!
 */
#if later
int
sscanf(
	register const unsigned char	*input,
	register const unsigned char	*fmt,
	...)
#else
int
sscanf( input, fmt, va_alist )
	unsigned char	*input;
	unsigned char	*fmt;
	va_dcl
#endif
{
	va_list		vp;
	register	int c;
	long		n;
	boolean_t	neg;
	const unsigned  char	*start = input;

#if later
	va_start(vp, fmt);
#else
	va_start(vp);
#endif

	while (c = *fmt++) {

	    if (c != '%' && c == *input) {
		input++;
		continue;
	    }

	    if (c != '%')
	        break;	/* mismatch */

	    c = *fmt++;

	    switch (c) {

	    case 'd':
	        n = 0;
		c = *input++;

		neg =  c == '-';
		if (neg) c = *input++;

		while (c >= '0' && c <= '9') {
		    n = n * 10 + (c - '0');
		    c = *input++;
		}
		input--;	/* retract lookahead */

		if (neg) n = -n;

		/* done, store it away */
		{
			int	*p = va_arg(vp, int *);
			*p = n;
		}

	        break;

	    default:
	        break;
	    }
	}

	va_end(vp);	

	return (vm_offset_t)input - (vm_offset_t)start;
}
