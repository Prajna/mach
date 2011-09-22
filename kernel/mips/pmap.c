/* 
 * Mach Operating System
 * Copyright (c) 1993-1988 Carnegie Mellon University
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
 * $Log:	pmap.c,v $
 * Revision 2.30  93/05/10  21:21:13  rvb
 * 	No more sys/types.h
 * 	[93/05/06  09:31:45  af]
 * 
 * Revision 2.29  93/01/14  17:52:28  danner
 * 	Removed thread_sleep/wakeup argument coerecion.
 * 	[93/01/14            danner]
 * 
 * Revision 2.28  92/05/22  15:49:22  jfriedl
 * 	Weird devices, weird fixes.
 * 	[92/05/19            af]
 * 	bmerge got confused and did not pick up jcb's fix to
 * 	the blooper in pmap_destroy_ranges().
 * 	[92/05/07            af]
 * 
 * Revision 2.27  92/05/05  10:47:04  danner
 * 	From jcb
 * 	[92/05/04            af]
 * 
 * Revision 2.26  92/04/03  17:42:26  rpd
 * 	Redid pmap_attributes and associated functions,
 * 	to fix bugs with multiple ranges in a single pmap.
 * 	[92/03/25            rpd]
 * 
 * Revision 2.25  92/03/05  11:37:23  rpd
 * 	Made non-cachability of I/O memory non-mandatory.
 * 	Do not account for I/O memory pages in the resident_count.
 * 	Removed some old cruft.
 * 	[92/03/04            af]
 * 
 * Revision 2.24  92/01/14  16:46:01  rpd
 * 	Removed pmap_list_resident_pages.
 * 	[91/12/31            rpd]
 * 
 * 	Fixed types in pmap_copy_page.
 * 	[91/12/28            rpd]
 * 
 * Revision 2.23  92/01/03  20:25:56  dbg
 * 	Made pmap_extract handle kernel virtual addresses in K0 or K1
 * 	space.
 * 	[91/09/19            dbg]
 * 
 * Revision 2.22  91/08/24  12:23:58  af
 * 	phys_to_k0seg() is out, kvtophys() is in.
 * 	[91/08/02  03:13:06  af]
 * 
 * Revision 2.21  91/06/19  11:56:28  rvb
 * 	#ifdef PMAX -> #ifdef DECSTATION and we include <platforms.h>
 * 	[91/06/12  14:08:54  rvb]
 * 
 * Revision 2.20  91/05/18  14:36:44  rpd
 * 	Removed pmap_update.
 * 	[91/04/12            rpd]
 * 
 * 	Added inuse_ptepages_count.
 * 	Added vm_page_fictitious_addr assertions.
 * 	[91/04/10            rpd]
 * 	Commented out pmap_lock.
 * 	[91/04/01            rpd]
 * 
 * 	Added check_simple_locks to get_free_ptepage.
 * 	[91/03/31            rpd]
 * 	Removed pmap_valid_page, arguments to pmap_bootstrap, pmap_init.
 * 	Added pmap_free_pages, pmap_steal_memory, pmap_startup.
 * 	[91/03/24            rpd]
 * 	Removed spl/splx calls.
 * 	Made compacted modify bits the default.
 * 	Fixed return_modify_bit.
 * 	[91/03/19            rpd]
 * 
 * Revision 2.19  91/05/14  17:37:05  mrt
 * 	Correcting copyright
 * 
 * Revision 2.18  91/05/13  06:06:26  af
 * 	Use the faster page copy routine.
 * 	Added a counter of in-use ptepages.
 * 	[91/05/12  15:58:13  af]
 * 
 * Revision 2.17  91/03/16  14:57:00  rpd
 * 	Updated for new kmem_alloc interface.
 * 	[91/03/03            rpd]
 * 	Removed pcache support.
 * 	[91/01/10            rpd]
 * 
 * Revision 2.16  91/02/14  14:36:08  mrt
 * 	Fixed pmap_attribute() to loop through all phys pages while
 * 	flushing the cache.  If this gets used heavily for large
 * 	ranges it will be wise to just flush the whole cache in
 * 	one shot, or keep perhaps track of which 'cache pages'
 * 	need flushing and which do not.
 * 	[91/02/12  12:33:25  af]
 * 
 * Revision 2.15  91/02/05  17:50:28  mrt
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:28:27  mrt]
 * 
 * Revision 2.13  90/12/05  20:50:04  af
 * 	Made attributes zone non-pageable.  This fixes a deadlock in
 * 	pmap_attributes_lookup() faulting with a read lock held on
 * 	the pmap system (from pmap_enter).
 * 
 * 	Reversing the meaning of "0" and "1" in pmap_reference_bits[]
 * 	saves one precious cycle in tlb_umiss.  Idea courtesy of dbg.
 * 
 * Revision 2.12  90/10/25  14:46:49  rwd
 * 	Fixed the OSF pmap_protect fixes.
 * 	[90/10/21            rpd]
 * 
 * Revision 2.11  90/10/12  12:36:29  rpd
 * 	Fixes from OSF:
 * 	Fixed pmap_pte_fault to check after allocating the pte page
 * 	whether the new page is still needed.
 * 	Fixed sparse address space support in pmap_remove_range.
 * 	Added sparse support to pmap_protect, pmap_extract, pmap_access.
 * 	Fixed pmap_protect to flush the I-cache when allowing execute access.
 * 	Fixed pmap_remove to use READ_LOCK/READ_UNLOCK.
 * 	Fixed pmap_enter to refetch ptaddr after locking.
 * 	[90/10/10  17:05:49  rpd]
 * 
 * Revision 2.10  90/09/09  14:33:34  rpd
 * 	Fixed get_free_ptepage to return the page!
 * 	[90/09/08            rpd]
 * 
 * Revision 2.9  90/08/28  19:06:31  dbg
 * 	Fixed boundary check in pmap_attribute_lookup()
 * 	[90/08/22            af]
 * 
 * Revision 2.8  90/08/07  22:21:48  rpd
 * 	Zero pages that we steal, we are no guaranteed they are clean at startup.
 * 	Take care of machines with LOTS of memory (3maxen).
 * 	Use pcache trick to cache tlb mappings for ptepages across csw.
 * 	Optimize pmap_remove for "large address spaces".
 * 	Optimize pmap_zero_page by unrolling loop.
 * 	[90/08/07  15:13:12  af]
 * 
 * 	Optimized zero_page in place, with appropriate unconditional unrolling.
 * 	[90/06/11  11:03:27  af]
 * 
 * 	Consolidated MATTR conditional.  Added pmap pcache support.
 * 	[90/05/30  16:56:50  af]
 * 
 * Revision 2.7  90/08/06  17:06:20  rpd
 * 	Fixed pmap_protect to never increase permissions and
 * 	to remove all access if read permission is not present.
 * 	[90/08/01            rpd]
 * 
 * Revision 2.6  90/06/02  15:03:13  rpd
 * 	Added dummy pmap_list_resident_pages, under MACH_VM_DEBUG.
 * 	[90/05/31            rpd]
 * 
 * 	Turned MIN_PTE_PAGES, MAX_PTE_PAGES, MORE_PTE_PAGES into variables.
 * 	Increased max_pte_pages from 13 to 15.
 * 	[90/04/26            rpd]
 * 
 * 	Purged pmap_reclaim_ptepages; now pmap_destroy always reclaims
 * 	and pmap_collect always does nothing.  Added a cap on the
 * 	size of the ptepage free list.  Now put_free_ptepage will
 * 	release extra pages to limit consumption of wired memory.
 * 	Added pmap_release_page, to really free a pte page.
 * 	[90/04/12            rpd]
 * 
 * 	Rewrote get_free_ptepage, put_free_ptepage
 * 	to make them less prone to deadlock.
 * 	[90/03/29            rpd]
 * 
 * 	Picked up locking fixes from the mainline.
 * 	Also picked up pmap_page_protect fix.
 * 	[90/03/29            rpd]
 * 	Use lock_pmap/unlock_pmap macros to avoid locking bugs.
 * 	[90/03/26  22:53:38  rpd]
 * 
 * Revision 2.5  90/02/22  20:04:31  dbg
 * 	Make pmap_protect do a pmap_remove if new protection is
 * 	VM_PROT_NONE.
 * 	[90/02/19            dbg]
 * 
 * Revision 2.4  90/01/22  23:07:57  af
 * 	Reworked machine attributes.  Fixed compacted mod bits to lock
 * 	appropriately, consolidated other conditionals.
 * 	[90/01/20  16:41:36  af]
 * 
 * 	Added ref bits and perf. monit. counters.
 * 	Removed wired bit garbage.
 * 	Changed modify bits into a bitmap, saving about 50k.
 * 	Reworked a number of functions for better speed.
 * 	Added pmap_attribute().
 * 	[89/12/09  11:04:11  af]
 * 
 * Revision 2.3  90/01/19  14:34:37  rwd
 * 	Add copy_from_phys routine
 * 	[90/01/17            rwd]
 * 
 * Revision 2.2  89/11/29  14:15:04  af
 * 	Moved over to pure kernel.
 * 	[89/10/29            af]
 * 
 * Revision 2.5  89/08/28  22:39:14  af
 * 	Added pmap_collect().
 * 	[89/08/09            af]
 * 
 * Revision 2.4  89/08/08  21:49:19  jsb
 * 	pmap_enter() should lookup the pte *before* taking the lock, or
 * 	else we deadlock (see comment).
 * 	Made sure the pvlist gets in k0seg, regardless of how vm_..alloc_fifo
 * 	is set.
 * 	[89/08/02            af]
 * 
 * Revision 2.3  89/07/14  15:28:20  rvb
 * 	Bob&I Catched THE_BIG_BUG.
 * 	Did some cleanup and tuneups.
 * 	[89/07/03            af]
 * 
 * Revision 2.2  89/05/31  12:31:24  rvb
 * 	Clean up. [af]
 * 
 * Revision 2.1  89/05/30  12:55:57  rvb
 * Rcs'ed.
 * 
 * 20-Apr-89  Alessandro Forin (af) at Carnegie-Mellon University
 *	Added pmap_verify_free() and pmap_valid_page().
 *	Modified so that non-handled physical pages can be
 *	mapped safely, e.g. for screen memory.
 *	Made pmap_enter() callable even before pmap_init()
 *	is called. I don't quite like it, but that's life.
 *
 * 12-Apr-89  Alessandro Forin (af) at Carnegie-Mellon University
 *	Reworked the case where the pmap operated upon is not the
 *	active_pmap, and related faults.  Removed the "current_pmap"
 *	hack.
 *
 * 30-Dec-88  Alessandro Forin (af) at Carnegie-Mellon University
 *	Started, keeping an eye on the Encore version.
 */
/*
 *	File:	pmap.c
 *	Author: Alessandro Forin
 *	Date:	1988
 *
 *	MIPS Physical Map management code.
 *	The original pmap code was done for the Vax by
 *	Avadis Tevanian, Jr., Michael Wayne Young
 *	which I used as a template.  Most of their comments are preserved.
 *
 */
/*
 *	Manages physical address maps.
 *
 *	In addition to hardware address maps, this
 *	module is called upon to provide software-use-only
 *	maps which may or may not be stored in the same
 *	form as hardware maps.  These pseudo-maps are
 *	used to store intermediate results from copy
 *	operations to and from address spaces.
 *
 *	Since the information managed by this module is
 *	also stored by the logical address mapping module,
 *	this module may throw away valid virtual-to-physical
 *	mappings at almost any time.  However, invalidations
 *	of virtual-to-physical mappings must be done as
 *	requested.
 *
 *	In order to cope with hardware architectures which
 *	make virtual-to-physical map invalidates expensive,
 *	this module may delay invalidate or reduced protection
 *	operations until such time as they are actually
 *	necessary.  This module is given full information as
 *	to which processors are currently using which maps,
 *	and to when physical maps must be made correct.
 */
#include <platforms.h>

#include <ref_bits.h>
#include <counters.h>

#include <mach/boolean.h>
#include <mach/vm_attributes.h>
#include <mach/vm_param.h>
#include <kern/macro_help.h>
#include <kern/lock.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>
#include <mips/mips_cpu.h>


struct pmap	kernel_pmap_store;
pmap_t		kernel_pmap;		/* kernel pmap, statically alloced */
pmap_t		active_pmap;		/* pmap for the current_thread() */
vm_map_t	pmap_submap;		/* kernel submap where we take
					   our pages from */
vm_offset_t	pmap_vlow, pmap_vhigh;	/* (virtual) limits for our needs */


vm_offset_t	root_kptes;		/* Second level kernel ptes */

struct zone	*pmap_zone;		/* Zone of pmap structures  */

vm_offset_t pmap_resident_extract();
void pmap_release_page();

#if	COUNTERS
unsigned counters[30];
#define vtp_count counters[0]		/* virt_to_pmap */
#define mod_count counters[1]		/* PMAP_SET_MODIFY */
#define mpp_count counters[2]		/* get_more_pte_pages */
#define ptf_count counters[3]		/* pmap_pte_fault */
#define pgc_count counters[4]		/* pte_page_gc */
#define bmp_count counters[5]		/* pmap_map */
#define pcr_count counters[6]		/* pmap_create */
#define pds_count counters[7]		/* pmap_destroy */
#define rrg_count counters[8]		/* pmap_remove_range */
#define ral_count counters[9]		/* pmap_remove_all */
#define cow_count counters[10]		/* pmap_copy_on_write */
#define ppt_count counters[11]		/* pmap_protect */
#define ent_count counters[12]		/* pmap_enter */
#define wir_count counters[13]		/* pmap_change_wiring */
#define ext_count counters[14]		/* pmap_extract */
#define acc_count counters[15]		/* pmap_access */
#define zer_count counters[16]		/* pmap_zero_page */
#define cop_count counters[17]		/* pmap_copy_page */
#define prt_count counters[18]		/* pmap_page_protect */
#define smd_count counters[19]		/* pmap_set_modify */
#define clr_count counters[20]		/* pmap_clear_modify */
#define imd_count counters[21]		/* pmap_is_modified */
#define crr_count counters[22]		/* pmap_clear_reference */
#define irr_count counters[23]		/* pmap_is_referenced */
#define tph_count counters[24]		/* pmap_copy_to_phys */
#define tpa_count counters[25]		/* assign_tlbpid */
#define tpf_count counters[26]		/* destroy_tlbpid */
#define ifl_count counters[27]		/* mipscache_Iflush */
#define ifn_count counters[28]		/* Iflush needed */
#define xx3_count counters[29]
#endif	/*COUNTERS*/
/*
 *	When we operate on another pmap we might fault on its
 *	ptepages.  The trap handler needs therefore to be able
 *	to get to the pmap given a ptepage address.  The following
 *	table does this reverse mapping ptepage->pmap.
 *	Note: this is the only place where it is known how many
 *	pmaps we can possible create.
 */
/*
 *	Use a simple table lookup
 */
#define	MAX_PMAPS	(KPTEADDR-VM_MAX_KERNEL_ADDRESS)/UPTESIZE
pmap_t ptepage_to_pmap[MAX_PMAPS];	/* its 447 of them! */

/*
 * Indexing is by subdivision of the allowed range for ptepages
 */
#define ptepage_to_pmap_index(_virt)					\
		(((unsigned)(_virt)-VM_MAX_KERNEL_ADDRESS)/UPTESIZE)

/*
 * Table insertion
 */
#define ptepage_to_pmap_enter(_pmap)					\
		ptepage_to_pmap[ptepage_to_pmap_index(_pmap->ptebase)]	\
			= (_pmap)
/*
 * Table lookup proper
 */
#define PTEPAGE_TO_PMAP_LOOKUP(virt)					\
		ptepage_to_pmap[ptepage_to_pmap_index(virt)]

/*
 * A function so that we do not export too much crap.
 * This is the one called by the tlb_miss() handler
 */
pmap_t virt_to_pmap(virt)
vm_offset_t virt;
{
#if	COUNTERS
	vtp_count++;
#endif	/*COUNTERS*/
	return PTEPAGE_TO_PMAP_LOOKUP(virt);
}


/*
 *	For each vm_page_t, there is a list of all currently
 *	valid virtual mappings of that page.  An entry is
 *	a pv_entry_t; the lists' heads are in the pv_table.
 *	There are analogous arrays for the modify (and reference) bits.
 */
struct pv_entry {
	struct pv_entry	*next;		/* next pv_entry */
	pmap_t		pmap;		/* pmap where mapping lies */
	vm_offset_t	va;		/* virtual address for mapping */
};

typedef struct pv_entry *pv_entry_t;
#define	PV_ENTRY_NULL	((pv_entry_t) 0)

/*
 * The pv_list proper, which is actually an array of lists
 */
pv_entry_t	pv_head_table;		/* array of entries, one per page */
zone_t		pv_list_zone;		/* zone of pv_entry structures */

#define	pa_index(pa)		(atop((pa) - vm_first_phys))
#define pa_to_pvh(pa)		(&pv_head_table[pa_index(pa)])

/*
 * The modify table is an array.  Could be compacted, but we might
 * need to put more info there sometimes.
 * Not all phys pages are accounted for.
 */
vm_offset_t	vm_first_phys;		/* range of phys pages which we can handle */
vm_offset_t	vm_last_phys;

					/* dirty bits table */
unsigned char	*pmap_modify_bits;

#define set_modify_bit(pa) 						\
MACRO_BEGIN								\
	register unsigned int ix = pa_index(pa);			\
	pmap_modify_bits[(ix >> 3)] |= (1 << (ix & 7));			\
MACRO_END

#define clear_modify_bit(pa) 						\
MACRO_BEGIN								\
	register unsigned int ix = pa_index(pa);			\
	pmap_modify_bits[(ix >> 3)] &= ~(1 << (ix & 7));		\
MACRO_END

#define return_modify_bit(pa) 						\
	register unsigned int ix = pa_index(pa);			\
	return (pmap_modify_bits[(ix >> 3)] & (1 << (ix & 7))) != 0;	\

#if	COUNTERS
#define PMAP_SET_MODIFY(p)					\
	{							\
	    mod_count++;					\
	    TRACE({printf("pmap_set_modify x%x\n", (p));})	\
	    if ((p) >= vm_first_phys && (p) < vm_last_phys)	\
		set_modify_bit((p));				\
	}
#else	COUNTERS
#define PMAP_SET_MODIFY(p)					\
	{							\
	    TRACE({printf("pmap_set_modify x%x\n", (p));})	\
	    if ((p) >= vm_first_phys && (p) < vm_last_phys)	\
		set_modify_bit((p));				\
	}
#endif	/*COUNTERS*/


#if	REF_BITS
					/* reference bits */

extern char	pmap_reference_bits[];	/* allocated in locore.s */

#define	ref_index(pa)		(mips_btop((pa)))
#define set_reference_bit(pa) \
	pmap_reference_bits[ref_index((pa))] = 0;	/* tlb_umiss knows! */

#define clear_reference_bit(pa) \
	pmap_reference_bits[ref_index((pa))] = 1;

#define return_reference_bit(pa) \
	return !pmap_reference_bits[ref_index((pa))];

#endif	/*REF_BITS*/

/*
 *	Lock and unlock macros for the pmap system.
 *
 *	This locking is incorrect, hence commented out.
 *		1) The pmap module calls zalloc while holding pmap locks.
 *		2) The pmap module calls READ_LOCK/WRITE_LOCK
 *		while our callers may hold simple locks.
 *	It is not needed on uniprocessors.
 */

#define	lock_pmap(pmap)		/* simple_lock(&(pmap)->lock) */
#define	unlock_pmap(pmap)	/* simple_unlock(&(pmap)->lock) */

lock_data_t	pmap_lock;	/* General lock for the pmap system */

#define READ_LOCK()		/* lock_read(&pmap_lock) */

#define READ_UNLOCK()		/* lock_read_done(&pmap_lock) */

#define WRITE_LOCK()		/* lock_write(&pmap_lock) */

#define WRITE_UNLOCK()		/* lock_write_done(&pmap_lock) */


static boolean_t pmap_module_is_initialized = FALSE;

int pmap_debug = 0;
/*#define TRACE(x) if (pmap_debug) x*/
#define TRACE(x)

/*
 * We know that a pte really is a long, but we have to tell the compiler
 */
typedef union {
	pt_entry_t	pte;
	unsigned long	raw;
} pte_template;


/*
 *	PTE page mgmt.
 *
 * Pages for use as page tables are managed by us, not the VM system.
 * We have reserved the virtual space above VM_MAX_KERNEL_ADDRESS
 * for this purpose, and a pool of pages gets mapped/remapped for
 * use in the various pmaps.
 * The association between a pmap and its ptebase never changes once
 * it is established, hence we cannot zfree a pmap to destroy it and
 * we must maintain our own free list.
 */
/*
 * Virtual address range for the page tables.
 * Grows down, towards VM_MAX_KERNEL_ADDRESS.
 */
vm_offset_t	 free_ptebase = KPTEADDR;
#define	next_ptebase()							\
		((free_ptebase <= VM_MAX_KERNEL_ADDRESS) ?		\
			panic("Too many pmaps") :			\
			(free_ptebase -= UPTESIZE))

/*
 * Pool of physical pages for use as page tables.
 * The problem with allocating these pages is that
 * we may need pte pages to allocate pte pages.
 * The solution is to keep a reserved pool, which may
 * may only be used if a thread is trying to allocate pte pages,
 * and either it needs a pte page to do so, or a VM-privileged
 * thread needs pte pages (presumably to make more free pages).
 */

decl_simple_lock_data(,ptepages_lock) /* protects the following variables */
vm_offset_t	free_ptepages = 0;
unsigned int	free_ptepages_count = 0;
unsigned int	inuse_ptepages_count = 0;	/* debugging */
thread_t	ptepages_thread = THREAD_NULL;
boolean_t	ptepages_wanted = FALSE;

unsigned int	ptepages_freed = 0;
unsigned int	ptepages_taken = 0;

/* enough to cover emergency needs while in get_more_pte_pages */
unsigned int	min_pte_pages = 3;

/* leave enough for minimum and a couple processes */
unsigned int	max_pte_pages = 3 + 2 * PMAP_MIN_PTEPAGES;

/* should be less than max_pte_pages - min_pte_pages */
unsigned int	more_pte_pages = PMAP_MIN_PTEPAGES;

vm_offset_t
get_free_ptepage()
{
	vm_offset_t page;

	check_simple_locks();

	for (;;) {
		simple_lock(&ptepages_lock);
		if (free_ptepages_count > min_pte_pages)
			break;

		if (ptepages_thread == THREAD_NULL) {
			/* nobody else is allocating ptepages */

			ptepages_thread = current_thread();
			simple_unlock(&ptepages_lock);

			get_more_pte_pages();

			simple_lock(&ptepages_lock);
			ptepages_thread = THREAD_NULL;
			simple_unlock(&ptepages_lock);

			/* must check again */
			continue;
		}

		/* somebody is allocating; check for reentrancy/privilege */

		if ((ptepages_thread == current_thread()) ||
		     current_thread()->vm_privilege) {
			if (free_ptepages_count == 0)
				panic("get_free_ptepage");
			break;
		}

		/* block until pages are available */

		ptepages_wanted = TRUE;
		thread_sleep(&ptepages_wanted,
			     simple_lock_addr(ptepages_lock),
			     FALSE);
	}

	page = free_ptepages;
	free_ptepages = * (vm_offset_t *) page;
	free_ptepages_count--;
	inuse_ptepages_count++;
	simple_unlock(&ptepages_lock);

	* (vm_offset_t *) page = 0;
	return page;
}

void
put_free_ptepage(page)
	vm_offset_t page;
{
	/*
	 *	Put the page back on the free list.
	 *	But don't let the free list get too big!
	 */

	simple_lock(&ptepages_lock);
	inuse_ptepages_count--;

	if (free_ptepages_count < max_pte_pages) {
		* (vm_offset_t *) page = free_ptepages;
		free_ptepages = page;
		free_ptepages_count++;
		page = 0;
	}

	if (ptepages_wanted) {
		ptepages_wanted = FALSE;
		thread_wakeup(&ptepages_wanted);
	}

	simple_unlock(&ptepages_lock);

	/*
	 *	Actually release the page if it
	 *	didn't go on the free list.
	 */

	if (page)
		pmap_release_page(K0SEG_TO_PHYS(page));
}

/*
 * Allocate more pages to the pool when they are scarce
 * If we allocate too many, put_free_ptepage will just free them.
 */

get_more_pte_pages()
{
	vm_offset_t vp, pp, pmap_resident_extract();
	int	    i;
	static int  ncalls = 0;

	ptepages_taken++;

#if	COUNTERS
	mpp_count++;
#endif	/*COUNTERS*/
	TRACE({dprintf("pmap: get_more_pte_pages %d\n", ++ncalls);})

	if (kmem_alloc_wired(pmap_submap, &vp, more_pte_pages * MIPS_PGBYTES)
						!= KERN_SUCCESS)
		panic("get_more_pte_pages");

	simple_lock(&ptepages_lock);
	inuse_ptepages_count += more_pte_pages;
	simple_unlock(&ptepages_lock);

	for (i = 0; i < more_pte_pages; i++) {
		/* we speak k0 here */
		pp = pmap_resident_extract(kernel_pmap, vp);
		pp = PHYS_TO_K0SEG(pp);
		bzero(pp, MIPS_PGBYTES);
		put_free_ptepage(pp);
		vp += MIPS_PGBYTES;
	}

	TRACE({printf("pmap: get_more_pte_pages done.\n");})
}

/*
 * Allocate a ptepage to a given pmap.
 * This function is called by the tlb_miss() handler when a ptepage
 * is needed to cover a fault.
 */
void pmap_pte_fault( map, ppte)
pmap_t map;
pte_template  *ppte;
{
	register vm_offset_t  vp, pp;
	register int i;

#if	COUNTERS
	ptf_count++;
#endif	/*COUNTERS*/
	TRACE({printf("pmap_pte_fault x%x x%x\n", map, ppte);})

	if (map == PMAP_NULL) panic("pmap_pte_fault");

	vp = get_free_ptepage();

	/*
	 *	get_free_ptepage() can block, during which time some other
	 *	thread might have covered this pte fault.  Check whether
	 *	we still want the page.
	 */
        if (ppte->raw & PG_V) {
		put_free_ptepage(vp);
		return;
	}

	TRACE({printf("ptepage at %x\n", vp);})

	pp = K0SEG_TO_PHYS(vp);
	ppte->raw = PHYSTOPTE(pp) | PG_V | PG_G | PG_M |
		((VM_PROT_READ|VM_PROT_WRITE) << PG_PROTOFF);
	PMAP_SET_MODIFY(pp);
	/*
	 * Now we need to remember which page we covered,
	 * so that we can dispose of it later.  Since tasks
	 * typically use very few ptepages (normally 3) we keep this
	 * info in the pmap structure itself. If that is not
	 * enough we put pages into an overflow list.
	 * A task can use at most 512 ptepages (2g) which
	 * fit entirely in a page.
	 *
	 * Given the way the kernel manages its virtual space
	 * this is not done for the kernel pmap, and the ptepages
	 * allocated to it are never disposed of.
	 */
	if (map == kernel_pmap) 
		return;

	lock_pmap(map);

	if ((i = map->ptepages_count - PMAP_MIN_PTEPAGES) >= 0) {
		if (i == 0) {
			/*
			 * We overflowed the default allocation.
			 * Grab another page for the oveflow list.
			 */
			vm_offset_t vp;

			vp = get_free_ptepage();
			TRACE({printf("ovf_ptepage at %x\n", vp);})
			map->ovf_ptepages = (vm_offset_t *)vp;
		}
		map->ovf_ptepages[i] = (vm_offset_t)ppte;
	} else
		map->ptepages[map->ptepages_count] = (vm_offset_t)ppte;
	map->ptepages_count++;

	unlock_pmap(map);
}

/*
 * Return a page to the pool, removing all the mappings it contains (if any)
 */
pte_page_gc(map, ppte, dirty)
pmap_t map;
register pt_entry_t  *ppte;
{
	vm_offset_t		pa;
	register vm_offset_t	vaddr;
	register int		i;

#if	COUNTERS
	pgc_count++;
#endif	/*COUNTERS*/
	TRACE({printf("pmap:pte_page_gc x%x x%x\n",map, ppte);})

	/*
	 * This should not be done lightly..
	 */
	if (map == kernel_pmap)
		return;
	/*
	 * Find the phys page
	 */
	pa = PTETOPHYS(ppte);

	/*
	 * Find its virtual address and unmap it
	 */
	vaddr = mips_ptob(((vm_offset_t)ppte - kernel_pmap->ptebase) >> 2);
	*(int*)ppte = 0;		/* Drop it from the kernel pmap */
	tlb_unmap(0, vaddr);		/* Drop it from the tlb */

	/*
	 * Now unmap all the pages that the given ptepage maps
	 */
	if (dirty) {
		ppte = (pt_entry_t*) PHYS_TO_K0SEG(pa);
		/*
		 * Is it possible that this page contains valid
		 * mappings ?  Not right now, but things change
		 */
#define FLUSH_DIRTY_PTES 0
#if	FLUSH_DIRTY_PTES
		vaddr = mips_ptob((vaddr - map->ptebase) >> 2);
					/* base user vaddr for the pages */
		for (i = 0; i < (MIPS_PGBYTES/sizeof(pt_entry_t));
		     i++, ppte++)
			if (ppte->pg_v) {
				tlb_unmap(map->pid, vaddr + mips_ptob(i));
				*(int*)ppte = 0;
			}
#else
		bzero(ppte, MIPS_PGBYTES);
#endif
	}
	/*
	 * And finally put the page back into the free list
	 */
	vaddr = PHYS_TO_K0SEG(pa);
	put_free_ptepage(vaddr);
}



/*
 *	Pmap mgmt.
 *
 */
vm_offset_t	*free_pmaps = 0;

/*
 * Operations defined on the pmap free list:
 *	- test if empty
 *	- add to list
 *	- remove from list
 *	- walk through list
 */
#define	pmap_pool_is_empty()	(free_pmaps == 0)

#define	pmap_pool_add(pmap)						\
MACRO_BEGIN								\
	*((vm_offset_t**)(pmap)) = free_pmaps;				\
	free_pmaps = (vm_offset_t*)(pmap);				\
MACRO_END

#define	pmap_pool_remove(_pmap)						\
MACRO_BEGIN								\
	if (pmap_pool_is_empty()) {					\
		_pmap = (pmap_t) zalloc(pmap_zone);			\
		if (_pmap == PMAP_NULL) panic("ouf of pmaps");		\
		bzero(_pmap, sizeof( struct pmap ));			\
		_pmap->ptebase = next_ptebase();			\
		ptepage_to_pmap_enter(_pmap);				\
	} else {							\
		_pmap = (pmap_t) free_pmaps;				\
		free_pmaps = (vm_offset_t*)*free_pmaps;			\
	}								\
MACRO_END

#define pmap_pool_walk(_p)						\
	for (_p = (pmap_t)free_pmaps; _p; _p = (pmap_t)*((vm_offset_t*)_p))


/*
 *	Machine-level page attributes
 *
 *	The only attribute that may be controlled right now is cacheability.
 *
 *	Obviously these attributes will be used in a sparse
 *	fashion, so we use a simple sorted list of address ranges
 *	which possess the attribute.
 */

zone_t	pmap_range_zone;
#define pmap_range_alloc()	(pmap_range_t)zalloc(pmap_range_zone)
#define pmap_range_free(a)	zfree(pmap_range_zone, (vm_offset_t)(a))

/*
 *	Destroy an attribute list.
 */
pmap_destroy_ranges(ranges)
	register pmap_range_t *ranges;
{
	register pmap_range_t this, next;

	this = *ranges;
	while (this != 0) {
		next = this->next;
		pmap_range_free(this);
		this = next;
	      }
	*ranges = 0;
}

/*
 *	Lookup an address in a sorted range list.
 */
boolean_t
pmap_range_lookup(ranges, address)
	pmap_range_t *ranges;
	vm_offset_t address;
{
	register pmap_range_t range;

	for (range = *ranges; range != 0; range = range->next) {
		if (address < range->start)
			return FALSE;
		if (address < range->end)
			return TRUE;
	}
	return FALSE;
}

/*
 *	Add a range to a list.
 *	The pmap must be locked.
 */
pmap_range_add(ranges, start, end)
	pmap_range_t *ranges;
	vm_offset_t start, end;
{
	register pmap_range_t range, *prev;

	/* look for the start address */

	for (prev = ranges; (range = *prev) != 0; prev = &range->next) {
		if (start < range->start)
			break;
		if (start <= range->end)
			goto start_overlaps;
	}

	/* start address is not present */

	if ((range == 0) || (end < range->start)) {
		/* no overlap; allocate a new range */

		range = pmap_range_alloc();
		range->start = start;
		range->end = end;
		range->next = *prev;
		*prev = range;
		return;
	}

	/* extend existing range forward to start */

	range->start = start;

    start_overlaps:
	assert((range->start <= start) && (start <= range->end));

	/* delete redundant ranges */

	while ((range->next != 0) && (range->next->start <= end)) {
		pmap_range_t old;

		old = range->next;
		range->next = old->next;
		range->end = old->end;
		pmap_range_free(old);
	}

	/* extend existing range backward to end */

	if (range->end < end)
		range->end = end;
}

/*
 *	Remove a range from a list.
 *	The pmap must be locked.
 */
pmap_range_remove(ranges, start, end)
	pmap_range_t *ranges;
	vm_offset_t start, end;
{
	register pmap_range_t range, *prev;

	/* look for start address */

	for (prev = ranges; (range = *prev) != 0; prev = &range->next) {
		if (start <= range->start)
			break;
		if (start < range->end) {
			if (end < range->end) {
				pmap_range_t new;

				/* split this range */

				new = pmap_range_alloc();
				new->next = range->next;
				new->start = end;
				new->end = range->end;

				range->next = new;
				range->end = start;
				return;
			}

			/* truncate this range */

			range->end = start;
		}
	}

	/* start address is not in the middle of a range */

	while ((range != 0) && (range->end <= end)) {
		*prev = range->next;
		pmap_range_free(range);
		range = *prev;
	}

	if ((range != 0) && (range->start < end))
		range->start = end;
}

/*
 *	Initialize the kernel's physical map.
 *
 *	This routine could be called at boot.  It maps a range of
 *	physical addresses into the kernel's virtual address space.
 *
 */
vm_offset_t pmap_map(virt, start, end, prot)
	vm_offset_t	virt;		/* Starting virtual address.	*/
	vm_offset_t	start;		/* Starting physical address.	*/
	vm_offset_t	end;		/* Ending physical address.	*/
	int		prot;		/* Machine indep. protection.	*/
{
	pte_template    pte;
	pt_entry_t     *ppte;
	int             num_mips_pages;

#if	COUNTERS
	bmp_count++;
#endif	/*COUNTERS*/
	num_mips_pages = mips_btop(mips_round_page(end - start));
	pte.raw = PHYSTOPTE(start) | PG_V | PG_G |
		(mips_protection(kernel_pmap, prot) << PG_PROTOFF);
	if (prot & VM_PROT_WRITE) {	/* cannot fault yet */
		pte.raw |= PG_M;
	}

	/*
	 *	Map all pages, in Mips page size steps. 
	 */
	for (; num_mips_pages > 0; num_mips_pages--,
	     virt += MIPS_PGBYTES, pte.raw += MIPS_PGBYTES) {
		ppte = pmap_pte(kernel_pmap, virt);
		*ppte = pte.pte;
		tlb_map_random(0, virt, pte.raw);	/* eager */
		if (pte.raw & PG_M)
			PMAP_SET_MODIFY(PTETOPHYS((&pte)));
	}

	return virt;
}

extern vm_offset_t avail_start, avail_end;
extern vm_offset_t virtual_avail, virtual_end;

/*
 *	Bootstrap the system enough to start using kernel virtual memory.
 *	Allocate the kernel pmap (system page table).
 *
 *	Parameters:
 *	avail_start	PA of first available physical page
 *	avail_end	PA of last available physical page
 *	virtual_avail	VA of first available page (after kernel bss)
 *	virtual_end	VA of last available page (end of kernel space)
 *
 *	&start_text	start of kernel text
 *	&etext		end of kernel text
 */
void pmap_bootstrap()
{
	pte_template 	pte;
	pt_entry_t	*ppte;

	/*
	 *	The kernel's pmap is statically allocated so we don't
	 *	have to use pmap_create, which is unlikely to work
	 *	correctly at this time.
	 */

	kernel_pmap = &kernel_pmap_store;
	simple_lock_init(&kernel_pmap->lock);
	kernel_pmap->ptebase = (vm_offset_t) KPTEADDR;
	/*
	 *	Allocate the distinguished tlbpid value of 0 to
	 *	the kernel pmap. The tlbpid allocator is statically
	 *	initialized and knows about this
	 */
	kernel_pmap->pid = 0;
	/*
	 *	Take a reference on the kernel pmap, even if it
	 *	wont go away.
	 */
	kernel_pmap->ref_count = 1;

	/*
	 *	Initialize our locks
	 */
	lock_init(&pmap_lock, FALSE);
	simple_lock_init(&ptepages_lock);

	/*
	 *	Allocate the kernel root page table entries from the front
	 *	of available physical memory. To avoid wiring one tlb entry
	 *	just for this table we keep the table in kseg0.
	 */

	root_kptes = (vm_offset_t) PHYS_TO_K0SEG(avail_start);
	bzero(root_kptes, MIPS_PGBYTES);
	avail_start += MIPS_PGBYTES;

	/*
	 * 	Note in the table itself that we have the page, although we
	 *	will not map it virtually.  Note that in this way we
	 *	will be able to use less space for the root_ptes (1k
	 *	instead of a full 4k).  Eventually. Maybe.
	 */

	pte.raw = PHYSTOPTE(K0SEG_TO_PHYS(root_kptes)) | PG_V | PG_M | PG_G |
		(mips_protection(kernel_pmap, (VM_PROT_READ | VM_PROT_WRITE)) << PG_PROTOFF);
	ppte = pmap_root_pte(-1);	/* e.g. the last virtual address */
	*ppte = pte.pte;

	/*
	 *	The kernel runs unmapped and cached (k0seg),
	 *	only dynamic data are mapped in k2seg.
	 *	==> No need to map it.
	 */

	/*
	 *	Reserve an initial amount of pages for the ptes.
	 */
	{	vm_offset_t	va, pa;
		int		num_pte_pages, i;

		num_pte_pages = min_pte_pages + 1;
		pa = avail_start;
		avail_start += num_pte_pages * MIPS_PGBYTES;
		for (va = PHYS_TO_K0SEG(pa), i = 0;
		     i < num_pte_pages; i++, va += MIPS_PGBYTES) {
			bzero(va, MIPS_PGBYTES);
			put_free_ptepage(va);
		}
	}
		
	/*
	 *	Assert the kernel virtual address limits.
	 */

	virtual_avail = round_page(VM_MIN_KERNEL_ADDRESS);
	virtual_end   = trunc_page(VM_MAX_KERNEL_ADDRESS);

	printf("Kernel virtual space from 0x%x to 0x%x.\n",
	       virtual_avail, virtual_end);
}

unsigned int pmap_free_pages()
{
	return atop(avail_end - avail_start);
}

vm_offset_t pmap_steal_memory(size)
	vm_size_t size;
{
	vm_offset_t addr;

	/*
	 *	We round the size to an integer multiple.
	 */

	size = (size + 3) &~ 3;
	addr = PHYS_TO_K0SEG(avail_start);
	avail_start += size;
	return addr;
}

void pmap_startup(startp, endp)
	vm_offset_t *startp, *endp;
{
	unsigned int npages, i;
	vm_size_t size;
	vm_page_t pages;

	/*
	 *	For each page, we need
	 *		1) PAGE_SIZE bytes
	 *		2) a vm_page structure
	 *		3) a pv_entry structure
	 *		4) one bit in the modify array
	 */

	npages = ((BYTE_SIZE * (avail_end - avail_start)) /
		  (BYTE_SIZE * (PAGE_SIZE + sizeof *pages +
				sizeof *pv_head_table) + 1));

	size = npages * sizeof *pages;
	pages = (vm_page_t) pmap_steal_memory(size);

	size = npages * sizeof *pv_head_table;
	pv_head_table = (pv_entry_t) pmap_steal_memory(size);
	bzero((char *) pv_head_table, size);

	size = (npages + BYTE_SIZE - 1) / BYTE_SIZE;
	pmap_modify_bits = (unsigned char *) pmap_steal_memory(size);
	bzero((char *) pmap_modify_bits, size);

	avail_start = round_page(avail_start);

	if (npages > pmap_free_pages())
		panic("pmap_startup");

	for (i = 0; i < npages; i++) {
		vm_page_init(&pages[i], avail_start + ptoa(i));
		vm_page_release(&pages[i]);
	}

	*startp = virtual_avail;
	*endp = virtual_end;
}

/*
 *	Initialize the pmap module.
 *	Called by vm_init, to initialize any structures that the pmap
 *	system needs to map virtual memory.
 */
void pmap_init()
{
	long		npages, vsize;
	vm_offset_t	addr, phys, e;
	vm_size_t	s;

	/*
	 *	How many phys pages will we be accounting for
	 */
	npages = atop(avail_end - avail_start);

	/*
	 *	Create our submap to allocate pages from
	 *
	 *	Take enough virtual to cover all phys mem if we
	 *	don't have much of it (pmaxen), but no more than
	 *	50 meg otherwise.  This should be plenty.
	 */
#define _PMAP_MAX_VIRT	0x3000000
	vsize = (npages * PAGE_SIZE);
	if (vsize > _PMAP_MAX_VIRT)
		vsize = _PMAP_MAX_VIRT;
	pmap_submap = kmem_suballoc(kernel_map, &pmap_vlow, &pmap_vhigh, vsize, FALSE);

	/*
	 *	Create the zones of physical maps
	 *	and physical_to_virtual entries
	 */
	s = (vm_size_t) sizeof(struct pmap);
	pmap_zone = zinit(s, 400*s, 4096, FALSE, "pmap");

	s = (vm_size_t) sizeof(struct pv_entry);
	pv_list_zone = zinit(s, 100000*s, 4096, FALSE, "pv_list");

	/*
	 *	Create the zone for cacheability info
	 */
	s = (vm_size_t) sizeof(struct pmap_range);
	pmap_range_zone = zinit( s, 400*s, 4096, FALSE, "pmap_range");

	/*
	 * Enable bookkeeping of the modify list
	 */
	vm_first_phys = avail_start;
	vm_last_phys =  avail_end;

	pmap_module_is_initialized = TRUE;	/* let's dance! */
}



/*
 *	Create and return a physical map.
 *
 *	If the size specified for the map is zero, the map is an actual 
 *	physical map, and may be referenced by the hardware.  In this
 *	case, space for the page table entries is allocated and cleared.
 *
 *	If the size specified is non-zero, the map will be used in software
 *	only, and is bounded by that size.
 */
pmap_t pmap_create(size)
	vm_size_t	size;
{
	register pmap_t			map;
	register pmap_statistics_t	stats;

	/*
	 *	A software use-only map doesn't even need a map.
	 */
	if (size != 0)
		return PMAP_NULL;

#if	COUNTERS
	pcr_count++;
#endif	/*COUNTERS*/
	TRACE({printf("pmap_create x%x\n", size);})

	/*
	 *	Allocate a pmap structure.  Also allocate enough kernel
	 *	virtual addresses for the ptes to cover a USEG segment.
	 *	This assumes the map will not need to cover both user and
	 *	kernel virtual addresses (which are disjoint anyways).
	 */
	pmap_pool_remove(map);

	/*
	 *	Initialize the various fields.
	 */
	map->ref_count = 1;
	simple_lock_init(&map->lock);
	map->pid = -2;		/* lazy evaluate the pid assignment */

	/*
	 *	Initialize statistics.
	 */

	stats = &map->stats;
	stats->resident_count = 0;
	stats->wired_count = 0;

	TRACE({ printf("pmap_create: created at x%x, ptebase x%x\n",
		map, map->ptebase);})

	return(map);
}


/*
 *	Retire the given physical map from service.  Should
 *	only be called if the map contains no valid mappings.
 */
void pmap_destroy(map)
	pmap_t	map;
{
	register vm_offset_t	*ppte, *eppte;
	int     	         c;

	if (map == PMAP_NULL)
		return;

	TRACE({printf("pmap_destroy x%x\n", map);})
#if	COUNTERS
	pds_count++;
#endif	/*COUNTERS*/

	c = --map->ref_count;
	if (c == 0)
		destroy_tlbpid(map->pid, FALSE);

	if (c != 0)
		return;

	/*
	 *	Release all pte pages, then the pmap itself 
	 *	No need to lock or protect here, noone else
	 *	can possibly need this pmap.
	 */

	ppte = map->ptepages;
	eppte = &ppte[PMAP_MIN_PTEPAGES];
	for ( ; ppte < eppte; ppte++)
		if (*ppte) {
			pte_page_gc( map, *ppte, 0);
			*ppte = 0;
			map->ptepages_count--;
		}

	if (map->ovf_ptepages) {
		/*
		 * Release the ones in the overflow list
		 */
		ppte = map->ovf_ptepages;
		eppte = &ppte[map->ptepages_count];
		for ( ; ppte < eppte; ppte++) {
			pte_page_gc( map, *ppte, 0);
			*ppte = 0;
			map->ptepages_count--;
		}
		put_free_ptepage(map->ovf_ptepages);
		map->ovf_ptepages = 0;
	}

	/* Destroy ranges, if any */
	if (map->notcached != 0)
		pmap_destroy_ranges(&map->notcached);

	/* No more hacking */
	map->hacking = 0;

	/* Finally, put the pmap to rest */
	pmap_pool_add(map);
}

/*
 *	Add a reference to the specified pmap.
 */

void pmap_reference(map)
	pmap_t	map;
{
	TRACE({printf("pmap_reference x%x\n", map);})

	if (map != PMAP_NULL) {
		map->ref_count++;
	}
}


/*
 *	Remove a range of mappings.  The given
 *	addresses are the first (inclusive) and last (exclusive)
 *	addresses for the VM pages.
 *
 *	The pmap must be locked.
 */

void pmap_remove_range(map, start, end)
	pmap_t	map;
	vm_offset_t	start, end;
{
	register pt_entry_t	*spte, *cpte;
	pt_entry_t		*epte;
	register pv_entry_t	 pv_h, prev, cur;
	vm_offset_t		 pa;
	int			 vtm;

    TRACE({printf("pmap_remove_range x%x x%x x%x\n", map, start, end);})
#if	COUNTERS
	rrg_count++;
#endif	/*COUNTERS*/

    /*
     * If we never actually used this pmap just return
     */
    if (map->pid == -2)
    	return;

    vtm = vm_to_mips(1);
    while (start < end) {

	spte = pmap_pte(map, start);

	/*
	 *	Since we handle "sparse address spaces", it might well
	 *	be that a ptepage for this VM range has never been
	 *	actually allocated.  Avoid doing it now.
	 *
	 *	Note that this optimization depends on the fact that
	 *	once a pte page is allocated to a pmap, it is never
	 *	reclaimed until the pmap is destroyed.
	 */
	if (!pmap_pte_page_exists(map, spte)) {
		/*
		 * Bump start to the address covered
		 * by the next pte page.
		 */
		start += MIPS_PGBYTES * PTES_PER_PAGE;
		start &= ~(MIPS_PGBYTES * PTES_PER_PAGE - 1);
		continue;
	}

	/*
	 *	Find the last (non-inclusive) pte,
	 *	or the last pte in the current pte
	 *	page, which ever is less.
	 *	Note that all ptes are contiguous.
	 */
	epte = (pt_entry_t *)mips_trunc_page(spte + PTES_PER_PAGE);
	if (epte > spte + mips_btop(end-start))
		epte = spte + mips_btop(end-start);

	/*
	 *	Remove each mapping from the PV list
	 *
	 */
	for (cpte = spte; cpte<epte; cpte += vtm, start += PAGE_SIZE) {

		/*
		 *	Get the physical address, skipping this
		 *	page if there is no physical mapping.
		 */
		pa = PTETOPHYS(cpte);
		if (pa == 0)
		    continue;	/* NOTE: this saves us from screwing up
		    		 * during initialization, too.
				 */

		/*
		 *	Selectively invalidate the tlb.
		 */
		if ((map->pid != -1) && ((*(int*)cpte) & PG_V)) {
			register int i;
			/*
			 *	Unroll the loop manually
			 */
			tlb_unmap(map->pid, start);
			for (i = vtm - 1; i; i--)
				tlb_unmap(map->pid, start + (i * MIPS_PGBYTES));
		}

		if (map->hacking)
			(*map->hacking)(map, start);

		if (pa >= vm_first_phys && pa < vm_last_phys) {

		    map->stats.resident_count--;

		    pv_h = pa_to_pvh(pa);
		    if (pv_h->pmap == PMAP_NULL)
			panic("pmap_remove_range: null pv_list!");

		    if (pv_h->va == start && pv_h->pmap == map) {
			/*
			 *	Header is the pv_entry.  Copy the next one
			 *	to header and free the next one (we can't
			 *	free the header)
			 */
			cur = pv_h->next;
			if (cur != PV_ENTRY_NULL) {
				*pv_h = *cur;
				zfree(pv_list_zone, (vm_offset_t) cur);
			}
			else {
			    	pv_h->pmap = PMAP_NULL;
			}
		    }
		    else {
			prev = pv_h;
			while ((cur = prev->next) != PV_ENTRY_NULL) {
				if (cur->va == start && cur->pmap == map)
					break;
				prev = cur;
			}
			if (cur == PV_ENTRY_NULL) {
				printf("va %x map %x pvh %x\n", start, map, pv_h);
				panic("pmap_remove_range: mapping not in pv_list!");
			}
			prev->next = cur->next;
			zfree(pv_list_zone, (vm_offset_t)cur);
		    }
		}
		/*
		 * 	Done with this page, zero pte(s)
		 */
		{ register int i;
			*(int*)cpte = 0;
			/*
			 *	Unroll the loop manually
			 */
			for (i = vtm - 1; i; i--)
				*(int*)(cpte + i) = 0;
		}
	}
    }
}


/*
 *	Remove the given range of addresses from the specified map.  It is 
 *	assumed that start and end are rounded to the VM page size.
 */

void pmap_remove(map, start, end)
	pmap_t		map;
	vm_offset_t	start, end;
{
	if (map == PMAP_NULL)
		return;

	READ_LOCK();
	lock_pmap(map);

	pmap_remove_range(map, start, end);

	unlock_pmap(map);
	READ_UNLOCK();
}


/*
 *	Routine:	pmap_remove_all
 *	Function:
 *		Removes this physical page from
 *		all physical maps in which it resides.
 */
void pmap_remove_all(phys)
	vm_offset_t	phys;
{
	register pv_entry_t	pv_h;
	pv_entry_t		cur;
	register pt_entry_t	*pte, *end_pte;
	vm_offset_t		va;
	register pmap_t 	map;

	TRACE({printf("pmap_remove_all x%x\n", phys);})
#if	COUNTERS
	ral_count++;
#endif	/*COUNTERS*/

	if (phys < vm_first_phys || phys >= vm_last_phys)
		return;	/* can't handle */

	/*
	 *	Lock the pmap system first, since we might be changing
	 *	several pmaps.
	 */

	WRITE_LOCK();

	/*
	 *	Walk down PV list, removing all mappings.
	 *	We have to do the same work as in pmap_remove_range
	 *	since that routine locks the pv_head.  We don't have
	 *	to lock the pv_head, since we have the entire pmap system.
	 */

	pv_h = pa_to_pvh(phys);
	while ( (map = pv_h->pmap) != PMAP_NULL) {

		lock_pmap(map);

		va = pv_h->va;
		pte = pmap_pte(map, va);

		map->stats.resident_count--;

		if ((cur = pv_h->next) != PV_ENTRY_NULL) {
			*pv_h = *cur;
			zfree(pv_list_zone, (vm_offset_t) cur);
		}
		else
			pv_h->pmap = PMAP_NULL;

		for (end_pte = pte + mips_btop(PAGE_SIZE) ; pte < end_pte ;
			pte++, va += MIPS_PGBYTES) {

			if (pte->pg_v)
				tlb_unmap(map->pid, va);

			if (map->hacking)
				(*map->hacking)(map, va);

			*(int *)pte = 0;
		}

		unlock_pmap(map);
	}
	WRITE_UNLOCK();
}


/*
 *	Routine:	pmap_copy_on_write
 *	Function:
 *		Remove write privileges from all
 *		physical maps for this physical page.
 */

void pmap_copy_on_write(phys)
	vm_offset_t	phys;
{
	register pv_entry_t	 pv_e;
	register pmap_t		 map;
	register pt_entry_t	*pte, *end_pte;
	vm_offset_t		 va;

	TRACE({printf("pmap_copy_on_write x%x\n", phys);})
#if	COUNTERS
	cow_count++;
#endif	/*COUNTERS*/

	if (phys < vm_first_phys || phys >= vm_last_phys)
		return;	/* can't handle */

	/*
	 *	Lock the entire pmap system, since we might change several maps.
	 */
	WRITE_LOCK();

	/*
	 *	Run down the list of mappings to this physical page,
	 *	disabling write privileges on each one.
	 */
	pv_e = pa_to_pvh(phys);

	if (pv_e->pmap != PMAP_NULL)
	do {
		/*
		 *	Lock this pmap.
		 */
		map = pv_e->pmap;		
		lock_pmap(map);

		/*
		 *	If there is a mapping, change the protections and
		 *  	modify the tlb entry accordingly.
		 */
		va = pv_e->va;
		pte = pmap_pte(map, va);

		for (end_pte = pte + mips_btop(PAGE_SIZE);
		     pte < end_pte; pte++, va += MIPS_PGBYTES) {
			register unsigned e = *(unsigned*)pte;
			if (e & (PG_V|VM_PROT_WRITE)) {
				tlb_modify(map->pid, va, 0);
				*(unsigned*)pte = e & ~(PG_M|VM_PROT_WRITE);
			}
		}
		/*
		 *	Done with this one, so drop the lock, and get next pv entry.
		 */
		unlock_pmap(map);
		pv_e = pv_e->next;

	} while (pv_e != PV_ENTRY_NULL);

	WRITE_UNLOCK();
}


/*
 *	Set the physical protection on the
 *	specified range of this map as requested.
 */
void pmap_protect(map, start, end, prot)
	register pmap_t map;
	register vm_offset_t start;
	vm_offset_t	end;
	vm_prot_t	prot;
{
	register pt_entry_t *pte, *end_pte;
	register boolean_t change_mappings = TRUE;
	register boolean_t flush_cache = FALSE;

	TRACE({printf("pmap_protect x%x x%x x%x x%x\n", map, start, end, prot);})

	if (map == PMAP_NULL)
		return;

	/*
	 *	Protection combinations without read are no access
	 *	pmap_protect may not increase protection, so RW is a no-op
	 *	Must flush cache if execute permission was specified.
	 */
	if ((prot & VM_PROT_READ) == 0) {
		pmap_remove(map, start, end);
		return;
	}

	if (prot & VM_PROT_EXECUTE)
		flush_cache = TRUE;
	if (prot & VM_PROT_WRITE)
		if (flush_cache)
			change_mappings = FALSE;
		else
			return;

#if	COUNTERS
	ppt_count++;
#endif	/*COUNTERS*/
	lock_pmap(map);

	while (start < end) {
		pte = pmap_pte(map, start);

		/*
		 *	Since we handle "sparse address spaces", it might well
		 *	be that a ptepage for this VM range has never been
		 *	actually allocated.  Avoid doing it now.
		 *
		 *	Note that this optimization depends on the fact that
		 *	once a pte page is allocated to a pmap, it is never
		 *	reclaimed until the pmap is destroyed.
		 */
		if (!pmap_pte_page_exists(map, pte)) {
			/*
			 * Bump start to the address covered
			 * by the next pte page.
			 */
			start += MIPS_PGBYTES * PTES_PER_PAGE;
			start &= ~(MIPS_PGBYTES * PTES_PER_PAGE - 1);
			continue;
		}

		/*
		 * Only change protection if there is a mapping.
		 * We are removing write protection.  If the entry
		 * was modifiable, we must modify the tlb.
		 */
		if (*(unsigned*)pte & PG_V) {
		    vm_offset_t p = PTETOPHYS(pte);

		    if (change_mappings) {
			for (end_pte = pte + mips_btop(PAGE_SIZE);
			     pte < end_pte;
			     pte++, start += MIPS_PGBYTES) {
				vm_prot_t new_prot = pte->pg_prot & prot;

				if (*(unsigned*)pte & PG_M) {
					pte->pg_m = 0;
					tlb_modify(map->pid, start, 0);
				}
				pte->pg_prot = mips_protection(map, new_prot);
			}
		    } else
			start += PAGE_SIZE;

		    /*
		     *	Now if this is executable code
		     *	there might be inconsistencies among the two
		     *	caches, so we flush the I-cache just in case.
		     *
		     *	WARNING: This may be needed on boxes that don't
		     *	    need the cache flush in pmap_enter.
		     */
		    if (flush_cache) {
			mipscache_Iflush(PHYS_TO_K0SEG(p), PAGE_SIZE);
#if	COUNTERS
			ifl_count++;
#endif	/*COUNTERS*/
		    }
		} else
		    start += PAGE_SIZE;
	}

	unlock_pmap(map);
}


/*
 *	Insert the given physical page (p) at
 *	the specified virtual address (v) in the
 *	target physical map with the protection requested.
 *
 *	If specified, the page will be wired down, meaning
 *	that the related pte can not be reclaimed.
 *
 *	NB:  This is the only routine which MAY NOT lazy-evaluate
 *	or lose information.  That is, this routine must actually
 *	insert this page into the given map NOW.
 */
boolean_t	pmap_uncache_io_space = TRUE;

void pmap_enter(map, v, p, prot, wired)
	register pmap_t map;
	vm_offset_t	v;
	vm_offset_t	p;
	vm_prot_t	prot;
	boolean_t	wired;
{
	register pt_entry_t	*pte, *end_pte;
	register pv_entry_t	pv_h;
	register pv_entry_t	pv_e;
	vm_offset_t		phys,ptaddr;
	unsigned		s,s1;
	pte_template		tmpl;
	boolean_t 		flush_needed = FALSE;

	assert(p != vm_page_fictitious_addr);
	TRACE({printf("pmap_enter x%x x%x x%x x%x x%x ", map, v, p, prot, wired);})

	if (map == PMAP_NULL)
		return;

#if	COUNTERS
	ent_count++;
#endif	/*COUNTERS*/
	if (pv_head_table == PV_ENTRY_NULL) {	/* pmap system up ? */
		pmap_map(v, p, p + PAGE_SIZE, prot);
		return;
	}

	/*
	 *	If we are going to need more ptepages grab them now,
	 *	before we take the lock on the pmap system.
	 *	Failure scenario:
	 *		pmap_enter -> get_more_ptepages -> kmem_alloc ->
	 *		no more pages, wakeup the pageout daemon
	 *		pmap_remove_all -> lock_write -> deadlock.
	 *	PTETOPHYS uses volatile, so the compiler should retain
	 *	the code, although this ptaddr isn't used.
	 */
	pte = pmap_pte(map,v);
	ptaddr = PTETOPHYS(pte);
	TRACE({printf("(%x)\n", ptaddr);})

	pv_e = PV_ENTRY_NULL;
Retry:
	READ_LOCK();
	lock_pmap(map);
	/*
	 * The READ_LOCK macro above can block, therefore we must get
	 * the ptaddr again in case it changed while we were asleep.
	 * This reference to the pte can't cause a pte page fault,
	 * since we made sure the pte page was present before we took
	 * the lock.
	 */
	ptaddr = PTETOPHYS(pte);
	
	/*
	 *	Enter the mapping in the PV list for this physical page.
	 *	If there is already a mapping, remove the old one first.
	 *	(If it's the same physical page, it's already in the PV list.)
	 */

	if (ptaddr != p) {
		if (ptaddr != 0)
			pmap_remove_range(map, v, (v + PAGE_SIZE));

		/*
		 *	Only keep the pv_list info for the pages we handle,
		 *	e.g. do not mess around with I/O or other weirdos.
		 */
		if (p >= vm_first_phys && p < vm_last_phys) {

			pv_h = pa_to_pvh(p);

			if (pv_h->pmap == PMAP_NULL) {
				/*
				 *	No mappings yet
				 */
				flush_needed = TRUE;
				pv_h->va = v;
				pv_h->pmap = map;
				pv_h->next = PV_ENTRY_NULL;
			} else {
				/*
				 *	Add new pv_entry after header.
				 */
				if (pv_e == PV_ENTRY_NULL) {
					unlock_pmap(map);
					READ_UNLOCK();
					pv_e = (pv_entry_t) zalloc(pv_list_zone);
					goto Retry;
				}
				pv_e->va = v;
				pv_e->pmap = map;
				pv_e->next = pv_h->next;
				pv_h->next = pv_e;
				/*
				 *	Remember that we used the pv_list entry.
				 */
				pv_e = PV_ENTRY_NULL;
			}

			map->stats.resident_count++;

		}
	}

	/*
	 *	Enter the mapping in each Mips pte.
	 */
	if (*(long *)pte & PG_V) {

		/*
		 *	Replacing a valid mapping - only worry about
		 *	write permissions, since we could only be changing
		 *	protection or wired bits here, not the phys page.
		 *
		 */

		if (pte->pg_prot != mips_protection(map, prot)) {
			/*
			 *	Changing protection and possibly wiring.
			 */
			for ( end_pte = pte + mips_btop(PAGE_SIZE);
			      pte < end_pte; pte++, v += MIPS_PGBYTES) {
				if ((prot & VM_PROT_WRITE) == 0) {
					tlb_modify(map->pid, v, 0);
					pte->pg_m = 0;
				}
				pte->pg_prot = mips_protection(map, prot);
			}
		}
	}
	else {
		/*
		 *	Not a valid mapping -
		 *	Set up a template to use in making the pte's.
		 */
		tmpl.raw = PHYSTOPTE(p)| PG_V |
				(mips_protection(map, prot) << PG_PROTOFF);

		if (ISA_K2SEG(v)) {
			/*
			 * Kernel virtual space is global
			 */
			tmpl.raw |= PG_G;
		} else {
			/*
			 * We are basically interested in I/O space here.
			 * Apparently, even a bitmap screen has no reasons
			 * to be cached [impeds performance of other programs
			 * by uselessly filling the cache], so we make
			 * all I/O space memory non-cachable.
			 */
			 if (pmap_uncache_io_space && (p > mem_size))
				tmpl.raw |= PG_N;
		}

		if ((map->notcached != 0) &&
		    pmap_range_lookup(&map->notcached, v))
			tmpl.raw |= PG_N;

		/*
		 *	Now use the template to set up all the pte's.
		 */
		for ( end_pte = pte + mips_btop(PAGE_SIZE) ; pte < end_pte; pte++,
			  tmpl.raw += MIPS_PGBYTES, v += MIPS_PGBYTES) {
				*pte = tmpl.pte;
				tlb_map_random(map->pid, v, tmpl.raw);
		  }
		/*
		 *	Now if this is executable code we are mapping
		 *	there might be inconsistencies among the two
		 *	caches, so we flush the I-cache just in case.
		 *	This only happens on boxes that do not do DMA,
		 *	otherwise bufflush() would do it.
		 */
#if	COUNTERS
		if (flush_needed) {
		    	ifn_count++;
			if (prot & VM_PROT_EXECUTE) {
				mipscache_Iflush(PHYS_TO_K0SEG(p), PAGE_SIZE);
				ifl_count++;
			}
		}
#else	COUNTERS
		if (flush_needed && (prot & VM_PROT_EXECUTE))
			mipscache_Iflush(PHYS_TO_K0SEG(p), PAGE_SIZE);
#endif	/*COUNTERS*/
	}

	if (pv_e != PV_ENTRY_NULL)
		zfree(pv_list_zone, (vm_offset_t) pv_e);

	unlock_pmap(map);
	READ_UNLOCK();
}


/*
 *	Routine:	pmap_change_wiring
 *	Function:	Change the wiring attribute for a map/virtual-address
 *			pair.
 *	In/out conditions:
 *			The mapping must already exist in the pmap.
 */
void pmap_change_wiring(map, v, wired)
	register pmap_t	map;
	vm_offset_t	v;
	boolean_t	wired;
{
}

/*
 *	Routine:	pmap_extract
 *	Function:
 *		Extract the physical page address associated with the given
 *		map/virtual_address pair.  The address includes the offset
 *		within a page.
 */

vm_offset_t pmap_extract(map, va)
	register pmap_t		map;
	vm_offset_t		va;
{
	pt_entry_t	 		*pte;
	unsigned long   	pa;

	TRACE({printf("pmap_extract x%x x%x ..", map, va);})
#if	COUNTERS
	ext_count++;
#endif	/*COUNTERS*/

	if (map == kernel_pmap) {
		/*
		 *	Special translation for kernel addresses
		 *	in K1 or K0 space (directly mapped to
		 *	physical addresses).
		 */
		if (ISA_K1SEG(va))
			return K1SEG_TO_PHYS(va);
		if (ISA_K0SEG(va))
			return K0SEG_TO_PHYS(va);
	}

	lock_pmap(map);

	pte = pmap_pte(map, va);
	if (!pmap_pte_page_exists(map, pte) || pte->pg_v == 0)
		pa = 0;
	else
		pa = PTETOPHYS(pte) + (va & VA_OFFMASK);

	unlock_pmap(map);

	TRACE({printf(" %x.\n", pa);})
	return (pa);
}


/*
 *	Routine:	pmap_resident_extract
 *	Function:
 *		Extract the physical page address associated with the given
 *		virtual address in the kernel pmap.  The address includes
 *		the offset within a page.  This routine does not lock the
 *		pmap; it is intended to be called only by device drivers
 *		or copyin/copyout routines that know the buffer whose address
 *		is being translated is memory-resident.
 */

vm_offset_t pmap_resident_extract(map, va)
pmap_t		map;
vm_offset_t	va;
{
	pt_entry_t     *pte;
	register vm_offset_t ret;

	TRACE({printf("pmap_resident_extract x%x x%x\n", map, va);})
#if	COUNTERS
	ext_count++;
#endif	/*COUNTERS*/

	pte = pmap_pte(map, va);
	if (pte->pg_v == 0)
		ret = 0;
	else
		ret = (PTETOPHYS(pte) + (va & VA_OFFMASK));
	return ret;
}


/*
 *	Routine:	pmap_access
 *	Function:
 *		Returns whether there is a valid mapping for the
 *		given virtual address stored in the given physical map.
 */

boolean_t pmap_access(map, va)
	register pmap_t		map;
	vm_offset_t		va;
{
	register pt_entry_t	*pte;

	TRACE({printf("pmap_access x%x x%x\n", map, va);})
#if	COUNTERS
	acc_count++;
#endif	/*COUNTERS*/

	pte = pmap_pte(map, va);
	return (pmap_pte_page_exists(map,pte) && pte->pg_v);
}


/*
 *	Routine:	pmap_release_page
 *	Function:
 *		Deallocate a pte page that is no longer wanted.
 *		The caller supplies the physical address, and
 *		we convert this to the virtual address in pmap_submap.
 */

void pmap_release_page(pa)
	vm_offset_t pa;
{
	register pv_entry_t pv;

	ptepages_freed++;

	/*
	 *	Look for the mapping in the kernel pmap
	 *	in the right virtual range.  There should
	 *	only be one such.
	 */

	for (pv = pa_to_pvh(pa);; pv = pv->next) {
		register pmap_t pmap = pv->pmap;

		if (pmap == PMAP_NULL)
			panic("pmap_release_page");

		if ((pv->pmap == kernel_pmap) &&
		    (pmap_vlow <= pv->va) &&
		    (pv->va < pmap_vhigh))
			break;
	}

	kmem_free(pmap_submap, pv->va, MIPS_PGBYTES);
}

/*
 *	Routine:	pmap_collect
 *	Function:
 *		Garbage collects the physical map system for
 *		pages which are no longer used.
 *		Success need not be guaranteed -- that is, there
 *		may well be pages which are not referenced, but
 *		others may be collected.
 *	Usage:
 *		Called by the pageout daemon when pages are scarce.
 *
 */
void pmap_collect(map)
	register pmap_t		map;
{
#ifdef	notdef
	register vm_offset_t   *pg, *epg, v;
	register pv_entry_t	pv;

	if (map == PMAP_NULL || map == kernel_pmap)
		return;

	/*
	 *	Since we fragmented pages badly, can't do anything
	 *	if the pagesize does not match the natural pagesize.
	 */
	if (PAGE_SIZE != MIPS_PGBYTES)
		return;

	/*
	 *	Walk through all ptepages and release them.
	 *	If there are any more than the default, walk
	 *	through the overflow list and release them.
	 *	Finally, release the overflow list itself.
	 */
	pg = map->ptepages, epg = &pg[PMAP_MIN_PTEPAGES];
	for (; pg < epg; pg++) {
		if (v = *pg) {
			*pg = 0, map->ptepages_count--;
			pmap_release_page(PTETOPHYS(v));
		}
	}

	if (map->ovf_ptepages != 0) {
		pg = map->ovf_ptepages, epg = &pg[map->ptepages_count];
		for (; pg < epg; pg++) {
			if (v = *pg) {
				*pg = 0, map->ptepages_count--;
				pmap_release_page(PTETOPHYS(v));
			}
		}
		map->ovf_ptepages = 0;
		pmap_release_page(K0SEG_TO_PHYS(v));
	}

	/*
	 *	If pages are so scarce then we should probably
	 *	give up all resources we stashed away
	 */
	pmap_pool_walk(map)
		pmap_collect(map);
#endif	notdef
}

/*
 *	Routine:	pmap_activate
 *	Function:
 *		Binds the given physical map to the given processor.
 */
void pmap_activate(map, th, cpu)
	pmap_t		map;
	thread_t	th;
	int		cpu;
{
	TRACE({printf("pmap_activate x%x x%x\n", map, th);})
	PMAP_ACTIVATE(map, th, cpu)
}


/*
 *	Routine:	pmap_deactivate
 *	Function:
 *		Indicates that the given physical map is no longer
 *		in use on the specified processor.
 */
void pmap_deactivate(map, th, cpu)
	pmap_t		map;
	thread_t	th;
	int		cpu;
{
	TRACE({printf("pmap_deactivate x%x x%x\n", map, th);})
	PMAP_DEACTIVATE(map, th, cpu);
}

/*
 *	Routine:	pmap_kernel
 *	Function:
 *		Returns the physical map handle for the kernel.
 */
#if	0
/* See pmap.h */
pmap_t pmap_kernel()
{
	return (kernel_pmap);
}
#endif	0


/*
 *	pmap_zero_page zeros the specified (machine independent) page.
 *	pmap_copy_page copies the specified (machine independent) pages.
 *
 */
pmap_zero_page(phys)
	register vm_offset_t	phys;
{
	/*
	 *	Use cache.
	 */
	register int	 i, max;
	register long	*addr;

	assert(phys != vm_page_fictitious_addr);
	TRACE({printf("pmap_zero_page x%x\n", phys);})
#if	COUNTERS
	zer_count++;
#endif	/*COUNTERS*/

	PMAP_SET_MODIFY(phys);

	addr = (long *)(PHYS_TO_K0SEG(phys));

#define UNROLL 16
	for (i = 0, max = PAGE_SIZE; i < max;
	     i += UNROLL*sizeof(long), addr += UNROLL) {
                addr[ 0] = 0; addr[ 1] = 0; addr[ 2] = 0; addr[ 3] = 0;
                addr[ 4] = 0; addr[ 5] = 0; addr[ 6] = 0; addr[ 7] = 0;
                addr[ 8] = 0; addr[ 9] = 0; addr[10] = 0; addr[11] = 0;
                addr[12] = 0; addr[13] = 0; addr[14] = 0; addr[15] = 0;
	}
#undef	UNROLL
}

pmap_copy_page(src, dst)
	register vm_offset_t src, dst;
{
	assert(src != vm_page_fictitious_addr);
	assert(dst != vm_page_fictitious_addr);
#if	COUNTERS
	cop_count++;
#endif	/*COUNTERS*/
	PMAP_SET_MODIFY(dst);
	src = PHYS_TO_K0SEG(src);
	dst = PHYS_TO_K0SEG(dst);

	aligned_block_copy(src, dst, PAGE_SIZE);
}


/*
 *	pmap_page_protect:
 *
 *	Lower the permission for all mappings to a given page.
 */
void	pmap_page_protect(phys, prot)
	vm_offset_t	phys;
	vm_prot_t	prot;
{
	assert(phys != vm_page_fictitious_addr);
	TRACE({printf("pmap_page_protect x%x x%x\n", phys, prot);})
#if	COUNTERS
	prt_count++;
#endif	/*COUNTERS*/
	switch (prot) {
		case VM_PROT_READ:
		case VM_PROT_READ|VM_PROT_EXECUTE:
			pmap_copy_on_write(phys);
			break;
		case VM_PROT_ALL:
			break;
		default:
			pmap_remove_all(phys);
			break;
	}
}

/*
 *	Routine:	pmap_pageable
 *	Function:
 *		Make the specified pages (by pmap, offset)
 *		pageable (or not) as requested.
 *
 *		A page which is not pageable may not take
 *		a fault; therefore, its page table entry
 *		must remain valid for the duration.
 *
 *		This routine is merely advisory; pmap_enter
 *		will specify that these pages are to be wired
 *		down (or not) as appropriate.
 */
#if	0
/* See pmap.h */
pmap_pageable(map, start, end, pageable)
	pmap_t		map;
	vm_offset_t	start;
	vm_offset_t	end;
	boolean_t	pageable;
{
}
#endif	0

/*
 *	Set the modify bits on the specified physical page.
 */

pmap_set_modify(phys)
	vm_offset_t	phys;
{
	assert(phys != vm_page_fictitious_addr);
#if	COUNTERS
	smd_count++;
#endif	/*COUNTERS*/

	PMAP_SET_MODIFY(phys)
}

/*
 *	Clear the modify bits on the specified physical page.
 */

void pmap_clear_modify(phys)
	vm_offset_t	phys;
{
	assert(phys != vm_page_fictitious_addr);
	TRACE({printf("pmap_clear_modify x%x\n", phys);})
#if	COUNTERS
	clr_count++;
#endif	/*COUNTERS*/

	if (phys >= vm_first_phys && phys < vm_last_phys)
		clear_modify_bit(phys);
}

/*
 *	pmap_is_modified:
 *
 *	Return whether or not the specified physical page is modified
 *	by any physical maps.
 *
 */

boolean_t pmap_is_modified(phys)
	vm_offset_t	phys;
{
	assert(phys != vm_page_fictitious_addr);
	TRACE({printf("pmap_is_modified x%x\n", phys);})
#if	COUNTERS
	imd_count++;
#endif	/*COUNTERS*/

	if (phys >= vm_first_phys && phys < vm_last_phys) {
		return_modify_bit(phys);
	} else
		return FALSE;
}

/*
 *	pmap_clear_reference:
 *
 *	Clear the reference bit on the specified physical page.
 *
 *	Since the Mips mmu does not have reference bits, we'd
 *	have to do it in software, e.g. using the tlb miss as
 *	trigger.  That surely slows dows the tlb miss code
 *	(critical), and provides unknown gains.  Since at least
 *	two machines (Vax and Encore) have successfully used
 *	the strategy of	putting all pages in the inactive list
 *	we do the same here.
 *	If you do not like it, (re)define REF_BITS
 *	and recompile.
 *	
 */

void pmap_clear_reference(phys)
	vm_offset_t	phys;
{
	assert(phys != vm_page_fictitious_addr);
#if	COUNTERS
	crr_count++;
#endif	/*COUNTERS*/
#if	REF_BITS
	if (phys < vm_last_phys)
		clear_reference_bit(phys);
#else	/*REF_BITS*/
	pmap_remove_all(phys);
#endif	/*REF_BITS*/
}

/*
 *	pmap_is_referenced:
 *
 *	Return whether or not the specified physical page is referenced
 *	by any physical maps.
 *
 *	See comment above.
 */
#undef pmap_is_referenced
boolean_t pmap_is_referenced(phys)
	vm_offset_t	phys;
{
	assert(phys != vm_page_fictitious_addr);
#if	COUNTERS
	irr_count++;
#endif	/*COUNTERS*/
#if	REF_BITS
	if (phys < vm_last_phys)
		return_reference_bit(phys);
#endif	/*REF_BITS*/
	return FALSE;
}

/*
 *	copy_from_phys:
 *
 *	Copy from a physically addressed page to a virtually
 *	addressed one.
 */

void copy_from_phys(src, dst, cnt)
	vm_offset_t	src, dst;
	unsigned int	cnt;
{
	assert(src != vm_page_fictitious_addr);
	TRACE({printf("pmap:copy_from_phys x%x x%x x%x\n", src, dst, cnt);})
	aligned_block_copy(PHYS_TO_K0SEG(src), dst, cnt);
}

/*
 *	copy_to_phys:
 *
 *	Copy from a virtually addressed page to a physically
 *	addressed one.
 */

void copy_to_phys(src, dst, cnt)
	vm_offset_t	src, dst;
	unsigned int	cnt;
{
	assert(dst != vm_page_fictitious_addr);
	TRACE({printf("pmap:copy_to_phys x%x x%x x%x\n", src, dst, cnt);})
#if	COUNTERS
	tph_count++;
#endif	/*COUNTERS*/
	aligned_block_copy(src, PHYS_TO_K0SEG(dst), cnt);
}

/*
 *	verify_free:
 *
 *	Check whether the given page is really not mapped in any pmap
 */
boolean_t pmap_verify_free(phys)
	vm_offset_t phys;
{
	pv_entry_t	pv;

	assert(phys != vm_page_fictitious_addr);
	if (!pmap_module_is_initialized)
		return(TRUE);

	if (phys < vm_first_phys || phys >= vm_last_phys)
		return(FALSE);

	pv = pa_to_pvh(phys);
	return (pv->pmap == PMAP_NULL);

}

/*
 *	kvtophys:
 *
 *	Translate from any kernel address to physical address
 */
vm_offset_t kvtophys(virt)
	vm_offset_t virt;
{
	if (ISA_K2SEG(virt))
		return pmap_resident_extract(kernel_pmap, virt);
	else if (ISA_K1SEG(virt))
		return K1SEG_TO_PHYS(virt);
	else if (ISA_K0SEG(virt))
		return K0SEG_TO_PHYS(virt);
	else
		return (vm_offset_t) 0;
}


/*
 *	pmap_attributes:
 *
 *	Set/Get special memory attributes
 *
 */
kern_return_t pmap_attribute(pmap, address, size, attribute, value)
	pmap_t		pmap;
	vm_offset_t	address;
	vm_size_t	size;
	vm_machine_attribute_t	attribute;
	vm_machine_attribute_val_t* value;		/* IN/OUT */
{
	vm_offset_t 	start, end;
	kern_return_t		ret;

	if (attribute != MATTR_CACHE)
		return KERN_INVALID_ARGUMENT;

	if (pmap == PMAP_NULL)
		return KERN_SUCCESS;

	start = trunc_page(address);
	end = round_page(address + size);
	ret = KERN_SUCCESS;

	lock_pmap(pmap);

	switch (*value) {

	case MATTR_VAL_OFF:		/* (generic) turn attribute off */

		pmap_range_add(&pmap->notcached, start, end);
		pmap_remove_range(pmap, start, end);
		break;

	case MATTR_VAL_ON:		/* (generic) turn attribute on */

		pmap_range_remove(&pmap->notcached, start, end);
		pmap_remove_range(pmap, start, end);
		break;

	case MATTR_VAL_GET:		/* (generic) return current value */

		if (pmap_range_lookup(&pmap->notcached, start))
			*value = MATTR_VAL_OFF;
		else
			*value = MATTR_VAL_ON;
		break;

	case MATTR_VAL_CACHE_FLUSH:	/* flush from all caches */
	case MATTR_VAL_DCACHE_FLUSH:	/* flush from data cache(s) */
	case MATTR_VAL_ICACHE_FLUSH: {	/* flush from instruction cache(s) */
		int op;

		/* which cache should we flush */
		op = 0;
		if (*value == MATTR_VAL_CACHE_FLUSH ||
		    *value == MATTR_VAL_ICACHE_FLUSH)
			op = 1;
		if (*value == MATTR_VAL_CACHE_FLUSH ||
		    *value == MATTR_VAL_DCACHE_FLUSH)
			op |= 2;

		for (; start < end; start += MIPS_PGBYTES) {
			vm_offset_t addr;

			addr = pmap_resident_extract(pmap, start);
			if (addr == 0)
				continue;
			addr = PHYS_TO_K0SEG(addr);

			if (op & 1)
				mipscache_Iflush(addr, MIPS_PGBYTES);
			if (op & 2)
				mipscache_Dflush(addr, MIPS_PGBYTES);
		}
		break;
	}

	default:
		ret = KERN_INVALID_ARGUMENT;
		break;

	}

out:
	unlock_pmap(pmap);

	return ret;
}

#if	0
print_attributes(pmap)
	pmap_t pmap;
{
	pmap_range_t range;

	for (range = pmap->notcached; range != 0; range = range->next)
		printf("%8x - %8x\n", range->start, range->end);
}
#endif

/***************************************************************************
 *
 *	TLBPID Management
 *
 *	The MIPS tlb (coprocessor 0) uses the TLBPID register
 *	while searching for a valid mapping of a virtual address.
 *	The register has only a limited size, hence this module
 *	handles the hashing from pmaps to TLBPIDs
 *
 */


#define MAX_PID		TLB_HI_NPID
#define	PID_MASK	(MAX_PID-1)

static struct pmap *pids_in_use[MAX_PID] = {0,};
static int next_pid = 1;
/*
 * NOTE: It might be possible to use the pid 0 too. Since it is
 *	 assigned to the kernel_pmap that only holds PG_G type
 *	 mappings the conflict would be unnoticeable.
 *	 But it tastes too awful.
 */

/*
 * Axioms:
 *	- next_pid always points to a free one, unless the table is full;
 *	  in that case it points to a likely candidate for recycling.
 *	- pmap.pid prevents from making duplicates: if -1 there is no
 *	  pid for it, otherwise there is one and only one entry at that index.
 *
 * assign_tlbpid	provides a tlbpid for the given pmap, creating
 *			a new one if necessary
 * destroy_tlbpid	returns a tlbpid to the pool of available ones
 */

assign_tlbpid(map)
struct pmap *map;
{
	register int pid;

	TRACE({printf("pmap:assign_tlbpid x%x --> ", map);})
#if	COUNTERS
	tpa_count++;
#endif	/*COUNTERS*/
	if (map->pid < 0) {

		if (pids_in_use[next_pid]) {
			/* are we _really_ sure it's full ? */
			for (pid = 1; pid < MAX_PID; pid++)
				if (pids_in_use[pid] == PMAP_NULL) {
					/* aha! */
					next_pid = pid;
					goto got_a_free_one;
				}
			/* Table full */
			if (active_pmap->pid == next_pid) {
				if (++next_pid == MAX_PID)
					next_pid = 1;
			}
			destroy_tlbpid(next_pid, TRUE);
		}
got_a_free_one:
		pids_in_use[next_pid] = map;
		map->pid = next_pid;
		if (++next_pid == MAX_PID)
			next_pid = 1;
	}
	TRACE({printf(" %d \n", map->pid);})
}

int tlbpid_recycle_fifo = 0;
int tlbpid_flushes = 0;

destroy_tlbpid(pid, flush)
int pid;
boolean_t flush;
{
	struct pmap    *map;

	/*
	 * NOTE:  We must get here with interrupts off
	 */
	TRACE({printf("pmap:destroy_tlbpid %d\n", pid);})
#if	COUNTERS
	tpf_count++;
#endif	/*COUNTERS*/

	if (pid < 0)	/* not in use anymore */
		return;

	/*
	 * Flush the tlb if necessary.
	 */
	if (flush || tlbpid_flushes)
		tlb_flush_pid(pid,0,TLB_SIZE);

	/*
	 * Make the pid available, and the map unassigned.
	 */
	map = pids_in_use[pid];
	pids_in_use[pid] = PMAP_NULL;
	if (tlbpid_recycle_fifo)
		next_pid = pid;
	map->pid = -1;
}

