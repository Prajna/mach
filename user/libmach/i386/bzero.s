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
 * $Log:	bzero.s,v $
 * Revision 2.5  93/05/10  17:50:50  rvb
 * 	Use C Comment
 * 	[93/05/04  17:57:28  rvb]
 * 
 * Revision 2.4  91/05/14  17:52:29  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/14  14:17:19  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  14:16:26  mrt]
 * 
 * Revision 2.2  90/05/03  15:54:09  dbg
 * 	From Bob Baron.
 * 	[90/04/30            dbg]
 * 
 */

#include <i386/asm.h>

/*
 * bzero(char * addr, unsigned int length)
 */
Entry(blkclr)
ENTRY(bzero)
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%edi
	movl	B_ARG1,%edx
	movl	B_ARG0,%edi
	xorl	%eax,%eax
	cld
/* zero longs */
	movl	%edx,%ecx
	shrl	$2,%ecx
	rep
	stosl
/* zero bytes */
	movl	%edx,%ecx
	andl	$3,%ecx
	rep
	stosb
	popl	%edi
	leave
	ret
