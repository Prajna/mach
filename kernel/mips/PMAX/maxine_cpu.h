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
 * $Log:	maxine_cpu.h,v $
 * Revision 2.3  92/05/05  10:46:37  danner
 * 	Made dtop interruptible from serial lines.
 * 	[92/05/04  11:36:44  af]
 * 
 * Revision 2.2  92/03/02  18:34:35  rpd
 * 	Created, from DEC specs.
 * 	[92/01/30            af]
 * 
 */
/*
 *	File: maxine_cpu.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	1/92
 *
 *	SPL definitions for MAXine
 */

/* Since clock comes in lower than I/O (grrrr) redefine priorities */
#define	XINE_INT_LEV5	(IP_LEV7|IP_LEV6|IP_LEV5|IP_LEV4|IP_LEV3)
#define	XINE_INT_LEV7	(IP_LEV7|IP_LEV6|IP_LEV4)
#define	XINE_INT_LEV8	(IP_LEV6|IP_LEV4)

/* Interrupt masks associated to IPL levels */
/* see XINE_INTR_* in maxine.h for bit meanings */

#define	XINE_IM0	0xffff9b6b	/* all good ones enabled */
					/* spl0, splsoftclock, splnet */
#define	XINE_IM1	0xfffe9a6b	/* all but lance */
					/* splimp */
#define	XINE_IM2	0xff70084b	/* all but scsi, TC, fdi and lance  */
					/* splbio */
#define	XINE_IM2a	0xf0700848	/* same, but disable DTOP */

#define	XINE_IM3	0x00700800	/* all but serial lines and above */
					/* spltty */
#define	XINE_IM4	0x00000000	/* nothing at all */
					/* splhigh & co */

/* We add to the SR 5 bits to point to the current IM in effect */

#define	XINE_SR_IMASK	0x0f800000		/* this is where in the SR */
#define	XINE_SR_SHIFT	23

#define	XINE_SR_IM4	0x00
#define	XINE_SR_IM3	0x10
#define	XINE_SR_IM2a	0x08
#define	XINE_SR_IM2	0x04
#define	XINE_SR_IM1	0x02
#define	XINE_SR_IM0	0x01

/* Interrupt masks for quickly recognizing certain device's interrupts */
/* see XINE_INTR_* in maxine.h for bit meanings */

#define	XINE_INTR_ASIC		0xffff0000
#define	XINE_INTR_DTOP		0x00000003
