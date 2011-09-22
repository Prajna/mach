/* 
 * Mach Operating System
 * Copyright (c) 1992,1991,1990,1989,1988 Carnegie Mellon University
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
 * $Log:	ipc_host.h,v $
 * Revision 2.6  93/01/14  17:34:26  danner
 * 	Added function prototypes.  Combined ipc_processor_init and
 * 	ipc_processor_enable.  Removed ipc_processor_disable and
 * 	ipc_processor_terminate; processor ports are never removed.
 * 	[92/11/17            dbg]
 * 	Added function prototypes.  Combined ipc_processor_init and
 * 	ipc_processor_enable.  Removed ipc_processor_disable and
 * 	ipc_processor_terminate; processor ports are never removed.
 * 	[92/11/17            dbg]
 * 
 * Revision 2.5  91/06/25  10:28:30  rpd
 * 	Changed the convert_foo_to_bar functions
 * 	to use ipc_port_t instead of mach_port_t.
 * 	[91/05/27            rpd]
 * 
 * Revision 2.4  91/05/14  16:41:53  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:26:34  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:12:40  mrt]
 * 
 * Revision 2.2  90/06/02  14:54:04  rpd
 * 	Created for new IPC.
 * 	[90/03/26  23:46:41  rpd]
 * 
 * 	Merge to X96
 * 	[89/08/02  22:55:33  dlb]
 * 
 * 	Created.
 * 	[88/12/01            dlb]
 * 
 * Revision 2.3  89/10/15  02:04:36  rpd
 * 	Minor cleanups.
 * 
 * Revision 2.2  89/10/11  14:07:24  dlb
 * 	Merge.
 * 	[89/09/01  17:25:42  dlb]
 * 
 */ 

#ifndef	_KERN_IPC_HOST_H_
#define	_KERN_IPC_HOST_H_

#include <mach/port.h>
#include <kern/processor.h>

extern void ipc_host_init(void);

extern void ipc_processor_init(processor_t);

extern void ipc_pset_init(processor_set_t);
extern void ipc_pset_enable(processor_set_t);
extern void ipc_pset_disable(processor_set_t);
extern void ipc_pset_terminate(processor_set_t);

extern struct host *
convert_port_to_host(struct ipc_port *);

extern struct ipc_port *
convert_host_to_port(struct host *);

extern struct host *
convert_port_to_host_priv(struct ipc_port *);

extern processor_t
convert_port_to_processor(struct ipc_port *);

extern struct ipc_port *
convert_processor_to_port(processor_t);

extern processor_set_t
convert_port_to_pset(struct ipc_port *);

extern struct ipc_port *
convert_pset_to_port(processor_set_t);

extern processor_set_t
convert_port_to_pset_name(struct ipc_port *);

extern struct ipc_port *
convert_pset_name_to_port(processor_set_t);

#endif	_KERN_IPC_HOST_H_
