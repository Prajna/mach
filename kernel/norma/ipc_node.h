/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * $Log:	ipc_node.h,v $
 * Revision 2.4  92/03/10  16:28:03  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:50:06  jsb]
 * 
 * Revision 2.3.2.2  92/01/09  18:45:45  jsb
 * 	Use MAX_SPECIAL_KERNEL_ID, not MAX_SPECIAL_ID, in IP_NORMA_SPECIAL.
 * 	[92/01/04  18:26:48  jsb]
 * 
 * Revision 2.3.2.1  92/01/03  16:37:54  jsb
 * 	Added IP_NORMA_{MAX_LID,SPECIAL} macros, and declaration
 * 	of host_special_port array.
 * 	[91/12/24  14:25:35  jsb]
 * 
 * Revision 2.3  91/12/13  14:00:49  jsb
 * 	Moved MAX_SPECIAL_ID to mach/norma_special_ports.h.
 * 
 * Revision 2.2  91/11/14  16:46:02  rpd
 * 	Created.
 * 
 */
/*
 *	File:	norma/ipc_node.h
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Definitions for norma nodes.
 */

#ifndef	_NORMA_IPC_NODE_H_
#define	_NORMA_IPC_NODE_H_

#include <mach/norma_special_ports.h>

/*
 * A uid (unique id) = node + lid (local id)
 * We embed the node id in the uid because receive rights rarely move.
 */

/*
 * 12 bits for node		-> max 4096 nodes
 * 20 bits for local id		-> max 1,048,575 exported ports per node
 *
 * (Byte allignment and intuitive node:lid placement aids debugging)
 */
#define	IP_NORMA_NODE(uid)	(((unsigned long)(uid) >> 20) & 0x00000fff)
#define	IP_NORMA_LID(uid)	((unsigned long)(uid) & 0x000fffff)
#define	IP_NORMA_UID(node, lid)	(((node) << 20) | (lid))

#define	IP_NORMA_MAX_LID	IP_NORMA_LID(0xffffffff)
#define	IP_NORMA_SPECIAL(uid)	(IP_NORMA_LID(uid) > 0 && \
				 IP_NORMA_LID(uid) <= MAX_SPECIAL_KERNEL_ID)

extern ipc_port_t		host_special_port[MAX_SPECIAL_ID];

#endif	_NORMA_IPC_NODE_H_
