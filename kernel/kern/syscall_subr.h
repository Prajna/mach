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
 * $Log:	syscall_subr.h,v $
 * Revision 2.5  91/05/18  14:33:56  rpd
 * 	Added thread_depress_timeout.
 * 	[91/03/31            rpd]
 * 
 * Revision 2.4  91/05/14  16:47:37  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:29:40  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:18:24  mrt]
 * 
 * Revision 2.2  90/06/02  14:56:22  rpd
 * 	Created.
 * 	[90/03/26  23:52:40  rpd]
 * 
 * Revision 2.5  89/10/11  14:27:28  dlb
 * 	Add thread_switch, remove kern_timestamp.
 * 	[89/09/01  17:43:07  dlb]
 * 
 * Revision 2.4  89/10/10  10:54:34  mwyoung
 * 	Add a new call to create an RFS link, a special form of
 * 	symbolic link that may contain null characters in the target.
 * 	[89/10/01            mwyoung]
 * 
 * Revision 2.3  89/05/01  17:28:32  rpd
 * 	Removed the ctimes() declaration; it's history.
 * 
 * Revision 2.2  89/05/01  17:01:50  rpd
 * 	Created.
 * 	[89/05/01  14:00:51  rpd]
 * 
 */

#ifndef	_KERN_SYSCALL_SUBR_H_
#define _KERN_SYSCALL_SUBR_H_

extern int	swtch();
extern int	swtch_pri();
extern int	thread_switch();
extern void	thread_depress_timeout();

#endif	_KERN_SYSCALL_SUBR_H_
