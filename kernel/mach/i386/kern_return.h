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
 * $Log:	kern_return.h,v $
 * Revision 2.5  93/01/14  17:41:59  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 
 * Revision 2.4  91/05/14  16:52:15  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:32:12  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:09:54  mrt]
 * 
 * Revision 2.2  90/05/03  15:47:51  dbg
 * 	First checkin.
 * 
 * Revision 1.3  89/03/09  20:19:48  rpd
 * 	More cleanup.
 * 
 * Revision 1.2  89/02/26  13:00:54  gm0w
 * 	Changes for cleanup.
 * 
 *  3-Mar-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Allow inclusion in assembler input.
 *
 * 14-Oct-85  Michael Wayne Young (mwyoung) at Carnegie-Mellon University
 *	Created.
 */

/*
 *	File:	kern_return.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1985
 *
 *	Machine-dependent kernel return definitions.
 */

#ifndef	_MACH_I386_KERN_RETURN_H_
#define _MACH_I386_KERN_RETURN_H_

#ifndef	ASSEMBLER
typedef	int		kern_return_t;
#endif	/* ASSEMBLER */
#endif	/* _MACH_I386_KERN_RETURN_H_ */
