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
 * $Log:	hdreg.h,v $
 * Revision 2.7  91/10/07  17:25:45  af
 * 	Made port addresses dependent on base address, cleaned up hh struct
 * 	[91/08/07            mg32]
 * 
 * Revision 2.6  91/05/14  16:23:39  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:17:17  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:43:16  mrt]
 * 
 * Revision 2.4  90/11/26  14:49:40  rvb
 * 	jsb bet me to XMK34, sigh ...
 * 	[90/11/26            rvb]
 * 	Synched 2.5 & 3.0 at I386q (r1.5.1.5) & XMK35 (r2.4)
 * 	[90/11/15            rvb]
 * 
 * Revision 1.5.1.4  90/07/27  11:25:57  rvb
 * 	Fix Intel Copyright as per B. Davies authorization.
 * 	[90/07/27            rvb]
 * 
 * Revision 2.2  90/05/03  15:42:24  dbg
 * 	First checkin.
 * 
 * Revision 1.5.1.3  90/03/29  18:59:52  rvb
 * 	Added some more ERROR_ codes.
 * 	[90/03/28            rvb]
 * 
 * Revision 1.5.1.2  90/01/08  13:31:35  rvb
 * 	Add Intel copyright.
 * 	[90/01/08            rvb]
 * 
 * Revision 1.5.1.1  89/12/21  18:00:06  rvb
 * 	Changes from Eugene:
 * 	  1) ERROR_BBD, ERROR_ECC, MAX_RETRIES, MAX_ALTBUFS
 * 	  2) field rename in hh:
 * 		controller_busy, retry_count, restore_request
 * 	[89/12/07            rvb]
 * 
 * Revision 1.5  89/09/09  15:22:00  rvb
 * 	hd.h -> hdreg.h; hd.h is now use for configuration.
 * 	[89/09/09            rvb]
 * 
 * Revision 1.4  89/07/17  10:40:31  rvb
 * 	Olivetti Changes to X79 upto 5/9/89:
 * 	[89/07/11            rvb]
 * 
 * Revision 1.3  89/02/26  12:36:47  gm0w
 * 	Changes for cleanup.
 * 
 */
 
/*
  Copyright 1988, 1989 by Intel Corporation, Santa Clara, California.

		All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Intel
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#define BOOTRECORDSIGNATURE			(0x55aa & 0x00ff)
#define FIXED_DISK_REG(ctrl)			((ctrl)?0x376:0x3f6)
#define FIXEDBITS				0x0a0
#define WHOLE_DISK(unit)			(((unit)<<4)+PART_DISK)

#define PORT_DATA(addr)				(0x0+(addr))
#define PORT_ERROR(addr)			(0x1+(addr))
#define PORT_PRECOMP(addr)			(0x1+(addr))
#define PORT_NSECTOR(addr)			(0x2+(addr))
#define PORT_SECTOR(addr)			(0x3+(addr))
#define PORT_CYLINDERLOWBYTE(addr)		(0x4+(addr))
#define PORT_CYLINDERHIBYTE(addr)		(0x5+(addr))
#define PORT_DRIVE_HEADREGISTER(addr)		(0x6+(addr))
#define PORT_STATUS(addr)			(0x7+(addr))
#define PORT_COMMAND(addr)			(0x7+(addr))

#define STAT_BUSY				0x080
#define STAT_READY 				0x040
#define STAT_WRITEFAULT				0x020
#define STAT_SEEKDONE				0x010
#define STAT_DATAREQUEST			0x008
#define STAT_ECC				0x004
#define STAT_INDEX 				0x002
#define STAT_ERROR				0x001 

#define CMD_RESTORE 				0x010
#define CMD_SEEK 				0x070
#define CMD_READ				0x020
#define CMD_WRITE				0x030
#define CMD_FORMAT				0x050
#define CMD_READVERIFY				0x040
#define CMD_DIAGNOSE				0x090
#define CMD_SETPARAMETERS			0x091

#define PATIENCE	3000000		/* how long to wait for controller */
#define PARTITION(z)	(minor(z) & 0x0f)
#define UNIT(z)		((minor(z) >> 4) & 0x03)

#define PDLOCATION	29
#define BAD_BLK		0x80			/* needed for V_VERIFY */

#define NDRIVES 2				/* drives per controller */
#define SECSIZE 512

/*  hh holds the state of the one and only (stupid board) current 
    block I/O request
*/
    
struct hh {
	u_char	curdrive;		/* drive the controller is using */
	u_char	controller_busy;	/* controller can't take cmd now */
	u_char	retry_count;		/* # of times cmd has been tried */
	u_char	restore_request;	/* restore command */
	int	rw_addr;		/* ram addr to read/write sector */
	int	physblock;		/* block # relative to partition 0 */
	u_int 	single_mode;		/* 1 = transfer one block each time */
	u_int	cylinder;		/* cylinder # rel. to part. 0 */
	u_int	head;			/* as it looks */
	u_int	sector;			/* as it looks */
	u_int	blockcount;		/* blocks done so far */
	u_int	blocktotal;		/* total blocks this request */
	u_int	start_of_unix[NDRIVES];	/* unix vs dos partitions */
};

/* the boot record partition table is documented in IBM AT Tech. Ref p. 9-6 */
struct 	boot_record	{
	u_char	boot_ind;	/* if it == 0x80, this partition is active */
	u_char	head;		/* driver does not look at this field */
	u_char	sector;		/* driver does not look at this field */
	u_char	cylinder;	/* driver does not look at this field */

	u_char	sys_ind;	/* driver does not look at this field */
	u_char	end_head;	/* driver does not look at this field */
	u_char	end_sector;	/* driver does not look at this field */
	u_char	end_cylinder;	/* driver does not look at this field */

	u_int	rel_sect;	/* where unix starts if boot_ind == 0x80 */
	u_int	num_sects;	/* driver does not look at this field */
};
