/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
 * $Log:	host.h,v $
 * Revision 2.4  91/05/14  16:41:25  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:26:20  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:12:24  mrt]
 * 
 * Revision 2.2  90/06/02  14:53:53  rpd
 * 	Created for new host/processor technology.
 * 	[90/03/26  23:50:40  rpd]
 * 
 * 	Cleanup changes.
 * 	[89/08/02            dlb]
 * 	extern realhost declaration.
 * 	[89/02/03            dlb]
 * 
 * Revision 2.3  89/10/15  02:04:17  rpd
 * 	Minor cleanups.
 * 
 * Revision 2.2  89/10/11  14:05:08  dlb
 * 	Cleanup.
 * 	[89/08/02            dlb]
 * 
 */

/*
 *	kern/host.h
 *
 *	Definitions for host data structures.
 *
 */

#ifndef	_KERN_HOST_H_
#define _KERN_HOST_H_

struct	host {
    	struct ipc_port *host_self;
	struct ipc_port *host_priv_self;
};

typedef struct host	*host_t;
typedef struct host	host_data_t;

#define HOST_NULL	((host_t)0)

extern host_data_t	realhost;

#endif	_KERN_HOST_H_
