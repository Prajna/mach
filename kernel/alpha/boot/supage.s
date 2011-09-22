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
 * $Log:	supage.s,v $
 * Revision 2.2  93/02/05  08:01:25  danner
 * 	First (working) draft.  Still needs to go hunting the shadow
 * 	for the ABOX_CTL register and *or* in our bits so that..
 * 	[93/02/04            af]
 * 
 */
/*
 *	File: supage.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	12/92
 *
 *	Enable super-page mappings for kernel.
 */

#if 0/*__osf__*/

#include <machine/asm.h>
#include <machine/regdef.h>
#define	op_imb 0x86
#define	op_ldqp 0x3
#define op_stqp 0x4

#else

#include <mach/alpha/asm.h>
#include <mach/alpha/alpha_instruction.h>

#endif

/*
 * Object:
 *	enable_suppage				EXPORTED routine
 *
 *		Horrible things to make sure we got it
 * Arguments:
 *	pal_dispatch_base			vm_offset_t
 *
 * Takes the phys address of the PAL dispatch vector (PAL_BASE),
 * drops code down in PAL space to clobber pal_call 10, flush
 * Icache, invoke downloaded code, restore old code, flush Icache
 * get back to user.  Simple.
 */

/* ICCSR, Ibox.
   Enable Ipage through bit 41w (22r)
 */
#define iccsr 2
#define iccsr_map 41

/* ABOX_CTL, Abox.
   Enable Dpages through bits 4 5
 */
#define abox_ctl 14
#define aboxctl_spe1 4

	.text
	.globl my_pal_code
my_pal_code:
	/* The GNU assembler knows about PAL instructions */

#ifdef	__GNU_AS__
	hw_mfpr/p t0, $iccsr		/* get pt2, shadows iccsr */
	 lda	t1,0x200(zero)		/* iccsr or-val */

	sll	t1,32,t1
	 or	t0,t1,t0		/* new iccsr value */

	hw_mtpr/pi t0, $iccsr		/* enable instrs suppage */
	 lda	t2, 0x43e(zero)		/* abox_ctl value */

	hw_mtpr/a t2, $abox_ctl		/* enable data suppages */
	 hw_rei				/* 30d|zero,zero|1|0|0 */

#else

	.long 0x64210082
	.long 0x205f0200
	.long 0x48441722
	.long 0x44220401
	.long 0x742100a2
	.long 0x207f043e
	.long 0x7463004e
	.long 0x7bff8000

#endif

#define	MY_PAL_SIZE	8	/* instructions */


/* What to clobber ? Privileged 10d (VMS2OSF) 'course! */
#define	CLOBBERED_CALL	10

	.text
LEAF(enable_suppage,1)
	ldgp	gp,0(pv)
	lda	sp,-48(sp)
	stq	ra,40(sp)
	stq	s0,32(sp)
	stq	a0,24(sp)

	lda	s0,1(zero)	# first time through
do_it_again:
	ldq	a0,24(sp)
	lda	s0,-1(s0)

	/* Decide where to do the load/stores */
	lda	a0,(0x2000+(CLOBBERED_CALL<<6))(a0)	# HW dispatch
	lda	a2,my_pal_code

	/* Copy down + save up loop */
	lda	t0,MY_PAL_SIZE(zero)
cplp:
	ldq	a1,0(a2)
	lda	t0,-2(t0)
	call_pal op_ldqp	# ldqp v0,0(a0)
	stq	v0,0(a2)
	call_pal op_stqp	# stqp a1,0(a0)
	addq	a0,8,a0
	addq	a2,8,a2
	bgt	t0,cplp

	/* Now must flush Icache */
	mb
	call_pal op_imb

	bne	s0,did_it

	/* Make call now.... Geronimo!! */
	call_pal CLOBBERED_CALL
	mb
	stq	t0,16(sp)

	/* Well, we made it.  Now do it again for fun */
	beq	s0,do_it_again

did_it:
	ldq	v0,16(sp)
	ldq	s0,32(sp)
	ldq	ra,40(sp)
	lda	sp,48(sp)
	ret
	END(enable_suppage)

