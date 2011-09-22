/* 
 * Mach Operating System
 * Copyright (c) 1991,1989 Carnegie Mellon University
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
 * $Log:	hdreg.h,v $
 * Revision 2.3  93/03/11  14:09:25  danner
 * 	u_long -> u_int
 * 	[93/03/09            danner]
 * 
 * Revision 2.2  93/02/04  08:00:04  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 * Revision 1.2  90/08/16  16:04:24  relyea
 * fix problems which blew GCC out of the water.
 * 
 * Revision 1.1  90/08/07  10:28:01  chuckr
 * Initial revision
 * 
 * Revision 1.1  90/02/23  00:27:23  devrcs
 * 	Updated for osc.5 build.
 * 	[90/02/20  16:05:16  kevins]
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
 *         INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *     This software is supplied under the terms of a license 
 *    agreement or nondisclosure agreement with Intel Corpo-
 *    ration and may not be copied or disclosed except in
 *    accordance with the terms of that agreement.
 *    Copyright 1988  Intel Corporation.
 */

#include <platforms.h>

#define BOOTRECORDSIGNATURE			(0x55aa & 0x00ff)
#define FIXED_DISK_REG				0x3f6
#define MORETHAN8HEADS				0x008
#define EIGHTHEADSORLESS			0x000
#define FIXEDBITS				0x0a0

#define PORT_DATA				0x1f0
#define PORT_ERROR				0x1f1
#define PORT_PRECOMP				0x1f1
#define PORT_NSECTOR				0x1f2
#define PORT_SECTOR				0x1f3
#define PORT_CYLINDERLOWBYTE			0x1f4
#define PORT_CYLINDERHIBYTE			0x1f5
#define PORT_DRIVE_HEADREGISTER			0x1f6
#define PORT_STATUS				0x1f7	
#define PORT_COMMAND				0x1f7

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

#define	ERROR_BBD				0x80
#define ERROR_ECC				0x40

#define	MAX_RETRIES				5
#define	MAX_ALTBUFS				4

#define PATIENCE	3000000		/* how long to wait for controller */
#define PARTITION(z)	(minor(z) & 0x0f)
#define UNIT(z)		(  (minor(z) >> 4)   & 0x01)
#define GOINGUP	1
#define GOINGDOWN 0

#define PDLOCATION	29	
#define GETALTTBL	( ('H' <<8) | 1)
#define FMTBAD		( ('H' <<8) | 2)
#define BAD_BLK		0x80			/* needed for V_VERIFY */

#define NDRIVES 2
#define NLIDS	2				/* number of LIDS allocated to abios disks */
#define SECSIZE 512
#define u_char	unsigned char
#define u_int	unsigned int

#define GDDEBUG_MISC BIT16
#define GDDEBUG_START BIT17
#define GDDEBUG_MINCNT BIT18

#ifndef B_VERIFY
#define B_VERIFY	0x02000000	/* JD_XXX needed by hd */
#endif
#ifndef B_FORMAT
#define B_FORMAT	0x04000000
#endif

#define STRAT_NORMAL	0x01
#define STRAT_URGENT	0xff

#define PAGESIZ		4096
#define BOOT_OFFSET	256	/* 256 blocks or 128k of boot code. */

/* The following macro returns the proper pmap for a virtual address.
 */
#ifdef	MACH_KERNEL
#define get_pmap(bp)	(kernel_pmap)		/* no b_proc to use! */
#else	MACH_KERNEL
#define get_pmap(bp)	( ((bp)->b_proc) ?\
			    (bp)->b_proc->task->map->pmap :\
			    kernel_pmap )
#endif	MACH_KERNEL
/* The following macro returns a page aligned address given the address of a
 * buffer.
 */
#define ADDR(A)		(caddr_t)((((long)(A)) + (I386_PGBYTES - 1)) \
			     & ~ (I386_PGBYTES - 1))
/* The following macro returns the virtual address in a given buf struct.
 */
#define paddr(X)	(paddr_t)(X->b_un.b_addr)
#define b_cylin		b_resid

#ifndef NULL
#define NULL		0
#endif

#ifndef HZ
#define HZ		100
#endif

#define retry_count	retries

typedef struct {
	unsigned short	ncylinders;
	unsigned short	nheads;
	unsigned short	precomp;
	unsigned short	landzone;
	unsigned short	nsecpertrack;
	unsigned short	flags;			/* flags from abios */
	unsigned short	retries;		/* suggested number of retries */
	unsigned char	lid;			/* Logical ID */
	unsigned char	unit;			/* unit on LID */
	unsigned long	nblocks;
	unsigned long	altinfo_loc;
	unsigned long	max_transfer;		/* maximum number of blocks to transfer */
} hdisk_t;

typedef struct {
	caddr_t		phys_addr;
	unsigned long	count;
} saveaddr_t;

/*  hh holds the state of the one and only (stupid board) current 
    block I/O request
*/
    
struct hh {
	int	un_aligned;
	u_char	curdrive;
	u_char	busy;
	u_char	retries;		/* # of times cmd has been tried */
	u_char	num_units;
	u_short	status;
	u_char	restoring;
	u_char	format_request;
#ifndef PS2
	u_char	interleave_tab[SECSIZE];
#endif
	u_int	format_track;
	u_char	absolute_sector;
	u_int 	single_mode;
	u_int 	block_is_bad;
	daddr_t	physblock;
	u_int	substituteblock;
	u_int	substitutetrack;
	paddr_t	rw_addr;
	paddr_t	vert_addr;
	u_int	phys_addr;
	u_int	cylinder;
	u_int	head;
	u_int	sector;
	u_int	blockcount;
	u_int	numblocks;
	u_int	blocktotal;
	u_int	start_of_unix[NDRIVES];
	int	sleep;
};

/* the boot record partition table is documented in IBM AT Tech. Ref p. 9-6 */
struct 	boot_record	{
	u_char	boot_ind;	/* if it == 0x80, this partition is active */
	u_char	head;		/* driver does not look at this field */
	u_char	sector;		/* driver does not look at this field */
	u_char	cylinder;	/* driver does not look at this field */

	u_char	sys_type;	/* Indicates the system type. */
	u_char	end_head;	/* driver does not look at this field */
	u_char	end_sector;	/* driver does not look at this field */
	u_char	end_cylinder;	/* driver does not look at this field */

	u_int	rel_sect;	/* where unix starts if boot_ind == 0x80 */
	u_int	num_sects;	/* driver does not look at this field */
};

