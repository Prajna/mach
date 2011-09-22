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
 * $Log:	kmin_cpu.h,v $
 * Revision 2.3  93/05/10  21:20:19  rvb
 * 	Let Halt interrupt always in.  Let DMA interrupts in
 * 	as much as possible.
 * 	[93/05/06  09:47:22  af]
 * 
 * Revision 2.2  91/08/24  12:21:15  af
 * 	Ignore, as hinted by specs, the "SCSI data available" interrupt.
 * 	The point is there seems to be no obvious way to clear it.
 * 	[91/08/22  11:14:10  af]
 * 
 * 	Created, from the DEC specs:
 * 	"3MIN System Module Functional Specification"  Revision 1.7
 * 	Workstation Systems Engineering, Palo Alto, CA. Sept 14, 1990.
 * 	"KN02BA Daughter Card Functional Specification" Revision 1.0
 * 	Workstation Systems Engineering, Palo Alto, CA. Aug  14, 1990.
 * 	[91/06/21            af]
 * 
 */
/*
 *	File: kmin_cpu.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/91
 *
 *	SPL definitions for 3min
 */

/* Interrupt masks associated to IPL levels */
/* see KMIN_INTR_* in kmin.h for bit meanings */

#define	KMIN_IM0	0xff0f13f1	/*- nvr nrmod fifo */
					/*+ DMAs serial scsi lance clock */
					/*--> spl0 splsoftclock splnet */
#define	KMIN_IM1	0xff0e12f1	/*- lance */
					/*+ DMA(-lance) serial scsi clock */
					/*--> splimp */
#define	KMIN_IM2	0xff0810f1	/*- scsi lance  */
					/*+ DMA(-lance-scsi_err) serial clock */
					/*--> splbio */
#define	KMIN_IM3	0x00081031	/*- serial scsi lance */
					/*+ scsi-DMA-reload clock mem&ps */
					/*--> spltty */
#define	KMIN_IM4	0x00001021	/*- serial scsi lance */
					/*+ memory errors and clock(!) */
					/*--> splclock */
#define	KMIN_IM5	0x00000001	/*- all but HALT button */
					/* splhigh & co */

/* We add to the SR 5 bits to point to the current IM in effect */

#define	KMIN_SR_IMASK	0x0f800000		/* this is where in the SR */
#define	KMIN_SR_SHIFT	23

#define	KMIN_SR_IM5	0x00
#define	KMIN_SR_IM4	0x10
#define	KMIN_SR_IM3	0x08
#define	KMIN_SR_IM2	0x04
#define	KMIN_SR_IM1	0x02
#define	KMIN_SR_IM0	0x01

/* Interrupt masks for quickly recognizing certain device's interrupts */
/* see KMIN_INTR_* in kmin.h for bit meanings */

#define	KMIN_INTR_ASIC		0xff0f0004
#define	KMIN_INTR_SCC		0x000000c0
