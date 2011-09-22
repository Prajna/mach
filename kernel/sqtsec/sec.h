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
 * $Log:	sec.h,v $
 * Revision 2.4  93/03/10  11:30:56  danner
 * 	u_long -> u_int
 * 	[93/03/10            danner]
 * 
 * Revision 2.3  91/07/31  18:07:40  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:07:21  dbg
 * 	Added volatile declarations.
 * 	[90/11/13            dbg]
 * 
 * 	Adapted for pure Mach kernel.
 * 	[90/10/04            dbg]
 * 
 */

/*
 * $Header: sec.h,v 2.4 93/03/10 11:30:56 danner Exp $
 */

/*
 * Revision 1.1  89/07/05  13:20:15  kak
 * Initial revision
 * 
 * Revision 2.12  88/03/11  18:01:43  davest
 * added MAX_SCED_ADDR_MEM -highest addressable memory SCED can access
 * 
 */

#ifndef	_SQTSEC_SEC_H_
#define	_SQTSEC_SEC_H_

#include <sys/types.h>

/* 
 * sec.h
 *
 * This file contains the definitions of the interfaces
 * to the SCSI/Ether (SEC) firmware.
 *
 *
 * Below are the device numbers that the firmware expects for the 
 * unit channel id when a message such as STARTIO is sent. The SCSI
 * devices have a range from 0x20 (32) to 0x5f (95) with each target
 * adapter having a maximum of 8 logical units. 
 *
 *	adapter#	lun	sed_chan#
 *	0		0-7	0x20-0x27
 *	1		0-7	0x28-0x2f
 *	2		0-7	0x30-0x37
 *	3		0-7	0x38-0x3f
 *	4		0-7	0x40-0x47
 *	5		0-7	0x48-0x4f
 *	6		0-7	0x50-0x57
 *	host(7)		0-7	0x58-0x5f
 *
 * The host adapter number by default is adapter number 7 but should
 * there be more than one SEC on a single scsi bus then the adapter
 * number should be changed as the board won't work if there are more
 * than one adapter with the same bus number.
 */

#define SDEV_SCSIBOARD		0	/* The SEC itself..... */
#define SDEV_ETHERREAD		1	/* ether input channel */
#define SDEV_ETHERWRITE		2	/* ether output channel */
#define SDEV_CONSOLE0IN		3	/* console port channel input */
#define SDEV_CONSOLE0OUT	4	/* console port channel output */
#define SDEV_CONSOLE1IN		5	/* diag port channel input */
#define SDEV_CONSOLE1OUT	6	/* diag port channel output */
#define SDEV_TOD		7	/* time of day clock channel */
#define SDEV_MEM		8	/* memory channel */
#define SDEV_WATCHDOG		9	/* watch dog timer channel */

#ifdef	KERNEL_PROFILING
#define SDEV_PROFILER		10	/* SEC profiler channel */
#define SDEV_NUM_NONSCSI	11	/* last non-scsi device */
#else
#define SDEV_NUM_NONSCSI	10	/* last non-scsi device */
#endif	KERNEL_PROFILING

#define SDEV_SCSISTART		0x20	/* scsi unit number start (32) */
#define SDEV_SCSIEND		0x5f	/* scsi unit number end (95) */
#define SDEV_NUM_DEVICES	96

/*
 * Indirect address table.
 */

#ifdef	ns32000
/* 
 * Must handle case where CLBYTES > 4095 which is a h/w limit
 * on the size of an xfer count. buf_iatsz() also handles this.
 */

#if (CLBYTES>4095)
#define IATBYTES	2048
#define IATSIZE		4
#else
#define IATSIZE		CLSIZE
#define IATBYTES	CLBYTES
#endif
#endif	ns32000

#ifdef	i386
/* 
 * Ideally, IATBYTES == CLBYTES.  However, IATBYTES must be < 4096, since
 * 4095 is a hardware limit on the size of an xfer count.  buf_iatsz()
 * also handles this.  Thus take power of two below this (2048).
 */
#define IATBYTES	2048
#endif	i386

#define SEC_IAT_FLAG		0x80000000	/* indirect bit for iat ptr */
#define SEC_IATIFY(addr)	((struct sec_iat *)(((int)(addr)) | SEC_IAT_FLAG))
#define IATVARIANCE		1		/* raw - for tail garbage */

struct sec_iat {
	u_char	*iat_data;		/* pointer to data */
	int	iat_count;		/* number of bytes to put there */
};


/*
 * Maximum amount of physical memory the SCED can address.  All transfers
 * must be to physical addresses below this address.
 */

#define	MAX_SCED_ADDR_MEM	(128 * 1024 * 1024)		/* 128Meg */

/*
 * device program for all devices except ether read and clocks
 */

#define SCSI_CMD_SIZE	16		/* Max no. of bytes in a scsi command */

struct sec_dev_prog {
	u_char	dp_status1;		/* byte 1 of status */
	u_char	dp_status2;		/* byte 2 of status */
	short	dp_reserved;
	int	dp_count;		/* number of bytes transferred */
	union {
		u_char	*dp_data;	/* ptr to data */
		struct sec_iat *dp_iat;	/* ptr to indirect address table */
	} dp_un;
	struct sec_dev_prog *dp_next;	/* ptr to next dev program if linked */
	int dp_data_len;		/* total number of bytes to transfer */
	int dp_cmd_len;			/* real size of next field */
	u_char dp_cmd[SCSI_CMD_SIZE];	/* SCSI Device command */
};

/*
 * channel instruction block
 */
struct sec_cib {
	int	cib_inst;		/* instruction */
  volatile
	int	*cib_status;		/* ptr to status or other structs */
};

/* Error Flags */
#define SEC_ERR_NONE		0
#define SEC_ERR_INVALID_INS	1
#define SEC_ERR_INVALID_DEV	2
#define SEC_ERR_NO_MORE_IO	3
#define SEC_ERR_NO_SENSE	4
#define SEC_ERR_COUNT_TOO_BIG	128
#define SEC_ERR_BAD_MODE	129

/*
 * Instructions SCSI/Ether controller.
 */
#define SINST_INSDONE		0x80000000	/* instruction complete bit */
#define SINST_INIT		0	/* Initialize instruction */
#define SINST_SETMODE		1	/* set modes instruction */
#define SINST_STARTIO		2	/* start io instruction */
#define SINST_GETMODE		3	/* get modes instruction */
#define SINST_FLUSHQUEUE	4	/* flush queue instruction */
#define SINST_RESTARTCURRENTIO	5	/* restart current inst */
#define SINST_RESTARTIO		6	/* restart instruction */
#define SINST_REQUESTSENSE	7	/* request sense inst */
#define SINST_STOPIO		8	/* stop io instruction */
#define SINST_RETTODIAG		9	/* return to diagnostics inst */
#define SINST_DIAG_REQ		10	/* special diagnostics request */

#define SCSI_ETHER_WRITE	0xA	/* SCSI first byte for write */
#define SCSI_ETHER_STATION	0x0	/* SCSI 2nd byte for station */
#define SCSI_ETHER_MULTICAST	0x1	/* SCSI 2nd byte for multicast */

/*
 * device program queue
 */

#define SEC_POWERUP_QUEUE_SIZE	3	/* Queue size at power-up */

struct sec_progq {
	u_int pq_head;			/* head of list */
	u_int pq_tail;			/* tail of list */
	union {
		struct sec_dev_prog *pq_progs[SEC_POWERUP_QUEUE_SIZE];
		struct sec_edev_prog {
			int	edp_iat_count;		/* no. of iat's */
			struct	sec_iat *edp_iat;	/* ptr to iat's */
		} *pq_eprogs[SEC_POWERUP_QUEUE_SIZE];
	} pq_un;
};

/*
 * ether read output queue
 */
struct sec_eprogq {
	u_int	epq_head;	/* head of list */
	u_int	epq_tail;	/* tail of list */
	struct sec_ether_status {
		u_char	es_status1;	/* byte 1 of status */
		u_char	es_status2;	/* byte 2 of status */
		short	es_reserved;
		int 	es_count;	/* number of bytes received */
		u_char	*es_data;	/* pointer to the first byte received */
	} epq_status[SEC_POWERUP_QUEUE_SIZE]; /* ether status blocks */
};

/*
 * SCSI Queues at power-up.
 * The address of this is passed by power-up code to the kernel
 * for auto-config of the SCSI/Ether controller.
 */
struct sec_powerup {
	struct sec_cib	pu_cib;			/* all devices */
	struct sec_progq	pu_requestq;	/* all devices but ether read */
	struct sec_progq	pu_doneq;	/* all devices but ether read */
	struct sec_progq	pu_erequestq;	/* for ether read */
	struct sec_eprogq	pu_edoneq;	/* for ether read */
};

/*
 * init channel instruction data structure (ptr to it is passed in cib status
 * pointer)
 */
struct sec_init_chan_data {
  volatile
	int	sic_status;		/* status of INIT instruction */
	struct	sec_cib	*sic_cib;	/* pointer to cibs (1/device) */
	struct sec_chan_descr {
		struct sec_progq *scd_requestq;	/* pointer to input queue */
		struct sec_progq *scd_doneq;	/* pointer to output queue */
		u_char	scd_bin;		/* bin to interrupt Unix on */
		u_char	scd_vector;		/* interrupt vector to return */
		u_char	scd_destslic;		/* Interrupt destination */
	} sic_chans[SDEV_NUM_DEVICES];	/* channel descriptors (1/device) */
};

/*
 * Diagnostic request.
 * A pointer to this is passed in the cib_status
 * when the cib_inst is SINST_DIAG_REQ.
 *
 * For SDR_CACHE_OFF and SDR_CACHE_ON, sdr_slic is the SLIC id
 * of the thing that needs its cache turned on or off.  This is
 * done with the processor held (via its PROC_CTL register),
 * if it is a processor.
 *
 * SDR_MEM_CHANGED has no arguments.
 *
 * SDR_WRSUBSLAVE writes the SLIC sub-slave register at SLIC
 * address sdr_slic, slave register sdr_s_slave and subslave
 * address sdr_s_subslave with the data in sdr_s_data.
 * This is intended primarily for use with SGS processor
 * boards, so it forces a bus PAUSE, then holds the
 * processor (and its neighbor) before doing the wrSubslave().
 * The VLSI wanted to assume these settings were static...
 */
struct sec_diag_req {
  volatile
	int	sdr_status;		/* status from this cmd */
	u_char	sdr_cmd;		/* command for SCED to execute */
	u_char	sdr_slic;		/* SLIC target operand for some cmds */
	u_char	sdr_compad[2];		/* common data pad */
	union sdr_un {			/* command-specific fields: */
		struct sdru_slave {
			u_char	sdrus_slave;	/* SLIC slave address */
			u_char	sdrus_subslave;	/* SLIC sub-slave addr (opt) */
			u_char	sdrus_data;	/* SLIC data for write */
		}	sdru_slave;	/* slave command for SCED to execute */
		u_int	sdru_pad[10];	/* lots of padding for cmd-specifics */
	} sdr_un;
};

/*
 * Values for sdr_cmd.
 */
#define SDR_CACHE_OFF	0x00		/* turn cache off for someone */
#define SDR_CACHE_ON	0x01		/* turn cache on for someone */
#define SDR_MEM_CHANGED	0x02		/* notice that memory config changed */
#define SDR_WRSUBSLAVE	0x03		/* write a SLIC sub-slave register */

/*
 * Make union entries easy to get to.
 */
#define sdr_s_slave	sdr_un.sdru_slave.sdrus_slave
#define sdr_s_subslave	sdr_un.sdru_slave.sdrus_subslave
#define sdr_s_data	sdr_un.sdru_slave.sdrus_data
#define sdr_pad	sdr_un.sdru_pad


#ifdef	KERNEL_PROFILING

struct pc_mode {
	unsigned int pm_pc;
	unsigned int pm_mode;	/* usermode != 0, kernel mode = 0 */
};

/*
 * profiling get and set mode structure
 * used for communication with the profiler pseudo device on a SCED
 */
struct kp_modes {
	int kpm_interval;	/* SCED timer interrupt interval in ms */
	int kpm_reload;		/* SCED sends NMI every reload timer intrs */
	unsigned long kpm_sced_nmis;/* Number of nmi's sent by fw */
	int kpm_state;		/* state of profiling */
	int kpm_binshift;	/* Log2(Size in bytes of a bin) */
	int kpm_bins;		/* Number of bins used in profiling */
	int kpm_engines;	/* Number of engines to profile */
	unsigned kpm_b_text;	/* addr of 1st text symbol */
	struct pc_mode *kpm_pc;	/* addr of pc/psr pairs for processors */
				/* addr in main mem: passed in from driver */
	unsigned long *kpm_cntrs;/* address of the profiling cntrs */
				/* addr in main mem: passed in from driver */
};
#endif	KERNEL_PROFILING


/*
 * structure for set modes command.  Ptr to this goes in channel status ptr
 */
struct sec_smode {
  volatile
	int	sm_status;			/* status from this cmd */
	union {
		struct sec_cons_modes {
			short	cm_baud;	/* baud rate */
			short	cm_flags;	/* flags for stop bits, dtr etc */
		} sm_cons;
		struct sec_ether_smodes {
			u_char	esm_addr[6];	/* ether address */
			short	esm_size;	/* constant iat chunk size */
			short	esm_flags;	/* receive mode flag */
		} sm_ether;
		struct sec_scsi_smodes {
			short	ssm_timeout;	/* bus timeout */
			short	ssm_flags;	/* used Single ended or diff connect */
		} sm_scsi;
		struct sec_tod_modes {
			int	tod_freq;	/* interrupt frequency */
			int	tod_newtime;	/* new time for TOD clock */
		} sm_tod;
		int	sm_wdt_mode;		/* wd timer mode */
 		struct sec_board_modes {
 			struct sec_powerup *sec_powerup;
 			short	sec_dopoll;
			short	sec_errlight;		/* error light */
 			struct reboot	*sec_reboot;	/* see cfg.h */
 		} sm_board;
		struct sec_mem {
			char *mm_buffer;	/* address of log buffer */
			char *mm_nextchar;	/* next free char in buffer */
			short mm_size;		/* buffer size */
			short mm_nchar;		/* number valid chars in buf */
		} sm_mem;
#ifdef	KERNEL_PROFILING
		struct kp_modes sm_kp;
#endif	KERNEL_PROFILING
	} sm_un;
};

/* Console Flags */
#define SCONS_STOP1		0x0000
#define SCONS_STOP1P5		0x0001
#define SCONS_STOP2		0x0002
#define SCONS_ALL_STOP		0x0003

#define SCONS_DATA8		0x0000
#define SCONS_DATA7		0x0004
#define SCONS_ALL_DATA		0x0004

#define SCONS_NO_PARITY		0x0000
#define SCONS_EVEN_PARITY	0x0008
#define SCONS_ODD_PARITY	0x0010
#define SCONS_ALL_PARITY	0x0018

#define SCONS_SEND_CARRIER	0x0000
#define SCONS_IGN_CARRIER	0x0020

#define SCONS_SET_DTR		0x0000
#define SCONS_CLEAR_DTR		0x0040

#define SCONS_SEND_BREAK	0x0000
#define SCONS_DIAG_BREAK	0x0080
#define SCONS_IGNORE_BREAK	0x0100
#define SCONS_ALL_BREAK		0x0180

#define SCONS_DHMODE		0x0000
#define SCONS_DZMODE		0x0200

#define SCONS_CARRIER_SET	0x0000
#define SCONS_CARRIER_CLEAR	0x0400

#define SCONS_SET_RTS		0x0000
#define SCONS_CLEAR_RTS		0x0800

#define SCONS_CLEAR_BREAK	0x0000
#define SCONS_SET_BREAK		0x1000

#define SCONS_CRT_ERASE		0x8000

#define SCONS_SETABLE	(SCONS_ALL_STOP | SCONS_ALL_DATA \
	| SCONS_ALL_PARITY | SCONS_IGN_CARRIER | SCONS_CLEAR_DTR \
	| SCONS_CLEAR_RTS | SCONS_SET_BREAK)

#define SCONS_PRESERVE	(SCONS_ALL_BREAK | SCONS_CRT_ERASE)

#define SCONS_TRANSIENT	(0xFFFF & ~(SCONS_PRESERVE | SCONS_SETABLE))

/* Console info error bits */
#define SCONS_TIMEOUT		0x1
#define SCONS_BREAK_DET		0x2
#define SCONS_CARR_DET		0x4
#define SCONS_OVRFLOW		0x8
#define SCONS_FLUSHED		0x10
#define SCONS_PARITY_ERR	0x20
#define SCONS_FRAME_ERR		0x40

/* Ether mode flags */
#define SETHER_DISABLE		0
#define SETHER_PROMISCUOUS	1
#define SETHER_S_AND_B		2
#define SETHER_MULTICAST	3
#define SETHER_LOOPBACK		4

/* Front panel error light */
#define SERR_LIGHT_ON		 1
#define SERR_LIGHT_SAME		 0
#define SERR_LIGHT_OFF		-1

/* value of the cib_status pointer upon SINST_RETTODIAG command */
#define SRD_BREAK	(int *)0	/* returning from BREAK, halts */
#define SRD_POWERUP	(int *)1	/* use powerup defaults */
#define SRD_REBOOT	(int *)2	/* use setmode data */

/*
 * structure for get modes command.  Ptr to this goes in channel status ptr
 */
struct sec_gmode {
  volatile
	int	gm_status;		/* status from this instruction */
  volatile
	union {
		struct sec_cons_modes gm_cons;		/* console data */
		struct sec_ether_gmodes {
			struct sec_ether_smodes egm_sm;	/* same as set modes */
			int egm_rx_ovfl;	/* number of dma overflows */
			int egm_rx_crc;		/* number of crc errors */
			int egm_rx_dribbles;	/* number of dribbles */
			int egm_rx_short;	/* number of short packets */
			int egm_rx_good;	/* number of good packets */
			int egm_tx_unfl;	/* number of dma underflows */
			int egm_tx_coll;	/* number of collisions */
			int egm_tx_16x_coll;	/* number of 16x collisions */
			int egm_tx_good;	/* number of good packets sent */
		} gm_ether;
		struct sec_wdt_gmodes {
			int gwdt_time;		/* time between expirations */
			int gwdt_expired;	/* number of times light has expired */
		} gm_wdt;
		struct sec_tod_modes	gm_tod;	/* time of day data */
		struct sec_scsi_gmodes {
			struct	sec_scsi_smodes sgm_sm;	/* same as scsi set mode */
			int	sgm_bus_parity;	/* number of bus parity errors seen */
		} gm_scsi;
 		struct sec_board_modes	gm_board; /* same as board set mode */
		struct sec_mem gm_mem;
#ifdef	KERNEL_PROFILING
		struct kp_modes gm_kp;
#endif	KERNEL_PROFILING
	} gm_un;
};

/*
 * request sense structure.  ptr goes in channel status ptr.
 */
struct sec_req_sense {
  volatile
	int	rs_status;
	struct sec_dev_prog rs_dev_prog;	/* device program to run for sense command */
};

/* SCSI Sense Info */
#define SSENSE_NOSENSE		0
#define SSENSE_RECOVERABLE	1
#define SSENSE_NOT_READY	2
#define SSENSE_MEDIA_ERR	3
#define SSENSE_HARD_ERR		4
#define SSENSE_ILL_REQ		5
#define SSENSE_UNIT_ATN		6
#define SSENSE_DATA_PROT	7
#define SSENSE_BLANK_CHK	8
#define SSENSE_ABORT		0xb
#define SSENSE_EQUAL		0xc
#define SSENSE_VOL_OVER		0xd
#define SSENSE_CHECK		2
#define SSENSE_INTSTAT		16
#define SSENSE_MOREINFO		0x80
#define SSENSE_HOST_ERR		1
#define SSENSE_NOTREADY		1

/*
 * SCSI bus device probe routine responses
 */

#define SECP_NOTFOUND		0x1		/* no good unit here */
#define SECP_NOTARGET		0x2		/* no good target here */
#define SECP_FOUND		0x4		/* multi-unit target found */
#define SECP_ONELUN		0x8		/* single-unit target found */

#endif	/* _SQTSEC_SEC_H_ */
