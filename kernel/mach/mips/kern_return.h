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
 * Revision 2.5  93/01/14  17:45:28  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 
 * Revision 2.4  91/05/14  16:57:10  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:34:40  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:12:34  mrt]
 * 
 * Revision 2.2  89/11/29  14:09:44  af
 * 	Copied for pure kernel.
 * 	[89/10/28  09:56:38  af]
 * 
 * Revision 2.1  89/05/30  16:55:45  rvb
 * Created.
 * 
 *  3-Jan-89  Alessandro Forin (af) at Carnegie-Mellon University
 *	Copied for Mips.
 *
 *  3-Mar-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Allow inclusion in assembler input.
 *
 * 14-Oct-85  Michael Wayne Young (mwyoung) at Carnegie-Mellon University
 *	Created.
 */

/*
 *	File:	machine/kern_error.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1985
 *
 *	Machine-dependent kernel return definitions.
 *
 */

#ifndef	_MACH_MIPS_KERN_RETURN_H_
#define	_MACH_MIPS_KERN_RETURN_H_

#ifndef	ASSEMBLER
typedef	int		kern_return_t;
#endif	/* ASSEMBLER */

#endif	/* _MACH_MIPS_KERN_RETURN_H_ */
