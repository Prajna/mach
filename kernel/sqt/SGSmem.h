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
 * $Log:	SGSmem.h,v $
 * Revision 2.3  91/07/31  17:58:18  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  12:50:08  dbg
 * 	Added, from Sequent SYMMETRY sources.
 * 	[91/04/26  14:45:32  dbg]
 * 
 */
/*
 * $Header: SGSmem.h,v 2.3 91/07/31 17:58:18 dbg Exp $
 *
 * SGSmem.h
 *	Second-Generation System memory controller definitions.
 */

/* SGSmem.h
 * Revision 1.1  89/07/19  14:48:50  kak
 * Initial revision
 * 
 */

#ifndef	_SQT_SGSMEM_H_
#define	_SQT_SGSMEM_H_

/*
 * SGS memory EDC Diagnostic Latch bits
 */
#define	    MEM_EDC_DL_MASK	0x7f	/* useful bits */
#define	    MEM_EDC_DL_CBX	0x01	/* Check Bit 0 Diagnostic */
#define	    MEM_EDC_DL_CB0	0x02	/* Check Bit 1 Diagnostic */
#define	    MEM_EDC_DL_CB1	0x04	/* Check Bit 2 Diagnostic */
#define	    MEM_EDC_DL_CB2	0x08	/* Check Bit 3 Diagnostic */
#define	    MEM_EDC_DL_CB4	0x10	/* Check Bit 4 Diagnostic */
#define	    MEM_EDC_DL_CB8	0x20	/* Check Bit 5 Diagnostic */
#define	    MEM_EDC_DL_CB16	0x40	/* Check Bit 6 Diagnostic */

/*
 * Load EDC Diag Latch: Write Only.
 *	A write of any value causes EDCs to latch data from BDPs
 */
#define	MEM_LOAD_EDC	0x30		/* Load EDC Diag Latch */

/*
 * Bank Enable Register: Read/Write.
 *	Bits 0-7 enables banks 0-7.
 *	Resets to zero on "hard resets" (power-up) only.
 */
#define	MEM_BANKS	0x40		/* Bank Enable Register */

/*
 * Config Register: Read/Write.
 *	Resets to zero on "hard resets" (power-up) only.
 */
#define	MEM_CFG	0x50			/* Config Register */
#define	    MEM_CFG_MASK	0xff	/* useful bits */
#define	    MEM_CFG_WIDE_BUS	0x80	/* Wide Bus Enable */
#define	    MEM_CFG_EN_EXT_MODE	0x40	/* Enable Extended Mode */
#define	    MEM_CFG_4MB		0x20	/* Decode for 4MB DRAMs */
#define	    MEM_CFG_4_BANKS	0x10	/* Decode for 4 banks maximum */
#define	    MEM_CFG_SWAP_BANKS	0x08	/* Decode swapping banks 0 & 1 */
#define	    MEM_CFG_INTERLEAVE	0x04	/* Enable Interleave */
#define	    MEM_CFG_INIT_MASK	0x03	/* useful bits */
#define	    MEM_CFG_SCRUB	0x03	/* Scrub always */
#define	    MEM_CFG_LOGGABLE	0x02	/* Scrub if loggable error */
#define	    MEM_CFG_CHECK	0x01	/* Check only */
#define	    MEM_CFG_INIT	0x00	/* Initialize */

/*
 * Start Initialization/Check: Write Only (Address Strobe Only)
 *	A write of any value causes initialization or check to begin,
 *	according to MEM_CFG_INIT_MASK bits.
 */
#define	MEM_START	0x60		/* Start Initialization/Check */

/*
 * Refresh Delay and EDC Control Register: Read/Write.
 *	Resets to zero on "hard resets" (power-up) only.
 */
#define	MEM_CTRL	0x70		/* Refresh Delay Register */
#define	    MEM_CTRL_MASK	0xff	/* useful bits */
#define	    MEM_CTRL_EN_EXP_ID	0x80	/* Enable Expansion ID */
#define	    MEM_CTRL_EDC_COR_EN	0x40	/* enable EDC Correct mode */
#define	    MEM_CTRL_EDC_MODE	0x30	/* EDC Diag Mode mask */
#define	    MEM_CTRL_EDC_MODE_0	0x10	/* EDC Diag Mode bit 0 */
#define	    MEM_CTRL_EDC_MODE_1	0x20	/* EDC Diag Mode bit 1 */
#define	    MEM_CTRL_REFRESH_EN	0x08	/* enable refresh */
#define	    MEM_CTRL_DMASK	0x07	/* delay count useful bits */
#define	    MEM_CTRL_0		0x07	/* no delay */
#define	    MEM_CTRL_1600	0x06	/* 1600-nsec delay */
#define	    MEM_CTRL_3200	0x05	/* 3200-nsec delay */
#define	    MEM_CTRL_4800	0x04	/* 4800-nsec delay */
#define	    MEM_CTRL_6400	0x03	/* 6400-nsec delay */
#define	    MEM_CTRL_8000	0x02	/* 8000-nsec delay */
#define	    MEM_CTRL_9600	0x01	/* 9600-nsec delay */
#define	    MEM_CTRL_11200	0x00	/* 11200-nsec delay */
					/* other values are invalid */
#define	    MEM_CTRL_DELAY(nsec) \
		(MEM_CTRL_DMASK & ~((nsec)/1600))

/* Diagnostic Generate Mode */
#define	MEM_EDC_DIAG_GEN	(MEM_CTRL_EDC_MODE_0)

/* Diagnostic Detect Mode */
#define	MEM_EDC_DIAG_DET	(MEM_CTRL_EDC_MODE_1)

/* Diagnostic Pass Thru Mode */
#define	MEM_EDC_DIAG_THRU	(MEM_CTRL_EDC_MODE_1 | MEM_CTRL_EDC_MODE_0)

/* Diagnostic Correct Mode */
#define	MEM_EDC_DIAG_COR	(MEM_CTRL_EDC_COR_EN | MEM_CTRL_EDC_MODE_1)

/* Initialize Mode */
#define	MEM_EDC_INIT		(MEM_CTRL_EDC_COR_EN | MEM_CTRL_EDC_MODE)


/*
 * Low BDP Register: Read/Write.
 */
#define	MEM_BDP_LO	0x80		/* Low BDP Register */

/*
 * High BDP Register: Read/Write.
 */
#define	MEM_BDP_HI	0x90		/* High BDP Register */

/*
 * BIC: Read/Write.
 */
#define	MEM_BIC	0xa0			/* BIC Register */

/*
 * Status Register: Read Only.
 *	Cleared by writing to MEM_CLR_EDC register.
 */
#define	MEM_STATUS	0xb0		/* Status Register */
#define	    MEM_STATUS_MASK	0x5	/* useful bits */
#define	    MEM_STATUS_RCYCLE	0x4	/* Refresh cycle occurred */
#define	    MEM_STATUS_CYCLE	0x1	/* Init/Check cycle complete */

/*
 * EDC Error Register: Read Only.
 *	Cleared by writing to MEM_CLR_EDC register.
 */
#define	MEM_EDC	0xc0			/* EDC Error Register */
#define	    MEM_EDC_MASK	0x3f	/* useful bits */
#define	    MEM_EDC_HI_MASK	0x38	/* High EDC useful bits */
#define	    MEM_EDC_HI_OV	0x20	/* High EDC overflow */
#define	    MEM_EDC_HI_ME	0x10	/* High EDC multiple-bit error */
#define	    MEM_EDC_HI_SE	0x08	/* High EDC single-bit error */
#define	    MEM_EDC_LO_MASK	0x07	/* Low EDC useful bits */
#define	    MEM_EDC_LO_OV	0x04	/* Low EDC overflow */
#define	    MEM_EDC_LO_ME	0x02	/* Low EDC multiple-bit error */
#define	    MEM_EDC_LO_SE	0x01	/* Low EDC single-bit error */

/*
 * Clear EDC Errors and Status: Write Only.
 *	A write of any value clears MEM_EDC_ERR and MEM_STATUS.
 */
#define	MEM_CLR_EDC	0xd0		/* Clear EDC Errors and Status */

/*
 * EDC Syndrome Registers: Read Only.
 *	Reads the EDCs' Syndrome registers, which are valid
 *	only if an EDC error has been logged.
 */
#define	MEM_SYND_LO	0xe0		/* Low EDC Syndrome */
#define	    MEM_SYND_MASK	0xff	/* useful bits */
#define	    MEM_NORMCY		0x80	/* Logged during a normal cycle */
#define	    MEM_SYND_BITS	0x7f	/* syndrome bits */
#define	MEM_SYND_HI	0xf0		/* High EDC Syndrome */
#define	    MEM_XOVER		0x80	/* Logged during a cross-over cycle */


/*
 *	Following are non-SLIC defines related to mem2 control:
 */

/*
 * Expansion ID register
 *	(read by accessing expansion memory with MEM_CFG_EN_EXP_ID asserted)
 */
#define	MEM_EXP_MASK	0xff		/* useful bits */
#define	MEM_EXP_NONE	0xff		/* Value for no expansion board */
#define	MEM_EXP_BANKS	0x07		/* Number of populated banks */
#define	MEM_EXP_4MB	0x08		/* 4-MBit DRAM (else 1-MBit) */
#define	MEM_EXP_WIDE	0x10		/* Supports wide bus */
#define	MEM_EXP_REV	0xe0		/* Revision level */

/*
 * bits to check against the controller's config prom's
 * list of acceptable expansions.
 */
#define	MEM_EXP_CHECK	(MEM_EXP_WIDE|MEM_EXP_REV)

#define	MEM_EXP_WIDTH(x)	(((x) & MEM_EXP_WIDE)? 64: 32)

/*
 * Expansion control
 *	These bits correspond to the bank select bits when the
 *	expansion memory is accessed with MEM_CFG_EN_EXP_ID
 *	asserted in the memory configuration register.
 */
#define MEM_EXP_CTL_SHIFT       0x2
#define MEM_EXP_CTL_INTERLEAVE  0x1

/*
 * Offset from cd_m_base of memory controllers interleaved
 * as ILEAVE_HIGH.
 */
#define	MEM_ILEAVE_OFFSET	0x20

#endif	/* _SQT_SGSMEM_H_ */
