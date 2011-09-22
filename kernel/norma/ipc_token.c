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
 * $Log:	ipc_token.c,v $
 * Revision 2.4  92/03/10  16:28:21  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:50:24  jsb]
 * 
 * Revision 2.3.2.1  92/01/09  18:45:55  jsb
 * 	Purged contents.
 * 	[92/01/04  18:25:34  jsb]
 * 
 * Revision 2.3  91/12/14  14:34:56  jsb
 * 	Removed ipc_fields.h hack.
 * 
 * Revision 2.2  91/11/14  16:46:09  rpd
 * 	Created.
 * 
 */
/*
 *	File:	norma/ipc_token.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Functions for implementing token-based algorithm for maintaining
 *	distributed reference count information as required for notification
 *	mechanism and for internal garbage collection.
 */
