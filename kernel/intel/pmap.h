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
 * $Log:	pmap.h,v $
 * Revision 2.7  91/12/10  16:32:23  jsb
 * 	Fixes from Intel
 * 	[91/12/10  15:51:47  jsb]
 * 
 * Revision 2.6  91/08/28  11:13:15  jsb
 * 	From Intel SSD: turn off caching for i860 for now.
 * 	[91/08/26  18:31:19  jsb]
 * 
 * Revision 2.5  91/05/14  16:30:44  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/05/08  12:46:54  dbg
 * 	Add volatile declarations.  Load CR3 when switching to kernel
 * 	pmap in PMAP_ACTIVATE_USER.  Fix PMAP_ACTIVATE_KERNEL.
 * 	[91/04/26  14:41:54  dbg]
 * 
 * Revision 2.3  91/02/05  17:20:49  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  18:17:51  mrt]
 * 
 * Revision 2.2  90/12/04  14:50:35  jsb
 * 	First checkin (for intel directory).
 * 	[90/12/03  21:55:40  jsb]
 * 
 * Revision 2.4  90/08/06  15:07:23  rwd
 * 	Remove ldt (not used).
 * 	[90/07/17            dbg]
 * 
 * Revision 2.3  90/06/02  14:48:53  rpd
 * 	Added PMAP_CONTEXT definition.
 * 	[90/06/02            rpd]
 * 
 * Revision 2.2  90/05/03  15:37:16  dbg
 * 	Move page-table definitions into i386/pmap.h.
 * 	[90/04/05            dbg]
 * 
 * 	Define separate Write and User bits in pte instead of protection
 * 	code.
 * 	[90/03/25            dbg]
 * 
 * 	Load dirbase directly from pmap.  Split PMAP_ACTIVATE and
 * 	PMAP_DEACTIVATE into separate user and kernel versions.
 * 	[90/02/08            dbg]
 * 
 * Revision 1.6  89/09/25  12:25:50  rvb
 * 	seg_desc -> fakedesc
 * 	[89/09/23            rvb]
 * 
 * Revision 1.5  89/09/05  20:41:38  jsb
 * 	Added pmap_phys_to_frame definition.
 * 	[89/09/05  18:47:08  jsb]
 * 
 * Revision 1.4  89/03/09  20:03:34  rpd
 * 	More cleanup.
 * 
 * Revision 1.3  89/02/26  12:33:18  gm0w
 * 	Changes for cleanup.
 * 
 * 31-Dec-88  Robert Baron (rvb) at Carnegie-Mellon University
 *	Derived from MACH2.0 vax release.
 *
 * 17-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	MARK_CPU_IDLE and MARK_CPU_ACTIVE must manipulate a separate
 *	cpu_idle set.  The scheduler's cpu_idle indication is NOT
 *	synchronized with these calls.  MARK_CPU_ACTIVE also needs spls.
 *
 */

/*
 *	File:	pmap.h
 *
 *	Authors:  Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1985
 *
 *	Machine-dependent structures for the physical map module.
 */

#ifndef	_PMAP_MACHINE_
#define _PMAP_MACHINE_	1

#ifndef	ASSEMBLER

#include <kern/zalloc.h>
#include <kern/lock.h>
#include <mach/machine/vm_param.h>
#include <mach/vm_statistics.h>
#include <mach/kern_return.h>

/*
 *	Define the generic in terms of the specific
 */

#if	i386
#define	INTEL_PGBYTES		I386_PGBYTES
#define INTEL_PGSHIFT		I386_PGSHIFT
#define	intel_btop(x)		i386_btop(x)
#define	intel_ptob(x)		i386_ptob(x)
#define	intel_round_page(x)	i386_round_page(x)
#define	intel_trunc_page(x)	i386_trunc_page(x)
#define trunc_intel_to_vm(x)	trunc_i386_to_vm(x)
#define round_intel_to_vm(x)	round_i386_to_vm(x)
#define vm_to_intel(x)		vm_to_i386(x)
#endif	i386
#if	i860
#define	INTEL_PGBYTES		I860_PGBYTES
#define INTEL_PGSHIFT		I860_PGSHIFT
#define	intel_btop(x)		i860_btop(x)
#define	intel_ptob(x)		i860_ptob(x)
#define	intel_round_page(x)	i860_round_page(x)
#define	intel_trunc_page(x)	i860_trunc_page(x)
#define trunc_intel_to_vm(x)	trunc_i860_to_vm(x)
#define round_intel_to_vm(x)	round_i860_to_vm(x)
#define vm_to_intel(x)		vm_to_i860(x)
#endif	i860

/*
 *	i386/i486/i860 Page Table Entry
 */

typedef unsigned int	pt_entry_t;
#define PT_ENTRY_NULL	((pt_entry_t *) 0)

#endif	ASSEMBLER

#define INTEL_OFFMASK	0xfff	/* offset within page */
#define PDESHIFT	22	/* page descriptor shift */
#define PDEMASK		0x3ff	/* mask for page descriptor index */
#define PTESHIFT	12	/* page table shift */
#define PTEMASK		0x3ff	/* mask for page table index */

/*
 *	Convert address offset to page descriptor index
 */
#define pdenum(a)	(((a) >> PDESHIFT) & PDEMASK)

/*
 *	Convert page descriptor index to user virtual address
 */
#define pdetova(a)	((vm_offset_t)(a) << PDESHIFT)

/*
 *	Convert address offset to page table index
 */
#define ptenum(a)	(((a) >> PTESHIFT) & PTEMASK)

#define NPTES	(intel_ptob(1)/sizeof(pt_entry_t))
#define NPDES	(intel_ptob(1)/sizeof(pt_entry_t))

/*
 *	Hardware pte bit definitions (to be used directly on the ptes
 *	without using the bit fields).
 */

#if	i860
#define INTEL_PTE_valid		0x00000001
#else
#define INTEL_PTE_VALID		0x00000001
#endif
#define INTEL_PTE_WRITE		0x00000002
#define INTEL_PTE_USER		0x00000004
#define INTEL_PTE_WTHRU		0x00000008
#define INTEL_PTE_NCACHE 	0x00000010
#define INTEL_PTE_REF		0x00000020
#define INTEL_PTE_MOD		0x00000040
#define INTEL_PTE_WIRED		0x00000200
#define INTEL_PTE_PFN		0xfffff000

#if	i860
#if	NOCACHE
#define	INTEL_PTE_VALID		(INTEL_PTE_valid	\
				|INTEL_PTE_WTHRU	\
				|INTEL_PTE_NCACHE	\
				|INTEL_PTE_REF		\
				|INTEL_PTE_MOD		\
				)
#else	NOCACHE
#define	INTEL_PTE_VALID		(INTEL_PTE_valid	\
				|INTEL_PTE_REF		\
				|INTEL_PTE_MOD		\
				)
#endif	NOCACHE
#endif	i860

#define	pa_to_pte(a)		((a) & INTEL_PTE_PFN)
#define	pte_to_pa(p)		((p) & INTEL_PTE_PFN)
#define	pte_increment_pa(p)	((p) += INTEL_OFFMASK+1)

/*
 *	Convert page table entry to kernel virtual address
 */
#define ptetokv(a)	(phystokv(pte_to_pa(a)))

#ifndef	ASSEMBLER
typedef	volatile long	cpu_set;	/* set of CPUs - must be <= 32 */
					/* changed by other processors */

struct pmap {
	pt_entry_t	*dirbase;	/* page directory pointer register */
	int		ref_count;	/* reference count */
	decl_simple_lock_data(,lock)
					/* lock on map */
	struct pmap_statistics	stats;	/* map statistics */
	cpu_set		cpus_using;	/* bitmap of cpus using pmap */
};

typedef struct pmap	*pmap_t;

#define PMAP_NULL	((pmap_t) 0)

#if	i860
/*#define	set_dirbase(dirbase)	flush_and_ctxsw(dirbase)*//*akp*/
#else
#define	set_dirbase(dirbase)	set_cr3(dirbase)
#endif

#if	NCPUS > 1
/*
 *	List of cpus that are actively using mapped memory.  Any
 *	pmap update operation must wait for all cpus in this list.
 *	Update operations must still be queued to cpus not in this
 *	list.
 */
cpu_set		cpus_active;

/*
 *	List of cpus that are idle, but still operating, and will want
 *	to see any kernel pmap updates when they become active.
 */
cpu_set		cpus_idle;

/*
 *	Quick test for pmap update requests.
 */
volatile
boolean_t	cpu_update_needed[NCPUS];

/*
 *	External declarations for PMAP_ACTIVATE.
 */

void		process_pmap_updates();
void		pmap_update_interrupt();
extern	pmap_t	kernel_pmap;

#endif	NCPUS > 1

/*
 *	Machine dependent routines that are used only for i386/i486/i860.
 */

pt_entry_t	*pmap_pte();

/*
 *	Macros for speed.
 */

#if	NCPUS > 1

/*
 *	For multiple CPUS, PMAP_ACTIVATE and PMAP_DEACTIVATE must manage
 *	fields to control TLB invalidation on other CPUS.
 */

#define	PMAP_ACTIVATE_KERNEL(my_cpu)	{				\
									\
	/*								\
	 *	Let pmap updates proceed while we wait for this pmap.	\
	 */								\
	i_bit_clear((my_cpu), &cpus_active);				\
									\
	/*								\
	 *	Lock the pmap to put this cpu in its active set.	\
	 *	Wait for updates here.					\
	 */								\
	simple_lock(&kernel_pmap->lock);				\
									\
	/*								\
	 *	Process invalidate requests for the kernel pmap.	\
	 */								\
	if (cpu_update_needed[(my_cpu)])				\
	    process_pmap_updates(kernel_pmap);				\
									\
	/*								\
	 *	Mark that this cpu is using the pmap.			\
	 */								\
	i_bit_set((my_cpu), &kernel_pmap->cpus_using);			\
									\
	/*								\
	 *	Mark this cpu active - IPL will be lowered by		\
	 *	load_context().						\
	 */								\
	i_bit_set((my_cpu), &cpus_active);				\
									\
	simple_unlock(&kernel_pmap->lock);				\
}

#define	PMAP_DEACTIVATE_KERNEL(my_cpu)	{				\
	/*								\
	 *	Mark pmap no longer in use by this cpu even if		\
	 *	pmap is locked against updates.				\
	 */								\
	i_bit_clear((my_cpu), &kernel_pmap->cpus_using);		\
}

#define PMAP_ACTIVATE_USER(pmap, th, my_cpu)	{			\
	register pmap_t		tpmap = (pmap);				\
									\
	if (tpmap == kernel_pmap) {					\
	    /*								\
	     *	If this is the kernel pmap, switch to its page tables.	\
	     */								\
	    set_dirbase(kvtophys(tpmap->dirbase));			\
	}								\
	else {								\
	    /*								\
	     *	Let pmap updates proceed while we wait for this pmap.	\
	     */								\
	    i_bit_clear((my_cpu), &cpus_active);			\
									\
	    /*								\
	     *	Lock the pmap to put this cpu in its active set.	\
	     *	Wait for updates here.					\
	     */								\
	    simple_lock(&tpmap->lock);					\
									\
	    /*								\
	     *	No need to invalidate the TLB - the entire user pmap	\
	     *	will be invalidated by reloading dirbase.		\
	     */								\
	    set_dirbase(kvtophys(tpmap->dirbase));			\
									\
	    /*								\
	     *	Mark that this cpu is using the pmap.			\
	     */								\
	    i_bit_set((my_cpu), &tpmap->cpus_using);			\
									\
	    /*								\
	     *	Mark this cpu active - IPL will be lowered by		\
	     *	load_context().						\
	     */								\
	    i_bit_set((my_cpu), &cpus_active);				\
									\
	    simple_unlock(&tpmap->lock);				\
	}								\
}

#define PMAP_DEACTIVATE_USER(pmap, thread, my_cpu)	{		\
	register pmap_t		tpmap = (pmap);				\
									\
	/*								\
	 *	Do nothing if this is the kernel pmap.			\
	 */								\
	if (tpmap != kernel_pmap) {					\
	    /*								\
	     *	Mark pmap no longer in use by this cpu even if		\
	     *	pmap is locked against updates.				\
	     */								\
	    i_bit_clear((my_cpu), &(pmap)->cpus_using);			\
	}								\
}

#define MARK_CPU_IDLE(my_cpu)	{					\
	/*								\
	 *	Mark this cpu idle, and remove it from the active set,	\
	 *	since it is not actively using any pmap.  Signal_cpus	\
	 *	will notice that it is idle, and avoid signaling it,	\
	 *	but will queue the update request for when the cpu	\
	 *	becomes active.						\
	 */								\
	int	s = splvm();						\
	i_bit_set((my_cpu), &cpus_idle);				\
	i_bit_clear((my_cpu), &cpus_active);				\
	splx(s);							\
}

#define MARK_CPU_ACTIVE(my_cpu)	{					\
									\
	int	s = splvm();						\
	/*								\
	 *	If a kernel_pmap update was requested while this cpu	\
	 *	was idle, process it as if we got the interrupt.	\
	 *	Before doing so, remove this cpu from the idle set.	\
	 *	Since we do not grab any pmap locks while we flush	\
	 *	our TLB, another cpu may start an update operation	\
	 *	before we finish.  Removing this cpu from the idle	\
	 *	set assures that we will receive another update		\
	 *	interrupt if this happens.				\
	 */								\
	i_bit_clear((my_cpu), &cpus_idle);				\
									\
	if (cpu_update_needed[(my_cpu)])				\
	    pmap_update_interrupt();					\
									\
	/*								\
	 *	Mark that this cpu is now active.			\
	 */								\
	i_bit_set((my_cpu), &cpus_active);				\
	splx(s);							\
}

#else	NCPUS > 1

/*
 *	With only one CPU, we just have to indicate whether the pmap is
 *	in use.
 */

#define	PMAP_ACTIVATE_KERNEL(my_cpu)	{				\
	kernel_pmap->cpus_using = TRUE;					\
}

#define	PMAP_DEACTIVATE_KERNEL(my_cpu)	{				\
	kernel_pmap->cpus_using = FALSE;				\
}

#define	PMAP_ACTIVATE_USER(pmap, th, my_cpu)	{			\
	register pmap_t		tpmap = (pmap);				\
									\
	set_dirbase(kvtophys(tpmap->dirbase));				\
	if (tpmap != kernel_pmap) {					\
	    tpmap->cpus_using = TRUE;					\
	}								\
}

#define PMAP_DEACTIVATE_USER(pmap, thread, cpu)	{			\
	if ((pmap) != kernel_pmap)					\
	    (pmap)->cpus_using = FALSE;					\
}

#endif	NCPUS > 1

#define PMAP_CONTEXT(pmap, thread)

#define	pmap_kernel()			(kernel_pmap)
#define pmap_resident_count(pmap)	((pmap)->stats.resident_count)
#define pmap_phys_address(frame)	((vm_offset_t) (intel_ptob(frame)))
#define pmap_phys_to_frame(phys)	((int) (intel_btop(phys)))
#define	pmap_copy(dst_pmap,src_pmap,dst_addr,len,src_addr)
#define	pmap_attribute(pmap,addr,size,attr,value) \
					(KERN_INVALID_ADDRESS)

#endif	ASSEMBLER

#endif	_PMAP_MACHINE_
