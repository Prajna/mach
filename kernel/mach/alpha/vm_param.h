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
 * $Log:	vm_param.h,v $
 * Revision 2.2  93/01/14  17:41:15  danner
 * 	Created, partly empty.
 * 	[91/12/29            af]
 * 
 */

/*
 *	File:	alpha/vm_param.h
 *	Author:	Alessandro Forin
 *	Date:	12/91
 *
 *	ALPHA machine dependent virtual memory parameters.
 *	Most declarations are preceeded by ALPHA_ (or alpha_)
 *	because only Alpha specific code should be using
 *	them.
 *
 */

#ifndef	_MACH_ALPHA_VM_PARAM_H_
#define _MACH_ALPHA_VM_PARAM_H_

#define BYTE_SIZE	8	/* byte size in bits */

#define ALPHA_PGBYTES	8192	/* bytes per alpha (min) phys page */
#define ALPHA_PGSHIFT	13	/* number of bits to shift for pages */

/*
 *	Convert bytes to pages and convert pages to bytes.
 *	No rounding is used.
 */

#define alpha_btop(x)		(((vm_offset_t)(x)) >> ALPHA_PGSHIFT)
#define alpha_ptob(x)		(((vm_offset_t)(x)) << ALPHA_PGSHIFT)

/*
 *	Round off or truncate to the nearest page.  These will work
 *	for either addresses or counts.  (i.e. 1 byte rounds to 1 page
 *	bytes.
 */

#define alpha_round_page(x)	((((vm_offset_t)(x)) + ALPHA_PGBYTES - 1) & \
					~(ALPHA_PGBYTES-1))
#define alpha_trunc_page(x)	(((vm_offset_t)(x)) & ~(ALPHA_PGBYTES-1))

/*
 * User level addressability
 *
 * The kernel must be mapped in the user's virtual
 * space, where to is completely arbitrary. Since
 * the virtual address range is subject to change
 * with implementations we cannot specify once and
 * forall where we place it.
 * [See alpha/alpha_cpu.h for details]
 */
#define VM_MIN_ADDRESS	((vm_offset_t) 0x0)
#define VM_MAX_ADDRESS	((vm_offset_t) 0x000003fe00000000)

/*
 * The kernel's virtual address range is a bit arbitrary
 */
#define VM_MIN_KERNEL_ADDRESS	VM_MAX_ADDRESS
#define VM_MAX_KERNEL_ADDRESS	((vm_offset_t) 0x0000040000000000)

#define KERNEL_STACK_SIZE	ALPHA_PGBYTES

/*
 *	Conversion between ALPHA pages and VM pages
 */

#define trunc_alpha_to_vm(p)	(atop(trunc_page(alpha_ptob(p))))
#define round_alpha_to_vm(p)	(atop(round_page(alpha_ptob(p))))
#define vm_to_alpha(p)		(alpha_btop(ptoa(p)))


#endif	_MACH_ALPHA_VM_PARAM_H_
