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
 * $Log:	vm_fault.h,v $
 * Revision 2.6  91/05/18  14:40:15  rpd
 * 	Added VM_FAULT_FICTITIOUS_SHORTAGE.
 * 	[91/03/29            rpd]
 * 
 * Revision 2.5  91/05/14  17:48:59  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/03/16  15:05:03  rpd
 * 	Added vm_fault_init.
 * 	[91/02/16            rpd]
 * 
 * Revision 2.3  91/02/05  17:58:12  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:32:04  mrt]
 * 
 * Revision 2.2  90/02/22  20:05:32  dbg
 * 	Add vm_fault_copy(), vm_fault_cleanup().  Remove
 * 	vm_fault_copy_entry().
 * 	[90/01/25            dbg]
 * 
 * Revision 2.1  89/08/03  16:44:57  rwd
 * Created.
 * 
 * Revision 2.6  89/04/18  21:25:22  mwyoung
 * 	Reset history.
 * 	[89/04/18            mwyoung]
 * 
 */
/*
 *	File:	vm/vm_fault.h
 *
 *	Page fault handling module declarations.
 */

#ifndef	_VM_VM_FAULT_H_
#define _VM_VM_FAULT_H_

#include <mach/kern_return.h>

/*
 *	Page fault handling based on vm_object only.
 */

typedef	kern_return_t	vm_fault_return_t;
#define VM_FAULT_SUCCESS		0
#define VM_FAULT_RETRY			1
#define VM_FAULT_INTERRUPTED		2
#define VM_FAULT_MEMORY_SHORTAGE 	3
#define VM_FAULT_FICTITIOUS_SHORTAGE 	4
#define VM_FAULT_MEMORY_ERROR		5

extern void vm_fault_init();
extern vm_fault_return_t vm_fault_page();

extern void		vm_fault_cleanup();
/*
 *	Page fault handling based on vm_map (or entries therein)
 */

extern kern_return_t	vm_fault();
extern void		vm_fault_wire();
extern void		vm_fault_unwire();

extern kern_return_t	vm_fault_copy();	/* Copy pages from
						 * one object to another
						 */
#endif	_VM_VM_FAULT_H_
