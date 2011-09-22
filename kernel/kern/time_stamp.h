/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
 * $Log:	time_stamp.h,v $
 * Revision 2.3  91/05/14  16:49:41  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:30:58  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:20:48  mrt]
 * 
 * Revision 2.1  89/08/03  15:57:28  rwd
 * Created.
 * 
 *  5-Apr-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Isolate machine dependencies - machine/time_stamp chooses a
 *	TS_FORMAT, if not choosed this module defaults it to 1.  Also
 *	guarded against multiple inclusion.
 *
 * 30-Mar-87  David Black (dlb) at Carnegie-Mellon University
 *	Created.
 *
 */ 

#ifndef	_KERN_TIME_STAMP_H_
#define _KERN_TIME_STAMP_H_

#include <machine/time_stamp.h>
/*
 *	time_stamp.h -- definitions for low-overhead timestamps.
 */

struct tsval {
	unsigned	low_val;	/* least significant word */
	unsigned	high_val;	/* most significant word */
};

/*
 *	Format definitions.
 */

#ifndef	TS_FORMAT
/*
 *	Default case - Just return a tick count for machines that
 *	don't support or haven't implemented this.  Assume 100Hz ticks.
 *
 *	low_val - Always 0.
 *	high_val - tick count.
 */
#define	TS_FORMAT	1

#if	KERNEL 
unsigned	ts_tick_count;
#endif	KERNEL
#endif	TS_FORMAT

/*
 *	List of all format definitions for convert_ts_to_tv.
 */

#define	TS_FORMAT_DEFAULT	1
#define TS_FORMAT_MMAX		2
#endif	_KERN_TIME_STAMP_H_
