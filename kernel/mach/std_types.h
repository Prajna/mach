/* 
 * Mach Operating System
 * Copyright (c) 1992,1991,1990,1989,1988 Carnegie Mellon University
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
 * $Log:	std_types.h,v $
 * Revision 2.5  93/01/14  17:46:53  danner
 * 	Added definition of vm_address_t from mach_types.h.
 * 	[92/12/14            pds]
 * 	Standardized include symbol name.
 * 	[92/06/10            pds]
 * 
 * Revision 2.4  91/06/25  10:30:50  rpd
 * 	Added ipc/ipc_port.h inside the kernel.
 * 	[91/05/27            rpd]
 * 
 * Revision 2.3  91/05/14  16:59:01  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:35:39  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:20:57  mrt]
 * 
 * Revision 2.1  89/08/03  16:04:44  rwd
 * Created.
 * 
 * Revision 2.3  89/02/25  18:40:23  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.2  89/01/15  16:31:59  rpd
 * 	Moved from kern/ to mach/.
 * 	[89/01/15  14:34:14  rpd]
 * 
 * Revision 2.2  89/01/12  07:59:07  rpd
 * 	Created.
 * 	[89/01/12  04:15:40  rpd]
 * 
 */
/*
 *	Mach standard external interface type definitions.
 *
 */

#ifndef	_MACH_STD_TYPES_H_
#define	_MACH_STD_TYPES_H_

#define	EXPORT_BOOLEAN

#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/port.h>
#include <mach/machine/vm_types.h>

typedef	vm_offset_t	pointer_t;
typedef	vm_offset_t	vm_address_t;

#ifdef	KERNEL
#include <ipc/ipc_port.h>
#endif	/* KERNEL */

#endif	/* _MACH_STD_TYPES_H_ */
