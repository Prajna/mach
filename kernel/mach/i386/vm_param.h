/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
 * $Log:	vm_param.h,v $
 * Revision 2.7  93/01/14  17:42:18  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 
 * Revision 2.6  92/01/03  20:20:33  dbg
 * 	Drop back to 1-page kernel stacks, since emulation_vector calls
 * 	now pass data out-of-line.
 * 	[92/01/03            dbg]
 * 
 * Revision 2.5  91/11/19  08:08:35  rvb
 * 	NORMA needs a larger stack so we do it for everyone,
 * 	since stack space usage does not matter anymore.
 * 
 * Revision 2.4  91/05/14  16:52:50  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:32:30  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:10:30  mrt]
 * 
 * Revision 2.2  90/05/03  15:48:20  dbg
 * 	Move page-table definitions into i386/pmap.h.
 * 	[90/04/05            dbg]
 * 
 * 	Remove misleading comment about kernel stack size.
 * 	[90/02/05            dbg]
 * 
 * Revision 1.3  89/03/09  20:20:06  rpd
 * 	More cleanup.
 * 
 * Revision 1.2  89/02/26  13:01:13  gm0w
 * 	Changes for cleanup.
 * 
 * 31-Dec-88  Robert Baron (rvb) at Carnegie-Mellon University
 *	Derived from MACH2.0 vax release.
 *
 * 16-Jan-87  David Golub (dbg) at Carnegie-Mellon University
 *	Made vax_ptob return 'unsigned' instead of caddr_t.
 *
 */

/*
 *	File:	vm_param.h
 *	Author:	Avadis Tevanian, Jr.
 *	Date:	1985
 *
 *	I386 machine dependent virtual memory parameters.
 *	Most of the declarations are preceeded by I386_ (or i386_)
 *	which is OK because only I386 specific code will be using
 *	them.
 */

#ifndef	_MACH_I386_VM_PARAM_H_
#define _MACH_I386_VM_PARAM_H_

#define BYTE_SIZE	8	/* byte size in bits */

#define I386_PGBYTES	4096	/* bytes per 80386 page */
#define I386_PGSHIFT	12	/* number of bits to shift for pages */

/*
 *	Convert bytes to pages and convert pages to bytes.
 *	No rounding is used.
 */

#define i386_btop(x)		(((unsigned)(x)) >> I386_PGSHIFT)
#define i386_ptob(x)		(((unsigned)(x)) << I386_PGSHIFT)

/*
 *	Round off or truncate to the nearest page.  These will work
 *	for either addresses or counts.  (i.e. 1 byte rounds to 1 page
 *	bytes.
 */

#define i386_round_page(x)	((((unsigned)(x)) + I386_PGBYTES - 1) & \
					~(I386_PGBYTES-1))
#define i386_trunc_page(x)	(((unsigned)(x)) & ~(I386_PGBYTES-1))

#define VM_MIN_ADDRESS		((vm_offset_t) 0)
#define VM_MAX_ADDRESS		((vm_offset_t) 0xc0000000)

#define VM_MIN_KERNEL_ADDRESS	((vm_offset_t) 0xc0000000)
#define VM_MAX_KERNEL_ADDRESS	((vm_offset_t) 0xffffffff)

#define KERNEL_STACK_SIZE	(1*I386_PGBYTES)
#define INTSTACK_SIZE		(1*I386_PGBYTES)
						/* interrupt stack size */

/*
 *	Conversion between 80386 pages and VM pages
 */

#define trunc_i386_to_vm(p)	(atop(trunc_page(i386_ptob(p))))
#define round_i386_to_vm(p)	(atop(round_page(i386_ptob(p))))
#define vm_to_i386(p)		(i386_btop(ptoa(p)))

/*
 *	Physical memory is mapped 1-1 with virtual memory starting
 *	at VM_MIN_KERNEL_ADDRESS.
 */
#define phystokv(a)	((vm_offset_t)(a) + VM_MIN_KERNEL_ADDRESS)

#endif	/* _MACH_I386_VM_PARAM_H_ */
