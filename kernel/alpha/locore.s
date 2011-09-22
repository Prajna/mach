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
 * $Log:	locore.s,v $
 * Revision 2.3  93/03/09  10:50:17  danner
 * 	Removed debugging code that did not fly under GCC.
 * 	Sigh, nothing like shoting yourself on the foot.
 * 	[93/02/19            af]
 * 
 * Revision 2.2  93/02/05  07:59:22  danner
 * 	Removed ISP hacks.  Console callbacks work now,
 * 	so they are not truly needed anymore.
 * 	[93/01/26            af]
 * 
 * 	Change all mov inst. to or instructions to avoid a strange chip
 * 	bug seen in starts.
 * 	[93/01/12            jeffreyh]
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:15:22  af]
 * 
 * 	Created.
 * 	[92/06/03            af]
 * 
 */

/*
 *	File: locore.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Low lever trap handlers
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 *
 */
#include <cpus.h>
#include <mach_kdb.h>

#include <mach/alpha/asm.h>
#include <mach/alpha/alpha_instruction.h>
#include <mach/alpha/vm_param.h>
#include <alpha/alpha_cpu.h>
#include <alpha/trap.h>

#include "assym.s"

#if	__GNU_AS__
#define	LDGP(x)	setgp x
#else
#define	LDGP(x)	ldgp gp,x
#endif

	.set	noat
	.set	noreorder

/*
 *	Object:
 *		gimmeabreak			EXPORTED function
 *
 *		Drop into the debugger
 *
 *	Arguments:
 *		none
 *
 *	:-))
 */
LEAF(gimmeabreak,0)
	call_pal op_bpt
	RET
	END(gimmeabreak)

/* when you need to patch away instructions.. */
EXPORT(alpha_nop)
	nop
	nop

/*
 *	Object:
 *		TRAP_dispatcher			EXPORTED VECTOR
 *
 *		Common trampoline for all traps
 *
 *	Arguments: (std trap)
 *		R2: SCBv -- here
 *		R3: SCBp -- pointer to handler routine
 *
 *	Here we do the common regsave and stack setup, then branch
 *	to the handler routine which was passed as argument.
 *	We can either branch here directly from PAL, or after a
 *	minimum register munging.
 */

	.align 5

VECTOR(TRAP_dispatcher,IM_T1|IM_T2|IM_T3|IM_T4|IM_T5|IM_T6|IM_RA)
	/*
	 * The PAL code has pushed on the stack:
	 *	sp-->	t1
	 *	+8	t2
	 *	+10	t3
	 *	+18	t4
	 *	+20	t5
	 *	+28	t6
	 *	+30	PC
	 *	+38	PS
	 *
	 * When we do the pal-rei this is popped off.
	 * t5-t6 are always free, t1-t2 contain the
	 * values at the SCB entry that shipped us here,
	 * t3-t4 are trap-specific.
	 * [t5 might be important if we came from TRAP_generic]
	 *
	 * So we have three scratch registers to play with.
	 *
	 * NOTE: we might take an interrupt at any time here,
	 *	 re-entering this very same code ==> careful
	 *	 with handling SP.
	 */
	ldq	t6,TF_PS(sp)	/* did we come from user mode ? */
	srl	t6,4,t1		/* NB: exec==kernel, superv==user */
	blbs	t1,from_user
	/*
	 * From kernel mode. Push full exception frame on stack.
	 */
			/* allocate the saved_state, and adjust sp */
	lda	sp,-MEL_TF-MSS_SIZE(sp)

	stq	gp,MSS_GP(sp)	/* fill a WB line */
	stq	a0,MSS_A0(sp)
	stq	a1,MSS_A1(sp)
	stq	a2,MSS_A2(sp)
#if	MACH_KDB
	/* The trapped SP has been aligned, so the old
	 * SP value must be recovered from the PS bits.
	 * This is done later in kdb_breakpoint, out of
	 * the main path. Normally we do not care. */
#endif
	stq	sp,MSS_SIZE+MEL_EFRAME(sp)	/* exc link */
	lda	a0,MSS_SIZE+MEL_TF(sp)		/* recover hw framep */
	stq	a0,MSS_FRAMEP(sp)
	or	sp,zero,a0
	
	/*
	 * Here we have:
	 *	t1 -> ?????
	 *	t6 -> PS
	 *	sp == a0 -> saved status
	 */
	br	zero,exc_save_registers

from_user:
#if	DEBUG
	lda	t1,0x1fff(zero)
	and	t1,sp,t1
	lda	t1,-0x1f80(t1)
	beq	t1,nobuggo
	call_pal	op_bpt
nobuggo:
#endif
	/*
	 * From user mode. Stack has been switched and
	 * HW frame pushed.
	 * On the stack we now have the HW frame, and
	 * on top of it the stack_base structure.
	 * In the stack_base structure there is the PCB pointer.
	 * The PCB has a backpointer setup already to the HW frame.
	 * t1 is free, it might point here or not.
	 */
	ldq	t1,TF_SIZE+MSB_PCB(sp)	/* PCB pointer */
	lda	sp,-MEL_TF-MKS_SIZE(sp)	/* adjust sp, alloc MKS */
	stq	gp,MSS_GP(t1)
	stq	a0,MSS_A0(t1)
	stq	a1,MSS_A1(t1)
	stq	a2,MSS_A2(t1)
	or	t1,zero,a0
	ldq	t1,TF_PC+MEL_TF+MKS_SIZE(sp)
	stq	t1,MSS_PC(a0)
#if 1
	/* aurghhh... iff we took a page fault */
	ldq	t1,TF_R2+MEL_TF+MKS_SIZE(sp)
	stq	t1,MSS_T1(a0)
	ldq	t1,TF_R3+MEL_TF+MKS_SIZE(sp)
	stq	t1,MSS_T2(a0)
	ldq	t1,TF_R4+MEL_TF+MKS_SIZE(sp)
	stq	t1,MSS_T3(a0)
	ldq	t1,TF_R5+MEL_TF+MKS_SIZE(sp)
	stq	t1,MSS_T4(a0)
	ldq	t1,TF_R6+MEL_TF+MKS_SIZE(sp)
	stq	t1,MSS_T5(a0)
	ldq	t1,TF_R7+MEL_TF+MKS_SIZE(sp)
	stq	t1,MSS_T6(a0)
#endif
			/* the loading of a proper GP is left for later */

exc_save_registers:
	/*
	 * Here:
	 *	sp -> <somewhere>
	 *	a0 -> saved status (pcb or stack)
	 *	t6 -> PS
	 *	gp -> invalid (but saved)
	 * Saved already: t1-t6, pc, ps, gp, a0-a2, user sp
	 */
	stq	a3,MSS_A3(a0)
	stq	a4,MSS_A4(a0)
	stq	a5,MSS_A5(a0)
	stq	ra,MSS_RA(a0)

	or	t3,zero,a1
	or	t4,zero,a2
	or	t5,zero,a3

	jsr	ra,(t2)

TRAP_end:
	/*
	 * Q: are we are going back to user mode ?
	 *
	 * A: see restore_all_other_regs. We get a
	 *    correct SP (in t1) and A0 values setup for us.
	 *    A1 is zero if going off to userland.
	 */
	bne	a1,out_we_go

	IMPORT(need_ast,4*NCPUS)

#if (NCPUS>1)
	or	v0,zero,a2
	call_pal op_mfpr_whami
	lda	a1,need_ast
	s4addq	v0,a1,a1
	or	a2,zero,v0
#else
	lda	a1,need_ast
#endif
	ldl	a1,0(a1)
	bne	a1,TRAP_ast
out_we_go:

	/* Memo: t1-t6 are scratch here.
	   t1 is used to hold the value of sp at exit */

	/* XXX add prefetching, both ways */
	ldq	gp,MSS_GP(a0)
/*	ldq	a0,MSS_A0(a0)	fetched now, loaded later 	*/
	ldq	a1,MSS_A1(a0)
	ldq	a2,MSS_A2(a0)

	ldq	a3,MSS_A3(a0)
	ldq	a4,MSS_A4(a0)
	ldq	a5,MSS_A5(a0)
	ldq	ra,MSS_RA(a0)

	ldq	a0,MSS_A0(a0)

	/* Only here can we let the SP go */
	or	t1,zero,sp

	call_pal op_rei


	END(TRAP_dispatcher)

/*
 *	Object:
 *		save_all_other_regs		LOCAL function
 *
 *	Argument:
 *		a0				alpha_saved_state *
 *
 *	Save all registers that TRAP_dispatcher did not.
 *	Same state as if dispatcher branched here stright.
 *	[Still unoptimized]
 *	Companion routine restore_all_other_regs SHOULD NOT BE MOVED
 *
 */
STATIC_LEAF(save_all_other_regs,1)
	stq	v0,MSS_V0(a0)
	stq	t0,MSS_T0(a0)
	stq	t7,MSS_T7(a0)

	stq	t8,MSS_T8(a0)
	stq	t9,MSS_T9(a0)
	stq	t10,MSS_T10(a0)
	stq	t11,MSS_T11(a0)

	stq	t12,MSS_T12(a0)
	stq	s0,MSS_S0(a0)
	stq	s1,MSS_S1(a0)
	stq	s2,MSS_S2(a0)

	stq	s3,MSS_S3(a0)
	stq	s4,MSS_S4(a0)
	stq	s5,MSS_S5(a0)
	stq	s6,MSS_S6(a0)

	stq	at,MSS_AT(a0)

#if	MACH_KDB
	/*
	 *	For debugger's sake, we declare here a VECTOR routine
	 *	so that we can tell from ra=restore_all_other_regs
	 *	that this is an exception frame.
	 */
	END(save_all_other_regs)
VECTOR(locore_exception_return, 0)

#endif	/* MACH_KDB */

	/*
	 * NOTE: This is just a (strange) call into C.
	 */
	or	ra,zero,pv
	jsr	ra,(pv)
	/* so that ra==restore_all_other_regs */
	/* DO NOT MOVE THE FOLLOWING THEN */

/*
 *	Object:
 *		restore_all_other_regs		LOCAL function
 *
 *	Argument:
 *		sp+MEL_FRAME			alpha_saved_state **
 *
 *	Restore all registers that TRAP_end will not.
 *	[Still unoptimized]
 *
 */
#if	MACH_KDB
restore_all_other_regs:
#else
STATIC_XLEAF(restore_all_other_regs,1)
#endif
	/*
	 * The following test assumes kernel stacks are
	 * aligned to their size. MUST be true, we shall
	 * need (mips docet) only 8k, e.g. twice what
	 * a 32 bit RISC needs.  Less than that, actually.
	 */
	lda	a1,KERNEL_STACK_SIZE-1 (zero)	/* < 32k ! */
	and	a1,sp,a1
	lda	a1,-(KERNEL_STACK_SIZE-MKS_SIZE-MEL_SIZE-MSB_SIZE) (a1)
	bne	a1,to_kernel_mode
to_user_mode:
	ldq	a0,MKS_SIZE+MEL_SIZE+MSB_PCB(sp)	/* PCB pointer */
	lda	t1,MEL_TF+MKS_SIZE(sp)		/* hw frame */
restore_them_registers:
	ldq	v0,MSS_V0(a0)
	ldq	t0,MSS_T0(a0)
	ldq	t7,MSS_T7(a0)

	ldq	t8,MSS_T8(a0)
	ldq	t9,MSS_T9(a0)
	ldq	t10,MSS_T10(a0)
	ldq	t11,MSS_T11(a0)

	ldq	t12,MSS_T12(a0)
	/* optimize away these, later */
	ldq	s0,MSS_S0(a0)
	ldq	s1,MSS_S1(a0)
	ldq	s2,MSS_S2(a0)

	ldq	s3,MSS_S3(a0)
	ldq	s4,MSS_S4(a0)
	ldq	s5,MSS_S5(a0)
	ldq	s6,MSS_S6(a0)

	ldq	at,MSS_AT(a0)

	br	zero,TRAP_end
	
to_kernel_mode:
	or	sp,zero,a0			/* saved_state pointer */
	lda	t1,MSS_SIZE+MEL_TF(sp)
	br	zero,restore_them_registers
	END(save_all_other_regs)


/*
 *	Object:
 *		TRAP_ast			EXPORTED VECTOR
 *
 */
VECTOR(TRAP_ast,0)
	/* See TRAP_end (assume jumped-to by restore_all_other_regs)
	   for the state here.  Its ok to just save_all_other_regs */
	IMPORT(ast_taken,4)
	lda	ra,ast_taken
	br	zero,save_all_other_regs
	/* NOTREACHED */
	END(TRAP_ast)

/*
 *	Object:
 *		TRAP_generic			EXPORTED VECTOR
 *
 *	Most trap get here from the SCB, we play with regs
 *	a bit then go to the dispatcher code.
 *
 */
VECTOR(TRAP_generic,IM_T1|IM_T2|IM_T3|IM_T4|IM_T5|IM_T6|IM_RA)
	/*
	 * t1 contains this address
	 * t2 contains the trap argument
	 */
	or	t2,zero,t5
	br	t2,TRAP_dispatcher	/* gets back here */
	LDGP(0(t2))
	/*
	 * Now we got to save registers, but we are
	 * all setup for calling C already.
	 * save_all_other_regs is also ready to avoid
	 * branching back here.
	 */
	lda	ra,trap
	br	zero,save_all_other_regs

	/* NOTREACHED */

	END(TRAP_generic)

/*
 *	Object:
 *		TRAP_interrupt			EXPORTED VECTOR
 *
 *	All interrupts get here from the SCB, we play with regs
 *	a bit then go to the dispatcher code.
 *
 */
VECTOR(TRAP_interrupt,IM_T1|IM_T2|IM_T3|IM_T4|IM_T5|IM_T6|IM_RA)
	/*
	 * t1 contains this address
	 * t2 contains the interrupt handler
	 */
	or	t2,zero,t5
	br	t2,TRAP_dispatcher	/* gets back here */
	LDGP(0(t2))
	/*
	 * Now we got to save registers, but we are
	 * all setup for calling C already.
	 * save_all_other_regs is also ready to avoid
	 * branching back here.
	 */

	/*
	 * We vector interrupts in two ways:
	 * (1) the argument t2 above is a function pointer
	 * (2) the argument t2 above is a small integer
	 * In the first case we call the function directly,
	 * recognizing the case by the high bit set (a k0seg
	 * address).  In the second case we dispatch to a
	 * C function for further processing. Note that
	 * argument registers have been saved already
	 */
	or	t5,zero,ra
	blt	t5,save_all_other_regs

	or	t5,zero,a1		/* needs further vectoring */
	IMPORT(interrupt,8)
	lda	ra,interrupt
	br	zero,save_all_other_regs

	/* NOTREACHED */

	END(TRAP_interrupt)

/*
 *	Object:
 *		thread_syscall_return		EXPORTED function
 *
 *		Return from syscall
 *
 *	Arguments:
 *		a0				int
 *
 *	Pop kernel stack, to get out to userland. Returns to
 *	the user the provided argument as result of the syscall.
 */
LEAF(Thread_syscall_return,1)

	or	a0,zero,v0				/* argument is return code */
	lda	t0,KERNEL_STACK_SIZE-1(zero)	/* get exception frame */
	or	t0,sp,t0
						/* pop the stack [later: again] */
	lda	sp,-MKS_SIZE-MEL_SIZE-MSB_SIZE+1(t0)
	lda	t1,-TF_SIZE-MSB_SIZE+1(t0)
						/* get pcb pointer */
	ldq	a0,TF_SIZE+MSB_PCB(t1)
	ldq	s0,MSS_S0(a0)
	ldq	s1,MSS_S1(a0)
	ldq	s2,MSS_S2(a0)
	ldq	s3,MSS_S3(a0)
	ldq	s4,MSS_S4(a0)
	ldq	s5,MSS_S5(a0)
	ldq	s6,MSS_S6(a0)
	or	zero,zero,a1				/* to user for sure */
	br	zero,TRAP_end
	END(thread_syscall_return)

/*
 *	Object:
 *		thread_bootstrap_return		EXPORTED function
 *
 *		Startup a USER-MODE thread.
 *		ALSO, return from user-mode page faults.
 *
 *	Arguments:
 *		a0				int
 *
 *	Pop kernel stack, to get out to userland. Returns to
 *	the user with the initial register status loaded up.
 */
LEAF(thread_bootstrap_return,0)

	lda	t0,KERNEL_STACK_SIZE-1(zero)	/* get 1st exception frame */
	or	t0,sp,t0
						/* pop the stack */
	lda	sp,-MKS_SIZE-MEL_SIZE-MSB_SIZE+1(t0)
						/* get pcb pointer */
	ldq	a0,MKS_SIZE+MEL_SIZE+MSB_PCB(sp)
						/* set backptr to hw frame */
	lda	t0,MKS_SIZE+MEL_TF(sp)
	stq	t0,MSS_FRAMEP(a0)
/*sanity*/
stq	zero,MKS_SIZE+MEL_EFRAME(sp)

	/* Loadup HW frame now */

	lda	a1,alpha_initial_ps_value(zero)
	stq	a1,MKS_SIZE+MEL_TF+TF_PS(sp)		/* user-mode ps */

	ldq	t0,MSS_PC(a0)
	stq	t0,MKS_SIZE+MEL_TF+TF_PC(sp)

	ldq	t0,MSS_T1(a0)
	stq	t0,MKS_SIZE+MEL_TF+TF_R2(sp)
	ldq	t0,MSS_T2(a0)
	stq	t0,MKS_SIZE+MEL_TF+TF_R3(sp)
	ldq	t0,MSS_T3(a0)
	stq	t0,MKS_SIZE+MEL_TF+TF_R4(sp)
	ldq	t0,MSS_T4(a0)
	stq	t0,MKS_SIZE+MEL_TF+TF_R5(sp)
	ldq	t0,MSS_T5(a0)
	stq	t0,MKS_SIZE+MEL_TF+TF_R6(sp)
	ldq	t0,MSS_T6(a0)
	stq	t0,MKS_SIZE+MEL_TF+TF_R7(sp)

	/* Now the S regs */
	ldq	s0,MSS_S0(a0)
	ldq	s1,MSS_S1(a0)
	ldq	s2,MSS_S2(a0)
	ldq	s3,MSS_S3(a0)
	ldq	s4,MSS_S4(a0)
	ldq	s5,MSS_S5(a0)
	ldq	s6,MSS_S6(a0)

	/* done */
	br	zero,restore_all_other_regs
	END(thread_bootstrap_return)


/*
 *	Object:
 *		call_continuation		EXPORTED function
 *
 *	Arguments:
 *		a0				int
 *
 *	Invoke the given function with a cleaned up kernel stack.
 *	Stack is set as if just in from user mode.
 */
LEAF(call_continuation,1)
	lda	t0,KERNEL_STACK_SIZE-1(zero)		/* pop the stack */
	or	t0,sp,t0
	lda	sp,-MKS_SIZE-MEL_SIZE-MSB_SIZE+1(t0)
	or	a0,zero,ra
	or	a0,zero,pv
	jmp	zero,(ra)				/* call continuation */
	END(call_continuation)


/*
 *	Object:
 *		kdb_breakpoint			EXPORTED function
 *
 *		Kernel breakpoint trap
 *
 *	Arguments:
 *		a0				saved_status *
 *
 *	Save as much state as possible on stack including IPRs
 *	status, invoke debugger.  On return, restore and continue.
 */
VECTOR(kdb_breakpoint,0)
	/* a0-a5 saved already, sp, ps, gp, ra too */
	LDGP(0(t2))		/* we know how dispatch works */
	stq	v0,MSS_V0(a0)
	stq	t0,MSS_T0(a0)

	/* t6 holds the PS */
	srl	t6,4,t1		/* NB: exec==kernel, superv==user */
	blbs	t1,kdb_of_user

	/* find SP value before trapping */
	srl	t6,56,t0		/* t6 still holds PS. Alignement.. */
	lda	v0,MSS_SIZE+MEL_TF(sp)	/* .. HW frame ... */ 
	addq	t0,v0,t0		/* .. adjust for alignment ... */
	lda	t0,MEL_SIZE-MEL_TF(t0)	/* .. got it */
	stq	t0,MSS_SP(a0)
	br	zero,copy_frame

kdb_of_user:
	ldq	t0,MSS_USP(a0)		/* user SP */
	lda	v0,MKS_SIZE+MEL_TF(sp)
	stq	t0,MSS_SP(a0)

copy_frame:
	stq	sp,MSS_BAD(a0)		/* cut this crap, later */
	/*
	 * Copy the exception frame where ddb can look at.
	 * This is not normally done on traps.
	 */
	stq	t6,MSS_PS(a0)		/* exc ps */

	ldq	t0,TF_PC(v0)		/* exc pc */
	subq	t0,4,t0			/* is next PC */
	stq	t0,MSS_PC(a0)

	ldq	t0,TF_R2(v0)
	stq	t0,MSS_T1(a0)
	ldq	t0,TF_R3(v0)
	stq	t0,MSS_T2(a0)
	ldq	t0,TF_R4(v0)
	stq	t0,MSS_T3(a0)
	ldq	t0,TF_R5(v0)
	stq	t0,MSS_T4(a0)
	ldq	t0,TF_R6(v0)
	stq	t0,MSS_T5(a0)
	ldq	t0,TF_R7(v0)
	stq	t0,MSS_T6(a0)

	/* duplicated so that save/restore are self-debuggable */
	stq	t7,MSS_T7(a0)
	stq	t8,MSS_T8(a0)
	stq	t9,MSS_T9(a0)
	stq	t10,MSS_T10(a0)
	stq	t11,MSS_T11(a0)
	stq	t12,MSS_T12(a0)

	stq	s0,MSS_S0(a0)
	stq	s1,MSS_S1(a0)
	stq	s2,MSS_S2(a0)
	stq	s3,MSS_S3(a0)
	stq	s4,MSS_S4(a0)
	stq	s5,MSS_S5(a0)
	stq	s6,MSS_S6(a0)

	stq	at,MSS_AT(a0)
/*	stq	zero,MSS_BAD(a0)	used for trapped SP */

	addq	zero,T_BP,t0
	stq	t0,MSS_CAUSE(a0)

	mov	zero,a1
	CALL(kdb_trap)

	/*
	 * The following test assumes kernel stacks are
	 * aligned to their size. MUST be true, we shall
	 * need (mips docet) only 8k, e.g. twice what
	 * a 32 bit RISC needs.  Less than that, actually.
	 */
	lda	a1,KERNEL_STACK_SIZE-1 (zero)	/* < 32k ! */
	and	a1,sp,a1
	lda	a1,-(KERNEL_STACK_SIZE-MKS_SIZE-MEL_SIZE-MSB_SIZE) (a1)
	beq	a1,to_user_mode_1
	or	sp,zero,a0			/* saved_state pointer */
	lda	t1,MSS_SIZE+MEL_TF(sp)
restore_them_1:
#if	0
	will play with matches later
	/* restore SP, possibly modified with KDB */
	ldq	sp,MSS_BAD(a0)
	stq	t0,MSS_SP(a0)
#endif
	ldq	t0,MSS_T1(a0)
	stq	t0,TF_R2(t1)
	ldq	t0,MSS_T2(a0)
	stq	t0,TF_R3(t1)
	ldq	t0,MSS_T3(a0)
	stq	t0,TF_R4(t1)
	ldq	t0,MSS_T4(a0)
	stq	t0,TF_R5(t1)
	ldq	t0,MSS_T5(a0)
	stq	t0,TF_R6(t1)
	ldq	t0,MSS_T6(a0)
	stq	t0,TF_R7(t1)
	ldq	t0,MSS_PC(a0)
	stq	t0,TF_PC(t1)

	ldq	v0,MSS_V0(a0)
	ldq	t0,MSS_T0(a0)

	ldq	t7,MSS_T7(a0)
	ldq	t8,MSS_T8(a0)
	ldq	t9,MSS_T9(a0)
	ldq	t10,MSS_T10(a0)
	ldq	t11,MSS_T11(a0)
	ldq	t12,MSS_T12(a0)

	ldq	s0,MSS_S0(a0)
	ldq	s1,MSS_S1(a0)
	ldq	s2,MSS_S2(a0)
	ldq	s3,MSS_S3(a0)
	ldq	s4,MSS_S4(a0)
	ldq	s5,MSS_S5(a0)
	ldq	s6,MSS_S6(a0)

	ldq	at,MSS_AT(a0)

	br	zero,TRAP_end

to_user_mode_1:
	ldq	a0,MKS_SIZE+MEL_SIZE+MSB_PCB(sp)	/* PCB pointer */
	lda	t1,MKS_SIZE+MEL_TF(sp)			/* hw frame */
	br	zero,restore_them_1

	END(kdb_breakpoint)
