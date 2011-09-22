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
 * $Log:	_setjmp.s,v $
 * Revision 2.5  93/02/04  07:55:31  danner
 * 	Convert asm comment "/" over to "/ *" "* /"
 * 	[93/01/27            rvb]
 * 
 * Revision 2.4  91/05/14  16:02:32  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:10:37  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:30:23  mrt]
 * 
 * Revision 2.2  90/05/03  15:21:17  dbg
 * 	First checkin.
 * 
 * Revision 1.3  89/11/30  19:52:47  kupfer
 * Changes for Tahoe and a.out.
 * 
 * Revision 1.2  89/11/16  21:22:02  kupfer
 * longjmp should reset the floating point coprocessor.
 */

/*
 * C library -- _setjmp, _longjmp
 *
 *	_longjmp(a,v)
 * will generate a "return(v)" from
 * the last call to
 *	_setjmp(a)
 * by restoring registers from the stack,
 * The previous signal state is NOT restored.
 *
 */

#include <i386/asm.h>

ENTRY(_setjmp)
	movl	4(%esp),%ecx		/* fetch buffer */
	movl	%ebx,0(%ecx)
	movl	%esi,4(%ecx)
	movl	%edi,8(%ecx)
	movl	%ebp,12(%ecx)		/* save frame pointer of caller */
	popl	%edx
	movl	%esp,16(%ecx)		/* save stack pointer of caller */
	movl	%edx,20(%ecx)		/* save pc of caller */
	xorl	%eax,%eax
        jmp     *%edx

ENTRY(_longjmp)
	movl	8(%esp),%eax		/* return(v) */
	movl	4(%esp),%ecx		/* fetch buffer */
	movl	0(%ecx),%ebx
	movl	4(%ecx),%esi
	movl	8(%ecx),%edi
	movl	12(%ecx),%ebp
	movl	16(%ecx),%esp
	orl	%eax,%eax
	jnz	0f
	incl	%eax
0:	jmp	*20(%ecx)		/* done, return.... */
