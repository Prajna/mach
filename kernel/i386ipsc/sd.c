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
 * $Log:	sd.c,v $
 * Revision 2.8  93/01/14  17:32:17  danner
 * 	Proper spl typing.
 * 	[92/12/10  17:54:55  af]
 * 
 * Revision 2.7  91/12/10  16:30:05  jsb
 * 	Fixes from Intel
 * 	[91/12/10  15:32:20  jsb]
 * 
 * Revision 2.6  91/11/18  17:10:01  rvb
 * 	Flush p_tag, and redo V_VALID computation like was done on
 * 	all the other i386 disks.
 * 
 * Revision 2.5  91/07/01  08:24:20  jsb
 * 	Replaced first_sdopen_ever with sd_initialized.
 * 	[91/06/29  16:09:42  jsb]
 * 
 * Revision 2.4  91/06/18  20:50:32  jsb
 * 	New copyright from Intel.
 * 	[91/06/18  20:07:42  jsb]
 * 
 * Revision 2.3  91/03/16  14:47:20  rpd
 * 	Updated for new kmem_alloc interface.
 * 	[91/03/03            rpd]
 * 
 * Revision 2.2  90/12/04  14:47:41  jsb
 * 	First checkin.
 * 	[90/12/04  10:58:10  jsb]
 * 
 */ 
/* 
 * sd.c Don Cameron August 1989
 *
 *    This is the top half of the SCSI disk driver for MACH on the iPSC/2.
 *    The code is a rewrite of hd.c for MACH on the AT. Much code is also
 *    stolen from scsidrive.c from NX.
 */
 
#include <sys/types.h>
#include <vm/vm_kern.h>
#ifdef	MACH_KERNEL
#include <device/buf.h>
#include <device/errno.h>
#else	MACH_KERNEL
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/vmparam.h>
#include <sys/uio.h>
#include <mach/vm_param.h>
#endif	MACH_KERNEL
#include <i386ipsc/scsi.h>
#include <i386at/disk.h>
#include <i386ipsc/sd.h>

/*
 * Relative unit numbering allows each IO node to have drives which are
 * virtually numbered from 0 even though the physical drives are not the
 * first in the peripheral module - prp 6/28/90
 */
#define UNITS_RELATIVE	1

#if	UNITS_RELATIVE
#define	RELUNIT(d)	(UNIT(d) + first_unit)
#else	UNITS_RELATIVE
#define	RELUNIT(d)	UNIT(d)
#endif	UNITS_RELATIVE


#define PRIBIO	20
#ifndef	NULL
#define NULL	0
#endif

/* Definitions for MAXTOR XT-8760s disks */
#define NSECPERTRACK	54
#define NCYLINDERS	1632
#define NHEADS		15

/* Hard coded values for partition info; this should be fixed */
#define PART0_START	0
#define PART0_SIZE	64800		/* 31.6 MB */

#define PART1_START	(PART0_SIZE + PART0_START)
#define PART1_SIZE	103680		/* 53 MB */

#define PART2_START	(PART1_SIZE + PART1_START)
#define PART2_SIZE	103680		/* 53 MB */

#define PART3_START	(PART2_SIZE + PART2_START)
#define PART3_SIZE	103680		/* 53 MB */

#define PART4_START	(PART3_SIZE + PART3_START)
#define PART4_SIZE	207360		/* 106 MB */

#define PART5_START	(PART4_SIZE + PART4_START)
#define PART5_SIZE	207360		/* 106 MB */

/* From sys/systm.h */
struct buf *geteblk();

caddr_t scsiphys0;
caddr_t scsiphys1;
caddr_t scsiphys2;
caddr_t scsiphys3;

#define paddr(X)	(paddr_t)(X->b_un.b_addr)

unsigned char	curdrive = 0;	/* drive we are using */

SCSI_INQUIRY	inquiry;	/* Result of inquiry command */
SCSI_SENSE	sense;		/* Result of sense command */

int 	scanning;		/* In drive scan, timeouts OK */
int	write_reserved;		/* Write fifo lock variable */
extern int target_id;		/* From sdintr.c */
extern int data_count;		/* From sdintr.c */
extern int step;		/* From sdintr.c */

struct hh 	hh[NDRIVES];
struct buf *bp1, 
	   *bp2,
	   *bp3;

partition_t	partition_struct[NDRIVES][V_NUMPAR];

typedef struct {
	unsigned short	ncylinders;
	unsigned short	nheads;
	unsigned short	nsecpertrack;
        unsigned short	present;	/* Boolean: TRUE if unit exists */
        unsigned short	opened;		/* Boolean: TRUE if any partition in
					   unit opened */
} sdisk_t;
sdisk_t	sdparams[NDRIVES];	

struct buf	sdbuf[NDRIVES]; 	/* buffer for raw io */

#define	DEBUG	0
#define	dprintf	if (DEBUG) printf

int sd_initialized = 0;
int sdstrategy();
int sdminphys();
int sdrawio();
caddr_t map_phys();

#if	UNITS_RELATIVE
int	first_unit;
#endif	UNITS_RELATIVE

/* forward declarations */
static void sendcmd();

/* XXX dependency on old unixism */
MIN(x, y)
{
	return (x < y ? x : y);
}

sdinit()
{
	unsigned long n;
	unsigned int x;

	if (sd_initialized) {
		return;
	}
	sd_initialized = 1;

#if	UNITS_RELATIVE
	first_unit = -1;
#endif	UNITS_RELATIVE

	printf("EXPERIMENTAL Mach iPSC/2 SCSI Disk Driver v1.0\n");

	/*
	 * Map SCSI physical addresses into kernel virtual memory.
	 */
	scsiphys0 = map_phys(SCSIPHYS0, SCSIPHYS0_LEN);
	scsiphys1 = map_phys(SCSIPHYS1, SCSIPHYS1_LEN);
	scsiphys2 = map_phys(SCSIPHYS2, SCSIPHYS2_LEN);
	scsiphys3 = map_phys(SCSIPHYS3, SCSIPHYS3_LEN);
        
	for (n = 0; n < NDRIVES; n++) {
		hh[n].buflst.b_actf = hh[n].buflst.b_actl = NULL;
		hh[n].buflst.b_active = 0;
		sdparams[n].ncylinders = NCYLINDERS;
		sdparams[n].nheads = NHEADS;
		sdparams[n].nsecpertrack = NSECPERTRACK;
		sdparams[n].opened = 0;
		sdparams[n].present = 0;
	}

        /*
         * Reset SCSI board and driver
         */

	reset_controller();

        /*
         * Reset SCSI bus
         */

        reset_bus();

	/*
	 * Scan for drives. Set scanning flag so interrupt routine can 
	 * special case errors.
	 */

	scanning = 1;

        for (n = 0; n < NDRIVES; n++) {

                /*
                 * Send inquiry command
                 */
		hh[n].cmd_busy = 1;
                bzero(hh[n].cmd, 6);
                hh[n].cmd_len = 6;
                hh[n].cmd[0] = SCSI_INQUIRY_CMD;
                hh[n].cmd[4] = sizeof(SCSI_INQUIRY);
                hh[n].transfer_length = sizeof(SCSI_INQUIRY);
                hh[n].rw_addr = (paddr_t)&inquiry;
		hh[n].buf_io = 0;
                hh[n].direction = 1;

                /* Loop while drive busy in case it is still spinning up */
                do {
                        sendcmd(n);
			sleep(&hh[n].cmd_busy, PRIBIO); 
               } while (hh[n].cmd_st == CMD_ST_BUSY);

                if (hh[n].cmd_st != CMD_ST_GOOD)
                        goto SCAN_NEXT;


                /*
                 * Send request sense command
                 */

		hh[n].cmd_busy = 1;
                bzero(hh[n].cmd, 6);
                hh[n].cmd_len = 6;
                hh[n].cmd[0] = SCSI_SENSE_CMD;
                hh[n].cmd[4] = sizeof(SCSI_SENSE);
                hh[n].transfer_length = sizeof(SCSI_SENSE);
                hh[n].rw_addr = (paddr_t)&sense;
		hh[n].buf_io = 0;
                hh[n].direction = 1;
                sendcmd(n);
		sleep(&hh[n].cmd_busy, PRIBIO); 

                if (hh[n].cmd_st != CMD_ST_GOOD) 
                        goto SCAN_NEXT;

                sdparams[n].present = 1;
#if	UNITS_RELATIVE
		if (first_unit < 0)
			first_unit = n;
#endif	UNITS_RELATIVE

SCAN_NEXT:      ;
	}
	scanning = 0;

#if	UNITS_RELATIVE
	if (first_unit < 0)
		first_unit = 0;
	dprintf("First drive %d\n", first_unit);
#endif	UNITS_RELATIVE
}


sdopen(dev, flags)
int dev;
int flags;
{
	unsigned char unit, part, n;
	int	errcode = 0;
	char 	c;

	if (! sd_initialized) {
		sdinit();
	}

	unit = RELUNIT(dev);
	part = PARTITION(dev);
	dprintf("sdopen: unit = 0x%x, partition = 0x%x\n", unit, part);

	if (part >= V_NUMPAR  || unit >= NDRIVES || !sdparams[unit].present) { 
		errcode = ENXIO;
		dprintf("sdopen: bad unit or partition\n");
	}
	else {	
		if (!sdparams[unit].opened) {
			getvtoc(unit);
			sdparams[unit].opened = 1;
		}
		set_partition_info(unit);

#ifdef	MACH_KERNEL
		if (partition_struct[unit][part].p_flag & V_VALID ) {
			partition_struct[unit][part].p_flag |= V_OPEN;
		}
#else	MACH_KERNEL
		if (u.u_uid == 0) {
			partition_struct[unit][part].p_flag |= V_OPEN;
		}
		else if (partition_struct[unit][part].p_flag & V_VALID ) {
			partition_struct[unit][part].p_flag |= V_OPEN;
		}
#endif	MACH_KERNEL
		else {
			dprintf("sdopen: not valid or not uid 0\n");
			errcode = ENXIO;
		}
	}

	return(errcode);
}


sdclose(dev)
{
	unsigned char unit, part;
	unsigned int old_priority;

	unit = RELUNIT(dev);
	part = PARTITION(dev);
	partition_struct[unit][part].p_flag &= ~V_OPEN;
}


#ifdef	MACH_KERNEL
/*
 *	No need to limit IO size to 4096 bytes.
 */
sdread(dev, ior)
dev_t		dev;
io_req_t	ior;
{
	dprintf("sdread\n");
	/*return(block_io(sdstrategy, sdminphys, ior));*/
	return(block_io(sdstrategy, minphys, ior));
}

sdwrite(dev, ior)
dev_t		dev;
io_req_t	ior;
{
	dprintf("sdwrite\n");
	/*return(block_io(sdstrategy, sdminphys, ior));*/
	return(block_io(sdstrategy, minphys, ior));
}

sdminphys(ior)
	register io_req_t	ior;
{
	if (ior->io_count > PAGE_SIZE)
		ior->io_count = PAGE_SIZE;
}

#else	MACH_KERNEL
sdread(dev,uio)
register short  dev;
struct uio 	*uio;
{
	dprintf("sdread\n");
	return(physio(sdstrategy, &sdbuf[RELUNIT(dev)], dev, B_READ, sdminphys, uio));
}

sdwrite(dev,uio)
dev_t	 	dev;
struct uio	*uio;
{
	dprintf("sdwrite\n");
	return(physio(sdstrategy, &sdbuf[RELUNIT(dev)], dev, B_WRITE, sdminphys, uio));
}

/* Trim buffer length if buffer-size is bigger than page size */
sdminphys(bp)
struct buf	*bp;
{
	if (bp->b_bcount > PAGE_SIZE)
		bp->b_bcount = PAGE_SIZE;
}
#endif	MACH_KERNEL

#ifdef	MACH_KERNEL
#else	MACH_KERNEL
sdioctl(dev, cmd, arg, mode)
dev_t dev;
int cmd;
caddr_t arg;
{
	unsigned char unit, part;
	union io_arg  *arg_kernel; 
	unsigned int i, snum, old_priority;
	struct absio *absio_kernel;
	union vfy_io *vfy_io_kernel;
	int xcount, errcode = 0;

	unit = RELUNIT(dev);
	part = PARTITION(dev);

	switch (cmd) {
	case V_CONFIG:
		dprintf("sdioctl: V_CONFIG\n");
		arg_kernel = (union io_arg *)arg;
		if (arg_kernel->ia_cd.secsiz != SECSIZE) {
			/* changing sector size NOT allowed */
		  	errcode = EINVAL;
			break;
		}
		sdparams[unit].ncylinders=(unsigned short)arg_kernel->ia_cd.ncyl;
		sdparams[unit].nheads = (unsigned short)arg_kernel->ia_cd.nhead;
		sdparams[unit].nsecpertrack = (unsigned short)arg_kernel->ia_cd.nsec;
		reset_controller(unit);
		break;

	case V_REMOUNT:
		dprintf("sdioctl: V_REMOUNT\n");
		getvtoc(unit);	
		break;

	case V_ADDBAD:
		printf("SCSI disk driver: adding bad blocks not supported\n");
		break;

	case V_GETPARMS:
		{
		struct disk_parms *disk_parms = (struct disk_parms *)arg;
		
		dprintf("sdioctl: V_GETPARAMS\n");
		disk_parms->dp_type = DPT_WINI;
		disk_parms->dp_heads = sdparams[unit].nheads;
		disk_parms->dp_cyls = sdparams[unit].ncylinders;
		disk_parms->dp_sectors  = sdparams[unit].nsecpertrack;
		disk_parms->dp_secsiz = SECSIZE;
		disk_parms->dp_ptag = 0;
		disk_parms->dp_pflag = partition_struct[unit][part].p_flag;
		disk_parms->dp_pstartsec = partition_struct[unit][part].p_start;
		disk_parms->dp_pnumsec =partition_struct[unit][part].p_size;
		break;
		}

	case V_FORMAT:
		dprintf("sdioctl: V_FORMAT\n");
		/* All formatting arguments ignored */
		format_command(unit);
		break;

	case V_PDLOC:
		{
		unsigned int *pd_loc;
		
		dprintf("sdioctl: V_PDLOC\n");
		pd_loc = (unsigned int *)arg;
		*pd_loc = (unsigned int) PDLOCATION; 
		break;
		}

	case V_RDABS:
		dprintf("sdioctl: V_RDABS\n");
		/* V_RDABS is relative to head 0, sector 0, cylinder 0 */
		if (u.u_uid != 0) {
			errcode = ENXIO;
			break;
		}
		bp1 = geteblk(SECSIZE);
		absio_kernel = (struct absio *)arg;
		bp1->b_flags = B_READ;

		/* subtract partition offset as it will be added later by sdstart */
		bp1->b_blkno = absio_kernel->abs_sec - 
			partition_struct[unit][0].p_start;

		bp1->b_dev = unit << 4;	/* 4 lsb's = 0 = partit 0 = RDABS */	
		bp1->b_bcount = SECSIZE;
		sdstrategy(bp1);
		biowait(bp1);
		if (bp1->b_flags & B_ERROR) {
			printf("SCSI disk driver: read failure on ioctl\n");
			errcode = ENXIO;
			brelse(bp1);
			break;
		}
		if ( copyout( (caddr_t)paddr(bp1), absio_kernel->abs_buf, 
				SECSIZE) !=0 ) {
			errcode = ENXIO;
		}
		brelse(bp1);
		break;

	case V_WRABS:

		dprintf("sdioctl: V_WRABS\n");
		/* V_WRABS is relative to head 0, sector 0, cylinder 0 */
		if (u.u_uid != 0) {
			errcode = ENXIO;
			break;
		}
		bp1 = geteblk(SECSIZE);
		absio_kernel = (struct absio *)arg;
		if ( copyin( absio_kernel->abs_buf, (caddr_t)paddr(bp1), 
				SECSIZE) !=0 ) {
			/* u.u_error =ENXIO; */
			errcode = ENXIO;
			brelse(bp1);
			break;
		}
		bp1->b_flags = B_WRITE;

		/* subtract partition offset as it will be added later by sdstart */
		bp1->b_blkno = absio_kernel->abs_sec - 
			partition_struct[unit][0].p_start;

		bp1->b_dev = unit << 4;	/* 4 lsb's = 0 = partit 0 = RDABS */	
		bp1->b_bcount = SECSIZE;
		sdstrategy(bp1);
		biowait(bp1);
		if (bp1->b_flags & B_ERROR) {
			printf("SCSI disk driver: write failure on ioctl\n");
			/* u.u_error =ENXIO; */
			errcode = ENXIO;
		}
		brelse(bp1);
		break;

	case V_VERIFY:

		dprintf("sdioctl: V_VERIFY\n");
		if (u.u_uid != 0) {
			errcode = ENXIO;
			break;
		}

		bp1 = geteblk(PAGE_SIZE);
		vfy_io_kernel = (union vfy_io *)arg;
		bp1->b_flags = B_READ;
		bp1->b_blkno = vfy_io_kernel->vfy_in.abs_sec;
		bp1->b_dev = unit << 4;	/* 4 lsb's = 0 = partit 0 = RDABS */	
		xcount = vfy_io_kernel->vfy_in.num_sec;
		vfy_io_kernel->vfy_out.err_code = 0;
		snum = PAGE_SIZE >> 9;
		while (xcount > 0) {
			i = (xcount > snum) ? snum : xcount;
			bp1->b_bcount = i << 9;
			sdstrategy(bp1);
			biowait(bp1);
			if (bp1->b_flags & B_ERROR) {
				vfy_io_kernel->vfy_out.err_code = BAD_BLK;
				break;
			}
			xcount -= i;
			bp1->b_blkno += i;
			bp1->b_flags &= ~B_DONE;
		}
		brelse(bp1);
		break;

	case V_XFORMAT:
		printf("V_XFORMAT not supported\n");
		break;

	case FMTBAD:
		printf("FMTBAD not supported\n");
		break;
	
	case GETALTTBL:
		printf("GETALTTBL not supported\n");
		break;

	default:
		printf("sdioctl(): do not recognize ioctl of 0x%x \n", cmd);
		/* u.u_error = EINVAL; */
		errcode = EINVAL;
	}
}
#endif	MACH_KERNEL

sdstrategy(bp)
struct	buf	*bp;
{
	struct	buf	*ptr;
	partition_t	*partition_p;
	unsigned char unit;
	unsigned int track, direction;
	spl_t	old_priority;

	unit = RELUNIT(bp->b_dev);
	if (bp->b_bcount == 0) {
		biodone(bp);
		return;
	}
	partition_p= &(partition_struct[unit][PARTITION(bp->b_dev)]);

	if ( !(partition_p->p_flag & V_VALID))
	{
		bp->b_flags = B_ERROR;
		bp->b_error = ENXIO;
		biodone(bp);
		return;
	}

	/* if request is off the end or trying to write last block on out */

	if ( (bp->b_blkno >  partition_p->p_size) ||
	     (bp->b_blkno == partition_p->p_size & !(bp->b_flags & B_READ))) {
		bp->b_flags = B_ERROR;
		bp->b_error = ENXIO;
		biodone(bp);
		return;
	}

	if (bp->b_blkno == partition_p->p_size) {
	/* indicate (read) EOF by setting b_resid to b_bcount on last block */ 
		bp->b_resid = bp->b_bcount;
		biodone(bp);
		return;
	}

	/* since BSD does NOT have bp->b_cylin, use bp->b_resid */
	bp->b_resid = (partition_p->p_start + bp->b_blkno) /
		      (sdparams[curdrive].nsecpertrack * 
		       sdparams[curdrive].nheads); 

	old_priority = spl5();

	disksort(&(hh[unit].buflst), bp);

	if (!hh[unit].cmd_busy) {
		sdstart();
	}
	splx(old_priority);
}


/* sdstart is called at spl5 */
sdstart()
{
	int drivecount;
	partition_t	*partition_p;
	struct hh	*hh_p;
	struct buf	*bp;
	int blocks;

	for (drivecount = 0; drivecount < NDRIVES; drivecount++) {
		if (curdrive < (NDRIVES - 1))
			curdrive++;
		else
			curdrive = 0;
		if ((bp = hh[curdrive].buflst.b_actf) != NULL)
			break;
	}
	if (drivecount == NDRIVES)
		return;

	hh_p = &hh[curdrive];

	partition_p = &partition_struct[RELUNIT(bp->b_dev)]
					[PARTITION(bp->b_dev)];
	
	/* see V_RDABS and V_WRABS in sdioctl() */
 	hh_p->physblock = partition_p->p_start + bp->b_blkno;
	dprintf("sdstart: blkno = 0x%x pstart = 0x%x psize = 0x%x\n",
		bp->b_blkno, partition_p->p_start, partition_p->p_size);
	dprintf("sdstart: unit = 0x%x partition = 0x%x\n",
		RELUNIT(bp->b_dev),PARTITION(bp->b_dev));

	if (bp->b_bcount > PAGE_SIZE) {
		dprintf("sdstart: big transfer\n");
	}
	hh_p->transfer_length = MIN(PAGE_SIZE, bp->b_bcount);
	blocks = (hh_p->transfer_length + (SECSIZE-1)) >> 9;
		
	if ((bp->b_blkno + blocks) > partition_p->p_size) {
		printf("sdstart(): hit the end of the partition\n");
		bp->b_bcount = (partition_p->p_size - bp->b_blkno) * SECSIZE;
		hh_p->transfer_length = MIN(hh_p->transfer_length, bp->b_bcount);
		blocks = (bp->b_bcount + (SECSIZE-1)) >> 9;
	}

	/* b_resid is set to the number of bytes to transfer */
	bp->b_resid = bp->b_bcount;

	hh_p->cmd_busy = 1;
	hh_p->rw_addr = (paddr_t) bp->b_un.b_addr;
	hh_p->buf_io = 1;
	hh_p->retry_count = 0;
	bzero(hh_p->cmd, 10);
	hh_p->cmd_len = 10;
	if (bp->b_flags & B_READ) {
		hh_p->cmd[0] = SCSI_READ_CMD;
		hh_p->direction = 1;
	}
	else {
		hh_p->cmd[0] = SCSI_WRITE_CMD;
		hh_p->direction = 0;
	}
	hh_p->cmd[2] = hh_p->physblock >> 24;
	hh_p->cmd[3] = hh_p->physblock >> 16;
	hh_p->cmd[4] = hh_p->physblock >> 8;
	hh_p->cmd[5] = hh_p->physblock;
	hh_p->cmd[8] = blocks;
	dprintf("sdstart: blocks=0x%x transfer_length=0x%x physblock=0x%x direction=0x%x\n", blocks, hh_p->transfer_length, hh_p->physblock, hh_p->direction);
	sendcmd(curdrive);
}

/* 
 * sdrestart is called at interrupt time to continue the transfer of
 * a buffer that was greater than 4k. This occurs during the initial 
 * mount of root when ufs_mountroot tries to read an 8k block 
 */
sdrestart()
{
	struct hh	*hh_p;
	struct buf	*bp;
	int blocks;

	dprintf("sdrestart\n");
	hh_p = &hh[curdrive];
	bp = hh_p->buflst.b_actf;
	hh_p->transfer_length = MIN(PAGE_SIZE, bp->b_resid);
	blocks = (hh_p->transfer_length + (SECSIZE-1)) >> 9;
 	hh_p->physblock += PAGE_SIZE / SECSIZE;
	hh_p->retry_count = 0;
	hh_p->cmd[2] = hh_p->physblock >> 24;
	hh_p->cmd[3] = hh_p->physblock >> 16;
	hh_p->cmd[4] = hh_p->physblock >> 8;
	hh_p->cmd[5] = hh_p->physblock;
	hh_p->cmd[8] = blocks;
	sendcmd(curdrive);
}


getvtoc(unit)
unsigned char	unit;
{
	unsigned char *c_p;
	unsigned int dev;
	unsigned int n, m;
	char *pt1, *pt2;
	struct boot_record *boot_record_p;
	struct pdinfo *pd_p;
	struct vtoc *vtoc_p;
	
	/* make unit into a device of the form unit/partion 0
	   first four bits of device number are partition */
	dev = unit << 4;
#if 0 /* ignore vtoc and pdinfo for now */
	bp1 = geteblk(SECSIZE);		/* for pdinfo */
	bp2 = geteblk(SECSIZE);		/* for vtoc */
#endif 0

	/* make partition 0 the whole disk in case of failure */
	partition_struct[unit][0].p_flag = V_OPEN|V_VALID;
	partition_struct[unit][0].p_start = 0; 
	partition_struct[unit][0].p_size = sdparams[unit].ncylinders *
		   sdparams[unit].nheads * sdparams[unit].nsecpertrack;

#if 0 /* ignore vtoc and pdinfo for now */
	/* get pdinfo */
	bp1->b_flags = B_READ;
	bp1->b_blkno = PDLOCATION;
	bp1->b_dev = dev;
	bp1->b_bcount = SECSIZE;
	sdstrategy(bp1);
	biowait(bp1);
	if (bp1->b_flags & B_ERROR) {
		printf("SCSI disk driver: can not read pdinfo on drive %d\n", unit);
		return;
	}
	pd_p = (struct pdinfo *)bp1->b_un.b_addr;

	if (pd_p->sanity != VALID_PD) {
		printf("SCSI disk driver: pdinfo invalid on drive %d\n",unit);
		return;
	}
	if (( pd_p->cyls != sdparams[unit].ncylinders)		||
	    ( pd_p->tracks != sdparams[unit].nheads)		||
	    ( pd_p->sectors != sdparams[unit].nsecpertrack))
		printf("SCSI disk driver: pdinfo/setup mismatch on drive %d\n", unit);

	if (pd_p->bytes != SECSIZE)
		printf("SCSI disk driver: assuming sector size of %d on drive %d\n",SECSIZE, unit );

	/* pd info from disk must be more accurate than hard coded values,
	   override sdparams and reset controller()
	*/			
	sdparams[unit].ncylinders = pd_p->cyls;
	sdparams[unit].nheads = pd_p->tracks;
	sdparams[unit].nsecpertrack = pd_p->sectors;
	reset_controller(unit);

		
	/* get vtoc */
	bp2->b_flags = B_READ;
	bp2->b_blkno = pd_p->vtoc_ptr/SECSIZE;
	bp2->b_dev = dev;
	bp2->b_bcount = SECSIZE;
	sdstrategy(bp2);
	biowait(bp2);
	if (bp2->b_flags & B_ERROR) {
		printf("SCSI disk driver: can not read vtoc on drive %d\n", unit);
		return;
	}
	vtoc_p = (struct vtoc *)( bp2->b_un.b_addr + 
				(pd_p->vtoc_ptr % SECSIZE) );

	if (vtoc_p->v_sanity != VTOC_SANE) {
		printf("SCSI disk driver: vtoc corrupted on drive %d\n", unit);
		return;
	}

	/* copy info on all valid partition, zero the others */

	for (n = 0; n < vtoc_p->v_nparts; n++) {
		/* this is a STRUCTURE copy */
		partition_struct[unit][n] = vtoc_p->v_part[n];
		if (vtoc_p->part[n].p_start >= 0 && vtoc_p->part[n].p_size >= 0)
			partition_struct[unit][n].p_flag = V_VALID;
		else
			partition_struct[unit][n].p_flag = 0;
	}
	for ( ; n < V_NUMPAR; n++) {
		partition_struct[unit][n].p_flag = 0;
		partition_struct[unit][n].p_size = 0;
	}	
#endif 0
	/* leave partition 0 "open" for raw I/O */
	partition_struct[unit][0].p_flag |= V_OPEN;


#if 0
	if (bp1 != NULL)
		brelse(bp1);
	if (bp2 != NULL)
		brelse(bp2);
#endif 0
}

format_command(unit)
	int unit;	/* device to format */
{
	printf("format_command()	--not implemented\n");
}


start_rw(n, read)
int n;		/* device index */
int read;	/* Boolean: TRUE indicates read */
{
	sendcmd(n);
}


dynamic_badblock()
{
	printf("dynamic_badblock()	--not implemented\n");
}

sdsize()
{
	printf("sdsize()	-- not implemented\n");
}

sddump()
{
	printf("sddump()	-- not implemented\n");
}

	
static void sendcmd(drive)
	int	drive;
{
	int	i;
	spl_t	old_priority;
	struct hh	*hh_p = &hh[drive];

	if (hh_p->cmd[0] == SCSI_WRITE_CMD) {
		write_reserved = 1;
	}

	old_priority = splhi();
	while(target_id != -1)
		sleep(&target_id, PRIBIO);
	target_id = drive;
	splx(old_priority);

	data_count = 0;
	step = CMD_STEP;
	SCSI_ESP_FIFO = MSG_IDENTIFY;
	shortdelay();
	SCSI_ID = target_id;
	for (i = 0; i < hh_p->cmd_len; i++) {
		SCSI_ESP_FIFO = hh_p->cmd[i];
	}
	SCSI_COMMAND = ESP_SELATN;
}

/*
 * map_phys: map physical SCSI addresses into kernel vm and return the
 * (virtual) address.
 */
caddr_t
map_phys(physaddr, length)
caddr_t physaddr;			/* address to map */
int length;				/* num bytes to map */
{
	vm_offset_t vmaddr;
	vm_offset_t pmap_addr;
	vm_offset_t pmap_map_bd();

	if (physaddr != (caddr_t)trunc_page(physaddr))
		panic("map_phys: Tryed to map address not on page boundary");
	if (kmem_alloc_pageable(kernel_map, &vmaddr, round_page(length))
						!= KERN_SUCCESS)
		panic("map_phys: Can't allocate VM");
	pmap_addr = pmap_map_bd(vmaddr, (vm_offset_t)physaddr, 
			(vm_offset_t)physaddr+length, 
			VM_PROT_READ | VM_PROT_WRITE);
	return((caddr_t) vmaddr);
}

set_partition_info(unit)
unsigned int	unit;
{
	dprintf("set_partition_info\n");

	partition_struct[unit][0].p_start = PART0_START; 
	partition_struct[unit][0].p_size = PART0_SIZE;
	partition_struct[unit][0].p_flag = V_VALID;

	partition_struct[unit][1].p_start = PART1_START; 
	partition_struct[unit][1].p_size = PART1_SIZE;
	partition_struct[unit][1].p_flag = V_VALID;

	partition_struct[unit][2].p_start = PART2_START; 
	partition_struct[unit][2].p_size = PART2_SIZE;
	partition_struct[unit][2].p_flag = V_VALID;

	partition_struct[unit][3].p_start = PART3_START; 
	partition_struct[unit][3].p_size = PART3_SIZE;
	partition_struct[unit][3].p_flag = V_VALID;

	partition_struct[unit][4].p_start = PART4_START; 
	partition_struct[unit][4].p_size = PART4_SIZE;
	partition_struct[unit][4].p_flag = V_VALID;

	partition_struct[unit][5].p_start = PART5_START; 
	partition_struct[unit][5].p_size = PART5_SIZE;
	partition_struct[unit][5].p_flag = V_VALID;
}

