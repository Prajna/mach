/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * Copyright (c) 1991 IBM Corporation 
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation,
 * and that the name IBM not be used in advertising or publicity 
 * pertaining to distribution of the software without specific, written
 * prior permission.
 * 
 * CARNEGIE MELLON AND IBM ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON AND IBM DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	if_trreg.h,v $
 * Revision 2.2  93/02/04  08:00:27  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */

/* $Header: if_trreg.h,v 2.2 93/02/04 08:00:27 danner Exp $ */
/* $ACIS:if_lanreg.h 12.0$ */

#if !defined(lint) && !defined(LOCORE)  && defined(RCS_HDRS)
static char *rcsidif_lanreg = "$Header: if_trreg.h,v 2.2 93/02/04 08:00:27 danner Exp $";
#endif

/* Memory offsets of adapter control areas */

/* Window for the shared ram */
#define TR_SRAM_WINDOW  0xc0000

/* Offset of MMIO region */
#define TR_MMIO_OFFSET	0x80000 

#define	TR_ACA_OFFSET	0x1e00		/* Offset of ACA in MMIO region */


/* Base address of global interrupt release register */
#define	GLOBAL_INT_REL		0x2f0

/* Bring-Up Test results */

#define	BUT_OK			0x0000	/* Initialization completed OK */
#define	BUT_PROCESSOR_FAIL	0x0020	/* Failed processor initialization */
#define	BUT_ROM_FAIL		0x0022	/* Failed ROM test diagnostic */
#define	BUT_RAM_FAIL		0x0024	/* Failed RAM test diagnostic */
#define	BUT_INST_FAIL		0x0026	/* Failed instruction test diag. */
#define	BUT_INTER_FAIL		0x0028	/* Failed interrupt test diagnostic */
#define	BUT_MEM_FAIL		0x002a	/* Failed memory interface diag. */
#define	BUT_PROTOCOL_FAIL	0x002c	/* Failed protocol handler diag. */


/* Direct PC-to-adapter commands */

#define	DIR_INTERRUPT		0x00	/* Cause adapter to interrupt the PC */
#define	DIR_MOD_OPEN_PARAMS	0x01	/* Modify open options */
#define DIR_RESTORE_OPEN_PARMS	0x02	/* Restore open options */
#define	DIR_OPEN_ADAPTER	0x03	/* Open the adapter card */
#define	DIR_CLOSE		0x04	/* Close adapter card */
#define	DIR_SET_GRP_ADDR	0x06	/* Set adapter group address */
#define	DIR_SET_FUNC_ADDR	0x07	/* Set adapter functional addr */
#define	DIR_READ_LOG		0x08	/* Read and reset error counters */
#define XMIT_DIR_FRAME		0x0a	/* Direct station transmit */


/* Adapter-Card-to-PC commands */

#define	REC_DATA		0x81	/* Data received from ring station */
#define	XMIT_DATA_REQ		0x82	/* Adapter needs data to xmit */
#define	DLC_STATUS   		0x83    /* DLC status has changed */
#define	RING_STAT_CHANGE	0x84	/* Adapter has new ring-status info */


/* Open options */

#define	OPEN_WRAP		0x8000	/* Wrap xmit data to receive data */
#define	OPEN_NO_HARD_ERR	0x4000	/* Ring hard error and xmit beacon */
					/* conditions do not cause interrupt */
#define	OPEN_NO_SOFT_ERR	0x2000	/* Ring soft errors do not cause */
					/* interrupt */
#define	OPEN_PASS_MAC		0x1000	/* Pass all adapter-class MAC frames */
					/* received but not supported by the */
					/* adapter */
#define	OPEN_PASS_ATTN_MAC	0x0800	/* Pass all attention-class MAC */
					/* frames != the previously received */
					/* attention MAC frame */
#define	OPEN_PASS_BCON_MAC	0x0100	/* Pass the first beacon MAC frame */
					/* and all subsequent beacon MAC */
					/* frames that have a change in */
					/* source address or beacon type */
#define	OPEN_CONT		0x0080	/* Adapter will participate in */
					/* monitor contention */

#define	NUM_RCV_BUF		16	/* Number of receive buffers in */
					/* shared RAM needed for adapter to */
					/* open */
#define	RCV_BUF_LEN		136	/* Length of each receive buffer */
#define	RCV_BUF_DLEN		RCV_BUF_LEN - 8	/* Length of data in rec buf */
#define	DHB_LENGTH		1544	/* Length of each transmit buffer */
#define	NUM_DHB			2	/* Number of transmit buffers */
#define	DLC_MAX_SAP		0	/* MAX number of SAPs */
#define DLC_MAX_STA		0	/* MAX number of link stations */
#define	DLC_MAX_GSAP		0	/* MAX number of group SAPs */
#define	DLC_MAX_GMEM		0	/* MAX number of SAPs that can be */
					/* assigned to any given group */
#define	DLC_TICK		0	/* Zero selects default of 40ms */


/* Open return codes */

#define	OPEN_OK			0x00	/* Open completed successfully */
#define	OPEN_BAD_COMMAND	0x01	/* Invalid command code */
#define	OPEN_ALREADY		0x03	/* Adapter is ALREADY open */
#define	OPEN_MISSING_PARAMS	0x05	/* Required paramaters missing */
#define	OPEN_UNRECOV_FAIL	0x07	/* Unrecoverable failure occurred */
#define	OPEN_INAD_REC_BUFS	0x30	/* Inadequate receive buffers */
#define	OPEN_BAD_NODE_ADDR	0x32	/* Invalid NODE address */
#define	OPEN_BAD_REC_BUF_LEN	0x33	/* Invalid receive buffer length */
#define	OPEN_BAD_XMIT_BUF_LEN	0x43	/* Invalid transmit buffer length */

/* Base address of Token-Ring adapter shared RAM */
#define	TR_RAM_BASE	0xd4


/* Bit definitions of ISRA High Byte, (Adapter Status)  */
#define	TIMER_STAT	0x40	/* A Timer Control Reg. has an interrupt */
#define	ACCESS_STAT	0x20	/* Shared RAM or MMIO access violation */
#define	DEADMAN_TIMER	0x10	/* The deadman timer has expired */
#define PROCESSOR_CK	0x08	/* Adapter Processor Check */
#define	H_INT_MASK	0x02	/* When on, no adapter hardware interrupts */
#define	S_INT_MASK	0x01	/* When on, no adapter software interrupts */

/* Bit definitions of ISRA Low Byte, (Used by PC to interrupt adapter) */
#define	CMD_IN_SRB	0x20	/* Inform adapter of command in SRB */
#define	RESP_IN_ASB	0x10	/* Inform adapter of response in ASB */
#define	SRB_FREE	0x08	/* Inform PC when SRB is FREE */
#define	ASB_FREE	0x04	/* Inform PC when ASB is FREE */
#define	ARB_FREE	0x02	/* Inform adapter ARB is FREE */
#define	SSB_FREE	0x01	/* Inform adapter SSB is FREE */

/* Bit definitions of ISRP High Byte, (PC interrupts and interrupt control) */
#define	NMI_INT_CTL	0x80	/* 1 = all interrupts to PC interrupt level */
				/* 0 = error and timer interrupts to PC NMI */
#define	INT_ENABLE	0x40	/* Allow adapter to interrupt the PC */
#define	TCR_INT		0x10	/* Timer Control Reg. has interrupt for PC */
#define	ERR_INT		0x08	/* Adap machine check, deadman timer, overrun */
#define	ACCESS_INT	0x04	/* Shared RAM or MMIO access violation */
#define	SHARED_INT_BLK	0x02	/* Shared interrupt blocked */
#define	PRIM_ALT_ADDR	0x01	/* 0 = primary adapter address */
				/* 1 = alternate adapter address */

/* Bit definitions of ISRP Low Byte, (PC interrupts) */
#define	ADAP_CHK_INT	0x40	/* The adapter has an unrecoverable error */
#define	SRB_RESP_INT	0x20	/* Adapter has placed a response in the SRB */
#define	ASB_FREE_INT	0x10	/* Adapter has read response in ARB */
#define	ARB_CMD_INT	0x08	/* ARB has command for PC to act on */
#define	SSB_RESP_INT	0x04	/* SSB has response to previous SRB command */


/* Received frame message types */
#define	MAC	0x02	/* MAC frame */
#define	UI	0x06	/* Unsequenced Information */
#define	UNIDENT	0x14	/* Other or unidentified message type */

/* Constants for Token-Ring physical header */
#define	TR_ADDR_LEN	0x6	/* Length of a token-ring physical address */
#define	DLC_HDR_LEN	0x3	/* Length of DLC header */
#define	AC		0x10	/* Value of access control field */
#define	FC		0x40	/* Value of frame control field */
#define	UI_CMD		0x3	/* Unsequenced Information Command */
#define	SNAP_LENGTH     0x05	/* SNAP field length */              
				/* protocol id = 3 bytes */
				/* ethertype = 2 bytes */
#define HDR_LNGTH_NOROUTE 14    /* length of header with no route info */
#define SKIP_DSAP_SSAP    0x02  /* length of dsap and ssap in llc frame */

/* SAP DLC SRB commands (page 6-50 Token Ring Tech. Ref.) */
#define OPEN_SAP        0x15    /* activate service access point */
#define CLOSE_SAP	0x16	/* de-activate SAP */
#define XMIT_UI_FRM 	0x0d	/* transmit unnumbered info frame */
#define XMIT_XID_CMD	0x0e	/* transmit XID command */
#define XMIT_TEST_CMD	0x11	/* transmit TEST command */

/* EXTENDED SAP STUFF */
#define EXTENDED_SAP 	0xaa	/* extended service access point */

/* BRIDGE STUFF */
#define TR_RI_PRESENT		0x80	/* routing info present bit */
#define TR_RCF_LEN_MASK		0x1f00
#define	TR_RCF_BROADCAST	0x8000
#define TR_RCF_FRAME2K		0x0020
#define TR_MAX_BRIDGE		8
#define SOURCE_ADDR_BYT0	8	/* byte zero of source address */
#define RCF_BYT0		14	/* byte zero of RCF fields */

/* ARB RING STATUS CHANGE */
#define SIGNAL_LOSS	0x8000	/* signal loss */
#define HARD_ERR	0x4000	/* beacon frames sent */
#define SOFT_ERR   	0x2000  /* soft error */
#define LOBE_FAULT 	0x0800  /* lobe wire fault */
#define LOG_OFLOW	0x0080	/* adapter error log counter overflow */
#define SINGLE_STATION	0x0040	/* single station or ring */
