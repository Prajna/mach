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
 * $Log:	context.s,v $
 * Revision 2.2  93/02/05  07:57:44  danner
 * 	Change all mov inst to or inst
 * 	[93/01/12            jeffreyh]
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:12:29  af]
 * 
 * 	Created.
 * 	[92/12/10  14:55:23  af]
 * 
 *
 */
/* 
 *	File: context.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Context switching and context save/restore primitives
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 */

#include <cpus.h>


#include <mach/alpha/asm.h>
#include <mach/alpha/alpha_instruction.h>

#include <alpha/alpha_cpu.h>
#include <mach/alpha/vm_param.h>
#include <alpha/pmap.h>
#include <alpha/thread.h>
#include <alpha/context.h>

#include <assym.s>

	.set	noreorder		/* unless overridden */

/*
 *	Object:
 *		active_threads			IMPORTED variable
 *		active_stacks			IMPORTED variable
 *		current_dispatch		EXPORTED variable
 *
 *	- Pointer to current thread
 *	- Pointer to current kernel stack
 *	- Pointer to emulation dispatch structure for same
 */
BSS(active_threads,8*NCPUS)
BSS(active_stacks,8*NCPUS)
BSS(current_dispatch,8*NCPUS)

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
 *	globals: active_threads, active_stacks
 *	Used to start the first thread on a CPU.
 *	Assumes splsched.
 *	Only used at boot time, and never again. So who cares.
 */
LEAF(load_context,1)
	ldgp	gp,0(pv)

#if	(NCPUS>1)
	call_pal op_mfpr_whami
	/*
	 *	If we are a secondary we can release the
	 *	boot stack now and let the next one go
	 */
	IMPORT(master_cpu,4)
	ldl	s0,master_cpu
	subl	s0,v0,s0
	beq	s0,1f
	IMPORT(slave_init_lock,8)
	lda	s0,slave_init_lock
	stq	zero,0(s0)
1:
#else
	or	zero,zero,v0
#endif

	/*
	 *	Part 1: Load registers
	 */
	ldq	s0,THREAD_KERNEL_STACK(a0)	/* get kernel stack */
	ldq	s1,THREAD_PCB(a0)		/* PCB pointer */

	/*
	 *	Part 2: Critical section
	 *
	 *	Setup important global variables.
	 *	Relevant globals are bracketed in comments
	 *
	 */

	lda	t0,active_threads
	s8addq	v0,t0,t0
	stq	a0,0(t0)		/* >>active_threads<< */

	lda	t0,active_stacks
	s8addq	v0,t0,t0
	stq	s0,0(t0)		/* >>active_stacks<< */

	lda	sp,KERNEL_STACK_SIZE-MSB_SIZE-MEL_SIZE-MKS_SIZE(s0)

	lda	t0,KERNEL_STACK_SIZE-MSB_SIZE-TF_SIZE(s0)
	stq	t0,MSS_FRAMEP(s1)

	stq	s1,KERNEL_STACK_SIZE-MSB_SIZE+MSB_PCB(s0)

	/* Do not swpctxt here, was done before calling */

	ldq	ra,MKS_PC(sp)
	ldq	sp,MKS_SP(sp)

	or	zero,zero,a0	/* return zero to thread_continue */
	or	ra,zero,pv	/* enter/return issue */
	ret	zero,(ra)

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
 *	globals: active_threads.
 *	Assumes interrupts are disabled.
 */
LEAF(Switch_context,3)
	ldgp	gp,0(pv)

#if	(NCPUS>1)
	call_pal op_mfpr_whami
#else
	or	zero,zero,v0
#endif
	lda	t0,active_stacks
	s8addq	v0,t0,t0
	ldq	t0,0(t0)		/* get old kernel stack */
	stq	a1,THREAD_SWAP_FUNC(a0)
	stq	t0,THREAD_KERNEL_STACK(a0)
	bne	a1,1f

	/*
	 *	Part 1: Save context
	 *
	 * 	We only need to save those registers that are callee-saved
	 *	in C, everything else is already on the stack.
	 *	We don't need to do this if there is an explicit continuation.
	 */
#define	MKS_OFFSET	(KERNEL_STACK_SIZE-MSB_SIZE-MEL_SIZE-MKS_SIZE)
	stq	sp,MKS_OFFSET+MKS_SP(t0)
	stq	ra,MKS_OFFSET+MKS_PC(t0)

	stq	s0,MKS_OFFSET+MKS_S0(t0)
	stq	s1,MKS_OFFSET+MKS_S1(t0)
	stq	s2,MKS_OFFSET+MKS_S2(t0)
	stq	s3,MKS_OFFSET+MKS_S3(t0)
	stq	s4,MKS_OFFSET+MKS_S4(t0)
	stq	s5,MKS_OFFSET+MKS_S5(t0)
	stq	s6,MKS_OFFSET+MKS_S6(t0)
#undef	MKS_OFFSET
1:
	ldq	t1,THREAD_KERNEL_STACK(a2)	/* get new kernel stack */

	/*
	 *	Part 2: Setup new context
	 *
	 *	Setup important global variables.
	 *	Relevant globals are bracketed in comments
	 *
	 */

	lda	t0,active_threads
	s8addq	v0,t0,t0
	stq	a2,0(t0)		/* >>active_threads<< */

	lda	t0,active_stacks
	s8addq	v0,t0,t0
	stq	t1,0(t0)		/* >>active_stacks<< */

	lda	t0,KERNEL_STACK_SIZE-MSB_SIZE-MEL_SIZE-MKS_SIZE(t1)

	ldq	sp,MKS_SP(t0)	/* get new sp value */

	ldq	t1,THREAD_TASK(a2)
	ldq	ra,MKS_PC(t0)
	ldq	t1,EML_DISPATCH(t1)
	ldq	s1,MKS_S1(t0)
	ldq	s2,MKS_S2(t0)
	ldq	s3,MKS_S3(t0)
	ldq	s4,MKS_S4(t0)
	ldq	s5,MKS_S5(t0)
	ldq	s6,MKS_S6(t0)
	ldq	s0,MKS_S0(t0)
	lda	t0,current_dispatch
	s8addq	v0,t0,t0
	stq	t1,0(t0)		/* >>current_dispatch<< */
	or	a0,zero,v0
	or	ra,zero,pv		/* enter/return issue */
	ret	zero,(ra)

	/*
	 *	We return the old thread in v0 for switch_context
	 *	and a0 for thread_continue.
	 */

	END(Switch_context)

/*
 *	Object:
 *		stack_handoff			EXPORTED function
 *
 *	Arguments:
 *		a0				old thread *
 *		a1				new thread *
 *		a2				old task *
 *		a3				new task *
 *
 *	Move the kernel stack from the old thread to the new thread.
 *	Installs the new thread in various globals.
 *	Changes address spaces if necessary.
 *	Assumes interrupts are disabled.
 *
 *	Flames:
 *	Here we see why we want our own PAL code.
 */

LEAF(Stack_handoff,4)
	ldgp	gp,0(pv)
reenter:
#if	(NCPUS>1)
	call_pal op_mfpr_whami
#else
	or	zero,zero,v0
#endif
	ldq	t6,THREAD_PCB(a1)	/* t6 is new pcb */
	cmpeq	a2,a3,pv
	bne	pv,stack_handoff_context

	/*
	 *	We have to switch address spaces.
	 */

	ldq	t2,TASK_MAP(a3)		/* t2 is new map */
	ldq	t7,EML_DISPATCH(a3)	/* t7 is new dispatch table */
	ldq	t3,MAP_PMAP(t2)		/* t3 is new pmap */

	lda	t4,current_dispatch
	s8addq	v0,t4,t4
	stq	t7,0(t4)

	ldl	t4,PMAP_PID(t3)		/* t4 is new tlbpid */
#if	(NCPUS > 1)
	bge	t4,stack_handoff_done
#else
	bge	t4,stack_handoff_asn
#endif

	/*
	 *	Bad news - we need a new tlbpid.
	 *	We use assign_tlbpid and try again.
	 */

	subq	sp,40,sp
	stq	a0,0(sp)
	stq	a1,8(sp)
	stq	a2,16(sp)
	stq	a3,24(sp)
	stq	ra,32(sp)

	or	t3,zero,a0
	CALL(pmap_assign_tlbpid)

	ldq	a0,0(sp)
	ldq	a1,8(sp)
	ldq	a2,16(sp)
	ldq	a3,24(sp)
	ldq	ra,32(sp)
	addq	sp,40,sp
	br	zero,reenter		/* try again */

stack_handoff_asn:
	/*
	 *	We have a tlbpid, swap process context now
	 */
	stl	t4,MSS_ASN(t6)		/* ASN */

stack_handoff_context:	/* did not do it in cover code */
	/* pcb ptr in t6, virtual */
	stq	sp,MSS_KSP(t6)
	or	a0,zero,t4
	or	v0,zero,t8
	zap	t6,0xf0,a0		/* make phys, quick and dirty */
	call_pal op_swpctxt
	or	t4,zero,a0
	or	t8,zero,v0

stack_handoff_done:

	/*
	 *	Change active_threads.
	 *	Attach the exception link to the new pcb.
	 */

	lda	t4,active_threads
	s8addq	v0,t4,t4
	stq	a1,0(t4)

	lda	t4,active_stacks
	s8addq	v0,t4,t4
	ldq	t4,0(t4)
	stq	t6,KERNEL_STACK_SIZE-MSB_SIZE+MSB_PCB(t4)

	/*
	 *	Now if *we* had designed the PAL code...
	 *	..we'd saved those 64 bytes in the PCB!
	 */
	lda	t0,KERNEL_STACK_SIZE-1(zero)	/* get exception frame */
	or	t0,sp,t0
	lda	t0,-TF_SIZE-MSB_SIZE+1(t0)

	/* save the old ones */
/* opt: seems TF_PC might be null ? for kernel threads, that is */
	ldq	t7,THREAD_PCB(a0)
	lda	t1,MSS_SAVEDF(t7)		/* switch framep to pcb */
	stq	t1,MSS_FRAMEP(t7)
	ldq	t1,TF_R2(t0)
	stq	t1,MSS_T1(t7)
	ldq	t1,TF_R3(t0)
	stq	t1,MSS_T2(t7)
	ldq	t1,TF_R4(t0)
	stq	t1,MSS_T3(t7)
	ldq	t1,TF_R5(t0)
	stq	t1,MSS_T4(t7)
	ldq	t1,TF_R6(t0)
	stq	t1,MSS_T5(t7)
	ldq	t1,TF_R7(t0)
	stq	t1,MSS_T6(t7)
	ldq	t1,TF_PC(t0)
	stq	t1,MSS_PC(t7)

	/* install new ones */
	stq	t0,MSS_FRAMEP(t6)		/* switch framep to stack */
	ldq	t1,MSS_T1(t6)
	stq	t1,TF_R2(t0)
	ldq	t1,MSS_T2(t6)
	stq	t1,TF_R3(t0)
	ldq	t1,MSS_T3(t6)
	stq	t1,TF_R4(t0)
	ldq	t1,MSS_T4(t6)
	stq	t1,TF_R5(t0)
	ldq	t1,MSS_T5(t6)
	stq	t1,TF_R6(t0)
	ldq	t1,MSS_T6(t6)
	stq	t1,TF_R7(t0)
	ldq	t1,MSS_PC(t6)
	stq	t1,TF_PC(t0)


	RET			/* true return */
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
LEAF(setjmp,1)
XLEAF(_setjmp,1)
	stq	ra,JB_PC(a0)
	stq	sp,JB_SP(a0)
	stq	s0,JB_S0(a0)
	stq	s1,JB_S1(a0)
	stq	s2,JB_S2(a0)
	stq	s3,JB_S3(a0)
	stq	s4,JB_S4(a0)
	stq	s5,JB_S5(a0)
	stq	s6,JB_S6(a0)
	or	a0,zero,t4		/* PAL clobbers */
	call_pal op_mfpr_ipl
	stq	v0,JB_PS(t4)
	or	zero,zero,v0		/* return zero */
	RET
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
LEAF(longjmp,2)
XLEAF(_longjmp,2)
	ldq	ra,JB_PC(a0)
	ldq	sp,JB_SP(a0)
	ldq	s0,JB_S0(a0)
	ldq	s1,JB_S1(a0)
	ldq	s2,JB_S2(a0)
	ldq	s3,JB_S3(a0)
	ldq	s4,JB_S4(a0)
	ldq	s5,JB_S5(a0)
	ldq	s6,JB_S6(a0)
	ldq	a0,JB_PS(a0)
	or	a1,zero,t4
	call_pal op_mtpr_ipl
	or	t4,zero,v0		/* return a1 */
	RET
	END(longjmp)
