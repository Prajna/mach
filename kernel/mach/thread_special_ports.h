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
 * $Log:	thread_special_ports.h,v $
 * Revision 2.6  93/01/14  17:48:03  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 
 * Revision 2.5  92/01/15  13:44:57  rpd
 * 	Changed MACH_IPC_COMPAT conditionals to default to not present.
 * 
 * Revision 2.4  91/05/14  17:01:15  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:36:38  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:21:48  mrt]
 * 
 * Revision 2.2  90/06/02  15:00:15  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  22:41:20  rpd]
 * 
 * Revision 2.1  89/08/03  16:06:13  rwd
 * Created.
 * 
 * Revision 2.3  89/02/25  18:41:23  gm0w
 * 	Changes for cleanup.
 * 
 * 17-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Created.
 *
 */
/*
 *	File:	mach/thread_special_ports.h
 *
 *	Defines codes for special_purpose thread ports.  These are NOT
 *	port identifiers - they are only used for the thread_get_special_port
 *	and thread_set_special_port routines.
 *	
 */

#ifndef	_MACH_THREAD_SPECIAL_PORTS_H_
#define _MACH_THREAD_SPECIAL_PORTS_H_

#ifdef	KERNEL
#include <mach_ipc_compat.h>
#endif	/* KERNEL */

#define THREAD_KERNEL_PORT	1	/* Represents the thread to the outside
					   world.*/
#define THREAD_EXCEPTION_PORT	3	/* Exception messages for the thread
					   are sent to this port. */

/*
 *	Definitions for ease of use
 */

#define thread_get_kernel_port(thread, port)	\
		(thread_get_special_port((thread), THREAD_KERNEL_PORT, (port)))

#define thread_set_kernel_port(thread, port)	\
		(thread_set_special_port((thread), THREAD_KERNEL_PORT, (port)))

#define thread_get_exception_port(thread, port)	\
		(thread_get_special_port((thread), THREAD_EXCEPTION_PORT, (port)))

#define thread_set_exception_port(thread, port)	\
		(thread_set_special_port((thread), THREAD_EXCEPTION_PORT, (port)))


/* Definitions for the old IPC interface. */

#if	MACH_IPC_COMPAT

#define THREAD_REPLY_PORT	2	/* Default reply port for the thread's
					   use. */

#define thread_get_reply_port(thread, port)	\
		(thread_get_special_port((thread), THREAD_REPLY_PORT, (port)))

#define thread_set_reply_port(thread, port)	\
		(thread_set_special_port((thread), THREAD_REPLY_PORT, (port)))

#endif	/* MACH_IPC_COMPAT */

#endif	/* _MACH_THREAD_SPECIAL_PORTS_H_ */
