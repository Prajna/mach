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
 * $Log:	scsi.h,v $
 * Revision 2.3  91/07/31  18:07:04  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:06:26  dbg
 * 	Added, from Sequent SYMMETRY sources.
 * 	[91/02/26            dbg]
 * 
 */

/*
 * $Header: scsi.h,v 2.3 91/07/31 18:07:04 dbg Exp $
 *
 * Supported SCSI commands
 */

/*
 * Revision 1.1  89/07/05  13:20:13  kak
 * Initial revision
 * 
 */

#ifndef	_SQT_SCSI_H_
#define	_SQT_SCSI_H_

/* class 00 commands */
#define	SCSI_TEST	0x00		/* test unit ready */
#define SCSI_REZERO	0x01		/* rezero unit */
#define SCSI_RSENSE	0x03		/* request sense */
#define SCSI_FORMAT	0x04		/* format unit */
#define	SCSI_READ	0x08		/* read */
#define	SCSI_WRITE	0x0a		/* write */
#define	SCSI_SEEK	0x0b		/* seek */
#define SCSI_TRAN	0x0f		/* translate logical to phys */
#define SCSI_INQUIRY	0x12		/* do inquiry */
#define SCSI_WRITEB	0x13		/* write buffer */
#define SCSI_READB	0x14		/* read buffer */
#define SCSI_MODES	0x15		/* mode select */
#define SCSI_RESRV	0x16		/* reserve unit */
#define SCSI_RELSE	0x17		/* release unit */
#define SCSI_MSENSE	0x1a		/* mode sense */
#define SCSI_STARTOP	0x1b		/* start/stop unit */
#define		SCSI_START_UNIT		0x01
#define		SCSI_STOP_UNIT		0x00
#define SCSI_RDIAG	0x1c		/* receive diagnostic */
#define SCSI_SDIAG	0x1d		/* send diagnostic */
#define		SCSI_SDIAG_REINIT	0x60
#define		SCSI_SDIAG_DUMP_HW	0x61
#define		SCSI_SDIAG_DUMP_RAM	0x62
#define		SCSI_SDIAG_PATCH_HW	0x63
#define		SCSI_SDIAG_PATCH_RAM	0x64
#define		SCSI_SDIAG_SET_RD_ERR	0x65
#define			SCSI_SDIAG_RD_ERR_DEF	0x00
#define			SCSI_SDIAG_RD_ERR_RPT	0x01
#define			SCSI_SDIAG_RD_ERR_NOC	0x02

/* class 01 commands */
#define SCSI_READC	0x25		/* read capacity */
#define		SCSI_FULL_CAP		0x00
#define		SCSI_PART_CAP		0x01

/*
 * unsupported SCSI commands
 */
#define SCSI_SET_THRESHOLD	0x10
#define SCSI_RD_USAGE_CTRS	0x11
#define SCSI_READ_EXTENDED	0x28
#define SCSI_WRITE_EXTENDED	0x2A
#define SCSI_WRITE_AND_VERIFY	0x2E
#define SCSI_VERIFY		0x2F
#define SCSI_SEARCH_DATA_EQUAL	0x31
#define SCSI_SET_LIMITS		0x33

/*
 * Tape commands
 */
#define SCSI_REWIND	0x01		/* Rewind command */
#define SCSI_RETENSION	0x02		/* Retention a tape */
#define SCSI_WFM	0x10		/* Write a file mark */
#define SCSI_SPACE	0x11		/* Space (default blocks) fwd */
#define SCSI_ERASE	0x19		/* Erase a tape */

/*
 * Sizes of data transferred for some standard commands
 */
#define SIZE_CAP	8	/* nbr bytes in Read Capacity input data */
#define SIZE_TRANS	8	/* nbr bytes in Translate input data */

/*
 * structure for SCSI mode select command
 */
#define SCSI_MODES_ILEN		22	/* bytes in data block */
#define SCSI_MODES_DLEN		8	/* length of extent descriptor list */

struct  scsi_modes {
			/* command list */
	u_char	m_type;		/* command type */
	u_char	m_unit;		/* upper 3 bits are unit */
	u_char	m_pad1[2];	/* reserved */
	u_char	m_ilen;		/* length of info passed */
	u_char	m_cont;		/* control byte */
			/* parameter list */
	u_char	m_pad2[3];	/* reserved */
	u_char	m_dlen;		/* length of descript list */
			/* extent descriptor list */
	u_char	m_density;	/* density code */
	u_char	m_pad3[4];	/* reserved */
	u_char	m_bsize[3];	/* block size */
			/* drive parameter list */
	u_char	m_fcode;	/* format code */
	u_char	m_cyls[2];	/* cylinder count */
	u_char	m_heads;	/* data head count */
	u_char	m_rwcc[2];	/* reduced write current cylinder */
	u_char	m_wpc[2];	/* write precompensation cylinder */
	char	m_lzone;	/* landing zone position */
	u_char	m_srate;	/* step pulse output rate code */
};

/* 
 * structures for SCSI format command
 */
struct dlist {			/* defect list entries */
	u_char	d_cyls[3];	/* cyl of defect */
	u_char	d_heads;	/* head nbr */
	u_char	d_bytes[4];	/* bytes from index */
};

#define FORMAT_BUF	1024	/* max bytes for Format Data */
#define MAX_DEFECTS	(FORMAT_BUF / sizeof(struct dlist))

struct	scsi_fmt {
			/* command list */
	u_char	f_type;		/* command type */
	u_char	f_misc;		/* 3 bit unit, data flag, complete list bits */
	u_char	f_data;		/* data pattern */
	u_char	f_ileave[2];	/* interleave */
	u_char	f_pad1;		/* reserved */
			/* defect list */
	u_char	f_full;		/* full or cyl flag */
	u_char	f_spares;	/* nbr spares/cyl */
	u_char	f_dlen[2];	/* length of defect list blocks */
	struct dlist dlist[MAX_DEFECTS]; /* blocks of defect list */
};

/*
 * scsi_fmt.f_misc flags
 */
#define FMT_BBL_DATA	0x10		/* bbl data exists */
#define FMT_CMPLT	0x08		/* bbl data is complete */
#define FMT_USER_FMT	0x04		/* use user-supplied fmt data */
#define FMT_DATA	0x02		/* use user-supplied data pattern */
#define FMT_ALL		(FMT_BBL_DATA | FMT_CMPLT | FMT_USER_FMT | FMT_DATA)

/*
 * scsi_fmt.f_data
 */
#define FMT_PAT		0x6D		/* worst winchester data */

/*
 * scsi_fmt.f_full flags
 */
#define	FMT_FULL	0x00		/* complete drive */
#define	FMT_CYL		0x01		/* single cylinder */

/*
 * scsi_fmt.f_code
 */
#define FMT_FCODE	0x01		/* must be 1 */

/*
 * SCSI_UNIT Macro:
 * 	bits 0-2: unit number on the hardware (up to 8)
 *	bits 3-5: target number (up to 8)
 *	bits 6-8: drive  type (up to 8), index into configuration table 
 *	bits 9-11: SEC board number
 *
 *	On embedded SCSI, the unit number is always 0.
 */
#define SCSI_UNIT(x)	(((x)>>0)&7)
#define	SCSI_TARGET(x)	(((x)>>3)&7)
#define SCSI_TYPE(x)	(((x)>>6)&7)
#define SCSI_BOARD(x)	(((x)>>9)&7)
#define SCSI_DEVNO(x)	(SDEV_SCSISTART + (SCSI_TARGET((x))<<3) + (SCSI_UNIT((x))) )

#endif	/* _SQT_SCSI_H_ */
