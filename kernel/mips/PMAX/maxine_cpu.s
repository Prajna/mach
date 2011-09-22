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
 * $Log:	maxine_cpu.s,v $
 * Revision 2.3  92/05/05  10:46:43  danner
 * 	Made dtop interruptible from serial lines.
 * 	[92/05/04  11:36:12  af]
 * 
 * Revision 2.2  92/03/02  18:34:39  rpd
 * 	Created, from DEC specs.
 * 	[92/01/30            af]
 * 
 */
/*
 *	File: maxine_cpu.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	1/92
 *
 *	SPL functions for MAXine
 */

#include <mach/mips/asm.h>

#include <mips/mips_cpu.h>
#undef	PHYS_TO_K1SEG
#define	PHYS_TO_K1SEG(x)	((x)|K1SEG_BASE)

#include <mips/PMAX/maxine.h>
#include <mips/PMAX/maxine_cpu.h>

	.set	noreorder

	IMPORT(xine_tc3_imask, 4)

/*
 *	Object:
 *		xine_encode_sr_im		LOCAL function
 *
 *		Encode the pair (status register, interrupt mask)
 *		into a single 32 bit value.
 *
 *	Arguments:
 *		a0, sr				unsigned
 *		a1, imask			unsigned
 *
 *	We use some spair bits in the SR to hold our info
 *	An interrupt mask value of XINE_IMx translates into
 *	XINE_SR_IMx inserted in the sr in the proper place.
 *	This encoded value is ONLY usable for saving the
 *	status, e.g. like = "s = splxxx(); ...; splx(s);"
 */

STATIC_LEAF(xine_encode_sr_im)
	beq	a1,zero,1f
	li	t0,XINE_SR_IM4

	srl	a1,16			# trust dma mask only

	li	t0,(XINE_IM3>>16)
	beq	a1,t0,1f
	li	t0,XINE_SR_IM3

	li	t0,(XINE_IM2>>16)
	beq	a1,t0,1f
	li	t0,XINE_SR_IM2

	li	t0,(XINE_IM2a>>16)
	beq	a1,t0,1f
	li	t0,XINE_SR_IM2a

	li	t0,(XINE_IM1>>16)
	beq	a1,t0,1f
	li	t0,XINE_SR_IM1

	li	t0,(XINE_IM0>>16)
	beq	a1,t0,1f
	li	t0,XINE_SR_IM0

	/* should not happen */
	break	1

1:	sll	v0,t0,XINE_SR_SHIFT	# IM encoded
	li	t0,~XINE_SR_IMASK
	and	a0,t0			# SR cleaned
	j	ra
	or	v0,a0			# return SR|IM
	END(xine_encode_sr_im)

/*
 *	Object:
 *		xine_splx			EXPORTED function
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
LEAF(xine_splx)
	.set	reorder
	li	t0, XINE_SR_IMASK
	and	v0,a0,t0		# imask value
	not	t0
	and	a0,t0			# the SR value is clean now
	sra	v0,XINE_SR_SHIFT
	move	a1,zero
	beq	v0,zero,1f
	li	a1,XINE_IM4
	beq	v0,XINE_SR_IM4,1f
	li	a1,XINE_IM3
	beq	v0,XINE_SR_IM3,1f
	li	a1,XINE_IM2a
	beq	v0,XINE_SR_IM2a,1f
	li	a1,XINE_IM2
	beq	v0,XINE_SR_IM2,1f
	li	a1,XINE_IM1
	beq	v0,XINE_SR_IM1,1f
	li	a1,XINE_IM0
	beq	v0,XINE_SR_IM0,1f
	PANIC("xine_splx")	
1:
	lw	t1,xine_tc3_imask	/* get currently valid devices */
	and	a1,t1			/* keep unwarranted devices off */
	li	t0, PHYS_TO_K1SEG(XINE_REG_IMSK)
	.set	noreorder
	mtc0	zero,c0_status		/* no intr, tx */
	sw	a1,0(t0)		/* install new imask */
	mtc0	a0,c0_status		/* both places */
	j	ra
	move	v0,zero
	END(xine_splx)

/*
 *	Object:
 *		xine_spl0			EXPORTED function
 *
 *		Enable all interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(xine_spl0)
	li	a0,SR_IEc|INT_LEV0

xine_spl0_1:	/* also come here from software interrupt levels */
	li	a2, XINE_IM0		/* new imask */

xine_spl0_2:	/* also come here from all of the other spls */
	lw	t0,xine_tc3_imask	/* get currently valid devices */
	mfc0	v0,c0_status		/* get current status */

	and	t1,a2,t0		/* keep unwarranted devices off */
	li	t0,PHYS_TO_K1SEG(XINE_REG_IMSK)
	lw	a1,0(t0)		/* old imask */
	mtc0	zero,c0_status		/* no intr, tx */
	sw	t1,0(t0)		/* install new imask */
	mtc0	a0,c0_status		/* both places */
	b	xine_encode_sr_im	/* no need to get back here */
	move	a0,v0
	END(xine_spl0)

/*
 *	Object:
 *		xine_splsoftclock		EXPORTED function
 *
 *		Block software clock interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(xine_splsoftclock)
	b	xine_spl0_1		/* same as spl0, just different sr */
	li	a0,SR_IEc|INT_LEV2
	END(xine_splsoftclock)

/*
 *	Object:
 *		xine_splnet			EXPORTED function
 *
 *		Block software network interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(xine_splnet)
	b	xine_spl0_1		/* same as spl0, just different sr */
	li	a0,SR_IEc|INT_LEV1
	END(xine_splnet)

/*
 *	Object:
 *		xine_splimp			EXPORTED function
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
LEAF(xine_splimp)
	li	a2, XINE_IM1
	b	xine_spl0_2
	li	a0,SR_IEc|XINE_INT_LEV5
	END(xine_splimp)

/*
 *	Object:
 *		xine_splbio			EXPORTED function
 *
 *		Block all BlockI/O device interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(xine_splbio)
	li	a2, XINE_IM2
	b	xine_spl0_2
	li	a0,SR_IEc|XINE_INT_LEV5
	END(xine_splbio)

/*
 *	Object:
 *		xine_spltty			EXPORTED function
 *
 *		Block character I/O device interrupts (console)
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(xine_spltty)
	li	a2, XINE_IM3
	b	xine_spl0_2
	li	a0,SR_IEc|XINE_INT_LEV5
	END(xine_spltty)

/*
 *	Object:
 *		xine_splclock			EXPORTED function
 *
 *		Block scheduling clock (hardware) interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
#if 0	/* no need to duplicate code */
LEAF(xine_splclock)
	li	a2, XINE_IM4
	b	xine_spl0_2
	li	a0,SR_IEc|XINE_INT_LEV7
	END(xine_splclock)
#endif

/*
 *	Object:
 *		xine_splvm			EXPORTED function
 *
 *		Block interrupts that might cause VM faults
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(xine_splvm)
XLEAF(xine_splclock)
	li	a2, XINE_IM4
	b	xine_spl0_2
	li	a0,SR_IEc|XINE_INT_LEV7
	END(xine_splvm)

/*
 *	Object:
 *		xine_splhigh			EXPORTED function
 *
 *		Block all interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	extended with the encoding of the interrupt mask
 */
LEAF(xine_splhigh)
	li	a0,SR_IEc|XINE_INT_LEV8
	b	xine_spl0_2
	li	a2, XINE_IM4
	END(xine_splhigh)
