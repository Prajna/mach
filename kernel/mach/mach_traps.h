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
 * $Log:	mach_traps.h,v $
 * Revision 2.8  93/05/10  17:47:51  rvb
 * 		386BSD gets confused iff mach_task_self is still defined.
 * 	[93/05/05            rvb]
 * 
 * Revision 2.7  93/01/14  17:44:28  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 
 * Revision 2.6  92/01/15  13:44:25  rpd
 * 	Changed MACH_IPC_COMPAT conditionals to default to not present.
 * 
 * Revision 2.5  91/05/14  16:54:55  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:33:35  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:18:21  mrt]
 * 
 * Revision 2.3  90/08/06  17:06:07  rpd
 * 	Removed mach_host_priv_self.
 * 	Removed definition of _MACH_INIT_.
 * 	[90/08/04            rpd]
 * 
 * Revision 2.2  90/06/02  14:58:30  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  22:33:07  rpd]
 * 
 * Revision 2.1  89/08/03  15:59:29  rwd
 * Created.
 * 
 * Revision 2.4  89/02/25  18:37:15  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.3  89/02/19  12:57:44  rpd
 * 	Moved from kern/ to mach/.
 * 
 * Revision 2.2  89/01/15  16:24:46  rpd
 * 	Updated includes for the new mach/ directory.
 * 	[89/01/15  15:03:03  rpd]
 * 
 * 18-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Add thread_reply.  Leave in task_data as an alternate name -
 *	they are functionally indistinguishable.
 *
 * 15-Oct-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Include ../kern/mach_types.h instead of <kern/mach_types.h> when
 *	building for the kernel.
 *
 *  1-Sep-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Created, mostly to help build the lint library.
 *	Should eventually include this in "syscall_sw.c".
 *
 */
/*
 *	Definitions of general Mach system traps.
 *
 *	IPC traps are defined in <mach/message.h>.
 *	Kernel RPC functions are defined in <mach/mach_interface.h>.
 */

#ifndef	_MACH_MACH_TRAPS_H_
#define _MACH_MACH_TRAPS_H_

#ifdef	KERNEL
#include <mach_ipc_compat.h>
#endif	/* KERNEL */

#include <mach/port.h>

mach_port_t	mach_reply_port
#ifdef	LINTLIBRARY
			()
	 { return MACH_PORT_NULL; }
#else	/* LINTLIBRARY */
			();
#endif	/* LINTLIBRARY */

mach_port_t	mach_thread_self
#ifdef	LINTLIBRARY
			()
	 { return MACH_PORT_NULL; }
#else	/* LINTLIBRARY */
			();
#endif	/* LINTLIBRARY */

#ifdef	__386BSD__
#undef mach_task_self
#endif
mach_port_t	mach_task_self
#ifdef	LINTLIBRARY
			()
	 { return MACH_PORT_NULL; }
#else	/* LINTLIBRARY */
			();
#endif	/* LINTLIBRARY */

mach_port_t	mach_host_self
#ifdef	LINTLIBRARY
			()
	 { return MACH_PORT_NULL; }
#else	/* LINTLIBRARY */
			();
#endif	/* LINTLIBRARY */


/* Definitions for the old IPC interface. */

#if	MACH_IPC_COMPAT

port_t		task_self
#ifdef	LINTLIBRARY
			()
	 { return(PORT_NULL); }
#else	/* LINTLIBRARY */
			();
#endif	/* LINTLIBRARY */

port_t		task_notify
#ifdef	LINTLIBRARY
			()
	 { return(PORT_NULL); }
#else	/* LINTLIBRARY */
			();
#endif	/* LINTLIBRARY */

port_t		thread_self
#ifdef	LINTLIBRARY
			()
	 { return(PORT_NULL); }
#else	/* LINTLIBRARY */
			();
#endif	/* LINTLIBRARY */

port_t		thread_reply
#ifdef	LINTLIBRARY
			()
	 { return(PORT_NULL); }
#else	/* LINTLIBRARY */
			();
#endif	/* LINTLIBRARY */

port_t		host_self
#ifdef	LINTLIBRARY
			()
	 { return(PORT_NULL); }
#else	/* LINTLIBRARY */
			();
#endif	/* LINTLIBRARY */

port_t		host_priv_self
#ifdef	LINTLIBRARY
			()
	 { return(PORT_NULL); }
#else	/* LINTLIBRARY */
			();
#endif	/* LINTLIBRARY */

#endif	/* MACH_IPC_COMPAT */

#endif	/* _MACH_MACH_TRAPS_H_ */
