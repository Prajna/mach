/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * Copyright (c) 1991 Sequent Computer Systems
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND SEQUENT COMPUTER SYSTEMS ALLOW FREE USE OF
 * THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * SEQUENT COMPUTER SYSTEMS DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	vm_defs.h,v $
 * Revision 2.3  91/07/31  18:04:53  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:01:36  dbg
 * 	Make KVTOPHYS use kvtophys to avoid locking kernel pmap.
 * 	[91/02/14            dbg]
 * 
 * 	Move cpu-local mappings (slic, leds, elapsed-time counter) to
 * 	level 1 page just below FPA.
 * 	[90/12/17            dbg]
 * 
 * 	Rewrote for pure Mach kernel.  Kernel is no longer mapped 1-1
 * 	with physical memory.
 * 	[90/09/27            dbg]
 * 
 */

#ifndef	_SQT_VM_DEFS_H_
#define	_SQT_VM_DEFS_H_

#include <mach/i386/vm_param.h>
#include <i386/pmap.h>

/*
 * Kernel address space layout.
 *
 * Physical memory is mapped 1-1 starting at 0xC0000000.  This includes
 * the kernel code, data, and bss, and the kernel page table directory.
 * Useful IO space is mapped to allocated virtual memory.
 * See machine/hwparam.h for description of IO space.
 *
 * User mapping consumes 3Gig of space, starting at 0.  User segment
 * registers declare full 4Gig space, to allow FPA mapping to be in the
 * last 4Meg of the top of the 4Gig space (Intel standard).
 */

#define	PDE_MAP_SIZE	(1 << PDESHIFT)
					/* amount mapped by a page directory */

#define	FPA_SPACE	(1 * PDE_MAP_SIZE)
					/* 1 L1 page for this */
#define	VA_FPA		(0 - FPA_SPACE)
					/* mapped FPA */


#define	HDW_SPACE	(2 * PDE_MAP_SIZE)
					/* slic, led, etc, sync-points */

#define	VA_SLIC		(VA_FPA - HDW_SPACE)
#define	VA_LED		(VA_SLIC + I386_PGBYTES)
#define	VA_ETC		(VA_LED  + I386_PGBYTES)

/*
 * Mapping between physical and virtual addresses.
 */
#define	PHYSTOKV(addr, type)	((type)phystokv(addr))
#define	KVTOPHYS(addr, type)	((type)kvtophys((vm_offset_t)(addr)))

#endif	/* _SQT_VM_DEFS_H_ */
