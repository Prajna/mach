/* 
 * Mach Operating System
 * Copyright (c) 1993,1992,1991,1990,1989 Carnegie Mellon University
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
 * $Log:	processor_info.h,v $
 * Revision 2.6  93/11/17  17:44:04  dbg
 * 	Finished the 64-bit cleanup: made processor_set_basic_info and
 * 	processor_set_sched_info be structs of integer_t, since they are
 * 	passed as processor_set_info_t.
 * 	[93/11/09            dbg]
 * 
 * Revision 2.5  93/01/14  17:46:35  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 	64bit cleanup.
 * 	[92/12/01            af]
 * 
 * Revision 2.4  91/05/14  16:58:46  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:35:31  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:20:39  mrt]
 * 
 * Revision 2.2  90/06/02  14:59:49  rpd
 * 	Created for new host/processor technology.
 * 	[90/03/26  23:51:38  rpd]
 * 
 * 	Merge to X96
 * 	[89/08/02  23:12:21  dlb]
 * 
 * 	Add scheduling flavor of information.
 * 	[89/07/25  18:52:18  dlb]
 * 
 * 	Add load average and mach factor to processor set basic info.
 * 	[89/02/09            dlb]
 * 
 * Revision 2.3  89/10/15  02:05:54  rpd
 * 	Minor cleanups.
 * 
 * Revision 2.2  89/10/11  14:41:03  dlb
 * 	Add scheduling flavor of information.
 * 
 * 	Add load average and mach factor to processor set basic info.
 * 	[89/02/09            dlb]
 * 
 */

/*
 *	File:	mach/processor_info.h
 *	Author:	David L. Black
 *	Date:	1988
 *
 *	Data structure definitions for processor_info, processor_set_info
 */

#ifndef	_MACH_PROCESSOR_INFO_H_
#define _MACH_PROCESSOR_INFO_H_

#include <mach/machine.h>

/*
 *	Generic information structure to allow for expansion.
 */
typedef integer_t	*processor_info_t;	/* varying array of int. */

#define PROCESSOR_INFO_MAX	(1024)		/* max array size */
typedef integer_t	processor_info_data_t[PROCESSOR_INFO_MAX];


typedef integer_t	*processor_set_info_t;	/* varying array of int. */

#define PROCESSOR_SET_INFO_MAX	(1024)		/* max array size */
typedef integer_t	processor_set_info_data_t[PROCESSOR_SET_INFO_MAX];

/*
 *	Currently defined information.
 */
#define	PROCESSOR_BASIC_INFO	1		/* basic information */

struct processor_basic_info {
	cpu_type_t	cpu_type;	/* type of cpu */
	cpu_subtype_t	cpu_subtype;	/* subtype of cpu */
/*boolean_t*/integer_t	running;	/* is processor running */
	integer_t	slot_num;	/* slot number */
/*boolean_t*/integer_t	is_master;	/* is this the master processor */
};

typedef	struct processor_basic_info	processor_basic_info_data_t;
typedef struct processor_basic_info	*processor_basic_info_t;
#define PROCESSOR_BASIC_INFO_COUNT \
		(sizeof(processor_basic_info_data_t)/sizeof(integer_t))


#define	PROCESSOR_SET_BASIC_INFO	1	/* basic information */

struct processor_set_basic_info {
	integer_t	processor_count;	/* How many processors */
	integer_t	task_count;		/* How many tasks */
	integer_t	thread_count;		/* How many threads */
	integer_t	load_average;		/* Scaled */
	integer_t	mach_factor;		/* Scaled */
};

/*
 *	Scaling factor for load_average, mach_factor.
 */
#define	LOAD_SCALE	1000		

typedef	struct processor_set_basic_info	processor_set_basic_info_data_t;
typedef struct processor_set_basic_info	*processor_set_basic_info_t;
#define PROCESSOR_SET_BASIC_INFO_COUNT \
		(sizeof(processor_set_basic_info_data_t)/sizeof(integer_t))

#define PROCESSOR_SET_SCHED_INFO	2	/* scheduling info */

struct processor_set_sched_info {
	integer_t	policies;	/* allowed policies */
	integer_t	max_priority;	/* max priority for new threads */
};

typedef	struct processor_set_sched_info	processor_set_sched_info_data_t;
typedef struct processor_set_sched_info	*processor_set_sched_info_t;
#define PROCESSOR_SET_SCHED_INFO_COUNT \
		(sizeof(processor_set_sched_info_data_t)/sizeof(integer_t))

#endif	/* _MACH_PROCESSOR_INFO_H_ */
