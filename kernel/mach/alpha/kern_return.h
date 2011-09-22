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
 * $Log:	kern_return.h,v $
 * Revision 2.2  93/01/14  17:40:55  danner
 * 	Created.
 * 	[91/12/29            af]
 * 
 */

/*
 *	File:	alpha/kern_return.h
 *	Author:	Alessandro Forin
 *	Date:	12/91
 *
 *	Machine-dependent kernel return definitions.
 *
 */

#ifndef	_MACH_ALPHA_KERN_RETURN_H_
#define	_MACH_ALPHA_KERN_RETURN_H_

#ifndef	ASSEMBLER
/*
 * Strictly speaking, this is should be a long (64 bits)
 * because the width of the return register is such.
 * However, (a) it does not need to and (b) it would
 * hurt MiG badly to do so.
 */
typedef	int		kern_return_t;
#endif	ASSEMBLER

#endif	_MACH_ALPHA_KERN_RETURN_H_
