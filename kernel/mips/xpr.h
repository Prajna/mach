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
 * $Log:	xpr.h,v $
 * Revision 2.4  91/05/14  17:39:42  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:52:32  mrt
 * 	Added author notices
 * 	[91/02/04  11:25:50  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:30:38  mrt]
 * 
 * Revision 2.2  89/11/29  14:15:48  af
 * 	More flushes, adapted for pure kernel.
 * 	[89/10/29            af]
 * 
 */
/*
 *	File:	mips/xpr.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Machine dependent module for the XPR tracing facility.
 *
 */

#ifndef	_MIPS_XPR_H_
#define	_MIPS_XPR_H_

#define XPR_TIMESTAMP	(0)

/*
 * Mips specific trace flags.
 */
#define XPR_TTY		(1 << 16)	/* mux i/o */
#define XPR_BIO		(1 << 17)	/* blk i/o */


#endif	_MIPS_XPR_H_
