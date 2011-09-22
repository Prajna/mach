/* 
 * Mach Operating System
 * Copyright (c) 1992,1991,1990,1989 Carnegie Mellon University
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
 * $Log:	mig_strncpy.c,v $
 * Revision 2.7  93/05/10  21:33:31  rvb
 * 	No size_t's.
 * 	[93/05/06  09:21:41  af]
 * 
 * Revision 2.6  93/05/10  17:51:00  rvb
 * 	Avoid size_t
 * 	[93/05/04  18:00:57  rvb]
 * 
 * Revision 2.5  93/01/14  18:03:46  danner
 * 	Fixed include of mig_support.h.
 * 	[92/12/14            pds]
 * 	Converted file to ANSI C.
 * 	[92/12/11            pds]
 * 
 * Revision 2.4  91/07/31  18:29:31  dbg
 * 	Changed to return the length of the string, including
 * 	the trailing 0.
 * 	[91/07/25            dbg]
 * 
 * Revision 2.3  91/05/14  17:53:46  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/14  14:18:02  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:44:34  mrt]
 * 
 * Revision 2.1  89/08/03  17:06:47  rwd
 * Created.
 * 
 * Revision 2.1  89/05/09  22:06:06  mrt
 * Created.
 * 
 */
/*
 * mig_strncpy.c - by Joshua Block
 *
 * mig_strncpy -- Bounded string copy.  Does what the library routine strncpy
 * OUGHT to do:  Copies the (null terminated) string in src into dest, a 
 * buffer of length len.  Assures that the copy is still null terminated
 * and doesn't overflow the buffer, truncating the copy if necessary.
 *
 * Parameters:
 * 
 *     dest - Pointer to destination buffer.
 * 
 *     src - Pointer to source string.
 * 
 *     len - Length of destination buffer.
 *
 * Result:
 *	length of string copied, INCLUDING the trailing 0.
 */

#include <mach/mig_support.h>

vm_size_t mig_strncpy(register char *dest, register const char *src,
		   register vm_size_t len)
{
    register vm_size_t i;

    if (len == 0)
	return 0;

    for (i=1; i<len; i++)
	if (! (*dest++ = *src++))
	    return i;

    *dest = '\0';
    return i;
}
