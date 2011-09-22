/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * Copyright 1988, 1989, 1990, 1991 by Intel Corporation,
 * Santa Clara, California.
 * 
 *                          All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and that
 * both the copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Intel not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 * 
 * INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
 * SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN ACTION OF CONTRACT, NEGLIGENCE, OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */
/*
 * HISTORY
 * $Log:	sd.h,v $
 * Revision 2.4  91/06/18  20:50:35  jsb
 * 	New copyright from Intel.
 * 	[91/06/18  20:07:53  jsb]
 * 
 * Revision 2.3  91/06/06  17:04:57  jsb
 * 	From Don Cameron: changed NDRIVES from 1 to 4.
 * 	[91/05/13  17:10:29  jsb]
 * 
 * Revision 2.2  90/12/04  14:47:45  jsb
 * 	First checkin.
 * 	[90/12/04  10:58:27  jsb]
 * 
 */ 

#define BOOTRECORDSIGNATURE			(0x55aa & 0x00ff)

#define PARTITION(z)	(minor(z) & 0x0f)
#define UNIT(z)		(minor(z) >> 4)

#define PDLOCATION	29	

#define NDRIVES 4
#define SECSIZE 512

#define GETALTTBL	(('H' <<8) | 1)
#define FMTBAD		(('H' <<8) | 2)
#define BAD_BLK		0x80		/* needed for V_VERIFY */

/*
 * Driver states
 */
#define NULL_STEP	0
#define MSG_STEP	1
#define STATUS_STEP	2
#define DISC_STEP	3
#define DONE_STEP	4
#define CMD_STEP	5
#define RSEL_STEP	6


typedef long paddr_t;

/*  hh holds the state of the current block I/O request for each drive */
struct hh {
	unsigned char	cmd[12];	/* SCSI command bytes */
	short	cmd_len;		/* Command length */
	char	cmd_busy;		/* Command not yet complete */
	char	direction;		/* Data transfer direction, 0 = out */
	long	transfer_length;	/* How much data to move */
	unsigned char	cmd_st;		/* Command completion status */
	unsigned char	cmd_msg;	/* Command completion message byte */
	unsigned char	state;		/* Current state (tape) */
	unsigned char	step;		/* Current step (tape) */
	unsigned char	retry_count;	/* # of times cmd has been tried */
	unsigned char	status;		/* stat read from controller port */
	struct	buf 	buflst;		/* head of list of IO buffers */
	short		buf_io;		/* TRUE if using IO buffer */
	unsigned char	wait_to_drain;	/* used by format, open, close */
	unsigned char	restore_request;/* restore command */
	unsigned char	format_request;	/* let's do some formatting */
	unsigned char	open_close_ioctl_mutex;	/* for open/closing/fmt partitions */
	daddr_t	physblock;		/* block # relative to partition 0 */
	paddr_t	rw_addr;		/* ram addr to read/write from/to */
	unsigned int	cylinder;	/* cylinder # rel. to part. 0 */
	unsigned int	head;		/* as it looks */
	unsigned int	sector;		/* as it looks */
};
