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
 * $Log:	_setjmp.s,v $
 * Revision 2.5  91/05/14  17:54:00  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/14  14:18:09  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:46:43  mrt]
 * 
 * Revision 2.3  90/06/02  15:12:46  rpd
 * 	Rearranged fields in jmpbuf, to put size field first.
 * 	This allows setjmp/longjmp to use a smaller jmpbuf.
 * 	[90/04/29            rpd]
 * 
 * Revision 2.2  89/11/29  14:18:33  af
 * 	Changed names, so that standalone programs do not end
 * 	up using the floating point unit for nothing.
 * 	[89/11/16  14:30:45  af]
 * 
 * 	Created.
 * 	[89/10/12            af]
 * 
 */

#include <mach/mips/asm.h>

/*
 *	Non local goto for user programs.
 *
 */

/*
 *	These definitions are here and not exported because
 *	it is unpleasant to think that someone might actually
 *	need to play with the content of a jumpbuf.
 */
#define JB_SIZE		0
#define	JB_PC		4
#define	JB_SP		8
#define	JB_S0		12
#define	JB_S1		16
#define	JB_S2		20
#define	JB_S3		24
#define	JB_S4		28
#define	JB_S5		32
#define	JB_S6		36
#define	JB_S7		40
#define	JB_FP		44

#define	JB_FPA_SR	48
#define JB_F20		52
#define JB_F22		56
#define JB_F24		60
#define JB_F26		64
#define JB_F28		68
#define JB_F30		72

#define	JB_SMALL	48
#define	JB_BIG		76

	.set	reorder

/*
 *	Object:
 *		setjmp				EXPORTED function
 *		_setjmp_			EXPORTED function
 *
 *		Save current context for non-local goto's
 *
 *	Arguments:
 *		a0				jmp_buf *
 *
 *	Saves all registers that are callee-saved in the
 *	given longjmp buffer.
 *	If a standalone program *really* needs to use the
 *	floating point accelerator it should use the slower
 *	alternate _setjmp_/_longjmp_ with some extra context
 *	switch time penalty.
 *	Note that our definitions of setjmp is the one that
 *	libc calls _setjmp, while libc's setjmp is what we
 *	call _setjmp_ [all this to avoid unintended use of
 *	the FPA...].
 *
 * 	Return 0.
 */
LEAF(_setjmp_)
	li	v0,JB_BIG
	sw	v0,JB_SIZE(a0)

	cfc1	v0,fpa_csr
	sw	v0,JB_FPA_SR(a0)
	s.d	$f20,JB_F20(a0)
	s.d	$f22,JB_F22(a0)
	s.d	$f24,JB_F24(a0)
	s.d	$f26,JB_F26(a0)
	s.d	$f28,JB_F28(a0)
	s.d	$f30,JB_F30(a0)
	b	skips

XLEAF(setjmp)
XLEAF(_setjmp)
	li	v0,JB_SMALL
	sw	v0,JB_SIZE(a0)

skips:	sw	ra,JB_PC(a0)
	sw	sp,JB_SP(a0)
	sw	fp,JB_FP(a0)
	sw	s0,JB_S0(a0)
	sw	s1,JB_S1(a0)
	sw	s2,JB_S2(a0)
	sw	s3,JB_S3(a0)
	sw	s4,JB_S4(a0)
	sw	s5,JB_S5(a0)
	sw	s6,JB_S6(a0)
	sw	s7,JB_S7(a0)
	move	v0,zero
	j	ra
	END(_setjmp)


/*
 *	Object:
 *		longjmp				EXPORTED function
 *
 *		Perform a non-local goto
 *
 *	Arguments:
 *		a0				jmp_buf *
 *		a1				int
 *
 *	Restores all registers that are callee-saved from the
 *	given longjmp buffer.  Mild checks.
 * 	Return the value in a1.
 */
LEAF(_longjmp_)
	lw	v0,JB_SIZE(a0)
	li	s0,JB_BIG
	bne	v0,s0,botch

	lw	v0,JB_FPA_SR(a0)
	ctc1	v0,fpa_csr
	l.d	$f20,JB_F20(a0)
	l.d	$f22,JB_F22(a0)
	l.d	$f24,JB_F24(a0)
	l.d	$f26,JB_F26(a0)
	l.d	$f28,JB_F28(a0)
	l.d	$f30,JB_F30(a0)
	b	skipl

XLEAF(longjmp)
XLEAF(_longjmp)
	lw	v0,JB_SIZE(a0)
	li	s0,JB_SMALL
	bne	v0,s0,botch

skipl:	lw	ra,JB_PC(a0)
	lw	sp,JB_SP(a0)
	lw	fp,JB_FP(a0)
	lw	s0,JB_S0(a0)
	lw	s1,JB_S1(a0)
	lw	s2,JB_S2(a0)
	lw	s3,JB_S3(a0)
	lw	s4,JB_S4(a0)
	lw	s5,JB_S5(a0)
	lw	s6,JB_S6(a0)
	lw	s7,JB_S7(a0)

	move	v0,a1
	j	ra

botch:
	break	0			# Illegal bp
	b	botch			# ..just in case..
	END(_longjmp)
