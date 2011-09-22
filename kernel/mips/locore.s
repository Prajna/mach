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
 * $Log:	locore.s,v $
 * Revision 2.23  93/01/14  17:51:36  danner
 * 	Restore FPA-disabled optimization that was broken in rev 2.13.
 * 	[92/12/10            af]
 * 
 * Revision 2.22  92/02/19  15:09:09  elf
 * 	Changed #-style comments, for ANSI cpp.
 * 	[92/02/19  13:11:00  rpd]
 * 
 * Revision 2.21  92/01/03  20:24:33  dbg
 * 	Remove fixed lower bound on emulated system call number.
 * 	[91/11/01            dbg]
 * 
 * Revision 2.20  91/08/24  12:22:47  af
 * 	Made clearing of buserrors variables, accomodates 3mins.
 * 	[91/08/02  03:19:50  af]
 * 
 * Revision 2.19  91/05/18  14:35:41  rpd
 * 	Picked up Sandro's interrupt-enable fix for TRAP_exception_exit.
 * 	[91/04/04            rpd]
 * 	Removed tlb_umiss_cause.
 * 	[91/03/21            rpd]
 * 
 * Revision 2.18  91/05/14  17:34:38  mrt
 * 	Correcting copyright
 * 
 * Revision 2.17  91/05/13  06:05:59  af
 * 	On exception return make sure interrupts are enabled,
 * 	higher level code should only play with spls().
 * 	Added kdb_go, but does not fly yet.
 * 	[91/05/12  15:59:20  af]
 * 
 * Revision 2.16  91/03/16  14:56:13  rpd
 * 	Pulled mips_float_state out of mips_machine_state.
 * 	[91/02/18            rpd]
 * 
 * 	Changed call_continuation to not change spl.
 * 	[91/02/17            rpd]
 * 	Changed TRAP_cu to disable fpa use for the old fpa owner.
 * 	[91/01/29            rpd]
 * 	Added call_continuation.
 * 	Changed the AST interface.
 * 	[91/01/18            rpd]
 * 
 * Revision 2.15  91/02/14  14:37:37  mrt
 * 	Reversed the meaning of the booleans in the ref_bits array.
 * 	[91/02/12  12:24:39  af]
 * 
 * Revision 2.14  91/02/05  17:48:46  mrt
 * 	Added author notices
 * 	[91/02/04  11:22:43  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:26:21  mrt]
 * 
 * Revision 2.13  91/01/08  15:50:12  rpd
 * 	Fixed gimmeabreak to allow the user to single-step out of it.
 * 	[91/01/06            rpd]
 * 	Added mips_stack_base.
 * 	[91/01/02            rpd]
 * 
 * 	Split mips_machine_state off of mips_kernel_state.
 * 	[90/12/30            rpd]
 * 	Moved thread_exception_return to mips/trap.c.
 * 	[90/12/23            rpd]
 * 
 * 	Replaced thread_bootstrap_user, thread_bootstrap_kernel
 * 	with thread_exception_return, thread_syscall_return.
 * 	Updated native syscall processing for new mach_trap_table layout.
 * 	[90/12/18            rpd]
 * 	Changed the exception frame/pcb layout.
 * 	[90/11/19            rpd]
 * 
 * Revision 2.9  90/08/27  22:08:24  dbg
 * 	Revised KDB support for new debugger: now most information is
 * 	saved on the thread's stack, making it more re-entrant.
 * 	Cleanups.
 * 	[90/08/18            af]
 * 
 * Revision 2.8  90/08/07  22:29:22  rpd
 * 	3max support, mispellings.
 * 	[90/08/07  15:18:54  af]
 * 
 * Revision 2.6.1.1  90/05/30  16:54:50  af
 * 	Added 3max support: clear bus errors right away, in a machdep way.
 * 
 * Revision 2.7  90/06/02  15:02:29  rpd
 * 	Put trap history code under new TRAP_HISTORY compilation option.
 * 	Cleaned up the code; made it save more information.
 * 	[90/05/12            rpd]
 * 	From jsb: added warning about arguments to traps.
 * 	[90/04/23            rpd]
 * 	Added code for traps with seven arguments.
 * 	[90/03/26  22:49:30  rpd]
 * 
 * Revision 2.6  90/05/29  18:38:09  rwd
 * 	Added support for 11 arguments to trap_syscall (rpd)
 * 	[90/05/22            rwd]
 * 
 * Revision 2.5  90/03/14  21:11:06  rwd
 * 	Added INCLUDE_VERSION (from af).
 * 	[90/03/06            rwd]
 * 
 * Revision 2.4  90/01/22  23:07:13  af
 * 	Renames and code shuffling to let KDB trace through exceptions.
 * 	[90/01/22            af]
 * 	Consolidated conditionals.
 * 	[90/01/20  17:08:58  af]
 * 
 * 	Fixed ref bits to check against I/O pages.
 * 	Do not use fast_thread_recover for tas emulation.
 * 	[89/12/09  10:58:40  af]
 * 
 * 	Added mods to the tlb_umiss handler to keep reference bits.
 * 	Experimental, but seems to work and do some good.
 * 	[89/12/05  02:20:16  af]
 * 
 * Revision 2.3  89/12/08  19:47:48  rwd
 * 	Added mods to the tlb_umiss handler to keep reference bits.
 * 	Experimental, but seems to work and do some good.
 * 	[89/12/05  02:20:16  af]
 * 
 * Revision 2.2  89/11/29  14:14:14  af
 * 	Thread_bootstrap() must reload all registers.
 * 	Slight modifications to the FPA support code.
 * 	[89/11/26  10:38:07  af]
 * 
 * 	Added emulated U*x syscalls.
 * 	Added scheduling of the FPA on demand.
 * 	Made kdb capable of debugging user-mode code.
 * 	Made traps within kdb not clobber the pcb.
 * 	Reduced max number of nested exceptions, to limit
 * 	stack space waste.
 * 	[89/11/16  14:34:37  af]
 * 
 * 	Well, it's working now.  Lots of little spots cleaned.
 * 	[89/11/03  16:22:38  af]
 * 
 * 	Created.
 * 	[89/10/06            af]
 * 
 */
/*
 *	File: locore.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	First level trap handlers for MIPS.
 *
 */

/*
 *		"Nel mezzo del cammin di nostra vita
 *		 Mi ritrovai per una selva oscura,
 *		 Che la diritta via era smarrita"
 *
 *			Dante Alighieri, La Divina Commedia, Inferno 1.1
 *
 */

#include <mach_kdb.h>
#include <mach_assert.h>
#include <ref_bits.h>
#include <trap_history.h>

#include <mach/mips/asm.h>
#include <mach/mips/mips_instruction.h>


#include <mach/mips/vm_param.h>
#include <mach/kern_return.h>
#include <kern/syscall_emulation.h>
#include <mips/ast.h>
#include <mips/mips_cpu.h>
#include <mips/prom_interface.h>	/* debugger call seq. */
#include <mips/mips_box.h>
#include <mips/thread.h>
#if	REF_BITS
#include <mips/pmap.h>
#endif	REF_BITS

#include <assym.s>

	.set	noreorder		/* unless overruled */

#define	DEBUG		1		/* debugging tools at end */
#define	DEBUG_USERS	1
#define TLB_UMISS_COUNT	0

/*
 *	Object:
 *		INCLUDE_VERSION			EXPORTED absolute
 *
 *  Arrange for the include file version number to appear directly in
 *  the name list.
 *
 */
#include <sys/version.h>
	ABS(_INCLUDE_VERSION, INCLUDE_VERSION)

	BSS(active_threads,4)

#if	TLB_UMISS_COUNT
	ABS(tlb_umiss_count, VEC_EXCEPTION-44)
#endif	TLB_UMISS_COUNT
#if	TRAP_HISTORY
	ABS(trap_save_area, VEC_EXCEPTION-40)
	BSS(trap_history_index, 4)
	BSS(trap_history, 32*256)

.globl	kdbactive

LEAF(trap_saver)
	/* k0 holds the address of trap_save_area */
	/* in trap_save_area we have */
	/*	0  original at	already saved */
	/*	4  original k1	not yet */
	/*	8  original ra	already saved */
	/*	12 original gp	not yet */
	/*	16 our ra	not yet */

	sw	k1,4(k0)
	sw	gp,12(k0)
	sw	ra,16(k0)

	/* check if kdbactive is true.  we don't record traps */
	/* when kdb is running.  also, make gp useful. */

	la	gp,_gp
	lw	k0,kdbactive
	nop
	bne	k0,zero,1f

	/* index into the trap_history array, using trap_history_index */
	/* we shift by 5 because array elements are 32 bytes */
	/*	0  c0_epc */
	/*	4  sp */
	/*	8  c0_status */
	/*	12 c0_cause */
	/*	16 our ra */
	/*	20 c0_tlbbad */
	/*	24 c0_tlbcxt */
	/*	28 */

	lw	k0,trap_history_index
	la	k1,trap_history
	sll	AT,k0,5
	addu	k1,AT

	addiu	k0,1
	andi	k0,0xff
	sw	k0,trap_history_index

	/* k1 contains pointer to the trap_history entry */
	/* k0 and ra are used as temp registers */

	sw	ra,16(k1)
	mfc0	k0,c0_epc
	mfc0	ra,c0_status
	sw	k0,0(k1)
	sw	ra,8(k1)
	mfc0	k0,c0_cause
	mfc0	ra,c0_tlbcxt
	sw	k0,12(k1)
	sw	ra,24(k1)
	mfc0	k0,c0_tlbbad
	sw	sp,4(k1)
	sw	k0,20(k1)

1:	la	k0,trap_save_area
	lw	AT,0(k0)
	lw	k1,4(k0)
	lw	ra,8(k0)
	lw	gp,12(k0)
	lw	k0,16(k0)
	nop
	j	k0
	nop
	END(trap_saver)
#endif	TRAP_HISTORY

/*
 *	Object:
 *		exc_nesting_check		LOCAL structure
 *		exc_crash_area			EXPORTED structure
 *
 *	The hardest thing to debug is when the machine loops
 *	in exceptions, not answering to KDB.
 *	If the machine never makes it to get back to user mode
 *	in a reasonable amount of exceptions we crash it and
 *	save all registers in the "crash_area" for later
 *	inspection.
 *
 *	Object:
 *		pmap_reference_bits		EXPORTED byte_array
 *
 *	Used when simulating hardware reference bits.
 *	We use a page or so of the hole left in low memory by
 *	loading the kernel at 80030000 (or similar values),
 *	being careful not to touch zones that the prom knows.
 *
 */
#if	DEBUG
	ABS(exc_nesting_check, VEC_EXCEPTION-20)
	BSS(crash_area, 160)
#endif	DEBUG
#if	REF_BITS
	ABS(pmap_reference_bits, K0SEG_BASE + 0x20000)
#endif	REF_BITS


/*
 *	Object:
 *		TRAP_tlb_umiss			EXPORTED handler
 *		TRAP_tlb_umiss_end		EXPORTED label
 *
 *		Handles TLB misses in the kuseg
 *
 *	Arguments:
 *		none
 *
 *	Reloading the TLB is (relatively) expensive, so we'd
 *	like this exception handler to be as fast as possible.
 *	It only uses the kernel scratch registers
 *	and does not touch the stack, and makes the minimum
 *	number of memory accesses (1 load, 1 store).
 *
 *	Note that we need to stash away the cause register,
 *	if we miss again on the pte it would get corrupted.
 *	Note also that the EPC is saved in k1, in case we
 *	double miss.
 *
 *	This code is compiled 'wherever', but copied down to
 *	VEC_TLB_UMISS at startup.  The TRAP_tlb_umiss_end
 *	label is used as a delimiter in the copy operation.
 */
NESTED(TRAP_tlb_umiss,0,k1)		/* KDB's sake */
 	.set	noat
#if	TRAP_HISTORY
	li	k0,trap_save_area
	sw	ra,8(k0)		/* save ra */
	jal	trap_saver
	sw	AT,0(k0)		/* save AT */
#endif	TRAP_HISTORY

#if	TLB_UMISS_COUNT
	lui	k1,tlb_umiss_count>>16
	lw	k0,+tlb_umiss_count&0x7fff(k1)
	nop
	addiu	k0,1
	sw	k0,+tlb_umiss_count&0x7fff(k1)
#endif	TLB_UMISS_COUNT

	mfc0	k0,c0_tlbcxt
	mfc0	k1,c0_epc
EXPORT(TRAP_tlb_umiss_load)
	lw	k0,0(k0)
#if	REF_BITS
	/*
	 *	If we missed above we won't get back here .
	 *	The reference bit array MUST be in kseg0
	 *	for the following code to work, and the
	 *	reference bits array statically allocated.
	 *
	 *	The overhead is 7 instructions, including
	 *	a store.  Presumably the write buffer is
	 *	empty by the time we do the store, so we
	 *	should be paying just about 7 extra cycles.
	 */
	lui	k1,0xf000		/* beware of mapped I/O pages */
	mtc0	k0,c0_tlblo
	and	k1,k0
	bne	k1,zero,1f
	tlbwr				/* delay slot */
	srl	k0,VA_PAGEOFF		/* get phys page number */
	li	k1,pmap_reference_bits
	addu	k1,k0
	sb	zero,0(k1)
1:	mfc0	k1,c0_epc
	nop
#else	REF_BITS
	nop
	mtc0	k0,c0_tlblo
	nop
	tlbwr
#endif	REF_BITS
	j	k1
	rfe
 	.set	at
EXPORT(TRAP_tlb_umiss_end)
	nop				/* pretty up KDB */
	END(tlb_umiss)

/*
 *	Object:
 *		TRAP_exception			EXPORTED function
 *		TRAP_exception_end		EXPORTED label
 *
 *		General exception handler.
 *
 *	Arguments:
 *		none
 *
 *	This is where all machine traps are dispatched, but for
 *	tlb_umisses and reset (which goes to the prom, unescapably).
 *	This code is also copied down to its workplace, which is
 *	somewhat narrow.  Therefore it only does the minimum work
 *	required before dispatching to specific trap handlers.
 *	Note that as soon as a jump is taken we get back to
 *	the original copy in higher memory (jumps, not branches).
 */
#define	MINSAVED (M_EXCFRM|M_SP|M_AT|M_GP|M_K1|M_RA|M_A0|M_A1|M_A2|M_A3)
VECTOR(TRAP_exception,MINSAVED)
	.set	noat
#if	TRAP_HISTORY
	li	k0,trap_save_area
	sw	ra,8(k0)		/* save ra */
	jal	trap_saver
	sw	AT,0(k0)		/* save AT */
#endif	TRAP_HISTORY

	mfc0	k0,c0_status		/* coming from user mode ? */
	nop
	sll	k0,31-SR_KUo_SHIFT	/* might have taken a tlb_umiss */
	bltz	k0,exc_switch_stack
	move	k0,AT			/* make AT usable asap */

#if	DEBUG
	li	AT,exc_nesting_check	/* looping in exceptions? */
	sw	k0,4(AT)
	lw	k0,0(AT)
	sw	k1,8(AT)
	addi	k0,1
	slt	k1,k0,16		/* max nested exceptions */
	bne	k1,zero,exc_nested_ok
	sw	k0,0(AT)
	lw	k1,8(AT)		/* max exceeded */
	lw	k0,4(AT)
	sw	zero,0(AT)
	la	AT,crashit
	j	AT
	nop
exc_nested_ok:
	lw	k0,4(AT)
	lw	k1,8(AT)
#endif	DEBUG

	/*
	 *	AT is saved in k0
	 *	We are coming from kernel mode, so we can just
	 *	push an exception frame on the existing stack.
	 */
	sw	sp,MSS_SP-MSS_SIZE(sp)	/* push exception frame on stack */
	sw	a0,MSS_A0-MSS_SIZE(sp)
	addiu	a0,sp,-MSS_SIZE
	addiu	sp,a0,-MEL_SIZE		/* push exception link on stack */
	sw	a0,MEL_EFRAME(sp)
	sw	k0,MSS_AT(a0)
	b	exc_save_registers
	mfc0	k0,c0_status

exc_switch_stack:
	/*
	 *	AT is saved in k0
	 *	We are coming from user mode.  We must switch
	 *	stacks and use the exception frame in the pcb.
	 */
	lw	AT,current_pcb		/* exc frame is in pcb */
	nop
	sw	k0,MSS_AT(AT)
	mfc0	k0,c0_status		/* we will pop off KUp */
	sw	sp,MSS_SP(AT)
	sw	a0,MSS_A0(AT)
	lw	sp,current_kstack	/* exception link already set up */
	move	a0,AT
	.set	at
	srl	k0,SR_KUp_SHIFT+1
	sll	k0,SR_KUp_SHIFT+1
	mtc0	k0,c0_status
	sw	gp,MSS_GP(a0)		/* switch GP too */
	la	gp,_gp

exc_save_registers:
	/*
	 *	a0, AT, sp saved in the exception frame
	 *	gp is saved if we are coming from user mode
	 *	a0 is exception frame pointer
	 *	k0 is c0_status
	 *	sp is kernel stack pointer/exception link
	 *	gp is kernel gp pointer
	 *	a0 == MEL_EFRAME(sp)
	 */
	sw	a2,MSS_A2(a0)
	mfc0	a2,c0_epc
	sw	k1,MSS_K1(a0)
	mfc0	k1,c0_cause
	sw	k0,MSS_SR(a0)
	sw	a2,MSS_PC(a0)
	sw	k1,MSS_CAUSE(a0)

	sw	a1,MSS_A1(a0)
	sw	a3,MSS_A3(a0)
					/* select handler */
	andi	a3,k1,CAUSE_EXC_MASK
	sll	a3,1
	la	a1,trap_table
	addu	a1,a3
	j	a1
	sw	ra,MSS_RA(a0)
	/*
	 *	Dispatching above leads to this register state:
	 *
	 *		REG	OLD	NEW
	 *		a0	saved	exception frame pointer
	 *		a1	saved	--
	 *		a2	saved	epc
	 *		a3	saved	--
	 *		epc	saved	unchanged
	 *		cause	saved	    "
	 *		status	saved	    "
	 *		ra	saved	    "
	 *		sp	saved	stack pointer
	 *		k0	--	status register
	 *		k1	saved	cause register
	 *		at	saved	--
	 *		gp	saved	kernel's
	 *
	 *	Handlers might also use the delay slot in the following
	 *	jump table for internal purposes.
	 */
trap_table:
	j	TRAP_interrupt
	move	a2,k0
	j	TRAP_mod
	mfc0	a1,c0_tlbbad
	j	TRAP_tlb_miss		/* read miss */
	move	a2,k1
	j	TRAP_tlb_miss		/* write miss */
	move	a2,k1
	j	TRAP_ade		/* read error */
	mfc0	ra,c0_tlbbad
	j	TRAP_ade		/* write error */
	mfc0	ra,c0_tlbbad
	j	TRAP_ibe
	mfc0	a1,c0_epc
	j	TRAP_dbe
	mfc0	a1,c0_epc
	j	TRAP_syscall
	move	a1,k1
	j	TRAP_bp
	lw	a1,MSS_PC(a0)
	j	TRAP_ri
	move	a1,k1
	j	TRAP_cu
	move	a1,k1
	j	TRAP_generic		/* overflow */
	move	a1,k1
	j	TRAP_reserved		/* undefined */
	mfc0	a1,c0_tlbbad
	j	TRAP_reserved		/* undefined */
	mfc0	a1,c0_tlbbad
	j	TRAP_reserved		/* undefined */
	mfc0	a1,c0_tlbbad
EXPORT(TRAP_exception_end)
	nop
	END(TRAP_exception)

VECTOR(TRAP_exception_exit,MINSAVED)
	/*
	 *	a0 is exception frame pointer
	 */
	mtc0	zero,c0_status		/* disable interrupts */
					/* all traps eventually return here  */
	lw	a1,MSS_A1(a0)
	lw	a2,MSS_A2(a0)
	lw	a3,MSS_A3(a0)
	lw	ra,MSS_RA(a0)
#if	DEBUG
	sw	zero,exc_nesting_check
#endif	DEBUG
	lw	k1,MSS_SR(a0)
	lw	k0,MSS_PC(a0)
 	.set	noat
	sll	AT,k1,31-SR_KUo_SHIFT	/* to user ? */
	bgez	AT,2f
	ori	k1,SR_IEp		/* enable non-masked interrupts */
	lw	AT,need_ast		/* yep, check for AST */
	ori	k1,(SR_KUp|SR_IEp)
	bne	AT,zero,TRAP_ast	/* take the AST */
	nop			

1:	lw	gp,MSS_GP(a0)		/* only if to user */
2:	lw	AT,MSS_AT(a0)
	lw	sp,MSS_SP(a0)
	lw	a0,MSS_A0(a0)
	mtc0	k1,c0_status
	j	k0
	rfe
 	.set	at

fast_exception_end:
	/*
	 *	a0 is exception frame pointer
	 */
					/* for emulated instructions&syscalls */
	mtc0	zero,c0_status		/* disable interrupts */
	lw	sp,MSS_SP(a0)
	lw	a1,MSS_A1(a0)
	lw	a2,MSS_A2(a0)
	lw	a3,MSS_A3(a0)
	lw	ra,MSS_RA(a0)
#if	DEBUG
	sw	zero,exc_nesting_check
#endif	DEBUG
	.set	noat
	lw	k1,MSS_SR(a0)
	lw	k0,MSS_PC(a0)
	lw	gp,MSS_GP(a0)		/* to user for sure */
	lw	AT,MSS_AT(a0)
	lw	a0,MSS_A0(a0)
	ori	k1,(SR_KUp|SR_IEp)
	mtc0	k1,c0_status
	j	k0
	rfe
	.set	at
	END(TRAP_exception_exit)

/*
 *	Object:
 *		restore_registers		LOCAL function alias
 *
 *	Arguments:
 *		a0				mips_saved_state
 *
 *	See note above.  This code does not return but goes
 *	straight to the exception epilogue.
 */
STATIC_LEAF(restore_registers)
	lw	t0,MSS_HI(a0)
	lw	t1,MSS_LO(a0)
	mthi	t0
	mtlo	t1
	lw	v0,MSS_V0(a0)
	lw	v1,MSS_V1(a0)
	lw	t0,MSS_T0(a0)
	lw	t1,MSS_T1(a0)
	lw	t2,MSS_T2(a0)
	lw	t3,MSS_T3(a0)
	lw	t4,MSS_T4(a0)
	lw	t5,MSS_T5(a0)
	lw	t6,MSS_T6(a0)
	lw	t7,MSS_T7(a0)
	lw	t8,MSS_T8(a0)
	b	TRAP_exception_exit
	lw	t9,MSS_T9(a0)
	END(restore_registers)



/*
 *	Object:
 *		save_registers			LOCAL function
 *
 *		Save caller-saved registers
 *
 *	Arguments:
 *		a0				mips_saved_state
 *
 *	Argument registers must have been saved already, as well
 *	as the 'ra' register.  We leave the argument registers unchanged.
 */
#define MEDSAVED (M_T0|M_T1|M_V0|M_V1|M_T2|M_T3|M_T4|M_T5|M_T6|M_T7|M_T8|M_T9)

STATIC_LEAF(save_registers)
	sw	t0,MSS_T0(a0)
	sw	t1,MSS_T1(a0)
	mfhi	t0
	mflo	t1
	sw	v0,MSS_V0(a0)
	sw	v1,MSS_V1(a0)
	sw	t0,MSS_HI(a0)
	sw	t1,MSS_LO(a0)
	sw	t2,MSS_T2(a0)
	sw	t3,MSS_T3(a0)
	sw	t4,MSS_T4(a0)
	sw	t5,MSS_T5(a0)
	sw	t6,MSS_T6(a0)
	sw	t7,MSS_T7(a0)
	sw	t8,MSS_T8(a0)
	j	ra
	sw	t9,MSS_T9(a0)
	END(save_registers)

/*
 *	Object:
 *		TRAP_tlb_miss			EXPORTED function
 *
 *		Handles read/write tlb misses
 *
 *	Arguments:
 *		none
 *
 *	Save registers, get faulting address and invoke C handler.
 *	To get stack push/pop right, the C handler returns here with
 *	an error idication rather than invoking the generic trap
 *	handler directly.
 *	Note we keep interrupts disabled, C handler might change that.
 *	If everything is ok we jump to the restore_registers
 *	code which then jumps back to the exception epilogue
 *	code.  Complicated, but efficient.
 */
VECTOR(TRAP_tlb_miss, (MINSAVED|MEDSAVED))
	/*
	 *	a0 is exception frame pointer
	 *	a2 is cause register
	 */
	mfc0	a1,c0_tlbbad
	jal	save_registers
	sw	a1,MSS_BAD(a0)
	jal	tlb_miss		/* jump to C code */
	nop
	lw	a0,MEL_EFRAME(sp)
	beq	v0,zero,restore_registers	/* returns 0 if ok, else */
	nop
					/* hand it to the generic trap handler */
	lw	a2,MSS_SR(a0)		/* reload status where expected */
	b	TRAP_generic_nosave	/* alternate for TRAP_generic */
	move	a1,v0			/* cause */
	END(TRAP_tlb_miss)

/*
 *	Object:
 *		TRAP_interrupt			EXPORTED function
 *
 *		Handles I/O interrupts
 *
 *	Arguments:
 *		none
 *
 *	Keep interrupts disabled, priority will be set correctly
 *	from C handler.
 *	In case this is an fpa interrupt we might have to eventually
 *	give the thread an exception, therefore we have to save
 *	all registers just in case. There is obviously no need
 *	to restore the callee-saved ones at the end.
 */
#define S_REGISTERS (M_S0|M_S1|M_S2|M_S3|M_S4|M_S5|M_S6|M_S7|M_FP)

VECTOR(TRAP_interrupt, (MINSAVED|MEDSAVED|S_REGISTERS))
	/*
	 *	a0 is exception frame pointer
	 *	a2 is status register
	 *	k1 is cause register
	 */
	jal	save_registers
	move	a1,k1
	.set	noat
	/*
	 *	a0 is exception frame pointer
	 *	a1 is cause register
	 *	a2 is status register
	 */
	and	AT,a1,IP_LEV_FPA
	beq	AT,zero,1f
	nop
	.set	at
	sw	s0,MSS_S0(a0)
	sw	s1,MSS_S1(a0)
	sw	s2,MSS_S2(a0)
	sw	s3,MSS_S3(a0)
	sw	s4,MSS_S4(a0)
	sw	s5,MSS_S5(a0)
	sw	s6,MSS_S6(a0)
	sw	s7,MSS_S7(a0)
	sw	fp,MSS_FP(a0)
1:	jal	interrupt
	nop
	lw	a0,MEL_EFRAME(sp)
	b	restore_registers
	nop
	END(TRAP_interrupt)


/*
 *	Object:
 *		TRAP_mod			EXPORTED function
 *
 *		Handles page modified exceptions
 *
 *	Arguments:
 *		none
 *
 *	Save registers and transfer to C code.
 *	Interrupts disabled, C handler will lower priority
 *	if necessary.
 *	If C handler says it is illegal handle as generic trap.
 */
VECTOR(TRAP_mod, (MINSAVED|MEDSAVED))
	/*
	 *	a0 is exception frame pointer
	 *	a1 is tlbbad register (in a cycle)
	 */
	jal	save_registers
	sw	a1,MSS_BAD(a0)
	jal	tlb_mod
	nop
	lw	a0,MEL_EFRAME(sp)
	beq	v0,zero,restore_registers
	nop
	lw	a2,MSS_SR(a0)		/* status where expected */
	b	TRAP_generic_nosave
	move	a1,v0			/* cause */
	END(TRAP_mod)


/*
 *	Object:
 *		TRAP_syscall			EXPORTED function
 *
 *		System calls
 *
 *	Arguments:
 *		none
 */
VECTOR(TRAP_syscall, MINSAVED)
	.extern	mach_trap_table
	IMPORT(mach_trap_count,4)
	/*
	 *	a0 is exception frame pointer
	 *	a1 is cause register
	 *	a2 is epc register
	 *	k0 is status register
	 *	k1 is cause register
	 *	v0 is syscall number
	 */
	.set	noat
	bltz	k1,TRAP_generic		/* not if BD */
					/* check for emulated syscalls */
	lw	k1,current_dispatch	/* N cycles */
	ori	k0,SR_IEc
	beq	k1,zero,syscall_native	/* min overhead: 4+N cycles */
	mtc0	k0,c0_status		/* enable interrupts */
	move	a1,k1			/* (still interrupt safe) */
	lw	AT,DISP_MIN(a1)		/* get minimum syscall number */
	lw	ra,DISP_COUNT(a1)	/* get number of syscalls */
	subu	AT,v0,AT		/* (syscall number - min) */
	sltu	ra,ra,AT		/* > count (unsigned comparison!) */
	bne	ra,zero,syscall_native	/* native if so (> count or < 0) */
	sll	AT,2			/* convert to word index */
	addu	a1,AT			/* index eml vector */
	lw	a1,DISP_VECTOR(a1)	/* PC to return to for emulation */
	.set	at
	nop
	beq	a1,zero,syscall_native	/* not emulated */
	sw	a1,MSS_PC(a0)		/* dirty trick (if branch) */
	b	fast_exception_end	/* take no AST */
	addu	v0,a2,4			/* PC to return after emulation */

syscall_native:
	addu	a2,4			/* advance user's pc */
	li	a1,-25			/* check for mach_msg_trap */
	beq	v0,a1,syscall_mach_msg
	sw	a2,MSS_PC(a0)
	lw	a1,mach_trap_count
	negu	v0			/* get the id right */
	bleu	v0,a1,1f
	sll	v0,4			/* sizeof struct mach_trap_t */
	b	TRAP_exception_exit	/* invalid trap */
	li	v0,KERN_FAILURE
1:
	/*
	 *	Load trap descriptor, and check the number of args.
	 *	Entries in the table are defined by:
	 * 		typedef struct {
	 *			int		mach_trap_nargs;
	 *			int		(*mach_trap_function)();
	 *			int		mach_trap_stack;
	 *			int		mach_trap_unused;
	 * 		} mach_trap_t;
	 */
	la	a1,mach_trap_table
	addu	a1,v0			/* get trap */
	lw	v0,8(a1)		/* get stack-discard indication */
	lw	v1,0(a1)		/* trap->mach_trap_nargs */
	beq	zero,v0,1f
	lw	v0,4(a1)		/* trap->mach_trap_function */
	/*
	 *	Save registers, because the trap might discard
	 *	its kernel stack.
	 */
	sw	s0,MSS_S0(a0)
	sw	s1,MSS_S1(a0)
	sw	s2,MSS_S2(a0)
	sw	s3,MSS_S3(a0)
	sw	s4,MSS_S4(a0)
	sw	s5,MSS_S5(a0)
	sw	s6,MSS_S6(a0)
	sw	s7,MSS_S7(a0)
	sw	fp,MSS_FP(a0)
1:
	.set	noat			/* assembler not smart enough */
	beq	v1,zero,1f		/* many traps take 0 args */
	sltiu	AT,v1,5			/* more than 4 args ? */
	lw	a1,MSS_A1(a0)
	lw	a2,MSS_A2(a0)
	lw	a3,MSS_A3(a0)
	bne	AT,zero,1f
	lw	a0,MSS_A0(a0)
	.set	at
	/*
	 * Warning: This now supports 11 arguments (SLOWLY)
	 */
	subu	sp,28			/* uhhmm, > 4 args */
	sw	t0,16(sp)		/* extra args in tN */
	sw	t1,20(sp)
	sw	t2,24(sp)
	sw	t3,28(sp)
	sw	t4,32(sp)
	sw	t5,36(sp)
	jal	v0			/* do the call */
	sw	t6,40(sp)

	lw	a0,MEL_EFRAME+28(sp)
	b	TRAP_exception_exit
	addu	sp,28

1:	jal	v0			/* do the call */
	nop

	lw	a0,MEL_EFRAME(sp)
	b	TRAP_exception_exit
	nop

syscall_mach_msg:
	/*
	 *	Save registers, because the trap might discard
	 *	its kernel stack.
	 */
	sw	s0,MSS_S0(a0)
	sw	s1,MSS_S1(a0)
	sw	s2,MSS_S2(a0)
	sw	s3,MSS_S3(a0)
	sw	s4,MSS_S4(a0)
	sw	s5,MSS_S5(a0)
	sw	s6,MSS_S6(a0)
	sw	s7,MSS_S7(a0)
	sw	fp,MSS_FP(a0)
	/*
	 *	Prepare arguments.  We only put three args on the stack,
	 *	but we adjust by 28 bytes, like other syscalls,
	 *	to simplify tracing back through syscalls.
	 */
	lw	a1,MSS_A1(a0)
	lw	a2,MSS_A2(a0)
	lw	a3,MSS_A3(a0)
	lw	a0,MSS_A0(a0)
	subu	sp,28
	sw	t0,16(sp)
	sw	t1,20(sp)
	jal	mach_msg_trap
	sw	t2,24(sp)

	/*
	 *	If we are returning through this code,
	 *	then the trap did not discard its kernel stack.
	 */
	lw	a0,MEL_EFRAME+28(sp)
	b	TRAP_exception_exit
	addu	sp,28
	END(TRAP_syscall)

LEAF(thread_syscall_return)
	move	v0,a0				/* argument is return code */
	ori	t0,sp,KERNEL_STACK_SIZE-1	/* get exception frame */
	lw	a0,MEL_EFRAME-MEL_SIZE-MKS_SIZE-MSB_SIZE+1(t0)
	addiu	sp,t0,-MEL_SIZE-MKS_SIZE-MSB_SIZE+1	/* pop the stack */
	lw	s0,MSS_S0(a0)
	lw	s1,MSS_S1(a0)
	lw	s2,MSS_S2(a0)
	lw	s3,MSS_S3(a0)
	lw	s4,MSS_S4(a0)
	lw	s5,MSS_S5(a0)
	lw	s6,MSS_S6(a0)
	lw	s7,MSS_S7(a0)
	b	TRAP_exception_exit
	lw	fp,MSS_FP(a0)
	END(thread_syscall_return)

LEAF(thread_bootstrap_return)
	ori	t0,sp,KERNEL_STACK_SIZE-1	/* get exception frame */
	lw	a0,MEL_EFRAME-MEL_SIZE-MKS_SIZE-MSB_SIZE+1(t0)
	addiu	sp,t0,-MEL_SIZE-MKS_SIZE-MSB_SIZE+1	/* pop the stack */
	lw	s0,MSS_S0(a0)
	lw	s1,MSS_S1(a0)
	lw	s2,MSS_S2(a0)
	lw	s3,MSS_S3(a0)
	lw	s4,MSS_S4(a0)
	lw	s5,MSS_S5(a0)
	lw	s6,MSS_S6(a0)
	lw	s7,MSS_S7(a0)
	b	restore_registers
	lw	fp,MSS_FP(a0)
	END(thread_bootstrap_return)

LEAF(call_continuation)
	ori	t0,sp,KERNEL_STACK_SIZE-1
	addiu	sp,t0,-MEL_SIZE-MKS_SIZE-MSB_SIZE+1	/* pop the stack */
	j	a0					/* call continuation */
	move	ra,zero					/* for backtraces */
	END(call_continuation)

/*
 *	Object:
 *		TRAP_ri				EXPORTED function
 *
 *		Illegal instruction
 *
 *	Arguments:
 *		none
 *
 *	Check for Mach pseudo instructions, if not let the
 *	generic handler take care of it.
 */
VECTOR(TRAP_ri, MINSAVED)
	/*
	 *	a0 is exception frame pointer
	 *	a1 is cause register
	 *	a2 is epc register
	 *	k0 is status register
	 *	k1 is cause register
	 */
	bltz	k1,TRAP_generic		/* not if BD */
	.set	noat
	li	AT,op_tas		/* load offending instruction */
	lw	a1,0(a2)		/* this cannot miss, right? */
	nop
	bne	a1,AT,TRAP_generic
	move	a1,k1
	.set	at
	/*
	 * Hey, a TAS instruction!
	 *
	 * Emulate it, keeping interrupts disabled should
	 * do the trick.
	 * There is a subtle semantic question about a tas
	 * if it fails: does it actually write or not?
	 * For instance, should a user that does a tas
	 * on e.g. his text segment get a bus error or not?
	 * I think he should.
	 */
	lw	a1,MSS_A0(a0)		/* get address of lock */
	nop
	bgez	a1,addr_ok		/* sneaking into kernel space ? */
	nop
	sw	a1,MSS_BAD(a0)		/* that's an ade exception */
	b	TRAP_generic
	li	a1,EXC_ADEL
addr_ok:
	lw	ra,the_current_thread
	la	a3,uaerror		/* might fault */
	sw	a3,THREAD_RECOVER(ra)
	lw	a3,0(a1)		/* 	.. TEST .. */
	nop
	sw	a1,0(a1)		/* 	.. AND SET */

	sw	zero,THREAD_RECOVER(ra)
	sw	a3,MSS_A0(a0)		/* return the result still in a0 */
	/*
	 * All done, increment PC and return
	 */
	addiu	a2,4			/* increment PC */
	b	fast_exception_end
	sw	a2,MSS_PC(a0)

uaerror:
	/*
	 * User gave a bad address.  Give him an exception in trap()
	 * that stems from a write miss.
	 */
	sw	a1,MSS_BAD(a0)
	li	a1,EXC_TLBS
	lw	k0,MSS_SR(a0)
	b	TRAP_generic
	sw	a1,MSS_CAUSE(a0)
	END(TRAP_ri)


/*
 *	Object:
 *		TRAP_cu				EXPORTED function
 *
 *		Coprocessor unusable (or reserved instruction)
 *
 *	Arguments:
 *		none
 *
 *	The kernel only uses coprocessor 0 (no FP code!).
 *	Coprocessor 0 is always usable for kernel, so if this
 *	is the one it is a reserved instruction.
 *	We currently only know of coprocessor 1 (fpa), but
 *	there is room for others.
 *	Enabling of interrupts is left to the coproc handler.
 */
VECTOR(TRAP_cu, MINSAVED)
	/*
	 *	a0 is exception frame pointer
	 *	a1 is cause register
	 *	k0 is status register
	 *	k1 is cause register
	 */
	srl	k1,CAUSE_CE_SHIFT-3
	andi	k1,CAUSE_CE_MASK>>(CAUSE_CE_SHIFT-3)
	la	a2,coproc_table
	addu	a2,k1
	j	a2
	move	a2,k0
	/*
	 *	After dispatching, we have
	 *	a0 is exception frame pointer
	 *	a1 is cause register
	 *	a2 is status register
	 *	k0 is status register
	 */
coproc_table:
	j	TRAP_generic
	nop
	j	fpa_unusable
	nop
	j	TRAP_generic
	nop
	j	TRAP_generic
	nop

fpa_unusable:
	lw	a3,fpa_type
	nop
	bne	a3,zero,1f		/* but do we have one ? */
	nop
	jal	fpa_broken		/* no fpa! ..or a broken one.. */
	nop

1:
	lw	a3,current_pcb
	nop
	lw	a3,MSS_SIZE+MMS_MFS(a3)
	nop
	bne	a3,zero,1f		/* do we have FP state? */
	nop
	j	TRAP_generic		/* to allocate the memory */
	nop

1:
	/*
	 *	Now we can take tlb misses,
	 *	because we don't need k0/k1.
	 */
	or	a3,a2,SR_CU1		/* make fpa usable for user */
	lw	a1,the_current_thread	/* is the current thread the */
	lw	a2,fpa_owner		/*  one currently owning fpa ? */
	sw	a3,MSS_SR(a0)
	beq	a2,a1,fast_exception_end/* how lucky. Take no AST. */
	nop
	li	a3,SR_CU1|SR_KUo	/* but is there an owner? */
	beq	a2,zero,fpa_free	/* no need to save unused fpa status */
	mtc0	a3,c0_status		/* enable fpa for kernel */

	/*
	 *	Before stealing the fpa, save its state
	 *
	 *	[Kane]: The floating-point control and status register 
	 *	must be read first to force all fpa pipelines to drain.
	 */
	nop
	lw	a3,THREAD_PCB(a2)	/* get owner's pcb */
	nop
	lw	a2,MSS_SR(a3)		/* get owner's status */
	nop
	and	a2,~SR_CU1
	sw	a2,MSS_SR(a3)		/* disable fpa for owner */
	cfc1	a2,fpa_csr
	lw	a3,MSS_SIZE+MMS_MFS(a3)	/* get owner's save area */
	nop
	sw	a2,MFS_CSR(a3)
	cfc1	a2,fpa_eir
	nop
	sw	a2,MFS_EIR(a3)
	swc1	$f31, MFS_REGS+31*4(a3)
	swc1	$f30, MFS_REGS+30*4(a3)
	swc1	$f29, MFS_REGS+29*4(a3)
	swc1	$f28, MFS_REGS+28*4(a3)
	swc1	$f27, MFS_REGS+27*4(a3)
	swc1	$f26, MFS_REGS+26*4(a3)
	swc1	$f25, MFS_REGS+25*4(a3)
	swc1	$f24, MFS_REGS+24*4(a3)
	swc1	$f23, MFS_REGS+23*4(a3)
	swc1	$f22, MFS_REGS+22*4(a3)
	swc1	$f21, MFS_REGS+21*4(a3)
	swc1	$f20, MFS_REGS+20*4(a3)
	swc1	$f19, MFS_REGS+19*4(a3)
	swc1	$f18, MFS_REGS+18*4(a3)
	swc1	$f17, MFS_REGS+17*4(a3)
	swc1	$f16, MFS_REGS+16*4(a3)
	swc1	$f15, MFS_REGS+15*4(a3)
	swc1	$f14, MFS_REGS+14*4(a3)
	swc1	$f13, MFS_REGS+13*4(a3)
	swc1	$f12, MFS_REGS+12*4(a3)
	swc1	$f11, MFS_REGS+11*4(a3)
	swc1	$f10, MFS_REGS+10*4(a3)
	swc1	$f9,  MFS_REGS+ 9*4(a3)
	swc1	$f8,  MFS_REGS+ 8*4(a3)
	swc1	$f7,  MFS_REGS+ 7*4(a3)
	swc1	$f6,  MFS_REGS+ 6*4(a3)
	swc1	$f5,  MFS_REGS+ 5*4(a3)
	swc1	$f4,  MFS_REGS+ 4*4(a3)
	swc1	$f3,  MFS_REGS+ 3*4(a3)
	swc1	$f2,  MFS_REGS+ 2*4(a3)
	swc1	$f1,  MFS_REGS+ 1*4(a3)
	swc1	$f0,  MFS_REGS+ 0*4(a3)
fpa_free:
	/*
	 *	Reload fpa state
	 */
	sw	a1,fpa_owner		/* got it */
	lw	a3,current_pcb
	nop
	lw	a3,MSS_SIZE+MMS_MFS(a3)	/* get our save area */
	nop
	lwc1	$f31, MFS_REGS+31*4(a3)
	lwc1	$f30, MFS_REGS+30*4(a3)
	lwc1	$f29, MFS_REGS+29*4(a3)
	lwc1	$f28, MFS_REGS+28*4(a3)
	lwc1	$f27, MFS_REGS+27*4(a3)
	lwc1	$f26, MFS_REGS+26*4(a3)
	lwc1	$f25, MFS_REGS+25*4(a3)
	lwc1	$f24, MFS_REGS+24*4(a3)
	lwc1	$f23, MFS_REGS+23*4(a3)
	lwc1	$f22, MFS_REGS+22*4(a3)
	lwc1	$f21, MFS_REGS+21*4(a3)
	lwc1	$f20, MFS_REGS+20*4(a3)
	lwc1	$f19, MFS_REGS+19*4(a3)
	lwc1	$f18, MFS_REGS+18*4(a3)
	lwc1	$f17, MFS_REGS+17*4(a3)
	lwc1	$f16, MFS_REGS+16*4(a3)
	lwc1	$f15, MFS_REGS+15*4(a3)
	lwc1	$f14, MFS_REGS+14*4(a3)
	lwc1	$f13, MFS_REGS+13*4(a3)
	lwc1	$f12, MFS_REGS+12*4(a3)
	lwc1	$f11, MFS_REGS+11*4(a3)
	lwc1	$f10, MFS_REGS+10*4(a3)
	lwc1	$f9,  MFS_REGS+ 9*4(a3)
	lwc1	$f8,  MFS_REGS+ 8*4(a3)
	lwc1	$f7,  MFS_REGS+ 7*4(a3)
	lwc1	$f6,  MFS_REGS+ 6*4(a3)
	lwc1	$f5,  MFS_REGS+ 5*4(a3)
	lwc1	$f4,  MFS_REGS+ 4*4(a3)
	lwc1	$f3,  MFS_REGS+ 3*4(a3)
	lwc1	$f2,  MFS_REGS+ 2*4(a3)
	lwc1	$f1,  MFS_REGS+ 1*4(a3)
	lwc1	$f0,  MFS_REGS+ 0*4(a3)
	ctc1	zero,fpa_csr		/* clear status [Kane] */
	lw	a1,MFS_EIR(a3)
	nop
	ctc1	a1,fpa_eir		/* restore trap status */
	lw	a1,MFS_CSR(a3)
	b	fast_exception_end	/* take no AST */
	ctc1	a1,fpa_csr		/* restore true fpa status */
	/* fpa will be disabled for kernel in the */
	/* standard epilogue code [caveat!] */

	END(TRAP_cu)


/*
 *	Object:
 *		TRAP_generic			EXPORTED function
 *
 *		All other exceptions
 *
 *	Arguments:
 *		none
 *
 *	This is the only handler that needs to fully save the
 *	status, as the thread might be on its way to a fatal
 *	exception.
 */
VECTOR(TRAP_generic, (MINSAVED|MEDSAVED|S_REGISTERS))
	/*
	 *	a0 is exception frame pointer
	 *	a1 is cause register
	 *	k0 is status register
	 */
	jal	save_registers
	move	a2,k0

TRAP_generic_nosave:
	/*
	 *	a0 is exception frame pointer
	 *	a1 is cause register
	 *	a2 is status register
	 *	save_registers has been called
	 */
	ori	a2,SR_IEc
	mtc0	a2,c0_status		/* enable interrupts */
					/* save full status */
	sw	s0,MSS_S0(a0)
	sw	s1,MSS_S1(a0)
	sw	s2,MSS_S2(a0)
	sw	s3,MSS_S3(a0)
	sw	s4,MSS_S4(a0)
	sw	s5,MSS_S5(a0)
	sw	s6,MSS_S6(a0)
	sw	s7,MSS_S7(a0)
	jal	trap			/* to C handler */
	sw	fp,MSS_FP(a0)

	lw	a0,MEL_EFRAME(sp)
	nop
	lw	s0,MSS_S0(a0)		/* full reload */
	lw	s1,MSS_S1(a0)
	lw	s2,MSS_S2(a0)
	lw	s3,MSS_S3(a0)
	lw	s4,MSS_S4(a0)
	lw	s5,MSS_S5(a0)
	lw	s6,MSS_S6(a0)
	lw	s7,MSS_S7(a0)
	b	restore_registers
	lw	fp,MSS_FP(a0)
	END(TRAP_generic)


/*
 *	Object:
 *		TRAP_ast			EXPORTED function
 *
 *		An AST.
 */
VECTOR(TRAP_ast, (MINSAVED|MEDSAVED|S_REGISTERS))
	/*
	 *	a0 is exception frame pointer
	 *
 	 *	We must save full status, because ast_taken
	 *	might wish to discard its kernel stack.
	 *	We call ast_taken with interrupts still disabled.
	 */

	jal	save_registers
	sw	s0,MSS_S0(a0)
	sw	s1,MSS_S1(a0)
	sw	s2,MSS_S2(a0)
	sw	s3,MSS_S3(a0)
	sw	s4,MSS_S4(a0)
	sw	s5,MSS_S5(a0)
	sw	s6,MSS_S6(a0)
	sw	s7,MSS_S7(a0)
	jal	ast_taken
	sw	fp,MSS_FP(a0)

	lw	a0,MEL_EFRAME(sp)
	b	restore_registers
	nop
	END(TRAP_ast)


/*
 *	Object:
 *		TRAP_ade			EXPORTED function
 *
 *		Unaligned load/stores, and user illegal
 *
 *	Arguments:
 *		none
 *
 *	Get bad address, handle as generic trap.
 */
VECTOR(TRAP_ade, MINSAVED)
	/*
	 *	a0 is exception frame pointer
	 *	k0 is status register
	 *	k1 is cause register
	 *	ra is tlbbad (in a cycle)
	 */
	move	a1,k1
	b	TRAP_generic
	sw	ra,MSS_BAD(a0)
	END(TRAP_ade)


/*
 *	Object:
 *		TRAP_ibe			EXPORTED function
 *
 *		Bus error on instruction fetch
 *
 *	Arguments:
 *		none
 *
 *	Set bad_vaddr, then proceed to the generic handler
 */
VECTOR(TRAP_ibe, MINSAVED)
	/*
	 *	a0 is exception frame pointer
	 *	a1 is epc (in a cycle)
	 *	k0 is status register
	 *	k1 is cause register
	 */
	bgez	k1,1f		/* account for BD slot */
	nop
	addiu	a1,4
1:	sw	a1,MSS_BAD(a0)
TRAP_dbe:
	/*
	 *	a0 is exception frame pointer
	 *	a1 is epc (possibly in a cycle)
	 *	k0 is status register
	 *	k1 is cause register
	 */
	lw	a2,mips_box_buserror
	move	a1,k1
	j	a2
	nop
	END(TRAP_ibe)

	.sdata
EXPORT(rdclr_buserror_address)	.word	RDCLR_BUSERR
	.text

LEAF(rdclr_buserror)
	/* Must read-acknowledge */
	lw	a2, rdclr_buserror_address
	b	TRAP_generic
	lw	zero, 0(a2)
	END(rdclr_buserror)

	.sdata
EXPORT(wrclr_buserror_address)	.word	WRCLR_BUSERR
	.text

LEAF(wrclr_buserror)
	/* Must write-acknowledge */
	lw	a2, wrclr_buserror_address
	la	ra, TRAP_generic	/* no need to return here */
	j	wbflush
	sw	zero, 0(a2)
	END(wrclr_buserror)

	.globl	mips_box_buserror
mips_box_buserror:	.word	rdclr_buserror


/*
 *	Object:
 *		TRAP_bp				EXPORTED function
 *
 *		Break instruction
 *
 *	Arguments:
 *		none
 *
 *	Find out whether instruction is for KDB or user.
 *	Former is handled locally, latter is handled as
 *	a common generic trap.
 */
VECTOR(TRAP_bp, MINSAVED)
	/*
	 *	a0 is exception frame pointer
	 *	a1 is epc (possibly in a cycle)
	 *	k0 is status register
	 *	k1 is cause register
	 */
	bgez	k1,1f			/* account for BD case */
	nop
	addiu	a1,4
1:	lw	a1,0(a1)
	.set	noat
	lw	AT,gimmeabreak		/* kernel BP ? */
	nop
	bne	a1,AT,TRAP_generic	/* nope */
	li	a1,EXC_BP
	.set	at
#if	DEBUG_USERS
					/* trojan horse! */
#else	DEBUG_USERS
	andi	a1,k0,SR_KUo
	bne	a1,zero,TRAP_generic
	li	a1,EXC_BP
#endif	DEBUG_USERS
	lw	a1,+RB_BPADDR		/* debugger available ? */
	nop
	beq	a1,zero,TRAP_generic
	li	a1,EXC_BP
#if	DEBUG
	sw	zero,exc_nesting_check
#endif	DEBUG
#if	DEBUG_USERS
	andi	a1,k0,SR_KUo
	beq	a1,zero,1f
	nop
	lw	gp,MSS_GP(a0)
#endif	DEBUG_USERS
1:	lw	a1,MSS_A1(a0)		/* restore clobbered registers */
	lw	a2,MSS_A2(a0)
	lw	a3,MSS_A3(a0)
	lw	k1,MSS_K1(a0)
	/*
	 *	The following strange calling sequence is for
	 *	compatibility with the MIPS prom debugger.
	 */
	.set	noat
	lw	AT,+RB_BPADDR		/* address of breakpoint handler */
	lw	k0,MSS_AT(a0)		/* save AT in k0 */
	lw	sp,MSS_SP(a0)
	j	AT			/* enter breakpoint handler */
	lw	a0,MSS_A0(a0)
	.set	at
	END(TRAP_bp)

/*
 *	Object:
 *		TRAP_reserved			EXPORTED function
 *
 *	Save as much state as possible and panic.
 *
 *	Arguments:
 *		none
 *
 */
VECTOR(TRAP_reserved, (~M_K0))
	/*
	 *	a0 is exception frame pointer
	 *	a1 is tlbbad (in a cycle)
	 *	k0 is status register
	 *	k1 is cause register
	 */
	jal	save_registers
	sw	a1,MSS_BAD(a0)
	.set	reorder
	sw	s0,MSS_S0(a0)
	sw	s1,MSS_S1(a0)
	sw	s2,MSS_S2(a0)
	sw	s3,MSS_S3(a0)
	sw	s4,MSS_S4(a0)
	sw	s5,MSS_S5(a0)
	sw	s6,MSS_S6(a0)
	sw	s7,MSS_S7(a0)
	sw	fp,MSS_FP(a0)
	PANIC("TRAP_reserved")
	.set	noreorder
	END(TRAP_reserved)



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
LEAF(gimmeabreak)
	break	BREAK_KERNEL
	j	ra
	nop
	/*
	 *	One more nop is needed to allow the user to step out
	 *	of gimmeabreak.  This is because the single-step code
	 *	will put a breakpoint here (and on $ra).  We obviously
	 *	can't allow a breakpoint in kdb_breakpoint.
	 */
	nop

	END(gimmeabreak)


#if	MACH_KDB
/*
 *	Object:
 *		kdb_breakpoint			EXPORTED function
 *
 *		Kernel breakpoint trap, and warm restart
 *
 *	Arguments:
 *		none, but AT must be in k0
 *
 *	Save as much state as possible on stack including TLB
 *	status, invoke debugger.  On return, restore and continue.
 */
VECTOR(kdb_breakpoint, MINSAVED)
 	.set	noat
	nop				/* wait for possible delay cycle */
	bltz	sp,1f			/* DEBUG_USERS & sanity */
	nop

	/*
	 *	AT is saved in k0
	 *	We are coming from user mode.  We must switch
	 *	stacks and use the exception frame in the pcb.
	 */
	lw	AT,current_pcb		/* exc frame is in pcb */
	nop
	sw	k0,MSS_AT(AT)
	sw	sp,MSS_SP(AT)
	sw	a0,MSS_A0(AT)
	lw	sp,current_kstack	/* exception link already set up */
	b	2f
	move	a0,AT

1:
	/*
	 *	AT is saved in k0
	 *	We are coming from kernel mode, so we can just
	 *	push an exception frame on the existing stack.
	 */
	sw	sp,MSS_SP-MSS_SIZE(sp)	/* push exception frame on stack */
	sw	a0,MSS_A0-MSS_SIZE(sp)
	addiu	a0,sp,-MSS_SIZE
	addiu	sp,a0,-MEL_SIZE		/* push exception link on stack */
	sw	a0,MEL_EFRAME(sp)
	sw	k0,MSS_AT(a0)

2:
	/*
	 *	exception frame/link set up
	 *	a0, AT, sp saved in exception frame
	 *	a0 is exception frame
	 *	sp is kernel stack
	 */
	.set	at
	li	k0,0xbad00bad		/* signify k0 is lost */
	sw	k0,MSS_K0(a0)
	sw	gp,MSS_GP(a0)		/* switch _gp, just in case */
	la	gp,_gp
	sw	k1,MSS_K1(a0)
	sw	v0,MSS_V0(a0)
	sw	v1,MSS_V1(a0)
	sw	a1,MSS_A1(a0)
	sw	a2,MSS_A2(a0)
	sw	a3,MSS_A3(a0)
	sw	t0,MSS_T0(a0)
	sw	t1,MSS_T1(a0)
	sw	t2,MSS_T2(a0)
	sw	t3,MSS_T3(a0)
	sw	t4,MSS_T4(a0)
	sw	t5,MSS_T5(a0)
	sw	t6,MSS_T6(a0)
	sw	t7,MSS_T7(a0)
	sw	s0,MSS_S0(a0)
	sw	s1,MSS_S1(a0)
	sw	s2,MSS_S2(a0)
	sw	s3,MSS_S3(a0)
	sw	s4,MSS_S4(a0)
	sw	s5,MSS_S5(a0)
	sw	s6,MSS_S6(a0)
	sw	s7,MSS_S7(a0)
	sw	t8,MSS_T8(a0)
	sw	t9,MSS_T9(a0)
	sw	fp,MSS_FP(a0)
	sw	ra,MSS_RA(a0)
	mflo	v0
	mfhi	v1
	sw	v0,MSS_LO(a0)
	sw	v1,MSS_HI(a0)
	mfc0	v0,c0_epc
	mfc0	v1,c0_status
	sw	v0,MSS_PC(a0)
	sw	v1,MSS_SR(a0)
	mfc0	v0,c0_tlbbad
	mfc0	v1,c0_cause
	sw	v0,MSS_BAD(a0)
	sw	v1,MSS_CAUSE(a0)
	mfc0	v0,c0_tlblo
	mfc0	v1,c0_tlbhi
	sw	v0,MSS_TLB_LO(a0)
	sw	v1,MSS_TLB_HI(a0)
	mfc0	v0,c0_tlbind
	mfc0	v1,c0_tlbcxt
	sw	v0,MSS_TLB_INX(a0)
	sw	v1,MSS_TLB_CTX(a0)

	jal	kdb_trap		/* kdb_trap(mips_saved_state,0) */
	move	a1,zero

	lw	a0,MEL_EFRAME(sp)	/* reload all regs from saved state */
	nop
	lw	a1,MSS_A1(a0)
	lw	a2,MSS_A2(a0)
	lw	a3,MSS_A3(a0)
	lw	t0,MSS_T0(a0)
	lw	t1,MSS_T1(a0)
	lw	t2,MSS_T2(a0)
	lw	t3,MSS_T3(a0)
	lw	t4,MSS_T4(a0)
	lw	t5,MSS_T5(a0)
	lw	t6,MSS_T6(a0)
	lw	t7,MSS_T7(a0)
	lw	s0,MSS_S0(a0)
	lw	s1,MSS_S1(a0)
	lw	s2,MSS_S2(a0)
	lw	s3,MSS_S3(a0)
	lw	s4,MSS_S4(a0)
	lw	s5,MSS_S5(a0)
	lw	s6,MSS_S6(a0)
	lw	s7,MSS_S7(a0)
	lw	t8,MSS_T8(a0)
	lw	t9,MSS_T9(a0)
	lw	k1,MSS_K1(a0)
	lw	fp,MSS_FP(a0)
	lw	ra,MSS_RA(a0)
	lw	v0,MSS_LO(a0)
	lw	v1,MSS_HI(a0)
	mtlo	v0
	mthi	v1
	lw	v0,MSS_TLB_INX(a0)
	lw	v1,MSS_TLB_LO(a0)
	mtc0	v0,c0_tlbind
	mtc0	v1,c0_tlblo
	lw	v0,MSS_TLB_HI(a0)
	lw	v1,MSS_TLB_CTX(a0)
	mtc0	v0,c0_tlbhi
	mtc0	v1,c0_tlbcxt
	lw	v0,MSS_CAUSE(a0)		/* might have SW bits! */
	lw	v1,MSS_SR(a0)
	mtc0	v0,c0_cause
	lw	v0,MSS_V0(a0)
#if	DEBUG_USERS
	.set	noat
	sll	AT,v1,31-SR_KUo_SHIFT	/* to user ? */
	bgez	AT,1f
	nop
	ori	v1,(SR_KUp|SR_IEp)
1:
#endif	DEBUG_USERS
					/* almost done, careful now */
	lw	AT,MSS_AT(a0)
	move	k0,a0
	lw	sp,MSS_SP(k0)
	lw	a0,MSS_A0(k0)
	lw	gp,MSS_GP(k0)
	mtc0	v1,c0_status
	lw	v1,MSS_V1(k0)
	lw	k0,MSS_PC(k0)
	nop
	j	k0
	rfe
	END(kdb_breakpoint)
	.set	at

/*
 *	Object:
 *		kdb_go				EXPORTED function
 *
 *		Emergency entry point to KDB
 *
 *	Arguments:
 *		none
 *
 *	Invoked from the PROM after pushing the loser's
 *	button.  Enters KDB with a preposterous state,
 *	but so that you can look around and understand.
 *	Re-installs exception handlers.
 */
NESTED(kdb_go,0,zero)
	la	gp,_gp
	la	sp,boot_stack

	/* tlb_umiss handler */
	la	a0,TRAP_tlb_umiss
	li	a1,VEC_TLB_UMISS
	la	a3,TRAP_tlb_umiss_end
	jal	bcopy
	sub	a3,a0

	/* general exception handler */
	la	a0,TRAP_exception
	li	a1,VEC_EXCEPTION
	la	a3,TRAP_exception_end
	jal	bcopy
	sub	a3,a0

	/* drop into kdb now, no return */
	b	kdb_breakpoint
	move	ra,zero
	END(kdb_go)

#endif	MACH_KDB

#if	DEBUG
/*
 *	Object:
 *		crashit				EXPORTED function
 *
 *		Save all registers and crash the machine
 *
 *	Arguments:
 *		none
 *
 */
LEAF(crashit)
	.set	noat
	la	AT,crash_area
	sw	v0,0(AT)
	sw	v1,4(AT)
	sw	a0,8(AT)
	sw	a1,12(AT)
	sw	a2,16(AT)
	sw	a3,20(AT)
	sw	t0,24(AT)
	sw	t1,28(AT)
	sw	t2,32(AT)
	sw	t3,36(AT)
	sw	t4,40(AT)
	sw	t5,44(AT)
	sw	t6,48(AT)
	sw	t7,52(AT)
	sw	s0,56(AT)
	sw	s1,60(AT)
	sw	s2,64(AT)
	sw	s3,68(AT)
	sw	s4,72(AT)
	sw	s5,76(AT)
	sw	s6,80(AT)
	sw	s7,84(AT)
	sw	t8,88(AT)
	sw	t9,92(AT)
	sw	k0,96(AT)
	sw	k1,100(AT)
	sw	gp,104(AT)
	sw	sp,108(AT)
	la	sp,boot_stack
	subu	sp,MSS_SIZE
	sw	fp,112(AT)
	sw	ra,116(AT)
	mflo	k0
	mfhi	k1
	sw	k0,120(AT)
	mfc0	k0,c0_tlbhi
	sw	k1,124(AT)
	mfc0	k1,c0_tlblo
	sw	k0,128(AT)
	mfc0	k0,c0_tlbind
	sw	k1,132(AT)
	mfc0	k1,c0_tlbcxt
	sw	k0,136(AT)
	mfc0	k0,c0_status
	sw	k1,140(AT)
	mfc0	k1,c0_tlbbad
	sw	k0,144(AT)
	mfc0	k0,c0_cause
	sw	k1,148(AT)
	mfc0	k1,c0_epc
	sw	k0,152(AT)
	sw	k1,156(AT)
	mtc0	zero,c0_status
	la	AT,halt
	j	AT
	.set	at
	END(crashit)
#endif	DEBUG
