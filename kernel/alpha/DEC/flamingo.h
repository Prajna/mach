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
 * $Log:	flamingo.h,v $
 * Revision 2.2  93/03/09  10:48:19  danner
 * 	Created.
 * 	[92/11/18            jeffreyh]
 * 
 */
/*
 *	File: kn15aa.h
 * 	Author: Jeffrey Heller
 *	Created from: kmin.h by Alessandro Forin, Carnegie Mellon University
 *	Date:	11/92
 *
 *	Definitions specific to the KN15AA processors and FLAMINGO
 */

#ifndef	ALPHA_KN15AA_H
#define	ALPHA_KN15AA_H 1

/*
 * Flamingo's Physical address space
 */

/*
 * Memory map
 */


/*
 * I/O map
 */


#define KN15AA_PHYS_TC_0_START_D 0x100000000	/* TURBOchannel, slot 0 */
#define KN15AA_PHYS_TC_0_END_D	 0x107ffffff	/*  128 Meg, option0 Dense*/
#define KN15AA_PHYS_TC_0_START_S 0x110000000	/* TURBOchannel, slot 0 */
#define KN15AA_PHYS_TC_0_END_S	 0x11fffffff	/*  256 Meg, option0 Sparse*/

#define KN15AA_PHYS_TC_1_START_D 0x120000000	/* TURBOchannel, slot 1 */
#define KN15AA_PHYS_TC_1_END_D	 0x127ffffff	/*  128 Meg, option1 Dense*/
#define KN15AA_PHYS_TC_1_START_S 0x130000000	/* TURBOchannel, slot 1 */
#define KN15AA_PHYS_TC_1_END_S	 0x13fffffff	/*  256 Meg, option1 Sparse*/

#define KN15AA_PHYS_TC_2_START_D 0x140000000	/* TURBOchannel, slot 2 */
#define KN15AA_PHYS_TC_2_END_D	 0x147ffffff	/*  128 Meg, option2 Dense*/
#define KN15AA_PHYS_TC_2_START_S 0x150000000	/* TURBOchannel, slot 2 */
#define KN15AA_PHYS_TC_2_END_S	 0x15fffffff	/*  256 Meg, option2 Sparse*/

#define KN15AA_PHYS_TC_3_START_D 0x160000000	/* TURBOchannel, slot 3 */
#define KN15AA_PHYS_TC_3_END_D	 0x167ffffff	/*  128 Meg, option3 Dense*/
#define KN15AA_PHYS_TC_3_START_S 0x170000000	/* TURBOchannel, slot 3 */
#define KN15AA_PHYS_TC_3_END_S	 0x17fffffff	/*  256 Meg, option3 Sparse*/

#define KN15AA_PHYS_TC_4_START_D 0x180000000	/* TURBOchannel, slot 4 */
#define KN15AA_PHYS_TC_4_END_D	 0x187ffffff	/*  128 Meg, option4 Dense*/
#define KN15AA_PHYS_TC_4_START_S 0x190000000	/* TURBOchannel, slot 4 */
#define KN15AA_PHYS_TC_4_END_S	 0x19fffffff	/*  256 Meg, option4 Sparse*/

#define KN15AA_PHYS_TC_5_START_D 0x1a0000000	/* TURBOchannel, slot 5 */
#define KN15AA_PHYS_TC_5_END_D	 0x1a7ffffff	/*  128 Meg, option5 Dense*/
#define KN15AA_PHYS_TC_5_START_S 0x1b0000000	/* TURBOchannel, slot 5 */
#define KN15AA_PHYS_TC_5_END_S	 0x1bfffffff	/*  256 Meg, option5 Sparse*/

#define KN15AA_PHYS_TC_6_START_D 0x1c0000000	/* TURBOchannel, slot 6 */
#define KN15AA_PHYS_TC_6_END_D	 0x1c7ffffff	/*  128 Meg, SCSI Dense*/
#define KN15AA_PHYS_TC_6_START_S 0x1d0000000	/* TURBOchannel, slot 6 */
#define KN15AA_PHYS_TC_6_END_S	 0x1dfffffff	/*  256 Meg, SCSI Sparse*/

#define KN15AA_PHYS_TC_7_START_D 0x1e0000000	/* TURBOchannel, slot 7 (SCSI)*/
#define KN15AA_PHYS_TC_7_END_D	 0x1e7ffffff	/*  128 Meg, system devs Dense*/
#define KN15AA_PHYS_TC_7_START_S 0x1f0000000	/* TURBOchannel, slot 7 */
#define KN15AA_PHYS_TC_7_END_S	 0x1ffffffff	/*  256 Meg, system devs Sparse*/

#define KN15AA_PHYS_TC_SCSI_START_D 0x1c0000000	/* TC, slot 7 (SCSI)*/
#define KN15AA_PHYS_TC_SCSI_END_D   0x1c1ffffff	/* TC, slot 7 (SCSI)*/
#define KN15AA_PHYS_TC_CSR0_START_D 0x1c2000000	/* TC, slot 7 (CSR's)*/
#define KN15AA_PHYS_TC_CSR0_END_D   0x1c3ffffff	/* TC, slot 7 (CSR's)*/
#define KN15AA_PHYS_TC_ASIC_START_D 0x1e0000000	/* TC, slot 8 (IOASIC)*/
#define KN15AA_PHYS_TC_ASIC_END_D   0x1e1ffffff	/* TC, slot 8 (IOASIC)*/
#define KN15AA_PHYS_TC_CXT_START_D  0x1e2000000	/* TC, slot 8 (CXTurbo)*/
#define KN15AA_PHYS_TC_CXT_END_D    0x1e3ffffff	/* TC, slot 8 (CXTurbo)*/

#define KN15AA_PHYS_TC_SCSI_START_S 0x1d0000000	/* TC, slot 7 (SCSI)*/
#define KN15AA_PHYS_TC_SCSI_END_S   0x1d3ffffff	/* TC, slot 7 (SCSI)*/
#define KN15AA_PHYS_TC_CSR0_START_S 0x1d4000000	/* TC, slot 7 (CSR's)*/
#define KN15AA_PHYS_TC_CSR0_END_S   0x1d7ffffff	/* TC, slot 7 (CSR's)*/
#define KN15AA_PHYS_TC_ASIC_START_S 0x1f0000000	/* TC, slot 8 (IOASIC)*/
#define KN15AA_PHYS_TC_ASIC_END_S   0x1f3ffffff	/* TC, slot 8 (IOASIC)*/
#define KN15AA_PHYS_TC_CXT_START_S  0x1f4000000	/* TC, slot 8 (CXTurbo)*/
#define KN15AA_PHYS_TC_CXT_END_S    0x1f7ffffff	/* TC, slot 8 (CXTurbo)*/

#define	KN15AA_PHYS_TC_START	KN15AA_PHYS_TC_0_START
#define	KN15AA_PHYS_TC_END	KN15AA_PHYS_TC_7_END	/* xxx Meg */

#define KN15AA_TC_NSLOTS	8
#define	KN15AA_TC_MIN		0
#define KN15AA_TC_MAX		5		/*
						 * don't look at system & scsi
						 * slots
						 */

/* Pseudo-TCslots */

#define KN15AA_SCC_0_SLOT		6
#define KN15AA_SCC_1_SLOT		7
#define KN15AA_SCSI1_SLOT		8
#define KN15AA_SCSI_SLOT		9
#define KN15AA_CFB_SLOT			10
#define KN15AA_REG_SLOT			11
#define KN15AA_ISDN_SLOT		12
#define KN15AA_TOY_SLOT			13
#define KN15AA_LANCE_SLOT		14

#define KN15AA_PSEUDO_SLOT_START	6
#define KN15AA_PSEUDO_SLOT_END		14

#define	KN15AA_ASIC_SLOT		7 

/*
 * System module space (IO ASIC)
 */

#include <alpha/DEC/asic.h>

#define	KN15AA_SYS_ASIC_D	( KN15AA_PHYS_TC_7_START_D + 0x0000000)
#define	KN15AA_SYS_ASIC_S	( KN15AA_PHYS_TC_7_START_S + 0x0000000)
/*
 * For now all of the below are Sparse addresses until I do something else.
 * The multiplier of 2 for each asic slot is to adjust for the sparse
 * space. If it were dense space there would be no need to adjust the address.
 */
/* Phys Addresses*/
#define	KN15AA_SYS_SCSI		   KN15AA_PHYS_TC_6_START_S
#define KN15AA_SYS_ASIC		   KN15AA_SYS_ASIC_S /* XXX */
#define	KN15AA_SYS_ROM_START	   ( KN15AA_SYS_ASIC + (ASIC_SLOT_0_START * 2))
#define KN15AA_SYS_ASIC_REGS	   ( KN15AA_SYS_ASIC + (ASIC_SLOT_1_START * 2))
#define	KN15AA_SYS_ETHER_ADDRESS   ( KN15AA_SYS_ASIC + (ASIC_SLOT_2_START * 2))
#define	KN15AA_SYS_LANCE	   ( KN15AA_SYS_ASIC + (ASIC_SLOT_3_START * 2))
#define	KN15AA_SYS_SCC_0	   ( KN15AA_SYS_ASIC + (ASIC_SLOT_4_START * 2))
#define	KN15AA_SYS_SCC_1	   ( KN15AA_SYS_ASIC + (ASIC_SLOT_6_START * 2))
#define	KN15AA_SYS_xxx1		   ( KN15AA_SYS_ASIC + (ASIC_SLOT_7_START * 2))
#define KN15AA_SYS_TOY		   ( KN15AA_SYS_ASIC + (ASIC_SLOT_8_START * 2))
#define KN15AA_SYS_ISDN		   ( KN15AA_SYS_ASIC + (ASIC_SLOT_9_START * 2))
#define KN15AA_SYS_xxx2		   ( KN15AA_SYS_ASIC + (ASIC_SLOT_10_START *2))
#define KN15AA_SYS_CXTurbo	   ( KN15AA_SYS_ASIC + 0x4000000)

/* Offsets from KN15AA_SYS_ASIC */
#define	KN15AA_OFF_SCSI		   0
#define KN15AA_OFF_ASIC		   0 /* XXX */
#define	KN15AA_OFF_ROM_START	   (ASIC_SLOT_0_START * 2)
#define KN15AA_OFF_ASIC_REGS	   (ASIC_SLOT_1_START * 2)
#define	KN15AA_OFF_ETHER_ADDRESS   (ASIC_SLOT_2_START * 2)
#define	KN15AA_OFF_LANCE	   (ASIC_SLOT_3_START * 2)
#define	KN15AA_OFF_SCC_0	   (ASIC_SLOT_4_START * 2)
#define	KN15AA_OFF_SCC_1	   (ASIC_SLOT_6_START * 2)
#define	KN15AA_OFF_xxx1		   (ASIC_SLOT_7_START * 2)
#define KN15AA_OFF_TOY		   (ASIC_SLOT_8_START * 2)
#define KN15AA_OFF_ISDN		   (ASIC_SLOT_9_START * 2)
#define KN15AA_OFF_xxx2		   (ASIC_SLOT_10_START *2)
#define KN15AA_OFF_CXTurbo	   0x4000000

/*
 * Interrupts
 */

#define	KN15AA_IO_INTERRUPT_SCB		SCB_INTERRUPT_FIRST

/*
 *  System registers addresses (IO Control ASIC)
 */

/* 
 * The TCCSR is what is called in the Flamingo documentation
 * CSR's. This is the base address for many system registers.
 * What is know to the Mach kernel as ASIC_CSR is know in the 
 * Flamingo documentation as SSR (System support register's)
 * For now I will follow the Mach example to keep the code from
 * port to port readable
 */ 
#define	KN15AA_SYS_TCCSR_D		KN15AA_PHYS_TC_6_START_D+0x2000000
#define	KN15AA_SYS_TCCSR_S		KN15AA_PHYS_TC_6_START_S+0x4000000

#define KN15AA_REG_IOSLOT		(KN15AA_SYS_TCCSR_S + 0x000000000)
#	define IOSLOT_PARITY		0x4
#	define IOSLOT_BLOCK		0x2
#	define IOSLOT_SGDMA		0x1
#define KN15AA_REG_TCCONFIG		(KN15AA_SYS_TCCSR_D + 0x8)
#define KN15AA_REG_FADR			(KN15AA_SYS_TCCSR_D + 0x10)
#define KN15AA_REG_TCEREG		(KN15AA_SYS_TCCSR_D + 0x18)

/*
 * Not sure which one I will need, define both, two things with the
 * same name and same stuff... Is one for each scsi adaptor?
 */
#define KN15AA_REG_IR_S			(KN15AA_SYS_TCCSR_S + 0x800000)
#define KN15AA_REG_IR_D			(KN15AA_SYS_TCCSR_D + 0x400000)
#define KN15AA_REG_IR			KN15AA_REG_IR_S

#define KN15AA_REG_IMR_WR		0x1c281fffc
#define KN15AA_REG_IMR_RD		0x1c2400000

#define KN15AA_REG_IR1_S		(KN15AA_SYS_TCCSR_S + 0xC00000)
#define KN15AA_REG_IR1_D		(KN15AA_SYS_TCCSR_D + 0x600000)
#define KN15AA_REG_IR1			KN15AA_REG_IR_S

/* 
 *XXX Do not use the dence version, It will set the Interrupt mask instead of
 *XXX the sgmap
 */
#define KN15AA_REG_SGMAP_D		(KN15AA_SYS_TCCSR_D + 0x800000) 
#define KN15AA_REG_SGMAP_S		(KN15AA_SYS_TCCSR_S + 0x1000000)
#define KN15AA_REG_SGMAP		KN15AA_REG_SGMAP_S /* Start of 128k map */

#define KN15AA_REG_TCRESET_D		(KN15AA_SYS_TCCSR_D + 0xa00000)
#define KN15AA_REG_TCRESET_S		(KN15AA_SYS_TCCSR_S + 0x1400000)
#define KN15AA_REG_TCRESET		KN15AA_REG_TCRESET_S		
/*
 * The following are Dense addresses
 */
#define	KN15AA_REG_LANCE_DMAPTR		( KN15AA_SYS_ASIC_D + ASIC_LANCE_DMAPTR )
#define	KN15AA_REG_SCC_T1_DMAPTR	( KN15AA_SYS_ASIC_D + ASIC_SCC_T1_DMAPTR )
#define	KN15AA_REG_SCC_R1_DMAPTR	( KN15AA_SYS_ASIC_D + ASIC_SCC_R1_DMAPTR )
#define	KN15AA_REG_SCC_T2_DMAPTR	( KN15AA_SYS_ASIC_D + ASIC_SCC_T2_DMAPTR )
#define	KN15AA_REG_SCC_R2_DMAPTR	( KN15AA_SYS_ASIC_D + ASIC_SCC_R2_DMAPTR )
/* 
 * As stated above, the following register is know in the
 * Flamingo documentaion as the SSR register
 */
#define	KN15AA_REG_CSR		( KN15AA_SYS_ASIC_D + ASIC_CSR )
/*
 * The following is know in the Flamingo documentation as the SIR, but
 * maps out as the interrupt register from the pmax asic.h.
 * It has the types of interrupts from the IOCTL ASIC (system slot)
 * Both the pmax like name and the Flamingo name are here for now.
 */
#define	KN15AA_REG_INTR		( KN15AA_SYS_ASIC_D + ASIC_INTR )
#define KN15AA_REG_SIR		KN15AA_REG_INTR
/*
 * SIR bit's 
 * They are defined in asic.h as ASIC_INTR_*
 */


/*
 * The following is know in the Flamingo documentation 
 * as the System Interupt mask
 */
#define	KN15AA_REG_SIMR		( KN15AA_SYS_ASIC_D + ASIC_IMSK )
/*
 * The following is know in the Flamingo documentation as the System address
 */
#define	KN15AA_REG_CURADDR	( KN15AA_SYS_ASIC_D + ASIC_CURADDR )
/*
 * The following is know in the Flamingo documentation 
 * as the System Interupt mask
 */
#define	KN15AA_REG_IMSK		( KN15AA_SYS_ASIC_D + ASIC_IMSK )

#define	KN15AA_REG_ISDN_X_DATA	( KN15AA_SYS_ASIC_D + ASIC_ISDN_X_DATA )
#define	KN15AA_REG_ISDN_R_DATA	( KN15AA_SYS_ASIC_D + ASIC_ISDN_R_DATA )

#define	KN15AA_REG_LANCE_DECODE	( KN15AA_SYS_ASIC_D + ASIC_LANCE_DECODE )
#define KN15AA_LANCE_CONFIG	3

#define	KN15AA_REG_SCC0_DECODE	( KN15AA_SYS_ASIC_D + ASIC_SCC0_DECODE )
#define KN15AA_SCC0_CONFIG		(0x10|4)
#define	KN15AA_REG_SCC1_DECODE	( KN15AA_SYS_ASIC_D + ASIC_SCC1_DECODE )
#define KN15AA_SCC1_CONFIG		(0x10|6)


/*
 *  System registers defines (MREG and CREG)
 */


/*
 * SCSI defines 
 */

/* 
 * The following register is the SCSI control interrupt register 
 * It starts, staps and resets scsi DMA. It take over the SCSI funtions
 * that were handled by the ASIC on the 3min
 */
#define KN15AA_REG_SCSI_CIR	(KN15AA_SYS_SCSI + 0x80000)
#define SCSI_CIR_AIOPAR		0x80000000 /* TC IO Address parity error */
#define SCSI_CIR_WDIOPAR	0x40000000 /* TC IO  write data parity error */
#define SCSI_CIR_DMARPAR1	0x20000000 /* SCSI[1] TC DMA read data parity */
#define SCSI_CIR_DMARPAR0	0x10000000 /* SCSI[0] TC DMA read data parity */
#define SCSI_CIR_DMABUFPAR1	0x08000000 /* SCSI[1] DMA buffer parity error */
#define SCSI_CIR_DMABUFPAR0	0x04000000 /* SCSI[0] DMA buffer parity error */
#define SCSI_CIR_DBPAR1		0x02000000 /* SCSI[1] DB parity error */
#define SCSI_CIR_DBPAR0		0x01000000 /* SCSI[0] DB parity error */
#define SCSI_CIR_DMAERR1	0x00800000 /* SCSI[1] DMA error */
#define SCSI_CIR_DMAERR0	0x00400000 /* SCSI[0] DMA error */
#if fmm50
#define SCSI_CIR_xxx0		0x00200000 /* RESERVED */
#define SCSI_CIR_xxx1		0x00100000 /* RESERVED */
#else
#define SCSI_CIR_PREF1		0x00200000 /* 53C94 prefetch interupt */
#define SCSI_CIR_PREF0		0x00100000 /* 53C94 prefetch interupt */
#endif
#define SCSI_CIR_53C94_INT1	0x00080000 /* SCSI[1] 53C94 Interupt */
#define SCSI_CIR_53C94_INT0	0x00040000 /* SCSI[0] 53C94 Interupt */
#define SCSI_CIR_53C94_DREQ1	0x00020000 /* SCSI[1] 53C94 DREQ */
#define SCSI_CIR_53C94_DREQ0	0x00010000 /* SCSI[0] 53C94 DREQ */
#define SCSI_CIR_TC_PAR_TEST	0x00008000 /* TC parity test mode */
#define SCSI_CIR_DB_PAR_TEST	0x00004000 /* DB parity test mode */
#define SCSI_CIR_DBUF_PAR_TEST1	0x00002000 /* SCSI[1] DMA buffer parity test */
#define SCSI_CIR_DBUF_PAR_TEST0	0x00001000 /* SCSI[0] DMA buffer parity test */
#define SCSI_CIR_RESET1		0x00000800 /* SCSI[1] ~Reset,enable(0)/disable(1) */
#define SCSI_CIR_RESET0		0x00000400 /* SCSI[0] ~Reset,enable(0)/disable(1) */
#define SCSI_CIR_DMAENA1	0x00000200 /* SCSI[1] DMA enable */
#define SCSI_CIR_DMAENA0	0x00000100 /* SCSI[1] DMA enable */
#define SCSI_CIR_GPI3		0x00000080 /* General purpose input <3> */
#define SCSI_CIR_GPI2		0x00000040 /* General purpose input <2> */
#define SCSI_CIR_GPI1		0x00000020 /* General purpose input <1> */
#define SCSI_CIR_GPI0		0x00000010 /* General purpose input <0> */
#define SCSI_CIR_TXDIS		0x00000008 /* TXDIS- serial transmit disable */
#define SCSI_CIR_GPO2		0x00000004 /* General purpose output <2> */
#define SCSI_CIR_GPO1		0x00000002 /* General purpose output <1> */
#define SCSI_CIR_GPO0		0x00000001 /* General purpose output <0> */
#define SCSI_CIR_ERROR (SCSI_CIR_AIOPAR | SCSI_CIR_WDIOPAR | SCSI_CIR_DMARPAR1 | SCSI_CIR_DMARPAR0 | SCSI_CIR_DMABUFPAR1 | SCSI_CIR_DMABUFPAR0 | SCSI_CIR_DBPAR1 |SCSI_CIR_DBPAR0 | SCSI_CIR_DMAERR1 | SCSI_CIR_DMAERR0 )

#define KN15AA_REG_SCSI_DMAPTR0 (KN15AA_SYS_SCSI + 0x82000)
#define KN15AA_REG_SCSI_DMAPTR1 (KN15AA_SYS_SCSI + 0x82200)

#define KN15AA_REG_SCSI_DIC0 (KN15AA_SYS_SCSI + 0x82008)
#define KN15AA_REG_SCSI_DIC1 (KN15AA_SYS_SCSI + 0x82208)
#define SCSI_DIC_DMADIR		0x00000080 /* DMA direction read(0)/write(1) */
#define SCSI_DIC_PREFENA	0x00000040 /* DMA read prefetch dis(0)/ena(1) */
#define SCSI_DIC_DMAADDR1	0x00000002 /* DMA address <1> */
#define SCSI_DIC_DMAADDR0	0x00000001 /* DMA address <0> */
#define SCSI_DIC_ADDR_MASK	(SCSI_DIC_DMAADDR0 |SCSI_DIC_DMAADDR1)

#define KN15AA_REG_SCSI_94REG0	(KN15AA_SYS_SCSI + 0x100000)
#define KN15AA_REG_SCSI_94REG1	(KN15AA_SYS_SCSI + 0x100200)

#define KN15AA_REG_SCSI_IMER	(KN15AA_SYS_SCSI + 0x80008)

/* these are the bits that were unalligned at the beginning of the dma */
#define KN15AA_REG_SCSI_DUDB0	(KN15AA_SYS_SCSI + 0x82010)
#define KN15AA_REG_SCSI_DUDB1	(KN15AA_SYS_SCSI + 0x82210)
#	define SCSI_DUDB_MASK01	0x00000001 /* Mask bit for byte[01] */
#	define SCSI_DUDB_MASK10	0x00000002 /* Mask bit for byte[10] */
#	define SCSI_DUDB_MASK11	0x00000004 /* Mask bit for byte[11] */

/* these are the bits that were unalligned at the end of the dma */
#define KN15AA_REG_SCSI_DUDE0	(KN15AA_SYS_SCSI + 0x82018)
#define KN15AA_REG_SCSI_DUDE1	(KN15AA_SYS_SCSI + 0x82218)
#	define SCSI_DUDE_MASK00	0x1000000 /* Mask bit for byte[00] */
#	define SCSI_DUDE_MASK01	0x2000000 /* Mask bit for byte[01] */
#	define SCSI_DUDE_MASK10	0x4000000 /* Mask bit for byte[10] */

#endif	ALPHA_KN15AA_H
