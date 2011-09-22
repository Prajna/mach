/* 
 * Mach Operating System
 * Copyright (c) 1993,1992,1991,1990,1989,1988 Carnegie Mellon University
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
 * $Log:	host_info.h,v $
 * Revision 2.7  93/11/17  17:38:28  dbg
 * 	Finished the 64-bit cleanup: made host_sched_info be a struct of
 * 	integer_t, since it is passed as a host_info_t.
 * 	[93/11/09            dbg]
 * 
 * Revision 2.6  93/01/14  17:41:41  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 	64bit cleanup.
 * 	[92/12/01            af]
 * 
 * Revision 2.5  92/03/10  16:26:51  jsb
 * 	From durriya@ri.osf.org: defined kernel_boot_info_t.
 * 	[92/03/07  08:16:27  jsb]
 * 
 * Revision 2.4  91/05/14  16:51:48  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:31:58  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:17:13  mrt]
 * 
 * Revision 2.2  90/06/02  14:57:58  rpd
 * 	Added HOST_LOAD_INFO and related definitions.
 * 	[90/04/27            rpd]
 * 	Created for new host/processor technology.
 * 	[90/03/26  23:50:51  rpd]
 * 
 * 	Cleanup changes.
 * 	[89/08/02            dlb]
 * 	Add sched_info flavor to return minimum times for use by
 * 	external schedulers.
 * 	[89/06/08            dlb]
 * 	Added kernel_version type definitions.
 * 	[88/12/02            dlb]
 * 
 * Revision 2.4  89/10/15  02:05:31  rpd
 * 	Minor cleanups.
 * 
 * Revision 2.3  89/10/11  17:32:15  dlb
 * 	Include mach/machine/vm_types.h instead of mach/vm_param.h
 * 	[89/10/11            dlb]
 * 
 * Revision 2.2  89/10/11  14:36:55  dlb
 * 	Add sched_info flavor to return minimum times for use by
 * 	external schedulers.
 * 	[89/06/08            dlb]
 * 
 * 	Added kernel_version type definitions.
 * 	[88/12/02            dlb]
 * 
 * 30-Nov-88  David Black (dlb) at Carnegie-Mellon University
 *	Created.  2 flavors so far: basic info,  slot numbers.
 *
 */

/*
 *	File:	mach/host_info.h
 *
 *	Definitions for host_info call.
 */

#ifndef	_MACH_HOST_INFO_H_
#define	_MACH_HOST_INFO_H_

#include <mach/machine.h>
#include <mach/machine/vm_types.h>

/*
 *	Generic information structure to allow for expansion.
 */
typedef integer_t	*host_info_t;	/* varying array of integers */

#define	HOST_INFO_MAX	(1024)		/* max array size */
typedef integer_t	host_info_data_t[HOST_INFO_MAX];

#define KERNEL_VERSION_MAX (512)
typedef char	kernel_version_t[KERNEL_VERSION_MAX];

#define KERNEL_BOOT_INFO_MAX (4096)
typedef char	kernel_boot_info_t[KERNEL_BOOT_INFO_MAX];

/*
 *	Currently defined information.
 */
#define HOST_BASIC_INFO		1	/* basic info */
#define HOST_PROCESSOR_SLOTS	2	/* processor slot numbers */
#define HOST_SCHED_INFO		3	/* scheduling info */
#define	HOST_LOAD_INFO		4	/* avenrun/mach_factor info */

struct host_basic_info {
	integer_t	max_cpus;	/* max number of cpus possible */
	integer_t	avail_cpus;	/* number of cpus now available */
	vm_size_t	memory_size;	/* size of memory in bytes */
	cpu_type_t	cpu_type;	/* cpu type */
	cpu_subtype_t	cpu_subtype;	/* cpu subtype */
};

typedef	struct host_basic_info	host_basic_info_data_t;
typedef struct host_basic_info	*host_basic_info_t;
#define HOST_BASIC_INFO_COUNT \
		(sizeof(host_basic_info_data_t)/sizeof(integer_t))

struct host_sched_info {
	integer_t	min_timeout;	/* minimum timeout in milliseconds */
	integer_t	min_quantum;	/* minimum quantum in milliseconds */
};

typedef	struct host_sched_info	host_sched_info_data_t;
typedef struct host_sched_info	*host_sched_info_t;
#define HOST_SCHED_INFO_COUNT \
		(sizeof(host_sched_info_data_t)/sizeof(integer_t))

struct host_load_info {
	integer_t	avenrun[3];	/* scaled by LOAD_SCALE */
	integer_t	mach_factor[3];	/* scaled by LOAD_SCALE */
};

typedef struct host_load_info	host_load_info_data_t;
typedef struct host_load_info	*host_load_info_t;
#define	HOST_LOAD_INFO_COUNT \
		(sizeof(host_load_info_data_t)/sizeof(integer_t))

#endif	/* _MACH_HOST_INFO_H_ */
