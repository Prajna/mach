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
 * $Log:	mutex.s,v $
 * Revision 2.3  91/07/31  18:03:23  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  12:58:26  dbg
 * 	Use entire word for lock to match lock primitives.
 * 	[91/02/05            dbg]
 * 
 * 	Put parentheses around substituted immediate expressions, so
 * 	that they will pass through the GNU preprocessor.
 * 	[91/01/31            dbg]
 * 
 * 	Adapted for pure kernel.
 * 	[90/09/24            dbg]
 * 
 */

/*
 * mutex.s
 *
 * Implements: simple locks, interlocked bit set
 * and clear, and bit locks.  Should eventually be moved to inline asm
 *
 * This code assumes that the caller uses spl to implement
 * interrupt mutex as needed.
 */

#include <machine/asm.h>
#include <assym.s>
#include <sqt/asm_macros.h>
#include <sqt/intctl.h>

/*
 * Simple locks occupy one word.  A value of zero is unlocked,
 * a value of one is locked.
 */

#define L_LOCKED	1
#define L_UNLOCKED	0
#define	CPLOCKFAIL	(-1)		/* invalid SPL value */

 /*
  * simple_lock_init(int *lock) - set the lock to the unlocked state
  */

ENTRY(simple_lock_init)
	movl	S_ARG0, %ecx		/ lock address
	movl	$(L_UNLOCKED), (%ecx)	/ unlock
	ret

 /*
  * simple_unlock(int *lock) - release a lock
  */

ENTRY(simple_unlock)
#ifdef	SIMPLE_LOCK_LOG
	pushl	S_ARG0
	pushl	$0
	call	_lock_log
	addl	$8, %esp
#endif	SIMPLE_LOCK_LOG
	movl	S_ARG0, %ecx		/ lock address
	movl	$(L_UNLOCKED), %eax	/ exchange value
	xchgl	%eax, (%ecx)		/ exchange lock and acc.
	ret

/*
 * simple_lock(int *lock) - set lock with busy wait
 */

ENTRY(simple_lock)
#ifdef	SIMPLE_LOCK_LOG
	pushl	S_ARG0
	pushl	$1
	call	_lock_log
	addl	$8, %esp
#endif	SIMPLE_LOCK_LOG
	movl	S_ARG0, %ecx		/ lock address
0:
	movl	$(L_LOCKED), %eax	/ exchange value
	xchgl	%eax, (%ecx)		/ exchange lock and acc.
	cmpl	$(L_UNLOCKED), %eax	/ test if was unlocked
	je	2f			/ jump if so - we have lock
1:
	cmpl	$(L_UNLOCKED), (%ecx)	/ otherwise, spin until
	jne	1b			/ value is UNLOCKED
	jmp	0b			/ and try again
2:
#ifdef	SIMPLE_LOCK_LOG
	pushl	S_ARG0
	pushl	$2
	call	_lock_log
	addl	$8, %esp
#endif	SIMPLE_LOCK_LOG
	ret				/ got lock, return 

/*
 * simple_lock_try(int *lock) - try setting lock.
 * Returns zero if unsuccessful and non-zero if successful.
 */

ENTRY(simple_lock_try)
	movl	S_ARG0, %ecx		/ lock address
	movl	$(L_LOCKED), %eax	/ exchange value
	xchgl	%eax, (%ecx)		/ exchange lock and acc.
	cmpl	$(L_UNLOCKED), %eax	/ was it unlocked?
	je	9f			/ jump if yes
	movl	$0, %eax		/ else, return 0
	ret
9:
	movb	$1, %al			/ non-zero return
	ret

/*
 * i_bit_set(int bitno, int *s) - atomically set bit in bit string
 */

ENTRY(i_bit_set)
	movl	S_ARG0, %ecx		/ bit number
	movl	S_ARG1, %eax		/ address
	lock
	btsl	%ecx, (%eax)		/ set bit
	ret

/*
 * i_bit_clear(int bitno, int *s) - atomically clear bit in bit string
 */

ENTRY(i_bit_clear)
ENTRY(bit_unlock)
	movl	S_ARG0, %ecx		/ bit number
	movl	S_ARG1, %eax		/ address
	lock
	btrl	%ecx, (%eax)		/ clear bit
	ret

/*
 * bit_lock(int bitno, int *s) - set a bit lock with busy wait
 */

ENTRY(bit_lock)
	movl	S_ARG0, %ecx		/ bit number
	movl	S_ARG1, %eax		/ address
1:
	lock
	btsl	%ecx, (%eax)		/ test and set bit
	jb	1b			/ if was set, loop	/* jc? */
	ret				/ otherwise have lock

/*
 * bit_lock_try(int bitno, int *s) - try to set bit lock.
 * Returns 0 on failure, 1 on success
 */

ENTRY(bit_lock_try)
	movl	S_ARG0, %ecx		/ bit number
	movl	S_ARG1, %eax		/ address
	lock
	btsl	%ecx, (%eax)		/ test and set bit
	jb	1f			/ jump, if was set	/* jc? */
	movl	$1, %eax 		/ else, return success
	ret
1:
	movl	$0,  %eax		/ return failure
	ret

/*
 * spl_t
 * p_lock(lockp, retipl)
 *	lock_t	*lockp;
 *	spl_t	retipl;
 *
 * Lock the lock and return at interrupt priority level "retipl".
 * Return previous interrupt priority level.
 *
 * When the assembler handles inter-sub-segment branches, the "fail/spin"
 * code can be moved out-of-line in (eg) ".text 3".
 *
 * After writing SLIC local mask, must do a read to synchronize the write
 * and then insure 500 ns = 8 cycles @ 16MHz to occur before the 
 * xchg (to allow a now masked interrupt to occur before hold the 
 * locked resource).  The time of the "read" counts 2 cycles towards the 500ns.
 */

	.globl	_p_lock
_p_lock:
	movb	S_ARG1, %ah			/ new interrupt mask
	movb	VA_SLIC+SL_LMASK, %al		/ old interrupt mask
/PEEPOFF					/ turn off peephole optimizer
0:	movb	%ah, VA_SLIC+SL_LMASK		/ write new mask
/*****************************************************************/
	movb	VA_SLIC+SL_LMASK, %dl		/ dummy read to synch write
						/ sync+2 clocks
	movl	$(L_LOCKED), %edx		/ value to exchange
						/ sync+4 clocks
	movl	S_ARG0, %ecx			/ &lock
						/ sync+8
#if MHz == 20
	movl	%ecx, %ecx			/ 2 cycle nop
	movl	%ecx, %ecx			/ 2 cycle nop
#endif
/PEEPON						/ turn peephole opt back on
/***************SLICSYNC 8/12 ***************************************/
	xchgl	%edx, (%ecx)			/ try for lock
	cmpl	$(L_UNLOCKED), %edx		/ got it?
	je	2f				/ yup
	movb	%al, VA_SLIC+SL_LMASK		/ restore previous mask
1:	cmpl	$(L_UNLOCKED), (%ecx)		/ spin until...
	je	0b				/	...lock is clear
	jmp	1b				/ while not clear...
2:
	ret


/*
 * spl_t
 * cp_lock(lockp, retipl)
 *	lock_t	*lockp;
 *	spl_t	retipl;
 *
 * Conditionally acquire a lock.
 *
 * If lock is available, lock the lock and return at interrupt priority
 * level "retipl". Return previous interrupt priority level.
 * If lock is unavailable, return CPLOCKFAIL.
 *
 * See comments in p_lock() about writing SLIC mask.
 */

	.globl	_cp_lock
_cp_lock:
	movb	S_ARG1, %ah			/ new interrupt mask
/PEEPOFF					/ turn off peephole optimizer
	movb	VA_SLIC+SL_LMASK, %al		/ old interrupt mask
/*****************************************************************/
	movb	%ah, VA_SLIC+SL_LMASK		/ write new mask
	movb	VA_SLIC+SL_LMASK, %dl		/ dummy read to synch write
						/ sync+2 clocks
	movl	$(L_LOCKED), %edx		/ value to exchange
						/ sync+4 clocks
	movl	S_ARG0, %ecx			/ &lock
						/ sync+8 clocks
	movb	$0, %ah				/ so %eax != CPLOCKFAIL
						/ sync+10 clocks
#if MHz == 20
	movl	%eax,%eax			/ 2 cycle nop
#endif
/***************SLICSYNC 10/12 ***************************************/
/PEEPON						/ turn peephole opt back on
	xchgl	%edx, (%ecx)			/ try for lock
						/ sync+13 clocks
	cmpl	$(L_UNLOCKED), %edx		/ got it?
						/ sync+15 clocks
	je	0f				/ yup
	movb	%al, VA_SLIC+SL_LMASK		/ restore previous mask
	movl	$(CPLOCKFAIL), %eax		/ and return failure
0:
	ret


