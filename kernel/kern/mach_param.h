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
 * $Log:	mach_param.h,v $
 * Revision 2.6  92/03/10  16:26:45  jsb
 * 	19-Feb-92 David L. Black (dlb) at Open Software Foundation
 * 	Double object slop in PORT_MAX; allow for extra (non-task)
 * 	ipc spaces (e.g. ipc_space_remote) in SPACE_MAX
 * 	[92/03/07  08:51:33  jsb]
 * 
 * Revision 2.5  91/05/14  16:44:25  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:27:56  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:15:07  mrt]
 * 
 * Revision 2.3  90/06/02  14:55:13  rpd
 * 	Added new IPC parameters.
 * 	[90/03/26  22:11:55  rpd]
 * 
 *
 * Condensed history:
 *	Moved TASK_MAX, PORT_MAX, etc. here from mach/mach_param.h (rpd).
 */
/*
 *	File:	kern/mach_param.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1986
 *
 *	Mach system sizing parameters
 *
 */

#ifndef	_KERN_MACH_PARAM_H_
#define _KERN_MACH_PARAM_H_

#define THREAD_MAX	1024		/* Max number of threads */
#define THREAD_CHUNK	64		/* Allocation chunk */

#define TASK_MAX	1024		/* Max number of tasks */
#define TASK_CHUNK	64		/* Allocation chunk */

#define PORT_MAX	((TASK_MAX * 3 + THREAD_MAX)	/* kernel */ \
				+ (THREAD_MAX * 2)	/* user */ \
				+ 40000)		/* slop for objects */
					/* Number of ports, system-wide */

#define SET_MAX		(TASK_MAX + THREAD_MAX + 200)
					/* Max number of port sets */

#define	ITE_MAX		(1 << 16)	/* Max number of splay tree entries */

#define	SPACE_MAX	(TASK_MAX + 5)	/* Max number of IPC spaces */

#define	IMAR_MAX	(1 << 10)	/* Max number of msg-accepted reqs */

#endif	_KERN_MACH_PARAM_H_
