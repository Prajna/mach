/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
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
 * $Log:	prom_routines.s,v $
 * Revision 2.4  93/05/20  21:03:04  mrt
 * 	Changed use of zero to ra in call to NESTED.
 * 	[93/05/18            mrt]
 * 
 * Revision 2.3  93/03/09  10:49:42  danner
 * 	Different prom dispatching, link safe.
 * 	[93/03/05            af]
 * 
 * Revision 2.2  93/02/05  08:01:14  danner
 * 	Created a while back.

 * 	[93/02/04            af]
 * 
 */
/*
 *	File: prom_routines.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	12/92
 *
 *	PROM entrypoints of interest to kernel
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 *
 *	"VMS for Alpha Platforms Internals and Data Structures"
 *	Digital Press 1992, Burlington, MA 01803
 *	Order number EY-L466E-P1/2, ISBN 1-55558-095-5
 *	[Especially volume 1, chapter 33 "Bootstrap processing"]
 */

#include <mach/alpha/asm.h>
#include <mach/alpha/alpha_instruction.h>

	.globl	prom_dispatch_v
	.comm	prom_dispatch_v 16

	.text
	.align	4

/*
 * Dispatcher routine. Independent of prom's calling standard,
 * saves our callee-saved registers as required by C.
 */
#define	D_RA			(7*8)
#define	D_S0			(8*8)
#define	D_S1			(9*8)
#define	D_S2			(10*8)
#define	D_S3			(11*8)
#define	D_S4			(12*8)
#define	D_S5			(13*8)
#define	D_S6			(14*8)
#define	DISPATCH_FRAME_SIZE	(15*8)
#define	DISPATCH_REGS		IM_RA|IM_S0|IM_S1|IM_S2|IM_S3|IM_S4|IM_S5|IM_S6

NESTED(prom_dispatch,5,DISPATCH_FRAME_SIZE,ra,DISPATCH_REGS,0)

	ldgp	gp,0(pv)

	lda	sp,-DISPATCH_FRAME_SIZE(sp)
	stq	ra,D_RA(sp)
	stq	s0,D_S0(sp)
	stq	s1,D_S1(sp)
	stq	s2,D_S2(sp)
	stq	s3,D_S3(sp)
	stq	s4,D_S4(sp)
	stq	s5,D_S5(sp)
	stq	s6,D_S6(sp)

	lda	pv,prom_dispatch_v
	ldq	v0,0(pv)		/* routine */
	ldq	pv,8(pv)		/* routine_arg */
	
	jsr	ra,(v0)

	ldq	ra,D_RA(sp)
	ldq	s0,D_S0(sp)
	ldq	s1,D_S1(sp)
	ldq	s2,D_S2(sp)
	ldq	s3,D_S3(sp)
	ldq	s4,D_S4(sp)
	ldq	s5,D_S5(sp)
	ldq	s6,D_S6(sp)
	lda	sp,DISPATCH_FRAME_SIZE(sp)
	RET

	END(prom_dispatch)


/*
 * Return to prom
 */
LEAF(prom_halt,0)
	call_pal op_halt
	END(prom_halt)

