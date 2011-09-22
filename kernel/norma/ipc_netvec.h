/* 
 * Mach Operating System
 * Copyright (c) 1991,1992 Carnegie Mellon University
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
 * $Log:	ipc_netvec.h,v $
 * Revision 2.2  92/03/10  16:28:00  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:50:03  jsb]
 * 
 * Revision 2.1.2.1  92/01/21  21:52:10  jsb
 * 	Added file/author/date comment.
 * 	[92/01/21  19:44:00  jsb]
 * 
 * 	First checkin. Contents moved here from norma/ipc_net.h.
 * 	[92/01/10  20:51:02  jsb]
 * 
 */ 
/*
 *	File:	norma/ipc_netvec.h
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Definition for scatter/gather vector.
 */

struct netvec {
	unsigned long	addr;
	unsigned long	size;
};
