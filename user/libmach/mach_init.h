/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987,1986 Carnegie Mellon University
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
 * $Log:	mach_init.h,v $
 * Revision 2.5  92/01/23  15:22:14  rpd
 * 	Restored non-standalone functionality.
 * 	It doesn't get in the way of standalone use.
 * 	Redefined trunc_page and round_page to avoid division.
 * 	[92/01/17            rpd]
 * 
 * Revision 2.4  91/05/14  17:53:06  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/14  14:17:39  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:44:11  mrt]
 * 
 * Revision 2.2  90/06/02  15:12:32  rpd
 * 	Converted to new IPC.
 * 	Added user versions of round_page and trunc_page.
 * 	[90/03/26  23:26:34  rpd]
 * 
 * Revision 2.1  89/08/03  17:06:52  rwd
 * Created.
 * 
 * 18-Jan-89  David Golub (dbg) at Carnegie-Mellon University
 *	Altered for stand-alone use:
 *	. Removed registered port list.
 *	. Removed task_data (obsolete), task_notify.
 *
 * 18-May-87  Mary Thompson
 *	removed dependency on MACH_ACC || MACH_IPC
 *
 * 24-Apr-86  Mary Thompson
 *	changed type of mach_init_ports_count to unsigned int
 *
 *  7-Nov-86  Michael Young
 * 	Add "service_port" 
 *
 *  8-Sep-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added definition of mach_init_ports items for server startups.
 *
 */

/*
 *	Items provided by the Mach environment initialization.
 */

#ifndef	_MACH_INIT_
#define	_MACH_INIT_	1

#include <mach/mach_types.h>

/*
 *	Calls to the Unix emulation to supply privileged ports:
 *	the privileged host port and the master device port.
 */

#define mach_host_priv_self()		task_by_pid(-1)
#define mach_master_device_port()	task_by_pid(-2)

/*
 *	Kernel-related ports; how a task/thread controls itself
 */

extern	mach_port_t	mach_task_self_;

#define	mach_task_self() mach_task_self_

#define	current_task()	mach_task_self()

/*
 *	Other important ports in the Mach user environment
 */

extern	mach_port_t	name_server_port;
extern	mach_port_t	environment_port;
extern	mach_port_t	service_port;

/*
 *	Where these ports occur in the "mach_ports_register"
 *	collection... only servers or the runtime library need know.
 */

#if	MACH_INIT_SLOTS
#define	NAME_SERVER_SLOT	0
#define	ENVIRONMENT_SLOT	1
#define SERVICE_SLOT		2

#define	MACH_PORTS_SLOTS_USED	3
#endif	MACH_INIT_SLOTS

/*
 *	Globally interesting numbers.
 *	These macros assume vm_page_size is a power-of-2.
 */

extern	vm_size_t	vm_page_size;

#define trunc_page(x)	((x) &~ (vm_page_size - 1))
#define round_page(x)	trunc_page((x) + (vm_page_size - 1))

#endif	_MACH_INIT_
