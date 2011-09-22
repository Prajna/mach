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
 * $Log:	time_value.h,v $
 * Revision 2.6  93/01/24  14:13:24  danner
 * 	Added include of mach/machine/vm_types.h to get definition
 * 	of integer_t.
 * 	[93/01/17            mrt]
 * 
 * Revision 2.5  93/01/14  17:48:20  danner
 * 	Standardized include symbol name.
 * 	[92/06/10            pds]
 * 	64bit cleanup.
 * 	[92/12/01            af]
 * 
 * Revision 2.4  91/05/18  14:35:13  rpd
 * 	Added mapped_time_value_t.
 * 	[91/03/21            rpd]
 * 
 * Revision 2.3  91/05/14  17:01:40  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:36:49  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:22:07  mrt]
 * 
 * Revision 2.1  89/08/03  16:06:24  rwd
 * Created.
 * 
 * Revision 2.4  89/02/25  18:41:34  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.3  89/02/07  00:53:58  mwyoung
 * Relocated from sys/time_value.h
 * 
 * Revision 2.2  89/01/31  01:21:58  rpd
 * 	TIME_MICROS_MAX should be 1 Million, not 10 Million.
 * 	[88/10/12            dlb]
 * 
 *  4-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Created.
 *
 */

#ifndef	_MACH_TIME_VALUE_H_
#define	_MACH_TIME_VALUE_H_

#include <mach/machine/vm_types.h>

/*
 *	Time value returned by kernel.
 */

struct time_value {
	integer_t	seconds;
	integer_t	microseconds;
};
typedef	struct time_value	time_value_t;

/*
 *	Macros to manipulate time values.  Assume that time values
 *	are normalized (microseconds <= 999999).
 */
#define	TIME_MICROS_MAX	(1000000)

#define	time_value_add_usec(val, micros)	{	\
	if (((val)->microseconds += (micros))		\
		>= TIME_MICROS_MAX) {			\
	    (val)->microseconds -= TIME_MICROS_MAX;	\
	    (val)->seconds++;				\
	}						\
}

#define	time_value_add(result, addend)		{		\
	(result)->microseconds += (addend)->microseconds;	\
	(result)->seconds += (addend)->seconds;			\
	if ((result)->microseconds >= TIME_MICROS_MAX) {	\
	    (result)->microseconds -= TIME_MICROS_MAX;		\
	    (result)->seconds++;				\
	}							\
}

/*
 *	Time value available through the mapped-time interface.
 *	Read this mapped value with
 *		do {
 *			secs = mtime->seconds;
 *			usecs = mtime->microseconds;
 *		} while (secs != mtime->check_seconds);
 */

typedef struct mapped_time_value {
	integer_t seconds;
	integer_t microseconds;
	integer_t check_seconds;
} mapped_time_value_t;

#endif	/* _MACH_TIME_VALUE_H_ */
