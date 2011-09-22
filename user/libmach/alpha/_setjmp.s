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
 * $Log:	_setjmp.s,v $
 * Revision 2.2  93/01/14  18:02:31  danner
 * 	Created.
 * 
 * 	Created.
 * 	[89/10/12            af]
 * 
 */

#include <mach/alpha/asm.h>

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
#define	JB_PC		8
#define	JB_SP		16
#define	JB_S0		24
#define	JB_S1		32
#define	JB_S2		40
#define	JB_S3		48
#define	JB_S4		56
#define	JB_S5		64
#define	JB_S6		72
#define	JB_GP		80

#define	JB_FPA_SR	88
#define JB_FS0		96
#define JB_FS1		104
#define JB_FS2		112
#define JB_FS3		120
#define JB_FS4		128
#define JB_FS5		136
#define JB_FS6		144
#define JB_FS7		152

#define	JB_SMALL	88
#define	JB_BIG		160

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
LEAF(_setjmp_, 1)
	mf_fpcr	ft0
	stt	ft0,JB_FPA_SR(a0)
	stt	fs0,JB_FS0(a0)
	stt	fs1,JB_FS1(a0)
	stt	fs2,JB_FS2(a0)
	stt	fs3,JB_FS3(a0)
	stt	fs4,JB_FS4(a0)
	stt	fs5,JB_FS5(a0)
	stt	fs6,JB_FS6(a0)
	stt	fs7,JB_FS7(a0)

	lda	v0,JB_BIG(zero)		/* li v0,JB_BIG */
	br	zero,skips

XLEAF(setjmp, 1)
XLEAF(_setjmp, 1)

	lda	v0,JB_SMALL(zero)	/* li	v0,JB_SMALL */

skips:
	stq	v0,JB_SIZE(a0)
	stq	ra,JB_PC(a0)
	stq	sp,JB_SP(a0)
	stq	gp,JB_GP(a0)
	stq	s0,JB_S0(a0)
	stq	s1,JB_S1(a0)
	stq	s2,JB_S2(a0)
	stq	s3,JB_S3(a0)
	stq	s4,JB_S4(a0)
	stq	s5,JB_S5(a0)
	stq	s6,JB_S6(a0)
	mov	zero,v0
	RET
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
LEAF(_longjmp_, 2)
	ldq	v0,JB_SIZE(a0)
	lda	s0,JB_BIG(zero)		/* li	s0,JB_BIG */
	cmpeq	v0,s0,v0
	beq	v0,botch

	ldt	ft0,JB_FPA_SR(a0)
	mt_fpcr	ft0
	ldt	fs0,JB_FS0(a0)
	ldt	fs1,JB_FS1(a0)
	ldt	fs2,JB_FS2(a0)
	ldt	fs3,JB_FS3(a0)
	ldt	fs4,JB_FS4(a0)
	ldt	fs5,JB_FS5(a0)
	ldt	fs6,JB_FS6(a0)
	ldt	fs7,JB_FS7(a0)
	br	zero,skipl

XLEAF(longjmp, 2)
XLEAF(_longjmp, 2)
	ldq	v0,JB_SIZE(a0)
	lda	s0,JB_SMALL(zero)		/* li	s0,JB_SMALL */
	cmpeq	v0,s0,v0
	beq	v0,botch

skipl:	ldq	ra,JB_PC(a0)
	ldq	sp,JB_SP(a0)
	ldq	gp,JB_GP(a0)
	ldq	s0,JB_S0(a0)
	ldq	s1,JB_S1(a0)
	ldq	s2,JB_S2(a0)
	ldq	s3,JB_S3(a0)
	ldq	s4,JB_S4(a0)
	ldq	s5,JB_S5(a0)
	ldq	s6,JB_S6(a0)

	mov	a1,v0
	RET

botch:
	call_pal 0x80			# a breakpoint
	br	zero,botch		# ..just in case..
	END(_longjmp)
