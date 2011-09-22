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
 * Revision 2.6  93/01/14  17:44:24  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 
 * Revision 2.5  92/01/15  13:44:51  rpd
 * 	Changed MACH_IPC_COMPAT conditionals to default to not present.
 * 
 * Revision 2.4  91/05/14  16:54:40  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:33:28  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:18:01  mrt]
 * 
 * Revision 2.2  90/06/02  14:58:21  rpd
 * 	Created.
 * 	[90/03/26  23:56:39  rpd]
 * 
 *
 * Condensed history:
 *	Moved implementation constants elsewhere (rpd).
 *	Added SET_MAX (rpd).
 *	Added KERN_MSG_SMALL_SIZE (mwyoung).
 *	Added PORT_BACKLOG_MAX (mwyoung).
 *	Added PORT_BACKLOG_MAX (mwyoung).
 *	Added TASK_PORT_REGISTER_MAX (mwyoung).
 *	Created (mwyoung).
 */
/*
 *	File:	mach/mach_param.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1986
 *
 *	Mach system sizing parameters
 */

#ifndef	_MACH_MACH_PARAM_H_
#define _MACH_MACH_PARAM_H_

#ifdef	KERNEL
#include <mach_ipc_compat.h>
#endif	/* KERNEL */

#define TASK_PORT_REGISTER_MAX	4	/* Number of "registered" ports */


/* Definitions for the old IPC interface. */

#if	MACH_IPC_COMPAT

#define PORT_BACKLOG_DEFAULT	5
#define PORT_BACKLOG_MAX	16

#endif	/* MACH_IPC_COMPAT */

#endif	/* _MACH_MACH_PARAM_H_ */
