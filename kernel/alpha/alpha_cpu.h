/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
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
 * $Log:	alpha_cpu.h,v $
 * Revision 2.2  93/02/05  07:57:12  danner
 * 	Added machine-check error register defines.
 * 	[93/02/04  00:42:24  af]
 * 
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:10:40  af]
 * 
 * 	Created.
 * 	[92/06/02            af]
 * 
 */
/*
 *	File: alpha_cpu.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Definitions for the ALPHA Architecture.
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 *	
 *	"DECChip 21064-AA RISC Microprocessor Preliminary Data Sheet"
 *	Apr 92, Digital Equipment Corporation, Maynard MA
 *	Available for anon ftp on the host gatekeeper.dec.com
 */

#ifndef	_ALPHA_ALPHA_CPU_H_
#define	_ALPHA_ALPHA_CPU_H_	1

/*
 * The Alpha virtual address space is logically a flat 64 bit one.
 * Implementations of the architecture so far provide a small help
 * to the OS called "Superpage mapping", which is a predefined
 * virtual-->physical translation (when in kernel) mode that helps
 * avoid TLB misses in critical code sections.
 *
 * We assume this feature WILL be provided in all implementations.
 */

/*
 * The ITB maps va<33:13> --> pa<33:13> when va<42:41> == 2
 * Plus, THERE IS CHECKED SIGN EXTENSION
 */
#define	SUPERPAGE_I_MASK	0x0000060000000000
#define	SUPERPAGE_I_PAGE	0x0000040000000000

#define	SUPERPAGE_I_START	0xfffffc0000000000
#define	SUPERPAGE_I_SIZE	0x0000020000000000


/*
 * The DTB does the same, plus there is a second mapping
 * to pa<33:30>==0 when va<42:30>==1ffe
 */
#define	SUPERPAGE_D_MASK	SUPERPAGE_I_MASK
#define	SUPERPAGE_D_PAGE	SUPERPAGE_I_PAGE
#define	SUPERPAGE_D_START	SUPERPAGE_I_START
#define	SUPERPAGE_D_SIZE	SUPERPAGE_I_SIZE

#define	SUPERPAGE_IO_MASK	0x000007ff80000000
#define	SUPERPAGE_IO_PAGE	SUPERPAGE_IO_MASK
#define	SUPERPAGE_IO_START	0xffffffff80000000
#define	SUPERPAGE_IO_SIZE	0x0000000080000000


/*
 *	We divide the address space into five segments:
 *
 *		kuseg:	user virtual space
 *		k0seg:	kernel space, directly mapped, cached
 *		k2seg:	kernel virtual space
 *		k1seg:	kernel space, directly mapped, uncached
 *		k3seg:	user virtual space
 *		k4seg:	unused (kernel virtual space, uncached)
 *
 *	Current implementations of the chip do not support k3seg.
 *	They actually do sign-extension on addresses, killing it.
 *
 */
#define	KUSEG_BASE		0			/* from ..0000.00. */
#define	KUSEG_SIZE		K2SEG_BASE		/* to   ..03fe.00. */
#define	KUSEG_END		KUSEG_SIZE
#define	K2SEG_BASE		(K3SEG_BASE-K2SEG_SIZE)	/* from ..03fe.00. */
#define	K2SEG_SIZE		0x0000000200000000	/* to   ..0400.00. */
#define	K3SEG_BASE		SUPERPAGE_I_PAGE	/* from ..0400.00. */
#define	K3SEG_SIZE		(K0SEG_BASE-K3SEG_BASE)	/* to   ..fc00.00. */
#define	K0SEG_BASE		SUPERPAGE_I_START	/* from ..fc00.00. */
#define	K0SEG_SIZE		SUPERPAGE_I_SIZE	/* to   ..fe00.00. */
#define	K4SEG_BASE		0xfffffe0000000000	/* from ..fe00.00. */
#define	K4SEG_SIZE		0x000001ff80000000	/* to   ..ffff.80. */
#define	K1SEG_BASE		SUPERPAGE_IO_START	/* from ..ffff.80. */
#define	K1SEG_SIZE		SUPERPAGE_IO_SIZE	/* to   ..ffff.ff. */

/*
 * Predicates.
 * Note: revise this if k3seg
 */
#define	ISA_KUSEG(x)		((vm_offset_t)(x)  <  K2SEG_BASE)
#define	ISA_K0SEG(x)		(((vm_offset_t)(x) >= K0SEG_BASE) && \
				 ((vm_offset_t)(x) <  K4SEG_BASE))
#define	ISA_K1SEG(x)		((vm_offset_t)(x) >= K1SEG_BASE)
#define	ISA_K2SEG(x)		(((vm_offset_t)(x) >= K2SEG_BASE) && \
				 ((vm_offset_t)(x) <  K3SEG_BASE))

/*
 *	Kernel segments 0 and 1 are directly mapped to
 *	physical memory, starting at physical address 0.
 *	
 */
#define	K0SEG_TO_PHYS(x)	((vm_offset_t)(x) & 0x00000003ffffffff)
#define	PHYS_TO_K0SEG(x)	((vm_offset_t)(x) | K0SEG_BASE)

#define	K1SEG_TO_PHYS(x)	K0SEG_TO_PHYS(x)
#define	PHYS_TO_K1SEG(x)	((vm_offset_t)(x) | K1SEG_BASE)

#define	K0SEG_TO_K1SEG(x)	((vm_offset_t)(x) | SUPERPAGE_IO_PAGE)
#define	K1SEG_TO_K0SEG(x)	PHYS_TO_K0SEG(K1SEG_TO_PHYS(x))

/*
 *	Architecturally defined registers
 */

/*
 * Program status word
 */
#define	PS_STACK_ALIGNMENT	0x3f00000000000000	/* in saved PS */
#define	PS_zero			0xc0ffffffffffe060
#define	PS_IPL_MASK		0x0000000000001f00
#define	PS_IPL_SHIFT		8
#define	PS_VIRTUAL_MACHINE	0x0000000000000080
#define	PS_CURRENT_MODE		0x0000000000000018
#define	PS_MODE_SHIFT		3
#define	PS_SOFTWARE		0x0000000000000007

#define	ALPHA_IPL_0		0
#define	ALPHA_IPL_SOFTC		8
#define	ALPHA_IPL_IO		21
#define	ALPHA_IPL_CLOCK		22
#define	ALPHA_IPL_HIGH		23

#define	PS_KERNEL_MODE		0
#define	PS_EXECUTIVE_MODE	1
#define	PS_SUPERVISOR_MODE	2
#define	PS_USER_MODE		3

#define	alpha_user_mode(ps)	\
	((((ps) & PS_CURRENT_MODE) >> PS_MODE_SHIFT) == PS_USER_MODE)

#define	alpha_initial_ps_value	\
	(PS_USER_MODE << PS_MODE_SHIFT)

/*
 *	Floating point control register
 */

#define	FPCR_SUM		0x8000000000000000
#define	FPCR_raz		0x700fffffffffffff
#define	FPCR_DYN_RM		0x0c00000000000000
#define	FPCR_IOV		0x0200000000000000
#define	FPCR_INE		0x0100000000000000
#define	FPCR_INF		0x0080000000000000
#define	FPCR_OVF		0x0040000000000000
#define	FPCR_DZE		0x0020000000000000
#define	FPCR_INV		0x0010000000000000


/*
 *	Machine check error register
 */

#define MCES_MCK		0x1
#define MCES_SCE		0x2
#define MCES_PCE		0x4
#define MCES_DPC		0x8
#define MCES_DSC		0x10


#endif	/* _ALPHA_ALPHA_CPU_H_ */
