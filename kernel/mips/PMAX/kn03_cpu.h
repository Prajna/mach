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
 * $Log:	kn03_cpu.h,v $
 * Revision 2.3  93/08/03  12:33:24  mrt
 * 	Fixed interrupt handling.
 * 	[93/07/29  23:28:03  af]
 * 
 * Revision 2.2  93/05/30  21:08:40  rvb
 * 	RCS-ed.
 * 	[93/05/29            af]
 * 
 */
/*
 *	File: kn03_cpu.h
 * 	Author: John Wroclawski, Massachusetts Institute of Technology
 *	Date:	?/93
 *
 *	SPL definitions for 3max+
 */

/* Interrupt masks associated to IPL levels */
/* see KN03_INTR_* in kn03.h for bit meanings */
/* XXX shut the turbos off at all H/W interrupt levels? */

#define	KN03_IM0	0xff0f3bd0	/*- nvr nrmod fifo pbn toy */
					/*+ DMAs serial scsi lance TCs clock */
					/*--> spl0, splsoftclock, splnet */

/* Try this instead if you need the tc slots off at splimp */
/* If you do, you might change the TC0-2 priorities in kn03_tcintr() too */
/* define KN03_IM1	0xff0e02d0	/* all but lance */

#define	KN03_IM1	0xff0e3ad0	/*- lance */
					/*+ DMA(-lance) serial scsi TCs clock */
					/*--> splimp */

#define	KN03_IM2	0xff0800d0	/*- scsi TCs lance */
					/*+ DMA(-lance-scsi_err) serial clock */
					/*--> splbio */

#define	KN03_IM3	0x00080010	/*- serial scsi TCs lance */
					/*+ scsi-DMA-reload clock mem&ps */
					/*--> spltty */

#define	KN03_IM4	0x00000000	/*- serial scsi TCs lance */
					/*+ memory errors and clock(!) */
					/*--> splclock, splhigh & co */

/* We add to the SR 5 bits to point to the current IM in effect */

#define	KN03_SR_IMASK	0x0f800000		/* this is where in the SR */
#define	KN03_SR_SHIFT	23

#define	KN03_SR_IM4	0x00
#define	KN03_SR_IM3	0x08
#define	KN03_SR_IM2	0x04
#define	KN03_SR_IM1	0x02
#define	KN03_SR_IM0	0x01

/* Interrupt masks for quickly recognizing certain device's interrupts */
/* see KN03_INTR_* in kn03.h for bit meanings */

#define	KN03_INTR_ASIC		0xff0f0004 /* asic-reported error conditions */
#define	KN03_INTR_SCC		0x000000c0 /* either serial line ready */
