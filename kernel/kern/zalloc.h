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
 * $Log:	zalloc.h,v $
 * Revision 2.9  91/05/18  14:34:57  rpd
 * 	Removed ZALLOC, ZGET, ZFREE.
 * 	Moved ADD_TO_ZONE, REMOVE_FROM_ZONE to kern/zalloc.c.
 * 	Moved extraneous zone GC declarations to kern/zalloc.c.
 * 	[91/03/31            rpd]
 * 	Added zdata, zdata_size.
 * 	[91/03/22            rpd]
 * 
 * Revision 2.8  91/05/14  16:50:52  mrt
 * 	Correcting copyright
 * 
 * Revision 2.7  91/02/05  17:31:33  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:22:11  mrt]
 * 
 * Revision 2.6  91/01/08  15:18:37  rpd
 * 	Removed COLLECT_ZONE_GARBAGE.  Added consider_zone_gc.
 * 	[91/01/03            rpd]
 * 
 * Revision 2.5  90/12/20  16:39:32  jeffreyh
 * 	Merged in changes done by jvs@osf from OSF/1 
 * 	[90/12/10            jeffreyh]
 * 
 * Revision 2.4  90/06/02  14:57:37  rpd
 * 	Use decl_simple_lock_data for the zone structure lock.
 * 	[90/04/23            rpd]
 * 
 * Revision 2.3  89/11/29  14:09:29  af
 * 	Now we know that collectible zones worked on mips all the time.
 * 	[89/11/16  15:05:58  af]
 * 
 * 	Made obvious that mips cannot collect zone garbage for now.
 * 	[89/10/28            af]
 * 
 * Revision 2.2  89/08/11  17:56:27  rwd
 * 	Added collectible zones.  Collectible zones allow storage to be
 * 	returned to system via kmem_free when pages are no longer used.
 * 	This option should only be used when zone memory is virtual
 * 	(rather than physical as in a MIPS architecture).
 * 	[89/07/22            rfr]
 * 
 * Revision 2.8  89/05/11  14:41:36  gm0w
 * 	Added next_zone field, to link all zones onto a list.
 * 	[89/05/08  21:35:14  rpd]
 * 
 * Revision 2.7  89/03/09  20:17:58  rpd
 * 	More cleanup.
 * 
 * Revision 2.6  89/02/25  18:11:22  gm0w
 * 	Kernel code cleanup.
 * 	Put macros under #indef KERNEL.
 * 	[89/02/15            mrt]
 * 
 * Revision 2.5  89/02/07  01:06:22  mwyoung
 * Relocated from sys/zalloc.h
 * 
 * Revision 2.4  89/01/15  16:36:01  rpd
 * 	Use decl_simple_lock_data.
 * 	[89/01/15  15:20:28  rpd]
 * 
 * Revision 2.3  88/12/19  02:52:11  mwyoung
 * 	Use MACRO_BEGIN and MACRO_END.  This corrects a problem
 * 	in the ZGET macro under lint.
 * 	[88/12/09            mwyoung]
 * 
 * Revision 2.2  88/08/24  02:56:21  mwyoung
 * 	Adjusted include file references.
 * 	[88/08/17  02:30:22  mwyoung]
 *
 *  8-Jan-88  Rick Rashid (rfr) at Carnegie-Mellon University
 *	Pageable zone lock added. NOTE ZALLOC and ZFREE macros assume
 *	non-pageable zones.
 *
 * 30-Sep-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Make ZALLOC, ZFREE not macros for lint.
 *
 * 12-Sep-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Modified to use a list instead of a queue - no need for expense
 *	of queue.  Also removed warning about assembly language hacks as
 *	there are none left that I know of.
 *
 *  1-Sep-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added zchange() declaration; added sleepable, exhaustible flags.
 *
 * 18-Apr-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Added ZALLOC and ZGET macros, note that the arguments are
 *	different that zalloc and zget due to language restrictions.
 *	For consistency, also made a ZFREE macro, zfree is now always
 *	a procedure call.
 *
 * 15-Apr-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Added warning about implementations that inline expand zalloc
 *	and zget.
 *
 * 23-Mar-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Simplified zfree macro.
 *
 *  9-Mar-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Make zfree a macro: a big win on a register-based machine
 *	with expensive procedure call; smaller change elsewhere.
 *
 *  3-Mar-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Reduce include of "../h/types.h" to "../mach/machine/vm_types.h".
 *
 * 12-Feb-87  Robert Sansom (rds) at Carnegie Mellon University
 *	Added zget.
 *
 * 12-Jan-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Eliminated use of the interlocked queue package.
 *
 *  9-Jun-85  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Created.
 */
/*
 *	File:	zalloc.h
 *	Author:	Avadis Tevanian, Jr.
 *	Date:	 1985
 *
 */

#ifndef	_KERN_ZALLOC_H_
#define _KERN_ZALLOC_H_

#include <mach/machine/vm_types.h>
#include <kern/lock.h>
#include <kern/queue.h>

/*
 *	A zone is a collection of fixed size blocks for which there
 *	is fast allocation/deallocation access.  Kernel routines can
 *	use zones to manage data structures dynamically, creating a zone
 *	for each type of data structure to be managed.
 *
 */

typedef struct zone {
	decl_simple_lock_data(,lock)	/* generic lock */
	int		count;		/* Number of elements used now */
	vm_offset_t	free_elements;
	vm_size_t	cur_size;	/* current memory utilization */
	vm_size_t	max_size;	/* how large can this zone grow */
	vm_size_t	elem_size;	/* size of an element */
	vm_size_t	alloc_size;	/* size used for more memory */
	boolean_t	doing_alloc;	/* is zone expanding now? */
	char		*zone_name;	/* a name for the zone */
	unsigned int
	/* boolean_t */	pageable :1,	/* zone pageable? */
	/* boolean_t */	sleepable :1,	/* sleep if empty? */
	/* boolean_t */ exhaustible :1,	/* merely return if empty? */
	/* boolean_t */	collectable :1,	/* garbage collect empty pages */
	/* boolean_t */	expandable :1;	/* expand zone (with message)? */
	lock_data_t	complex_lock;	/* Lock for pageable zones */
	struct zone *	next_zone;	/* Link for all-zones list */
} *zone_t;

#define		ZONE_NULL	((zone_t) 0)

extern void		zone_gc();
extern void		consider_zone_gc();

extern vm_offset_t	zalloc();
extern vm_offset_t	zget();
extern zone_t		zinit();
extern void		zfree();
extern void		zchange();
extern void		zcollectable();

extern vm_offset_t	zdata;
extern vm_offset_t	zdata_size;

extern void		zcram();
extern void		zone_bootstrap();
extern void		zone_init();

#endif	_KERN_ZALLOC_H_
