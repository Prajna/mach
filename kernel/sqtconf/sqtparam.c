/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * Copyright (c) 1991 Sequent Computer Systems
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND SEQUENT COMPUTER SYSTEMS ALLOW FREE USE OF
 * THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * SEQUENT COMPUTER SYSTEMS DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	sqtparam.c,v $
 * Revision 2.3  91/07/31  18:06:02  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:04:20  dbg
 * 	Adapted for pure Mach kernel.
 * 	[90/10/03            dbg]
 * 
 */

#include <sqt/sqtparam.h>
#include <sqt/clock.h>

/*
 * light_show decides which LED's are used to display system activity.
 * B8k's can only display on the processor board LED's; B21k's can display
 * on the front-panel and processor board LED's.  light_show == 1 displays
 * in one place (front-panel if B21k); light_show == 2 displays in both places
 * if possible; light_show == 0 doesn't flash at all (quite boring).
 */

#ifdef MFG
int	light_show = 2;		/* display on everywhere (==2) */
#else
int	light_show = 1;		/* display on most useful place (==1) */
#endif

#ifndef	CPUSPEED		/* approximate speed of cpu in VAX MIPS */
				/* scaled for an 100Mhz machine         */
#define	CPUSPEED	25	/* used for spin loops in various places */
#endif
int	lcpuspeed	= CPUSPEED;
int	memintvl	= MEMINTVL * HZ;

/*
 * Non-zero resphysmem constrains amount of physical memory the system will
 * use, reserving some for special purpose drivers, accelerators, etc.
 * Physical address of reserved memory is held in `topmem'.
 * Must be a multiple of CLBYTES.
 */

#ifndef	RESPHYSMEM
#define	RESPHYSMEM	0	/* default: none reserved */
#endif

int	resphysmem = RESPHYSMEM;

#ifndef CPURATE
#define CPURATE		16	/* lowest cpu rate to used (Mhz) */
#endif
int	cpurate		= CPURATE;
