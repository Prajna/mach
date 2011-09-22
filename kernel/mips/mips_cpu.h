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
 * $Log:	mips_cpu.h,v $
 * Revision 2.5  91/05/14  17:35:37  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:49:27  mrt
 * 	Added author notices
 * 	[91/02/04  11:23:28  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:27:12  mrt]
 * 
 * Revision 2.3  89/12/08  19:48:00  rwd
 * 	Typo in tlblo format.
 * 	[89/12/05  02:21:05  af]
 * 
 * Revision 2.2  89/11/29  14:14:33  af
 * 	Need to shift KUo too.
 * 	[89/11/03  16:35:09  af]
 * 
 * 	Created.
 * 	[89/10/05            af]
 */
/*
 *	File: mips_cpu.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Definitions for the MIPS cpu chipset:
 *		mmu, tlb, exceptions, fpa
 *
 *	Reference: G. Kane "MIPS RISC Architecture", Prentice Hall
 */

/*
 *	The MIPS address space is divided into four segments:
 *
 *		kuseg:	user virtual space
 *		k0seg:	kernel space, directly mapped, cached
 *		k1seg:	kernel space, directly mapped, uncached
 *		k2seg:	kernel virtual space
 *
 */
#define	KUSEG_BASE		0
#define	KUSEG_SIZE		0x80000000
#define	K0SEG_BASE		0x80000000
#define	K0SEG_SIZE		0x20000000
#define	K1SEG_BASE		0xa0000000
#define	K1SEG_SIZE		0x20000000
#define	K2SEG_BASE		0xc0000000
#define	K2SEG_SIZE		0x40000000

#define	ISA_KUSEG(x)		((unsigned)(x)  <  K0SEG_BASE)
#define	ISA_K0SEG(x)		(((unsigned)(x) >= K0SEG_BASE) && \
				 ((unsigned)(x) <  K1SEG_BASE))
#define	ISA_K1SEG(x)		(((unsigned)(x) >= K1SEG_BASE) && \
				 ((unsigned)(x) <  K2SEG_BASE))
#define	ISA_K2SEG(x)		((unsigned)(x)  >= K2SEG_BASE)

/*
 *	The first two kernel segments are directly mapped to
 *	physical memory, starting at physical address 0.
 *	
 */
#define	K0SEG_TO_PHYS(x)	((unsigned)(x) & 0x1fffffff)
#define	PHYS_TO_K0SEG(x)	((unsigned)(x) | K0SEG_BASE)

#define	K1SEG_TO_PHYS(x)	((unsigned)(x) & 0x1fffffff)
#define	PHYS_TO_K1SEG(x)	((unsigned)(x) | K1SEG_BASE)

#define	K0SEG_TO_K1SEG(x)	((unsigned)(x) | K1SEG_BASE)
#define	K1SEG_TO_K0SEG(x)	((unsigned)(x) & 0x9fffffff)

/*
 *
 *	Coprocessor "0" definitions: the TLB
 *	
 *	The 64 entries in the TLB are divided in two parts,
 *	the first one is not accessed by a tlbwr instruction.
 */

#define	TLB_BASE		0
#define	TLB_SIZE		64
#define TLB_SAFE_BASE		0
#define	TLB_SAFE_SIZE		8
#define	TLB_USER_BASE		8
#define	TLB_USER_SIZE		56


/*
 *	Coprocessor "0" registers:
 *		TLB: 	EntryHi, EntryLo, Index, Random, Context, BadVaddr
 *		Misc:	Status, Cause, EPC, PrId
 *		
 */

#define	c0_tlbhi		$10		/* EntryHi Register */
#define	TLB_HI_VPN_MASK		0xfffff000	/* 20 high bits */
#define	TLB_HI_VPN_SHIFT	12
#define	TLB_HI_PID_MASK		0xfc0		/* 6 bits */
#define	TLB_HI_PID_SHIFT	6
#define	TLB_HI_NPID		64		/* tlb context ids */
#define TLB_HI_FMT		"\20\40\15VPN=\14\7PID="


#define	c0_tlblo		$2		/* Entry Lo Register */
#define	TLB_LO_PFN_MASK		0xfffff000	/* 20 high bits */
#define	TLB_LO_PFN_SHIFT	12
#define	TLB_LO_N		0x800		/* No cache */
#define	TLB_LO_D		0x400		/* Dirty */
#define	TLB_LO_V		0x200		/* Valid */
#define	TLB_LO_G		0x100		/* Global */
#define	TLB_LO_FMT		"\20\40\15PFN=\14N\13M\12V\11G"


#define	c0_tlbind		$0		/* Index Register */
#define	TLB_IND_P		0x80000000	/* Probe */
#define	TLB_IND_INX_MASK	0x00003f00	/* Index */
#define	TLB_IND_INX_SHIFT	8
#define	TLB_IND_FMT		"\20\40P\16\11\INX="


#define c0_tlbrnd		$1		/* Random Register */
#define	TLB_RND_MASK		0x00003f00	/* Index */
#define	TLB_RND_SHIFT		8
#define TLB_RND_FMT		"\20\16\11\RND="


#define c0_tlbcxt		$4		/* Context Register */
#define	TLB_CXT_PTE_MASK	0xffe00000	/* PTE base */
#define	TLB_CXT_PTE_SHIFT	21
#define	TLB_CXT_VPN_MASK	0x001ffffc	/* Failed VPN */
#define	TLB_CXT_VPN_SHIFT	2
#define TLB_CXT_FMT		"\20\40\26BASE=\25\1VPN="


#define	c0_tlbbad		$8		/* BadVAddr Register */


#define	c0_status		$12		/* Status Register */
#define	SR_CU_MASK		0xf0000000	/* Coproc Usable */
#	define	SR_CU3		0x80000000	/* Coprocessor 3 usable */
#	define	SR_CU2		0x40000000	/* Coprocessor 2 usable */
#	define	SR_CU1		0x20000000	/* Coprocessor 1 usable */
#	define	SR_CU0		0x10000000	/* Coprocessor 0 usable */
#define	SR_BEV			0x00400000	/* Boot Exception Vectors */
#define	SR_TS			0x00200000	/* TLB shutdown */
#define	SR_PE			0x00100000	/* Parity Error (cache) */
#define	SR_CM			0x00080000	/* Cache Miss */
#define	SR_PZ			0x00040000	/* Parity Zero (cache) */
#define	SR_SwC			0x00020000	/* Swap Caches */
#define	SR_IsC			0x00010000	/* Isolate (data) Cache */
#define	SR_INT_MASK		0x0000ff00	/* Interrupt Mask (1=enable)*/
#	define	INT_LEV8	0x00000000	/* Interrupt level 8 */
#	define	INT_LEV7	0x00008000	/* Interrupt level 7 */
#	define	INT_LEV6	0x0000c000	/* Interrupt level 6 */
#	define	INT_LEV5	0x0000e000	/* Interrupt level 5 */
#	define	INT_LEV4	0x0000f000	/* Interrupt level 4 */
#	define	INT_LEV3	0x0000f800	/* Interrupt level 3 */
#	define	INT_LEV2	0x0000fc00	/* Interrupt level 2 */
#	define	INT_LEV1	0x0000fe00	/* Interrupt level 1 */
#	define	INT_LEV0	0x0000ff00	/* Interrupt level 0 */
#define	SR_KUo			0x00000020	/* old  mode (1 => user) */
#define	SR_KUp			0x00000008	/* previous mode */
#define		SR_KUo_SHIFT	5
#define		SR_KUp_SHIFT	3
#define	SR_KUc			0x00000002	/* currrent mode */
#define	SR_IEo			0x00000010	/* old IntEn (1 => enable) */
#define	SR_IEp			0x00000004	/* previous IntEn */
#define	SR_IEc			0x00000001	/* Current IntEn */
#define	SR_FMT			"\20\40CU3\37CU2\36CU1\35CU0\27BV\26TS\25PE\24CM\23PZ\22SwC\21IsC\20EN7\17EN6\16EN5\15EN4\14EN3\13EN2\12EN1\11EN0\6KUo\5IEo\4KUp\3IEp\2KUc\1IEc"


#define	c0_cause		$13		/* Cause Register */
#define	CAUSE_BD		0x80000000	/* Branch Delay */
#define	CAUSE_CE_MASK		0x30000000	/* Coprocessor Error */
#define	CAUSE_CE_SHIFT		28
#define	CAUSE_IP_MASK		0x0000ff00	/* Interrupts Pending */
#define	CAUSE_IP_SHIFT		8
#	define	IP_NLEV		8		/* No. of Interrupt Levels */
#	define	IP_LEV7		0x00008000	/* Hardware level 7 pending */
#	define	IP_LEV6		0x00004000	/* Hardware level 6 pending */
#	define	IP_LEV5		0x00002000	/* Hardware level 5 pending */
#	define	IP_LEV4		0x00001000	/* Hardware level 4 pending */
#	define	IP_LEV3		0x00000800	/* Hardware level 3 pending */
#	define	IP_LEV2		0x00000400	/* Hardware level 2 pending */
#	define	IP_LEV1		0x00000200	/* Software level 1 pending */
#	define	IP_LEV0		0x00000100	/* Software level 0 pending */
#define	CAUSE_EXC_MASK		0x0000003c	/* Exc Code */
#define	CAUSE_EXC_SHIFT		2
#	define	EXC_INT		0x00000000	/* Interrupt */
#	define	EXC_MOD		0x00000004	/* TLB mod */
#	define	EXC_TLBL	0x00000008	/* TLB Miss (Load) */
#	define	EXC_TLBS	0x0000000c	/* TLB Miss (Store) */
#	define	EXC_ADEL	0x00000010	/* Address Error (Load) */
#	define	EXC_ADES	0x00000014	/* Address Error (Store) */
#	define	EXC_IBE		0x00000018	/* Bus Error (Instruction) */
#	define	EXC_DBE		0x0000001c	/* Bus Error (Data) */
#	define	EXC_SYS		0x00000020	/* Syscall */
#	define	EXC_BP		0x00000024	/* Breakpoint */
#	define	EXC_RI		0x00000028	/* Reserved Instruction */
#	define	EXC_CU		0x0000002c	/* Coprocessor Unusable */
#	define	EXC_OVF		0x00000030	/* Arithmetic Overflow */
				/* 34-38-3c	   Reserved */
#define	CAUSE_FMT		"\20\40BD\36\35COP=\20IP7\17IP6\16IP5\15IP4\14IP3\13IP2\12SW1\11SW0\6\1EXC="


#define	c0_epc			$14		/* ExceptionPC Register */


#define	c0_prid			$15		/* Processor Id Register */

/*
 *	Exception Vector Locations
 *
 *	Machine traps are vectored at three addresses,
 *	two of them depend on the state of the BEV bit
 *	in the Status Register (e.g. they get mapped to
 *	prom locations during reset)
 */
#define	VEC_TLB_UMISS		K0SEG_BASE
#define	VEC_EXCEPTION		(K0SEG_BASE+0x80)
#define	VEC_RESET		(K1SEG_BASE+0x1fc00000)
#define	VEC_BEV_TLB_UMISS	(K1SEG_BASE+0x1fc00100)
#define	VEC_BEV_EXCEPTION	(K1SEG_BASE+0x1fc00180)


/*
 *
 *	Coprocessor "1" Definitions (Floating Point Accelerator)
 *
 */

#define	fpa_irr			$0		/* FPA Impl&Rev Register */
#define	FPA_IRR_IMP		0x0000ff00	/* Implementation */
#define	FPA_IRR_REV		0x000000ff	/* Revision */
#define FPA_IRR_FMT		"\20\20\11IMP=\10\1REV="

#define	fpa_eir			$30		/* FPA Exc Inst Register */

#define fpa_csr			$31		/* FPA Status Register */
#define	FPA_CSR_C		0x00800000	/* Condition Code */
#define FPA_CSR_EE		0x00020000	/* Exc fatal Error */
#define FPA_CSR_EV		0x00010000	/* Exc Invalid */
#define FPA_CSR_EZ		0x00008000	/* Exc Zerodivide */
#define FPA_CSR_EO		0x00004000	/* Exc Overflow */
#define FPA_CSR_EU		0x00002000	/* Exc Underflow */
#define FPA_CSR_EI		0x00001000	/* Exc Inexact */
#define FPA_CSR_TV		0x00000800	/* Trap Invalid */
#define FPA_CSR_TZ		0x00000400	/* Trap Zerodivide */
#define FPA_CSR_TO		0x00000200	/* Trap Overflow */
#define FPA_CSR_TU		0x00000100	/* Trap Underflow */
#define FPA_CSR_TI		0x00000080	/* Trap Inexact */
#define FPA_CSR_SV		0x00000040	/* State Invalid */
#define FPA_CSR_SZ		0x00000020	/* State Zerodivide */
#define FPA_CSR_SO		0x00000010	/* State Overflow */
#define FPA_CSR_SU		0x00000008	/* State Underflow */
#define FPA_CSR_SI		0x00000004	/* State Inexact */
#define FPA_CSR_RM		0x00000003	/* Rounding Mode */
#	define FPA_CSR_RM_RN	0x00		/* round to nearest */
#	define FPA_CSR_RM_RZ	0x01		/* round to zero */
#	define FPA_CSR_RM_RP	0x02		/* round to plus inf */
#	define FPA_CSR_RM_RM	0x03		/* round to minus inf */
#define	FPA_CSR_FMT		"\20\30C\22EE\21EV\20EZ\17EO\16EU\15EI\14TV\13TZ\12TO\11TU\10TI\7SV\6SZ\5SO\4SU\3SI\2\1RM="
