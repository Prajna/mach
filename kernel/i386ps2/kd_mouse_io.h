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
 * $Log:	kd_mouse_io.h,v $
 * Revision 2.2  93/02/04  07:58:49  danner
 * 	Removed non-ansi-style ioctl definitions.
 * 	[93/01/18            chs]
 * 
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */

/* Mouse line discipline number */
#define MSLINEDISC  5         /* Must match entry in linesw[] in  tty_conf.c */

/* Mouse ioctls */
#define MSIC_STATUS     _IOR('m',1,long)   /* return pointer to status struct */
#define MSIC_DISABLE    _IO('m',2)
#define MSIC_ENABLE     _IO('m',3)
#define MSIC_SCALE      _IOW('m',4,long)   /* actually expects a char */
#define MSIC_SAMPLE     _IOW('m',5,long)   /* actually expects a short */
#define MSIC_SAMP	MSIC_SAMPLE	   /* 4.3 compatibilty */
#define MSIC_RESL       _IOW('m',6,long)   /* actually expects a short */
#define MSIC_PDIC       _IOR('m',7,long)   /* actually returns a char */

/*
 * SAMPLE RATE CHART -- Data for MSIC_SAMP ioctl
 *      Sample Rate     Data  in Reports/second
 */
#define MS_RATE_10      0x0A
#define MS_RATE_20      0x14
#define MS_RATE_40      0x28
#define MS_RATE_60      0x3C
#define MS_RATE_80      0x50
#define MS_RATE_100     0x64
#define MS_RATE_200     0xC8

/*
 * SET RESOLUTION CHART -- Data for MSIC_RESL ioctl
 *                                          RESOLUTION
 *                      Data       Counts/mm          Counts/inch
 */
#define MS_RES_200      0x00    /*      8                  200        */
#define MS_RES_100      0x01    /*      4                  100        */
#define MS_RES_50       0x02    /*      2                   50        */
#define MS_RES_25       0x03    /*      1                   25        */

/*
 * SET SCALING FACTOR -- Data for MSIC_SCALE ioctl
 *
 *                      Data         Scale
 */
#define MS_SCALE_1      0x01         /* 1:1 */
#define MS_SCALE_2      0x02         /* 2:1 */

/* Status - Data for MSIC_STATUS is a pointer to this structure */
struct mouse_status {
     u_char interface_status;
     u_char data_package_size;
     u_short flag_word;
     u_short current_resolution;
     u_short current_sample_rate;
   };


