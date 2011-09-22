/* 
 * Mach Operating System
 * Copyright (c) 1993-1989 Carnegie Mellon University
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
 * $Log:	kn03_cpu.s,v $
 * Revision 2.4  93/11/17  17:51:14  dbg
 * 	In splx, a no-reorder got turned into a reorder.
 * 	I wonder what bit 23 in the status register might do...
 * 	[93/09/21            af]
 * 
 * Revision 2.3  93/08/03  12:33:27  mrt
 * 	Fixed interrupt handling.
 * 	[93/07/29  23:28:27  af]
 * 
 * Revision 2.2  93/05/30  21:08:46  rvb
 * 	RCS-ed.
 * 	[93/05/29            af]
 * 
 */
/*
 *	File: kn03_cpu.s
 * 	Author: John Wroclawski, Massachusetts Institute of Technology
 *	Date:	?/93
 *
 *	SPL functions for 3max+
 *	Based on kmin_cpu.s
 */

#include <mach/mips/asm.h>

#include <mips/mips_cpu.h>
#undef	PHYS_TO_K1SEG
#define	PHYS_TO_K1SEG(x)	((x)|K1SEG_BASE)

#include <mips/PMAX/kn03.h>
#include <mips/PMAX/kn03_cpu.h>

	.set	noreorder

	IMPORT(kn03_imask, 4)

/*
 *	Object:
 *		kn03_encode_sr_im		LOCAL function
 *
 *		Encode the pair (status register, interrupt mask)
 *		into a single 32 bit value.
 *
 *	Arguments:
 *		a0, sr				unsigned
 *		a1, imask			unsigned
 *
 *	We use some spare bits in the SR to hold our info
 *	An interrupt mask value of KN03_IMx translates into
 *	KN03_SR_IMx inserted in the sr in the proper place
 */
STATIC_LEAF(kn03_encode_sr_im)

	li	t0,KN03_IM4
	beq	a1,t0,1f
	li	t0,KN03_SR_IM4

	li	t0,KN03_IM3
	beq	a1,t0,1f
	li	t0,KN03_SR_IM3

#if 1/*debug*/
	move a2,a1
#endif
	srl	a1,16			# only trust dma masks

	li	t0,(KN03_IM2>>16)
	beq	a1,t0,1f
	li	t0,KN03_SR_IM2

	li	t0,(KN03_IM1>>16)
	beq	a1,t0,1f
	li	t0,KN03_SR_IM1

	li	t0,(KN03_IM0>>16)
	beq	a1,t0,1f
	li	t0,KN03_SR_IM0

	/* should not happen */
	break	1

1:	sll	v0,t0,KN03_SR_SHIFT	/* IM encoded */
	li	t0,~KN03_SR_IMASK
	and	a0,t0			/* SR cleaned */
	j	ra
	or	v0,a0			/* return SR|IM */
	END(kn03_encode_sr_im)

/*
 *	Object:
 *		kn03_splx			EXPORTED function
 *
 *		Restore priority level
 *
 *	Arguments:
 *		a0				unsigned
 *
 *	Set priority level to the value in a0, returns NOTHING.
 *	Actually, since the IPL is not a separate register on MIPS just
 *	use the entire content of the status register, plus look at the
 *	extra bits we put in to indicate the interrupt mask in effect.
 */
LEAF(kn03_splx)
	.set	noreorder
	li	t0, KN03_SR_IMASK
	and	v0,a0,t0		/* imask value */
	not	t0
	sra	v0,KN03_SR_SHIFT        /* one of KMIN_SR_IM* now */

	li	a1,KN03_IM0		/* to-spl0 is popular, do first */
	beq	v0,KN03_SR_IM0,1f
	and	a0,t0			/* the SR value is clean now */

	.set	reorder

	li	a1,KN03_IM4
	beq	v0,KN03_SR_IM4,1f

	li	a1,KN03_IM3
	beq	v0,KN03_SR_IM3,1f

	li	a1,KN03_IM2
	beq	v0,KN03_SR_IM2,1f

	li	a1,KN03_IM1
	beq	v0,KN03_SR_IM1,1f

	PANIC("kn03_splx")	
1:
	lw	t1,kn03_imask		/* get currently valid devices */
	and	a1,t1			/* keep unwarranted devices off */
	li	t0, PHYS_TO_K1SEG(KN03_REG_IMSK)
	.set	noreorder
	mtc0	zero,c0_status		/* no intr, tx */
	sw	a1,0(t0)		/* install new imask */
	mtc0	a0,c0_status		/* both places */
	j	ra
	move	v0,zero
	END(kn03_splx)

/*
 *	Object:
 *		kn03_spl0			EXPORTED function
 *
 *		Enable all interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(kn03_spl0)
	li	a0,SR_IEc|INT_LEV0

kn03_spl0_1:	/* also come here from software interrupt levels */
	li	a2, KN03_IM0		/* new imask */

kn03_spl0_2:	/* also come here from all of the other spls */
		/* a0 new status reg value, a2 new mask */
	lw	t0,kn03_imask		/* get currently valid devices */
	mfc0	v0,c0_status		/* get current status */

	and	t1,a2,t0		/* keep unwarranted devices off */
	li	t0,PHYS_TO_K1SEG(KN03_REG_IMSK)
	lw	a1,0(t0)		/* old imask */
	mtc0	zero,c0_status		/* no intr, tx */
	sw	t1,0(t0)		/* install new imask */
	mtc0	a0,c0_status		/* both places */
	b	kn03_encode_sr_im	/* no need to get back here */
	move	a0,v0
	END(kn03_spl0)

/*
 *	Object:
 *		kn03_splsoftclock		EXPORTED function
 *
 *		Block software clock interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(kn03_splsoftclock)
	b	kn03_spl0_1		/* same as spl0, just different sr */
	li	a0,SR_IEc|INT_LEV2
	END(kn03_splsoftclock)

/*
 *	Object:
 *		kn03_splnet			EXPORTED function
 *
 *		Block software network interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(kn03_splnet)
	b	kn03_spl0_1		/* same as spl0, just different sr */
	li	a0,SR_IEc|INT_LEV1
	END(kn03_splnet)

/*
 *	Object:
 *		kn03_splimp			EXPORTED function
 *
 *		Block network hardware interrupts
 *
 *	Arguments:
 *		none
 *
 *	Unlike Vax, does not block hardware clock.
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(kn03_splimp)
	li	a2, KN03_IM1
	b	kn03_spl0_2
	li	a0,SR_IEc|INT_LEV2
	END(kn03_splimp)

/*
 *	Object:
 *		kn03_splbio			EXPORTED function
 *
 *		Block all BlockI/O device interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(kn03_splbio)
	li	a2, KN03_IM2
	b	kn03_spl0_2
	li	a0,SR_IEc|INT_LEV2
	END(kn03_splbio)

/*
 *	Object:
 *		kn03_spltty			EXPORTED function
 *
 *		Block character I/O device interrupts (console)
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(kn03_spltty)
	li	a2, KN03_IM3
	b	kn03_spl0_2
	li	a0,SR_IEc|INT_LEV2
	END(kn03_spltty)

/*
 *	Object:
 *		kn03_splclock			EXPORTED function
 *
 *		Block scheduling clock (hardware) interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(kn03_splclock)
	li	a2, KN03_IM4
	b	kn03_spl0_2
	li	a0,SR_IEc|INT_LEV5
	END(kn03_splclock)

/*
 *	Object:
 *		kn03_splvm			EXPORTED function
 *
 *		Block interrupts that might cause VM faults
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(kn03_splvm)
	li	a2, KN03_IM4
	b	kn03_spl0_2
	li	a0,SR_IEc|INT_LEV5
	END(kn03_splvm)

/*
 *	Object:
 *		kn03_splhigh			EXPORTED function
 *
 *		Block all interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(kn03_splhigh)
	li	a2, KN03_IM4
	b	kn03_spl0_2
	li	a0,SR_IEc|INT_LEV6
	END(kn03_splhigh)
