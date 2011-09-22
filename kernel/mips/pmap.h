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
 * $Log:	pmap.h,v $
 * Revision 2.14  92/05/22  15:49:37  jfriedl
 * 	Weird devices, weird fixes.
 * 	[92/05/19            af]
 * 
 * Revision 2.13  92/03/31  15:18:25  rpd
 * 	Added a pmap_range structure.
 * 	[92/03/25            rpd]
 * 
 * Revision 2.12  91/05/18  14:37:02  rpd
 * 	Removed pmap_update.
 * 	[91/04/12            rpd]
 * 	Added MACHINE_PAGES.
 * 	[91/03/23            rpd]
 * 
 * Revision 2.11  91/05/14  17:37:22  mrt
 * 	Correcting copyright
 * 
 * Revision 2.10  91/03/16  14:57:20  rpd
 * 	Removed pcache support.
 * 	[91/01/10            rpd]
 * 
 * Revision 2.9  91/02/14  14:36:26  mrt
 * 	Made sure PTETOPHYS() does not get optimized away.
 * 	[91/01/10            af]
 * 
 * Revision 2.8  91/02/05  17:50:43  mrt
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:28:42  mrt]
 * 
 * Revision 2.7  90/10/12  12:38:37  rpd
 * 	For OSF pmap fixes:
 * 	Added PTES_PER_PAGE, NPCACHE definitions.
 * 	Parenthesized pmap_pte, pmap_root_pte macros.
 * 	Added pmap_pte_page_exists macro.
 * 	[90/10/10  17:07:54  rpd]
 * 
 * Revision 2.6  90/09/09  14:33:45  rpd
 * 	Use decl_simple_lock_data.
 * 	[90/08/30            rpd]
 * 
 * Revision 2.5  90/08/07  22:30:00  rpd
 * 	Pmap pcache.
 * 	[90/08/07  15:19:13  af]
 * 
 * Revision 2.3.3.1  90/05/30  16:57:13  af
 * 	Added pmap pcache.
 * 
 * Revision 2.4  90/06/02  15:03:28  rpd
 * 	Changed PMAP_DEACTIVATE to be null.  Added null PMAP_CONTEXT.
 * 	[90/03/26  22:54:47  rpd]
 * 
 * Revision 2.3  90/01/22  23:08:08  af
 * 	Added machine attributes pointer to pmap structure.
 * 	[90/01/20  16:43:35  af]
 * 
 * 	Removed useless wired bit from ptes.
 * 	Removed nullification of defuncted pmap_redzone.
 * 	Turned pmap_is_referenced() back into a function,
 * 	as we have now (optional) reference bits.
 * 	[89/12/09  10:56:56  af]
 * 
 * Revision 2.1.1.1  89/10/29  15:17:22  af
 * 	Moved over to pure kernel.
 * 
 * Revision 2.6  89/09/05  20:42:21  jsb
 * 	Added pmap_phys_to_frame definition.
 * 	[89/09/05  18:45:08  jsb]
 * 
 * Revision 2.5  89/08/28  22:39:22  af
 * 	[89/08/28  17:21:44  af]
 * 
 *	Added overflow list to ptepage cache, shrinked size back
 *	to initial small size. Cleanups.
 * 	[89/08/20            af]
 * 
 * Revision 2.4  89/08/08  21:49:30  jsb
 * 	Expanded the size of the ptepage cache, more cleanups.
 * 	[89/08/02            af]
 * 
 * Revision 2.3  89/07/14  15:28:30  rvb
 * 	Small cleanup.
 * 	[89/07/14            rvb]
 * 
 * Revision 2.2  89/05/31  12:31:51  rvb
 * 	BYTE_MSF and friends vs MIPSEL and friends. [af]
 * 
 * Revision 2.1  89/05/30  12:55:59  rvb
 * Rcs'ed.
 * 
 * 30-Dec-88  Alessandro Forin (af) at Carnegie-Mellon University
 *	Started, from Vax template.
 *
 */
/*
 *	File:	pmap.h
 *	Author:	Alessandro Forin
 *	Date:	1988
 *
 *	MIPS version machine-dependent structures for 
 *	the physical map module.
 *
 */

#ifndef	_PMAP_MACHINE_
#define	_PMAP_MACHINE_	1

#ifndef	ASSEMBLER
#include <mach/boolean.h>
#include <kern/zalloc.h>
#include <kern/lock.h>
#include <mach/machine/vm_param.h>
#include <mach/vm_statistics.h>

/*
 *	MIPS Page Table Entry.
 */

struct pt_entry {
#if	BYTE_MSF
unsigned int	pg_pfnum:20,		/* HW: physical page frame number */
		pg_n:1,			/* HW: non-cacheable bit */
		pg_m:1,			/* HW: modified (dirty) bit */
		pg_v:1,			/* HW: valid bit */
		pg_g:1,			/* HW: ignore pid bit */
		pg_SW:5,		/* SW: unused */
		pg_prot:3;		/* SW: Mach protection */
#else	BYTE_MSF
unsigned int	pg_prot:3,		/* SW: Mach protection */
		pg_SW:5,		/* SW: unused */
		pg_g:1,			/* HW: ignore pid bit */
		pg_v:1,			/* HW: valid bit */
		pg_m:1,			/* HW: modified (dirty) bit */
		pg_n:1,			/* HW: non-cacheable bit */
		pg_pfnum:20;		/* HW: physical page frame number */
#endif	BYTE_MSF
};

typedef struct pt_entry	pt_entry_t;
#define	PT_ENTRY_NULL	((pt_entry_t *) 0)

typedef	unsigned tlbpid_t;	/* range 0..63 */

#define PTES_PER_PAGE	(MIPS_PGBYTES / sizeof(pt_entry_t))

#endif	!ASSEMBLER

#define VA_PAGEOFF	12
#define	VA_PAGEMASK	0xfffff000
#define	VA_OFFMASK	0x00000fff
#define	PG_N		0x00000800
#define	PG_M		0x00000400
#define	PG_V		0x00000200
#define	PG_G		0x00000100
#define	PG_PROT		0x00000007
#define	PG_PROTOFF	0

/*
 * Back and forth between ptes and physical addresses
 */
#define PHYSTOPTE(addr)		((unsigned)(addr) & VA_PAGEMASK)
#define PTETOPHYS(ppte)		((*(volatile int *)ppte) & VA_PAGEMASK)

/*
 * Mips has no hardware encoding of page protections
 */
#define mips_protection(pmap, prot)	(prot)

#ifndef	ASSEMBLER
/*
 *	Pmap proper
 */
struct pmap {
	int		pid;		/* TLBPID when in use 		*/
	vm_offset_t	ptebase;	/* Base of pte array 		*/
	int		ref_count;	/* Reference count.		*/
	decl_simple_lock_data(,	lock)	/* Lock on map.			*/
#define	PMAP_MIN_PTEPAGES	5	/* Average no. of ptepages	*/
	vm_offset_t	ptepages[PMAP_MIN_PTEPAGES];
					/* Physical pages for ptes 	*/
	vm_offset_t	*ovf_ptepages;	/* more of the above		*/
	int		ptepages_count;	/* How many we're using		*/
	struct pmap_range *notcached;	/* Uncached (PG_N) addresses	*/
	int		(*hacking)();	/* horrible things needed	*/
	struct pmap_statistics	stats;	/* Map statistics.		*/
};

typedef struct pmap_range {
	struct pmap_range	*next;
	vm_offset_t		start;
	vm_offset_t		end;
} *pmap_range_t;

typedef struct pmap	*pmap_t;
#define	PMAP_NULL	((pmap_t) 0)


/*
 *	Macros
 */

/*
 *	Given an offset and a map, return the virtual address of the
 *	pte that maps the address.
 */
#define	pmap_pte(map,addr) 						\
	((pt_entry_t *) ((map)->ptebase +				\
			((((unsigned)addr) >> MIPS_PGSHIFT) << 2)))

#define pmap_root_pte(addr)						\
	((pt_entry_t *) (root_kptes +					\
		      ((((unsigned)(addr) - KPTEADDR) >> MIPS_PGSHIFT) << 2)))

/*
 *	Check whether the pte page for a pte exists.  This is used to
 *	avoid faulting in pte pages unnecessarily.  Note that we may
 *	take a kernel pte page fault when checking a user pte.
 */
#define pmap_pte_page_exists(map, pte) 					\
	(((map) == kernel_pmap) ? (pmap_root_pte(pte)->pg_v) :	\
	 			  (pmap_pte(kernel_pmap, (pte))->pg_v))

#ifdef	KERNEL

#define PMAP_ACTIVATE(pmap, th, my_cpu)			\
	{ if (pmap->pid < 0) assign_tlbpid(pmap);	\
	  tlb_set_context(pmap);			\
	}

/*
 *	This is a sleazy definition, but given the places
 *	where PMAP_DEACTIVATE it actually used, it works.
 *	And it makes context switches faster.
 */

#define PMAP_DEACTIVATE(pmap, thread, cpu)

#define PMAP_CONTEXT(pmap, new_thread)


#define	pmap_resident_count(pmap)	((pmap)->stats.resident_count)
#define	pmap_phys_address(frame)	((vm_offset_t) (mips_ptob(frame)))
#define pmap_phys_to_frame(phys)	((int) (mips_btop(phys)))
#define pmap_copy(dst,src,from,len,to)
#define pmap_kernel()			kernel_pmap
#define	pmap_pageable(map,start,end,pageable)

/*
 *	We want to implement pmap_steal_memory and pmap_startup.
 */

#define	MACHINE_PAGES

/*
 *	Data structures this module exports
 */
pmap_t	kernel_pmap;			/* pointer to the kernel pmap	*/
pmap_t	active_pmap;			/* pmap for the current thread  */
vm_offset_t root_kptes;			/* ptes to back up the kernel's pte */

#endif	KERNEL
#endif	!ASSEMBLER

#endif	_PMAP_MACHINE_
