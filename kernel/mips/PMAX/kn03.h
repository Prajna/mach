/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
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
 * $Log:	kn03.h,v $
 * Revision 2.2  93/05/30  21:08:36  rvb
 * 	RCS-ed.
 * 	[93/05/29            af]
 * 
 */

/*
 *	File: kn03.h
 * 	Author: John Wroclawski, Massachusetts Institute of Technology
 *	Date:	?/93
 *
 *	Definitions specific to the KN03 processor (DS5000/240)
 *
 */

/*
 *	This machine uses a combination of the memory system
 *	of the 3max and the I/O system from the later
 *	(asic-based) decstations.
 *
 *	This file was constructed from pieces of the various other
 *	decstations definition files and a certain amount of
 *	experimentation.
 */

#ifndef	MIPS_KN03_H
#define	MIPS_KN03_H 1

/*
 * KN03's Physical address space
 */

#define KN03_PHYS_MIN		0x00000000	/* 512 Meg */
#define KN03_PHYS_MAX		0x1fffffff

/*
 * Memory map
 */

#define KN03_PHYS_MEMORY_START	0x00000000
#define KN03_PHYS_MEMORY_END	0x1dffffff	/* 480 Meg in 15 slots */

/*
 * I/O map
 */

#define KN03_PHYS_TC_0_START	0x1e000000	/* TURBOchannel, slot 0 */
#define KN03_PHYS_TC_0_END	0x1e7fffff	/*   8 Meg, option0 */

#define KN03_PHYS_TC_1_START	0x1e800000	/* TURBOchannel, slot 1 */
#define KN03_PHYS_TC_1_END	0x1effffff	/*   8 Meg, option1 */

#define KN03_PHYS_TC_2_START	0x1f000000	/* TURBOchannel, slot 2 */
#define KN03_PHYS_TC_2_END	0x1f7fffff	/*   8 Meg, option2 */

#define KN03_PHYS_TC_3_START	0x1f800000	/* TURBOchannel, slot 3 */
#define KN03_PHYS_TC_3_END	0x1fffffff	/*   8 Meg, internal devices */

#define	KN03_PHYS_TC_START	KN03_PHYS_TC_0_START
#define	KN03_PHYS_TC_END	KN03_PHYS_TC_3_END	/* 32 Meg */

#define KN03_TC_NSLOTS		4
#define	KN03_TC_MIN		0
#define KN03_TC_MAX		2		/* don't look at system slot */

/*
 * Pseudo-TC slots (arg to kn03_enable_interrupt and friends
 */
#define	KN03_SCSI_SLOT		3
#define	KN03_LANCE_SLOT		4
#define	KN03_SCC0_SLOT		5
#define	KN03_SCC1_SLOT		6
#define	KN03_ASIC_SLOT		7

#define KN03_FIRST_PSEUDOSLOT	3
#define KN03_LAST_PSEUDOSLOT	7

/*
 * System module space
 */

#include <mips/PMAX/asic.h>

#define KN03_SYS_ASIC		( KN03_PHYS_TC_3_START + 0x000000 )

#define	KN03_SYS_ROM_START	( KN03_SYS_ASIC + ASIC_SLOT_0_START )
#define	KN03_SYS_ROM_END	( KN03_SYS_ROM_START + 0x03ffff )

#define	KN03_SYS_ASIC_REGS	( KN03_SYS_ASIC + ASIC_SLOT_1_START )

#define KN03_SYS_ETHER_ADDR	( KN03_SYS_ASIC + ASIC_SLOT_2_START )

#define KN03_SYS_LANCE		( KN03_SYS_ASIC + ASIC_SLOT_3_START )

#define KN03_SYS_SCC_0		( KN03_SYS_ASIC + ASIC_SLOT_4_START )

#define KN03_SYS_xxx1		( KN03_SYS_ASIC + ASIC_SLOT_5_START )

#define KN03_SYS_SCC_1		( KN03_SYS_ASIC + ASIC_SLOT_6_START )

#define KN03_SYS_xxx2		( KN03_SYS_ASIC + ASIC_SLOT_7_START )

#define KN03_SYS_CLOCK		( KN03_SYS_ASIC + ASIC_SLOT_8_START )

#define	KN03_SYS_ERRADR		( KN03_SYS_ASIC + ASIC_SLOT_9_START )

#define	KN03_SYS_CHKSYN		( KN03_SYS_ASIC + ASIC_SLOT_10_START )

#define	KN03_SYS_MCR		( KN03_SYS_ASIC + ASIC_SLOT_11_START )

#define	KN03_SYS_SCSI		( KN03_SYS_ASIC + ASIC_SLOT_12_START )

#define	KN03_SYS_xxx3		( KN03_SYS_ASIC + ASIC_SLOT_13_START )

#define	KN03_SYS_SCSI_DMA	( KN03_SYS_ASIC + ASIC_SLOT_14_START )

#define	KN03_SYS_xxx4		( KN03_SYS_ASIC + ASIC_SLOT_15_START )

#define	KN03_SYS_BOOT_ROM_START	( KN02_PHYS_TC_3_START + 0x400000 )
#define	KN03_SYS_BOOT_ROM_END	( KN03_SYS_BOOT_ROM_START + 0x3ffff )


/*
 * Interrupts
 */

#define KN03_INT_FPA		IP_LEV7		/* Floating Point coproc */
#define KN03_INT_HALTB		IP_LEV6		/* Halt button */
#define KN03_INT_MEM		IP_LEV5		/* memory controller */
#define KN03_INT_RES2		IP_LEV4		/* reserved, unused */
#define KN03_INT_CLOCK		IP_LEV3		/* rtc chip */
#define KN03_INT_IO		IP_LEV2		/* I/O slots */

/*
 * System board registers
 */

/* Error address status register */

#define KN03_ERR_ADDRESS	0x07ffffff	/* phys address */
#define KN03_ERR_RESERVED	0x08000000	/* unused */
#define KN03_ERR_ECCERR		0x10000000	/* ECC error */
#define KN03_ERR_WRITE		0x20000000	/* read/write transaction */
#define KN03_ERR_CPU		0x40000000	/* CPU or device initiator */
#define KN03_ERR_VALID		0x80000000	/* Info is valid */

/* ECC check/syndrome status register */

#define KN03_ECC_SYNLO		0x0000007f	/* syndrome, even bank	*/
#define KN03_ECC_SNGLO		0x00000080	/* single bit err, " 	*/
#define KN03_ECC_CHKLO		0x00007f00	/* check bits,	"  "	*/
#define KN03_ECC_VLDLO		0x00008000	/* info valid for  "	*/
#define KN03_ECC_SYNHI		0x007f0000	/* syndrome, odd bank	*/
#define KN03_ECC_SNGHI		0x00800000	/* single bit err, "	*/
#define KN03_ECC_CHKHI		0x7f000000	/* check bits,  "  "	*/
#define KN03_ECC_VLDHI		0x80000000	/* info valid for  "	*/

/* Sys (really MEM) CSR */

#define KN03_MCR_xxx1		0xffffa000
#define KN03_MCR_CORRECT	0x00002000	/* Set to enable ECC H/W */
#define	KN03_MCR_xxx2		0x00001800
#define KN03_MCR_BNK32M		0x00000400	/* Set -> 32MB mem slots */
#define KN03_MCR_xxx3		0x000003ff

/* Registers in the I/O ASIC */

#define	KN03_REG_SCSI_DMAPTR	( KN03_SYS_ASIC + ASIC_SCSI_DMAPTR )
#define	KN03_REG_SCSI_DMANPTR	( KN03_SYS_ASIC + ASIC_SCSI_NEXTPTR )
#define	KN03_REG_LANCE_DMAPTR	( KN03_SYS_ASIC + ASIC_LANCE_DMAPTR )
#define	KN03_REG_SCC_T1_DMAPTR	( KN03_SYS_ASIC + ASIC_SCC_T1_DMAPTR )
#define	KN03_REG_SCC_R1_DMAPTR	( KN03_SYS_ASIC + ASIC_SCC_R1_DMAPTR )
#define	KN03_REG_SCC_T2_DMAPTR	( KN03_SYS_ASIC + ASIC_SCC_T2_DMAPTR )
#define	KN03_REG_SCC_R2_DMAPTR	( KN03_SYS_ASIC + ASIC_SCC_R2_DMAPTR )
/*				( KN03_SYS_ASIC + ASIC_FLOPPY_DMAPTR )	*/
/*				( KN03_SYS_ASIC + ASIC_ISDN_X_DMAPTR )	*/
/*				( KN03_SYS_ASIC + ASIC_ISDN_X_NEXTPTR )	*/
/*				( KN03_SYS_ASIC + ASIC_ISDN_R_DMAPTR )	*/
/*				( KN03_SYS_ASIC + ASIC_ISDN_R_NEXTPTR )	*/
#define KN03_REG_BUFF0		( KN03_SYS_ASIC + ASIC_BUFF0 )
#define KN03_REG_BUFF1		( KN03_SYS_ASIC + ASIC_BUFF1 )
#define KN03_REG_BUFF2		( KN03_SYS_ASIC + ASIC_BUFF2 )
#define KN03_REG_BUFF3		( KN03_SYS_ASIC + ASIC_BUFF3 )
#define	KN03_REG_CSR		( KN03_SYS_ASIC + ASIC_CSR )
#define	KN03_REG_INTR		( KN03_SYS_ASIC + ASIC_INTR )
#define	KN03_REG_IMSK		( KN03_SYS_ASIC + ASIC_IMSK )
#define	KN03_REG_CURADDR	( KN03_SYS_ASIC + ASIC_CURADDR )
/*				( KN03_SYS_ASIC + ASIC_ISDN_X_DATA )	*/
/*				( KN03_SYS_ASIC + ASIC_ISDN_R_DATA )	*/
#define	KN03_REG_LANCE_DECODE	( KN03_SYS_ASIC + ASIC_LANCE_DECODE )
#define	KN03_REG_SCSI_DECODE	( KN03_SYS_ASIC + ASIC_SCSI_DECODE )
#define	KN03_REG_SCC0_DECODE	( KN03_SYS_ASIC + ASIC_SCC0_DECODE )
#define	KN03_REG_SCC1_DECODE	( KN03_SYS_ASIC + ASIC_SCC1_DECODE )
#	define KN03_LANCE_CONFIG	3
#	define KN03_SCSI_CONFIG		14
#	define KN03_SCC0_CONFIG		(0x10|4)
#	define KN03_SCC1_CONFIG		(0x10|6)

/*				( KN03_SYS_ASIC + ASIC_FLOPPY_DECODE )	*/
#define	KN03_REG_SCSI_SCR	( KN03_SYS_ASIC + ASIC_SCSI_SCR )
#define	KN03_REG_SCSI_SDR0	( KN03_SYS_ASIC + ASIC_SCSI_SDR0 )
#define	KN03_REG_SCSI_SDR1	( KN03_SYS_ASIC + ASIC_SCSI_SDR1 )

#	define KN03_LANCE_CONFIG	3
#	define KN03_SCSI_CONFIG		14
#	define KN03_SCC0_CONFIG		(0x10|4)
#	define KN03_SCC1_CONFIG		(0x10|6)

/* (re)defines for the ASIC CSR (KN03_REG_CSR) */

#define KN03_CSR_DMAEN_T1	ASIC_CSR_DMAEN_T1
#define KN03_CSR_DMAEN_R1	ASIC_CSR_DMAEN_R1
#define KN03_CSR_DMAEN_DTOP_T	ASIC_CSR_DMAEN_T2
#define KN03_CSR_DMAEN_DTOP_R	ASIC_CSR_DMAEN_R2
/*				ASIC_CSR_FLOPPY_DIR	*/
/*				ASIC_CSR_DMAEN_FLOPPY	*/
/*				ASIC_CSR_DMAEN_ISDN_T	*/
/*				ASIC_CSR_DMAEN_ISDN_R	*/
#define KN03_CSR_SCSI_DIR	ASIC_CSR_SCSI_DIR
#define KN03_CSR_DMAEN_SCSI	ASIC_CSR_DMAEN_SCSI
#define KN03_CSR_DMAEN_LANCE	ASIC_CSR_DMAEN_LANCE

#define KN03_CSR_DIAGDN		0x00008000	/* rw */
#define KN03_CSR_TXDIS_2	0x00004000	/* rw */
#define KN03_CSR_TXDIS_1	0x00002000	/* rw */
#define KN03_xxx_ENABLE		0x00001000	/* rw - ISDN on maxine */
#define KN03_CSR_SCC_ENABLE	0x00000800	/* rw */
#define KN03_CSR_RTC_ENABLE	0x00000400	/* rw */
#define KN03_CSR_SCSI_ENABLE	0x00000200	/* rw */
#define KN03_CSR_LANCE_ENABLE	0x00000100	/* rw */
#define KN03_CSR_LEDS		0x000000ff	/* rw */

/* The "enable" bits above appear to actually drive the corresponding
   chip's reset lines when set to zero */

/* (re)defines for the ASIC Interrupt and Mask Registers (KN03_REG_INTR)*/

#define	KN03_INTR_T1_PAGE_END	ASIC_INTR_T1_PAGE_END
#define	KN03_INTR_T1_READ_E	ASIC_INTR_T1_READ_E
#define	KN03_INTR_R1_HALF_PAGE	ASIC_INTR_R1_HALF_PAGE
#define	KN03_INTR_R1_DMA_OVRUN	ASIC_INTR_R1_DMA_OVRUN
#define	KN03_INTR_T2_PAGE_END	ASIC_INTR_T2_PAGE_END
#define	KN03_INTR_T2_READ_E	ASIC_INTR_T2_READ_E
#define	KN03_INTR_R2_HALF_PAGE	ASIC_INTR_R2_HALF_PAGE
#define	KN03_INTR_R2_DMA_OVRUN	ASIC_INTR_R2_DMA_OVRUN
#define	KN03_INTR_SCSI_PTR_LOAD	ASIC_INTR_SCSI_PTR_LOAD
#define	KN03_INTR_SCSI_OVRUN	ASIC_INTR_SCSI_OVRUN
#define	KN03_INTR_SCSI_READ_E	ASIC_INTR_SCSI_READ_E
#define	KN03_INTR_LANCE_READ_E	ASIC_INTR_LANCE_READ_E

#define	KN03_INTR_NVR_JUMPER	0x00004000	/* ro */
#define KN03_INTR_TC_2		0x00002000	/* ro */
#define KN03_INTR_TC_1		0x00001000	/* ro */
#define KN03_INTR_TC_0		0x00000800	/* ro */
#define	KN03_INTR_NRMOD_JUMPER	0x00000400	/* ro */
#define	KN03_INTR_SCSI		0x00000200	/* ro */
#define	KN03_INTR_LANCE		0x00000100	/* ro */
#define	KN03_INTR_SCC_1		0x00000080	/* ro */
#define	KN03_INTR_SCC_0		0x00000040	/* ro */
#define	KN03_INTR_CLOCK		0x00000020	/* ro */
#define	KN03_INTR_PSWARN	0x00000010	/* ro */
#define	KN03_INTR_SCSI_FIFO	0x00000004	/* ro */
#define	KN03_INTR_PBNC		0x00000002	/* ro */
#define	KN03_INTR_PBNO		0x00000001	/* ro */

#endif	MIPS_KN03_H
