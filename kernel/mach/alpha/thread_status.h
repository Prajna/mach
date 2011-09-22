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
 * $Log:	thread_status.h,v $
 * Revision 2.2  93/01/14  17:41:08  danner
 * 	Revised for new calling sequence.
 * 	[92/06/07            af]
 * 
 * 	Created.
 * 	[91/12/29            af]
 * 
 */

/*
 *	File:	alpha/thread_status.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	12/91
 *
 *
 *	This file contains the structure definitions for the thread
 *	state as applicable to Alpha processors.
 *
 */

#ifndef	_MACH_ALPHA_THREAD_STATE_
#define	_MACH_ALPHA_THREAD_STATE_

#include <mach/machine/vm_types.h>

/*
 *	The structures defined in here are exported to users for
 *	use in status/mutate calls.
 *
 *	alpha_thread_state	basic machine state
 *
 *	alpha_float_state	state of floating point coprocessor
 *
 *	alpha_exc_state		exception state (fault address, etc.)
 */

#define	ALPHA_THREAD_STATE	(1)
#define ALPHA_FLOAT_STATE	(2)
#define ALPHA_EXC_STATE		(3)

struct alpha_thread_state {
	integer_t	r0;		/* v0:  return value */
	integer_t	r1;		/* t0:  caller saved 0 */
	integer_t	r2;		/* t1:  caller saved 1 */
	integer_t	r3;		/* t2:  caller saved 2 */
	integer_t	r4;		/* t3:  caller saved 3 */
	integer_t	r5;		/* t4:  caller saved 4 */
	integer_t	r6;		/* t5:  caller saved 5 */
	integer_t	r7;		/* t6:  caller saved 6 */
	integer_t	r8;		/* t7:  caller saved 7 */
	integer_t	r9;		/* s0:  callee saved 0 */
	integer_t	r10;		/* s1:  callee saved 1 */
	integer_t	r11;		/* s2:  callee saved 2 */
	integer_t	r12;		/* s3:  callee saved 3 */
	integer_t	r13;		/* s4:  callee saved 4 */
	integer_t	r14;		/* s5:  callee saved 5 */
	integer_t	r15;		/* s6:  callee saved 6 */
	integer_t	r16;		/* a0:  argument 0 */
	integer_t	r17;		/* a1:  argument 1 */
	integer_t	r18;		/* a2:  argument 2 */
	integer_t	r19;		/* a3:  argument 3 */
	integer_t	r20;		/* a4:  argument 4 */
	integer_t	r21;		/* a5:  argument 5 */
	integer_t	r22;		/* t8:  caller saved 8 */
	integer_t	r23;		/* t9:  caller saved 9 */
	integer_t	r24;		/* t10: caller saved 10 */
	integer_t	r25;		/* t11: caller saved 11 */
	integer_t	r26;		/* ra:  return address */
	integer_t	r27;		/* pv:  procedure value (caller saved) */
	integer_t	r28;		/* at:  assembler temporary */
	integer_t	r29;		/* gp:  procedure's data pointer */
	integer_t	r30;		/* sp:  stack pointer */
/*	integer_t	r31;		/* wired zero, not returned */
	integer_t	pc;		/* user-mode PC */
};

#define	ALPHA_THREAD_STATE_COUNT	(sizeof(struct alpha_thread_state)/sizeof(natural_t))


struct alpha_float_state {
	integer_t	r0;	/* 31 general registers + status */
	integer_t	r1;
	integer_t	r2;
	integer_t	r3;
	integer_t	r4;
	integer_t	r5;
	integer_t	r6;
	integer_t	r7;
	integer_t	r8;
	integer_t	r9;
	integer_t	r10;
	integer_t	r11;
	integer_t	r12;
	integer_t	r13;
	integer_t	r14;
	integer_t	r15;
	integer_t	r16;
	integer_t	r17;
	integer_t	r18;
	integer_t	r19;
	integer_t	r20;
	integer_t	r21;
	integer_t	r22;
	integer_t	r23;
	integer_t	r24;
	integer_t	r25;
	integer_t	r26;
	integer_t	r27;
	integer_t	r28;
	integer_t	r29;
	integer_t	r30;
/*	integer_t	r31;	wired zero */
	integer_t	csr;	/* status register */
};

#define	ALPHA_FLOAT_STATE_COUNT	(sizeof(struct alpha_float_state)/sizeof(natural_t))


struct alpha_exc_state {
	vm_offset_t	address;	/* last invalid virtual address */
	unsigned int	cause;		/* machine-level trap code */
#	define ALPHA_EXC_SET_SSTEP	1
	boolean_t	used_fpa;	/* did it ever use floats */
};

#define	ALPHA_EXC_STATE_COUNT	(sizeof(struct alpha_exc_state)/sizeof(natural_t))

#endif	_MACH_ALPHA_THREAD_STATE_
