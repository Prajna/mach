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
 * $Log:	exception.h,v $
 * Revision 2.2  93/01/14  17:40:51  danner
 * 	Created.
 * 	[91/12/29            af]
 * 
 */
/*
 *	File: alpha/exception.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	12/91
 *
 *	Codes and subcodes for Alpha exceptions.
 */

#ifndef	_MACH_ALPHA_EXCEPTION_H_
#define	_MACH_ALPHA_EXCEPTION_H_

/*
 *	Hardware level exceptions
 */


/*
 *	Software exception codes
 */


/*
 *	Bad instruction subcodes
 */

#define	EXC_ALPHA_PRIVINST		1
#define	EXC_ALPHA_RESOPND		2
#define	EXC_ALPHA_RESADDR		3

/*
 *	EXC_ARITHMETIC subcodes
 *
 *	NOTE: This is incompatible with OSF1's definitions.
 *	      The reason is that more than one exception might
 *	      be reported at once, so we want to OR the bits.
 *
 *	The subcode argument is the "register write mask".
 */
#define EXC_ALPHA_FLT_COMPLETE		0x01
#define EXC_ALPHA_FLT_INVALID		0x02
#define EXC_ALPHA_FLT_DIVIDE0		0x04
#define EXC_ALPHA_FLT_FOVERFLOW		0x08
#define EXC_ALPHA_FLT_UNDERFLOW		0x10
#define EXC_ALPHA_FLT_INEXACT		0x20
#define EXC_ALPHA_FLT_IOVERFLOW		0x40

/*
 *	EXC_BREAKPOINT subcodes
 */

#define	EXC_BREAK_BPT			0
#define	EXC_BREAK_SSTEP			1


#endif	_MACH_ALPHA_EXCEPTION_H_
