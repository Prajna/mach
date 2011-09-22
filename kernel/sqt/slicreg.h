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
 * $Log:	slicreg.h,v $
 * Revision 2.3  91/07/31  18:04:07  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  12:59:55  dbg
 * 	Added, from Sequent SYMMETRY sources.
 * 	[91/02/26            dbg]
 * 
 */

/*
 * $Header: slicreg.h,v 2.3 91/07/31 18:04:07 dbg Exp $
 *
 * slicreg.h
 * 	SLIC registers and their contents.
 */

/*
 * Revision 1.1  89/07/19  14:48:55  kak
 * Initial revision
 * 
 * Revision 2.23  88/10/10  15:07:43  rto
 * Added 532-specific definitions in support of a common inter-project
 * build environment.
 * 
 * Revision 2.22  88/10/06  09:32:27  rto
 * 532port:  Added support for 532-specific configuration options.
 * 
 * Revision 2.22  88/09/28  11:29:35  corene
 * added new defines for Wombat (3167 Weitek FPA)
 * 
 * Revision 2.20  88/09/22  08:58:35  gak
 * Add extra SL_P2_SPEED values.
 * 
 * Revision 2.19  88/03/24  10:42:07  dilip
 * Corrected values for SL_P2_386_D, SL_P2_387_C and renamed
 * WEITEK define.
 * 
 * Revision 2.18  88/03/23  06:57:14  gak
 * Add new bits to Symmetry proc config proms.
 * 
 */

#ifndef	_SQT_SLIREG_H_
#define	_SQT_SLIREG_H_

/* General registers */
#define SL_G_BOARDTYPE	0
#define		SLB_PROCBOARD	0x0
#define		SLB_MEMBOARD	0x1
#define		SLB_MBABOARD	0x2
#define 	SLB_SCSIBOARD	0x3
#define		SLB_ZDCBOARD	0x4
#define		SLB_CLKARBBOARD	0x5
#define		SLB_SGSPROCBOARD 0x6
#define		SLB_SSMBOARD    0x8
#define		SLB_532PROCBOARD 0x9
#define		SLB_SGSMEMBOARD	0x7
#define		SLB_FOUNDBOARD	0x40
#define		SLB_GFDEBOARD	0x41
#define		SLB_KXXBOARD	0x80

#define SL_G_VARIATION	 1		/* board variation */
#define SL_G_HGENERATION 2		/* h/w board generation */
#define SL_G_SGENERATION 3		/* s/w board generation */
#define	SL_G_LAST_SEQ	20		/* last Sequent-reserved location */
#define SL_G_NONZERO55	29		/* this register should always be 55 */
#define SL_G_NONZEROAA	30		/* this register should always be AA */
#define SL_G_CHKSUM	31		/* checksum for slave reg PROM */
#define SL_G_BERR	32		/* bus error reg */
#define		SLB_IBE		0x01	/* initiated bus transfer */
#define		SLB_RBE		0x02	/* received bus transfer */
#define 	SLB_SBE		0x04	/* saw "bus error" signal */
#define		SLB_DBE		0x08	/* detected bus error */
#define		SLB_ISE		0x10	/* initiated SLIC transfer */
#define		SLB_RSE		0x20	/* received SLIC transfer */
#define		SLB_SSE		0x40	/* saw "SLIC ERROR" signal */
#define		SLB_DSE		0x80	/* detected SLIC error */
#define SL_G_ACCERR	33		/* only on initiator boards */
#define	SL_G_ACCERR0	SL_G_ACCERR	/* Channel 0 Access Error Register */
#define	SL_G_ACCERR1	34		/* Channel 1 Access Error Register */
#define	SL_G_ACCERR2	35		/* Channel 2 Access Error Register */
#define	SL_G_ACCERR3	36		/* Channel 3 Access Error Register */
#define		SLB_AEMSK 	0x3F	/* useful bits in this reg */
#define		SLB_ATMSK 	0x07	/* mask for access type */
#define 	SLB_ACCERR	0x20	/* access error seen */
#define		SLB_AEIO	0x10	/* IO response */
#define		SLB_AERD	0x08	/* read response */
#define		SLB_AETIMOUT	0x00	/* bus timeout (128 cycles) */
#define		SLB_AEFATAL	0x03	/* fatal error */
#define		SLB_AENONFAT	0x05	/* non fatal error */
#define		SLB_AEOK	0x06	/* access completed OK */
#define	SL_G_CHAN0	37		/* Channel 0 Control Register */
#define	SL_G_CHAN1	38		/* Channel 1 Control Register */
#define	SL_G_CHAN2	39		/* Channel 2 Control Register */
#define	SL_G_CHAN3	40		/* Channel 3 Control Register */
#define		SLB_TVAL	0x0F	/* Throttle Value */
#define		SLB_TH_ENB	0x10	/* Throttle Enable */
#define		SLB_AS_REQ	0x20	/* Asynchronous Request Inputs */
#define		SLB_EXT_RD	0x40	/* External Read Responses */
#define		SLB_RESPONDER	0x80	/* Responder Channel */
#define	SL_G_POLICY	41		/* Policy Register */
#define		SLB_ENBERR	0x01	/* Enable Bus Error */
#define		SLB_ENNFE	0x02	/* Enable Non-Fatal Error */
#define		SLB_PDEPTH	0x04	/* Pipe Depth Value */
#define		SLB_FIXEDPRI	0x08	/* Fixed Pri = 1, Round-robin = 0 */
#define		SLB_ENBD	0x80	/* Enable Board */

/* Processor Board specific registers */
#define	SL_P_PCCHS	34		/* proc control and cache hit status */
#define		SLB_RES		0x01	/* reset processor */
#define		SLB_PAUSE	0x02	/* pause processor */
#define		SLB_HIT0	0x04	/* cache hit set 0 */
#define		SLB_HIT1	0x08	/* cache hit set 1 */
#define SL_P_CACHEPAR	35		/* cache parity error status */
#define		SLB_CPARMSK	0x3F	/* useful bits in this reg */
#define		SLB_B0ERR	0x01	/* byte 0 error */
#define		SLB_B1ERR	0x02	/* byte 1 error */
#define		SLB_B2ERR	0x04	/* byte 2 error */
#define		SLB_B3ERR	0x08	/* byte 3 error */
#define		SLB_SET0ERR	0x10	/* set 0 error */
#define		SLB_SET1ERR	0x20	/* set 1 error */
#define	SL_P_CLCACHE	36		/* clear cache match bits */
#define SL_P_CONTROL	37		/* control bits */
#define		SLB_ENB0	0x01	/* enable set 0 */
#define		SLB_ENB1	0x02	/* enable set 1 */
#define		SLB_INV0	0x04	/* invalidate set 0 (neg) */
#define		SLB_INV1	0x08	/* invalidate set 1 (neg) */
#define		SLB_E_WR_BUF	0x10	/* enable write buffer */
#define		SLB_E_NMI	0x20	/* enable NMIs */
#define		SLB_IGNOR_BE	0x40	/* ignore bus error */
#define		SLB_DIS_BE	0x80	/* disable bus error */
#define SL_P_LIGHTON	38		/* read ==> turn ON processor LED */
#define SL_P_LIGHTOFF	39		/* read ==> turn OFF processor LED */

/* Symmetry Series processor-specific registers (subject to change) */
#define	SL_P2_SPEED	4		/* clock speed in MHz */
#define		SLP_16MHZ	16
#define		SLP_20MHZ	20
#define		SLP_25MHZ	25
#define		SLP_30MHZ	30
#define	SL_P2_FP_TYPE	5		/* floating-pt type */
#define 	SLP_381		0x01    /* has ns32381 fpu */
#define		SLP_387		0x01	/* has 80387 (else none) */
#define		SLP_FPA		0x02	/* has Weitek FPA (else no FPA) */
#define		SLP_3167	0x08	/* Wombat (3167) (else 1167) */
#define	SL_P2_BUS_WIDTH	6		/* bus width in bits */
#define	SL_P2_CACHE_SETS	7	/* no. of cache sets */
#define	SL_P2_SET_SIZE	8		/* size of each cache set */
#define		SL_P2_SET_4K	0x03	/* 4Kbyte cache sets */
#define		SL_P2_SET_8K	0x04	/* 8Kbyte cache sets */
#define		SL_P2_SET_16K	0x05	/* 16Kbyte cache sets */
#define		SL_P2_SET_32K	0x06	/* 32Kbyte cache sets */
#define		SL_P2_SET_64K	0x07	/* 64Kbyte cache sets */
#define		SL_P2_SET_128K	0x08	/* 128Kbyte cache sets */
#define		SL_P2_SET_256K	0x09	/* 256Kbyte cache sets */
#define	SL_P2_CUSTOM	9		/* custom settings for Symmetry */
#define		SL_P2_BIC_SYNC	0x01	/* force synchronous BIC */
#define		SL_P2_CMC_FAST	0x02	/* force CMC fast */
#define		SL_P2_CMC_SYNC	0x04	/* force CMC synchronous */
#define		SL_P2_FRC_EXT	0x08	/* force extended mode in proc BIC */
#define		SL_P2_SUB_BLOCK	0x30	/* sub-blocking style */
#define		SL_P2_NO_SBLOCK	0x00	/* no sub-blocking */
#define		SL_P2_SBLOCK_8	0x10	/* 8-byte sub-blocking */
#define		SL_P2_SBLOCK_16	0x20	/* 16-byte sub-blocking */
#define		SL_P2_BLOCK_32	0x40	/* 32-byte blocks */
#define		SL_P2_COMPAT	0x80	/* jumper for compatibility mode */
#define	SL_P2_CUSTOM2	10		/* more custom settings */
#define		SL_P2_MODELB	0x01	/* this is a model B processor board */
#define		SL_P2_NO_Q_BD	0x02	/* old layout without Q board */
#define	SL_P2_INTEL_VER	11		/* version of Intel parts in board */
#define SL_P2_NATNL_VER 11		/* same location for Natnl chip revs */

/* chip version numbers encoded in the config prom */
#define	SL_P2_WEITEK_VER(slic)	(rdslave((slic), SL_P2_CUSTOM)>>4 & 0xF)
#define		SL_P2_1167	0x00	/* 1163A, 1164, 1165 */
#define		SL_P2_3167	0x01	/* 3167 (Wombat) */

#define	SL_P2_386_VER(slic)	(rdslave((slic),SL_P2_INTEL_VER) & 0xF)
#define		SL_P2_386_B	0x00	/* B stepping of 386 */
#define		SL_P2_386_D	0x01	/* D stepping of 386 */

#define	SL_P2_387_VER(slic)	(rdslave((slic),SL_P2_INTEL_VER)>>4 & 0xF)
#define		SL_P2_387_B3	0x00	/* B3 stepping of 387 */
#define		SL_P2_387_C	0x01	/* C stepping of 387 */

#define	SL_P2_532_VER(slic)	(rdslave((slic),SL_P2_NATNL_VER) & 0xF)
#define		SL_P2_532_B3	0x00	/* B3 stepping of 532 */

#define	SL_P2_381_VER(slic)	(rdslave((slic),SL_P2_NATNL_VER)>>4 & 0xF)
#define		SL_P2_381_B2	0x00	/* B2 stepping of 381 */

/* Memory Board specific registers */
#define SL_M_BSIZE	SL_G_VARIATION	/* onboard RAM size and type */
#define 	SLB_LTYPE	0x01	/* set == 64k technology */
#define		SLB_LSIZE	0x30	/* size of onboard RAM */
#define SL_M_ACPTB	4		/* accept expansion board patterns */
#define SL_M_NUMACPT		8	/* number of accept registers */
#define	SL_M_ENABLES	33		/* board enables register */
#define		SLB_MEM_ENB	0x01	/* enable memory array */
#define		SLB_BE_ENB	0x02	/* bus error enable */
#define		SLB_REF_ENB	0x10	/* enable memory refresh */
#define		SLB_INTLV	0x20	/* interleave */
#define SL_M_EXP	34		/* expansion board register */
#define		SLB_R64K	0x01	/* expansion is 64k chips */
#define		SLB_RTYPE	0x0F	/* type of expansion board */
#define		SLB_RSIZE	0x30	/* size of memory off board */
#define SL_M_ECC	35		/* ECC control register */
#define		SLB_EN_UCE_LOG	0x80	/* enable ECC and UCE log */
#define		SLB_UCE_OV	0x40	/* UCE overflow */
#define		SLB_UCE		0x20	/* uncorrectable error */
#define		SLB_REP_UCE	0x10	/* report UCEs */
#define		SLB_EN_CE_LOG	0x08	/* enable ECC and CE log */
#define		SLB_CE_OV	0x04	/* CE overflow */
#define		SLB_CE		0x02	/* correctable error */
#define		SLB_REP_CE	0x01	/* report CEs */
#define	SL_M_MISC	36		/* misc error register */
#define		SLB_ECC_SWAP	0x80	/* swap in check bits */
#define		SLB_FLIP_CYCLE	0x10	/* flip cycle type parity */
#define		SLB_FLIP_B3	0x08	/* flip parity byte 0 */
#define		SLB_FLIP_B2	0x04	/* flip parity byte 0 */
#define		SLB_FLIP_B1	0x02	/* flip parity byte 0 */
#define		SLB_FLIP_B0	0x01	/* flip parity byte 0 */
#define	SL_M_ADDR	37		/* base address */
#define SL_M_REFRESH	38		/* refresh slot */
#define 	SLB_REF0	255	/* immediate refresh */
#define		SLB_REFL	177	/* refresh last slot */
#define SL_M_EADD_L	40		/* address of error */
#define		SLB_ROW		0x3	/* row address */
#define		SLB_CNTRL	0x3	/* Controller board */
#define SL_M_EADD_M	41
#define SL_M_EADD_H	42
#define SL_M_ES		43		/* error status */
#define 	SLB_CT		0x07	/* cycle type */
#define		SLB_BM		0xF0	/* byte marks */
#define SL_M_SYNDR	44		/* error syndrome */

#define REF	7
#define WA8	6
#define RA4	5
#define RA8	4
#define WA4H	3
#define WA4L	2
#define WAPH	1
#define WAPL	0

/* Symmetry Series memory controller registers (subject to change) */
#define	SL_M2_TOTAL_MB	4		/* total size in Mbytes */
#define	SL_M2_BUS_WIDTH	5		/* bus width in bits */
#define	SL_M2_RAM	6		/* ram info */
#define	    SL_M2_RAM_DENS	0x01	/* ram density mask */
#define		SL_M2_RAM_1MB	0x00	/* 1 Mbit chips */
#define		SL_M2_RAM_4MB	0x01	/* 4 Mbit chips */
#define	    SL_M2_RAM_POP	0x80	/* ram population mask */
#define		SL_M2_RAM_FULL	0x00	/* populated by full-banks */
#define		SL_M2_RAM_HALF	0x80	/* populated by half-banks */
#define	SL_M2_FILLED	7		/* number of banks filled */
#define	SL_M2_ACPTB	8		/* first register defining good exp */
#define	SL_M2_LAST_ACPTB	11	/* last register defining good exp */
#define	SL_M2_NUMACPT	(SL_M2_LAST_ACPTB-SL_M2_ACPTB+1)

#define	SL_M2_ACPTB_VAL1(x)	(((x) << 4) & MEM_EXP_CHECK)
#define	SL_M2_ACPTB_VAL2(x)	((x) & MEM_EXP_CHECK)

/* Multibus Adaptor specific registers */
#define SL_A_CSR	34
#define		SLB_S0		0x01	/* select 1/4Mb range of Multibus mem */
#define		SLB_S1		0x02
#define		SLB_A0		0x04	/* select MB of I/O space */
#define		SLB_A1		0x08
#define		SLB_EN_BERR	0x10	/* disable bus errors */
#define 	SLB_EN_MBA	0x20	/* enable MBIF resp; no bus affect */

/* SCSI/E board specific registers */
#define SL_S_ETH_CRC0	4		/* LSB of Ether CRC */
#define SL_S_ETH_CRC1	5		/* MSB of Ether CRC */
#define SL_S_ETH_ADD0	6		/* LSB of Ether Address */
#define SL_S_ETH_ADD1	7		/* 5th MSB of Ether Address */
#define SL_S_ETH_ADD2	8		/* 4th MSB of Ether Address */
#define SL_S_ETH_ADD3	9		/* 3rd MSB of Ether Address */
#define SL_S_ETH_ADD4	10		/* 2nd MSB of Ether Address */
#define SL_S_ETH_ADD5	11		/* MSB of Ether Address */
#define	SL_S_FLAGS	12		/* slave register of flags */
#define		SLF_FUJITSU	0x01	/* Fujitsu 8795B on SCED */
#define SL_S_DIAG_STAT	34		/* diagnostic status register */
#define SL_S_RESET	35		/* hard reset of SCSI/Ether board */

/* Clock/Arbitration board specific registers */
#define SL_C_TRIG	34		/* W scope trigger */
#define SL_C_FPTYPE	36		/* Front panel type */
#define SL_C_DIAG_CTRL	37		/* Diagnostics and control register */
#define		SLB_DCMASK	0x0F	/* Mask for useful bits */
#define		SLB_EN_BE	0x01	/* Enable bus error reporting */
#define		SLB_OPT_PRI	0x02	/* Use optional slot priorities */
#define		SLB_EN_DIAG	0x04	/* Enable diagnostic bus requests */
#define		SLB_DIAG_VGNT	0x08	/* Valid grant clock for diagnostics */
#define SL_C_LOPRI_GNT	38		/* Encoded low-priority bus grant */
#define		SLB_GNTMASK	0x0F	/* Mask for encoded grants */
#define		SLB_UPPERGNT	0x08	/* Select upper eight grants */
#define		SLB_GSEL	0x10	/* This priority's group select */
#define		SLB_XPENABLE	0x20	/* This priority's enable - INVERTED! */
#define		SLB_LOWERGNT	0x40	/* Select lower eight grants */
#define		SLB_XPEQUAL	0x80	/* This priority's equal - INVERTED! */
#define SL_C_HIPRI_GNT	39		/* Encoded high-priority bus grant */
					/* Uses same bit masks as register 38 */
#define SL_C_LOPRI_REQ0	40		/* Low-priority bus requests 0-7 */
#define SL_C_LOPRI_REQ1	41		/* Low-priority bus requests 8-15 */
#define SL_C_HIPRI_REQ0	42		/* High-priority bus requests 0-7 */
#define SL_C_HIPRI_REQ1	43		/* High-priority bus requests 8-15 */
#define SL_C_DM_START0	44		/* Start data mover using table 0 */
#define SL_C_DM_START1	45		/* Start data mover using table 1 */
#define SL_C_DM_BA0	46		/* LSB data mover base address */
#define SL_C_DM_BA1	47		/* MSB data mover base address */

#define SL_C_SYSID0	56		/* LSB of system ID number */
#define SL_C_SYSID1	57		/* 3rd MSB of system ID number */
#define SL_C_SYSID2	58		/* 2nd MSB of system ID number */
#define SL_C_SYSID3	59		/* MSB of system ID number */
#define	SL_C_BP55	60		/* Should always be 0x55 */
#define	SL_C_BPAA	61		/* Should always be 0xAA */
#define	SL_C_CHKSUM	62		/* 2's comp checksum of SYSID prom */
/*
 * There are 48 leds starting at slic address SL_C_FP_LIGHT
 * Each pair of addresses represent 1 led. The even turns off the led.
 * The odd turns on the led.
 */
#define	SL_C_FP_LIGHT		128		/* Front panel led array */
#define	SL_C_IO_ACTIVE		229		/* I/O (disk) activity led */
#define	SL_C_IO_ONLINE		225		/* I/O (disk) online led */
#define	SL_C_IO_ERROR		227		/* I/O (disk) error led */

/* ZDC board specific registers */
#define	SL_Z_VERSION	 4		/* FW Generation */
#define	SL_Z_CNTRL	64		/* SLIC to HSC Control register */
#define		SLB_COMM	0x0F	/* mask for comm lines */
#define		SLB_UNRESET	0x10	/* 0 - reset, 1 - unreset */
#define		SLB_MODEMASK	0xC0	/* HSC Mode mask */
#define		SLB_STOP_EW	0x40	/* Stop, enable WREG on YBUS */
#define		SLB_RUN_SP	0x80	/* Run, Stop on Pause */
#define		SLB_RUN_RP	0xC0	/* Run, Run on Pause */
#define	SL_Z_STATUS	SL_Z_CNTRL	/* SLIC Status register */
#define		SLB_ZPARERR	0x10	/* ZDC Parity Error */
#define		SLB_HSCRUNNING	0x20	/* HSC Running */
#define		SLB_COMM0	0x40	/* Comm0 to HSC */
#define		SLB_EEREADY	0x80	/* EEPROM Ready */
#define	SL_Z_UPC0_3	65		/* 2910 uPC address bits 0-3 */
#define	SL_Z_UPC4_A	66		/* 2910 uPC address bits 4-10 */
#define	SL_Z_EEBANK	67		/* Most significant address bits 7-14 */
#define	SL_Z_WREG0_3	69		/* WREG address bits 0-3 */
#define	SL_Z_WREG4_A	70		/* WREG address bits 4-10 */
#define	SL_Z_SCANSR	96		/* SCAN Shift Register */
#define	SL_Z_SHADTOWCS	97		/* Shadow to WCS command */
#define	SL_Z_PREGTOSHAD	98		/* PREG to Shadow command */
#define	SL_Z_KBUSTOSHAD	99		/* KBUS to Shadow command */
#define	SL_Z_SHADTOPREG	101		/* Shadow to PREG command */
#define	SL_Z_WCSTOPREG	102		/* WCS data to PREG command */
#define	SL_Z_STARTHSC	103		/* Start HSC command */
#define	SL_Z_EEWINDOW	128		/* Base 128 byte window into EEPROM */

#endif	/* _SQT_SLIREG_H_ */
