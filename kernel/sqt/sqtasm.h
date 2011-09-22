/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * Copyright (c) 1991 Sequent Computer Systems
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND SEQUENT COMPUTER SYSTEMS ALLOW FREE USE OF
 * THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * SEQUENT COMPUTER SYSTEMS DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	sqtasm.h,v $
 * Revision 2.3  91/07/31  18:04:22  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:00:26  dbg
 * 	Changes for pure kernel.
 * 	[90/05/02            dbg]
 * 
 */

#ifndef	_SQT_SQTASM_H_
#define	_SQT_SQTASM_H_

#if	defined(KXX) || defined(SLIC_GATES)

/*
 * In-line ASM macros for portable gate and lock implementations.
 * Macros don't block interrupts; caller must do this.
 *
 * P_GATE_ASM	grab a gate, used in ASM code.
 * CP_GATE_ASM	conditionally grab a gate, used in ASM code.
 * V_GATE_ASM	release a gate in ASM code.
 *
 * These macros touch only EAX.
 */

/*
 * P_SLIC_GATE and V_SLIC_GATE lock/unlock slic gate whose number is in %al.
 * Since these may be called from (eg) p_gate() and cp_gate(), the must be
 * sure to save/restore interrupt enable flag.  This is ok in the prototype
 * (K20), and a non-issue on the real HW.
 */

#define	P_SLIC_GATE(gateno) \
	pushfl; cli; \
	movb	$GATE_GROUP, SL_DEST+VA_SLIC; \
	movb	gateno, SL_SMESSAGE+VA_SLIC; \
9:	movb	$SL_REQG, SL_CMD_STAT+VA_SLIC; \
8:	testb	$SL_BUSY, SL_CMD_STAT+VA_SLIC; \
	jne	8b; \
	testb	$SL_OK, SL_CMD_STAT+VA_SLIC; \
	je	9b

#define	V_SLIC_GATE(gateno) \
	movb	$GATE_GROUP, SL_DEST+VA_SLIC; \
	movb	gateno, SL_SMESSAGE+VA_SLIC; \
	movb	$SL_RELG, SL_CMD_STAT+VA_SLIC; \
9:	testb	$SL_BUSY, SL_CMD_STAT+VA_SLIC; \
	jne	9b; \
	popfl

#define	P_GATE_ASM(gaddr) \
	leal	gaddr, %eax; \
	shrb	$2, %al; \
	andb	$0x3f, %al; \
5:	cmpb	$G_UNLOCKED, gaddr; \
	jne	5b; \
	P_SLIC_GATE(%al); \
	movb	gaddr, %ah; \
	cmpb	$G_UNLOCKED, %ah; \
	jne	6f; \
	movb	$G_LOCKED, gaddr; \
6:	V_SLIC_GATE(%al); \
	cmpb	$G_UNLOCKED, %ah; \
	jne	5b

#define	CP_GATE_ASM(gaddr,fail) \
	leal	gaddr, %eax; \
	shrb	$2, %al; \
	andb	$0x3f, %al; \
	P_SLIC_GATE(%al); \
	movb	gaddr, %ah; \
	cmpb	$G_UNLOCKED, %ah; \
	jne	6f; \
	movb	$G_LOCKED, gaddr; \
6:	V_SLIC_GATE(%al); \
	cmpb	$G_UNLOCKED, %ah; \
	jne	fail

#define	V_GATE_ASM(gaddr)	movb	$G_UNLOCKED, gaddr

#else	Real HW

/*
 * Gate and lock accesses on real HW are in-line expanded.
 * See machine/mutex.h
 */

#define	V_GATE_ASM(gaddr) \
	movb	$G_UNLOCKED, %al; \
	xchgb	%al, gaddr

#endif	KXX

/*
 * V_LOCK_ASM() assumes gate and lock data-structure and values are idential.
 */

#define	V_LOCK_ASM(lock)	V_GATE_ASM(lock)

/*
 * SPL_ASM(new,old)	raise SPL to "new", put old value in "old" (mod's %ah).
 * SPLX_ASM(old)	lower SPL back to "old".
 *
 * See machine/intctl.h for detail on spl synch with SLIC.
 * need 8 clocks at 16Mhz 10 at 20 Mhz  12 at 24 Mhz etc
 * modelC requires an extra 50 ns
 * all machines may run at 10% margins.
 * Note: movb X,r	will contribute 2 cycles
 *	 nop	        is 3 cycles
 *	 movl	r,r	is 2 cycles
 */

#ifndef	MHz
#define	MHz	20
#endif	MHz

/***************SLICSYNC 2 ***************************************/
#if MHz == 16
#define	SPL_ASM(new,old) \
	movb	VA_SLIC+SL_LMASK, old; \
	movb	new, VA_SLIC+SL_LMASK; \
 	movb	VA_SLIC+SL_LMASK, %ah; \
	nop; nop;
#else
#if MHz == 20
#define	SPL_ASM(new,old) \
	movb	VA_SLIC+SL_LMASK, old; \
	movb	new, VA_SLIC+SL_LMASK; \
 	movb	VA_SLIC+SL_LMASK, %ah; \
	movl	%eax,%eax; \
	movl	%eax,%eax; \
	nop; nop; 
#else
	ERROR not 16Mhz nor 20Hz
#endif
#endif

#define	SPLX_ASM(old)	movb	old, VA_SLIC+SL_LMASK

/*
 * Put cpu_number() is passed arg
 */

#define CPU_NUMBER(arg) \
	movzbl	VA_SLIC+SL_PROCID, arg; \
	movzbl	_slic_to_cpu(arg), arg

/*
 * Put current_thread pointer into passed arg
 * See current_thread() defn in thread.h
 */

#define CURRENT_THREAD(arg) \
	CPU_NUMBER(arg); \
	movl	_active_threads(,arg,4), arg

/*
 * Put pointer to current pcb in arg
 */

#define CURRENT_PCB(arg) \
	CURRENT_THREAD(arg); \
	movl	THREAD_PCB(arg), arg

/*
 * Put pointer to engine[] structure in arg
 */

#define ENGINE_POINTER(arg) \
	CPU_NUMBER(arg); \
	imull	$ENGSIZE, arg; \
	addl	_engine, arg

/*
 * Put pointer to plocal structure in arg
 */

#define LOCAL_POINTER(arg) \
	ENGINE_POINTER(arg); \
	movl	E_LOCAL(arg), arg

#endif	/* _SQT_SQTASM_H_ */
