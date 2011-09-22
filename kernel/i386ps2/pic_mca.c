/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * Copyright (c) 1991 IBM Corporation 
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation,
 * and that the name IBM not be used in advertising or publicity 
 * pertaining to distribution of the software without specific, written
 * prior permission.
 * 
 * CARNEGIE MELLON AND IBM ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON AND IBM DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	pic_mca.c,v $
 * Revision 2.2  93/02/04  08:01:32  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 	Synched 2.5 & 3.0 at I386q (r2.1.1.2) & XMK35 (r2.1)
 * 	[90/11/15            rvb]
 * 
 * 	We don't want to see clock interrupts till clkstart(),
 * 	but we can not turn the interrupt off so we disable them.
 * 	2.5 ONLY!
 * 	[90/11/05            rvb]
 * 	Created based on pic.c
 * 	[90/06/16            rvb]
 * 
 */

#include <sys/types.h>
#include <i386/ipl.h>
#include <i386/pic.h>


/* These interrupts are always present */
extern intnull(), fpintr(), hardclock(), kdintr();
extern prtnull();

int (*ivect[NINTR])() = {
	/* 00 */	hardclock,	/* always */
	/* 01 */	kdintr,		/* kdintr, ... */
	/* 02 */	intnull,
	/* 03 */	intnull,	/* lnpoll, comintr, ... */

	/* 04 */	intnull,	/* comintr, ... */
	/* 05 */	intnull,	/* comintr, wtintr, ... */
	/* 06 */	intnull,	/* fdintr, ... */
	/* 07 */	prtnull,	/* qdintr, ... */

	/* 08 */	intnull,
	/* 09 */	intnull,	/* ether */
	/* 10 */	intnull,
	/* 11 */	intnull,

	/* 12 */	intnull,
	/* 13 */	fpintr,		/* always */
	/* 14 */	intnull,	/* hdintr, ... */
	/* 15 */	intnull,	/* ??? */
};

u_char intpri[NINTR] = {
#ifdef	MACH_KERNEL
	/* 00 */	SPLHI,
	/* 01 */	0,
	/* 02 */	0,
	/* 03 */	0,
#else	MACH_KERNEL
	/* 00 */   	0,
	/* 01 */	0,
	/* 02 */	0,
	/* 03 */	0,
#endif	MACH_KERNEL
	/* 04 */	0,	0,	0,	0,
	/* 08 */	0,	0,	0,	0,
	/* 12 */	0,	SPL1,	0,	0,
};
