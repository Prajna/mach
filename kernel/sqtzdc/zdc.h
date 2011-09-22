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
 * $Log:	zdc.h,v $
 * Revision 2.4  93/03/11  14:05:53  danner
 * 	u_long -> u_int
 * 	[93/03/10            danner]
 * 
 * Revision 2.3  91/07/31  18:08:29  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:08:20  dbg
 * 	Adapted for pure Mach kernel.
 * 	[90/10/04            dbg]
 * 
 */

/*
 * $Header: zdc.h,v 2.4 93/03/11 14:05:53 danner Exp $
 *
 * zdc.h
 *	ZDC disk controller definitions and data structures.
 */

/*
 * Revision 1.1  89/07/05  13:21:06  kak
 * Initial revision
 * 
 * Revision 1.10  88/03/23  18:27:44  neal
 * Added m2382k (swallow 5) support to ZDC.
 * 
 */
#ifndef	_SQTZDC_ZDC_H_
#define	_SQTZDC_ZDC_H_

#include <sys/types.h>
#include <mach/time_value.h>
#include <device/buf.h>
#include <sqt/mutex.h>

#define ZDC_MAXDRIVES	16			/* drives on a controller */
#define ZDC_MAXDRSHFT	 4			/* drives on ctrlr log2 */
#define	NCBPERDRIVE	 2			/* no. of CBs per drive */
#define	NCBDRSHFT	 1			/* drive on ctrlr log2 */
#define	NCBPERZDC	(NCBPERDRIVE*ZDC_MAXDRIVES)	/* no. of CBs per ZDC */
#define	NCBZDCSHFT	(NCBDRSHFT+ZDC_MAXDRSHFT)	/* shft for NCBPERZDC */
#define	NSTATBYTES	7			/* no of status bytes */
#define	CBBIN		7			/* SLIC Bin for CB commands */
#define CLRERRBIN	5			/* SLIC Bin to clear ctlr errors */

#ifndef	ZDC_MICROCODE
/*
 * Software / firmware interface command block.
 */
struct cb {
	u_int	cb_reserved0;		/* reserved for SEQUENT use */
	struct diskaddr {
		u_char	da_sect;	/* sector */
		u_char	da_head;	/* head */
		u_short	da_cyl;		/* cylinder */
	} cb_diskaddr;
	u_int	cb_addr;		/* If cb_iovec nonzero, then
					 * virtual address. Else,
					 * physical address.  */
	u_int	cb_count;		/* transfer count */
	u_int	*cb_iovec;		/* if nonzero, physical address
					 * of list of iovectors. */
	u_char	cb_status[NSTATBYTES];	/* drive status and extended status */
	u_char	cb_reqstat;		/* Internal to ZDC only */
	u_char	cb_psect;		/* phys sector - offset from index */
	u_char	cb_mod;			/* ZDC command modifiers */
	u_char	cb_cmd;			/* ZDC command */
	u_char	cb_compcode;		/* completion code */

	/* start of Software only part */
	struct	buf *cb_bp;		/* ptr to buf header 
					 * null == CB not busy */
	u_int	*cb_iovstart;		/* start of cb_iovec buffer */
	u_short	cb_errcnt;		/* retries */
	short	cb_unit;		/* index into unit table; -1 invalid */
	struct	diskaddr cb_contaddr;	/* where to continue after revectoring */
	int	cb_contiovsz;		/* # of bytes iovecs on continuation */
	int	cb_transfrd;		/* # of bytes transferred previously */
	u_char	cb_state;		/* current job state */
	u_int	cb_fill[1];		/* fill to 64 byte size */
};
#define	cb_cyl	cb_diskaddr.da_cyl
#define	cb_head	cb_diskaddr.da_head
#define	cb_sect	cb_diskaddr.da_sect
#endif	ZDC_MICROCODE

#define	ADDRALIGN	16		/* cb_addr must be 16-byte aligned */
#define	CNTMULT		16		/* cb_count must be 16-byte multiple */
#define	IOVALIGN	32		/* cb_iovec's must be 32-byte aligned */
#define	FWCBSIZE	32		/* size excluding SW only half */

/*
 * ZDC command codes	(ZDC ucode depends on the order of these)
 */
#define ZDC_READ		0x01	/* normal read */
#define ZDC_WRITE		0x02	/* normal write */
#define ZDC_INIT		0x03	/* initialize ZDC */
#define ZDC_PROBE		0x04	/* get drive configuration */
#define	ZDC_NOP			0x05	/* Do nothing */
#define	ZDC_READ_LRAM		0x06	/* Read 4 bytes of ZDC Local RAM */
#define	ZDC_WRITE_LRAM		0x07	/* Write 4 bytes of ZDC Local RAM */
#define	ZDC_SEEK		0x08	/* seek to given cylinder/head */
#define ZDC_STAT		0x09	/* read drive status */
#define ZDC_RESET		0x0a	/* reset drive */
#define ZDC_PROBEDRIVE		0x0b	/* get drive config - specific drive */
#define ZDC_GET_CHANCFG		0x0c	/* get the channel configuration */
#define ZDC_SET_CHANCFG		0x0d	/* set the channel configuration */
#define	ZDC_FMT_SS		0x0e	/* format special sector */
#define	ZDC_READ_SS		0x0f	/* read special sector */
#define	ZDC_WRITE_SS		0x10	/* write special sector */
#define ZDC_FMTTRK		0x11	/* format a track */
#define ZDC_READ_HDRS		0x12	/* read all headers on a track */
#define	ZDC_WHDR_WDATA		0x13	/* Write a sector's header and data */
#define ZDC_LONG_READ		0x14	/* read sector and ECC appendage */
#define ZDC_LONG_WRITE		0x15	/* write sector and ECC appendage */
#define ZDC_REC_DATA		0x16	/* Recover data from sector w/bad hdr */

#define	ZDC_MAXCMD		0x16	/* Last legal command */

/*
 * Command Modifier Codes
 */
#define	ZDC_SERVOPLUS		0x01	/* Servo offset plus */
#define	ZDC_SERVOMINUS		0x02	/* Servo offset minus */
#define	ZDC_STROBEARLY		0x04	/* Data Strobe early */
#define	ZDC_STROBELATE		0x08	/* Data Strobe late */
#define	ZDC_NOECC		0x10	/* Inhibit ECC correction */
#define	ZDC_NOREVECTOR		0x20	/* Inhibit Auto-revectoring */

/*
 * ZDC completion codes.
 */
#define	ZDC_BUSY		0x00	/* ZDC processing CB */
#define	ZDC_DONE		0x01	/* successful completion */
#define	ZDC_DRVPROT		0x02	/* write protect fault */
#define	ZDC_DRVFLT		0x03	/* Drive Fault */
#define	ZDC_SEEKERR		0x04	/* Seek error */
#define	ZDC_SEEK_TO		0x05	/* seek timeout */
#define	ZDC_CH_TO		0x06	/* Channel timeout */
#define	ZDC_DMA_TO		0x07	/* DMA timeout */
#define	ZDC_HDR_ECC		0x08	/* header ECC error */
#define	ZDC_SOFTECC		0x09	/* soft ECC error (retry successful) */
#define	ZDC_CORRECC		0x0a	/* correctable ECC error */
#define	ZDC_ECC			0x0b	/* uncorrectable ECC error */
#define	ZDC_SNF			0x0c	/* sector not found */
#define	ZDC_REVECT		0x0d	/* bad data sector */
#define	ZDC_SO			0x0e	/* sector overrun */
#define	ZDC_NDS			0x0f	/* no data synch */
#define	ZDC_FDL			0x10	/* fifo data lost */
#define	ZDC_ILLCMD		0x11	/* illegal cb_cmd */
#define	ZDC_ILLMOD		0x12	/* illegal cb_mod */
#define	ZDC_ILLCHS		0x13	/* illegal disk address */
#define	ZDC_ILLALIGN		0x14	/* cb_addr not 16-byte aligned */
#define	ZDC_ILLCNT		0x15	/* cb_count not multiple of 16 */
#define	ZDC_ILLIOV		0x16	/* cb_iovec not 32-byte aligned */
#define	ZDC_ILLVECIO		0x17	/* cb_iovec != 0 && page size invalid */
#define	ZDC_ILLPGSZ		0x18	/* illegal icb_pagesize */
#define	ZDC_ILLDUMPADR		0x19	/* icb_dumpaddr not 1K-byte aligned */
#define	ZDC_BADDRV		0x1a	/* drive not online... */
#define	ZDC_CBREUSE		0x1b	/* in use CB was reused */
#define	ZDC_ACCERR		0x1c	/* access error during DMA */
#define	ZDC_NOCFG		0x1d	/* channel not configured */
#define	ZDC_CH_RESET		0x1e	/* channel was reset */
#define	ZDC_DDC_STAT		0x1f	/* unexpected status from DDC */

/*
 * State bit codes
 */
#define	ZD_NORMAL	0x00	/* Normal state */
#define	ZD_REVECTOR	0x01	/* Revectoring to replacement sector */
#define	ZD_RESET	0x02	/* Reset for retry sequence in progress */

#ifndef	ZDC_MICROCODE
/*
 * Command block for ZDC_INIT command.
 */
struct init_cb {
	u_int	icb_reserved0[3];	/* reserved for SEQUENT use */
	u_int	icb_pagesize;		/* page size in bytes */
	caddr_t	icb_dumpaddr;		/* Physical address of 8 KB region in
					 * memory where LRAM may be dumped.
					 */

	/* completion interrupts initialization */

	u_char	icb_dest;		/* slic destination */
	u_char	icb_bin;		/* slic bin */
	u_char	icb_vecbase;		/* slic message (vector) */
	u_char	icb_reserved1;		/* more reserved */

	u_char	icb_errdest;		/* slic destination, ctlr errs */
	u_char	icb_errbin;		/* slic bin, ctlr errs */
	u_char	icb_errvector;		/* slic message, ctlr errs */
	u_char	icb_reserved2;		/* more reserved */

	u_char	icb_reserved3;		/* more reserved */
	u_char	icb_ctrl;		/* ZDC Control */
	u_char	icb_cmd;		/* ZDC command */
	u_char	icb_compcode;		/* completion code */

	/* start of Software only part */
	u_int	icb_sw[8];		/* Reserved for Sequent SW use */
};
#endif	ZDC_MICROCODE

/*
 * Control byte encoding.
 */
#define	ZDC_ENABLE_INTR		0x01	/* Enable interrupts */
#define	ZDC_DUMPONPANIC		0x02	/* Dump LRAM on controller panic */
#define	ZDC_SIXTEEN		0x04	/* Do 16 byte DMA; else 8 byte */

#define	ZDC_LRAMSZ	8192	/* 8K local RAM to dump at icb_dumpaddr */

#ifndef	ZDC_MICROCODE
/*
 * Command Block for ZDC_PROBE command.
 */
struct probe_cb {
	u_int	pcb_reserved0[3];		/* reserved for SEQUENT use */
	u_char	pcb_drivecfg[ZDC_MAXDRIVES];	/* drive configuration */
	u_short	pcb_reserved1;			/* more reserved */
	u_char	pcb_cmd;			/* ZDC command */
	u_char	pcb_compcode;			/* completion code */

	/* start of Software only part */
	u_int	pcb_sw[8];			/* reserved Sequent SW use */
};
#endif	ZDC_MICROCODE

/*
 * Drive configuration bit encoding.
 */
#define	ZD_PRESENT	0x01		/* Drive present */
#define	ZD_ONLINE	0x02		/* Drive is online (spun up) */
#define	ZD_FORMATTED	0x04		/* Drive is formatted */
#define ZD_MATCH	0x08		/* Format matches rest of channel */
#define	ZD_READONLY	0x10		/* Drive is write-protected */

#define	ZD_NOTFOUND	0x00		/* Drive not found */

/*
 * SLIC slave status register (SL_Z_STATUS) error bits
 */
#define	ZDC_ERRMASK	0x2F		/* mask to determine error type */
#define	ZDC_SRESET	0x00		/* ZDC in reset state */
#define	ZDC_SINIT	0x21		/* ZDC initializing */
#define	ZDC_READY	0x28		/* If no parity error present */
/*
 * This consists of errors where the controller is still running.
 * Note that codes: 0x25 - 0x27 are reserved for future use.
 */
#define	ZDC_OBCB	0x22		/* Error - Out of Bounds CB */
#define	ZDC_NOCB	0x23		/* Error - Request without CB base */
#define	ZDC_CBACCESS	0x24		/* Error - CB Access Error */
/*
 * The following error codes are for fatal errors where the HSC stops.
 * These currently include Parity errors (SLB_ZPARERR),
 * FW internal errors (Panic) and DMA C timeout.
 * Note that codes: 0x0a - 0x0f are reserved.
 */
#define	ZDC_WCSPE	0x01		/* WCS Parity Error */
#define	ZDC_ZDCTOOK	0x02		/* ZDC->OK Parity Error */
#define	ZDC_OKTOZDC	0x03		/* OK->ZDC Parity Error */
#define	ZDC_LRAMTOHSC	0x04		/* LRAM->HSC Parity Error */
#define	ZDC_LRAMTOBUS	0x05		/* LRAM->SB8000 Parity Error */
#define	ZDC_LRAMTOCHA	0x07		/* LRAM->Channel A Parity Error */
#define	ZDC_LRAMTOCHB	0x06		/* LRAM->Channel B Parity Error */

#define	ZDC_FW_PANIC	0x08		/* Error - FW panic (internal error) */
#define	ZDC_DMAC_TO	0x09		/* Error - DMA C Timeout */

#ifndef	ZDC_MICROCODE
/*
 * Track 0 Disk Description structure definition
 * Note: these fields are order dependent since shared data with microcode.
 */
struct zdcdd {
	u_int	zdd_magic;		/* magic number for sanity */
	u_char	zdd_ecc_bytes;		/* # of bytes of ECC */
	u_char	zdd_spare;		/* spares per track */
	u_char	zdd_sectors;		/* sectors per track (formatted) */
	u_char	zdd_tracks;		/* tracks per cylinder */
	u_short	zdd_cyls;		/* number of cylinders */

	u_char	zdd_drive_type;		/* Soft drive type.  That is, the type
					 * of the drive as it was formatted.
					 * Used in driver for selection of
					 * partition tables.
					 */
	u_char	zdd_xfer_rate;		/* drive transfer rate in MHz */
	u_short	zdd_runt;		/* no. of bytes in runt sector */
	u_short	zdd_chdelay;		/* channel delay */
	u_char	zdd_hsdelay;		/* head switch delay */
	u_char	zdd_hpo_rd_bc;		/* header postamble byte cnt (read) */
	u_char	zdd_hpo_fmt_bc;		/* header postamble byte cnt (fmt) */
	u_char	zdd_cskew;		/* format skew between cylinders */
	u_char	zdd_tskew;		/* format skew between tracks */
	u_char	zdd_hdr_bc;		/* header byte cnt (inc head scatter) */
	u_short	zdd_sector_bc;		/* sector byte cnt (inc header & gap) */
	u_char	zdd_strt_ign_bc;	/* bytes at the start of the sector
					 * where defects can be ignored.
					 */
	u_char	zdd_end_ign_bc;		/* bytes at the end of the sector
					 * where defects can be ignored.
					 */
	u_char	reserved1[2];
	u_int	reserved2[3];		/* reserved */

	/*
	 * The following structure contains an entry for each register
	 * in the NSC DP8466 (DDC) in order of hex address.
	 * Read only registers are included for convenience of the
	 * ZDC microcode.  They are marked "RO".
	 */
	struct ddc_regs {
		u_char	dr_status;	/* RO  status register */
		u_char	dr_error;	/* RO  error register */

		u_char	dr_ppb0;	/* polynomial preset byte 0 */
		u_char	dr_ppb1;	/* polynomial preset byte 1 */
		u_char	dr_ppb2;	/* polynomial preset byte 2 */
		u_char	dr_ppb3;	/* polynomial preset byte 3 */
		u_char	dr_ppb4;	/* polynomial preset byte 4 */
		u_char	dr_ppb5;	/* polynomial preset byte 5 */
		u_char	dr_ptb0;	/* polynomial tap byte 0 */
		u_char	dr_ptb1;	/* polynomial tap byte 1 */
		u_char	dr_ptb2;	/* polynomial tap byte 2 */
		u_char	dr_ptb3;	/* polynomial tap byte 3 */
		u_char	dr_ptb4;	/* polynomial tap byte 4 */
		u_char	dr_ptb5;	/* polynomial tap byte 5 */
		u_char	dr_ec_ctrl;	/* ECC/CRC control */

		u_char	dr_hbc;		/* header byte count */

		u_char	dr_dc;		/* drive command register */
		u_char	dr_oc;		/* operation command register */
		u_char	dr_sc;		/* sector count */
		u_char	dr_nso;		/* number of sector operations */

		u_char	dr_hb0_pat;	/* header byte 0 pattern */
		u_char	dr_hb1_pat;	/* header byte 1 pattern */
		u_char	dr_hb2_pat;	/* header byte 2 pattern */
		u_char	dr_hb3_pat;	/* header byte 3 pattern */
		u_char	dr_hb4_pat;	/* header byte 4 pattern */
		u_char	dr_hb5_pat;	/* header byte 5 pattern */

		u_short	dr_rdbc;	/* remote data byte count */
		u_int	dr_dma_addr;	/* DMA address */

		u_char	dr_dpo_bc;	/* data postamble byte count */
		u_char	dr_hpr_bc;	/* header preamble byte count */
		u_char	dr_hs1_bc;	/* header synch #1 byte count */
		u_char	dr_hs2_bc;	/* header synch #2 byte count */

		u_char	dr_hb0_ctrl;	/* header byte 0 control */
		u_char	dr_hb1_ctrl;	/* header byte 1 control */
		u_char	dr_hb2_ctrl;	/* header byte 2 control */
		u_char	dr_hb3_ctrl;	/* header byte 3 control */
		u_char	dr_hb4_ctrl;	/* header byte 4 control */
		u_char	dr_hb5_ctrl;	/* header byte 5 control */

		u_char	dr_extdecc_bc;	/* external data ECC byte count */
		u_char	dr_exthecc_bc;	/* external header ECC byte count */

		u_char	dr_hpo_wr_bc;	/* header postamble byte count (write)*/
		u_char	dr_dpr_wr_bc;	/* data preamble byte count (write) */
		u_char	dr_ds1_bc;	/* data synch #1 byte count */
		u_char	dr_ds2_bc;	/* data synch #2 byte count */
		u_char	dr_dpo_pat;	/* data postamble pattern */
		u_char	dr_hpr_pat;	/* header preamble pattern */
		u_char	dr_hs1_pat;	/* header synch #1 pattern */
		u_char	dr_hs2_pat;	/* header synch #2 pattern */
		u_char	dr_gap_bc;	/* gap byte count */

		u_char	dr_df;		/* disk format register */
		u_char	dr_ltr;		/* local transfer register */
		u_char	dr_rtr;		/* remote transfer register */

		u_short	dr_sector_bc;	/* sector byte count */

		u_char	dr_gap_pat;	/* gap pattern */
		u_char	dr_dfmt_pat;	/* data format pattern */
		u_char	dr_hpo_pat;	/* header postamble pattern */
		u_char	dr_dpr_pat;	/* data preamble pattern */
		u_char	dr_ds1_pat;	/* data synch #1 pattern */
		u_char	dr_ds2_pat;	/* data synch #2 pattern */
	} zdd_ddc_regs;

	struct zdd_format_desc {	/* Format description */
		time_t	fd_fdate;	/* date/time of format */
		u_char	fd_rev;		/* revision of formatter */
		u_char	fd_passes;	/* number of verify passes */
	} zdd_format_desc;

	u_char	zdd_filler[15];
	u_char	zdd_checksum;
};
#endif	ZDC_MICROCODE

#define	ZDD_DDCYL	0			/* disk desc cylinder */
#define	ZDD_SS_SIZE	(DEV_BSIZE - 64)	/* SS sector size */

/*
 * Format specific defines.
 */
#define	ZDD_NDDSECTORS	4	/* no. of sectors containing zdcdd structure */
#define	ZDD_NDGNCYL	2	/* no. of cylinders reserved for diagnostics */
#define	ZDD_NDGNSPT	32	/* Min good sectors needed per DGN track */
#define	ZDD_NDGNSPTRKS	4	/* no. of dgn special tracks */

/*
 * Header types
 */
#define	ZD_GOODSECT	0xC9	/* A Good Used sector */
#define	ZD_GOODRPL	0x36	/* A Good Replacement sector */
#define	ZD_GOODSS	0x0A	/* A Good Special Sector */
#define	ZD_BADREVECT	0x50	/* A Bad (auto)Revector sector */
#define	ZD_BADUNUSED	0xA5	/* A Bad Unused sector */
#define	ZD_GOODSPARE	0x6C	/* A Good Spare sector (formatter only) */

#define	ZD_TORESOLVE	0x2	/* Flag byte - sector bad. Replacement tbd */
#define	ZD_ERRTYPE	0x1	/* Flag byte - sector bad. Error type. */

#define	ZD_BUCYL	0xC9C9	/* cyl # in ZD_BADUNUSED header */
#define	ZD_BUHEAD	0xC9	/* head # in ZD_BADUNUSED header */

#define	ZD_AUTOBIT	0x80	/* top bit set - autorevector replacement */
#define	ZD_INVALSECT	0xFF	/* Invalid sector number */
#define	ZD_MAXSECT	(ZD_INVALSECT - 1)	/* Max. valid sector number */

/*
 * Drive types
 *
 * Used as index into partition table list
 * and as well as disk description table list.
 *
 * Drive types 0-15 are reserved for Sequent use
 * Customer added drive types start at 16
 */

#define	ZDT_M2333K	0	/* Fujitsu M2333K (swallow) */
#define	ZDT_M2351A	1	/* Fujitsu M2351A (eagle) */
#define	ZDT_M2344K	2	/* Fujitsu M2344K (swallow 4) */
#define	ZDT_M2382K	3	/* Fujitsu M2382K (swallow 5) */

#define	ZDT_CDC9715_340	16	/* CDC 9715-340 (FSD) */
#define	ZDT_CDC9771_800	17	/* CDC 9771-800 (XMD) */

#ifndef	ZDC_MICROCODE
/*
 * Software only. Structure definitions and miscellaneaous definitions.
 */

#define	NUMPARTS	8		/* Number of partitions on drive */
#define ZDC_MAXCTRLR	8		/* Maximum number of controllers */

/*
 * ZDC controller structure
 *
 * 1 per ZDC.
 * Contains info collected via ZDC_PROBE and ZDC_GET_CHANCFG.
 */
struct	zdc_ctlr {
	simple_lock_data_t
		zdc_ctlrlock;			/* mutex when necessary */ 
	u_char	zdc_slicaddr;			/* 6-bit slic address (slot). */
	u_char	zdc_diagflag;			/* copy from cfg_zdc */
	u_char	zdc_state;			/* see below */
	u_char	zdc_dma_throttle;		/* DMA channel throttled? */
	struct	cb *zdc_cbp;			/* cb array for controller */
	caddr_t	zdc_dumpaddr;			/* ZDC fw dump region */
	u_char	zdc_drivecfg[ZDC_MAXDRIVES];	/* drive cfg info */
	struct	zdcdd zdc_chanA;		/* disk desc. channel A */
	struct	zdcdd zdc_chanB;		/* disk desc. channel B */
};

/* State field encoding */
#define	ZDC_NOTFOUND	0		/* not found */
#define	ZDC_DEAD	1		/* dead */
#define	ZDC_ALIVE	2		/* alive and operational */

/*
 * ZDC unit structure
 *
 * 1 per configured drive.
 */

#ifdef	KERNEL
#define	ZD_MAXUNITS	32		/* Maximum number of units */

struct zd_unit {
	simple_lock_data_t
		zu_lock;		/* mutex access to this thing */
	char	zu_ctrlr;		/* controller */
	char	zu_drive;		/* drive */
	char	zu_drive_type;		/* Configured drive type */
	u_char	zu_state;		/* If set, unit valid */
	u_char 	zu_cfg;			/* drive cfg info for this unit */
	short	zu_nopen;		/* no. of concurrent opens */
	struct	cb	*zu_cbptr;	/* ptr to first CB for drive */
	struct	zdbad	*zu_zdbad;	/* bad block list */
	struct	dk	*zu_dkstats;	/* statistics */
	time_value_t	zu_starttime;	/* start time for busy drive */
	sema_t	zu_ocsema;		/* Semaphore to serialize opens */
	struct	buf	zu_bhead;	/* buffer queue */
	struct	buf	zu_ioctl;	/* buffer for ioctls */
	struct 	cb	zu_ioctlcb;	/* cb for ioctls */
};

/*
 * zu_state values
 */
#define ZU_NOTFOUND	0		/* unit not found */
#define	ZU_BAD		1		/* unit unusable */
#define	ZU_NO_RW	2		/* unit usable; no read/write allowed */
#define	ZU_GOOD		3		/* unit usable */
#endif	KERNEL

/* spl level when using zu_lock */
#define	SPLZD	SPL5

/*
 * Partition data
 * The partition tables for a given drive type are defined
 * in the binary configuration file.
 */
struct	zdsize	{
	long	p_length;		/* length in blocks */
	long	p_cyloff;		/* starting cylinder */
};
#endif	ZDC_MICROCODE

#endif	/* _SQTZDC_ZDC_H_ */
