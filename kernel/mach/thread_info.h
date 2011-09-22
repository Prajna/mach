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
 * $Log:	thread_info.h,v $
 * Revision 2.5  93/01/14  17:47:57  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 	64bit cleanup.
 * 	[92/12/01            af]
 * 
 * Revision 2.4  91/05/14  17:01:06  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:36:34  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:21:39  mrt]
 * 
 * Revision 2.2  90/06/02  15:00:08  rpd
 * 	Updated for new scheduling info.
 * 	[90/03/26  22:40:55  rpd]
 * 
 * Revision 2.1  89/08/03  16:06:07  rwd
 * Created.
 * 
 * Revision 2.4  89/02/25  18:41:18  gm0w
 * 	Changes for cleanup.
 * 
 *  4-Mar-88  David Black (dlb) at Carnegie-Mellon University
 *	Added TH_USAGE_SCALE for cpu_usage field.
 *
 * 15-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Changed to generic interface (variable-length array) to allow
 *	for expansion.  Renamed to thread_info.
 *
 *  1-Jun-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Created.
 *
 */
/*
 *	File:	mach/thread_info
 *
 *	Thread information structure and definitions.
 *
 *	The defintions in this file are exported to the user.  The kernel
 *	will translate its internal data structures to these structures
 *	as appropriate.
 *
 */

#ifndef	_MACH_THREAD_INFO_H_
#define _MACH_THREAD_INFO_H_

#include <mach/boolean.h>
#include <mach/policy.h>
#include <mach/time_value.h>

/*
 *	Generic information structure to allow for expansion.
 */
typedef	integer_t	*thread_info_t;		/* varying array of ints */

#define THREAD_INFO_MAX		(1024)	/* maximum array size */
typedef	integer_t	thread_info_data_t[THREAD_INFO_MAX];

/*
 *	Currently defined information.
 */
#define THREAD_BASIC_INFO	1		/* basic information */

struct thread_basic_info {
	time_value_t	user_time;	/* user run time */
	time_value_t	system_time;	/* system run time */
	integer_t	cpu_usage;	/* scaled cpu usage percentage */
	integer_t	base_priority;	/* base scheduling priority */
	integer_t	cur_priority;	/* current scheduling priority */
	integer_t	run_state;	/* run state (see below) */
	integer_t	flags;		/* various flags (see below) */
	integer_t	suspend_count;	/* suspend count for thread */
	integer_t	sleep_time;	/* number of seconds that thread
					   has been sleeping */
};

typedef struct thread_basic_info	thread_basic_info_data_t;
typedef struct thread_basic_info	*thread_basic_info_t;
#define THREAD_BASIC_INFO_COUNT	\
		(sizeof(thread_basic_info_data_t) / sizeof(natural_t))

/*
 *	Scale factor for usage field.
 */

#define TH_USAGE_SCALE	1000

/*
 *	Thread run states (state field).
 */

#define TH_STATE_RUNNING	1	/* thread is running normally */
#define TH_STATE_STOPPED	2	/* thread is stopped */
#define TH_STATE_WAITING	3	/* thread is waiting normally */
#define TH_STATE_UNINTERRUPTIBLE 4	/* thread is in an uninterruptible
					   wait */
#define TH_STATE_HALTED		5	/* thread is halted at a
					   clean point */

/*
 *	Thread flags (flags field).
 */
#define TH_FLAGS_SWAPPED	0x1	/* thread is swapped out */
#define TH_FLAGS_IDLE		0x2	/* thread is an idle thread */

#define THREAD_SCHED_INFO	2

struct thread_sched_info {
	integer_t	policy;		/* scheduling policy */
	integer_t	data;		/* associated data */
	integer_t	base_priority;	/* base priority */
	integer_t	max_priority;   /* max priority */
	integer_t	cur_priority;	/* current priority */
/*boolean_t*/integer_t	depressed;	/* depressed ? */
	integer_t	depress_priority; /* priority depressed from */
};

typedef struct thread_sched_info	thread_sched_info_data_t;
typedef struct thread_sched_info	*thread_sched_info_t;
#define	THREAD_SCHED_INFO_COUNT	\
		(sizeof(thread_sched_info_data_t) / sizeof(natural_t))

#endif	/* _MACH_THREAD_INFO_H_ */
