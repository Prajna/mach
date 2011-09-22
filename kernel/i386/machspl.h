/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 * $Log:	machspl.h,v $
 * Revision 2.2  93/05/15  20:58:58  mrt
 * 	Used to be machparam.h
 * 	[93/05/15            mrt]
 * 
 * Revision 2.5  93/01/14  17:29:17  danner
 * 	Proper spl typing.
 * 	[92/11/30            af]
 * 
 * Revision 2.4  91/05/14  16:11:57  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:13:13  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:36:08  mrt]
 * 
 * Revision 2.2  90/05/03  15:34:01  dbg
 * 	Created.
 * 	[90/02/08            dbg]
 * 
 */

#ifndef	_MACHINE_MACHSPL_H_
#define	_MACHINE_MACHSPL_H_ 1

/*
 *	Machine-dependent SPL definitions.
 *
 *	SPLs are true functions on i386, defined elsewhere.
 */

typedef int	spl_t;

#endif	/* _MACHINE_MACHSPL_H_ */
