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
 * $Log:	trap.h,v $
 * Revision 2.3  91/07/31  18:04:46  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:01:23  dbg
 * 	Added, from Sequent SYMMETRY sources.
 * 	[91/02/26            dbg]
 * 
 */

/*
 * $Header: trap.h,v 2.3 91/07/31 18:04:46 dbg Exp $
 *
 * trap.h
 *	Trap type values.
 *
 * Values of types match Intel 80386 vector numbers.
 */

/*
 * Revision 1.1  89/07/05  13:15:50  kak
 * Initial revision
 * 
 */

#ifndef	_SQT_TRAP_H_
#define	_SQT_TRAP_H_

/*
 * Interrupts, traps, and exceptions use the Interrupt Descriptor Table
 * (IDT), indexed for the particular interrupt, trap, or exception.
 *
 * There is a copy of this table per processor to avoid LOCK# contention
 * during IDT access (see plocal.h).
 */

#define	T_DIVERR	 0		/* integer zero-divide, divide error */
#define	T_DBG		 1		/* debug exceptions */
#define	T_NMI		 2		/* non-maskable interrupt */
#define	T_INT3		 3		/* single-byte interrupt (breakpoint) */
#define	T_INTO		 4		/* interrupt on overflow */
#define	T_CHECK		 5		/* array bounds check */
#define	T_UND		 6		/* undefined/illegal op-code */
#define	T_DNA		 7		/* device not available (FPU) */
#define	T_SYSERR	 8		/* system error (serious problem) */
#define	T_RES		 9		/* reserved vector (9,15) */
#define	T_BADTSS	10		/* invalid task-state segment */
#define	T_NOTPRES	11		/* segment/gate not present */
#define	T_STKFLT	12		/* stack fault */
#define	T_GPFLT		13		/* general protection fault */
#define	T_PGFLT		14		/* page fault */
#define	T_COPERR	16		/* co-processor error (FPU) */
/*
 * Intel reserves thru 31.
 */
#define	T_SWTCH		17		/* redispatch (SW only, no IDT access)*/
#define	T_USER		0x20		/* value to OR if USER trap */

/*
 * SLIC interrupt vectors.
 */

#define	T_BIN0		32		/* Bin 0 interrupt (SW interrupt) */
#define	T_BIN1		33		/* Bin 1 interrupt (SW interrupt) */
#define	T_BIN2		34		/* Bin 2 interrupt (SW interrupt) */
#define	T_BIN3		35		/* Bin 3 interrupt (SW interrupt) */
#define	T_BIN4		36		/* Bin 4 interrupt (SW interrupt) */
#define	T_BIN5		37		/* Bin 5 interrupt (SW interrupt) */
#define	T_BIN6		38		/* Bin 6 interrupt (SW interrupt) */
#define	T_BIN7		39		/* Bin 7 interrupt (SW interrupt) */

/*
 * System calls use "INT n" instructions to have same stack entry as
 * traps and exceptions.  There is one syscall entry per number of
 * arguments, to streamline argument copying.  Call gates don't work
 * due to different stack frame and the detail semantics (don't work
 * well thru syscall interfaces (libc)).
 */

#define	T_SVC0		40		/* 0-arg system call */
#define	T_SVC1		41		/* 1-arg system call */
#define	T_SVC2		42		/* 2-arg system call */
#define	T_SVC3		43		/* 3-arg system call */
#define	T_SVC4		44		/* 4-arg system call */
#define	T_SVC5		45		/* 5-arg system call */
#define	T_SVC6		46		/* 6-arg system call */

#ifdef	FPA
/*
 * Weitek FPA exception vectors.
 */

#define	T_FPA		64		/* FPA exception interrupt */
#endif	FPA

#define	T_MACH		128		/* Mach system call entry */

/*
 * IDT gets fully filled out (256 entries) -- see IDT_SIZE in machime/gdt.h.
 * Higher number entries are made illegal by vectoring all illegal entries
 * to "t_res" -- generates T_RES trap.
 */

#endif	/* _SQT_TRAP_H_ */
