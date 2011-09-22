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
 * $Log:	disk.h,v $
 * Revision 2.11  93/08/10  15:57:47  mrt
 * 	Deal with bsd labels
 * 	[93/07/09            rvb]
 * 
 * Revision 2.10  93/03/11  13:57:56  danner
 * 	u_long -> u_int.
 * 	[93/03/09            danner]
 * 
 * Revision 2.9  91/10/07  17:25:18  af
 * 	From 2.5:
 * 	 	p_flag & p_tag -> p_flag
 * 	[91/09/09            rvb]
 * 
 * Revision 2.8  91/05/14  16:22:27  mrt
 * 	Correcting copyright
 * 
 * Revision 2.7  91/03/16  14:45:57  rpd
 * 	Fixed ioctl definitions for ANSI C.
 * 	[91/02/20            rpd]
 * 
 * Revision 2.6  91/02/05  17:16:54  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:42:52  mrt]
 * 
 * Revision 2.5  91/01/08  17:32:46  rpd
 * 	Add V_ABS to log absolute address for 3.0 get/set stat version
 * 	of V_RDABS AND V_WRABS
 * 	[91/01/04  12:18:16  rvb]
 * 
 * Revision 2.4  90/11/26  14:49:32  rvb
 * 	jsb bet me to XMK34, sigh ...
 * 	[90/11/26            rvb]
 * 	Synched 2.5 & 3.0 at I386q (r2.2.1.9) & XMK35 (r2.4)
 * 	Minor syntax cleanup
 * 	[90/11/15            rvb]
 * 
 * Revision 2.2.1.8  90/09/18  08:38:34  rvb
 * 	Move setparams here and get IOC correct.
 * 	[90/09/08            rvb]
 * 
 * Revision 2.2.1.7  90/07/27  11:25:51  rvb
 * 	Fix Intel Copyright as per B. Davies authorization.
 * 	[90/07/27            rvb]
 * 
 * Revision 2.2  90/05/03  15:42:03  dbg
 * 	First checkin.
 * 
 * Revision 2.2.1.6  90/02/28  15:49:27  rvb
 * 	Make DISK_PART value conditional on OLD_PARTITIONS.
 * 	[90/02/28            rvb]
 * 
 * Revision 2.2.1.5  90/01/16  15:54:26  rvb
 * 	FLush pdinfo/vtoc -> evtoc
 * 	[90/01/16            rvb]
 * 
 * Revision 2.2.1.4  90/01/08  13:31:52  rvb
 * 	Add Intel copyright.
 * 	[90/01/08            rvb]
 * 
 * Revision 2.2.1.3  90/01/02  13:50:46  rvb
 * 	Add evtoc definition.
 * 
 * Revision 2.2.1.2  89/12/21  17:59:52  rvb
 * 	Changes from Eugene:
 * 		typedef of altinfo_t & partition_t
 * 	[89/12/07            rvb]
 * 
 * Revision 2.2.1.1  89/10/22  11:33:54  rvb
 * 	Add PART_DISK to indicate the "c" slice.
 * 	[89/10/21            rvb]
 * 
 * Revision 2.2  89/09/25  12:32:57  rvb
 * 	File was provided by Intel 9/18/89.
 * 	[89/09/23            rvb]
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

/*
 * disk.h
 */

#define PART_DISK	2		/* partition number for entire disk */

#define MAX_ALTENTS     253		/* Maximum # of slots for alts */
					/* allowed for in the table. */

#define ALT_SANITY      0xdeadbeef      /* magic # to validate alt table */

struct  alt_table {
	u_short	alt_used;		/* # of alternates already assigned */
	u_short	alt_reserved;		/* # of alternates reserved on disk */
	long	alt_base;		/* 1st sector (abs) of the alt area */
	long	alt_bad[MAX_ALTENTS];	/* list of bad sectors/tracks	*/
};

struct alt_info {			/* table length should be multiple of 512 */
	long	alt_sanity;		/* to validate correctness */
	u_short	alt_version;		/* to corroborate vintage */
	u_short	alt_pad;		/* padding for alignment */
	struct alt_table alt_trk;	/* bad track table */
	struct alt_table alt_sec;	/* bad sector table */
};
typedef struct alt_info altinfo_t;

#define V_NUMPAR        16              /* maximum number of partitions */

#define VTOC_SANE       0x600DDEEE      /* Indicates a sane VTOC */

#define	HDPDLOC		29		/* location of pdinfo/vtoc */
#define	LBLLOC		1		/* label block for xxxbsd */

/* Partition permission flags */
#define V_OPEN          0x100           /* Partition open (for driver use) */
#define V_VALID         0x200           /* Partition is valid to use */

/* driver ioctl() commands */
#define V_CONFIG        _IOW('v',1,union io_arg)/* Configure Drive */
#define V_REMOUNT       _IO('v',2)    		/* Remount Drive */
#define V_ADDBAD        _IOW('v',3,union io_arg)/* Add Bad Sector */
#define V_GETPARMS      _IOR('v',4,struct disk_parms)   /* Get drive/partition parameters */
#define V_FORMAT        _IOW('v',5,union io_arg)/* Format track(s) */
#define V_PDLOC		_IOR('v',6,int)		/* Ask driver where pdinfo is on disk */

#define V_ABS		_IOW('v',9,int)		/* set a sector for an absolute addr */
#define V_RDABS		_IOW('v',10,struct absio)/* Read a sector at an absolute addr */
#define V_WRABS		_IOW('v',11,struct absio)/* Write a sector to absolute addr */
#define V_VERIFY	_IOWR('v',12,union vfy_io)/* Read verify sector(s) */
#define V_XFORMAT	_IO('v',13)		/* Selectively mark sectors as bad */
#define V_SETPARMS	_IOW('v',14,int)	/* Set drivers parameters */



/* Sanity word for the physical description area */
#define VALID_PD		0xCA5E600D

struct localpartition	{
	u_int 	p_flag;			/*permision flags*/
	long	p_start;		/*physical start sector no of partition*/
	long	p_size;			/*# of physical sectors in partition*/
};
typedef struct localpartition localpartition_t;

struct evtoc {
	u_int 	fill0[6];
	u_int 	cyls;			/*number of cylinders per drive*/
	u_int 	tracks;			/*number tracks per cylinder*/
	u_int 	sectors;		/*number sectors per track*/
	u_int 	fill1[13];
	u_int 	version;		/*layout version*/
	u_int 	alt_ptr;		/*byte offset of alternates table*/
	u_short	alt_len;		/*byte length of alternates table*/
	u_int 	sanity;			/*to verify vtoc sanity*/
	u_int 	xcyls;			/*number of cylinders per drive*/
	u_int 	xtracks;		/*number tracks per cylinder*/
	u_int 	xsectors;		/*number sectors per track*/
	u_short	nparts;			/*number of partitions*/
	u_short	fill2;			/*pad for 286 compiler*/
	char	label[40];
	struct localpartition part[V_NUMPAR];/*partition headers*/
	char	fill[512-352];
};

union   io_arg {
	struct  {
		u_short	ncyl;		/* number of cylinders on drive */
		u_char	nhead;		/* number of heads/cyl */
		u_char	nsec;		/* number of sectors/track */
		u_short	secsiz;		/* number of bytes/sector */
		} ia_cd;		/* used for Configure Drive cmd */
	struct  {
		u_short	flags;		/* flags (see below) */
		long	bad_sector;	/* absolute sector number */
		long	new_sector;	/* RETURNED alternate sect assigned */
		} ia_abs;		/* used for Add Bad Sector cmd */
	struct  {
		u_short	start_trk;	/* first track # */
		u_short	num_trks;	/* number of tracks to format */
		u_short	intlv;		/* interleave factor */
		} ia_fmt;		/* used for Format Tracks cmd */
	struct	{
		u_short	start_trk;	/* first track	*/
		char	*intlv_tbl;	/* interleave table */
		} ia_xfmt;		/* used for the V_XFORMAT ioctl */
};

/*
 * Data structure for the V_VERIFY ioctl
 */
union	vfy_io	{
	struct	{
		long	abs_sec;	/* absolute sector number        */
		u_short	num_sec;	/* number of sectors to verify   */
		u_short	time_flg;	/* flag to indicate time the ops */
		}vfy_in;
	struct	{
		long	deltatime;	/* duration of operation */
		u_short	err_code;	/* reason for failure    */
		}vfy_out;
};


/* data structure returned by the Get Parameters ioctl: */
struct  disk_parms {
/*00*/	char	dp_type;		/* Disk type (see below) */
	u_char	dp_heads;		/* Number of heads */
	u_short	dp_cyls;		/* Number of cylinders */
/*04*/	u_char	dp_sectors;		/* Number of sectors/track */
	u_short	dp_secsiz;		/* Number of bytes/sector */
					/* for this partition: */
/*08*/	u_short	dp_ptag;		/* Partition tag */
	u_short	dp_pflag;		/* Partition flag */
/*0c*/	long	dp_pstartsec;		/* Starting absolute sector number */
/*10*/	long	dp_pnumsec;		/* Number of sectors */
/*14*/	u_char	dp_dosheads;		/* Number of heads */
	u_short	dp_doscyls;		/* Number of cylinders */
/*18*/	u_char	dp_dossectors;		/* Number of sectors/track */
};

/* Disk types for disk_parms.dp_type: */
#define DPT_WINI        1               /* Winchester disk */
#define DPT_FLOPPY      2               /* Floppy */
#define DPT_OTHER       3               /* Other type of disk */
#define DPT_NOTDISK     0               /* Not a disk device */

/* Data structure for V_RDABS/V_WRABS ioctl's */
struct absio {
	long	abs_sec;		/* Absolute sector number (from 0) */
	char	*abs_buf;		/* Sector buffer */
};


#define BOOTSZ		446		/* size of boot code in master boot block */
#define FD_NUMPART	4		/* number of 'partitions' in fdisk table */
#define ACTIVE		128		/* indicator of active partition */
#define	BOOT_MAGIC	0xAA55		/* signature of the boot record */
#define	UNIXOS		99		/* UNIX partition */
#define BSDOS		165
extern	int		OS;		/* what partition we came from */

/*
 * structure to hold the fdisk partition table
 */
struct ipart {
	u_char	bootid;			/* bootable or not */
	u_char	beghead;		/* beginning head, sector, cylinder */
	u_char	begsect;		/* begcyl is a 10-bit number. High 2 bits */
	u_char	begcyl;			/*     are in begsect. */
	u_char	systid;			/* OS type */
	u_char	endhead;		/* ending head, sector, cylinder */
	u_char	endsect;		/* endcyl is a 10-bit number.  High 2 bits */
	u_char	endcyl;			/*     are in endsect. */
	long	relsect;		/* first sector relative to start of disk */
	long	numsect;		/* number of sectors in partition */
};

/*
 * structure to hold master boot block in physical sector 0 of the disk.
 * Note that partitions stuff can't be directly included in the structure
 * because of lameo '386 compiler alignment design.
 */
struct  mboot {				     /* master boot block */
	char	bootinst[BOOTSZ];
	char	parts[FD_NUMPART * sizeof(struct ipart)];
	u_short	signature;
};
