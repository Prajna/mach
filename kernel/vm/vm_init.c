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
 * $Log:	vm_init.c,v $
 * Revision 2.10  92/08/03  18:00:35  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.9  92/05/21  17:25:56  jfriedl
 * 	tried prototypes.
 * 	[92/05/20            jfriedl]
 * 
 * Revision 2.8  92/01/14  16:47:54  rpd
 * 	Split vm_mem_init into vm_mem_bootstrap and vm_mem_init.
 * 	[91/12/29            rpd]
 * 
 * Revision 2.7  91/05/18  14:40:21  rpd
 * 	Replaced vm_page_startup with vm_page_bootstrap.
 * 	[91/04/10            rpd]
 * 
 * Revision 2.6  91/05/14  17:49:05  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/03/16  15:05:10  rpd
 * 	Added vm_fault_init.
 * 	[91/02/16            rpd]
 * 
 * Revision 2.4  91/02/05  17:58:17  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:32:11  mrt]
 * 
 * Revision 2.3  90/02/22  20:05:35  dbg
 * 	Reduce lint.
 * 	[90/01/29            dbg]
 * 
 * Revision 2.2  89/09/08  11:28:15  dbg
 * 	Initialize kalloc package here.
 * 	[89/09/06            dbg]
 * 
 * 	Removed non-XP code.
 * 	[89/04/28            dbg]
 * 
 * Revision 2.10  89/04/22  15:35:20  gm0w
 * 	Removed MACH_VFS code.
 * 	[89/04/14            gm0w]
 * 
 * Revision 2.9  89/04/18  21:25:30  mwyoung
 * 	Recent history:
 * 	 	Add memory_manager_default_init(), vm_page_module_init().
 * 
 * 	Older history has been integrated into the documentation for
 * 	this module.
 * 	[89/04/18            mwyoung]
 * 
 */
/*
 *	File:	vm/vm_init.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1985
 *
 *	Initialize the Virtual Memory subsystem.
 */

#include <mach/machine/vm_types.h>
#include <kern/zalloc.h>
#include <kern/kalloc.h>
#include <vm/vm_object.h>
#include <vm/vm_map.h>
#include <vm/vm_page.h>
#include <vm/vm_kern.h>
#include <vm/memory_object.h>



/*
 *	vm_mem_bootstrap initializes the virtual memory system.
 *	This is done only by the first cpu up.
 */

void vm_mem_bootstrap()
{
	vm_offset_t	start, end;

	/*
	 *	Initializes resident memory structures.
	 *	From here on, all physical memory is accounted for,
	 *	and we use only virtual addresses.
	 */

	vm_page_bootstrap(&start, &end);

	/*
	 *	Initialize other VM packages
	 */

	zone_bootstrap();
	vm_object_bootstrap();
	vm_map_init();
	kmem_init(start, end);
	pmap_init();
	zone_init();
	kalloc_init();
	vm_fault_init();
	vm_page_module_init();
	memory_manager_default_init();
}

void vm_mem_init()
{
	vm_object_init();
}
