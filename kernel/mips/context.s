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
 * $Log:	context.s,v $
 * Revision 2.11  92/02/19  15:08:46  elf
 * 	Changed #-style comments, for ANSI cpp.
 * 	[92/02/19  13:10:55  rpd]
 * 
 * Revision 2.10  91/05/14  17:33:11  mrt
 * 	Correcting copyright
 * 
 * Revision 2.9  91/03/16  14:55:20  rpd
 * 	Replaced stack_switch with stack_handoff
 * 	and switch_{task,thread}_context with Switch_context.
 * 	[91/02/17            rpd]
 * 	Moved stack_switch here again.
 * 	Added active_stacks.
 * 	[91/01/28            rpd]
 * 
 * Revision 2.8  91/02/05  17:47:39  mrt
 * 	Added author notices
 * 	[91/02/04  11:21:38  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:25:11  mrt]
 * 
 * Revision 2.7  91/01/08  15:49:15  rpd
 * 	Added mips_stack_base.
 * 	[91/01/02            rpd]
 * 
 * 	Split mips_machine_state off of mips_kernel_state.
 * 	Moved stack_switch to mips/pcb.c.
 * 	[90/12/30            rpd]
 * 	Removed load_context_ipc, save_context.
 * 	Added switch_task_context, switch_thread_context.
 * 	[90/12/08            rpd]
 * 
 * 	Added stack_switch, load_context_ipc.
 * 	[90/11/29            rpd]
 * 
 * 	Changed save_context/load_context for pcb reorganization.
 * 	Removed stack wiring code from load_context.
 * 	Disabled pmap_pcache code.
 * 	[90/11/12            rpd]
 * 
 * Revision 2.4  90/08/07  22:29:08  rpd
 * 	Support for pmap_pcache.
 * 	[90/08/07  15:25:14  af]
 * 
 * Revision 2.3  90/01/22  23:06:19  af
 * 	Added optional return value to longjmp(), as needed by KDB.
 * 	Which is the only user, btw.
 * 	[90/01/20  17:04:35  af]
 * 
 * Revision 2.2  89/11/29  14:12:54  af
 * 	Forgot to preserve a1 across the pmap_set_modify() call.
 * 	How comes the im-pure kernel never stomped into this ?
 * 	[89/11/27            af]
 * 
 * 	Support emulated syscalls by caching the dispatch pointer.
 * 	Support scheduling of the FPA by exporting the pcb pointer.
 * 	[89/11/16  14:42:52  af]
 * 
 * 	Kernel stack grew to 2 pages. Sigh.
 * 	[89/11/03  16:36:51  af]
 * 
 * 	Pure kernel: kernel stack reduced to 1 page.
 * 	U_ADDRESS hack is gone.
 * 	[89/10/29            af]
 * 
 * Revision 2.7  89/09/22  13:56:30  af
 * 	Now that the EAGER_PG_M thing is gone from pmap.c, need to make
 * 	sure kernel stack pages are writable. This is peculiar to a soft
 * 	managed tlb like ours, as we need a writable stack to take tlbmod
 * 	exceptions in software.
 * 	[89/09/01  09:52:12  af]
 * 
 * Revision 2.6  89/09/09  16:23:02  rvb
 * 	Have load_context make kernel stack writable, or we are in deep
 * 	trouble [af].
 * 	[89/09/09            rvb]
 * 
 * Revision 2.5  89/08/28  22:38:42  af
 * 	Polished and optimized. Moved probe_and_wire in tlb module where
 * 	it belongs. Made thread's PCB pointer a global, so that we do not
 * 	need the third wired entry anymore. Documented functions
 * 	and made comments (possibly) more useful.
 * 	[89/08/06            af]
 * 
 * Revision 2.4  89/08/08  21:48:41  jsb
 * 	Unbelievable bug in load_context(), which always restored
 * 	a status register of 1. Hard to believe, but the kernel still
 * 	worked.	Cleanups.
 * 	[89/08/02            af]
 * 
 * Revision 2.3  89/07/14  15:27:43  rvb
 * 	Call load_context_data for U_ADDRESS hack.
 * 	[89/07/05            rvb]
 * 
 * Revision 2.2  89/05/31  12:29:03  rvb
 * 	Changes. [af]
 * 
 * Revision 2.1  89/05/30  12:55:30  rvb
 * Under source control.
 * 
 * 13-Jan-89  Alessandro Forin (af) at Carnegie-Mellon University
 *	Created.
 *
 */
/* 
 *	File: context.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	1/89
 *
 *	Context switching and context save/restore primitives
 *
 */


#include <mach/mips/asm.h>

#include <mips/mips_cpu.h>
#include <mach/mips/vm_param.h>
#include <mips/pmap.h>
#include <mips/thread.h>
#include <mips/context.h>

#include <assym.s>

	.set	noreorder		/* unless overridden */

/*
 *	Object:
 *		active_threads			IMPORTED variable
 *		active_stacks			IMPORTED variable
 *		current_kstack			EXPORTED variable
 *		current_dispatch		EXPORTED variable
 *		current_pcb			EXPORTED variable
 *
 *	- Pointer to current thread
 *	- Pointer to current kernel stack
 *	- Pointer to top of kernel stack for current thread
 *	- Pointer to emulation dispatch structure for same
 *	- Pointer to PCB for current thread
 */
BSS(active_threads,4)
BSS(active_stacks,4)
BSS(current_kstack,4)
BSS(current_dispatch,4)
BSS(current_pcb,4)

/*
 *	Object:
 *		load_context			EXPORTED function
 *
 *	 	Load the state of the target thread
 *
 *	Arguments:
 *		a0				thread_t
 *
 *	Installs this as the current thread on all the various
 *	globals: active_threads, current_kstack, current_pcb.
 *	Used to start the first thread on a CPU.
 *	Assumes splsched.
 */
LEAF(load_context)
	/*
	 * There are various possible accidents we must prevent here.
	 * Basically we might screw up by taking a tlb miss when we
	 * are not ready, or by not setting things up right so that
	 * going into user mode and back fails.
	 *
	 * The first is only really a problem when inside this function.
	 * It is avoided by collectiong all the informations we
	 * need in registers when still in a state that allows
	 * tlb misses, and then entering a small critical section where
	 * misses are NOT allowed. CAVEAT MAINTAINER!
	 *
	 * The second requires that the thread's kernel stack and pcb
	 * be mapped at all times, as we need to save the user's
	 * registers before we clobber them.
	 * As an optimization, the current pcb pointer is held in a
	 * local variable to reduce tlb misses.
	 * A further optimization is to keep the kernel stack pointer
	 * of the current thread in a global variable so that we can
	 * get it quickly when we switch back to kernel mode.
	 */

	/*
	 *	Part 1: Load registers
	 */
	lw	s0,THREAD_KERNEL_STACK(a0)	/* get kernel stack, might miss */
	lw	s1,THREAD_PCB(a0)		/* PCB pointer, might miss */

	/*
	 *	Part 2: Critical section
	 *
	 *	Setup important global variables.
	 *	Relevant globals are bracketed in comments
	 *
	 *	>>> TLB MISSES NOT ALLOWED HERE <<<
	 */

	sw	a0,the_current_thread	/* >>active_threads<< */

	sw	s0,active_stacks	/* >>active_stacks<< */
	addu	s0,KERNEL_STACK_SIZE-MSB_SIZE-MKS_SIZE-MEL_SIZE
	sw	s0,current_kstack	/* >>current_kstack<< */

	lw	sp,MEL_SIZE+MKS_SP(s0)	/* get new sp value */
	sw	s1,current_pcb		/* cache >>pcb pointer<< */

	/*
	 *	>> END OF CRITICAL SECTION <<
	 *
	 *	We can take misses again now, but we're not done yet.
	 */
					/* cache emulation table */
	lw	a2,THREAD_TASK(a0)	/* thread->task->dispatch */
	nop
	lw	a2,EML_DISPATCH(a2)	/* up to 2 misses here. Sigh. */
	nop
	sw	a2,current_dispatch	/* >>current_dispatch<< */

	/*
	 *	Part 3: Reload thread's registers and return
	 */
	lw	v1,MEL_SIZE+MKS_PC(s0)
	lw	fp,MEL_SIZE+MKS_FP(s0)
	lw	s1,MEL_SIZE+MKS_S1(s0)
	lw	s2,MEL_SIZE+MKS_S2(s0)
	lw	s3,MEL_SIZE+MKS_S3(s0)
	lw	s4,MEL_SIZE+MKS_S4(s0)
	lw	s5,MEL_SIZE+MKS_S5(s0)
	lw	s6,MEL_SIZE+MKS_S6(s0)
	lw	s7,MEL_SIZE+MKS_S7(s0)
	lw	s0,MEL_SIZE+MKS_S0(s0)
	move	a0,zero		/* return zero to thread_continue */
	j	v1
	move	ra,zero		/* for backtraces */
	END(load_context)

/*
 *	Object:
 *		Switch_context			EXPORTED function
 *
 *	 	Save state of the current thread,
 *		and resume new thread,
 *		returning the current thread.
 *
 *	Arguments:
 *		a0				old thread_t
 *		a1				function
 *		a2				new thread_t
 *
 *	Installs this as the current thread on all the various
 *	globals: active_threads, current_kstack, current_pcb.
 *	Assumes interrupts are disabled.
 */
LEAF(Switch_context)
	/*
	 * There are various possible accidents we must prevent here.
	 * Basically we might screw up by taking a tlb miss when we
	 * are not ready, or by not setting things up right so that
	 * going into user mode and back fails.
	 *
	 * The first is only really a problem when inside this function.
	 * It is avoided by collectiong all the informations we
	 * need in registers when still in a state that allows
	 * tlb misses, and then entering a small critical section where
	 * misses are NOT allowed. CAVEAT MAINTAINER!
	 *
	 * The second requires that the thread's kernel stack and pcb
	 * be mapped at all times, as we need to save the user's
	 * registers before we clobber them.
	 * As an optimization, the current pcb pointer is held in a
	 * local variable to reduce tlb misses.
	 * A further optimization is to keep the kernel stack pointer
	 * of the current thread in a global variable so that we can
	 * get it quickly when we switch back to kernel mode.
	 */

	lw	t0,active_stacks	/* get old kernel stack */
	sw	a1,THREAD_SWAP_FUNC(a0)
	bne	a1,zero,1f
	sw	t0,THREAD_KERNEL_STACK(a0)

	/*
	 *	Part 1: Save context
	 *
	 * 	We only need to save those registers that are callee-saved
	 *	in C, everything else is already on the stack.
	 *	We don't need to do this if there is an explicit continuation.
	 */
	sw	sp,KERNEL_STACK_SIZE-MSB_SIZE-MKS_SIZE+MKS_SP(t0)
	sw	ra,KERNEL_STACK_SIZE-MSB_SIZE-MKS_SIZE+MKS_PC(t0)

	sw	s0,KERNEL_STACK_SIZE-MSB_SIZE-MKS_SIZE+MKS_S0(t0)
	sw	s1,KERNEL_STACK_SIZE-MSB_SIZE-MKS_SIZE+MKS_S1(t0)
	sw	s2,KERNEL_STACK_SIZE-MSB_SIZE-MKS_SIZE+MKS_S2(t0)
	sw	s3,KERNEL_STACK_SIZE-MSB_SIZE-MKS_SIZE+MKS_S3(t0)
	sw	s4,KERNEL_STACK_SIZE-MSB_SIZE-MKS_SIZE+MKS_S4(t0)
	sw	s5,KERNEL_STACK_SIZE-MSB_SIZE-MKS_SIZE+MKS_S5(t0)
	sw	s6,KERNEL_STACK_SIZE-MSB_SIZE-MKS_SIZE+MKS_S6(t0)
	sw	s7,KERNEL_STACK_SIZE-MSB_SIZE-MKS_SIZE+MKS_S7(t0)
	sw	fp,KERNEL_STACK_SIZE-MSB_SIZE-MKS_SIZE+MKS_FP(t0)
1:
	lw	t0,THREAD_KERNEL_STACK(a2)	/* get new kernel stack */
	lw	t1,THREAD_PCB(a2)		/* get new thread's PCB */

	/*
	 *	Part 2: Critical section
	 *
	 *	Setup important global variables.
	 *	Relevant globals are bracketed in comments
	 *
	 *	>>> TLB MISSES NOT ALLOWED HERE <<<
	 */

	sw	a2,the_current_thread	/* >>active_threads<< */
	sw	t1,current_pcb		/* >>current_pcb<< */

	sw	t0,active_stacks	/* >>active_stacks<< */
	addu	t0,KERNEL_STACK_SIZE-MSB_SIZE-MKS_SIZE-MEL_SIZE
	sw	t0,current_kstack	/* >>current_kstack<< */

	lw	sp,MEL_SIZE+MKS_SP(t0)	/* get new sp value */

	/*
	 *	>> END OF CRITICAL SECTION <<
	 *
	 *	We can take misses again now, but we're not done yet.
	 *	Part 4: Reload thread's registers and return
	 */
	lw	t1,THREAD_TASK(a2)
	lw	v1,MEL_SIZE+MKS_PC(t0)
	lw	t1,EML_DISPATCH(t1)
	lw	fp,MEL_SIZE+MKS_FP(t0)
	sw	t1,current_dispatch	/* >>current_dispatch<< */
	lw	s1,MEL_SIZE+MKS_S1(t0)
	lw	s2,MEL_SIZE+MKS_S2(t0)
	lw	s3,MEL_SIZE+MKS_S3(t0)
	lw	s4,MEL_SIZE+MKS_S4(t0)
	lw	s5,MEL_SIZE+MKS_S5(t0)
	lw	s6,MEL_SIZE+MKS_S6(t0)
	lw	s7,MEL_SIZE+MKS_S7(t0)
	lw	s0,MEL_SIZE+MKS_S0(t0)
	move	v0,a0
	j	v1
	move	ra,zero			/* for backtraces */
	/*
	 *	We return the old thread in v0 for switch_context
	 *	and a0 for thread_continue.
	 */
	END(Switch_context)

/*
 *	stack_handoff:
 *
 *	Move the kernel stack from the old thread to the new thread.
 *	Installs the new thread in various globals.
 *	Changes address spaces if necessary.
 *	Assumes interrupts are disabled.
 *
 *	Arguments:
 *		a0		old thread
 *		a1		new thread
 */

LEAF(stack_handoff)
	lw	t0,THREAD_TASK(a0)	/* t0 is old task */
	lw	t1,THREAD_TASK(a1)	/* t1 is new task */
	lw	t6,THREAD_PCB(a1)	/* t6 is new pcb */
	beq	t0,t1,stack_handoff_done
	sw	t6,current_pcb

	/*
	 *	We have to switch address spaces.
	 */

	lw	t2,TASK_MAP(t1)		/* t2 is new map */
	lw	t7,EML_DISPATCH(t1)	/* t7 is new dispatch table */
	lw	t3,MAP_PMAP(t2)		/* t3 is new pmap */
	sw	t7,current_dispatch
	lw	t4,PMAP_PID(t3)		/* t4 is new tlbpid */
	lw	t5,PMAP_PTEBASE(t3)	/* t5 is page table base */
	bgez	t4,stack_handoff_context
	sll	t4,TLB_HI_PID_SHIFT	/* line up pid bits */

	/*
	 *	Bad news - we need a new tlbpid.
	 *	We use assign_tlbpid and try again.
	 */

	sw	a0,0(sp)
	sw	a1,4(sp)
	subu	sp,20
	sw	ra,16(sp)

	jal	assign_tlbpid
	move	a0,t3

	lw	ra,16(sp)
	addu	sp,20
	lw	a0,0(sp)
	b	stack_handoff		/* try again */
	lw	a1,4(sp)		/* first instruction doesn't use a1 */

stack_handoff_context:
	/*
	 *	We have a tlbpid.
	 *	This is essentially tlb_set_context.
	 */

	sw	t3,active_pmap		/* assert new pmap */
	mtc0	t5,c0_tlbcxt		/* tlb_umiss is happy now */
	and	t4,TLB_HI_PID_MASK	/* sanity */
	mtc0	t4,c0_tlbhi		/* assert new pid */

stack_handoff_done:
	/*
	 *	Change active_threads.
	 *	Attach the exception link to the new pcb.
	 */
	sw	a1,active_threads
	lw	t8,active_stacks
	j	ra
	sw	t6,KERNEL_STACK_SIZE-MSB_SIZE-MKS_SIZE-MEL_SIZE+MEL_EFRAME(t8)
	END(stack_handoff)

/*
 *	Object:
 *		setjmp				EXPORTED function
 *		_setjmp				EXPORTED function alias
 *
 *		Save current context for non-local goto's
 *
 *	Arguments:
 *		a0				jmp_buf *
 *
 *	Saves all registers that are callee-saved in the
 *	given longjmp buffer.  Same as user level _setjmp,
 *	but kernel does not use fpa.
 * 	Return 0
 */
LEAF(setjmp)
XLEAF(_setjmp)
	sw	ra,JB_PC(a0)
	sw	sp,JB_SP(a0)
	sw	fp,JB_FP(a0)
	sw	s0,JB_S0(a0)
	sw	s1,JB_S1(a0)
	sw	s2,JB_S2(a0)
	sw	s3,JB_S3(a0)
	sw	s4,JB_S4(a0)
	sw	s5,JB_S5(a0)
	sw	s6,JB_S6(a0)
	mfc0	v0,c0_status
	sw	s7,JB_S7(a0)
	sw	v0,JB_SR(a0)
	j	ra
	move	v0,zero			/* return zero */
	END(setjmp)


/*
 *	Object:
 *		longjmp				EXPORTED function
 *		_longjmp			EXPORTED function
 *
 *		Perform a non-local goto
 *
 *	Arguments:
 *		a0				jmp_buf *
 *		a1				unsigned
 *
 *	Restores all registers that are callee-saved from the
 *	given longjmp buffer.  Same as user level _longjmp
 * 	Return the second argument.
 */
LEAF(longjmp)
XLEAF(_longjmp)
	lw	ra,JB_PC(a0)
	lw	sp,JB_SP(a0)
	lw	fp,JB_FP(a0)
	lw	s0,JB_S0(a0)
	lw	s1,JB_S1(a0)
	lw	s2,JB_S2(a0)
	lw	s3,JB_S3(a0)
	lw	s4,JB_S4(a0)
	lw	s5,JB_S5(a0)
	lw	s6,JB_S6(a0)
	lw	v0,JB_SR(a0)
	lw	s7,JB_S7(a0)
	mtc0	v0,c0_status
	j	ra
	move	v0,a1			/* return a1 */
	END(longjmp)
