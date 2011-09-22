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
 * $Log:	vm_inherit.h,v $
 * Revision 2.5  93/01/14  17:49:02  danner
 * 	Standardized include symbol name.
 * 	[92/06/10            pds]
 * 
 * Revision 2.4  92/03/10  16:27:13  jsb
 * 	Removed VM_INHERIT_DONATE_COPY.
 * 	[92/03/07  08:37:13  jsb]
 * 
 * Revision 2.3  91/05/14  17:02:47  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:37:31  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:22:24  mrt]
 * 
 * Revision 2.1  89/08/03  16:06:30  rwd
 * Created.
 * 
 * Revision 2.3  89/02/25  18:42:18  gm0w
 * 	Changes for cleanup.
 * 
 * 16-Sep-85  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Created.
 *
 */
/*
 *	File:	mach/vm_inherit.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Virtual memory map inheritance definitions.
 *
 */

#ifndef	_MACH_VM_INHERIT_H_
#define	_MACH_VM_INHERIT_H_

/*
 *	Types defined:
 *
 *	vm_inherit_t	inheritance codes.
 */

typedef int		vm_inherit_t;	/* might want to change this */

/*
 *	Enumeration of valid values for vm_inherit_t.
 */

#define	VM_INHERIT_SHARE	((vm_inherit_t) 0)	/* share with child */
#define	VM_INHERIT_COPY		((vm_inherit_t) 1)	/* copy into child */
#define VM_INHERIT_NONE		((vm_inherit_t) 2)	/* absent from child */

#define VM_INHERIT_DEFAULT	VM_INHERIT_COPY

#endif	/* _MACH_VM_INHERIT_H_ */
