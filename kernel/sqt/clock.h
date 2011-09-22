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
 * $Log:	clock.h,v $
 * Revision 2.3  91/07/31  18:00:31  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  12:55:15  dbg
 * 	Added, from Sequent SYMMETRY sources.
 * 	[91/02/26            dbg]
 * 
 */

/*
 * $Header: clock.h,v 2.3 91/07/31 18:00:31 dbg Exp $
 */

/*
 * Revision 1.1  89/07/19  14:48:53  kak
 * Initial revision
 * 
 */

#ifndef	_SQT_CLOCK_H_
#define	_SQT_CLOCK_H_

#define	HZ	100

#define TODFREQ		1000 / HZ			/* 10 milliseconds */
#define	SECDAY		((unsigned)(24*60*60))		/* seconds per day */
#define	SECYR		((unsigned)(365*SECDAY))	/* per common year */

#define	YRREF		1970
#define	LEAPYEAR(year)	((year)%4==0)	/* good till time becomes negative */

/*
 * Local and tod clock bin and vector.  The vector is defined by SLIC as 0x00.
 */

#define	LCLKBIN		7		/* highest priority */
#define	LCLKVEC		0x00		/* according to SLIC */

#define TODCLKBIN	7		/* highest priority */
#define	TODCLKVEC	0x01


/*
 * System clock rate in Mhz
 */

extern unsigned int sys_clock_rate;

#endif	/* _SQT_CLOCK_H_ */
