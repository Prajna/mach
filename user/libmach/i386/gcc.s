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
 * $Log:	gcc.s,v $
 * Revision 2.4  91/05/14  17:52:47  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/14  14:17:26  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  14:16:34  mrt]
 * 
 * Revision 2.2  90/05/03  15:54:17  dbg
 * 	Created.
 * 	[90/04/30  15:32:32  dbg]
 * 
 * 	Replacement for gnulib calls made from kernel.
 * 	[89/12/21  17:58:40  rvb]
 * 
 * 20-Dec-89  Robert Baron (rvb) at Carnegie-Mellon University
 *	Created.
 */

#include <i386/asm.h>

ENTRY(__divsi3)
	movl	4(%esp), %eax
	cdq
	idivl	8(%esp), %eax
	ret

ENTRY(__udivsi3)
	movl	4(%esp), %eax
	xorl	%edx, %edx
	divl	8(%esp), %eax
	ret

