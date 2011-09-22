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
 * $Log:	bcopy.s,v $
 * Revision 2.6  93/02/04  07:55:39  danner
 * 	Convert asm comment "/" over to "/ *" "* /"
 * 	[93/01/27            rvb]
 * 
 * Revision 2.5  91/05/14  16:03:58  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:10:49  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:30:41  mrt]
 * 
 * Revision 2.3  90/11/05  14:26:58  rpd
 * 	Introduce bcopy16.  For 16bit copies to bus memory
 * 	[90/11/02            rvb]
 * 
 * Revision 2.2  90/05/03  15:24:53  dbg
 * 	From Bob Baron.
 * 	[90/04/30            dbg]
 * 
 */

#include <i386/asm.h>

/* bcopy(from, to, bcount) */

ENTRY(bcopy)
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%edi
	pushl	%esi
	movl	B_ARG0,%esi
	movl	B_ARG1,%edi
bcopy_common:
	movl	B_ARG2,%edx
	cld
/* move longs */
	movl	%edx,%ecx
	sarl	$2,%ecx
	rep
	movsl
/* move bytes */
	movl	%edx,%ecx
	andl	$3,%ecx
	rep
	movsb
	popl	%esi
	popl	%edi
	leave
	ret


/* memcpy(to, from, count) */

ENTRY(memcpy)
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%edi
	pushl	%esi
	movl	B_ARG0,%edi
	movl	B_ARG1,%esi
	jmp	bcopy_common

/* bcopy16(from, to, bcount) using word moves */

ENTRY(bcopy16)
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%edi
	pushl	%esi
/*	movl	8+12(%esp),%edx		/  8 for the two pushes above */
/*	movl	8+ 8(%esp),%edi */
/*	movl	8+ 4(%esp),%esi */
	movl	B_ARG2, %edx
	movl	B_ARG1, %edi
	movl	B_ARG0, %esi
/* move words */
0:	cld
	movl	%edx,%ecx
	sarl	$1,%ecx
	rep
	movsw
/* move bytes */
	movl	%edx,%ecx
	andl	$1,%ecx
	rep
	movsb
	popl	%esi
	popl	%edi
	leave
	ret	

