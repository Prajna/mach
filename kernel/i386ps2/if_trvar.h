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
 * $Log:	if_trvar.h,v $
 * Revision 2.2  93/02/04  08:00:33  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */

/* $Header: if_trvar.h,v 2.2 93/02/04 08:00:33 danner Exp $ */
/* $ACIS:if_lanvar.h 12.0$ */

#if !defined(lint) && !defined(LOCORE)  && defined(RCS_HDRS)
static char    *rcsidif_lanvar = "$Header: if_trvar.h,v 2.2 93/02/04 08:00:33 danner Exp $";
#endif

/*
 * This file contains structures used in the "tr" driver for the
 *		IBM TOKEN-RING NETWORK PC ADAPTER
 */


/*
 * Adapter PIO registers 
 */
struct trdevice {
	unsigned char   switch_read;	/* ROM/MMIO domain switches */
	unsigned char   reset;	/* write causes adapter reset */
	unsigned char   release_reset;	/* write releases adapter reset */
	unsigned char   clear_interrupt;	/* write clears adapter
						 * interrupt */
};




/* ACA registers */
struct aca_reg {
	union {			/* RAM Relocation Register */
#define	rrr	aca_RRR.RRR	/* Whole register */
#define	rrr_h	aca_RRR.b.RRRh	/* High byte */
#define	rrr_l	aca_RRR.b.RRRl	/* Low byte */
		unsigned short  RRR;
		struct {
			unsigned char   RRRh;
			unsigned char   RRRl;
		}               b;
	}               aca_RRR;
	union {			/* Write Region Base management register */
#define	wrb	aca_WRB.WRB	/* Whole register */
#define wrb_h	aca_WRB.b.WRBh	/* High byte */
#define	wrb_l	aca_WRB.b.WRBl	/* Low byte */
		unsigned short  WRB;
		struct {
			unsigned char   WRBh;
			unsigned char   WRBl;
		}               b;
	}               aca_WRB;
	union {			/* Write Window Open management register */
#define	wwo	aca_WWO.WWO	/* Whole register */
#define wwo_h	aca_WWO.b.WWOh	/* High byte */
#define wwo_l	aca_WWO.b.WWOl	/* Low byte */
		unsigned short  WWO;
		struct {
			unsigned char   WWOh;
			unsigned char   WWOl;
		}               b;
	}               aca_WWO;
	union {			/* Write Window Close management register */
#define wwc	aca_WWC.WWC	/* Whole register */
#define	wwc_h	aca_WWC.b.WWCh	/* High byte */
#define	wwc_l	aca_WWC.b.WWCl	/* Low byte */
		unsigned short  WWC;
		struct {
			unsigned char   WWCh;
			unsigned char   WWCl;
		}               b;
	}               aca_WWC;
	union {			/* Interrupt Status Register-PC */
#define	isrp	aca_ISRP.ISRP	/* Whole register */
#define	isrp_h	aca_ISRP.b.ISRPh/* High byte */
#define	isrp_l	aca_ISRP.b.ISRPl/* Low byte */
		unsigned short  ISRP;
		struct {
			unsigned char   ISRPh;
			unsigned char   ISRPl;
		}               b;
	}               aca_ISRP;
	union {			/* Interrupt Status Register-Adapter */
#define	isra	aca_ISRA.ISRA	/* Whole register */
#define	isra_h	aca_ISRA.b.ISRAh/* High byte */
#define	isra_l	aca_ISRA.b.ISRAl/* Low byte */
		unsigned short  ISRA;
		struct {
			unsigned char   ISRAh;
			unsigned char   ISRAl;
		}               b;
	}               aca_ISRA;
	union {			/* Timer Control Register */
#define	tcr	aca_TCR.TCR	/* Whole register */
#define	tcr_h	aca_TCR.b.TCRh	/* High byte */
#define	tcr_l	aca_TCR.b.TCRl	/* Low byte */
		unsigned short  TCR;
		struct {
			unsigned char   TCRh;
			unsigned char   TCRl;
		}               b;
	}               aca_TCR;
	union {			/* Timer Value Register */
#define	tvr	aca_TVR.TVR	/* Whole register */
#define	tvr_h	aca_TVR.b.TVRh	/* High byte */
#define	tvr_l	aca_TVR.b.TVRl	/* Low byte */
		unsigned short  TVR;
		struct {
			unsigned char   TVRh;
			unsigned char   TVRl;
		}               b;
	}               aca_TVR;
};




/*
 * Adapter Attachment Control Area (ACA)
 */
struct aca {
	struct aca_reg  rw;	/* Read/write access to ACA */
	char            space0[16];
	struct aca_reg  reset;	/* Reset access to ACA */
	char            space1[16];
	struct aca_reg  set;	/* Set access to ACA */
};




/*
 * Structure of SSB (System Status Block)
 */
struct sr_ssb {
	unsigned char   command;/* The xmit command from the SRB */
	unsigned char   cmd_corr;	/* PC/Adapter command correlator */
	unsigned char   retcode;/* Completion code */
	unsigned char   res0;	/* reserved */
	unsigned short  station_id;	/* ID of station providing status */
	unsigned char   xmit_err;	/* The FS byte if retcode=0x22 */
};



/*
 * Structure of ARB (Adapter Request Block)
 */
union sr_arb {
	struct {		/* Receive-data command block */
		unsigned char   command;
		unsigned char   res0[3];
		unsigned short  station_id;	/* ID of receiving station */
		unsigned short  buf_addr;	/* RAM offset of 1st rec buf */
		unsigned char   lan_hdr_len;	/* Length of LAN header */
		unsigned char   dlc_hdr_len;	/* Length of DLC header */
		unsigned short  frame_len;	/* Length of entire frame */
		unsigned char   msg_type;	/* Category of message */
	}               rec;
	struct {		/* Ring-Status_Change information block */
		unsigned char   command;
		unsigned char   res0[5];	/* reserved */
		unsigned short  ring_status;	/* Current ring status */
	}               stat;
	struct {		/* Transmit-data-request cmd and response
				 * block */
		unsigned char   command;
		unsigned char   cmd_corr;	/* Command correlator */
		unsigned char   res0[2];	/* reserved */
		unsigned short  station_id;	/* ID of sending station */
		unsigned short  dhb_addr;	/* Addr of Data Holding Buf */
	}               xmit;
	struct {		/* DLC status change response block */
		unsigned char   command;
		unsigned char   res0[3];	/* reserved */
		unsigned short  station_id;	/* ID of sending station */
		unsigned short  status;	/* status info field */
		unsigned char   frmr_data[5];	/* ? */
		unsigned char   acc_priority;
		unsigned char   rem_address[6];	/* remote address */
		unsigned char   rsap;	/* remote sap */
	}               dlc;
};




/*
 * Contents of SRB after adapter reset
 */
struct init_resp {
	union {
		unsigned char   top[2];
		unsigned short  align;	/* force short alignment of struct */
	}               s;
#define init_command s.top[0];	/* should be 0x80: init complete */
	unsigned char   res0[4];/* reserved */
	unsigned short  bring_up_res;	/* bring up code result */
	unsigned short  encoded_addr;	/* RAM address of adapter's */
	/* permanent encoded address */
	unsigned short  level_addr;	/* RAM address of adapter's */
	/* microcode level */
	unsigned short  adap_addr;	/* Address of adapter address */
	unsigned short  params_addr;	/* address of adapter parameters */
	unsigned short  mac_addr;	/* address of adapter MAC buffer */
};




/*
 * Structure of SRB (System Request Block)
 */
union sr_srb {
	struct init_resp reset_resp;	/* Adapter card reset response block */
	struct {		/* Open command block */
		unsigned char   command;
		unsigned char   res0[7];	/* reserved */
		unsigned short  open_options;	/* Open options */
		unsigned char   node_addr[6];	/* Adapter's ring address */
		unsigned char   group_addr[4];	/* The group address to set */
		unsigned char   funct_addr[4];	/* Functional address to set */
		unsigned short  num_rcv_buf;	/* Number of receive buffers */
		unsigned short  rcv_buf_len;	/* Length of receive buffers */
		unsigned short  dhb_length;	/* Length of xmit buffers */
		unsigned char   num_dhb;	/* Number of DHBs */
		unsigned char   res1;	/* reserved */
		unsigned char   dlc_max_sap;	/* Max number of SAPs */
		unsigned char   dlc_max_sta;	/* Max num of link stations */
		unsigned char   dlc_max_gsap;	/* Max number of group SAPs */
		unsigned char   dlc_max_gmem;	/* Max members per group SAP */
		unsigned char   dlc_t1_tick_one;	/* Timer T1 interval grp
							 * one */
		unsigned char   dlc_t2_tick_one;	/* Timer T2 interval grp
							 * one */
		unsigned char   dlc_ti_tick_one;	/* Timer TI interval grp
							 * one */
		unsigned char   dlc_t1_tick_two;	/* Timer T1 interval grp
							 * two */
		unsigned char   dlc_t2_tick_two;	/* Timer T2 interval grp
							 * two */
		unsigned char   dlc_ti_tick_two;	/* Timer TI interval grp
							 * two */
		unsigned char   product_id[18];	/* Product Identification */
	}               open_cmd;
	struct {		/* Open response block */
		unsigned char   command;
		unsigned char   res0;	/* reserved */
		unsigned char   retcode;	/* Return code */
		unsigned char   res1[3];	/* reserved */
		unsigned short  open_err_code;	/* Open error code */
		unsigned short  asb_addr;	/* ASB addr in shared RAM */
		unsigned short  srb_addr;	/* SRB addr in shared RAM */
		unsigned short  arb_addr;	/* ARB addr in shared RAM */
		unsigned short  ssb_addr;	/* SSB addr in shared RAM */
	}               open_resp;
	struct {		/* Close command and response block */
		unsigned char   command;
		unsigned char   res0;	/* reserved */
		unsigned char   retcode;	/* Return code */
	}               close;
	struct {		/* Interrupt-PC cmd and resp block */
		unsigned char   command;
		unsigned char   res0;	/* reserved */
		unsigned char   retcode;	/* Return code */
	}               intr;
	struct {		/* Mod-open-params cmd and resp block */
		unsigned char   command;
		unsigned char   res0;	/* reserved */
		unsigned char   retcode;	/* Return code */
		unsigned char   res1;	/* reserved */
		unsigned short  open_options;	/* New options */
	}               mod_params;
	struct {		/* Set-funct-addr cmd and resp block */
		/* Set-group-addr cmd and resp block */
		unsigned char   command;
		unsigned char   res0;	/* reserved */
		unsigned char   retcode;	/* Return code */
		unsigned char   res1[3];	/* reserved */
		unsigned char   new_addr[4];	/* New funct or grp addr */
	}               set_addr;
	struct {		/* Transmit cmd and response block */
		unsigned char   command;
		unsigned char   cmd_corr;	/* Command correlator */
		unsigned char   retcode;	/* Return code */
		unsigned char   res0;	/* reserved */
		unsigned short  station_id;	/* ID of sending station */
	}               xmit;
	struct {		/* open sap cmd response block */
		unsigned char   command;
		unsigned char   res0;	/* reserved */
		unsigned char   retcode;	/* return code */
		unsigned char   res1;	/* reserved */
		unsigned short  station_id;	/* ID of SAP after open */
		unsigned char   timer_t1;	/* response timer */
		unsigned char   timer_t2;	/* acknowledge timer */
		unsigned char   timer_ti;	/* inactivity timer */
		unsigned char   maxout;	/* max xmits without ack */
		unsigned char   maxin;	/* max recvs without ack */
		unsigned char   maxout_incr;	/* window increment value */
		unsigned char   maxretry;	/* N2 value ? */
		unsigned char   gsapmaxmem;	/* max saps for a group sap */
		unsigned short  max_i_field;	/* max recv info field length */
		unsigned char   sap_value;	/* sap to be opened */
		unsigned char   sap_options;	/* optiions to be set */
		unsigned char   station_cnt;	/* num of res link stations */
		unsigned char   sap_gsap_mems;	/* num of gsap members
						 * allowed */
		unsigned char   gsap1;	/* first gsap request */
	}               open_sap;
	struct {		/* Adapter card error log */
		unsigned char   command;
		unsigned char   res0;	/* reserved */
		unsigned char   retcode;	/* Set by adapter on return */
		unsigned char   res1[3];	/* reserved */
		unsigned char   data[14];	/* Log data set by adapter */
	}               log;
};



/*
 * Structure of ASB (Adapter Status Block)
 */
union sr_asb {
	struct {		/* PC response to adapter receive cmd */
		unsigned char   command;
		unsigned char   res0;	/* reserved */
		unsigned char   retcode;	/* Return code */
		unsigned char   res1;	/* reserved */
		unsigned short  station_id;	/* Receiving station ID */
		unsigned short  rec_buf_addr;	/* Receive buffer address */
	}               rec_resp;
	struct {		/* PC response to xmit-req-data cmd */
		unsigned char   command;
		unsigned char   cmd_corr;	/* Command correlator */
		unsigned char   retcode;	/* Return code */
		unsigned char   res0;	/* reserved */
		unsigned short  station_id;	/* ID of sending station */
		unsigned short  frame_len;	/* Length of entire frame */
		unsigned char   header_len;	/* Length of LAN header */
		unsigned char   rsap_value;	/* Remote SAP */
	}               xmit_resp;
};




/*
 *	Adapter addresses
 */
struct adapt_addr {
	unsigned char   node_addr[6];	/* Adapter node address */
	unsigned char   grp_addr[4];	/* Adapter group address */
	unsigned char   func_addr[4];	/* Adapter functional address */
};




/*
 * Addresses of shared RAM control blocks
 */
struct trs_cb {
	struct sr_ssb  *ssb;	/* pointer to System Status Block */
	union sr_arb   *arb;	/* pointer to Adapter Request Block */
	union sr_srb   *srb;	/* pointer to System Request Block */
	union sr_asb   *asb;	/* pointer to Adapter Status Block */
};




/*
 *	Adapter parameters
 */
struct param_addr {
	unsigned char   phys_addr[4];	/* Adapter physical address */
	unsigned char   up_node_addr[6];	/* Next active upstream node
						 * addr */
	unsigned char   up_phys_addr[4];	/* Next active upstream phys
						 * addr */
	unsigned char   poll_addr[6];	/* Last poll address */
	unsigned char   res0[2];/* Reserved */
	unsigned char   acc_priority[2];	/* Transmit access priority */
	unsigned char   src_class[2];	/* Source class authorization */
	unsigned char   att_code[2];	/* Last attention code */
	unsigned char   src_addr[6];	/* Last source address */
	unsigned char   bcon_type[2];	/* Last beacon type */
	unsigned char   major_vector[2];	/* Last major vector */
	unsigned char   ring_stat[2];	/* ring status */
	unsigned char   soft_error[2];	/* soft error timer value */
	unsigned char   fe_error[2];	/* front end error counter */
	unsigned char   next_state[2];	/* next state indicator */
	unsigned char   mon_error[2];	/* Monitor error code */
	unsigned char   bcon_xmit[2];	/* Beacon transmit type */
	unsigned char   bcon_receive[2];	/* Beacon receive type */
	unsigned char   frame_correl[2];	/* Frame correlator save */
	unsigned char   bcon_naun[6];	/* beacon station NAUN */
	unsigned char   res1[4];/* Reserved */
	unsigned char   bcon_phys[4];	/* Beacon station physical addr */
};


/* Adapter receive buffer structure */
struct rec_buf {
	unsigned short  res0;	/* reserved */
	unsigned short  buf_pointer;	/* addr of next buf plus 2, in sram */
	unsigned char   res1;	/* reserved */
	unsigned char   rec_fs;	/* FS/addr match (last buf only) */
	unsigned short  buf_len;/* length of data in this buffer */
	unsigned char   data[RCV_BUF_DLEN];	/* frame data */
};


/* Receive buffer control block */
struct rbcb {
	struct rec_buf *rbufp;	/* pointer to current receive buffer */
	struct rec_buf *rbufp_next;	/* pointer to next receive buffer */
	unsigned char  *rbuf_datap;	/* pointer to data in receive buffer */
	unsigned short  data_len;	/* amount of data in this rec buffer */
};


/* Token-Ring physical header */
struct tr_head {
	unsigned char   ac;	/* access control field */
	unsigned char   fc;	/* frame control field */
	unsigned char   daddr[TR_ADDR_LEN];	/* destination address */
	unsigned char   saddr[TR_ADDR_LEN];	/* source address */
	unsigned short  rcf;	/* route control field */
	unsigned short  rseg[8];/* routing registers */
};
/* Token Ring LLC structure */
struct tr_llc {
	unsigned char   dsap;	/* destination SAP */
	unsigned char   ssap;	/* source SAP */
	unsigned char   llc;	/* LLC control field */
	unsigned char   protid[3];	/* protocol id */
	unsigned short  ethertype;	/* ether type field */
};
