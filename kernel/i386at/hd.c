/* 
 * Mach Operating System
 * Copyright (c) 1993,1991,1990,1989 Carnegie Mellon University
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
 * $Log:	hd.c,v $
 * Revision 2.23  93/08/10  15:57:25  mrt
 * 	We now use bsd labels internally and convert from "local labels"
 * 	if we find them on the disk. 
 * 	[93/08/03  20:36:05  rvb]
 * 
 *	Fix DIOCxxx ioctls to deal with MAXPARTITION or MAXPARTITION+1
 *	parititions.  We'll do this better next release.	
 * 
 * Revision 2.22  93/05/10  20:03:39  rvb
 * 	Types.
 * 	[93/05/08  11:17:48  af]
 * 
 * Revision 2.21  93/03/11  13:58:01  danner
 * 	u_long -> u_int.
 * 	[93/03/09            danner]
 * 
 * Revision 2.20  93/01/24  13:15:53  danner
 * 	Deal with DPT WD1003 emulation that clears BUSY before READY is
 * 	set.
 * 	[92/10/01            rvb]
 * 
 * Revision 2.19  93/01/14  17:30:21  danner
 * 	Proper spl typing.
 * 	[92/11/30            af]
 * 
 * Revision 2.18  92/07/09  22:54:06  rvb
 * 	Transcription error missed a line.
 * 	[92/06/23  11:16:20  rvb]
 * 
 * 	Setcontroller should be called synchronously with hd_start().  So
 * 	that the controller is idle.
 * 	[92/06/20            rvb]
 * 
 * 	Defer setcontroller(unit) to getvtoc();  So we only setcontroller()
 * 	what we open, not what we probe.  ESDI+SCSI messes up the device
 * 	count.
 * 	[92/06/18            rvb]
 * 
 * Revision 2.17  92/03/01  00:39:53  rpd
 * 	Cleaned up syntactically incorrect ifdefs.
 * 	[92/02/29            rpd]
 * 
 * Revision 2.16  92/02/23  22:43:07  elf
 * 	Added (mandatory) DEV_GET_SIZE flavor of get_status.
 * 	[92/02/22            af]
 * 
 * Revision 2.15  92/02/19  16:29:51  elf
 * 	On 25-Jan, did not consider NO ACTIVE mach parition.
 * 	Add "BIOS" support -- always boot mach partition NOT active one.
 * 	[92/01/31            rvb]
 * 
 * Revision 2.14  92/01/27  16:43:06  rpd
 * 	Fixed hdgetstat and hdsetstat to return D_INVALID_OPERATION
 * 	for unsupported flavors.
 * 	[92/01/26            rpd]
 * 
 * Revision 2.13  92/01/14  16:43:51  rpd
 * 	Error in badblock_mapping code in the case there was sector replacement.
 * 	For all the sectors in the disk block before the bad sector, you
 * 	badblock_mapping should give the identity map and it was not.
 * 	[92/01/08            rvb]
 * 
 * Revision 2.12  91/11/18  17:34:19  rvb
 * 	For now, back out the hdprobe(), hdslave() probes and use
 * 	the old simple test and believe BIOS.
 * 
 * Revision 2.11  91/11/12  11:09:32  rvb
 * 	Amazing how hard getting the probe to work for all machines is.
 * 	V_REMOUNT must clear gotvtoc[].
 * 	[91/10/16            rvb]
 * 
 * Revision 2.10  91/10/07  17:25:35  af
 * 	Now works with 2 disk drives, new probe/slave routines, misc cleanup
 * 	[91/08/07            mg32]
 * 
 * 	From 2.5:
 *	Rationalize p_flag
 *	Kill nuisance print out.
 *	Removed "hdioctl(): do not recognize ioctl ..." printf().
 * 	[91/08/07            rvb]
 * 
 * Revision 2.9  91/08/28  11:11:42  jsb
 * 	Replace hdbsize with hddevinfo.
 * 	[91/08/12  17:33:59  dlb]
 * 
 * 	Add block size routine.
 * 	[91/08/05  17:39:16  dlb]
 * 
 * Revision 2.8  91/08/24  11:57:46  af
 * 	New MI autoconf.
 * 	[91/08/02  02:52:47  af]
 * 
 * Revision 2.7  91/05/14  16:23:24  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/02/05  17:17:01  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:43:01  mrt]
 * 
 * Revision 2.5  91/01/08  17:32:51  rpd
 * 	Allow ioctl's
 * 	[90/12/19            rvb]
 * 
 * Revision 2.4  90/11/26  14:49:37  rvb
 * 	jsb bet me to XMK34, sigh ...
 * 	[90/11/26            rvb]
 * 	Synched 2.5 & 3.0 at I386q (r1.8.1.15) & XMK35 (r2.4)
 * 	[90/11/15            rvb]
 * 
 * Revision 1.8.1.14  90/09/18  08:38:49  rvb
 * 	Typo & vs && at line 592.		[contrib]
 * 	Make Status printout on error only conditional on hd_print_error.
 * 	So we can get printout during clobber_my_disk.
 * 	[90/09/08            rvb]
 * 
 * Revision 1.8.1.13  90/08/25  15:44:38  rvb
 * 	Use take_<>_irq() vs direct manipulations of ivect and friends.
 * 	[90/08/20            rvb]
 * 
 * Revision 1.8.1.12  90/07/27  11:25:30  rvb
 * 	Fix Intel Copyright as per B. Davies authorization.
 * 	Let anyone who as opened the disk do absolute io.
 * 	[90/07/27            rvb]
 * 
 * Revision 1.8.1.11  90/07/10  11:43:22  rvb
 * 	Unbelievable bug in setcontroller.
 * 	New style probe/slave/attach.
 * 	[90/06/15            rvb]
 * 
 * Revision 1.8.1.10  90/03/29  19:00:00  rvb
 * 	Conditionally, print out state info for "state error".
 * 	[90/03/26            rvb]
 * 
 * Revision 1.8.1.8  90/03/10  00:27:20  rvb
 * 	Fence post error iff (bp->b_blkno + hh.blocktotal ) > partition_p->p_size)
 * 	[90/03/10            rvb]
 * 
 * Revision 1.8.1.7  90/02/28  15:49:35  rvb
 * 	Fix numerous typo's in Olivetti disclaimer.
 * 	[90/02/28            rvb]
 * 
 * Revision 1.8.1.6  90/01/16  15:54:14  rvb
 * 	FLush pdinfo/vtoc -> evtoc
 * 	[90/01/16            rvb]
 * 
 * 	Must be able to return "dos{cyl,head,sector}"
 * 	[90/01/12            rvb]
 * 
 * 	Be careful about p_size bound's checks if B_MD1 is true.
 * 	[90/01/12            rvb]
 * 
 * Revision 1.8.1.5  90/01/08  13:29:29  rvb
 * 	Add Intel copyright.
 * 	Add Olivetti copyright.
 * 	[90/01/08            rvb]
 * 
 * 	It is no longer possible to set the start and size of disk
 * 	partition "PART_DISK" -- it is always loaded from the DOS
 * 	partition data.
 * 	[90/01/08            rvb]
 * 
 * Revision 1.8.1.4  90/01/02  13:54:58  rvb
 * 	Temporarily regress driver to one that is known to work with Vectra's.
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
 *  AT Hard Disk Driver
 *  Copyright Ing. C. Olivetti & S.p.A. 1989
 *  All rights reserved.
 *
 */
/*
  Copyright 1988, 1989 by Olivetti Advanced Technology Center, Inc.,
Cupertino, California.

		All Rights Reserved

  Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Olivetti
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

  OLIVETTI DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL OLIVETTI BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUR OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <hd.h>

#include <sys/types.h>
#ifdef	MACH_KERNEL
#define PRIBIO 20
#include <device/buf.h>
#include <device/errno.h>
#include <device/device_types.h>
#include <device/disk_status.h>
#else	MACH_KERNEL
#include <sys/buf.h>
#include <sys/user.h> 
#endif	MACH_KERNEL
#include <sys/ioctl.h>
#include <i386/pio.h>
#include <i386/ipl.h>
#include <i386at/disk.h>
#include <chips/busses.h>
#include <i386at/hdreg.h>

#define LABEL_DEBUG(x,y) if (label_flag&x) y

/* From sys/systm.h */
struct buf *geteblk();

#define b_cylin		b_resid
#define PAGESIZ 4096

int devaddr[NHD/2];
int hdalive[NHD];
int hdgotvtoc[NHD];
struct hh hh[NHD/2];
struct alt_info alt_info[NHD];
struct buf hdbuf[NHD], hdunit[NHD];

int need_set_controller[NHD/2];

struct disklabel label[NHD];
int	   labeloffset[NHD];
int	   labelsector[NHD];

struct hd_param {
	unsigned short	ncyl;
	unsigned short	nheads;
	unsigned short	precomp;
	unsigned short	nsec;
}	        dosparm[NHD];

int hdstrategy(), hdminphys(), hdprobe(), hdslave(), hdintr();
void hdattach();
struct bus_device *hdinfo[NHD];

vm_offset_t	hd_std[NHD] = { 0 };
struct	bus_device *hd_dinfo[NHD*NDRIVES];
struct	bus_ctlr *hd_minfo[NHD];
struct	bus_driver	hddriver = {
	hdprobe, hdslave, hdattach, 0, hd_std, "hd", hd_dinfo, "hdc", hd_minfo, 0};


hdprobe(port, ctlr)
struct bus_ctlr *ctlr;
{
	int 	i,
		ctrl = ctlr->unit;
	int 	addr = devaddr[ctrl] = (int)ctlr->address;

	outb(PORT_DRIVE_HEADREGISTER(addr),0);
	outb(PORT_COMMAND(addr),CMD_RESTORE);
	for (i=500000; i && inb(PORT_STATUS(addr))&STAT_BUSY; i--);
	for (; i; i--) {
		if (inb(PORT_STATUS(addr))&STAT_READY) {
			take_ctlr_irq(ctlr);
			hh[ctrl].curdrive = ctrl<<1;
			printf("%s%d: port = %x, spl = %d, pic = %d.\n", ctlr->name,
				ctlr->unit, ctlr->address, ctlr->sysdep, ctlr->sysdep1);
#if	0
			/* may be necesary for two controllers */
			outb(FIXED_DISK_REG(ctrl), 4);
			for(i = 0; i < 10000; i++);
			outb(FIXED_DISK_REG(ctrl), 0);
#endif
			return(1);
		}
	}
	return(0);
}

/*
 * hdslave:
 *
 *	Actually should be thought of as a slave probe.
 *
 */

hdslave(dev, xxx)
struct bus_device *dev;
{
	int	i, j,
		addr = devaddr[dev->ctlr];
	u_char	*bios_magic = (u_char *)(0xc0000475);

	if (dev->ctlr == 0)				/* for now: believe DOS */
		if (*bios_magic >= 1 + dev->slave)
			return 1;
		else
			return 0;
	else
		return 1;
		
#if	0
	/* it is working on all types of PCs */
	outb(PORT_DRIVE_HEADREGISTER(addr),dev->slave<<4);
	outb(PORT_COMMAND(addr),CMD_RESTORE);

	for (i=350000; i && (j = inb(PORT_STATUS(addr)))&STAT_BUSY; i--);
	for (j = 0; i && !(j & STAT_READY); i--)
		j = inb(PORT_STATUS(addr));
	if (i == 0) {
		outb(FIXED_DISK_REG(dev->ctlr), 4);
		for(i = 0; i < 10000; i++);
		outb(FIXED_DISK_REG(dev->ctlr), 0);
		setcontroller(dev->slave);
		return 0;
	}
	return(j&STAT_READY);
#endif
}

/*
 * hdattach:
 *
 *	Attach the drive unit that has been successfully probed.  For the
 *	AT ESDI drives we will initialize all driver specific structures
 *	and complete the controller attach of the drive.
 *
 */

void hdattach(dev)
struct bus_device *dev;
{
	int 			unit = dev->unit;
	struct disklabel	*lp = &label[unit];
	u_int			n;
	u_char			*tbl;

	hdalive[unit] = 1;
	n = *(unsigned long *)phystokv(dev->address);
	tbl = (unsigned char *)phystokv((n&0xffff) + ((n>>12)&0xffff0));
	dosparm[unit].ncyl	= *(unsigned short *)tbl;
	dosparm[unit].nheads	= *(unsigned char  *)(tbl+2);
	dosparm[unit].precomp	= *(unsigned short *)(tbl+5);
	dosparm[unit].nsec	= *(unsigned char  *)(tbl+14);
	printf(", stat = %x, spl = %d, pic = %d\n",
		dev->address, dev->sysdep, dev->sysdep1);

	/*
	 * copy initial parameters into label
	 */
	fudge_bsd_label(lp, DTYPE_ESDI, dosparm[unit].ncyl, dosparm[unit].nheads,
			dosparm[unit].nsec, 2);

	if (unit<2)
		printf(" hd%d:   %dMeg, cyls %d, heads %d, secs %d, precomp %d",
			unit, lp->d_secperunit * 512/1000000,
			lp->d_ncylinders, lp->d_ntracks, lp->d_nsectors,
			dosparm[unit].precomp);
	else
		printf("hd%d:   Capacity not available through bios\n",unit);
}

hdopen(dev, flags)
{
	u_char	unit = UNIT(dev),
		part = PARTITION(dev);

	if (!hdalive[unit] || part >= MAXPARTITIONS)
		return(ENXIO);
	getvtoc(unit);
	if (!(label[unit].d_partitions[part].p_offset) &&
	    !(label[unit].d_partitions[part].p_size))
		return(ENXIO);
	return(0);
}

hdclose(dev)
{
	return D_SUCCESS;
}

#ifdef	MACH_KERNEL
/*
 *	No need to limit IO size to 4096 bytes.
 */
int hdread(dev, ior)
dev_t dev;
io_req_t ior;
{
	return (block_io(hdstrategy, minphys, ior));
}

int hdwrite(dev, ior)
dev_t dev;
io_req_t ior;
{
	return (block_io(hdstrategy, minphys, ior));
}
#else	MACH_KERNEL
hdminphys(bp)
struct buf *bp;
{
	if (bp->b_bcount > PAGESIZ)
		bp->b_bcount = PAGESIZ;
}

hdread(dev,uio)
dev_t dev;
struct uio *uio;
{
	return(physio(hdstrategy,&hdbuf[UNIT(dev)],dev,B_READ,hdminphys,uio));
}

hdwrite(dev,uio)
dev_t dev;
struct uio *uio;
{
	return(physio(hdstrategy,&hdbuf[UNIT(dev)],dev,B_WRITE,hdminphys,uio));
}
#endif	MACH_KERNEL

#ifdef	MACH_KERNEL
int abs_sec   = -1;
int abs_count = -1;
/* IOC_OUT only and not IOC_INOUT */
io_return_t hdgetstat(dev, flavor, data, count)
	dev_t dev;
	int flavor;
	int *data;		/* pointer to OUT array */
	unsigned int *count;	/* OUT */
{
	int			unit = UNIT(dev);
	struct disklabel	*lp = &label[unit];
	struct buf		*bp1;
	int			i;

	switch (flavor) {

	/* Mandatory flavors */

	case DEV_GET_SIZE: {
		int part = PARTITION(dev);
		data[DEV_GET_SIZE_DEVICE_SIZE] = lp->d_partitions[part].p_size * lp->d_secsize;
		data[DEV_GET_SIZE_RECORD_SIZE] = lp->d_secsize;
		*count = DEV_GET_SIZE_COUNT;
		break;
	}

	/* BsdLabel flavors */
	case DIOCGDINFO:
	case DIOCGDINFO - (0x10<<16):
		dkgetlabel(lp, flavor, data, count);
		break;

	/* Extra flavors */
	case V_GETPARMS: {
		struct disk_parms *dp;
		int part = PARTITION(dev);
		if (*count < sizeof (struct disk_parms)/sizeof(int))
			return (D_INVALID_OPERATION);
		dp = (struct disk_parms *) data;
		dp->dp_type = DPT_WINI;
		dp->dp_heads = 	lp->d_ntracks;
		dp->dp_cyls = 	lp->d_ncylinders;
		dp->dp_sectors  = lp->d_nsectors;
  		dp->dp_dosheads = dosparm[unit].nheads;
		dp->dp_doscyls = dosparm[unit].ncyl;
		dp->dp_dossectors  = dosparm[unit].nsec;
		dp->dp_secsiz = lp->d_secsize;
		dp->dp_ptag = 0;
		dp->dp_pflag = 0;
		dp->dp_pstartsec = lp->d_partitions[part].p_offset;
		dp->dp_pnumsec = lp->d_partitions[part].p_size;
		*count = sizeof(struct disk_parms)/sizeof(int);
		break;
	}
	case V_RDABS: {
		/* V_RDABS is relative to head 0, sector 0, cylinder 0 */
		if (*count < SECSIZE/sizeof (int)) {
			printf("hd%d: RDABS bad size %x", unit, count);
			return (D_INVALID_OPERATION);
		}
		bp1 = geteblk(SECSIZE);
		bp1->b_flags = B_READ | B_MD1;	/* MD1 is be absolute */
		bp1->b_blkno = abs_sec;
		bp1->b_dev = WHOLE_DISK(unit);	/* C partition */
		bp1->b_bcount = SECSIZE;
		hdstrategy(bp1);
		biowait(bp1);
		if (bp1->b_flags & B_ERROR) {
			printf("hd%d hdsetstat(): read failure RDABS\n", unit);
			brelse(bp1);
			return (ENXIO);
		}
		bcopy(bp1->b_un.b_addr, (caddr_t)data, SECSIZE);
		brelse(bp1);
		*count = SECSIZE/sizeof(int);
		break;
	}
	case V_VERIFY: {
		unsigned int snum;
		int xcount;
		int code = 0;
		bp1 = geteblk(PAGESIZ);
		bp1->b_flags = B_READ;
		bp1->b_blkno = abs_sec;
		bp1->b_dev = WHOLE_DISK(unit);	/* C partition */
		xcount = abs_count;
		snum = PAGESIZ >> 9;
		while (xcount > 0) {
			i = (xcount > snum) ? snum : xcount;
			bp1->b_bcount = i << 9;
			bp1->b_flags |= B_MD1;
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
	}
	default:
		return (D_INVALID_OPERATION);
	}
	return (D_SUCCESS);
}

/* IOC_VOID or IOC_IN or IOC_INOUT */
io_return_t hdsetstat(dev, flavor, data, count)
dev_t dev;
int flavor;
int * data;
unsigned int count;
{
	struct buf		*bp1;
	io_return_t		errcode = D_SUCCESS;
	int			unit = UNIT(dev);
	struct disklabel	*lp = &label[unit];

	switch (flavor) {
	/* BsdLabel flavors */
	case DIOCWLABEL:
	case DIOCWLABEL - (0x10<<16):
		if (*(int*)data)
/*
			tgt->flags |= TGT_WRITE_LABEL;
		else
			tgt->flags &= ~TGT_WRITE_LABEL;
*/		break;
	case DIOCSDINFO:
	case DIOCSDINFO - (0x10<<16):
	case DIOCWDINFO:
	case DIOCWDINFO - (0x10<<16):
		if (count != sizeof(struct disklabel) / sizeof(int))
			return D_INVALID_SIZE;
		/*
		 * setdisklabel is in scsi/rz_disk.c; but is generic
		 */
		errcode = setdisklabel(lp, (struct disklabel*) data);
		if (errcode || (flavor == DIOCSDINFO) || (flavor == DIOCSDINFO - (0x10<<16)))
			return errcode;
		errcode = hdwritelabel(unit);
		break;
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
		/* V_WRABS is relative to head 0, sector 0, cylinder 0 */
		if (count < SECSIZE/sizeof (int)) {
			printf("hd%d: WRABS bad size %x", unit, count);
			return (D_INVALID_OPERATION);
		}
		bp1 = geteblk(SECSIZE);
		bcopy((caddr_t)data, bp1->b_un.b_addr, SECSIZE);
		bp1->b_flags = B_WRITE | B_MD1;	/* MD1 is be absolute */
		bp1->b_blkno = abs_sec;
		bp1->b_dev = WHOLE_DISK(unit);	/* C partition */
		bp1->b_bcount = SECSIZE;
		hdstrategy(bp1);
		biowait(bp1);
		if (bp1->b_flags & B_ERROR) {
			printf("hd%d: hdsetstat() read failure WRABS\n", unit);
			errcode = ENXIO;
		}
		brelse(bp1);
		break;
	default:
		return (D_INVALID_OPERATION);
	}
	return (errcode);
}

#else	MACH_KERNEL
hdioctl(dev, cmd, arg, mode)
dev_t dev;
int cmd;
caddr_t arg;
{
	u_char			unit = UNIT(dev),
				ctrl = unit>>1;
	struct disklabel	*lp = &label[unit];
	union io_arg		*arg_kernel;
	int			snum, xcount, i;
	struct buf		*bp1;

	switch (cmd) {
	case V_CONFIG:
		arg_kernel = (union io_arg *)arg;
		if (arg_kernel->ia_cd.secsiz != SECSIZE)
			/* changing sector size NOT allowed */
		  	return(EINVAL);
		lp->d_ncylinders=(unsigned short)arg_kernel->ia_cd.ncyl;
		lp->d_ntracks=(unsigned short)arg_kernel->ia_cd.nhead;
		lp->d_nsectors=(unsigned short)arg_kernel->ia_cd.nsec;
		fudge_bsd_label(lp, lp->d_type,
				lp->d_ncylinders, lp->d_ntracks,
				lp->d_nsectors, lp->d_npartitions);
		setcontroller(unit);
		break;
	case V_REMOUNT:
		hdgotvtoc[unit] = 0;
		getvtoc(unit);	
		break;
	case V_ADDBAD:
		/* this adds a bad block to IN CORE alts table ONLY */
		alt_info[unit].alt_sec.alt_used++;
		alt_info[unit].alt_sec.alt_bad[alt_info[unit].alt_sec.alt_used]
			= ((union io_arg *)arg)->ia_abs.bad_sector;
		break;
	case V_GETPARMS:
		{
		unsigned char part = PARTITION(dev);
		struct disk_parms *disk_parms = (struct disk_parms *)arg;
		disk_parms->dp_type = DPT_WINI;
		disk_parms->dp_heads = lp->d_ntracks;
		disk_parms->dp_cyls = lp->d_ncylinders;
		disk_parms->dp_sectors  = lp->d_nsectors
  		disk_parms->dp_dosheads = dosparm[unit].nheads;
		disk_parms->dp_doscyls = dosparm[unit].ncyl;
		disk_parms->dp_dossectors  = dosparm[unit].nsec;
		disk_parms->dp_secsiz = SECSIZE;
		disk_parms->dp_ptag = 0;
		disk_parms->dp_pflag = 0;
		disk_parms->dp_pstartsec =lp->d_partitions[part].p_offset;
		disk_parms->dp_pnumsec =lp->d_partitions[part].p_size;
		break;
		}
	case V_RDABS:
		/* V_RDABS is relative to head 0, sector 0, cylinder 0 */
		bp1 = geteblk(SECSIZE);
		bp1->b_flags = B_READ | B_MD1;	/* MD1 is be absolute */
		bp1->b_blkno = ((struct absio *)arg)->abs_sec;
		bp1->b_dev = WHOLE_DISK(unit);	/* C partition for RDABS */
		bp1->b_bcount = SECSIZE;
		hdstrategy(bp1);
		biowait(bp1);
		if (bp1->b_flags & B_ERROR) {
			printf("hd: read failure on ioctl\n");
			brelse(bp1);
			return(ENXIO);
		}
		if(copyout(bp1->b_un.b_addr,((struct absio *)arg)->abs_buf,
			   SECSIZE)){
			brelse(bp1);
			return(ENXIO);
		}
		brelse(bp1);
		break;
	case V_WRABS:
		/* V_WRABS is relative to head 0, sector 0, cylinder 0 */
		bp1 = geteblk(SECSIZE);
		if (copyin(((struct absio *)arg)->abs_buf,bp1->b_un.b_addr,
			   SECSIZE)){
			brelse(bp1);
			return(ENXIO);
		}
		bp1->b_flags = B_WRITE | B_MD1;	/* MD1 is be absolute */
		bp1->b_blkno = ((struct absio *)arg)->abs_sec;
		bp1->b_dev = WHOLE_DISK(unit);	/* C partition for RDABS */
		bp1->b_bcount = SECSIZE;
		hdstrategy(bp1);
		biowait(bp1);
		if (bp1->b_flags & B_ERROR) {
			printf("hd: write failure on ioctl\n");
			brelse(bp1);
			return(ENXIO);
		}
		brelse(bp1);
		break;
	case V_VERIFY:
		if (u.u_uid)
			return(ENXIO);
		bp1 = geteblk(PAGESIZ);
		bp1->b_flags = B_READ | B_MD1;
		bp1->b_blkno = ((union vfy_io *)arg)->vfy_in.abs_sec;
		bp1->b_dev = WHOLE_DISK(unit);	/* C partition for RDABS */
		xcount = ((union vfy_io *)arg)->vfy_in.num_sec;
		((union vfy_io *)arg)->vfy_out.err_code = 0;
		snum = PAGESIZ >> 9;
		while (xcount > 0) {
			i = (xcount > snum) ? snum : xcount;
			bp1->b_bcount = i << 9;
			hdstrategy(bp1);
			biowait(bp1);
			if (bp1->b_flags & B_ERROR) {
			       ((union vfy_io *)arg)->vfy_out.err_code=BAD_BLK;
			       break;
			}
			xcount -= i;
			bp1->b_blkno += i;
			bp1->b_flags &= ~B_DONE;
		}
		brelse(bp1);
		break;
	default:
		return(EINVAL);
	}
	return(0);
}
#endif	MACH_KERNEL

hdstrategy(bp)
struct buf *bp;
{
	u_char			unit = UNIT(bp->b_dev),
				ctrl = unit>>1;
	struct disklabel	*lp = &label[unit];
	struct partition	*part_p = &lp->d_partitions[PARTITION(bp->b_dev)];
	spl_t			opri;

	if (!bp->b_bcount)
		goto done;
	/* if request is off the end or trying to write last block on out */
	if (bp->b_flags & B_MD1) {
		if (bp->b_blkno > lp->d_partitions[PART_DISK].p_offset +
		    lp->d_partitions[PART_DISK].p_size - 1) {
			bp->b_error = ENXIO;
			goto bad;
		}
	} else {
		if ((bp->b_blkno > part_p->p_size) ||
		    (bp->b_blkno==part_p->p_size && !(bp->b_flags & B_READ))) {
			bp->b_error = ENXIO;
			goto bad;
		}
		if (bp->b_blkno == part_p->p_size) {
			/* indicate (read) EOF by setting b_resid to b_bcount on last block */ 
			bp->b_resid = bp->b_bcount;
			goto done;
		}
	}
	bp->b_cylin = ((bp->b_flags&B_MD1 ? 0 : part_p->p_offset) + bp->b_blkno)
		/ (lp->d_nsectors * lp->d_ntracks);
	opri = spl5();
	disksort(&hdunit[unit], bp);
	if (!hh[ctrl].controller_busy)
		hdstart(ctrl);
	splx(opri);
	return;
bad:
	bp->b_flags |= B_ERROR;
done:
	iodone(bp);
	return;
}

/* hdstart() is called at spl5 */
hdstart(ctrl)
int ctrl;
{
	struct partition	*part_p;
	register struct buf	*bp;
	int			i;

	/* things should be quiet */
	if (i = need_set_controller[ctrl]) {
		if (i&1) set_controller(ctrl<<1);
		if (i&2) set_controller((ctrl<<1)||1);
		need_set_controller[ctrl] = 0;
	}
	if (bp = hdunit[hh[ctrl].curdrive^1].b_actf)
		hh[ctrl].curdrive^=1;
	else if (!(bp = hdunit[hh[ctrl].curdrive].b_actf))
		return;
	hh[ctrl].controller_busy = 1;
	hh[ctrl].blocktotal = (bp->b_bcount + 511) >> 9;
	part_p = &label[UNIT(bp->b_dev)].d_partitions[PARTITION(bp->b_dev)];
	/* see V_RDABS and V_WRABS in hdioctl() */
	if (bp->b_flags & B_MD1) {
		struct disklabel *lp = &label[hh[ctrl].curdrive];
		int end = lp->d_partitions[PART_DISK].p_offset +
			lp->d_partitions[PART_DISK].p_size - 1;
		hh[ctrl].physblock = bp->b_blkno;
		if ((bp->b_blkno + hh[ctrl].blocktotal) > end)
			hh[ctrl].blocktotal = end - bp->b_blkno + 1;
	} else {
 		hh[ctrl].physblock = part_p->p_offset + bp->b_blkno;
		if ((bp->b_blkno + hh[ctrl].blocktotal) > part_p->p_size)
			hh[ctrl].blocktotal = part_p->p_size - bp->b_blkno + 1;
	}
	hh[ctrl].blockcount = 0;
	hh[ctrl].rw_addr = (int)bp->b_un.b_addr;
	hh[ctrl].retry_count = 0;
	start_rw(bp->b_flags & B_READ, ctrl);
}

int hd_print_error = 0;
hdintr(ctrl)
int ctrl;
{
	register struct buf	*bp;
	int			addr = devaddr[ctrl],
				unit = hh[ctrl].curdrive;
	u_char status;

	if (!hh[ctrl].controller_busy)
		return;
	waitcontroller(ctrl);
	status = inb(PORT_STATUS(addr));
	bp = hdunit[unit].b_actf;
	if (hh[ctrl].restore_request) { /* Restore command has completed */
		hh[ctrl].restore_request = 0;
		if (status & STAT_ERROR)
			hderror(bp,ctrl);
		else if (bp)
			start_rw(bp->b_flags & B_READ,ctrl);
		return;
	}
	if (status & STAT_WRITEFAULT) {
		if (hd_print_error) {
			 printf("hdintr: write fault. block %d, count %d, total %d\n",
			 	hh[ctrl].physblock, hh[ctrl].blockcount,
				hh[ctrl].blocktotal);
			 printf("hdintr: write fault. cyl %d, head %d, sector %d\n",
			 	hh[ctrl].cylinder, hh[ctrl].head,
				hh[ctrl].sector);

		}
		panic("hd: write fault\n");
	}
	if (status & STAT_ERROR) {
		if (hd_print_error) {
			 printf("hdintr: state error %x, error = %x\n",
			 	status, inb(PORT_ERROR(addr)));
			 printf("hdintr: state error. block %d, count %d, total %d\n",
			 	hh[ctrl].physblock, hh[ctrl].blockcount,
				hh[ctrl].blocktotal);
			 printf("hdintr: state error. cyl %d, head %d, sector %d\n",
			 	hh[ctrl].cylinder, hh[ctrl].head,
				hh[ctrl].sector);

		}
		hderror(bp,ctrl);
		return;
	}
	if (status & STAT_ECC) 
		printf("hd: ECC soft error fixed, unit %d, partition %d, physical block %d \n",
			unit, PARTITION(bp->b_dev), hh[ctrl].physblock);
	if (!bp) {
		/* there should be a read/write buffer queued at this point */
		printf("hd%d: no bp buffer to read or write\n",unit);
		return;
	}
	if (bp->b_flags & B_READ) {
		while (!(inb(PORT_STATUS(addr)) & STAT_DATAREQUEST));
		linw(PORT_DATA(addr), hh[ctrl].rw_addr, SECSIZE/2); 
	}
	if (++hh[ctrl].blockcount == hh[ctrl].blocktotal) {
		hdunit[unit].b_actf = bp->av_forw;
		bp->b_resid = 0;
		iodone(bp);
		hh[ctrl].controller_busy = 0;
		hdstart(ctrl);
	} else {
		hh[ctrl].rw_addr += SECSIZE;
		hh[ctrl].physblock++;
		if (hh[ctrl].single_mode)
			start_rw(bp->b_flags & B_READ,ctrl);
		else if (!(bp->b_flags & B_READ)) {
			/* Load sector into controller for next write */
			while (!(inb(PORT_STATUS(addr)) & STAT_DATAREQUEST));
			loutw(PORT_DATA(addr), hh[ctrl].rw_addr, SECSIZE/2);
		}
	}
}

hderror(bp,ctrl)
struct buf *bp;
int ctrl;
{
	int	addr = devaddr[ctrl],
		unit = hh[ctrl].curdrive;

	if(++hh[ctrl].retry_count > 3) {
		if(bp) {
			int i;
			/****************************************************
			* We have a problem with this block, set the error  *
			* flag, terminate the operation and move on to the  *
			* next request.  With every hard disk transaction   *
			* error we set the reset requested flag so that the *
			* controller is reset before the next operation is  *
			* started.					    *
			****************************************************/
			hdunit[unit].b_actf = bp->av_forw;
			bp->b_flags |= B_ERROR;
			bp->b_resid = 0;
			iodone(bp);
			outb(FIXED_DISK_REG(ctrl), 4);
			for(i = 0; i < 10000; i++);
			outb(FIXED_DISK_REG(ctrl), 0);
			setcontroller(unit);
			hh[ctrl].controller_busy = 0;
			hdstart(ctrl);
		}
	}
	else {
		/* lets do a recalibration */
		waitcontroller(ctrl);
		hh[ctrl].restore_request = 1;
		outb(PORT_PRECOMP(addr), dosparm[unit].precomp>>2);
		outb(PORT_NSECTOR(addr), label[unit].d_nsectors);
		outb(PORT_SECTOR(addr), hh[ctrl].sector);
		outb(PORT_CYLINDERLOWBYTE(addr), hh[ctrl].cylinder & 0xff);
		outb(PORT_CYLINDERHIBYTE(addr), (hh[ctrl].cylinder>>8) & 0xff);
		outb(PORT_DRIVE_HEADREGISTER(addr), (unit&1)<<4);
		outb(PORT_COMMAND(addr), CMD_RESTORE);
	}
}

getvtoc(unit)
{
	hdreadlabel(unit, 1);
}

setcontroller(unit)
{
	need_set_controller[unit>>1] |= (1<<(unit&1));
}

set_controller(unit)
{
	int			ctrl = unit >> 1;
	int			addr = devaddr[ctrl];
	struct disklabel	*lp = &label[unit];

	waitcontroller(ctrl);
	outb(PORT_DRIVE_HEADREGISTER(addr), (lp->d_ntracks - 1) |
	     ((unit&1) << 4) | FIXEDBITS);
	outb(PORT_NSECTOR(addr), lp->d_nsectors);
	outb(PORT_COMMAND(addr), CMD_SETPARAMETERS);
	waitcontroller(ctrl);
}

waitcontroller(ctrl)
{
	u_int	n = PATIENCE;

	while (--n && inb(PORT_STATUS(devaddr[ctrl])) & STAT_BUSY);
	if (n)
		return;
	panic("hd%d: waitcontroller() timed out",ctrl);
}

start_rw(read, ctrl)
     int read, ctrl;
{
	int			addr = devaddr[ctrl],
				unit = hh[ctrl].curdrive;
	struct disklabel	*lp = &label[unit];
	u_int			track, disk_block, xblk;

	disk_block = hh[ctrl].physblock;
	xblk=hh[ctrl].blocktotal - hh[ctrl].blockcount; /*# blks to transfer*/
	if (!(hdunit[unit].b_actf->b_flags & B_MD1) &&
	    (hh[ctrl].single_mode = xfermode(ctrl))) {
		xblk = 1;
		if (PARTITION(hdunit[unit].b_actf->b_dev) != PART_DISK)
			disk_block = badblock_mapping(ctrl);
	}
	/* disk is formatted starting sector 1, not sector 0 */
	hh[ctrl].sector = (disk_block % lp->d_nsectors) + 1;
	track = disk_block / lp->d_nsectors;
	hh[ctrl].head = track % lp->d_ntracks | 
		(unit&1) << 4 | FIXEDBITS;
	hh[ctrl].cylinder = track / lp->d_ntracks;
	waitcontroller(ctrl);
	outb(PORT_PRECOMP(addr), dosparm[unit].precomp >>2);
	outb(PORT_NSECTOR(addr), xblk);
	outb(PORT_SECTOR(addr), hh[ctrl].sector);
	outb(PORT_CYLINDERLOWBYTE(addr), hh[ctrl].cylinder & 0xff );
	outb(PORT_CYLINDERHIBYTE(addr),  (hh[ctrl].cylinder >> 8) & 0xff );
	outb(PORT_DRIVE_HEADREGISTER(addr), hh[ctrl].head);
	if(read) {
		outb(PORT_COMMAND(addr), CMD_READ);
	} else {
 		outb(PORT_COMMAND(addr), CMD_WRITE);
		waitcontroller(ctrl);
		while (!(inb(PORT_STATUS(addr)) & STAT_DATAREQUEST));
		loutw(PORT_DATA(addr), hh[ctrl].rw_addr, SECSIZE/2);
	}
}

int badblock_mapping(ctrl)
int ctrl;
{
	u_short	n;
	u_int	track,
		unit = hh[ctrl].curdrive,
		block = hh[ctrl].physblock,
		nsec = label[unit].d_nsectors;

	track = block / nsec;
	for (n = 0; n < alt_info[unit].alt_trk.alt_used; n++)
		if (track == alt_info[unit].alt_trk.alt_bad[n])
			return alt_info[unit].alt_trk.alt_base +
			       nsec * n + (block % nsec);
	/* BAD BLOCK MAPPING */
	for (n = 0; n < alt_info[unit].alt_sec.alt_used; n++)
		if (block == alt_info[unit].alt_sec.alt_bad[n])
			return alt_info[unit].alt_sec.alt_base + n;
	return block;
}

/*
 *  determine single block or multiple blocks transfer mode
 */
int xfermode(ctrl)
int ctrl;
{
	int	n, bblk, eblk, btrk, etrk;
	int	unit = hh[ctrl].curdrive;

	bblk = hh[ctrl].physblock;
	eblk = bblk + hh[ctrl].blocktotal - 1;
	btrk = bblk / label[unit].d_nsectors;
	etrk = eblk / label[unit].d_nsectors;
	
	for (n = 0; n < alt_info[unit].alt_trk.alt_used; n++)
		if ((btrk <= alt_info[unit].alt_trk.alt_bad[n]) &&
		    (etrk >= alt_info[unit].alt_trk.alt_bad[n]))
			return 1;
	for (n = 0; n < alt_info[unit].alt_sec.alt_used; n++)
		if ((bblk <= alt_info[unit].alt_sec.alt_bad[n]) &&
		    (eblk >= alt_info[unit].alt_sec.alt_bad[n]))
			return 1;
	return 0;
}

hdsize()
{
	printf("hdsize()		-- not implemented\n");
}

hddump()
{
	printf("hddump()		-- not implemented\n");
}

#ifdef	MACH_KERNEL
/*
 *	Routine to return information to kernel.
 */
int
hddevinfo(dev, flavor, info)
dev_t	dev;
int	flavor;
char	*info;
{
	register int	result;

	result = D_SUCCESS;

	switch (flavor) {
	case D_INFO_BLOCK_SIZE:
		*((int *) info) = SECSIZE;
		break;
	default:
		result = D_INVALID_OPERATION;
	}

	return(result);
}
#endif	MACH_KERNEL
