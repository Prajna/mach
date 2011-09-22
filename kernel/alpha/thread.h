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
 * $Log:	thread.h,v $
 * Revision 2.4  93/03/09  10:51:09  danner
 * 	There was no indirection bug, thread->pcb truly is a pointer.
 * 	[93/03/05            af]
 * 
 * Revision 2.3  93/02/04  07:55:19  danner
 * 	Missing indirection in user_regs declaration.
 * 	[93/02/02            danner]
 * 
 * Revision 2.2  93/01/14  17:14:28  danner
 * 	Created, from mips version.
 * 	[92/05/31            af]
 * 
 */
/*
 *	File:	thread.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	5/92
 *
 *	This file defines machine specific, thread related structures,
 *	variables and macros.
 *
*/

#ifndef	_ALPHA_THREAD_H_
#define	_ALPHA_THREAD_H_

#if	!defined(ASSEMBLER)

#include <mach/alpha/alpha_instruction.h>
#include <alpha/context.h>
#include <alpha/frame.h>

/*
 *	Kernel state.  Saved and restored across context-switches
 *	inside the kernel.  We can ignore caller-saved registers.
 *	Kept at the base of the thread's stack.
 */

struct alpha_kernel_state {
	vm_offset_t	s0;		/* callee-saved */
	vm_offset_t	s1;
	vm_offset_t	s2;
	vm_offset_t	s3;
	vm_offset_t	s4;
	vm_offset_t	s5;
	vm_offset_t	s6;
	vm_offset_t	sp;		/* stack  pointer */
	vm_offset_t	pc;		/* suspended program counter */
};

/*
 *	Machine state.  Includes all machine registers and other
 *	field used by machine-level code to provide in software
 *	things that architectures other than ALPHA might provide
 *	in hardware, e.g. single-stepping.  The FPA state is scheduled
 *	asyncronously and saved here also, on demand.  Part of the pcb.
 *	We allocate space for this state as needed.
 */

struct alpha_sstep_state {
	int	ss_count;	/* no. of breakpoints installed */
	struct	breakpoint {
		vm_offset_t	address;	/* where */
		alpha_instruction instruction;	/* original inst. */
	} ss_bp[2];		/* taken/nontaken sides of branch */
};

struct alpha_machine_state {
	struct alpha_float_state *mfs;	/* see mach/alpha/thread_status.h */
	struct alpha_sstep_state *msss;	/* single-stepping if present */
};

/*
 *	Saved state.  Holds the state of user registers upon kernel entry
 *	(saved in pcb) and kernel registers for exceptions in kernel mode
 *	(saved on kernel stack).
 */

/* REVISE, BASED ON ACTUAL USE (best WB/CACHE behaviour) */
struct alpha_saved_state {
	struct hw_pcb	hw_pcb;		/* with usp */
/* wline */
	struct trap_frame
			*framep;	/* t1-t6, pc, ps */
	vm_offset_t	gp;		/* global pointer */
	vm_offset_t	a0;		/* argument 0 */
	vm_offset_t	a1;		/* argument 1 */
/* wline */
	vm_offset_t	a2;		/* argument 2 */
	vm_offset_t	a3;		/* argument 3 */
	vm_offset_t	a4;		/* argument 4 */
	vm_offset_t	a5;		/* argument 5 */
/* wline */
	vm_offset_t	ra;		/* return address */
	vm_offset_t	v0;		/* return value 0 */
	vm_offset_t	t0;		/* caller saved 0 */
	vm_offset_t	t7;		/* caller saved 7 */
/* wline */
	vm_offset_t	t8;		/* caller saved 8 */
	vm_offset_t	t9;		/* caller saved 9 */
	vm_offset_t	t10;		/* caller saved 10 */
	vm_offset_t	t11;		/* caller saved 11 */
/* wline */
	vm_offset_t	t12;		/* caller saved 12 */
	vm_offset_t	s0;		/* callee saved 0 */
	vm_offset_t	s1;		/* callee saved 1 */
	vm_offset_t	s2;		/* callee saved 2 */
/* wline */
	vm_offset_t	s3;		/* callee saved 3 */
	vm_offset_t	s4;		/* callee saved 4 */
	vm_offset_t	s5;		/* callee saved 5 */
	vm_offset_t	s6;		/* callee saved 6 */
/* wline */
	vm_offset_t	at;		/* assembler temporary */
	vm_offset_t	sp;		/* stack pointer (if kernel) */
	vm_offset_t	bad_address;	/* bad virtual address */
	vm_offset_t	cause;		/* trap cause */

	struct trap_frame
			saved_frame;	/* t1-t6, pc, ps */
};

/*
 *	At the base of a kernel stack is an "exception link" record.
 *	It contains the C calling sequence's argument save area.
 *	It also contains a pointer to the exception frame (alpha_saved_state).
 *	If the exception happened in user mode, then the exception frame
 *	is in the thread's pcb.  If the exception happed in kernel mode,
 *	then the exception frame is further up the kernel stack.
 */
struct alpha_exception_link {
	struct alpha_saved_state *eframe;/* pointer to exception frame */
	struct trap_frame	tf;	/* HW saves regs here, and pc+ps */
};

/*
 *	Lives at the base of a kernel stack.
 *	The full arrangement is
 *	stack:	...
 *		struct alpha_exception_link
 *		struct alpha_kernel_state
 *		struct alpha_stack_base
 *	stack+KERNEL_STACK_SIZE:
 */
typedef struct pcb {
	struct alpha_saved_state	mss;	/* includes hw_pcb, first! */
	struct alpha_machine_state	mms;
	/* roundup, cuz HW wants it 128-byte aligned */
	char	pad[ 512 -
			(sizeof(struct alpha_saved_state) +
			 sizeof(struct alpha_machine_state)) ];
} *pcb_t;	/* exported */

struct alpha_stack_base {
	vm_offset_t	next;		/* next stack on free list */
	struct vm_page	*page;		/* page structure for this stack */
	pcb_t		pcb;		/* pointer to our pcb */
					/* align, cuz trap_frame will */
	char		pad[64-sizeof(vm_offset_t)-sizeof(struct vm_page*)-sizeof(pcb_t)];
};

#define	USER_REGS(th)	((th)->pcb)

#define STACK_MSB(stack)	\
	((struct alpha_stack_base *)((stack) + KERNEL_STACK_SIZE) - 1)
#define STACK_MEL(stack)	\
	((struct alpha_exception_link *)STACK_MSB(stack) - 1)
#define STACK_MKS(stack)	\
	((struct alpha_kernel_state *)STACK_MEL(stack) - 1)

/*
 *	Routine definitions
 */
#include <mach/kern_return.h>

void		pcb_init(), pcb_terminate(), pcb_collect();
kern_return_t	thread_setstatus(), thread_getstatus();
void		syscall_emulation_sync();

#endif	!defined(ASSEMBLER)

/*
 *	Later on..
 */
#if 0
#define	current_thread()	mfpr_....
#endif

/*
 *	We have our own alpha-specific implementations of
 *	stack_alloc_try/stack_alloc/stack_free/stack_statistics.
 */
#define	MACHINE_STACK

#endif	_ALPHA_THREAD_H_
