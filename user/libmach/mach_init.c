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
 * $Log:	mach_init.c,v $
 * Revision 2.5  92/01/23  15:22:10  rpd
 * 	Restored non-standalone functionality,
 * 	in a way that doesn't interfere with standalone use.
 * 	[92/01/17            rpd]
 * 
 * Revision 2.4  91/05/14  17:53:00  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/14  14:17:35  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:44:06  mrt]
 * 
 * Revision 2.2  90/06/02  15:12:28  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  23:26:00  rpd]
 * 
 * Revision 2.1  89/08/03  17:05:42  rwd
 * Created.
 * 
 * 18-Jan-89  David Golub (dbg) at Carnegie-Mellon University
 *	Altered for stand-alone use:
 *	. Removed registered port list.
 *	. Removed task_data (obsolete), task_notify.
 *	. Removed Accent compatibility calls.
 *
 * 23-Nov-87  Mary Thompson (mrt) at Carnegie Mellon
 *	removed includes of <servers/msgn.h> and <servers/netname.h>
 *	as they are no longer used.
 *
 * 5-Oct-87   Mary Thompson (mrt) at Carnegie Mellon
 *	Added an extern void definition of mig_init to keep
 *	lint happy
 *
 * 30-Jul-87  Mary Thompson (mrt) at Carnegie Mellon
 *	Changed the intialization of the mig_reply_port to be
 *	a call to mig_init instead init_mach.
 *
 * 27-May-87  Richard Draves (rpd) at Carnegie-Mellon University
 *	Changed initialization of mach interface, because the
 *	new mig doesn't export an alloc_port_mach function.
 *
 * 15-May-87  Mary Thompson
 *	Removed include of sys/features.h and conditional
 *	compliations
 *
 *  4-May-87  Mary Thompson
 *	vm_deallocted the init_ports array so that brk might 
 *	have a chance to be correct.
 *
 * 24-Apr-87  Mary Thompson
 *	changed type of mach_init_ports_count to unsigned int
 *
 * 12-Nov-86	Mary Thompson
 *	Added initialization call to init_netname the new
 *	interface for the net name server.
 *
 *  7-Nov-86  Michael Young
 * 	Add "service_port" 
 *
 *  7-Sep-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added initialization for MACH_IPC, using mach_ports_lookup.
 *
 * 29-Aug-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added other interesting global ports.
 */

#include <mach.h>

extern void mig_init();
extern void mach_init_ports();

mach_port_t	mach_task_self_ = MACH_PORT_NULL;

vm_size_t	vm_page_size;

int		mach_init()
{
	vm_statistics_data_t	vm_stat;
	kern_return_t		kr;

#undef	mach_task_self

	/*
	 *	Get the important ports into the cached values,
	 *	as required by "mach_init.h".
	 */
	 
	mach_task_self_ = mach_task_self();

	/*
	 *	Initialize the single mig reply port
	 */

	mig_init(0);

	/*
	 *	Cache some other valuable system constants
	 */

	(void) vm_statistics(mach_task_self_, &vm_stat);
	vm_page_size = vm_stat.pagesize;

	mach_init_ports();

	return(0);
}

int		(*mach_init_routine)() = mach_init;
