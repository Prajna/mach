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
 * $Log:	msg_type.h,v $
 * Revision 2.4  93/01/14  17:46:05  danner
 * 	Standardized include symbol name.
 * 	[92/06/10            pds]
 * 
 * Revision 2.3  91/05/14  16:58:02  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:35:10  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:17:32  mrt]
 * 
 * Revision 2.1  89/08/03  16:03:38  rwd
 * Created.
 * 
 * Revision 2.3  89/02/25  18:39:26  gm0w
 * 	Changes for cleanup.
 * 
 *  4-Mar-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added MSG_TYPE_RPC.
 *
 *  22-Dec-86 Mary Thompson
 *	defined MSG_TYPE_CAMELOT, and MSG_TYPE_ENCRYPTED
 *
 */
/*
 *    This file defines user msg types that may be ored into
 *    the msg_type field in a msg header. Values 0-5 are reserved
 *    for use by the kernel and are defined in message.h. 
 *
 */

#ifndef	_MACH_MSG_TYPE_H_
#define	_MACH_MSG_TYPE_H_

#define MSG_TYPE_CAMELOT	(1 << 6)
#define MSG_TYPE_ENCRYPTED	(1 << 7)
#define	MSG_TYPE_RPC		(1 << 8)	/* Reply expected */

#include <mach/message.h>

#endif	/* _MACH_MSG_TYPE_H_ */
