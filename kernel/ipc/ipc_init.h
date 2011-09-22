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
 * $Log:	ipc_init.h,v $
 * Revision 2.4  91/05/14  16:32:45  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:21:42  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:45:16  mrt]
 * 
 * Revision 2.2  90/06/02  14:49:59  rpd
 * 	Created for new IPC.
 * 	[90/03/26  20:55:26  rpd]
 * 
 */
/*
 *	File:	ipc/ipc_init.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Declarations of functions to initialize the IPC system.
 */

#ifndef	_IPC_IPC_INIT_H_
#define _IPC_IPC_INIT_H_

extern int ipc_space_max;
extern int ipc_tree_entry_max;
extern int ipc_port_max;
extern int ipc_pset_max;

extern void
ipc_bootstrap();

extern void
ipc_init();

#endif	_IPC_IPC_INIT_H_
