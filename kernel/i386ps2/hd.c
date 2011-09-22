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

static char sccsid[] = "@(#)66  1.1  mk/src/latest/kernel/i386ps2/hd.c, root 4/4/91 10:22:11";
/* 
 * HISTORY
 * $Log:	hd.c,v $
 * Revision 2.5  93/08/10  16:00:26  mrt
 * 	changed partition -> localpartition for type declaration so not
 * 	to conflict with partition struct introduced in disklabel.h
 * 	[93/08/09            rvb]
 * 
 * Revision 2.4  93/05/28  21:22:25  rvb
 * 	In the getstat and setstat routines, return failure
 * 	if the flavor isn't one we know about.
 * 	[93/05/28  15:07:28  chs]
 * 
 * Revision 2.3  93/03/11  14:09:19  danner
 * 	u_long -> u_int
 * 	[93/03/09            danner]
 * 
 * Revision 2.2  93/02/04  07:59:55  danner
 * 	Using i386at/disk.h rather than i386ps2/disk.h == They are
 * 	identical. 
 * 	[93/01/25            rvb]
 * 
 * 	Changed to new partition scheme.
 * 	Pick up recent bugfixes from i386at/hd.c.
 * 	[93/01/18            zon]
 * 
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 * Revision 1.17  90/11/02  11:15:42  webb
 * 1. move code out of hdinit into hdprobe and hdslave routines 
 * 2. make other appropriate changes to work fully with autoconfig.
 * 
 * Revision 1.16  90/11/01  10:44:52  webb
 * changes for new version of config 
 * 
 * Revision 1.15  90/10/23  15:36:13  webb
 * remove dead code and make changes so that multiple LIDS (and multiple 
 * interrupt vectors) are supported.
 * 
 * Revision 1.7  90/09/12  11:10:09  webb
 * 1. clean up code and debugging printf's
 * 2. improve error message output's 
 * 3. delete code not needed for this driver 
 * 4. put in error retrying
 * 
 * Revision 1.6  90/09/05  10:00:47  webb
 * 1. add error decoding 
 * 2. use write-verify instead of write for now 
 * 3. allow driver to return an error instead of just panicing.
 * 
 * Revision 1.4  90/08/16  15:51:32  relyea
 * rci.rsc nps2 hdreg.h
 * fix problems which blew GCC out of the water.
 * 
 * Revision 1.3  90/08/08  11:20:18  webb
 * changes to make gcc happy.
 * 
 * Revision 1.1  90/02/23  00:27:20  devrcs
 * 	Latest version for osc.5
 * 	[90/02/20  11:57:26  kevins]
 * 
 * Revision 1.6  89/10/24  20:20:52  lance
 * Trying to fix the random hang bug.
 * 
 * Revision 1.5  89/09/26  11:56:47  lance
 * X109 checkin
 * 
 * Revision 1.8  89/09/25  12:26:48  rvb
 * 	i386at/alttbl.h, i386/vtoc.h, i386/iobuf.h, i386/elog.h -> disk.h
 * 	[89/09/23            rvb]
 * 
 * Revision 1.7  89/09/20  17:28:21  rvb
 * 	Revision 1.3  89/08/01  16:54:04  kupfer
 * 	Debugging hacks for when driver gets wedged.
 * 
 * Revision 1.2  89/07/12  11:13:41  lance
 * New paths to disk (both through buffer cache & physio).
 * 
 * Revision 1.4  89/03/09  20:05:37  rpd
 * 	More cleanup.
 * 
 * Revision 1.3  89/02/26  12:36:37  gm0w
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

/*
 *  AT Hard Disk Driver
 *  Copyright Ing. C. Olivetti & S.p.A. 1989
 *  All rights reserved.
 *
 */

#include <platforms.h>
#include <hd.h>

#if NHD > 0

#include <sys/types.h>
#define PRIBIO 20
#include <device/io_req.h>
#include <device/buf.h>
#include <device/errno.h>
#include <device/device_types.h>
#include <device/disk_status.h>

typedef long paddr_t;		/* a physical address */

#include <sys/ioctl.h>
#include <i386/ipl.h>
#include <i386ps2/bus.h>
#include <i386ps2/abios.h>
#include <i386ps2/debugf.h>
#include <i386ps2/gdabios.h>
#include <i386ps2/hdreg.h>
#include <i386at/disk.h>

#define LABEL_DEBUG(x,y) if (label_flag&x) y

extern	struct buf *geteblk();

#ifndef NHDC
#define NHDC 1			/* for systems without a full config */
#endif

#define ABIOS_WRITE_VERIFY	ABIOS_ADDITIONAL_XFER
#define B_RESET	0x10000000	/* resetting the disk */

struct hh 		hh;
struct buf 		hd_buF[NHD]; 	/* buffer for raw io */
struct buf 		hdunit[NHD];
struct alt_info alt_info[NHD];

int hdgotvtoc[NHD];

#define HD_MAX_XFER (I386_PGBYTES * 8)
static char unaligned_buf[NHD][HD_MAX_XFER]; /* buffer for unaligned xfers */
static caddr_t			u_buf[NHD];		    /* pointer to the buffer */
static struct buf 		*bp1, *bp2;
static struct Gd_request 	rb[NHD];
static struct Gd_request 	ctlr_rb[NHDC];
static struct Gd_request 	save_rb[NHD];
struct disklabel		label[NHD];
int				labeloffset[NHD];
int				labelsector[NHD];
static hdisk_t			hdparams[NHD];	
static saveaddr_t		save_addr[NHD][34];
static struct Logical_id_params params = {0};

int hd_write_verify = 0; /* will we use write-verify function? */

static void start_rw(long read);
static int hdintr(int vec);
static int hdabioswait(int abios_function, int gd_flag, int unit);
int hdstrategy(struct buf *);
static void hdinit(void);
static hd_error_decode(int code, char *msg);

static int hdprobe();
static int hdslave();
static int hdattach();

int (*hdcintrs[])(int) = {hdintr, 0};

static struct i386_dev *hdinfo[NHD];
static struct i386_ctlr *hdcinfo[NHDC];

#define WHOLE_DISK(unit) ((unit << 4) + PART_DISK)

struct  i386_driver      hdcdriver = {
	/* probe slave    attach   dname  dinfo   mname  minfo */
        hdprobe, hdslave, hdattach, "hd", hdinfo, "hdc", hdcinfo};

#define DEV_CTRL_FLAGS_FMT "\20\3CHANGE-SIGNAL\4WRITE-MANY\5CACHING\6READABLE\7LOCKABLE\10SEQUENTIAL\11EJECTABLE\12CONCURRENT\13ST506\14FORMAT-TRACK\15FORMAT-UNIT\17SCSI\20SCB"
#define LOGICAL_ID_FLAGS_FMT "\20\1DATA-POINTER1-LOGICAL\2DATA-POINTER2-PHYSICAL\4OVERLAPPED-IO"

/*
 * probe for the specified controller. 
 * we look for the n'th (actually stored in unit) hard disk LID.
 * if not found we return that the device doesn't exist.
 * if found we get the information about that controller. 
 * we use "rb" as a working area, and copy the results back
 * into ctlr_rb after we are finished. This is mainly so that we 
 * don't have to change abioswait to accept a request block parameter.
 */
static int hdprobe(addr, ctlr)
int addr;
struct i386_ctlr *ctlr;
{
	int lid_num = 2; /* starting lid number */
	int i;
	int unit = ctlr->ctlr_ctlr;
	static intr_num = -1; /* avoid duplicating interrupt handlers */
	struct Gd_request *rbp = &rb[unit];

	hdinit();		/* do other initializations */

	/*
	 * scan thru the available abios devices looking for this 
	 * controller (unit) number.
	 */
	for (i=0; (lid_num = abios_next_LID(HD_ID, lid_num)) && i < unit; ++i);

	if (lid_num == 0)
		return 0; /* no such LID */

	/*
	 * at this point we have determined that the requested controller
	 * (e.g. LID) exists.
	 */

	rbp->r_current_req_blck_len = sizeof(struct Gd_request);
	rbp->request_header.Request_Block_Flags = 0;
	rbp->request_header.ELA_Offset = 0;

	GD_SET_RESERVED_ABIOS_LOGICAL_PARAMETER(*rbp);
	rbp->r_logical_id = lid_num;
	rbp->r_unit = 0;

	if (hdabioswait(ABIOS_LOGICAL_PARAMETER, params.Logical_id_flags, unit))
		return ENXIO;

	params = rbp->un.logical_id_params;

	/*
	 * Now we need to update certain field based on what ABIOS
	 * just told us...
	 */

	if (rbp->r_request_block_length > rbp->r_current_req_blck_len) {
		printf("hdc%d: Required Request block length is too large.\n",unit);
		return(0);
	}

	rbp->r_current_req_blck_len = rbp->r_request_block_length;

	if (!(params.Logical_id_flags & 0x2)) {
		printf ("hdc%d: can not support virtual addresses.\n", unit);
		return(0);
	}

	/* if we haven't already gotten this IRQ then obtain it */
	if (intr_num != rbp->r_hardware_intr) {
		ctlr->ctlr_pic = intr_num = rbp->r_hardware_intr;
		ctlr->ctlr_spl = SPL5;
		take_ctlr_irq(ctlr);
		printf("hdc%d: port = %x, spl = %d, pic = %d.\n",
			ctlr->ctlr_ctlr, ctlr->ctlr_addr, ctlr->ctlr_spl, ctlr->ctlr_pic);
	}

	ctlr_rb[unit] = *rbp;		        /* save controller's rb */

	return 1;

}

/*
 * see if this unit exists and if it does get the information about it.
 */
static int hdslave(iod)
struct i386_dev *iod;
{
	int unit = iod->dev_unit;
	int slave = iod->dev_slave;

	if (slave >= ctlr_rb[iod->dev_ctlr].r_number_units)
		return(0);		/* not that many units */

	rb[unit] = ctlr_rb[iod->dev_ctlr];	/* copy from controller */
	rb[unit].r_unit = slave;
	hdunit[unit].b_active = 0;
	hdunit[unit].b_actf = 0;
	hdunit[unit].b_actl = 0;
	hdparams[unit].landzone = 0;
	hdparams[unit].precomp = 0;
	hdparams[unit].lid  = rb[unit].r_logical_id;
	hdparams[unit].unit  = slave;
	GD_SET_RESERVED_ABIOS_READ_PARAMETER(rb[unit]);

	if (hdabioswait(ABIOS_READ_PARAMETER, params.Logical_id_flags, unit))
		return(0);

	hdparams[unit].ncylinders = gd_number_cylinders(rb[unit]);
	hdparams[unit].nheads = gd_number_heads(rb[unit]);
	hdparams[unit].nblocks = gd_max_block_number(rb[unit]);
	hdparams[unit].nsecpertrack = gd_sector_per_track(rb[unit]);
	hdparams[unit].flags = gd_dev_ctrl_flag(rb[unit]);
	hdparams[unit].retries = gd_max_retries(rb[unit]);
	hdparams[unit].max_transfer = gd_max_transfer(rb[unit]);
	/*
	 * copy initial parameters into label
	 */
	fudge_bsd_label(&label[unit], DTYPE_ESDI,
			hdparams[unit].ncylinders,
			hdparams[unit].nheads,
			hdparams[unit].nsecpertrack,
			2);

	printf("hd%d: ncylinders=%d nheads=%d nsecpertrack=%d nblocks=%d\n",
	        unit,
		hdparams[unit].ncylinders,
	        hdparams[unit].nheads, 
		hdparams[unit].nsecpertrack,
	        hdparams[unit].nblocks);

	hh.num_units++;	/* accumulate total number of units */
	params.Device_id = rb[unit].r_logical_id;;

	return(1);
}

static hdattach(iod)
struct i386_dev *iod;
{
}

/*
 * initialize the disk.
 */
static void hdinit(void)
{
	int i;
	static int hdinit_done = 0;

	if (hdinit_done++)
		return;

	for (i = 0; i < NHD; i++)
		u_buf[i] = &unaligned_buf[i][0];
	hh.un_aligned = 0;

}

static void hd_delay(register int ticks)
{
	assert_wait(0, FALSE);
	thread_set_timeout(ticks);
	thread_block((continuation_t) 0);
}

static int hdabioswait(int abios_function, int gd_flag, int unit)
{
	int i, j, n;

	struct Gd_request *rbp = &rb[unit];

	rbp->r_function = abios_function;
	rbp->r_return_code = ABIOS_UNDEFINED;

	abios_common_start(rbp, gd_flag);

	/* Wait for ABIOS to tell us that it is done. */
	while (rbp->r_return_code == ABIOS_UNDEFINED)
		hd_delay(10);	

	if (rbp->r_return_code & 0x8000) {
		printf("[hdabioswait] rb[unit].r_return_code [%x] ", rbp->r_return_code);
		hd_error_decode(rbp->r_return_code, "\n");
		return ENXIO;
	}
	
	hh.status = rbp->r_return_code;

	while (rbp->r_return_code != ABIOS_DONE) {

		hh.status = rbp->r_return_code; 

		switch (hh.status) {
		
			case ABIOS_STAGE_ON_INT:
				hd_delay(1);
				break;

			case ABIOS_STAGE_ON_TIME:
				hd_delay(gd_wait_time(*rbp));
				abios_common_interrupt(rbp, params.Logical_id_flags);
				break;

			case (ABIOS_NOT_MY_INT | ABIOS_STAGE_ON_INT):
				break;

			case ABIOS_DONE:
				return;

			default:
				printf("hdabioswait: return code [%x]...\n", hh.status);
				hd_error_decode(rbp->r_return_code,"\n");
				return ENXIO;
		}
	}
	
	return 0;

}
getvtoc(unit)
{
	hdreadlabel(unit, 0);
}

static hdreset(int unit)
{
	struct buf *bp = (struct buf *)geteblk(SECSIZE);

#if	0
	if (label[unit].d_partitions[PART_DISK].p_tag == 0) {
		label[unit].d_partitions[PART_DISK].p_tag = V_BACKUP;
		label[unit].d_partitions[PART_DISK].p_flag = V_OPEN | V_UNMNT | V_VALID;
		label[unit].d_partitions[PART_DISK].p_offset = 0; 
		label[unit].d_partitions[PART_DISK].p_size = 10;
	}
#endif	/* 0 */

	bp->b_flags = B_RESET|B_MD1;
	bp->b_blkno = 0;
	bp->b_dev = WHOLE_DISK(unit);
	bp->b_bcount = 0;
	hdstrategy(bp);
	biowait(bp);

	getvtoc(unit);

}

io_return_t hdopen(register dev_t dev,
                   register dev_mode_t flags,
                   io_req_t not_used)
{
	int unit = UNIT(dev);
	int part = PARTITION(dev);
	int n;
	static int hdopen_done = 0;

	if (part >= MAXPARTITIONS || unit >= NHD)
		return ENXIO;

	if (!hdopen_done++) {
		hdinit();
		for (n = 0; n < hh.num_units; n++)
			hdreset(n);
	}

	if (!label[unit].d_partitions[part].p_offset &&
	    !label[unit].d_partitions[part].p_size)
		return ENXIO;

	return D_SUCCESS;

}


io_return_t hdclose(register dev_t dev)
{

	return D_SUCCESS;

}

int hdread(register dev_t dev, register struct uio *uio)
{
	register struct buf *bp = &hd_buF[UNIT(dev)];

	return physio(hdstrategy, bp, dev, B_READ, minphys, uio);

}

int hdwrite(register dev_t dev, register struct uio *uio)
{
	register struct buf *bp = &hd_buF[UNIT(dev)];

	return physio(hdstrategy, bp, dev, B_WRITE, minphys, uio);

}

static int abs_sec   = -1;
static int abs_count = -1;

io_return_t hdgetstat(dev, flavor, data, count)
	dev_t		dev;
	int		flavor;
	int *		data;		/* pointer to OUT array */
	unsigned int	*count;		/* OUT */
{
	int unit = UNIT(dev);
	struct disklabel *lp = &label[unit];
	int part = PARTITION(dev);
	io_return_t errcode = D_SUCCESS;
	struct disk_parms *dp;
	unsigned int snum;
	int xcount, i;
	int code;


	switch (flavor) {

	case DEV_GET_SIZE:
		data[DEV_GET_SIZE_DEVICE_SIZE] =
			lp->d_partitions[part].p_size * lp->d_secsize;
		data[DEV_GET_SIZE_RECORD_SIZE] = lp->d_secsize;
		*count = DEV_GET_SIZE_COUNT;
		break;

	/* BsdLabel flavors */
	case DIOCGDINFO:
	case DIOCGDINFO - (0x10<<16):
		dkgetlabel(lp, flavor, data, count);
		break;

	/* Extra flavors */
	case V_GETPARMS:
		if (*count < sizeof (struct disk_parms)/sizeof(int)) {
			printf("hd%d%c: V_GETPARMS bad size %x",
			       unit,
			       'a' + part,
			       count);
			return D_INVALID_OPERATION;
		}
		dp = (struct disk_parms *) data;
		dp->dp_type = DPT_WINI;
		dp->dp_heads = lp->d_ntracks;
		dp->dp_cyls = lp->d_ncylinders;
		dp->dp_sectors  = lp->d_nsectors;
  		dp->dp_dosheads = hdparams[unit].nheads;
		dp->dp_doscyls = hdparams[unit].ncylinders;
		dp->dp_dossectors = hdparams[unit].nsecpertrack;
		dp->dp_secsiz = SECSIZE;
		dp->dp_ptag = 0 /* label[unit].d_partitions[part].p_tag */;
		dp->dp_pflag = 0 /* label[unit].d_partitions[part].p_flag */;
		dp->dp_pstartsec = label[unit].d_partitions[part].p_offset;
		dp->dp_pnumsec =label[unit].d_partitions[part].p_size;
		*count = sizeof (struct disk_parms)/sizeof(int);
		break;

	case V_PDLOC:
		*data = label[unit].d_partitions[PART_DISK].p_offset + PDLOCATION;
		break;

	case V_RDABS:
		if (*count < SECSIZE/sizeof (int)) {
			printf("hd%d: V_RDABS bad size %x",
			       unit,
			       count);
			return D_INVALID_OPERATION;
		}
		bp1 = geteblk(SECSIZE);
		bp1->b_flags = B_READ | B_MD1;
		bp1->b_blkno = abs_sec;
		bp1->b_dev = WHOLE_DISK(unit);
		bp1->b_bcount = SECSIZE;
		hdstrategy(bp1);
		biowait(bp1);
		if (bp1->b_flags & B_ERROR) {
			printf("hd%d: V_RDABS failed\n", unit);
			errcode = ENXIO;
			brelse(bp1);
			break;
		}
		bcopy((caddr_t)paddr(bp1), (caddr_t) data, SECSIZE);
		brelse(bp1);
		*count = SECSIZE/sizeof(int);
		break;

	case V_VERIFY:
		bp1 = geteblk(PAGESIZ);
		bp1->b_flags = B_READ | B_MD1;
		bp1->b_blkno = abs_sec;
		bp1->b_dev = WHOLE_DISK(unit);
		xcount = abs_count;
		snum = PAGESIZ >> 9;
		code = 0;
		while (xcount > 0) {
			i = (xcount > snum) ? snum : xcount;
			bp1->b_bcount = i << 9;
			hdstrategy(bp1);
			biowait(bp1);
			if (bp1->b_flags & B_ERROR) {
				code = BAD_BLK;
				break;
			}
			xcount -= i;
			bp1->b_blkno += i;
			bp1->b_flags &= ~B_DONE;
		}
		brelse(bp1);
		data[0] = code;
		*count = 1;
		break;

	default:
		return D_INVALID_OPERATION;
	}

	return errcode;

}

io_return_t hdsetstat(dev_t dev, int flavor, int *data, unsigned int count)
{
	io_return_t	errcode = D_SUCCESS;
	int		unit = UNIT(dev);
	struct disklabel*lp = &label[unit];
	int		part = PARTITION(dev);

	switch (flavor) {
	/* BsdLabel flavors */
	case DIOCWLABEL:
	case DIOCWLABEL - (0x10<<16):
	case DIOCSDINFO:
	case DIOCSDINFO - (0x10<<16):
	case DIOCWDINFO:
	case DIOCWDINFO - (0x10<<16):
		return hdsetlabel(dev, flavor, data, count);

	/* LocalLabel flavors */
	case V_REMOUNT:
		hdgotvtoc[unit] = 0;
		getvtoc(unit);	
		break;

	case V_ABS:
		abs_sec = *(int *)data;
		if (count == 2)
			abs_count = data[1];
		break;

	case V_WRABS:
		if (count < 512/sizeof (int)) {
			printf("hdsetstat: hd%d WRABS bad size %x",
			       unit,
			       count);
			return D_INVALID_OPERATION;
		}
		bp1 = geteblk(SECSIZE);
		bcopy( (caddr_t)data, (caddr_t)paddr(bp1), SECSIZE);
		bp1->b_flags = B_WRITE | B_MD1;
		bp1->b_blkno = abs_sec;
		bp1->b_dev = WHOLE_DISK(unit);
		bp1->b_bcount = SECSIZE;
		hdstrategy(bp1);
		biowait(bp1);
		if (bp1->b_flags & B_ERROR) {
			printf("hdsetstat: hd%d WRABS failed\n", unit);
			errcode = ENXIO;
		}
		brelse(bp1);
		break;

	default:
		return D_INVALID_OPERATION;
	}

	return errcode;
}

setcontroller(int unit)
{
}

/* hdstart() is called at spl5 */
static void hdstart(void)
{
	struct partition *part_p;
	int drivecount;
	register struct buf *bp, *dp;

	if (hh.busy)
		return;

	for (drivecount = 0; drivecount < NHD; drivecount++) {
		if (hh.curdrive < (NHD-1))
			hh.curdrive++;
		else
			hh.curdrive = 0;
		dp = &hdunit[hh.curdrive];
		if ((bp = dp->b_actf) != NULL)
			break;
	}
	if (drivecount == NHD)
		return;

	hh.busy = 1;
	hh.blocktotal = (bp->b_bcount + 511) >> 9;

	assert(hh.blocktotal > 0 || (bp->b_flags&B_RESET));

	part_p = &label[UNIT(bp->b_dev)].d_partitions[PARTITION(bp->b_dev)];
	
	if (bp->b_flags & B_MD1) {
		int end = label[UNIT(bp->b_dev)].d_secperunit - 1;
		hh.physblock = bp->b_blkno;
		if ((bp->b_blkno + hh.blocktotal) > end)
			hh.blocktotal = end - bp->b_blkno + 1;
	} else {
 		hh.physblock = part_p->p_offset + bp->b_blkno;
		if ((bp->b_blkno + hh.blocktotal ) > part_p->p_size) {
			hh.blocktotal = part_p->p_size - bp->b_blkno + 1;
		}
	}
	hh.blockcount = 0;
	hh.rw_addr = (paddr_t)bp->b_un.b_addr;
	hh.retries = 0;

	start_rw(bp->b_flags & (B_READ | B_RESET)); 

}

hdstrategy(struct buf *bp)
{
	unsigned char unit, partition;
	struct disklabel *lp;
	struct partition *part_p;
	unsigned int opri;
	long count, total;
	u_int p_addr, baddr;
	int i;
	long size;

	if (!bp->b_bcount)
		if (bp->b_flags != (bp->b_flags & B_RESET))
			goto done;

	unit = UNIT((bp->b_dev));
	lp = &label[unit];
	partition = PARTITION((bp->b_dev));
	part_p= &(label[unit].d_partitions[partition]);

#if	0
	if (!(bp->b_flags & B_READ) && (part_p->p_flag & V_RONLY))  {
		printf("hd%d%c: attempt to write read-only partition\n",
		       unit,
		       'a' + partition);
		bp->b_error = ENXIO;
		goto bad;
	}
#endif	/* 0 */

	/* if request is off the end or trying to write last block on out */
	size = (bp->b_flags & B_MD1) ? lp->d_secperunit : part_p->p_size;
	if ((bp->b_blkno > size) ||
	    (bp->b_blkno == size & !(bp->b_flags & B_READ))) {
		printf("hd%d%c: invalid block %d >= %d\n",
		       unit,
		       'a' + partition,
		       bp->b_blkno,
		       size);
		bp->b_error = ENXIO;
		goto bad;
	}
	if (bp->b_blkno == size) {
		/* indicate (read) EOF by setting b_resid to b_bcount on last block */ 
		bp->b_resid = bp->b_bcount;
		goto done;
	}
	bp->b_cylin = ((bp->b_flags&B_MD1 ? 0 : part_p->p_offset) + bp->b_blkno)
		/ (lp->d_nsectors * lp->d_ntracks);

	opri = spl5();
	disksort(&hdunit[unit], bp);
	if (!hh.busy)
		hdstart();
	splx(opri);

	return;

bad:
	bp->b_flags |= B_ERROR;
done:
	iodone(bp);
	return;

}

static hderror(struct buf *bp)
{
	int unit = UNIT(bp->b_dev);

	if ((hh.status&0x8000) == 0 || ++hh.retry_count > hdparams[unit].retries) {
		if(bp) {
			/************************************************
			* We have a problem with this block, set the	*
			* error flag, terminate the operation and move	*
			* on to the next request.			*
			* With every hard disk transaction error we set	*
			* the reset requested flag so that the contrlr	*
			* is reset before next operation is started.	*
			* A reset is a relatively long operation, the	*
			* upper level routines are better qualified for	*
			* such an operation than the interrupt service	*
			* routines.					*
			************************************************/
			hdunit[hh.curdrive].b_actf = bp->av_forw;
			bp->b_flags |= B_ERROR;
			bp->b_resid = 0;
			biodone(bp);
			hh.busy = 0;
			hdstart();
		}
	} else {
		/*
		 * retry the operation using the saved request block
		 */
		rb[unit] = save_rb[unit];
		abios_common_start(&rb[unit], params.Logical_id_flags);
	}
}

/*
 * The following function is used to get the proper physical
 * addresses for transfering data.
 */
static u_int hd_max_page(struct buf *bp,
                          u_int baddr,
                          u_int *p_addr,
                          u_int max)
{
	u_int	count = ((u_int)baddr) & (I386_PGBYTES - 1);
	u_int	old_addr;
	u_int	new_addr;


	if ((*p_addr = pmap_extract(get_pmap(bp), baddr)) == 0)
		return(0);

	/* if the count is aligned, we can transfer a whole page. */
	if (count==0)
		count = I386_PGBYTES;
	else
		count = I386_PGBYTES - count;

	/* Now loop through and get the phisical address of each of the 
	   following pages.  Break on the first non-contiguous page, or when
	   we exceed count.
	*/

	baddr += count;
	old_addr = *p_addr + count;
	while ((count <max) &&
	    ((new_addr = pmap_extract(get_pmap(bp), baddr)) != 0) &&
	    (new_addr == old_addr)) {
		count += I386_PGBYTES;
		baddr += I386_PGBYTES;
		old_addr += I386_PGBYTES;
	}

	return (count > max) ? max : count;

}

/*
 * routine called from timeout code at lower priority
 * we just call hdintr with the provided parameter.
 */
static void hdtimeout(int vec)
{
	unsigned int  s = spl5();
	hdintr(vec);
	splx(s);
}

static void start_rw(long read)
{
	unsigned int track, disk_block, xblk;
	u_int numblocks;
	unsigned int p_block;
	int function;
	paddr_t	vert_addr;
	u_int 	p_addr;
	register struct buf *bp, *dp;
	int	i;
	u_int	total;
	u_char	unit;

	dp = &hdunit[unit = hh.curdrive];
	bp = dp->b_actf;

restart:

	hh.retry_count = 0; /* start of operation */
	rb[unit].r_current_req_blck_len = sizeof(struct Gd_request);
	rb[unit].r_logical_id = hdparams[unit].lid;
	rb[unit].r_unit = hdparams[unit].unit;
	rb[unit].r_return_code = ABIOS_UNDEFINED;

	vert_addr = hh.rw_addr + (hh.blockcount * SECSIZE);
	hh.vert_addr = vert_addr;
	if ((u_int)vert_addr & 0x1ff) {
		hh.un_aligned = 1;
		vert_addr = (paddr_t)u_buf[unit];
	}
	p_block = hh.physblock + hh.blockcount;

	numblocks = hd_max_page(bp, vert_addr, &p_addr, 
	    ((hh.blocktotal - hh.blockcount)*SECSIZE));
	numblocks  = ((numblocks + 511) >> 9);

	assert(numblocks > 0 || (bp->b_flags&B_RESET));

	hh.phys_addr = p_addr;
	hh.blockcount += numblocks;
	hh.numblocks = numblocks;

	switch (read) {

	case B_READ:
		rb[unit].r_function = ABIOS_READ;
		function = ABIOS_READ;
		rb[unit].request_header.Request_Block_Flags = 0;
		rb[unit].request_header.ELA_Offset = 0;
		gd_physical_ptr(rb[unit]) = p_addr;
		gd_logical_ptr(rb[unit]) = 0;
		gd_blocks_to_read(rb[unit]) = numblocks; /*hh.blocktotal;*/
		gd_relative_block_address(rb[unit]) = p_block; /*hh.physblock;*/
		gd_caching_ok(rb[unit]) = GD_DONT_CACHE;
		GD_SET_RESERVED_ABIOS_READ(rb[unit]);
		break;

	case B_RESET:
		rb[unit].r_function = ABIOS_RESET;
		function = ABIOS_RESET;
		GD_SET_RESERVED_ABIOS_RESET(rb[unit]);
		break;

	default:
		rb[unit].r_function = hd_write_verify ? ABIOS_WRITE_VERIFY : ABIOS_WRITE;
		function = hd_write_verify ? ABIOS_WRITE_VERIFY : ABIOS_WRITE;
		rb[unit].request_header.Request_Block_Flags = 0;
		rb[unit].request_header.ELA_Offset = 0;
		gd_physical_ptr(rb[unit]) = p_addr;
		gd_logical_ptr(rb[unit]) = 0;
		gd_blocks_to_read(rb[unit]) = numblocks; /*hh.blocktotal;*/
		gd_relative_block_address(rb[unit]) = p_block; /*hh.physblock;*/
		gd_caching_ok(rb[unit]) = GD_DONT_CACHE;
		GD_SET_RESERVED_ABIOS_WRITE(rb[unit]);
		if(hh.un_aligned) {
		    i = 0;
		    total = 0;
		    while (save_addr[unit][i].phys_addr != NULL) {
			bcopy(phystokv(save_addr[unit][i].phys_addr), 
			    (caddr_t)((u_int)u_buf[unit] + total), 
			    save_addr[unit][i].count);
			total += save_addr[unit][i].count;
			i++;
		    }
		}
		break;
	}

	save_rb[unit] = rb[unit]; /* save in case of retry */

	abios_common_start(&rb[unit], params.Logical_id_flags);

	hh.status = rb[unit].r_return_code; 

	switch (hh.status) {
		
		case ABIOS_STAGE_ON_INT:
			break;

		case ABIOS_STAGE_ON_TIME:
			timeout(hdtimeout,0,(gd_wait_time(rb[unit])/1000000)*HZ);
			break;

		case (ABIOS_NOT_MY_INT | ABIOS_STAGE_ON_INT):
			/* Eventually we need to chain to the next */
			/* interrupt. */
			break;

		case ABIOS_DONE:
			if (hh.blockcount == hh.blocktotal) {
			    hh.blockcount = 0;
			    hh.blockcount = 0;
			    hh.phys_addr = 0;
			    hh.vert_addr = 0;
			    hh.numblocks = 0;
			    dp->b_actf = bp->av_forw;
			    bp->b_resid = 0;
			    iodone(bp);
			    hh.busy = 0;
			    hdstart();
			    break;
			}
			else {
			    hh.phys_addr = 0;
			    hh.vert_addr = 0;
			    hh.numblocks = 0;
			    goto restart;
			}

		default:
			printf("start_rw: return code [%x]...\n", hh.status); 
			hd_error_decode(hh.status, "\n");
	}	

}

#define B_FLAGS_FMT "\20\1READ\2DONE\3ERROR\4BUSY\5PHYS\6XXX\7WANTED\10AGE\11ASYNC\12DELWRI\13TAPE\20CACHE\21INVAL\22LOCKED\23HEAD\24USELESS\25BAD\27RAW\30NOCACHE\31PRIVATE\32WRITEV\33HWRELOC\34WANTFREE"

int hd_bp_all = 0;

static void hd_bp_print(char *msg1, struct buf *bp, char *msg2)
{
	printf("%sbp=%x ", msg1 ? msg1 : "", bp);
	if (bp) {
		printf("b_dev=0x%x b_blkno=0x%x (%d) b_addr=0x%x b_bcount=0x%x b_flags=%b",
			bp->b_dev, bp->b_blkno, bp->b_blkno, bp->b_un.b_addr, bp->b_bcount,
			bp->b_flags,B_FLAGS_FMT);
		if (hd_bp_all) {
			printf(" b_forw=0x%x b_back=0x%x av_forw=0x%x av_back=0x%x b_bufsize=%d",
				bp->b_forw, bp->b_back, bp->av_forw, bp->av_back,
				bp->b_bufsize);
		}
	}
	if (msg2)
		printf(msg2);
}

static struct hd_errors { int code; char *msg; } hd_err_codes [] = {
	{0x00,	"timeout"},
	{0x01,	"bad-command"},
	{0x02,	"address-mark-not-found"},
	{0x04,	"record-not-found"},
	{0x05,	"reset-failed"},
	{0x07,	"activity-failed"},
	{0x0a,	"defective-sector"},
	{0x0b,	"bad-track"},
	{0x0d,	"invalid-sector"},
	{0x0e,	"CAM-detected"},
	{0x0f,	"DMA-arb-level-bad"},
	{0x10,	"bad-ecc-error"},
	{0x11,	"ecc-corrected"},
	{0x20,	"bad-controller"},
	{0x21,	"equipment-check"},
	{0x40,	"bad-seek"},
	{0x80,	"device-didn't-respond"},
	{0xaa,	"drive-not-ready"},
	{0xbb,	"undefined-error"},
	{0xcc,	"write-fault"},
	{0xff,	"incomplete-sense"},
	{0xc000,"invalid LID"},
	{0xc001,"invalid function"},
	{0xc003,"invalid unit number"},
	{0xc004,"invalid request block length"},
	{0xc005,"invalid parameter"}

};

static hd_error_decode(int code, char *msg)
{
	int i;
	int n = code&0x00ff;

	if ((code&0x8000) == 0) {
		char *p;
		switch(code)
		{
		case 0:
			p = "completed ok";
			break;
		case 1:
			p = "stage on int";
			break;
		case 2:
			p = "stage on time";
			break;
		case 5:
			p = "not my int";
			break;
		default:
			printf("unknown[0x%x]%s", code, msg);
			return;
		}
		printf("%s%s", p, msg);
		return;
	}
	if (code&0x4000) {
		printf("PARAMETER ");
		n = code;
	}
	if (code&0x2000)
		printf("TIME-OUT ");
	if (code&0x1000)
		printf("DEVICE ");
	if (code&0x100)
		printf("RETRYABLE ");
	printf("ERROR ");
	for (i=0; i<(sizeof hd_err_codes)/(sizeof hd_err_codes[0]); ++i)
		if (hd_err_codes[i].code == n)
			{
			printf("%s%s", hd_err_codes[i].msg, msg);
			return;
			}
	printf("unknown[0x%x]%s", code, msg);
}

static int hdintr(int vec)
{
	register struct buf *bp, *dp;
	int i;
	unsigned long	count, total;
	unsigned char unit = hh.curdrive;
	int rc = 1;				/* assume it is ours */

	if (!hh.busy && !hh.restoring) {
		printf("hdintr: false interrupt continuing . . .\n");
		printf("hdintr: hh.busy = %d, hh.restoring = %d\n",
			hh.busy, hh.restoring);
		printf("hdintr: hh.single_mode = %d, ",hh.single_mode);
		printf("hdintr: hh.physblock = %d, ",hh.physblock);
		printf("hh.blockcount = %d, ",hh.blockcount);
		printf("hh.blocktotal = %d\n",hh.blocktotal);
		return;
	}

	abios_common_interrupt(&rb[unit], params.Logical_id_flags);

	hh.status = rb[unit].r_return_code; 

	dp = &hdunit[hh.curdrive];
	bp = dp->b_actf;

	switch (hh.status) {
	
	case ABIOS_STAGE_ON_INT:
		break;

	case ABIOS_STAGE_ON_TIME:
		timeout(hdtimeout, 0, (gd_wait_time(rb[unit])/1000000)*HZ);
		break;

	case ABIOS_NOT_MY_INT:
	case (ABIOS_NOT_MY_INT | ABIOS_STAGE_ON_INT):
		rc = 0;
		break;

	case ABIOS_DONE:
		if (hh.un_aligned) {
		    hh.un_aligned = 0;
		    if (bp->b_flags & B_READ) {
			unit = UNIT((bp->b_dev));
			i = 0;
			total = 0;
			while (save_addr[unit][i].phys_addr) {
			    bcopy( 
				(caddr_t)((u_int)u_buf[unit] + total), 
				phystokv(save_addr[unit][i].phys_addr),
				save_addr[unit][i].count);
			    total += save_addr[unit][i].count;
			    i++;
			}
		    }
		}
		if (hh.blockcount == hh.blocktotal) {
			dp->b_actf = bp->av_forw;
			bp->b_resid = 0;
			if (hh.retry_count)
				printf("retry worked!\n");
			biodone(bp);
			hh.busy = 0;
			hdstart();
		}
		else {
			start_rw(bp->b_flags & (B_READ|B_RESET));
		}
		break;

	default:
		printf("[HD Driver] return code [%x]...", hh.status);
		hd_error_decode(hh.status,""); 
		hd_bp_print(" ", bp, "\n");
		hderror(bp);
		break;

	}

	return rc;

}

#endif NHD > 0
