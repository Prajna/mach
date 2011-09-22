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
 * $Log:	thread.h,v $
 * Revision 2.8  92/03/03  12:28:45  rpd
 * 	Removed pcb_synch, thread_start declarations.
 * 	Added syscall_emulation_sync, pcb_collect declarations.
 * 	[92/03/03            rpd]
 * 
 * Revision 2.7  91/05/14  17:38:57  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/03/16  14:57:27  rpd
 * 	Removed sr from mips_kernel_state.
 * 	[91/03/01            rpd]
 * 
 * 	Broke mips_float_state, mips_sstep_state out of mips_machine_state.
 * 	[91/02/17            rpd]
 * 
 * Revision 2.5  91/02/05  17:51:55  mrt
 * 	Added author notices
 * 	[91/02/04  11:25:13  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:29:59  mrt]
 * 
 * Revision 2.4  91/01/08  15:51:46  rpd
 * 	Added mips_stack_base.
 * 	[91/01/02            rpd]
 * 	Split mips_machine_state off of mips_kernel_state.
 * 	Changed mips_exception_state to mips_exception_link.
 * 	[90/12/30            rpd]
 * 	Added definition of MACHINE_STACK.
 * 	[90/12/14            rpd]
 * 	Added mips_exception_state, USER_LINK.
 * 	[90/11/13            rpd]
 * 
 * 	Changed pcb to include both mips_kernel_state and mips_saved_state.
 * 	[90/11/11            rpd]
 * 
 * Revision 2.3  90/08/27  22:08:57  dbg
 * 	Earthquake to make (new) kernel debugger more reentrant:
 * 	keep all trap info on stack.  Therefore reduced size of
 * 	mips_kernel_state and enlarged mips_saved_state.
 * 	[90/08/18            af]
 * 
 * Revision 2.2  89/11/29  14:15:27  af
 * 	Pure kernel defines (pure:-) pcb in here.
 * 	[89/10/04            af]
 * 
 * Revision 2.2  89/07/14  15:29:04  rvb
 * 	Rather than have "u" expand to current_thread()->u_address, have
 * 	it expand to the constant U_ADDRESS which is updated by load_context
 * 	when the thread changes.  If "u" is defined then user.h won't define
 * 	it.  U_ADDRESS is extern because when we are included by sys/thread.h
 * 	u_address has not been defined yet.
 * 	[89/07/05            rvb]
 * 
 * Revision 2.1  89/05/30  12:56:20  rvb
 * Created.
 * 
 *  3-Jan-89  Alessandro Forin (af) at Carnegie-Mellon University
 *	Created, no definitions yet.
 */
/*
 *	File:	thread.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	1/89
 *
 *	This file defines machine specific, thread related structures,
 *	variables and macros.
 *
*/

#ifndef	_MIPS_THREAD_H_
#define	_MIPS_THREAD_H_

#if	!defined(ASSEMBLER)

/*
 *	Kernel state.  Saved and restored across context-switches
 *	inside the kernel.  We can ignore caller-saved registers.
 *	Kept at the base of the thread's stack.
 */

struct mips_kernel_state {
	unsigned	s0;		/* callee-saved */
	unsigned	s1;
	unsigned	s2;
	unsigned	s3;
	unsigned	s4;
	unsigned	s5;
	unsigned	s6;
	unsigned	s7;
	unsigned	sp;		/* stack pointer */
	unsigned	fp;		/* frame pointer, unused */
#define		s8	fp	/* C compiler uses as callee-saved */
	unsigned	pc;		/* suspended program counter */
};

/*
 *	Machine state.  Includes all machine registers and other
 *	field used by machine-level code to provide in software
 *	things that architectures other than MIPS might provide
 *	in hardware, e.g. single-stepping.  The FPA state is scheduled
 *	asyncronously and saved here also, on demand.  Part of the pcb.
 *	We allocate space for this state as needed.
 */

struct mips_sstep_state {
	int	ss_count;	/* no. of breakpoints installed */
	struct	breakpoint {
		unsigned	*address;	/* where */
		unsigned	instruction;	/* original inst. */
	} ss_bp[2];		/* taken/nontaken sides of branch */
};

struct mips_machine_state {
	struct mips_float_state *mfs;	/* see mach/mips/thread_status.h */
	struct mips_sstep_state *msss;	/* single-stepping if present */
};

/*
 *	Saved state.  Holds the state of user registers upon kernel entry
 *	(saved in pcb) and kernel registers for exceptions in kernel mode
 *	(saved on kernel stack).
 */

struct mips_saved_state {
	unsigned	tlb_low;	/* TLB (partial) state, for KDB */
	unsigned	tlb_high;
	unsigned	tlb_index;
	unsigned	tlb_context;
	unsigned	at;		/* assembler temporary */
	unsigned	v0;		/* return value 0 */
	unsigned	v1;		/* return value 1 */
	unsigned	a0;		/* argument 0 */
	unsigned	a1;		/* argument 1 */
	unsigned	a2;		/* argument 2 */
	unsigned	a3;		/* argument 3 */
	unsigned	t0;		/* caller saved 0 */
	unsigned	t1;		/* caller saved 1 */
	unsigned	t2;		/* caller saved 2 */
	unsigned	t3;		/* caller saved 3 */
	unsigned	t4;		/* caller saved 4 */
	unsigned	t5;		/* caller saved 5 */
	unsigned	t6;		/* caller saved 6 */
	unsigned	t7;		/* caller saved 7 */
	unsigned	s0;		/* callee saved 0 */
	unsigned	s1;		/* callee saved 1 */
	unsigned	s2;		/* callee saved 2 */
	unsigned	s3;		/* callee saved 3 */
	unsigned	s4;		/* callee saved 4 */
	unsigned	s5;		/* callee saved 5 */
	unsigned	s6;		/* callee saved 6 */
	unsigned	s7;		/* callee saved 7 */
	unsigned	t8;		/* code generator 0 */
	unsigned	t9;		/* code generator 1 */
	unsigned	k0;		/* kernel temporary 0 */
	unsigned	k1;		/* kernel temporary 1 */
	unsigned	gp;		/* global pointer */
	unsigned	sp;		/* stack pointer */
	unsigned	fp;		/* frame pointer */
	unsigned	ra;		/* return address */
	unsigned	sr;		/* status register */
	unsigned	mdlo;		/* low mult result */
	unsigned	mdhi;		/* high mult result */
	unsigned	bad_address;	/* bad virtual address */
	unsigned	cause;		/* cause register */
	unsigned	pc;		/* program counter */
};

/*
 *	At the base of a kernel stack is an "exception link" record.
 *	It contains the C calling sequence's argument save area.
 *	It also contains a pointer to the exception frame (mips_saved_state).
 *	If the exception happened in user mode, then the exception frame
 *	is in the thread's pcb.  If the exception happed in kernel mode,
 *	then the exception frame is farther up the kernel stack.
 */
struct mips_exception_link {
	unsigned	arg0;		/* arg save for c calling seq */
	unsigned	arg1;		/* arg save for c calling seq */
	unsigned	arg2;		/* arg save for c calling seq */
	unsigned	arg3;		/* arg save for c calling seq */
	struct mips_saved_state *eframe;/* pointer to exception frame */
};

/*
 *	Lives at the base of a kernel stack.
 *	The full arrangement is
 *	stack:	...
 *		struct mips_exception_link
 *		struct mips_kernel_state
 *		struct mips_stack_base
 *	stack+KERNEL_STACK_SIZE:
 */
struct mips_stack_base {
	vm_offset_t	next;		/* next stack on free list */
	struct vm_page	*page;		/* page structure for this stack */
};

typedef struct pcb {
	struct mips_saved_state mss;
	struct mips_machine_state mms;
} *pcb_t;	/* exported */

#define	USER_REGS(th)	(&(th)->pcb->mss)

#define STACK_MSB(stack)	\
	((struct mips_stack_base *)((stack) + KERNEL_STACK_SIZE) - 1)
#define STACK_MKS(stack)	\
	((struct mips_kernel_state *)STACK_MSB(stack) - 1)
#define STACK_MEL(stack)	\
	((struct mips_exception_link *)STACK_MKS(stack) - 1)

/*
 *	Routine definitions
 */
#include <mach/kern_return.h>

void		pcb_init(), pcb_terminate(), pcb_collect();
kern_return_t	thread_setstatus(), thread_getstatus();
void		syscall_emulation_sync();

#endif	!defined(ASSEMBLER)

/*
 *	Unlike Vaxen et al. we don't really have special scratch
 *	registers that we can use, so we go the canonical way..
 */
#ifdef	ASSEMBLER
#define	the_current_thread	active_threads
#endif	ASSEMBLER

/*
 *	We have our own mips-specific implementations of
 *	stack_alloc_try/stack_alloc/stack_free/stack_statistics.
 */
#define	MACHINE_STACK

#endif	_MIPS_THREAD_H_
