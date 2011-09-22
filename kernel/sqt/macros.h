/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * $Log:	macros.h,v $
 * Revision 2.3  91/07/31  18:02:19  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  12:57:21  dbg
 * 	Created.
 * 	[91/04/26  14:53:43  dbg]
 * 
 */

/*
 * Commonly used macros.
 */
#ifndef	_SQT_MACROS_H_
#define	_SQT_MACROS_H_

#define	MIN(x,y)	( ((x) < (y)) ? (x) : (y) )
#define	MAX(x,y)	( ((x) > (y)) ? (x) : (y) )

#define	howmany(x,y)	(((x) + (y) - 1)/(y))
#define	roundup(x,y)	(howmany(x,y)*(y))

#endif	_SQT_MACROS_H_
