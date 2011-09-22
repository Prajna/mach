/* 
 * Mach Operating System
 * Copyright (c) 1992,1991,1990 Carnegie Mellon University
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
 * $Log:	ipc_sched.h,v $
 * Revision 2.8  93/01/14  17:34:45  danner
 * 	Moved contents to kern/sched_prim.h.
 * 	[92/12/28            dbg]
 * 
 * Revision 2.7  92/04/04  15:19:24  rpd
 * 	Added convert_ipc_timeout_to_ticks.
 * 	[92/04/04            rpd]
 * 
 * Revision 2.6  91/05/14  16:42:41  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/03/16  14:50:17  rpd
 * 	Replaced ipc_thread_switch with thread_handoff.
 * 	Renamed ipc_thread_{go,will_wait,will_wait_with_timeout}
 * 	to thread_{go,will_wait,will_wait_with_timeout}.
 * 	Removed ipc_thread_block.
 * 	[91/02/17            rpd]
 * 
 * Revision 2.4  91/02/05  17:26:58  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:13:32  mrt]
 * 
 * Revision 2.3  91/01/08  15:15:58  rpd
 * 	Removed ipc_thread_go_and_block.
 * 	Added ipc_thread_switch.
 * 	Added continuation argument to ipc_thread_block.
 * 	[90/12/08            rpd]
 * 
 * Revision 2.2  90/06/02  14:54:27  rpd
 * 	Created for new IPC.
 * 	[90/03/26  23:46:16  rpd]
 * 
 */

#ifndef	_KERN_IPC_SCHED_H_
#define	_KERN_IPC_SCHED_H_

#include <kern/sched_prim.h>

#endif	/* _KERN_IPC_SCHED_H_ */
