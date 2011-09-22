/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
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
 * $Log:	memcpy.s,v $
 * Revision 2.4  93/05/10  17:50:58  rvb
 * 	Use C Comment
 * 	[93/05/04  17:57:33  rvb]
 * 
 * Revision 2.3  93/01/24  13:23:47  danner
 * 	Returned a pointer to the destination memory, to conform
 * 	to the specification of memcpy.
 * 	[92/10/15            mrt]
 * 
 * Revision 2.2  92/02/19  16:02:14  elf
 * 	Created.
 * 
 */

#include <i386/asm.h>

/* memcpy(to, from, bcount) */

ENTRY(memcpy)
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%edi
	pushl	%esi
	movl	B_ARG0,%edi
	movl	B_ARG1,%esi
	movl	B_ARG2,%edx
	cld			/* move longs */
	movl	%edx,%ecx
	sarl	$2,%ecx
	rep
	movsl
	movl	%edx,%ecx	/* move bytes */
	andl	$3,%ecx
	rep
	movsb
        movl    B_ARG0,%eax
	popl	%esi
	popl	%edi
	leave
	ret
