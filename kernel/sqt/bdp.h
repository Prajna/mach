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
 * $Log:	bdp.h,v $
 * Revision 2.3  91/07/31  17:59:34  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  12:52:42  dbg
 * 	Added from Sequent SYMMETRY sources.
 * 	[91/04/26  14:48:02  dbg]
 * 
 */

/*
 * $Header: bdp.h,v 2.3 91/07/31 17:59:34 dbg Exp $
 *
 * Buffered Data Path (BDP) sub-registers and bits.
 */

/*
 * Revision 1.1  89/07/19  14:48:50  kak
 * Initial revision
 * 
 */

#ifndef	_SQT_BDP_H_
#define	_SQT_BDP_H_

/*
 * BDP ID Register: Read Only.
 */

#define	BDP_ID		0x00
#define	    BDPI_VAL		0x02	/* what the BDP says it is */

/*
 * BDP Revision register: Read only
 */
#define	BDP_VERSION	0x01		/* version and revision number */
#define	    BDPV_VER_MASK	0xF0	/* version number mask */
#define	    BDPV_REV_MASK	0x0F	/* revision number mask */

#define	    BDPV_REVISION(x)	((x) & BDPV_REV_MASK)
#define	    BDPV_VERSION(x)	(((x) & BDPV_VER_MASK) >> 4)

/*
 * BDP Command Register:  Write Only.
 */

#define	BDP_CMD		0x02		/* command latch */
#define	    BDPC_MASK		0x03	/* useful bits */
#define	    BDPC_EXECDIAG	0x02	/* execute diag latch contents */
#define	    BDPC_RESET		0x01	/* reset BDP */

/*
 * BDP Config Register:  Write Only.
 */

#define	BDP_CONF	0x03		/* configuration latch register */
#define	    BDPCF_MASK		0xFF	/* useful bits */
#define	    BDPCF_INH_ETC	0x80	/* inhibit ETC counting */
#define	    BDPCF_ENAB_PAR	0x40	/* enable data parity checking */
#define	    BDPCF_HIGH		0x20	/* BDP connected to bytes 7-4 */
#define	    BDPCF_MEM		0x10	/* special SGS mem behavior */
#define	    BDPCF_DISAB_DIAG	0x08	/* disable diagnostic mode */
#define	    BDPCF_TSIZ		0x07	/* transfer size select ... */
#define		BDPCF_TSIZ_W32	0x00	/* wide 32 bytes */
#define		BDPCF_TSIZ_W16	0x03	/* wide 16 bytes */
#define		BDPCF_TSIZ_N16	0x04	/* narrow 16 bytes */
#define		BDPCF_TSIZ_4	0x06	/* 4 bytes */
#define		BDPCF_TSIZ_N8	0x07	/* narrow 8 bytes */

/*
 * BDP Throttle Latch: Write Only.
 */

#define	BDP_THROTTLE	0x04

/*
 * BDP Diagnostic Latches:  Write Only.
 */

#define	BDP_DIAG1	0x05
#define	    BDPD1_MASK		0x7F	/* useful bits */
#define	    BDPD1_DDEST		0x60	/* data destination selection... */
#define		BDPD1_DDEST_OUTQ0	0x00	/* OUTQ0 */
#define		BDPD1_DDEST_OUTQ1	0x20	/* OUTQ1 */
#define		BDPD1_DDEST_ETC		0x40	/* ETC */
#define		BDPD1_DDEST_NONE	0x60	/* none */
#define	    BDPD1_SEL		0x10		/* BDP select */
#define	    BDPD1_DSRC		0x0C		/* data source selection... */
#define		BDPD1_DSRC_INQ2	0x00		/* INQ2 */
#define		BDPD1_DSRC_INQ3	0x04		/* INQ3 */
#define		BDPD1_DSRC_ETC	0x08		/* elapsed time counter */
#define		BDPD1_DSRC_NONE	0x0C		/* none */
#define	    BDPD1_BASE_LD	0x02
#define	    BDPD1_MASK_LD	0x01

#define	BDP_DIAG2	0x06
#define	    BDPD2_MASK		0xFF	/* useful bits */
#define	    BDPD2_INQ0_RD	0x80	/* increment INQ0 read pointer */
#define	    BDPD2_INQ1_RD	0x40	/* increment INQ1 read pointer */
#define	    BDPD2_LADR_LD	0x20	/* diag lock address strobe */
#define	    BDPD2_WADR_LD	0x10	/* diag write address strobe */
#define	    BDPD2_RADR_LD	0x08	/* diag read address strobe */
#define	    BDPD2_ASRC		0x07	/* address source selection... */
#define		BDPD2_ASRC_INQ0		0x00	/* INQ0 */
#define		BDPD2_ASRC_INQ1		0x01	/* INQ1 */
#define		BDPD2_ASRC_AR		0x02	/* addr reg */
#define		BDPD2_ASRC_ARINC	0x03	/* addr reg (incr) */
#define		BDPD2_ASRC_SLIC		0x05	/* SLIC data latch */
#define		BDPD2_ASRC_ETC		0x06	/* PROC: ETC (init) */
#define		BDPD2_ASRC_RAR		0x06	/* MEM: RAR; bit 4 inverted */
#define		BDPD2_ASRC_ETC_REF	0x07	/* ETC (refresh) */

#define	BDP_DIAG3	0x07
#define	    BDPD3_MASK		0xFF
#define	    BDPD3_INQ0_LD	0x80
#define	    BDPD3_INQ1_LD	0x40
#define	    BDPD3_INQ2_WR	0x20
#define	    BDPD3_INQ3_WR	0x10
#define	    BDPD3_GO		0x08
#define	    BDPD3_GOSEL		0x07	/* which output is on bus drivers */
#define		BDPD3_GOSEL_WAR		0x00	/* write address register */
#define		BDPD3_GOSEL_OUTQ1	0x01	/* OUTQ1 */
#define		BDPD3_GOSEL_RAR		0x02	/* read address register */
#define		BDPD3_GOSEL_LAR		0x03	/* lock address register */
#define		BDPD3_GOSEL_OUTQ0	0x04	/* OUTQ0 */

/*
 * Data Parity Latch: Read/Write.
 */

#define	BDP_DATAPAR	0x08
#define	    BDPDP_MASK	0xFF		/* useful bits */
#define	    BDPDP_DPAR	0xF0		/* data parity error */
#define	    BDPDP_FORCE	0x0F		/* data parity error to force */

/*
 * System Parity Latch: Read/Write.
 */

#define	BDP_SYSPAR	0x09
#define	    BDPSP_MASK	0xFF		/* useful bits */
#define	    BDPSP_SPAR	0xF0		/* system parity error */
#define	    BDPSP_FORCE	0x0F		/* system parity error to force */

/*
 * System Address Parity Latch: Read/Write.
 */

#define	BDP_SYSPARAD	0x0A
#define	    BDPSPA_MASK		0xC7	/* useful bits */
#define	    BDPSPA_SAPAR	0x80	/* sysad parity error */
#define	    BDPSPA_FORCE	0x40	/* sysad parity error to force */
#define	    BDPSPA_LHIT		0x04	/* lock hit r/w */
#define	    BDPSPA_RHIT		0x02	/* read hit r/w */
#define	    BDPSPA_AHIT		0x01	/* address hit r/w */

/*
 * Read Bytes: Read Only.
 * Used in combination with the Read Byte Groups
 * and to get to the diagnostic data latches.
 */

#define	BDP_BYTE4	0x0B
#define	    BDP_CYCTYPE_HI	0xF0	/* cycle type (6:3) */
#define	    BDP_BM	0x0F		/* byte marks */
#define	BDP_BYTE3	0x0C
#define	BDP_BYTE2	0x0D
#define	BDP_BYTE1	0x0E
#define	BDP_BYTE0	0x0F
#define	    BDP_BYTE0_ADDR	0xFC	/* address bits 7-2 */
#define	    BDP_CYCTYPE_LO	0x03	/* cycle type (2:1) */

#define	BDP_NBYTES	(BDP_BYTE0 - BDP_BYTE4 + 1)	/* no. of read bytes */

/*
 * Read Byte Groups: Read Only.
 *
 * Each of the registers below defines
 * the group of BDP registers used to
 * read the specified quantities.
 * For example,
 *	rdSubslave(slic, PROC_BDP_LO, BDP_INQ_DATA | BDP_BYTE3)
 * reads byte three from the BDP
 * Input Queue Data Mux.
 *
 * Macros should be provided to handle
 * the usual cases for reading these groups.
 */

#define	BDP_INQ_DATA	0x10		/* input queue data mux */
#define	BDP_INQ_ADDR	0x20		/* input queue address mux */
#define	BDP_WAR		0x30		/* write address register (for ECC) */
#define	BDP_BASE_ADDR	0x40		/* base address latch */
#define	BDP_MASK_ADDR	0x50		/* mask address latch */
#define	BDP_QUEUE_PTRS	0x60		/* queue pointers */
#define	BDP_GOSEL_MUX	0x70		/* go select MUX */

/*
 * Generic masks for bit manipulations
 */
#define	BITS72		0xFC		/* 7:2 bits of a byte */

/*
 * Input queue address mux byte mark macro
 */
#define	BDP_IQA_BM(bm)		((bm) & BM_MASK)

/*
 * Input queue zero flag (4:0) macros
 */
#define	BDP_IQZ_WR(flg)		(CYCLE3(flg))
#define	BDP_IQZ_INV(flg)	(~(CYCLE6(flg) ^ CYCLE3(flg)))

/*
 * BDP_IQZ_SZ:
 * Size bit combinations (5:4:1 for bits 2:0) are: 
 *				32Bw	001
 *				16Bw	011
 *				16Bn	100
 *				4Bn	110
 *				8Bn	111
 */
#define	BDP_IQZ_SZ(flg)	( (CYCLE5(flg)|CYCLE4(flg)) >>3 | (CYCLE1(flg)>>1) )

/*
 * Extract queue pointers from a BDP.
 */
#define	BDPQP_INQ1_WR(slic, slave) \
	(((rdSubslave((slic), (slave), BDP_QUEUE_PTRS + 0xE) & 1) << 2) \
	 | ((rdSubslave((slic), (slave), BDP_QUEUE_PTRS + 0xF) >> 6) & 0x3))

#define	BDPQP_INQ1_RD(slic, slave) \
	((rdSubslave((slic), (slave), BDP_QUEUE_PTRS + 0xE) >> 1) & 0x7)

/*
 * Gosel byte mark macro
 */
#define	BDP_GS_BM(bm)		(((bm) & 0x0F) << 4)

/*
 * Base address and mask address byte mark macros
 */
#define	BDP_BAMA_BM(bm)		((bm) & 0x0F)

/*
 * BDP data byte latch macros
 */

/* Queue input 		NOTE: addr is type (u_int)	*/
#define	BDP_BYTE4Q(ct, bm)	((u_char)((CYCLE63(ct) << 1) | (bm & 0x0F)))
#define	BDP_BYTE3Q(addr)	(((u_char *)&(addr)) [3])	/* get byte 3 */
#define	BDP_BYTE2Q(addr)	(((u_char *)&(addr)) [2])	/* get byte 2 */
#define	BDP_BYTE1Q(addr)	(((u_char *)&(addr)) [1])	/* get byte 1 */
#define	BDP_BYTE0Q(addr, ct)	((((u_char *)&(addr)) [0] & BITS72)|(CYCLE21(ct) >> 1))

/* Address Mux 		NOTE: addr is type (u_int)	*/
#define	BDP_BYTE4A(bm)		((u_char)(bm & 0x0F))
#define	BDP_BYTE3A(addr)	(((u_char *)&(addr)) [3])	/* get byte 3 */
#define	BDP_BYTE2A(addr)	(((u_char *)&(addr)) [2])	/* get byte 2 */
#define	BDP_BYTE1A(addr)	(((u_char *)&(addr)) [1])	/* get byte 1 */
#define	BDP_BYTE0A(addr)	(((u_char *)&(addr)) [0] & BITS72)

/* Data Mux 		NOTE: data is type (u_int)	*/
#define	BDP_BYTE4D(rr)		((u_char)((rr) & RR_BITS31))
#define	BDP_BYTE3D(data)	(((u_char *)&(data)) [3])	/* get byte 3 */
#define	BDP_BYTE2D(data)	(((u_char *)&(data)) [2])	/* get byte 2 */
#define	BDP_BYTE1D(data)	(((u_char *)&(data)) [1])	/* get byte 1 */
#define	BDP_BYTE0D(data)	(((u_char *)&(data)) [0])	/* get byte 0 */


/*
 * Bits used after BDPD2_ASRC_ARINC
 */
#define	BDPB0_COUNTER	0x03		/* counter in byte 0 */

/*
 * Gather the cycle type bits together from
 * bytes 4 and 0 of a group.
 */

#define	BDP_CYCTYPE(byte4,byte0) \
	( (((byte4) & BDP_CYCTYPE_HI) >> 2) | ((byte0) & BDP_CYCTYPE_LO) )

#endif	/* _SQT_BDP_H_ */
