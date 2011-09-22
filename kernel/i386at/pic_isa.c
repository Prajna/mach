/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 * $Log:	pic_isa.c,v $
 * Revision 2.5  91/05/14  16:29:38  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:20:18  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:46:58  mrt]
 * 
 * Revision 2.3  91/01/08  17:33:40  rpd
 * 	Initially, do not allow clock interrupts
 * 	[91/01/04  12:21:54  rvb]
 * 
 * Revision 2.2  90/11/26  14:50:57  rvb
 * 	jsb bet me to XMK34, sigh ...
 * 	[90/11/26            rvb]
 * 	Apparently first revision is -r2.2
 * 	[90/11/25  10:47:33  rvb]
 * 
 * 	Synched 2.5 & 3.0 at I386q (r2.1.1.2) & XMK35 (r2.2)
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
	/* 00 */   	0,	SPL6,	0,	0,
	/* 04 */	0,	0,	0,	0,
	/* 08 */	0,	0,	0,	0,
	/* 12 */	0,	SPL1,	0,	0,
};
