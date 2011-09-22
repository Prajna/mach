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
 * $Log:	conf_sd.c,v $
 * Revision 2.4  93/01/14  17:56:14  danner
 * 	Added vm_types.h include.
 * 	[93/01/14            danner]
 * 
 * Revision 2.3  91/07/31  18:05:33  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:02:49  dbg
 * 	Adapted for pure Mach kernel.
 * 	[90/10/04            dbg]
 * 
 */

#ifndef	lint
static	char	rcsid[] = "$Header: conf_sd.c,v 2.4 93/01/14 17:56:14 danner Exp $";
#endif

/*
 * conf_sd.c - scsi disk device driver configuration file
 */

/*
 * Revision 1.1  89/07/05  13:17:40  kak
 * Initial revision
 * 
 */

#include <mach/machine/vm_types.h>
#include <sqt/mutex.h>			/* gate_t */
#include <sqt/ioconf.h>			/* IO Configuration Definitions */
#include <sqtsec/sec.h>			/* scsi common data structures */
#include <sqtsec/sd.h>			/* driver local structures */

/*
 * Partition tables. 
 * NOTE: Should be cleanly merged with the standalone.
 *       These partitions that go to the end of the disk are grossly
 *       exaggerated so that varying disk sizes can be used.
 * NOTE: The newfs utility ASSUMES that the 'c' partition starts at the
 *	 beginning of the disk when writing the bootstrap program.
 *	 The bootstrap program is written when a root filesystem is created.
 *	 The newfs utility ASSUMES that the 'a' partition is the root
 *	 filesystem. However, by writing the bootstrap to partition 'c' the
 *	 'a' partition may be moved to the middle of the disk to reduce
 *	 seek latency.
 *	 If the 'c' partition is changed so that it does not include the
 *	 start of the disk, then be sure to use the "-n" option to newfs
 *	 and use /stand/installboot to write the bootstrap program (at least
 *	 1 partition must start at the beginning of the disk).
 * NOTE: Disk partitions that extend to the end of the disk are sized
 *	 as SD_END which allows several different sized drives to work with
 *	 the same partition table.  DYNIX adjusts to the actual size of the
 *	 drive.
 */
struct sd_partition sdpart0[] = {
/* 
start,			length,		*/
0,			15884,		/* minor 0 ('a') */
15884,			33440,		/* minor 1 ('b') */
0,			SD_END,		/* minor 2 ('c') */
15884+33440,		15884,		/* minor 3 ('d') */
15884+33440+15884,	SD_END,		/* minor 4 ('e') */
0,			0,		/* minor 5 ('f') */
15884+33440,		SD_END,		/* minor 6 ('g') */
0,			0,		/* minor 7 ('h') */
};

/*
 * Configure the device's tuning parameters.
 *
 * The number to the far right in the table below will be the 
 * unit number portion of the devices major/minor pair.
 *
 * The structure of the minor number is
 * bits 0-2 are the partition table index; 3-7 is the index into
 * the binary configuration table.
 *
 * buf_sz:	currently used to handle ioctl return information.
 * partab:	partition table entry for each unit.
 * num_iat:	number of iat's which are calloc'd for operation.
 *		This parameter must be large enough to handle all device
 *		programs allocated to allow low and thresh to work properly and
 *		should be set to (num_device_progs*(CLSIZE>=7?7:CLSIZE))
 *		Where the maximum raw transfer size will be constrained to
 *		a minimum of ((num_iat-1)*CLBYTES).
 * low:		once all device programs have been filled out the interrupt
 *		procedure will not queue any more until the queue has drained
 *		off to below low. Minimum value is 2.
 * thresh:	number of device programs to place in the queue maximum on
 *		each strategy or interrupt call to the start procedure. This
 *		allow the queue to be filled up at a controlled rate.
 *
 * NOTE: The partition table entry(below) must contain a valid partition
 *	 table that has the proper number of entries(above).
 *	 UNPREDICTABLE DRIVER ACTION AND RESULTS WILL OCCUR OTHERWISE.
 */
struct sd_bconf sdbconf[] = {				/*
buf_sz,	partab,		num_iat, low,	thresh, bps		   */
{128,	sdpart0,	129,	5,	2,	60*17 },	/*0*/
{128,	sdpart0,	129,	5,	2,	60*17 },	/*1*/
{128,	sdpart0,	129,	5,	2,	60*17 },	/*2*/
{128,	sdpart0,	129,	5,	2,	60*17 },	/*3*/
{128,	sdpart0,	129,	5,	2,	60*17 },	/*4*/
{128,	sdpart0,	129,	5,	2,	60*17 },	/*5*/
{128,	sdpart0,	129,	5,	2,	60*17 },	/*6*/
{128,	sdpart0,	129,	5,	2,	60*17 },	/*7*/
};

int	sdretrys = 4;	/* Number of retrys before allowing a hard error   */
gate_t	sdgate = 58;	/* gate for this device driver */

#ifdef SDDEBUG
/*
 * spin time for async timeouts which should never occur in a
 * production system unless there is bad hardware (target adapter)
 * which never comes back or timeouts.
 */
int	sdspintime = 2000000;
#endif SDDEBUG

/*
 * bit patterns expected in results from INQUIRY command.  To identify
 * units on a target adaptor, byte 3 must be sdinq_targformat; CCS
 * disks will have byte 3 = sdinq_ccsformat.  Byte 0 must be sddevtype
 * on all recognized SCSI disks.
 */

u_char	sddevtype = 0;
u_char	sdinq_targformat = 0x0;
u_char	sdinq_ccsformat = 0x1;

/*
 * DON'T CHANGE ANY THING BELOW THIS LINE OR ALL BETS ARE OFF!
 */
int	sdmaxminor = sizeof(sdbconf)/sizeof(struct sd_bconf);
int	sdsensebuf_sz = 32 * sizeof(char);
