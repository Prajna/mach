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
 * $Log:	kmin_cpu.s,v $
 * Revision 2.5  93/05/10  21:20:22  rvb
 * 	Let Halt interrupt always in.
 * 	[93/05/06  09:49:35  af]
 * 
 * Revision 2.4  92/03/02  18:33:49  rpd
 * 	Changed kmin_encode_sr_im() to make it more robust.
 * 	[92/03/02  02:26:41  af]
 * 
 * Revision 2.3  92/02/19  15:08:42  elf
 * 	Changed #-style comments, for ANSI cpp.
 * 	[92/02/19  13:11:34  rpd]
 * 
 * Revision 2.2  91/08/24  12:21:20  af
 * 	Created, from the DEC specs:
 * 	"3MIN System Module Functional Specification"  Revision 1.7
 * 	Workstation Systems Engineering, Palo Alto, CA. Sept 14, 1990.
 * 	"KN02BA Daughter Card Functional Specification" Revision 1.0
 * 	Workstation Systems Engineering, Palo Alto, CA. Aug  14, 1990.
 * 	[91/06/21            af]
 * 
 */
/*
 *	File: kmin_cpu.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/91
 *
 *	SPL functions for 3min
 */

#if	MACH_KERNEL
#include <mach/mips/asm.h>
#else
#include <mips/asm.h>
#endif

#include <mips/mips_cpu.h>
#undef	PHYS_TO_K1SEG
#define	PHYS_TO_K1SEG(x)	((x)|K1SEG_BASE)

#include <mips/PMAX/kmin.h>
#include <mips/PMAX/kmin_cpu.h>

	.set	noreorder

	IMPORT(kmin_tc3_imask, 4)

/*
 *	Object:
 *		kmin_encode_sr_im		LOCAL function
 *
 *		Encode the pair (status register, interrupt mask)
 *		into a single 32 bit value.
 *
 *	Arguments:
 *		a0, sr				unsigned
 *		a1, imask			unsigned
 *
 *	We use some spare bits in the SR to hold our info
 *	An interrupt mask value of KMIN_IMx translates into
 *	KMIN_SR_IMx inserted in the sr in the proper place
 */
STATIC_LEAF(kmin_encode_sr_im)
	li	t0,KMIN_IM5
	beq	a1,t0,1f
	li	t0,KMIN_SR_IM5

	li	t0,KMIN_IM4
	beq	a1,t0,1f
	li	t0,KMIN_SR_IM4

	li	t0,KMIN_IM3
	beq	a1,t0,1f
	li	t0,KMIN_SR_IM3

#if 1/*debug*/
	move a2,a1
#endif
	srl	a1,16			# only trust dma masks

	li	t0,(KMIN_IM2>>16)
	beq	a1,t0,1f
	li	t0,KMIN_SR_IM2

	li	t0,(KMIN_IM1>>16)
	beq	a1,t0,1f
	li	t0,KMIN_SR_IM1

	li	t0,(KMIN_IM0>>16)
	beq	a1,t0,1f
	li	t0,KMIN_SR_IM0

	/* should not happen */
	break	1

1:	sll	v0,t0,KMIN_SR_SHIFT	/* IM encoded */
	li	t0,~KMIN_SR_IMASK
	and	a0,t0			/* SR cleaned */
	j	ra
	or	v0,a0			/* return SR|IM */
	END(kmin_encode_sr_im)

/*
 *	Object:
 *		kmin_splx			EXPORTED function
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
LEAF(kmin_splx)
	.set	noreorder
	li	t0, KMIN_SR_IMASK
	and	v0,a0,t0		/* get imask encoding */
	not	t0
	sra	v0,KMIN_SR_SHIFT	/* one of KMIN_SR_IM* now */

	li	a1,KMIN_IM0		/* to-spl0 is popular, do first */
	beq	v0,KMIN_SR_IM0,1f
	and	a0,t0			/* the SR value is clean now */

	.set	reorder

	li	a1,KMIN_IM5
	beq	v0,KMIN_SR_IM5,1f

	li	a1,KMIN_IM4
	beq	v0,KMIN_SR_IM4,1f

	li	a1,KMIN_IM3
	beq	v0,KMIN_SR_IM3,1f

	li	a1,KMIN_IM2
	beq	v0,KMIN_SR_IM2,1f

	li	a1,KMIN_IM1
	beq	v0,KMIN_SR_IM1,1f

	PANIC("kmin_splx")	
1:
	lw	t1,kmin_tc3_imask	/* get currently valid devices */
	and	a1,t1			/* keep unwarranted devices off */
	li	t0, PHYS_TO_K1SEG(KMIN_REG_IMSK)
	.set	noreorder
	mtc0	zero,c0_status		/* no intr, tx */
	sw	a1,0(t0)		/* install new imask */
	mtc0	a0,c0_status		/* both places */
	j	ra
	move	v0,zero
	END(kmin_splx)

/*
 *	Object:
 *		kmin_spl0			EXPORTED function
 *
 *		Enable all interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(kmin_spl0)
	li	a0,SR_IEc|INT_LEV0

kmin_spl0_1:	/* also come here from software interrupt levels */
	li	a2, KMIN_IM0		/* new imask */

kmin_spl0_2:	/* also come here from all of the other spls */
	lw	t0,kmin_tc3_imask	/* get currently valid devices */
	mfc0	v0,c0_status		/* get current status */

	and	t1,a2,t0		/* keep unwarranted devices off */
	li	t0,PHYS_TO_K1SEG(KMIN_REG_IMSK)
	lw	a1,0(t0)		/* old imask */
	mtc0	zero,c0_status		/* no intr, tx */
	sw	t1,0(t0)		/* install new imask */
	mtc0	a0,c0_status		/* both places */
	b	kmin_encode_sr_im	/* no need to get back here */
	move	a0,v0
	END(kmin_spl0)

/*
 *	Object:
 *		kmin_splsoftclock		EXPORTED function
 *
 *		Block software clock interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(kmin_splsoftclock)
	b	kmin_spl0_1		/* same as spl0, just different sr */
	li	a0,SR_IEc|INT_LEV2
	END(kmin_splsoftclock)

/*
 *	Object:
 *		kmin_splnet			EXPORTED function
 *
 *		Block software network interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(kmin_splnet)
	b	kmin_spl0_1		/* same as spl0, just different sr */
	li	a0,SR_IEc|INT_LEV1
	END(kmin_splnet)

/*
 *	Object:
 *		kmin_splimp			EXPORTED function
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
LEAF(kmin_splimp)
	li	a2, KMIN_IM1
	b	kmin_spl0_2
	li	a0,SR_IEc|INT_LEV5
	END(kmin_splimp)

/*
 *	Object:
 *		kmin_splbio			EXPORTED function
 *
 *		Block all BlockI/O device interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(kmin_splbio)
	li	a2, KMIN_IM2
	b	kmin_spl0_2
	li	a0,SR_IEc|INT_LEV5
	END(kmin_splbio)

/*
 *	Object:
 *		kmin_spltty			EXPORTED function
 *
 *		Block character I/O device interrupts (console)
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(kmin_spltty)
	li	a2, KMIN_IM3
	b	kmin_spl0_2
	li	a0,SR_IEc|INT_LEV5
	END(kmin_spltty)

/*
 *	Object:
 *		kmin_splclock			EXPORTED function
 *
 *		Block scheduling clock (hardware) interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(kmin_splclock)
	li	a2, KMIN_IM4
	b	kmin_spl0_2
	li	a0,SR_IEc|INT_LEV5
	END(kmin_splclock)

/*
 *	Object:
 *		kmin_splvm			EXPORTED function
 *
 *		Block interrupts that might cause VM faults
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(kmin_splvm)
	li	a2, KMIN_IM5
	b	kmin_spl0_2
	li	a0,SR_IEc|INT_LEV6
	END(kmin_splvm)

/*
 *	Object:
 *		kmin_splhigh			EXPORTED function
 *
 *		Block all interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(kmin_splhigh)
	li	a2, KMIN_IM5
	b	kmin_spl0_2
	li	a0,SR_IEc|INT_LEV6
	END(kmin_splhigh)
