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
 * $Log:	csw.s,v $
 * Revision 2.7  93/01/14  18:05:44  danner
 * 	Fixes for ANSI CPP.
 * 	[92/08/22            jvh]
 * 
 * Revision 2.6  91/05/14  17:57:51  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/14  14:20:35  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:38:39  mrt]
 * 
 * Revision 2.4  90/06/02  15:14:09  rpd
 * 	Added definition of cthread_sp.
 * 	[90/04/24            rpd]
 * 
 * Revision 2.3  89/12/08  19:49:17  rwd
 * 	Changes for new cthreads from af
 * 	[89/12/06            rwd]
 * 
 * Revision 2.2  89/11/29  14:18:59  af
 * 	Created.
 * 	[89/07/06            af]
 * 
 */
/*
 * pmax/csw.s
 *
 * Context switch and cproc startup for MIPS COROUTINE implementation.
 */
#include <mach/mips/asm.h>

	.text
	.align	2

#define ARG_SAVE	(4*4)
#define SAVED_S0	(4*4)
#define SAVED_S1	(5*4)
#define SAVED_S2	(6*4)
#define SAVED_S3	(7*4)
#define SAVED_S4	(8*4)
#define SAVED_S5	(9*4)
#define SAVED_S6	(10*4)
#define SAVED_S7	(11*4)
#define SAVED_FP	(12*4)
#define SAVED_PC	(13*4)
#define SAVED_BYTES	(14*4)

/*
 * Suspend the current thread and resume the next one.
 *
 *	void
 *	cproc_switch(cur, next, lock)
 *		int *cur;
 *		int *next;
 *		simple_lock *lock;
 */
LEAF(cproc_switch)
	subu	sp,sp,SAVED_BYTES	/* allocate space for 10 registers */
					/* Save them registers */
	sw	ra,SAVED_PC(sp)
	sw	fp,SAVED_FP(sp)
	sw	s0,SAVED_S0(sp)
	sw	s1,SAVED_S1(sp)
	sw	s2,SAVED_S2(sp)
	sw	s3,SAVED_S3(sp)
	sw	s4,SAVED_S4(sp)
	sw	s5,SAVED_S5(sp)
	sw	s6,SAVED_S6(sp)
	sw	s7,SAVED_S7(sp)

	sw	sp,0(a0)		/* save current sp */
	lw	sp,0(a1)		/* restore next sp */
					/* Reload them registers */
	lw	ra,SAVED_PC(sp)
	lw	fp,SAVED_FP(sp)
	lw	s0,SAVED_S0(sp)
	lw	s1,SAVED_S1(sp)
	lw	s2,SAVED_S2(sp)
	lw	s3,SAVED_S3(sp)
	lw	s4,SAVED_S4(sp)
	lw	s5,SAVED_S5(sp)
	lw	s6,SAVED_S6(sp)
	lw	s7,SAVED_S7(sp)
					/* return to next thread */
	sw	zero,0(a2)		/* clear lock */
	addu	sp,sp,SAVED_BYTES
	j	ra
END(cproc_switch)

/*
 *	void
 *	cproc_start_wait(parent_context, child, stackp, lock)
 *		int *parent_context;
 *		cproc_t child;
 *		int stackp;
 *		simple_lock *lock;
 */
NESTED(cproc_start_wait, SAVED_BYTES, zero)
	subu	sp,sp,SAVED_BYTES	/* allocate space for 10 registers */
					/* Save parent registers */
	sw	ra,SAVED_PC(sp)
	sw	fp,SAVED_FP(sp)
	sw	s0,SAVED_S0(sp)
	sw	s1,SAVED_S1(sp)
	sw	s2,SAVED_S2(sp)
	sw	s3,SAVED_S3(sp)
	sw	s4,SAVED_S4(sp)
	sw	s5,SAVED_S5(sp)
	sw	s6,SAVED_S6(sp)
	sw	s7,SAVED_S7(sp)

	sw	sp,0(a0)		/* save parent sp */
	sw	zero,0(a3)		/* release lock */
	move	sp,a2			/* get child sp */
	subu	sp,sp,4*4		/* standard regsave */
	move	a0,a1
	jal	cproc_waiting		/* cproc_waiting(child) */
	/*
	 * Control never returns here.
	 */
	END(cproc_start_wait)

/*
 *	void
 *	cproc_prepare(child, child_context, stack)
 *		int	*child_context;
 *		int	*stack;
 */
LEAF(cproc_prepare)
	subu	a2,ARG_SAVE	/* cthread_body's fake frame */
	sw	a0,0(a2)	/* cthread_body(child) */
	subu	a2,SAVED_BYTES	/* cproc_switch's ``frame'' */
	sw	s0,SAVED_S0(a2)
	sw	s1,SAVED_S1(a2)
	sw	s2,SAVED_S2(a2)
	sw	s3,SAVED_S3(a2)
	sw	s4,SAVED_S4(a2)
	sw	s5,SAVED_S5(a2)
	sw	s6,SAVED_S6(a2)
	sw	s7,SAVED_S7(a2)
	sw	fp,SAVED_FP(a2)
	sw	a2,0(a1)	/* child context */
	la	v0,1f
	sw	v0,SAVED_PC(a2)
	j	ra

	/*
	 *	The reason we are getting here is to load
	 *	arguments in registers where they are supposed
	 *	to be.  The code above only put the argument(s)
	 *	on the stack, now we'll load them.
	 */
1:	la	v0,cthread_body
	lw	a0,0(sp)
	j	v0
	END(cproc_prepare)

/*
 *	int
 *	cthread_sp()
 *
 *	Returns the current stack pointer.
 */

LEAF(cthread_sp)
	move	v0, sp
	j	ra
	END(cthread_sp);
