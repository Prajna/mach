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
 *  Software Distribution Coordinator   or   Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they made and grant Carnegie Mellon the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	fork.s,v $
 * Revision 2.3  93/05/10  21:33:28  rvb
 * 	Quiet if no -w.
 * 	[93/04/11            af]
 * 
 * Revision 2.2  93/01/14  18:02:53  danner
 * 	Created.
 * 	[92/05/31            af]
 * 
 */

#include <mach/alpha/asm.h>

#define SYS_fork	2

	.set	noat

SYSCALL(fork,0)
	beq	at,parent
	subq	sp,8,sp		/* need some temp stack space */
	stq	ra,0(sp)
				/* need GP setup to make funcalls */
	br	pv,1f
1:	ldgp	gp,0(pv)

	CALL(mach_init)

	ldq	ra,0(sp)
	addq	sp,8,sp		/* restore sp.. */
	mov	zero,v0
parent:
	RET			/* pid = fork() */

	END(fork)
