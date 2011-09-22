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

/* @(#)63       1.1  mk/src/latest/kernel/i386/PS2/fdreg.h, MACH 4/4/91 10:22:07 */
#ifndef _H_FD
#define _H_FD
/*
 * COMPONENT_NAME: (SYSXFD) Diskette Device Driver Header File
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 */

/*
 * HISTORY
 * $Log:	fdreg.h,v $
 * Revision 2.3  93/03/11  14:09:08  danner
 * 	u_long -> u_int
 * 	[93/03/09            danner]
 * 
 * Revision 2.2  93/02/04  07:59:42  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */

/* Diskette types
 */
#define	D_48	2		/* 48 tpi, (360K, 5.25" drive) */
#define	D_135H	8		/* 135 tpi, (1.44M, 3.5" drive) */

/*
 * Diskette minor numbers use the following convention:
 *
 *       Diskette Diameter
 *              |
 *    Drive     |     
 *    Number    |        Diskette Type
 *    ______    |    _____________________
 *   |      |  | |  |                     |
 *    0    0    0    0    0    0    0    0
 *
 */

/*
 * Defines for each of the diskette minor device types.            
 */

/* NEEDS WORK -- these #'s should be merged with #'s in fd_defs.h */
#define	FDGENERIC	0	/* generic minor device number */
#define	FD1440_3	0x01	/* 3.5 inch, 1.44M diskette */
#define	FD720_3		0x02	/* 3.5 inch, 720K diskette */
#define	FD1200_5	0x21	/* 5.25 inch, 1.2M diskette */
#define	FD360_5		0x22	/* 5.25 inch, 360K diskette */

/*
 * Diskette device driver ioctl operations.
 */

#define	FDIOC	('F'<<8)
#define	FDIOCDSELDRV	(FDIOC|1)	/* de-select the drive */
#define	FDIOCFORMAT	(FDIOC|2)	/* format track */
#define	FDIOCGETPARMS	(FDIOC|3)	/* get diskette & drive
						characteristics */
#define	FDIOCGINFO	(FDIOC|4)	/* get drive info */
#define	FDIOCNORETRY	(FDIOC|5)	/* disable retries on errors */
#define	FDIOCREADID	(FDIOC|6)	/* read first address field found */
#define	FDIOCRECAL	(FDIOC|7)	/* recalibrate the drive */
#define	FDIOCRESET	(FDIOC|8)	/* reset diskette controller */
#define	FDIOCRETRY	(FDIOC|9)	/* enable retries on errors */
#define	FDIOCSEEK	(FDIOC|10)	/* seek to designated cylinder */
#define	FDIOCSELDRV	(FDIOC|11)	/* select the drive */
#define	FDIOCSETPARMS	(FDIOC|12)	/* set diskette characteristics */
#define	FDIOCSETTLE	(FDIOC|13)	/* check the head settle time */
#define	FDIOCSINFO	(FDIOC|14)	/* set drive info */
#define	FDIOCSPEED	(FDIOC|15)	/* check the diskette rotation speed */
#define	FDIOCSTATUS	(FDIOC|16)	/* get the drive status */

/*
 * The following is the structure used by the FDIOCGINFO and FDIOCSINFO
 * ioctl operations.
 */

struct fdinfo {
	short	type;			/* drive type (D_48, D_135H) */
	short	info_reserved;
	int	nsects;			/* number of sectors/track */
	int	sides;			/* number of sides */
	int	ncyls;			/* number of cylinders */
};

/*
 * The following is the structure used by the FDIOCSTATUS ioctl
 * system call.                                               
 */

struct fd_status {
	u_char	status1;	/* status byte 1 */
#define	FDNODRIVE	1	/* no drive selected */
#define	FDDRIVE0	2	/* drive 0 selected */
#define	FDDRIVE1	4	/* drive 1 selected */
#define	FD250KBPS	8	/* 250 kbps data transfer rate */
#define	FD300KBPS	16	/* 300 kbps data transfer rate */
#define	FD500KBPS	32	/* 500 kbps data transfer rare */
	u_char	status2;	/* status byte 2 */
#define	FD3INCHHIGH	2	/* 1.44 Megabyte drive */
#define	FD5INCHLOW	8	/* 360 Kilobyte drive */
#define	FDRETRY		16	/* retries are enabled */
#define	FDTIMEOUT	32	/* device driver timer expired */
	u_char	status3;	/* status byte 3 */
#define	FDDOUBLE	2	/* doubled-sided diskette */
#define	FD9PRTRCK	8	/* 9 sectors per track */
#define	FD15PRTRCK	16	/* 15 sectors per track */
#define	FD18PRTRCK	32	/* 18 sectors per track */
#define	FD40CYLS	64	/* 40 cylinders */
#define	FD80CYLS	28	/* 80 cylinders */
	u_char	command0;	/* command phase byte 0 */
	u_char	command1;	/* command phase byte 1 */
	u_char	mainstat;	/* controller main status register */
	u_char	dsktchng;	/* controller disk changed register */
	u_char	result0;	/* result phase status register 0 */
	u_char	result1;	/* result phase status register 1 */
	u_char	result2;	/* result phase status register 2 */
	u_char	result3;	/* result phase status register 3 */
	u_char	cylinder_num; 	/* cylinder number */
	u_char	head_num;	/* head number */
	u_char	sector_num;	/* sector number */
	u_char	bytes_num;	/* number of bytes per sector */
	u_char	head_settle_time;	/* time in milliseconds */
	u_int	motor_speed;	/* time in milliseconds */
	u_int	Mbytes_read;	/* number of Mbytes read since last config */
	u_int	Mbytes_written;	/* number of Mbytes written since last
					config */
	u_int	status_reserved0;
	u_int	status_reserved1;
};

/*
 * The following is the structure used by the FDIOCSETPARMS and the
 * FDIOCGETPARMS ioctl system calls.
 */

struct fdparms {
	u_char	diskette_type;	/* ORed together flags describing type */
	u_char	sector_size;	/* encoded sector size */
	u_char	tracks_per_cylinder;	/* decimal value */
	u_char	data_rate;	/* encoded data rate */
	u_char	head_settle_time;	/* head settle time in milliseconds */
	u_char	head_load;	/* encoded head settle time */
	u_char	fill_byte;	/* hex data pattern */
	u_char	step_rate;	/* encoded step rate time */
	u_char	step_rate_time;	/* step rate in milliseconds */
	u_char	gap;		/* r/w gap */
	u_char	format_gap;	/* format gap */
	u_char	data_length;	/* see data sheet for usage info */
	u_char	motor_off_time;	/* time before motor is turned off (secs) */
	u_short	sectors_per_track;	/* decimal value */
	u_short	sectors_per_cylinder;	/* decimal value */
	u_short	cylinders_per_disk;	/* decimal value */
	u_short	bytes_per_sector;	/* decimal value */
	u_short	number_of_blocks;	/* total number of sectors on the 
						diskette */
	u_short	motor_start;	/* motor start time in milliseconds */
	u_short	motor_ticks;	/* motor start time in timer ticks */
};

/* BASED ON R2 1.49 src */

/* New for OSF:
*/
#define		splfd	spl5
#define		SPLFD	5

#define FDMAXDRIVES	2	/* maximum number of drives allowed */
#ifndef NFD
#define NFD		FDMAXDRIVES
#endif
#define next_unit(unit)		( ((unit) + 1) % FDMAXDRIVES )
/* NEEDS WORK -- next_unit() maybe could use fdadapter.fd_numd ?? */

#define FDMAXRETRIES	6	/* maximum # of retries after overrun or CRC */

#define FDDRIVEMASK     0xc0	/* mask to extract the drive number */
#define FDTYPEMASK      0x3f	/* mask to delete the drive number */
#define FDSIZEMASK      0xdf	/* mask to delete the diskette diameter flag */

#define fd_drive(dev)	( (minor((dev)) & FDDRIVEMASK) >> 6 )

/* Differnt characteristics.  These are bitmasks for minor(devno) */
#define FD_HIGH		0x01		/* high density */
#define FD_LOW		0x02		/* low density */
#define FD_3		0x00		/* 3 1/2" drive */
#define FD_5		0x20		/* 5 1/4" drive */

/* Different types of minors:  minor(devno) & FDTYPEMASK */
#define FD_GENERIC	0
#define FD_3H		(FD_3 | FD_HIGH)
#define FD_3L		(FD_3 | FD_LOW)
#define FD_5H		(FD_5 | FD_HIGH)
#define FD_5L		(FD_5 | FD_LOW)

#define FDSUCCESS          0	/* rc for successful subroutine completion */
#define FDFAILURE         -1	/* rc for unsuccessful subroutine completion */

/* Commands for fdexecute_command():
**  POLLED commands have the 0x8000 bit set
**  Other extended commands (non ABIOS) have the 0x100 bit set
**
** Note: These commands need to use the same value as the ABIOS request
** number in order to facilitate easy retries (see fd_intr.c).
*/
#define FDABIOS_INIT		ABIOS_LOGICAL_PARAMETER
#define FDABIOS_RESET		ABIOS_RESET
#define FDREAD_PARMS		ABIOS_READ_PARAMETER
#define FDREAD_DATA		ABIOS_READ
#define FDMOTOR_OFF		ABIOS_FD_TURN_OFF_MOTOR
#define FDWRITE_DATA		ABIOS_WRITE
#define FDRMEDIA_PARMS		ABIOS_FD_READ_MEDIA_TYPE
#define FDFORMAT_TRACK		ABIOS_ADDITIONAL_XFER
#define FDDISK_SAME		ABIOS_FD_CHANGE_SIGNAL_STATUS
#define FD_SETDP		ABIOS_WRITE_PARAMETER
#define FD_SETMT		ABIOS_FD_SET_MEDIA_TYPE

#define FDABIOS_POLL		0x8000
#define FDREAD_DATA_POLL	(FDREAD_DATA | FDABIOS_POLL)
#define FDWRITE_DATA_POLL	(FDWRITE_DATA | FDABIOS_POLL)
#define FDFORMAT_TRACK_POLL	(FDFORMAT_TRACK | FDABIOS_POLL)
#define FD_SETDP_POLL		(FD_SETDP | FDABIOS_POLL)
#define FD_SETMT_POLL		(FD_SETMT | FDABIOS_POLL)

/* This should be in this order...val & ~0x100 is used as an index! */
#define FDLOAD_144		0x100
#define FDLOAD_720		0x101
#define FDLOAD_360		0x102

/*  The following are for FDDISK_TRUE.  Not that they cannot start at 0.
** See fdissue_command case for FDISK_SAME.
*/
#define DISK_CHANGED		0x01
#define DISK_SAME		0x02

/* Structure for storing the physical address of data.
*/
struct phys_add {
	u_char cylinder;
	u_char head;
	u_char sector;
	u_int transfer_length;	/* length (in bytes) of the current transfer */
};

/* fdp->drive_state values.
*/
#define FDSTAT_OPEN	0x01	/* drive is open */
#define FDSTAT_CLOS	0x00	/* drive is not open */
#define FDSTAT_EXOP	0x02	/* drive is exclusively opened */

struct floppy {
	dev_t device_number;	/* save off the 'devno' for later use */
	u_short abios_flags;	/* Holds ABIOS ctrl flags */
	u_char fd_change;	/* Support for ABIOS change signal */
	u_char open_check;	/* TRUE if doing open() media checks */
	u_char close_waiting;	/* TRUE or FALSE that the close */
	u_char drive_state;	/* current state of the drive */
	u_char drive_type;	/* type of diskette drive, uses same
				 * defines as fdinfo structure. */
	u_char diskette_type;	/* type of diskette in the drive */
	u_char sectors_per_track;	/* number of sectors per track */
	u_char sectors_per_cylinder;	/* number of sectors per cylinder */
	u_char tracks_per_cylinder;	/* number of tracks per cylinder */
	u_char cylinders_per_disk;	/* number of cylinders on the disk */
	u_char data_rate;	/* data rate setting for diskette type */
	u_char gap;		/* inter-sector gap length */
	u_char format_gap;	/* format inter-sector gap length */
	u_char timeout_flag;	/* TRUE means that a timeout has occurred */
	u_char retry_flag;	/* 0 = disabled, anything else = enabled */
	u_char fill_byte;	/* fill byte for formmating */
	u_char cylinder_id;	/* cylinder number for format ioctls */
	u_char head_id;		/* head number for format ioctls */
	u_char head_settle_time;	/* head settle time (milliseconds) */
	u_char data_length;	/* data length controller parameter */
	u_char motor_off_time;	/* inactive time before motor turned
				 * off (seconds) */
	u_short format_size;	/* size of format buffer */
	u_short bytes_per_sector;	/* raw sector size */
	u_short number_of_blocks;	/* total # of blocks on the disk */
	u_short motor_start;	/* motor start time (in milliseconds) */
	u_short start_block;	/* starting block # for read or write */
	u_int modified_bcount;	/* modified byte count for transfers */
	u_int retry_count;	/* retry count for the i/o operation */
	u_int sector_count;	/* total number of sectors to read/write */
	u_int sectors_done;	/* number of sectors already transfered */
	u_int buf_offset;	/* the offset in the data buffer for
				 * this io */
	int close_sleep;	/* sleep event list anchor for close routine */

	/*  The next four members keep track of the number of bytes
	** transfered since the drive was last configured.
	*/
	u_int read_count_bytes;
	u_int read_count_megabytes;
	u_int write_count_bytes;
	u_int write_count_megabytes;

	char *format_buffer;	/* pointer to buffer with format info */
	struct buf *headptr;	/* pointer to first queued buffer header */
	struct buf *tailptr;	/* pointer to last queued buffer header */

	struct phys_add start;	/* current transfer address on diskette */
	u_char transfer_split;	/* transfer spans cylinders */
	u_char e_bytesps;	/* encoded bytes per secter */
};

/* defines for the fd_change field above.  It describles the various states
** the diskette change signal can be in.  After our 1st read it is an
** error to recieve this signal, but it is ok on our first read.
*/
#define FDCHANGE_NEW	0
#define FDCHANGE_INIT	1

#define FDMOT_WD	0x10
#define FDINT_WD	0x11
#define TIMEOUT(fdw)	(timeout((fdw).func,(caddr_t)&(fdw),(fdw).time))
#define UNTIMEOUT(fdw)	(untimeout((fdw).func,(caddr_t)&(fdw)))
struct fdwatchdog {
	void (*func)();
	int time;
	struct floppy *fdp;
	u_char type;
};

/* Internal error code values for fdlog_error
*/
#define FD_UNSUP_DRIVE		0
#define FD_UNSUP_SECT		1
#define FD_UNSUP_MINOR		2
#define FD_ERR_LOSTINT		3
#define FD_ERR_NOMEDIA		4
#define FD_ERR_RETRY		5
#define FD_ERR_CHMEDIA		6

/* Structure describing controller (global) status.
*/
#define FDFREE           0		/* need_controller */
#define FDBUSY           1
#define FDNOTWAITING     0		/* close_waiting */
#define FDWAITING        1

struct adapter_structure {
	u_char fdintr_status;		/* is the interrupt handler busy? */
	u_char need_controller;		/* is the controller needed? */
	u_char fd_intl;			/* interrupt level as told by ABIOS */
	u_char fd_numd;			/* number of units really there */
	struct intr *fdhandler;		/* ptr to interrupt handler struct */
	struct fdwatchdog int_wd;	/* interrupt watchdog timer struct */
	struct fdwatchdog mot_wd;	/* motor watchdog timer structure */

	/* This is the ABIOS interface stuff */
	struct Fd_request fd_req;		/* request block */
	int fd_flag;				/* save info from ABIOS */
	int motor_off_delay;			/* motor off delay */
	int need_sleep;				/* for need_controller */
	caddr_t vaddr;				/* for fddoor_check */
};

/*  This structure is used to store the default configuration for
** the various types of drives in order to make the set parms ioctl
** easy to implemnt.
*/
struct fd_data {
	short sectors;
	short cylinders;
	short heads;
	short gap;
	short format_gap;
	short data_len;
};
#endif /* _H_FD */
