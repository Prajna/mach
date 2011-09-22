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
 * $Log:	net_io.h,v $
 * Revision 2.8  91/05/14  15:59:55  mrt
 * 	Correcting copyright
 * 
 * Revision 2.7  91/03/16  14:43:26  rpd
 * 	Added net_packet.
 * 	[91/01/14            rpd]
 * 
 * Revision 2.6  91/02/05  17:10:03  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:30:15  mrt]
 * 
 * Revision 2.5  91/01/08  15:09:56  rpd
 * 	Replaced NET_KMSG_GET, NET_KMSG_FREE
 * 	with net_kmsg_get, net_kmsg_put, net_kmsg_collect.
 * 	[91/01/05            rpd]
 * 
 * Revision 2.4  90/06/02  14:48:20  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  21:58:23  rpd]
 * 
 * Revision 2.3  90/02/22  20:02:26  dbg
 * 	kmsg->queue_head becomds kmsg->chain.
 * 	[90/01/25            dbg]
 * 
 * Revision 2.2  90/01/11  11:42:26  dbg
 * 	Make run in parallel.
 * 	[89/11/27            dbg]
 * 
 */
/*
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date: 	ll/89
 */

#ifndef	_DEVICE_NET_IO_H_
#define	_DEVICE_NET_IO_H_

/*
 * Utilities for playing with network messages.
 */

#include <mach/machine/vm_types.h>
#include <ipc/ipc_kmsg.h>

#include <kern/macro_help.h>
#include <kern/lock.h>
#include <kern/kalloc.h>

#include <device/net_status.h>

/*
 * A network packet is wrapped in a kernel message while in
 * the kernel.
 */

#define	net_kmsg(kmsg)	((net_rcv_msg_t)&(kmsg)->ikm_header)

/*
 * Interrupt routines may allocate and free net_kmsgs with these
 * functions.  net_kmsg_get may return IKM_NULL.
 */

extern ipc_kmsg_t net_kmsg_get();
extern void net_kmsg_put();

/*
 * Network utility routines.
 */

extern void net_packet();
extern void net_filter();
extern io_return_t net_getstat();
extern io_return_t net_write();

/*
 * Non-interrupt code may allocate and free net_kmsgs with these functions.
 */

extern vm_size_t net_kmsg_size;

#define net_kmsg_alloc()	((ipc_kmsg_t) kalloc(net_kmsg_size))
#define net_kmsg_free(kmsg)	kfree((vm_offset_t) (kmsg), net_kmsg_size)

#endif	_DEVICE_NET_IO_H_
