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
 * Revision 2.6  93/01/14  17:45:21  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 
 * Revision 2.5  91/05/14  16:57:01  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/05/13  06:03:18  af
 * 	Added author note.
 * 	[91/05/12  15:54:58  af]
 * 
 * Revision 2.3.1.1  91/02/21  18:36:23  af
 * 	Added author note.
 * 
 * 
 * Revision 2.3  91/02/05  17:34:37  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:12:23  mrt]
 * 
 * Revision 2.2  89/11/29  14:09:41  af
 * 	Copied for pure kernel.
 * 	[89/10/28  09:56:20  af]
 * 
 * Revision 2.1  89/05/30  16:55:45  rvb
 * Created.
 * 
 *  3-Jan-89  Alessandro Forin (af) at Carnegie-Mellon University
 *	Created.
 */
/*
 *	File: exception.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	1/89
 *
 *	Codes and subcodes for Mips exceptions.
 */

#ifndef	_MACH_MIPS_EXCEPTION_H_
#define	_MACH_MIPS_EXCEPTION_H_

/*
 *	Hardware level exceptions
 */

#define	EXC_MIPS_INT		0	/* interrupt */
#define	EXC_MIPS_MOD		1	/* TLB mod */
#define	EXC_MIPS_RMISS		2	/* Read TLB Miss */
#define	EXC_MIPS_WMISS		3	/* Write TLB Miss */
#define	EXC_MIPS_RADE		4	/* Read Address Error */
#define	EXC_MIPS_WADE		5	/* Write Address Error */
#define	EXC_MIPS_IBE		6	/* Instruction Bus Error */
#define	EXC_MIPS_DBE		7	/* Data Bus Error */
#define	EXC_MIPS_SYSCALL	8	/* SYSCALL */
#define	EXC_MIPS_BREAK		9	/* BREAKpoint */
#define	EXC_MIPS_II		10	/* Illegal Instruction */
#define	EXC_MIPS_CPU		11	/* CoProcessor Unusable */
#define	EXC_MIPS_OV		12	/* OVerflow */

/*
 *	Software exception codes
 */
#define	EXC_MIPS_SOFT_SEGV	16	/* Software detected seg viol */
#define	EXC_MIPS_SOFT_CPU	19	/* coprocessor unusable */


/*
 *	Bad instruction subcodes
 */

#define	EXC_MIPS_PRIVINST		1
#define	EXC_MIPS_RESOPND		2
#define	EXC_MIPS_RESADDR		3

/*
 *	EXC_ARITHMETIC subcodes
 */

#define	EXC_MIPS_FLT_UNIMP	1
#define	EXC_MIPS_FLT_INVALID	2
#define	EXC_MIPS_FLT_DIVIDE0	3
#define	EXC_MIPS_FLT_OVERFLOW	4
#define	EXC_MIPS_FLT_UNDERFLOW	5
#define	EXC_MIPS_FLT_INEXACT	6

/*
 *	EXC_BREAKPOINT subcodes
 */

#define	EXC_MIPS_BPT			1
#define EXC_MIPS_TRACE			2


#endif	/* _MACH_MIPS_EXCEPTION_H_ */
