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
 * $Log:	tlb.s,v $
 * Revision 2.11  92/02/19  15:09:42  elf
 * 	Changed #-style comments, for ANSI cpp.
 * 	[92/02/19  13:11:31  rpd]
 * 
 * Revision 2.10  91/05/14  17:39:11  mrt
 * 	Correcting copyright
 * 
 * Revision 2.9  91/02/05  17:52:11  mrt
 * 	Added author notices
 * 	[91/02/04  11:25:28  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:30:12  mrt]
 * 
 * Revision 2.8  91/01/08  15:52:02  rpd
 * 	Removed tlb_probe_and_wire.
 * 	[90/11/12            rpd]
 * 
 * Revision 2.7  90/12/05  23:39:00  af
 * 
 * 
 * Revision 2.6  90/12/05  20:50:16  af
 * 	Save one cycle in tlb_umiss(), idea from dbg.
 * 	[90/12/03            af]
 * 
 * Revision 2.5  90/09/28  16:57:17  jsb
 * 	Picked up Larry Allen's fix to tlb_flush_pid.
 * 	[90/09/22            rpd]
 * 
 * Revision 2.4  90/01/22  23:08:34  af
 * 	Consolidated conditionals.
 * 	[90/01/20  17:10:24  af]
 * 
 * 	Fixed ref bits to avoid I/O pages.
 * 	[89/12/09  11:04:51  af]
 * 
 * 	Added bookeepings for reference bits.  Works, but consider
 * 	it experimental for now.
 * 	[89/12/05  02:19:17  af]
 * 
 * Revision 2.3  89/12/08  19:48:37  rwd
 * 	Added bookeepings for reference bits.  Works, but consider
 * 	it experimental for now.
 * 	[89/12/05  02:19:17  af]
 * 
 * Revision 2.2  89/11/29  14:15:36  af
 * 	Use assembly mnemonics wherever possible.
 * 	Fixed KDB support functions.
 * 	[89/11/03  16:34:04  af]
 * 
 * 	Upgraded for pure kernel.
 * 	Added tlb peek/poke functions for KDB's use.
 * 	[89/10/12            af]
 * 
 * Revision 2.3  89/08/28  22:39:52  af
 * 	Optimized and polished.  Made all functions names prefixed by
 * 	this module's name.  Compacted mod/unmodtlb into tlb_modify.
 * 	Removed debugging code.
 * 	[89/08/06            af]
 * 
 * Revision 2.2  89/07/14  15:29:07  rvb
 * 	Cleanup.
 * 	[89/07/14            rvb]
 * 
 * Revision 2.1  89/05/30  12:56:21  rvb
 * rcs'ed.
 * 
 * 10-Feb-89  Alessandro Forin (af) at Carnegie-Mellon University
 *	Created.
 */
/*
 *	File: tlb.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	2/89
 *	
 *	Assembly functions to manipulate the mips TLB.
 *
 */

#include <ref_bits.h>
#include <mach_kdb.h>

#include <mach/mips/asm.h>
#include <mach/mips/mips_instruction.h>

#include <mips/mips_cpu.h>
#if	REF_BITS
#include <mips/pmap.h>
	ABS(pmap_reference_bits, K0SEG_BASE + 0x20000)
#endif	REF_BITS

#include <assym.s>

	.set	noreorder

/*
 *	Object:
 *		tlb_probe_entry			LOCAL macro
 *
 *		Common code to probe for an entry in the tlb
 *
 *	Arguments:
 *		pid				register
 *		addr				register
 *		oldpid				register
 *		entry				register
 *		index				register
 *
 *	Preserves the current pid in 'oldpid', returns the 'index'
 *	of the mapping for the pair ('pid','addr'). If none is
 *	found returns a negative value.
 *	As a byproduct, leaves the appropriate mapping entry
 *	ready for the tlbhi register in 'entry'.
 *	Note that the macro needs a delay cycle to
 *	complete and make 'index' valid.
 */
#define tlb_probe_entry(pid,addr,oldpid,entry,index)	\
	mfc0	oldpid,c0_tlbhi;	/* save current tlb pid */	\
	and	addr,TLB_HI_VPN_MASK;	/* drop offset from address */	\
	sll	pid,TLB_HI_PID_SHIFT;	/* shift pid in place */	\
	or	entry,addr,pid;		/* tlbhi value to look for */	\
	mtc0	entry,c0_tlbhi;		/* put args into tlbhi */	\
	nop;				/* let tlbhi get through pipe */\
	tlbp;				/* probe for entry */		\
	nop;				/* pipeline */			\
	nop;				/* pipeline */			\
	mfc0	index,c0_tlbind;		/* see what happened */


/*
 *	Object:
 *		tlb_probe			EXPORTED function
 *
 *		Probe the tlb for a valid mapping of (pid,vaddr)
 *
 *	Arguments:
 *		pid				tlbpid_t
 *		vaddr				vm_offset_t
 *
 *	Returns 1 if the tuple is mapped, else 0
 */
LEAF(tlb_probe)
	mfc0	t0,c0_status		/* save SR and disable interrupts */
	mtc0	zero,c0_status

	tlb_probe_entry(a0,a1,t1,a0,a0)

	nop
	bltz	a0,1f			/* found any ? */
	move	v0,zero			/* assume not */
	li	v0,1
1:	mtc0	t1,c0_tlbhi		/* restore old tlbpid */
	j	ra
	mtc0	t0,c0_status		/* restore sr and return */
	END(tlb_probe)


/*
 *	Object:
 *		tlb_zero			EXPORTED function
 *
 *		Invalidate the i-th TLB entry.
 *
 *	Arguments:
 *		i				unsigned
 *
 *	Puts some special neutral mapping at entry i.
 *	Uses a virtual address in the k1seg, which will
 *	not be looked for translation. Uses zero for
 *	the physical page, for cosmetic reasons.
 */
LEAF(tlb_zero)
	mfc0	t0,c0_status		/* save SR and disable interrupts */
	mtc0	zero,c0_status
	li	t2,K1SEG_BASE&TLB_HI_VPN_MASK	/* neutral vaddr */
	mfc0	t1,c0_tlbhi		/* save current tlbpid */
	sll	a0,TLB_IND_INX_SHIFT	/* shift pid in place */
	mtc0	t2,c0_tlbhi		/* set VPN and TLBPID */
	mtc0	zero,c0_tlblo		/* set PPN and protection */
	mtc0	a0,c0_tlbind		/* set INDEX */
	nop
	tlbwi				/* drop it in */
	nop
	mtc0	t1,c0_tlbhi		/* restore old tlbpid */
	j	ra
	mtc0	t0,c0_status		/* restore SR and return */
	END(tlb_zero)

/*
 *	Object:
 *		tlb_map				EXPORTED function
 *
 *		Insert a mapping in a tlb line
 *
 *	Arguments:
 *		line				unsigned
 *		pid				tlbpid_t
 *		vaddr				vm_offset_t
 *		pte				pt_entry_t
 *
 *	Builds a tlb entry for (pid,vaddr,pte) and inserts
 *	it at the given tlb line. Valid lines are indexed
 *	from 0 to TLB_SIZE, but validity is not checked for.
 *	Address does not need to be rounded.
 *	Note that no check is made against conflicting mappings.
 */
LEAF(tlb_map)
	mfc0	t0,c0_status		/* save SR and disable interrupts */
	mtc0	zero,c0_status
	sll	a0,TLB_IND_INX_SHIFT	/* shift line index in place */
	mfc0	t1,c0_tlbhi		/* save current tlbpid */
	sll	a1,TLB_HI_PID_SHIFT	/* line up pid bits */
	and	a2,TLB_HI_VPN_MASK	/* chop offset bits */
	or	a1,a2
	mtc0	a1,c0_tlbhi		/* set VPN and TLBPID */
	mtc0	a3,c0_tlblo		/* set PPN and access bits */
	mtc0	a0,c0_tlbind		/* set INDEX to wired entry */
#if	REF_BITS
	lui	a1,0xf000		/* I/O page ? */
	tlbwi				/* drop it in */
	and	a1,a3
	bne	a1,zero,1f
	li	a1,pmap_reference_bits
	srl	a3,VA_PAGEOFF
	addu	a1,a3			/* reference bit address */
	sb	zero,0(a1)		/* set ref bit */
1:
#else	REF_BITS
	nop
	tlbwi				/* drop it in */
	nop
#endif	REF_BITS
	mtc0	t1,c0_tlbhi		/* restore old tlbpid */
	j	ra
	mtc0	t0,c0_status		/* restore SR and return */
	END(tlbwired)

/*
 *	Object:
 *		tlb_map_random			EXPORTED function
 *
 *		Insert a mapping someplace in the tlb
 *
 *	Arguments:
 *		pid				tlbpid_t
 *		vaddr				vm_offset_t
 *		pte				pt_eptry_t
 *
 *	Like tlb_map(), but uses one of the lines indexed by
 *	the random register (randomly :-).
 *	This one also checks against existing mapping
 *	that would cause conflict and overrides them.
 *	Returns whether this happened (1) or not (0).
 */
LEAF(tlb_map_random)
	mfc0	t0,c0_status		/* save SR and disable interrupts */
	mtc0	zero,c0_status

	tlb_probe_entry(a0,a1,t1,t2,t3)

	move	v0,zero			/* assume no conflicts */
	bltz	t3,1f			/* not found */
	mtc0	a2,c0_tlblo		/* pte for new entry (BDSLOT) */
	nop
	tlbwi				/* re-use line */
	b	2f
	li	v0,1			/* overridden */
1:
#if	REF_BITS
	lui	a1,0xf000		/* I/O page ? */
	tlbwr				/* use random slot */
	and	a1,a2
	bne	a1,zero,2f
	li	a1,pmap_reference_bits
	srl	a2,VA_PAGEOFF
	addu	a1,a2			/* reference bit address */
	sb	zero,0(a1)		/* set ref bit (no misses!) */
#else	REF_BITS
	nop
	tlbwr				/* use random slot */
	nop
#endif	REF_BITS
2:	mtc0	t1,c0_tlbhi		/* restore tlbpid */
	j	ra
	mtc0	t0,c0_status		/* restore SR and return */
	END(tlb_map_random)


/*
 *	Object:
 *		tlb_modify			EXPORTED function
 *
 *		Set or clear the writeable bit of an entry
 *
 *	Arguments:
 *		pid				tlbpid_t
 *		vaddr				vm_offset_t
 *		writeable			boolean_t
 *
 *	Probes first for a mapping, if found changes it
 *	to be writeable or not as requested.
 *	[Protection is independent of user/kernel mode.]
 */

LEAF(tlb_modify)
	mfc0	t0,c0_status		/* save SR and disable interrupts */
	mtc0	zero,c0_status

	tlb_probe_entry(a0,a1,t1,t2,t3)

	nop
	bltz	t3,2f
	move	v0,zero			/* assume probe failed (BDSLOT) */
	tlbr				/* load entry in TLBLO/TLBHI */
	nop
	nop
	mfc0	t2,c0_tlblo
	bne	a2,zero,1f		/* make it writeable ? */
	or	t2,TLB_LO_D		/* set writeable bit (BDSLOT) */
	and	t2,~TLB_LO_D		/* clear writeable bit */
1:	mtc0	t2,c0_tlblo		/* change entry */
	nop
	tlbwi
	nop
2:	mtc0	t1,c0_tlbhi		/* restore old tlbpid */
	j	ra
	mtc0	t0,c0_status		/* restore SR and return */
	END(tlb_modify)

/*
 *	Object:
 *		tlb_unmap			EXPORTED function
 *
 *		Remove the mapping for address vaddr with id pid.
 *
 *	Arguments:
 *		pid				tlbpid_t
 *		vaddr				vm_offset_t
 *
 *	Probes to see if there is any such mapping.  Note that
 *	interrupts must be disabled in this and the other
 *	tlb operations since an interrupt could cause
 *	a miss which could alter the content of the 
 *	tlbhi register.
 */
LEAF(tlb_unmap)
	mfc0	t0,c0_status		/* save SR and disable interrupts */
	mtc0	zero,c0_status

	tlb_probe_entry(a0,a1,t1,t2,t3)

	move	v0,zero			/* assume not there */
	bltz	t3,1f			/* probe failed */
	li	t2,K1SEG_BASE&TLB_HI_VPN_MASK	/* invalid vaddr (not 0!) */
	mtc0	t2,c0_tlbhi		/* invalidate entry */
	mtc0	zero,c0_tlblo		/* cosmetic */
	nop
	tlbwi
	nop
1:	mtc0	t1,c0_tlbhi		/* restore old tlbpid */
	j	ra
	mtc0	t0,c0_status		/* restore SR and return */
	END(tlb_unmap)


/*
 *	Object:
 *		tlb_flush			EXPORTED function
 *
 *		Flush all tlb lines in the given range (inclusive)
 *
 *	Arguments:
 *		from_line			unsigned
 *		to_line				unsigned
 *
 *	Zeroes all entries in the given range of lines.
 *	Could be done with tlb_zero, but this is slightly
 *	faster. Besides, one would probably expect this
 *	function anyways.
 */
LEAF(tlb_flush)
	mfc0	t0,c0_status		/* save SR and disable interrupts */
	mtc0	zero,c0_status
	li	t2,K1SEG_BASE&TLB_HI_VPN_MASK	/* set up to invalidate entries */
	mfc0	t1,c0_tlbhi		/* save current tlbpid */
	mtc0	zero,c0_tlblo		/* setup neutral mapping */
	mtc0	t2,c0_tlbhi
	sll	t2,a0,TLB_IND_INX_SHIFT
1:
	mtc0	t2,c0_tlbind		/* set index */
	addu	a0,1			/* bump to next entry */
	tlbwi				/* invalidate */
	bne	a0,a1,1b		/* more to do */
	sll	t2,a0,TLB_IND_INX_SHIFT	/* prepare index (BDSLOT) */

	mtc0	t1,c0_tlbhi		/* restore tlbpid */
	j	ra
	mtc0	t0,c0_status		/* restore SR and return */
	END(flush_tlb)

/*
 *	Object:
 *		tlb_flush_pid			EXPORTED function
 *
 *		Flush all the entries with the given pid
 *
 *	Arguments:
 *		pid				tlbpid_t
 *		from_line			unsigned
 *		to_line				unsigned
 *
 *	Makes sure no entries for the given pid are in the tlb.
 *	Walks through all entries in the given range (inclusive).
 *	
 */
LEAF(tlb_flush_pid)
	mfc0	t0,c0_status		/* save SR and disable interrupts */
	mtc0	zero,c0_status
	li	t2,K1SEG_BASE&TLB_HI_VPN_MASK	/* set up to invalidate entries */
	mfc0	t1,c0_tlbhi		/* save current tlbpid */
	sll	t3,a1,TLB_IND_INX_SHIFT	/* starting index */
	sll	a0,TLB_HI_PID_SHIFT	/* lined up pid to check against */
1:
	mtc0	t3,c0_tlbind		/* set index */
	addu	a1,1			/* bump to next entry */
	tlbr				/* read the entry */
	nop
	nop
	mfc0	t4,c0_tlbhi
	mtc0	zero,c0_tlblo		/* assume a match */
	and	t4,TLB_HI_PID_MASK	/* does the pid match ? */
	bne	t4,a0,2f
	mtc0	t2,c0_tlbhi		/* yep, flush it */
	nop
	tlbwi				/* invalidate */
2:	bne	a1,a2,1b		/* more to do */
	sll	t3,a1,TLB_IND_INX_SHIFT	/* prepare next index (BDSLOT) */

	mtc0	t1,c0_tlbhi		/* restore tlbpid */
	j	ra
	mtc0	t0,c0_status		/* restore SR and return */
	END(flush_tlbpid)

/*
 *	Object:
 *		tlb_set_context			EXPORTED function
 *
 *		Setup the tlb to use the given pmap
 *
 *	Arguments:
 *		pmap				pmap_t
 *
 *	Changes the current TLBPID, sets the CONTEXT register
 *	for use by the tlb_umiss handler.
 *	Used at initialization and context switching.
 */
LEAF(tlb_set_context)
	mfc0	t0,c0_status		/* save SR and disable interrupts */
	mtc0	zero,c0_status
	lw	t2,PMAP_PTEBASE(a0)	/* get page table base */
	lw	t1,PMAP_PID(a0)		/* get new tlbpid */
	mtc0	t2,c0_tlbcxt		/* tlb_umiss is happy now */
	sll	t1,TLB_HI_PID_SHIFT	/* line up pid bits */
	and	t1,TLB_HI_PID_MASK	/* sanity */
	mtc0	t1,c0_tlbhi		/* assert new pid */
	sw	a0,active_pmap		/* assert new pmap */
	j	ra
	mtc0	t0,c0_status		/* restore SR and return */
	END(set_tlb_context)

#if	0
/*
 *	Object:
 *		tlb_probe_and_wire		EXPORTED function
 *
 *		Wire an (existing) tlb entry
 *
 *	Arguments:
 *		a0				<preserved>
 *		entry				tlbhi_t
 *		pte				pt_entry_t
 *		index				unsigned
 *
 *	Probe for 'entry' (a formatted TLBHI value) in the TLB.
 *	If a line exists for it, remove it.
 *	Insert the entry at line index.
 *
 *	Calling sequence
 *		a0 -- 		preserved
 *		a1 -- entry	clobbered
 *		a2 -- pte	clobbered
 *		a3 -- index	clobbered
 *		v0 -- 		clobbered
 *
 * 	Side effects: Changes the TLBPID.
 */
LEAF(tlb_probe_and_wire)
	mtc0	a2,c0_tlbhi		/* probe (vaddr,0) */
	nop
	tlbp
	nop
	nop
	mfc0	v0,c0_tlbind		/* get index */
	nop
	bltz	v0,1f			/* nope, missing */
	nop
	beq	v0,a3,1f		/* already in place ? */
	li	v0,K1SEG_BASE&TLB_HI_VPN_MASK

	mtc0	v0,c0_tlbhi		/* invalidate old one */
	mtc0	zero,c0_tlblo
	nop
	tlbwi
	nop
	nop				/* fall through */

1:	/* Entry not (anymore) in tlb, wire it */
	mtc0	a3,c0_tlbind
	mtc0	a1,c0_tlblo
	mtc0	a2,c0_tlbhi
#if	REF_BITS
	/*
	 *  NOTE: this function is currently only used for stack pages
	 *	  If this changes, beware of I/O pages (see tlb_map())
	 */
	srl	a1,VA_PAGEOFF
	tlbwi
	li	a2,pmap_reference_bits
	addu	a2,a1			/* reference bit address */
	j	ra
	sb	zero,0(a2)		/* set ref bit (no misses!) */
#else	REF_BITS
	nop
	tlbwi
	j	ra
	nop
#endif	REF_BITS
	END(tlb_probe_and_wire)
#endif	0

#if	MACH_KDB
/*
 *	Object:
 *		tlb_read_tlbhi			EXPORTED function
 *
 *		Returns the TLBHI register
 *
 *	Arguments:
 *		none
 *
 */
LEAF(tlb_read_tlbhi)
	mfc0	v0,c0_tlbhi
	j	ra
	nop
	END(tlb_read_tlbhi)

/*
 *	Object:
 *		tlb_read_tlblo			EXPORTED function
 *
 *		Returns the TLBLO register
 *
 *	Arguments:
 *		none
 *
 */
LEAF(tlb_read_tlblo)
	mfc0	v0,c0_tlblo
	j	ra
	nop
	END(tlb_read_tlblo)

/*
 *	Object:
 *		tlb_read_line			EXPORTED function
 *
 *		Read the i-th tlb line
 *
 *	Arguments:
 *		line				unsigned
 *
 *	Side effect: Changes both TLBHI and TLBLO
 */
LEAF(tlb_read_line)
	sll	a0,TLB_IND_INX_SHIFT
	mtc0	a0,c0_tlbind
	nop
	tlbr
	j	ra
	nop
	END(tlb_read_line)

/*
 *	Object:
 *		tlb_write_line			EXPORTED function
 *
 *		Write a tlb line
 *
 *	Arguments:
 *		line				int
 *		hi				tlbhi_t
 *		lo				tlblo_t
 *
 *	Extra special values recognized for 'line':
 *		-1	use the current tlbind value
 *		-2	use a random line
 *		-3	only set tlbhi/lo
 */
LEAF(tlb_write_line)
	bgez	a0,std_write
	li	t0,-1
	beq	a0,t0,2f
	sub	t0,1
	beq	a0,t0,1f
	sub	t0,1
	bne	a0,t0,3f
	nop
	mtc0	a1,c0_tlbhi		/* only set hi/lo */
	j	ra
	mtc0	a2,c0_tlblo
	
1:	mtc0	a1,c0_tlbhi		/* write random line */
	mtc0	a2,c0_tlblo
	b	3f
	tlbwr
std_write:				/* write specific line */
	sll	a0,TLB_IND_INX_SHIFT
	mtc0	a0,c0_tlbind
2:
	mtc0	a1,c0_tlbhi		/* write current line */
	mtc0	a2,c0_tlblo
	nop
	tlbwi
3:	j	ra
	nop
	END(tlb_write_line)

#endif	MACH_KDB
