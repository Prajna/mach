/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	ipc_pset.h,v $
 * Revision 2.6  91/10/09  16:10:13  af
 * 	 Revision 2.5.2.1  91/09/16  10:15:55  rpd
 * 	 	Added (unconditional) ipc_pset_print declaration.
 * 	 	[91/09/02            rpd]
 * 
 * Revision 2.5.2.1  91/09/16  10:15:55  rpd
 * 	Added (unconditional) ipc_pset_print declaration.
 * 	[91/09/02            rpd]
 * 
 * Revision 2.5  91/05/14  16:35:58  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:23:20  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:50:34  mrt]
 * 
 * Revision 2.3  90/11/05  14:29:57  rpd
 * 	Added ipc_pset_reference, ipc_pset_release.
 * 	[90/10/29            rpd]
 * 
 * Revision 2.2  90/06/02  14:51:23  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:02:09  rpd]
 * 
 */
/*
 *	File:	ipc/ipc_pset.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Definitions for port sets.
 */

#ifndef	_IPC_IPC_PSET_H_
#define _IPC_IPC_PSET_H_

#include <mach/port.h>
#include <mach/kern_return.h>
#include <ipc/ipc_object.h>
#include <ipc/ipc_mqueue.h>

typedef struct ipc_pset {
	struct ipc_object ips_object;

	mach_port_t ips_local_name;
	struct ipc_mqueue ips_messages;
} *ipc_pset_t;

#define	ips_references		ips_object.io_references

#define	IPS_NULL		((ipc_pset_t) IO_NULL)

#define	ips_active(pset)	io_active(&(pset)->ips_object)
#define	ips_lock(pset)		io_lock(&(pset)->ips_object)
#define	ips_lock_try(pset)	io_lock_try(&(pset)->ips_object)
#define	ips_unlock(pset)	io_unlock(&(pset)->ips_object)
#define	ips_check_unlock(pset)	io_check_unlock(&(pset)->ips_object)
#define	ips_reference(pset)	io_reference(&(pset)->ips_object)
#define	ips_release(pset)	io_release(&(pset)->ips_object)

extern kern_return_t
ipc_pset_alloc(/* ipc_space_t, mach_port_t *, ipc_pset_t * */);

extern kern_return_t
ipc_pset_alloc_name(/* ipc_space_t, mach_port_t, ipc_pset_t * */);

extern void
ipc_pset_add(/* ipc_pset_t, ipc_port_t */);

extern void
ipc_pset_remove(/* ipc_pset_t, ipc_port_t */);

extern kern_return_t
ipc_pset_move(/* ipc_space_t, mach_port_t, mach_port_t */);

extern void
ipc_pset_destroy(/* ipc_pset_t */);

#define	ipc_pset_reference(pset)	\
		ipc_object_reference(&(pset)->ips_object)

#define	ipc_pset_release(pset)		\
		ipc_object_release(&(pset)->ips_object)

extern void
ipc_pset_print(/* ipc_pset_t */);

#endif	_IPC_IPC_PSET_H_
