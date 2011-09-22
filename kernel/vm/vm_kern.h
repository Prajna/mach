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
 * $Log:	vm_kern.h,v $
 * Revision 2.10  93/08/10  15:12:46  mrt
 * 	Included support for projected buffers.
 * 	[93/02/16  09:30:12  jcb]
 * 
 * Revision 2.9  91/08/28  11:18:05  jsb
 * 	Delete io_wire, io_unwire.
 * 	[91/08/06  17:18:36  dlb]
 * 
 * 	Add declarations for kmem_io_map_{copyout,deallocate}.
 * 	[91/08/05  17:46:55  dlb]
 * 
 * Revision 2.8  91/05/18  14:40:39  rpd
 * 	Added kmem_alloc_aligned.
 * 	[91/05/02            rpd]
 * 
 * Revision 2.7  91/05/14  17:49:27  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/03/16  15:05:31  rpd
 * 	Added kmem_realloc.  Changed kmem_alloc and friends
 * 	to return a kern_return_t.
 * 	[91/03/03            rpd]
 * 
 * Revision 2.5  91/02/05  17:58:31  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:32:28  mrt]
 * 
 * Revision 2.4  90/06/02  15:10:51  rpd
 * 	Added ipc_kernel_map.
 * 	[90/03/26  23:13:05  rpd]
 * 
 * Revision 2.3  90/02/22  20:05:46  dbg
 * 	Remove vm_move(), kmem_alloc_wait(), kmem_free_wakeup().
 * 	[90/01/25            dbg]
 * 
 * Revision 2.2  90/01/11  11:47:53  dbg
 * 	Remove kmem_mb_alloc, mb_map.  Add io_wire and io_unwire.
 * 	[89/12/11            dbg]
 * 
 * 	Changes for MACH_KERNEL:
 * 	. Added kmem_alloc_wired.
 * 	. Removed non-MACH header files, user_pt_map, vm_kern_zero_page.
 * 	[89/04/28            dbg]
 * 
 * Revision 2.1  89/08/03  16:45:13  rwd
 * Created.
 * 
 * Revision 2.8  89/04/18  21:25:51  mwyoung
 * 	No relevant history.
 * 	[89/04/18            mwyoung]
 * 
 */
/*
 *	File:	vm/vm_kern.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1985
 *
 *	Kernel memory management definitions.
 */

#ifndef	_VM_VM_KERN_H_
#define _VM_VM_KERN_H_

#include <mach/kern_return.h>
#include <vm/vm_map.h>

extern kern_return_t    projected_buffer_allocate();
extern kern_return_t    projected_buffer_deallocate();
extern kern_return_t    projected_buffer_map();
extern kern_return_t    projected_buffer_collect();

extern void		kmem_init();

extern kern_return_t	kmem_alloc();
extern kern_return_t	kmem_alloc_pageable();
extern kern_return_t	kmem_alloc_wired();
extern kern_return_t	kmem_alloc_aligned();
extern kern_return_t	kmem_realloc();
extern void		kmem_free();

extern vm_map_t		kmem_suballoc();

extern kern_return_t	kmem_io_map_copyout();
extern void		kmem_io_map_deallocate();

extern vm_map_t	kernel_map;
extern vm_map_t	kernel_pageable_map;
extern vm_map_t ipc_kernel_map;

#endif	_VM_VM_KERN_H_
