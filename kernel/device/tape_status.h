/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	tape_status.h,v $
 * Revision 2.7  91/05/14  16:01:44  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/05/13  06:02:28  af
 * 	Modified tape_status structure to hold more info
 * 	either way.  Added flags field to hold special
 * 	behavioural properties such as rewind-on-close.
 * 	[91/05/12            af]
 * 
 * Revision 2.5  91/03/16  14:43:42  rpd
 * 	Fixed ioctl definitions for ANSI C.
 * 	[91/02/20            rpd]
 * 
 * Revision 2.4  91/02/05  17:10:21  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:30:46  mrt]
 * 
 * Revision 2.3  90/12/05  23:28:29  af
 * 	Created.
 * 
 * Revision 2.2  90/12/05  20:42:10  af
 * 	Created, from BSD 4.3Reno mtio.h.
 * 	[90/11/11            af]
 * 
 */
/*
 * Copyright (c) 1982, 1986 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)mtio.h	7.4 (Berkeley) 8/31/88
 */

#ifndef	_TAPE_STATUS_H_
#define	_TAPE_STATUS_H_

/*
 * Tape status
 */

struct tape_status {
	unsigned int	mt_type;
	unsigned int	speed;
	unsigned int	density;
	unsigned int	flags;
#	define TAPE_FLG_REWIND	0x1
#	define TAPE_FLG_WP	0x2
};
#define	TAPE_STATUS_COUNT	(sizeof(struct tape_status)/sizeof(int))
#define	TAPE_STATUS		(('m'<<16) + 1)

/*
 * Constants for mt_type.  These are the same
 * for controllers compatible with the types listed.
 */
#define	MT_ISTS		0x01		/* TS-11 */
#define	MT_ISHT		0x02		/* TM03 Massbus: TE16, TU45, TU77 */
#define	MT_ISTM		0x03		/* TM11/TE10 Unibus */
#define	MT_ISMT		0x04		/* TM78/TU78 Massbus */
#define	MT_ISUT		0x05		/* SI TU-45 emulation on Unibus */
#define	MT_ISCPC	0x06		/* SUN */
#define	MT_ISAR		0x07		/* SUN */
#define	MT_ISTMSCP	0x08		/* DEC TMSCP protocol (TU81, TK50) */
#define	MT_ISCY		0x09		/* CCI Cipher */
#define	MT_ISSCSI	0x0a		/* SCSI tape (all brands) */


/*
 * Set status parameters
 */

struct tape_params {
	unsigned int	mt_operation;
	unsigned int	mt_repeat_count;
};

/* operations */
#define MTWEOF		0	/* write an end-of-file record */
#define MTFSF		1	/* forward space file */
#define MTBSF		2	/* backward space file */
#define MTFSR		3	/* forward space record */
#define MTBSR		4	/* backward space record */
#define MTREW		5	/* rewind */
#define MTOFFL		6	/* rewind and put the drive offline */
#define MTNOP		7	/* no operation, sets status only */
#define MTCACHE		8	/* enable controller cache */
#define MTNOCACHE	9	/* disable controller cache */


/*
 * U*x compatibility
 */

/* structure for MTIOCGET - mag tape get status command */

struct mtget {
	short	mt_type;	/* type of magtape device */
/* the following two registers are grossly device dependent */
	short	mt_dsreg;	/* ``drive status'' register */
	short	mt_erreg;	/* ``error'' register */
/* end device-dependent registers */
	short	mt_resid;	/* residual count */
/* the following two are not yet implemented */
	unsigned long	mt_fileno;	/* file number of current position */
	unsigned long	mt_blkno;	/* block number of current position */
/* end not yet implemented */
};


/* mag tape io control commands */
#define	MTIOCTOP	_IOW('m', 1, struct tape_params)/* do a mag tape op */
#define	MTIOCGET	_IOR('m', 2, struct mtget)	/* get tape status */
#define MTIOCIEOT	_IO('m', 3)			/* ignore EOT error */
#define MTIOCEEOT	_IO('m', 4)			/* enable EOT error */


#endif	_TAPE_STATUS_H_
