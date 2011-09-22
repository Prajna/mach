/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 * $Log:	setjmp.h,v $
 * Revision 2.4  91/05/14  17:37:46  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:50:59  mrt
 * 	Added author notices
 * 	[91/02/04  11:24:43  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:29:02  mrt]
 * 
 * Revision 2.2  90/08/27  22:08:52  dbg
 * 	Created.
 * 	[90/07/18            dbg]
 */
/*
 *	File: setjmp.h
 * 	Author: David Golub, Carnegie Mellon University
 *	Date:	7/90
 *
 *	Common name for setjmp/longjmp buffer.
 */

#ifndef	_MIPS_SETJMP_H_
#define	_MIPS_SETJMP_H_

#include <mips/context.h>		/* It's defined here */
typedef jmp_buf	jmp_buf_t;		/* The expected name */

#endif	_MIPS_SETJMP_H_
