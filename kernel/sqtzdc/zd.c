/* 
 * Mach Operating System
 * Copyright (c) 1992-1 Carnegie Mellon University
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
 * $Log:	zd.c,v $
 * Revision 2.7  93/03/11  14:05:45  danner
 * 	u_long -> u_int
 * 	[93/03/10            danner]
 * 
 * Revision 2.6  92/08/03  18:16:35  jfriedl
 * 	Add "zd_devinfo" routine.
 * 	[92/08/03  14:36:10  jms]
 * 
 * Revision 2.5  92/02/24  09:58:46  elf
 * 	Removed gratuitous reference to undefined variable in zdgetstat.
 * 	[92/02/24            elf]
 * 
 * Revision 2.4  92/02/23  22:45:21  elf
 * 	Added zdgetstat().
 * 	[92/02/22  19:52:59  af]
 * 
 * Revision 2.3  91/07/31  18:08:03  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:07:55  dbg
 * 	Changes for new kmem_alloc interface.
 * 
 * 	Adapted for pure Mach kernel.
 * 	[91/03/22            dbg]
 * 
 */

/*
 * zd.c
 *
 * ZDC SMD Disk Driver
 */

#ifdef	MACH_KERNEL
#include <vm/vm_kern.h>

#include <sys/types.h>
#include <sys/time.h>

#include <device/buf.h>
#include <device/errno.h>
#include <device/param.h>

#include <sqt/macros.h>
#include <sqt/vm_defs.h>
#include <sqt/slicreg.h>
#include <sqt/clkarb.h>

#include <sqt/mutex.h>
#include <sqt/intctl.h>

#include <sqtzdc/zdc.h>
#include <sqtzdc/zdbad.h>
#include <sqtzdc/ioconf.h>

#define	PZERO		(0)		/* unused */
#define	FWRITE		D_WRITE
#define	EACCES		D_ALREADY_OPEN

#else	MACH_KERNEL
#include "sys/param.h"
#include "sys/file.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/systm.h"
#include "sys/uio.h"
#include "sys/ioctl.h"
#include "sys/proc.h"
#include "sys/kernel.h"
#include "sys/dk.h"
#include "sys/vm.h"

#include "sqt/slicreg.h"
#include "sqt/clkarb.h"

#include "sqt/mutex.h"
#include "sqt/intctl.h"
#include "sqt/pte.h"
#include "sqt/mftpr.h"
#include "sqt/plocal.h"

#include "sqtzdc/zdc.h"
#include "sqtzdc/zdbad.h"
#include "sqtzdc/ioconf.h"
#endif	MACH_KERNEL

#define	ZDUNIT(dev)	(minor(dev) >> 0x3)
#define	ZDPART(dev)	(minor(dev) & 0x7)
#define PARTCHR(dev)	('a' + ZDPART(dev))
#define	NUDGE_ZDC(ctlp, cbp, s)	{ \
	(s) = splhi(); \
	mIntr((ctlp)->zdc_slicaddr, CBBIN, (u_char)((cbp) - (ctlp)->zdc_cbp)); \
	splx((s)); \
	}
#define	PZOPEN		(PZERO - 1)		/* not signallable */
#define	b_diskaddr	b_resid
#define	b_psect		b_error
#define	B_IOCTL		B_MD1
/*
 * PTECLOFF returns offset in cluster of memory pointed at by pte.
 */
#define	PTECLOFF(pte)	((unsigned)((*(int*)(&(pte))) & ((~(NBPG-1))&CLOFSET)))

extern	struct	zdc_ctlr *zdctrlr;	/* zdctrlr array */
extern	struct	zd_unit	 *zdunit;	/* zdunit array */
extern	struct	zdsize	 *zdparts[];	/* Partition tables */
extern	int	zdntypes;		/* known drive types */
extern	int	zdc_iovpercb;		/* no of iovecs per cb */
extern	int	zdc_AB_throttle;	/* Channel A&B DMA throttle */
extern	short	zdcretry;		/* retry count on errors */
extern	u_char	zdctrl;			/* additional icb_ctrl bits */
extern	u_char	base_cb_intr;		/* base interrupt for zdc driver */
extern	u_char	base_err_intr;		/* base controller interrupt */
extern	simple_lock_data_t
		zdcprlock;		/* coordinate error printfs */

#define	DEBUG 1

#ifdef	DEBUG
int	zddebug = 0;
#endif	DEBUG

caddr_t	zd_compcodes[] = {
	"Command in progress",
	"Successful completion",
	"Write protect fault",
	"Drive Fault",
	"Seek error",
	"Seek timeout",
	"Channel timeout",
	"DMA timeout",
	"Header ECC error",
	"Soft ECC error",
	"Correctable ECC error",
	"Uncorrectable ECC error",
	"Sector not found",
	"Bad data sector",
	"Sector overrun",
	"No data synch",
	"Fifo data lost",
	"Illegal cb_cmd",
	"Illegal cb_mod",
	"Illegal disk address",
	"cb_addr not 16-byte aligned",
	"Illegal cb_count",
	"cb_iovec not 32-byte aligned",
	"Non-zero cb_iovec and page size invalid",
	"Illegal icb_pagesize",
	"icb_dumpaddr not 16-byte aligned",
	"Bad drive",
	"In-use CB reused",
	"Access error during DMA",
	"Channel not configured",
	"Channel was reset",
	"Unexpected status from DDC",
	"Unknown Completion code"
};
int	zdncompcodes = sizeof(zd_compcodes) / sizeof(zd_compcodes[0]);

#ifdef	MACH
static struct buf	zdbuf;		/* needed for physio */
#endif

/*
 * zdopen
 *
 * Return:
 *	0 - success.
 *	EACCES - open for write but drive write-protected.
 *	ENXIO - all other failures.
 */
zdopen(dev, mode)
	dev_t	dev;
	int	mode;
{
	register struct zd_unit *up;
	register struct zdcdd	*dd;		/* channel configuration */
	register struct	zdc_dev *zdv;		/* config data */
	int	retval;				/* return value */
	u_char	oldcfg;				/* previous cfg state */
	struct	zdcdd	*zdget_chancfg();

	up = &zdunit[ZDUNIT(dev)];

#ifdef	DEBUG
	if (zddebug)
		printf("O");
#endif	DEBUG

	/*
	 * Fail open if the unit is bad or if the unit was not bound
	 * to a drive during configuration.
	 */
	if (ZDUNIT(dev) >= zdc_conf->zc_nent ||
	    up->zu_state == ZU_NOTFOUND || up->zu_state == ZU_BAD)
		return (ENXIO);

	/*
	 * Now get serious
	 */
	p_sema(&up->zu_ocsema, PZOPEN);

	(void)p_lock(&up->zu_lock, SPLZD);
	/*
	 * check to see if still good drive.
	 */
	if (up->zu_state == ZU_BAD) {
		v_lock(&up->zu_lock, SPL0);
		v_sema(&up->zu_ocsema);
		return (ENXIO);
	}
	up->zu_nopen++;
	retval = 0;
	if (up->zu_nopen > 1) {
		/*
		 * Already opened at least once.
		 */
		if (up->zu_state == ZU_NO_RW) {
			/*
			 * Only one formatter at a time please...
			 */
			retval = EACCES;
		} else if (zdparts[up->zu_drive_type][ZDPART(dev)].p_length == 0) {
			/*
			 * Good drive but fail open if partition table is
			 * invalid.
			 */
			retval = ENXIO;
		} else if ((mode & FWRITE) && (up->zu_cfg & ZD_READONLY)) {
			/*
			 * Fail open for write on a write-protected drive.
			 */
			retval = EACCES;
		}
		v_lock(&up->zu_lock, SPL0);
		if (retval != 0)
			--up->zu_nopen;
		v_sema(&up->zu_ocsema);
		return (retval);
	}

	/*
	 * First open!
	 * Drop lock, we don't need as no other I/O can be in progress
	 * on this drive.
	 */
	v_lock(&up->zu_lock, SPL0);

	up->zu_state = ZU_GOOD;		/* assume good */
	oldcfg = up->zu_cfg;
	if ((retval = zdprobe_drive(dev, up)) != 0) {
		/*
		 * Error whilst attempting probe!
		 */
		goto out;
	}
	/*
	 * Error if drive still not there or not online.
	 */
	if (up->zu_cfg == ZD_NOTFOUND ||		/* Not there */
	    (up->zu_cfg & ZD_ONLINE) != ZD_ONLINE) {	/* Offline */
		if ((oldcfg & ZD_ONLINE) == ZD_ONLINE) {
			/*
			 * It had been online.
			 */
			disk_offline();
		}
		retval = ENXIO;
		goto out;
	}

	if (oldcfg == ZD_NOTFOUND || ((oldcfg & ZD_ONLINE) != ZD_ONLINE)) {
		/*
		 * It had been offline.
		 */
		disk_online();
	}

	if ((mode & FWRITE) && (up->zu_cfg & ZD_READONLY)) {
		/*
		 * Fail open for write on a write-protected drive.
		 */
		retval = EACCES;
		goto out;
	}

	dd = zdget_chancfg(dev, up);
	if (dd == NULL) {
		retval = EIO;
		goto out;
	}
	zdv = &zdc_conf->zc_dev[ZDUNIT(dev)];
	/*
	 * Does channel configuration match the drive configuration.
	 * If bound drive_type and mismatch, then drive is marked to
	 * disallow normal read/write operations.
	 */
	if (dd->zdd_sectors == 0 ||
	    (zdv->zdv_drive_type != ANY &&
	     zdv->zdv_drive_type != dd->zdd_drive_type)) {
		up->zu_state = ZU_NO_RW;
		printf("zd%d: drive type mismatch - check configuration.\n",
			up - zdunit);
		goto out;
	}
	/*
	 * If the drive is not formatted or is formatted differently than
	 * the other drives on the channel, then cannot read the bad block
	 * list. In this case, only format operations via ioctl will be
	 * allowed. Read and write operations will return error.
	 */
	if ((up->zu_cfg & (ZD_FORMATTED|ZD_MATCH)) != (ZD_FORMATTED|ZD_MATCH)) {
		up->zu_state = ZU_NO_RW;
		printf("zd%d: warning: %s.\n", up - zdunit,
			((up->zu_cfg & ZD_FORMATTED) != ZD_FORMATTED)
				? "drive unformatted"
				: "drive/channel mismatch");
		goto out;
	}
	if (dd->zdd_drive_type >= zdntypes) {
		/*
		 * Unknown drive type. Check zdparts[] in binary conf file.
		 */
		up->zu_state = ZU_NO_RW;	/* allow reformat */
		printf("zd%d: unknown drive type - check configuration.\n",
			up - zdunit);
		goto out;
	}

	up->zu_drive_type = dd->zdd_drive_type;
	if (zdparts[up->zu_drive_type][ZDPART(dev)].p_length == 0) {
		/*
		 * Good drive but fail open if partition table is invalid.
		 */
		retval = ENXIO;
		goto out;
	}

	/*
	 * Online and formatted drive - get bad block list.
	 * If cannot correctly read bad block list - only allow ioctl
	 * operations.
	 */
	if (zdgetbad(dev, up) < 0) {
		printf("zd%d: Cannot read bad block list.\n", up - zdunit);
		up->zu_state = ZU_NO_RW;
	}

out:
	if (retval != 0)
		--up->zu_nopen;
	v_sema(&up->zu_ocsema);
	return (retval);
}

/*
 * zdclose
 *	Close the device.
 *	If last close, free memory holding bad block list.
 */
/*ARGSUSED*/
zdclose(dev, flag)
	dev_t dev;
	int flag;
{
	register struct zd_unit *up;
	register int size;

	up = &zdunit[ZDUNIT(dev)];
#ifdef	DEBUG
	if (zddebug)
		printf("C");
#endif	DEBUG
	p_sema(&up->zu_ocsema, PZOPEN);
	if (--up->zu_nopen == 0) {
		/*
		 * Last close!
		 * Free memory allocated to hold bad block list.
		 * If drive has removable media, the next open may
		 * have different bad block list.
		 */
		if (up->zu_zdbad != NULL) {
			size = (up->zu_zdbad->bz_nsnf * sizeof(struct bz_bad))
				+ sizeof(struct zdbad) - sizeof(struct bz_bad);
			size = roundup(size, DEV_BSIZE);
			kmem_free(kernel_map, (vm_offset_t)up->zu_zdbad, size);
			up->zu_zdbad = NULL;
		}
	}
	v_sema(&up->zu_ocsema);
}

/*
 * zdprobe_drive
 *	Probe for the status of a particular drive then update
 *	appropriate fields in controller and unit structures.
 */
int
zdprobe_drive(dev, up)
	dev_t	dev;
	register struct zd_unit *up;
{
	register struct buf *bp;	/* ioctl buffer */
	register struct cb  *cbp;	/* cb argument */
	int	error;

	bp  = &up->zu_ioctl;
	cbp = &up->zu_ioctlcb;
#ifndef	MACH
	bufalloc(bp);			/* will always get since 1st open */
#endif	MACH
	bp->b_flags = B_IOCTL;
	bp->b_bcount = 0;		/* data in CB */
	bp->b_un.b_addr = NULL;
	bp->b_dev = dev;
	bp->b_blkno = 0;
	bp->b_error = 0;
#ifndef	MACH_KERNEL
	bp->b_proc = u.u_procp;
#endif	MACH_KERNEL
#ifndef	MACH
	bp->b_iotype = B_FILIO;
	BIODONE(bp) = 0;
#endif	MACH
	/* Fill out CB */
	cbp->cb_cmd = ZDC_PROBEDRIVE;
	/*
	 * Insert at head of list and zdstart!
	 */
	(void)p_lock(&up->zu_lock, SPLZD);
	bp->av_forw = NULL;
	up->zu_bhead.av_forw = bp;
	zdstart(up);
	v_lock(&up->zu_lock, SPL0);
	biowait(bp);
#ifdef	MACH_KERNEL
	error = bp->b_error;
#else	MACH_KERNEL
	error = geterror(bp);
#endif	MACH_KERNEL
	if (error) {
#ifndef	MACH
		buffree(bp);
#endif	MACH
		return (error);
	}
	/*
	 * extract drive cfg data.
	 */
	up->zu_cfg = ((struct probe_cb *)cbp)->pcb_drivecfg[up->zu_drive];
	zdctrlr[up->zu_ctrlr].zdc_drivecfg[up->zu_drive] = up->zu_cfg;
#ifndef	MACH
	buffree(bp);
#endif	MACH
	return (0);
}

/*
 * zdget_chancfg
 *	Get the channel configuration for the channel on which this
 *	drive resides. Fills in channel configuration in controller structure.
 */
struct zdcdd *
zdget_chancfg(dev, up)
	dev_t	dev;
	register struct zd_unit *up;
{
	register struct buf	*bp;	/* ioctl buffer */
	register struct cb	*cbp;	/* cb argument */
	struct zdcdd	*dd;		/* channel configuration data */
	int	chan;			/* Channel A (0) or channel B (1) */

	bp  = &up->zu_ioctl;
	cbp = &up->zu_ioctlcb;
#ifndef	MACH
	bufalloc(bp);			/* will always get since 1st open */
#endif	MACH
	if (kmem_alloc(kernel_map,
		       (vm_offset_t *)&bp->b_un.b_addr,
		       sizeof(struct zdcdd)) != KERN_SUCCESS)
	    panic("zdget_chancfg");
	bzero(bp->b_un.b_addr, sizeof(struct zdcdd));
	bp->b_flags = B_READ | B_IOCTL;
	bp->b_bcount = sizeof(struct zdcdd);
	bp->b_dev = dev;
	bp->b_blkno = 0;
	bp->b_error = 0;
#ifndef	MACH_KERNEL
	bp->b_proc = u.u_procp;
#endif	MACH_KERNEL
#ifndef	MACH
	bp->b_iotype = B_FILIO;
	BIODONE(bp) = 0;
#endif	MACH
	/* Fill out CB */
	cbp->cb_cmd = ZDC_GET_CHANCFG;
	cbp->cb_addr = KVTOPHYS(bp->b_un.b_addr, u_int);
	cbp->cb_count = sizeof(struct zdcdd);
	/*
	 * Insert at head of list and zdstart!
	 */
	(void)p_lock(&up->zu_lock, SPLZD);
	bp->av_forw = NULL;
	up->zu_bhead.av_forw = bp;
	zdstart(up);
	v_lock(&up->zu_lock, SPL0);
	biowait(bp);
	if (bp->b_flags & B_ERROR) {
		kmem_free(kernel_map, bp->b_un.b_addr, sizeof(struct zdcdd));
#ifndef	MACH
		buffree(bp);
#endif	MACH
		return (NULL);
	}
	/*
	 * Extract Channel configuration data.
	 */
	chan = up->zu_drive & 1;
	dd = (chan) ? &zdctrlr[up->zu_ctrlr].zdc_chanB
		    : &zdctrlr[up->zu_ctrlr].zdc_chanA;
	bcopy(bp->b_un.b_addr, (caddr_t)dd, sizeof(struct zdcdd));
	kmem_free(kernel_map, bp->b_un.b_addr, sizeof(struct zdcdd));
#ifndef	MACH
	buffree(bp);
#endif	MACH
	/*
	 * On the first get of the channel configuration set the
	 * dma throttle for the channel. Make sure that the configuration
	 * is valid.
	 */
	if (dd->zdd_sectors != 0 &&
	    (zdctrlr[up->zu_ctrlr].zdc_dma_throttle & (1 << chan)) == 0)
		set_dma_throttle(&zdctrlr[up->zu_ctrlr], dd, chan);
	return (dd);
}

/*
 * set_dma_throttle
 *	If 1st open on channel, set the dma throttle
 */
/*ARGSUSED*/
set_dma_throttle(ctlrp, dd, chan)
	register struct zdc_ctlr *ctlrp;	/* controller */
	struct zdcdd	*dd;			/* channel configuration data */
	int	chan;				/* channel A (0) or B (1) */
{
	register int	throttle;	/* throttle count */
	spl_t	s_ipl;

	s_ipl = p_lock(&ctlrp->zdc_ctlrlock, SPLHI);
	if ((ctlrp->zdc_dma_throttle & (1 << chan)) == 0) {
		throttle = zdc_AB_throttle;
		if (throttle > SLB_TVAL)
			throttle = SLB_TVAL;
		wrslave(ctlrp->zdc_slicaddr, (u_char)(SL_G_CHAN0 + chan),
					(u_char)(SLB_TH_ENB | throttle));
		ctlrp->zdc_dma_throttle |= (1 << chan);
	}
	v_lock(&ctlrp->zdc_ctlrlock, s_ipl);
}

/*
 * getchksum
 *	Calculate bad block list checksum
 */
static
getchksum(lptr, nelem, seed)
	register long *lptr;
	register int nelem;
	long seed;
{
	register long sum;

	sum = seed;
	while (nelem-- > 0) {
		sum ^= *lptr;
		++lptr; 
	}
	return (sum);
}

/*
 * zdgetbad
 *	- get the bad block list for this unit.
 * Return:
 *	 0 - success
 *	-1 - failure
 */
int
zdgetbad(dev, up)
	dev_t	dev;
	struct zd_unit *up;
{
	register struct cb	*cbp;
	register struct	bz_bad	*fbzp, *tbzp;
	register struct buf	*bp;		/* ioctl buffer */
	register int	size;
	struct	zdbad	*zdp;			/* bad block list */
	struct	zdcdd	*dd;			/* disk description */
	int	zdpsize;
	int	block;
	caddr_t	addr;
	static struct	zdbad	null_bbl;	/* fake bad block list */

	/*
	 * Initialize a fake bad block list in case we receive a
	 * SNF failure whilst reading the bad block list!
	 */
	null_bbl.bz_nelem = 0;
	null_bbl.bz_nsnf = 0;
	up->zu_zdbad = &null_bbl;

	dd = (up->zu_drive & 1) ? &zdctrlr[up->zu_ctrlr].zdc_chanB
				: &zdctrlr[up->zu_ctrlr].zdc_chanA;
	size = 1;				/* get 1 for starters */
	if (kmem_alloc(kernel_map,
		       (vm_offset_t *)&zdp,
		       DEV_BSIZE) != KERN_SUCCESS)
	    panic("zdgetbad");

	/*
	 * Read bad block list.
	 */
	bp = &up->zu_ioctl;
#ifndef	MACH
	bufalloc(bp);
#endif	MACH
	bp->b_dev = dev;
	bp->b_blkno = 0;
#ifndef	MACH_KERNEL
	bp->b_proc = u.u_procp;
#endif	MACH_KERNEL
#ifndef	MACH
	bp->b_iotype = B_FILIO;
#endif	MACH
	cbp = &up->zu_ioctlcb;
	cbp->cb_mod = 0;
	cbp->cb_cmd = ZDC_READ;
	cbp->cb_cyl = ZDD_DDCYL;
	addr = (caddr_t)zdp;
	for (block = 0; block < size; block++) {
		cbp->cb_head = 0;
		cbp->cb_sect = ZDD_NDDSECTORS + block;
		while (cbp->cb_head < MIN(dd->zdd_tracks, BZ_NBADCOPY)) {
			bp->b_bcount = DEV_BSIZE;
			bp->b_flags = B_READ | B_IOCTL;
			bp->b_un.b_addr = addr;
			bp->b_error = 0;
#ifndef	MACH
			BIODONE(bp) = 0;
#endif	MACH
			cbp->cb_count = DEV_BSIZE;
			cbp->cb_addr = KVTOPHYS(addr, u_int);
			/*
			 * Queue and start I/O
			 */
			(void)p_lock(&up->zu_lock, SPLZD);
			bp->av_forw = NULL;
			up->zu_bhead.av_forw = bp;
			zdstart(up);
			v_lock(&up->zu_lock, SPL0);
			biowait(bp);
			if ((bp->b_flags & B_ERROR) != B_ERROR)
				break;
			/*
			 * If could not read - try next track.
			 */
			cbp->cb_head++;
			cbp->cb_sect = block;
		}

		/*
		 * If cannot read the first block of bad block list.
		 * give up and return.
		 */
		if (bp->b_flags & B_ERROR) {
			printf("zd%d: Cannot read block %d of bad block list.\n",
					up - zdunit, block);
			if (block == 0)
			    kmem_free(kernel_map, (vm_offset_t)zdp, DEV_BSIZE);
			else
			    kmem_free(kernel_map, (vm_offset_t)zdp,
						  dbtob(size));
#ifndef	MACH
			buffree(bp);
#endif	MACH
			up->zu_zdbad = (struct zdbad *)NULL;
			return (-1);
		}
		if (block == 0) {
			size = (zdp->bz_nelem * sizeof(struct bz_bad))
				+ sizeof(struct zdbad) - sizeof(struct bz_bad);
			size = howmany(size, DEV_BSIZE);
			if (size > ((dd->zdd_sectors - ZDD_NDDSECTORS) >> 1)) {
				printf("zd%d: Bad block list corrupted!\n",
					up - zdunit);
				kmem_free(kernel_map, (vm_offset_t)zdp,
							DEV_BSIZE);
				up->zu_zdbad = (struct zdbad *)NULL;
				return (-1);
			}
			zdpsize = dbtob(size);
			/*
			 * copy block 0 to new location in free memory, so
			 * that bad block list will be contiguous.
			 */
			if (kmem_alloc(kernel_map,
				       (vm_offset_t *)&addr,
				       zdpsize) != KERN_SUCCESS)
			    panic("zdgetbad");
			bcopy((caddr_t)zdp, addr, (unsigned)DEV_BSIZE);
			kmem_free(kernel_map, (vm_offset_t)zdp, DEV_BSIZE);
			zdp = (struct zdbad *)addr;
		}
		addr += DEV_BSIZE;
	} /* end of for */ 

#ifndef	MACH
	/*
	 * done with I/O - free buf header.
	 */
	buffree(bp);
#endif	MACH

	/*
	 * Confirm data integrity via checksum.
	 */
	size = (zdp->bz_nelem * sizeof(struct bz_bad)) / sizeof(long);
	if (zdp->bz_csn != getchksum((long *)zdp->bz_bad, size,
				(long)(zdp->bz_nelem ^ zdp->bz_nsnf))) {
		printf("zd%d: Checksum failed!\n", up - zdunit);
		kmem_free(kernel_map, (vm_offset_t)zdp, zdpsize);
		up->zu_zdbad = (struct zdbad *)NULL;
		return (-1);
	}

	/*
	 * Copy only BZ_SNF entries into unit's bad block list.
	 * 
	 * Calculate size of needed bad block list.
	 * That is, only BZ_SNF entries are needed.
	 */
	size = (zdp->bz_nsnf * sizeof(struct bz_bad))
		+ sizeof(struct zdbad) - sizeof(struct bz_bad);
	size = roundup(size, DEV_BSIZE);
	if (kmem_alloc(kernel_map,
		       (vm_offset_t *)&up->zu_zdbad,
		       size) != KERN_SUCCESS)
	    panic("zdgetbad");
#ifdef	DEBUG
	bzero((caddr_t)up->zu_zdbad, (u_int)size);
#endif	DEBUG
	*up->zu_zdbad = *zdp;
	tbzp = up->zu_zdbad->bz_bad;
	for (fbzp = zdp->bz_bad; fbzp < &zdp->bz_bad[zdp->bz_nelem]; fbzp++) {
		if (fbzp->bz_rtype == BZ_SNF)
			*tbzp++ = *fbzp;
	}
	kmem_free(kernel_map, (vm_offset_t)zdp, zdpsize);
	return (0);
}

/*
 * zdstrat
 * 	zd disk read/write routine.
 * Perform various checks and queue request and call start routine to
 * initiate I/O to the device.
 */
zdstrat(bp)
	register struct buf *bp;
{
	register struct zd_unit *up;
	register struct zdcdd	*dd;		/* channel configuration */
	register int sector;
	register int nspc;
	struct	zdsize	*part;
	struct	diskaddr diskaddress;
	spl_t s_ipl;

#ifdef	DEBUG
	if (zddebug > 1)
		printf("zdstrat(%c): bp=%x, dev=%x, cnt=%d, blk=%x, vaddr=%x\n",
			(bp->b_flags & B_READ) ? 'R' : 'W',
			bp, bp->b_dev, bp->b_bcount, bp->b_blkno,
			bp->b_un.b_addr);
	else	if (zddebug)
			printf("%c", (bp->b_flags & B_READ) ? 'R' : 'W');
#endif	DEBUG

	up = &zdunit[ZDUNIT(bp->b_dev)];
	/*
	 * Error if NO_RW operations are permitted.
	 */
	if (up->zu_state == ZU_NO_RW) {
		bp->b_flags |= B_ERROR;
		bp->b_error = EIO;
		bp->b_resid = bp->b_bcount;
		biodone(bp);
		return;
	}

	part = &zdparts[up->zu_drive_type][ZDPART(bp->b_dev)];
	/*
	 * Size and partitioning check.
	 *
	 * Fail request if bogus byte count, if address not aligned to
	 * ADDRALIGN boundary, or if transfer is not entirely within a
	 * disk partition.
	 */
	if (bp->b_bcount <= 0
	||  ((bp->b_bcount & (DEV_BSIZE -1)) != 0)		/* size */
#ifdef	MACH_KERNEL
	||  (
#else	MACH_KERNEL
#ifndef	MACH
	||  ((bp->b_iotype == B_RAWIO) &&
#else	MACH
	||  ((bp->b_flags & B_PHYS) &&
#endif	MACH
#endif	MACH_KERNEL
	     (((int)bp->b_un.b_addr & (ADDRALIGN - 1)) != 0))	/* alignment */
	||  (bp->b_blkno < 0)					/* partition */
	||  ((bp->b_blkno + btodb(bp->b_bcount)) > part->p_length)) {
		bp->b_flags |= B_ERROR;
		bp->b_error = EINVAL;
		bp->b_resid = bp->b_bcount;
		biodone(bp);
		return;
	}

	dd = (up->zu_drive & 1)	? &zdctrlr[up->zu_ctrlr].zdc_chanB
				: &zdctrlr[up->zu_ctrlr].zdc_chanA;
	nspc = dd->zdd_sectors * dd->zdd_tracks;
	sector = bp->b_blkno;
#if 1
	{ /* gcc bug workaround */
	    int	temp;
	    u_short temp2;
	    temp = sector / nspc + part->p_cyloff;
	    temp2 = temp & 0xFFFF;
	    diskaddress.da_cyl = temp2;
	}
#else
	diskaddress.da_cyl = sector / nspc + part->p_cyloff;
#endif
	sector %= nspc;
	diskaddress.da_head = sector / dd->zdd_sectors;
	diskaddress.da_sect = sector % dd->zdd_sectors;
	bp->b_diskaddr = *(long *)&diskaddress;
	bp->b_psect = zdgetpsect(&diskaddress, dd);

	s_ipl = p_lock(&up->zu_lock, SPLZD);
	if (up->zu_state == ZU_BAD) {
		v_lock(&up->zu_lock, s_ipl);
		/*
		 * Controller/channel/Drive has gone bad!
		 */
		bp->b_flags |= B_ERROR;
		bp->b_error = EIO;
		bp->b_resid = bp->b_bcount;
		biodone(bp);
		return;
	}
	disksort(&up->zu_bhead, bp);
	zdstart(up);
	v_lock(&up->zu_lock, s_ipl);
}

/*
 * zdgetpsect
 *	Determine physical sector in track where the I/O  transfer is to
 *	occur. Used by ZDC firmware for RPS optimization.
 */
int
zdgetpsect(dp, dd)
	register struct	diskaddr *dp;
	register struct zdcdd	 *dd;
{
	if (dd->zdd_tskew == 1)
		return (( ((dd->zdd_tracks - 1 + dd->zdd_cskew) * dp->da_cyl)
				+ dp->da_head + dp->da_sect)
					 % (dd->zdd_sectors + dd->zdd_spare));
	/*
	 * track skew != 1
	 */
	return (( (((dd->zdd_tskew*(dd->zdd_tracks-1)) + dd->zdd_cskew)
								 * dp->da_cyl)
		  + (dd->zdd_tskew*dp->da_head) + dp->da_sect)
					% (dd->zdd_sectors + dd->zdd_spare));
}

/*
 * zdfill_iovec
 *	- fill out cb_iovec for the I/O request.
 *
 * B_RAWIO, B_PTEIO, B_PTBIO cases must flush TLB to avoid stale mappings
 * thru Usrptmap[], since this is callable from interrupt procedures (SGS only).
 *
 * Panics if bad pte found; "can't" happen.
 */

#ifndef	MACH

static u_int
zdfill_iovec(bp, iovstart)
#ifndef	i386
	register				/* want optimial on 032's */
#endif
	struct buf *bp;
	u_int *iovstart;
{
	register struct pte *pte;
	register int	count;
	register u_int	*iovp;
	u_int	retval;
	unsigned offset;
	extern struct pte *vtopte();

	/*
	 * Source/target pte's are found differently based on type
	 * of IO operation.
	 */
	switch(bp->b_iotype) {

	case B_RAWIO:					/* RAW IO */
		/*
		 * In this case, must look into alignment of physical
		 * memory, since we can start on any ADDRALIGN boundary.
		 */
		flush_tlb();
		pte = vtopte(bp->b_proc, clbase(btop(bp->b_un.b_addr)));
		count = (((int)bp->b_un.b_addr & CLOFSET) + bp->b_bcount
							 + CLOFSET) / CLBYTES;
		retval = (u_int)bp->b_un.b_addr;
		break;

	case B_FILIO:					/* file-sys IO */
		/*
		 * Filesys/buffer-cache IO.  These are always cluster aligned
		 * both physically and virtually.
		 * Note: also used when to kernel memory acquired via wmemall().
		 * For example, channel configuration buffer in zdget_chancfg().
		 */
		pte = &Sysmap[btop(bp->b_un.b_addr)];
		count = (bp->b_bcount + CLOFSET) / CLBYTES;
		retval = (u_int)bp->b_un.b_addr;
		break;

	case B_PTEIO:					/* swap/page IO */
		/*
		 * Pte-based IO -- already know pte of 1st page, which
		 * is cluster aligned, and b_count is a multiple of CLBYTES.
		 */
		flush_tlb();
		pte = bp->b_un.b_pte;
		count = (bp->b_bcount + CLOFSET) / CLBYTES;
		retval = PTETOPHYS(*pte);
		break;

	case B_PTBIO:					/* Page-Table IO */
		/*
		 * Page-Table IO: like B_PTEIO, but can start/end with
		 * non-cluster aligned memory (but is always HW page
		 * aligned). Count is multiple of NBPG.
		 *
		 * Separate case for greater efficiency in B_PTEIO.
		 */
		flush_tlb();
		pte = bp->b_un.b_pte;
		retval = PTETOPHYS(*pte);
		offset = PTECLOFF(*pte);
		pte -= btop(offset);
		count = (offset + bp->b_bcount + CLOFSET) / CLBYTES;
		break;

	default:
		panic("zdfill_iovec: bad b_iotype");
		/*NOTREACHED*/
	}

	/*
	 * Now translate PTEs and fill-in iovectors.
	 */
	iovp = iovstart;
	while (count--) {
		*iovp++ = PTETOPHYS(*pte);
		pte += CLSIZE;
	}
	return (retval);
}

#else	MACH

#ifdef	MACH_KERNEL
/*
 * pure Mach kernel version.
 */
static u_int
zdfill_iovec(bp, iovp)
	register struct buf *bp;
	register u_int	*iovp;
{
	register pt_entry_t *pte;
	register int	count;
	vm_offset_t	start, end;

	/*
	 * In this case, must look into alignment of physical
	 * memory, since we can start on any ADDRALIGN boundary.
	 *
	 * Since ptes mapping kernel address space are contiguous in
	 * kernel address space, can bump pte inside loop.
	 */

	start = i386_trunc_page((vm_offset_t)bp->b_un.b_addr);
	end   = i386_round_page((vm_offset_t)bp->b_un.b_addr + bp->b_bcount);
	pte   = pmap_pte(kernel_pmap, start);
	count = i386_btop(end - start);
	while (count--) {
		*iovp++ = pte_to_pa(*pte);
		pte++;
	}
	return((u_int)bp->b_un.b_addr);
}
#else	MACH_KERNEL
/*
 * MACH version.
 */
static u_int
zdfill_iovec(bp, iovp)
	register struct buf *bp;
	register u_int	*iovp;
{
	register struct pte *pte;
	register int	count;

	if (bp->b_flags & B_PHYS) {
		/*
		 * In this case, must look into alignment of physical
		 * memory, since we can start on any ADDRALIGN boundary.
		 *
		 * Since pte's mapping user task can be dis-contiguous in
		 * kernel address space, need to re-compute pte inside loop.
		 */
		vm_offset_t	addr;
		pmap_t		map;

		addr = ((vm_offset_t)bp->b_un.b_addr & ~CLOFSET) | VA_USER;
		map = vm_map_pmap(bp->b_proc->task->map);
		count = (((int)bp->b_un.b_addr & CLOFSET) + bp->b_bcount
							 + CLOFSET) / CLBYTES;
		while (count--) {
			pte = (struct pte *) pmap_pte(map, addr);
			*iovp++ = PTETOPHYS(*pte);
			addr += CLBYTES;
		}
	} else {
		/*
		 * Filesys/buffer-cache IO.  These are always cluster aligned
		 * both physically and virtually.
		 * Note: also used on kernel memory acquired via wmemall().
		 * For example, channel configuration buffer in zdget_chancfg().
		 *
		 * Since pte's mapping kernel address space are contiguous
		 * virtually, can bump pte inside loop.
		 */
		pte = &Sysmap[btop(bp->b_un.b_addr)];
		count = (bp->b_bcount + CLOFSET) / CLBYTES;
		while (count--) {
			*iovp++ = PTETOPHYS(*pte);
			pte += CLSIZE;
		}
	}
	return((u_int)bp->b_un.b_addr);
}
#endif	MACH_KERNEL
#endif	MACH

/*
 * zdstart
 *	- intitiate I/O request to controller.
 * If controller is busy just return. Otherwise stuff appropriate CB
 * with request and notify ZDC.
 *
 * Called with unit structure locked at SPLZD.
 */
zdstart(up)
	register struct zd_unit *up;
{
	register struct cb	*cbp;
	register struct buf	*bp;
	register struct zdc_ctlr *ctlrp;
	spl_t	s_ipl;

#ifdef	DEBUG
	if (zddebug)
		printf("S");
#endif	DEBUG
	bp = up->zu_bhead.av_forw;
	ctlrp = &zdctrlr[up->zu_ctrlr];
	cbp = up->zu_cbptr;
	if (cbp->cb_bp == NULL && cbp[1].cb_bp == NULL) {
		/*
		 * Drive is idle.
		 * If fp_lights - turn activity light on.
		 * Get starting time.
		 */
		if (fp_lights) {
			s_ipl = splhi();
			FP_IO_ACTIVE;
			splx(s_ipl);
		}
		up->zu_starttime = time;
	}
	if (cbp->cb_bp == NULL || cbp[1].cb_bp == NULL) {
		/*
		 * Fill CB.
		 */
		if (cbp->cb_bp)
			++cbp;
		if (bp->b_flags & B_IOCTL) {
			/*
			 * copy 1st half of cb (what fw will see)
			 */
			bcopy((caddr_t)&up->zu_ioctlcb, (caddr_t)cbp, FWCBSIZE);
			if (cbp->cb_cmd != ZDC_READ_LRAM
			&&  cbp->cb_cmd != ZDC_WRITE_LRAM)
				cbp->cb_addr = zdfill_iovec(bp, cbp->cb_iovstart);
		} else {
			*(long *)&cbp->cb_diskaddr = bp->b_diskaddr;
			cbp->cb_psect = (u_char)bp->b_psect;
			cbp->cb_count = bp->b_bcount;
			cbp->cb_mod = 0;
			cbp->cb_cmd = (bp->b_flags & B_READ) ? ZDC_READ : ZDC_WRITE;
			cbp->cb_addr = zdfill_iovec(bp, cbp->cb_iovstart);
#ifdef	MACH_KERNEL
			bp->b_error = 0;	/* was b_psect */
#endif	/* MACH_KERNEL */
		}
		bp->b_resid = bp->b_bcount;
		cbp->cb_transfrd = 0;
		cbp->cb_bp = bp;
		cbp->cb_errcnt = 0;
		cbp->cb_state = ZD_NORMAL;
		cbp->cb_iovec = KVTOPHYS(cbp->cb_iovstart, u_int *);
		/*
		 * Notify ZDC of job request.
		 */
#ifdef	DEBUG
		if (zddebug > 2)
			zddumpcb(cbp);
#endif	DEBUG
		NUDGE_ZDC(ctlrp, cbp, s_ipl);
		up->zu_bhead.av_forw = bp->av_forw;
		bp = bp->av_forw;
		cbp = up->zu_cbptr;
	}
}

/*
 * zdintr
 *	Normal request completion interrupt handler.
 */
zdintr(vector)
	u_char	vector;
{
	register struct cb	 *cbp;
	register struct	zd_unit  *up;
	register struct	buf	 *donebp;
	register struct	zdc_ctlr *ctlrp;
	struct	zdcdd *dd;
	int	zdcvec;
	daddr_t	blkno;
	int	i;
	int	partition;
	u_char	val;
	spl_t	s_ipl;

#ifdef	DEBUG
	if (zddebug)
		printf("I");
#endif	DEBUG
	zdcvec = vector - base_cb_intr;
	ctlrp = &zdctrlr[zdcvec >> NCBZDCSHFT];
	cbp = ctlrp->zdc_cbp + (zdcvec & (NCBPERZDC-1));
	up = &zdunit[cbp->cb_unit];
	if (cbp->cb_bp == NULL) {
		if (ctlrp->zdc_state == ZDC_DEAD) {
			printf("zdc%d drive %d: Spurious interrupt from dead controller.\n",
				ctlrp - zdctrlr,
				(cbp - ctlrp->zdc_cbp) / NCBPERDRIVE);
			return;
		}
		if (cbp->cb_unit < 0) {
			printf("zdc%d drive %d: Interrupt from unknown unit.\n",
				ctlrp - zdctrlr,
				(cbp - ctlrp->zdc_cbp) / NCBPERDRIVE);
			return;
		}
		printf("zd%d: Spurious interrupt.\n", cbp->cb_unit);
		return;
	}

	/*
	 * Separate normal case from error cases for performance.
	 */
	if (cbp->cb_compcode == ZDC_DONE) {
		if (cbp->cb_state == ZD_NORMAL) {
			donebp = cbp->cb_bp;
			donebp->b_resid = 0;
			goto donext;
		}
		if (cbp->cb_state & ZD_RESET) {
			/*
			 * Reset completed.
			 */
			cbp->cb_state &= ~ZD_RESET;
			if (cbp->cb_errcnt++ < zdcretry) {
				zdretry(cbp, up, ctlrp);
				return;
			}
			/*
			 * Fail I/O request.
			 */
			donebp = cbp->cb_bp;
			donebp->b_flags |= B_ERROR;
			donebp->b_error = EIO;
			goto donext;
		}
		if (cbp->cb_state & ZD_REVECTOR) {
			/*
			 * Completed revector. Continue with rest of transfer.
			 */
			zdcontinue(cbp, up, ctlrp);
			return;
		}
	}

	/*
	 * E R R O R  O C C U R R E D !
	 */

	/*
	 * If not doing retry reset or revectoring, update b_resid.
	 */
	if (cbp->cb_state == ZD_NORMAL)
		cbp->cb_bp->b_resid = cbp->cb_count;

	blkno = btodb(cbp->cb_bp->b_bcount - cbp->cb_bp->b_resid)
		+ cbp->cb_bp->b_blkno;

	if (cbp->cb_state & ZD_RESET) {
		/*
		 * Error occurred during attempted reset for previous
		 * error. Give up and shutdown the drive.
		 */
		s_ipl = p_lock(&zdcprlock, SPLZD);
		printf("zd%d: Reset Failed.\n", cbp->cb_unit);
		if (cbp->cb_compcode >= zdncompcodes) {
			printf("zd%d: Bad cb_compcode 0x%x.\n",
				cbp->cb_unit, cbp->cb_compcode);
			cbp->cb_compcode = zdncompcodes - 1;
		}
		zd_hard_error(cbp, blkno);
		zddumpstatus(cbp);
		v_lock(&zdcprlock, s_ipl);
		zdshutdrive(up);
	} else switch (cbp->cb_compcode) {

	/*
	 * Command completed successfully but an ecc error occurred.
	 */
	case ZDC_SOFTECC:
	case ZDC_CORRECC:
		/*
		 * Corrected or Soft ECC error.
		 */
		dd = (up->zu_drive & 1) ? &ctlrp->zdc_chanB : &ctlrp->zdc_chanA;
		blkno = cbp->cb_cyl
			- zdparts[up->zu_drive_type][ZDPART(cbp->cb_bp->b_dev)].p_cyloff;
		blkno *= (dd->zdd_sectors * dd->zdd_tracks);
		blkno += (cbp->cb_head * dd->zdd_sectors);
		blkno += cbp->cb_sect;
		s_ipl = p_lock(&zdcprlock, SPLZD);
		printf("zd%d%c: %s at (%d, %d, %d).\n",
			cbp->cb_unit, PARTCHR(cbp->cb_bp->b_dev),
			zd_compcodes[cbp->cb_compcode],
			cbp->cb_cyl, cbp->cb_head, cbp->cb_sect);
		printf("zd%d%c: Filesystem blkno = %d.\n", cbp->cb_unit,
			PARTCHR(cbp->cb_bp->b_dev), blkno);
		zddumpstatus(cbp);
		v_lock(&zdcprlock, s_ipl);
		if (cbp->cb_state & ZD_REVECTOR) {
			/*
			 * Completed revector. Continue with rest of transfer.
			 */
			zdcontinue(cbp, up, ctlrp);
			return;
		}
		donebp = cbp->cb_bp;
		goto donext;		/* Not fatal */

	/*
	 * Fail the job immediately - no retries.
	 */
	case ZDC_DRVPROT:
	case ZDC_ECC:
		s_ipl = p_lock(&zdcprlock, SPLZD);
		zd_hard_error(cbp, blkno);
		zddumpstatus(cbp);
		v_lock(&zdcprlock, s_ipl);
		break;

	case ZDC_CH_RESET:		/* Shutdown drive */
	case ZDC_BADDRV:
	case ZDC_DDC_STAT:
		s_ipl = p_lock(&zdcprlock, SPLZD);
		zd_hard_error(cbp, blkno);
		zddumpstatus(cbp);
		v_lock(&zdcprlock, s_ipl);
		zdshutdrive(up);
		break;

	case ZDC_DMA_TO:		/* Shutdown Channel */
		s_ipl = p_lock(&zdcprlock, SPLZD);
		zd_hard_error(cbp, blkno);
		v_lock(&zdcprlock, s_ipl);
		zdshutchan(ctlrp, up->zu_drive & 1);
		break;

	case ZDC_NOCFG:
		if (cbp->cb_cmd == ZDC_GET_CHANCFG) {
			donebp = cbp->cb_bp;
			goto donext;		/* Not an error */
		}
		/*
		 * FW assumed insane...
		 */
	case ZDC_REVECT:		/* FW hosed - shutdown controller */
	case ZDC_ILLVECIO:
	case ZDC_ILLPGSZ:
	case ZDC_ILLDUMPADR:
		s_ipl = p_lock(&zdcprlock, SPLZD);
		zd_hard_error(cbp, blkno);
		zddumpstatus(cbp);	/* for ZDC_REVECT */
		v_lock(&zdcprlock, s_ipl);
		zdshutctlr(ctlrp);
		break;

	default:
		/*
		 * Unknown completion code - controller bad?
		 */
		s_ipl = p_lock(&zdcprlock, SPLZD);
		printf("zd%d: Bad cb_compcode 0x%x.\n",
			cbp->cb_unit, cbp->cb_compcode);
		cbp->cb_compcode = zdncompcodes - 1;	/* nice message */
		zd_hard_error(cbp, blkno);
		v_lock(&zdcprlock, s_ipl);
		zdshutctlr(ctlrp);
		break;

	case ZDC_ILLCMD:
		s_ipl = p_lock(&zdcprlock, SPLZD);
		zd_hard_error(cbp, blkno);
		if (cbp->cb_cmd == 0 || cbp->cb_cmd > ZDC_MAXCMD) {
			printf("zd%d: cb_cmd 0x%x corrupted.\n",
				cbp->cb_unit, cbp->cb_cmd);
			v_lock(&zdcprlock, s_ipl);
			panic("zdintr: cb corrupted");
		}
		v_lock(&zdcprlock, s_ipl);
		/*
		 * Cb looks ok - assume controller unreliable
		 */
		zdshutctlr(ctlrp);
		break;

	case ZDC_ILLMOD:
		s_ipl = p_lock(&zdcprlock, SPLZD);
		zd_hard_error(cbp, blkno);
		switch(cbp->cb_cmd) {

		case ZDC_READ:
		case ZDC_READ_SS:
		case ZDC_READ_HDRS:
		case ZDC_LONG_READ:
		case ZDC_REC_DATA:
			if (cbp->cb_mod != up->zu_ioctlcb.cb_mod) {
				printf("zd%d: cb_mod 0x%x corrupted.\n",
					cbp->cb_unit, cbp->cb_mod);
				v_lock(&zdcprlock, s_ipl);
				panic("zdintr: cb corrupted");
			}
		}
		v_lock(&zdcprlock, s_ipl);
		/*
		 * Cb looks ok - assume controller unreliable
		 */
		zdshutctlr(ctlrp);
		break;

	case ZDC_ILLALIGN:
		s_ipl = p_lock(&zdcprlock, SPLZD);
		zd_hard_error(cbp, blkno);
		if ((cbp->cb_addr & (ADDRALIGN - 1)) != 0) {
			printf("zd%d: cb_addr 0x%x corrupted.\n",
				cbp->cb_unit, cbp->cb_addr);
			v_lock(&zdcprlock, s_ipl);
			panic("zdintr: cb corrupted");
		}
		v_lock(&zdcprlock, s_ipl);
		/*
		 * Cb looks ok - assume controller unreliable
		 */
		zdshutctlr(ctlrp);
		break;

	case ZDC_ILLCNT:
		s_ipl = p_lock(&zdcprlock, SPLZD);
		zd_hard_error(cbp, blkno);
		if (cbp->cb_count == 0 || (cbp->cb_count & (CNTMULT-1)) != 0) {
			printf("zd%d: cb_count 0x%x corrupted.\n",
				cbp->cb_unit, cbp->cb_count);
			v_lock(&zdcprlock, s_ipl);
			panic("zdintr: cb corrupted");
		}
		v_lock(&zdcprlock, s_ipl);
		/*
		 * Cb looks ok - assume controller unreliable
		 */
		zdshutctlr(ctlrp);
		break;

	case ZDC_ILLIOV:
		s_ipl = p_lock(&zdcprlock, SPLZD);
		zd_hard_error(cbp, blkno);
		if (((u_int)cbp->cb_iovec & (IOVALIGN - 1)) != 0) {
			printf("zd%d: cb_iovec 0x%x corrupted.\n",
				cbp->cb_unit, cbp->cb_iovec);
			v_lock(&zdcprlock, s_ipl);
			panic("zdintr: cb corrupted");
		}
		v_lock(&zdcprlock, s_ipl);
		/*
		 * Cb looks ok - assume controller unreliable
		 */
		zdshutctlr(ctlrp);
		break;

	case ZDC_ILLCHS:
		s_ipl = p_lock(&zdcprlock, SPLZD);
		zd_hard_error(cbp, blkno);
		dd = (up->zu_drive & 1) ? &ctlrp->zdc_chanB : &ctlrp->zdc_chanA;
		if (cbp->cb_cyl >= dd->zdd_cyls ||
		    cbp->cb_head >= dd->zdd_tracks ||
		    cbp->cb_sect >= dd->zdd_sectors + dd->zdd_spare) {
			printf("zd%d: cb_diskaddr (%d, %d, %d) corrupted.\n",
				cbp->cb_unit,
				cbp->cb_cyl, cbp->cb_head, cbp->cb_sect);
			v_lock(&zdcprlock, s_ipl);
			panic("zdintr: cb corrupted");
		}
		v_lock(&zdcprlock, s_ipl);
		/*
		 * Cb looks ok - assume controller unreliable
		 */
		zdshutctlr(ctlrp);
		break;

	case ZDC_CBREUSE:
		printf("zd%d: CBREUSE error cb = 0x%x\n", cbp->cb_unit, cbp);
		panic("zdc: ZDC_CBREUSE error");

	case ZDC_ACCERR:
		s_ipl = splhi();
		val = rdslave(ctlrp->zdc_slicaddr,
			      (u_char)((up->zu_drive & 1) ? SL_G_ACCERR1
							  : SL_G_ACCERR0));
		printf("zd%d: Access error on transfer starting at physical address 0x%x.\n",
				cbp->cb_unit, cbp->cb_addr);
		access_error(val);
		val = ~val;
		if (((val & SLB_ATMSK) == SLB_AEFATAL) &&
		    ((val & SLB_AEIO) != SLB_AEIO)) {
			/*
			 * Uncorrectable memory error!
			 * Clear access error to restart controller and
			 * panic the system.
			 */
			wrslave(ctlrp->zdc_slicaddr,
				(u_char)((up->zu_drive & 1) ? SL_G_ACCERR1
							    : SL_G_ACCERR0),
				(u_char)0xbb);
			panic("zdc access error");
		}
		splx(s_ipl);

		/*
		 * Shutdown the channel and restart the controller.
		 */
		zdshutchan(ctlrp, up->zu_drive & 1);
		s_ipl = splhi();
		wrslave(ctlrp->zdc_slicaddr,
			(u_char)((up->zu_drive & 1) ? SL_G_ACCERR1
						    : SL_G_ACCERR0),
			(u_char)0xbb);
		splx(s_ipl);
		break;

	case ZDC_CH_TO:			/* Retry without reset */
	case ZDC_FDL:
		s_ipl = p_lock(&zdcprlock, SPLZD);
		if (cbp->cb_errcnt++ < zdcretry) {
#ifdef	DEBUG
			zddumpcb(cbp);
#endif	DEBUG
			printf("zd%d%c: Error (%s); cmd 0x%x at (%d, %d, %d).\n",
				cbp->cb_unit, PARTCHR(cbp->cb_bp->b_dev),
				zd_compcodes[cbp->cb_compcode],
				cbp->cb_cmd, cbp->cb_cyl, cbp->cb_head,
				cbp->cb_sect);
			printf("zd%d%c: Filesystem blkno = %d.\n", cbp->cb_unit,
				PARTCHR(cbp->cb_bp->b_dev), blkno);
			zddumpstatus(cbp);
			v_lock(&zdcprlock, s_ipl);
			zdretry(cbp, up, ctlrp);
			return;
		}
		/*
		 * Exceeded retry count.
		 * Shutdown the channel.
		 */
		zd_hard_error(cbp, blkno);
		zddumpstatus(cbp);
		v_lock(&zdcprlock, s_ipl);
		zdshutchan(ctlrp, up->zu_drive & 1);
		break;

	case ZDC_DRVFLT:
		/*
		 * cb_count and cb_diskaddr are unreliable on drive fault
		 * so retry entire request.
		 */
		cbp->cb_bp->b_resid = cbp->cb_bp->b_bcount - cbp->cb_transfrd;
		blkno =	btodb(cbp->cb_bp->b_bcount - cbp->cb_bp->b_resid)
				+ cbp->cb_bp->b_blkno;
		dd = (up->zu_drive & 1) ? &ctlrp->zdc_chanB : &ctlrp->zdc_chanA;
		i = blkno;
		partition = ZDPART(cbp->cb_bp->b_dev);
#if 1
		{ /* gcc bug workaround */
		    int temp;
		    short temp2;
		    temp =
			i / (dd->zdd_sectors * dd->zdd_tracks) +
			zdparts[up->zu_drive_type][partition].p_cyloff;
		    temp2 = temp;
		    cbp->cb_diskaddr.da_cyl = temp2;
		}
#else
		cbp->cb_diskaddr.da_cyl = 
			i / (dd->zdd_sectors * dd->zdd_tracks) +
			zdparts[up->zu_drive_type][partition].p_cyloff;
#endif
		i %= (dd->zdd_sectors * dd->zdd_tracks);
		cbp->cb_diskaddr.da_head = i / dd->zdd_sectors;
		cbp->cb_diskaddr.da_sect = i % dd->zdd_sectors;
		/* Fall into */
	case ZDC_SEEKERR:	/* reset and retry */
	case ZDC_SEEK_TO:
		s_ipl = p_lock(&zdcprlock, SPLZD);
		if (cbp->cb_errcnt < zdcretry) {
#ifdef	DEBUG
			zddumpcb(cbp);
#endif	DEBUG
			printf("zd%d%c: Error (%s); cmd 0x%x at (%d, %d, %d).\n",
				cbp->cb_unit, PARTCHR(cbp->cb_bp->b_dev),
				zd_compcodes[cbp->cb_compcode],
				cbp->cb_cmd, cbp->cb_cyl, cbp->cb_head,
				cbp->cb_sect);
			printf("zd%d%c: Filesystem blkno = %d.\n", cbp->cb_unit,
				PARTCHR(cbp->cb_bp->b_dev), blkno);
			zddumpstatus(cbp);
			v_lock(&zdcprlock, s_ipl);
			/*
			 * reset drive
			 */
			cbp->cb_state |= ZD_RESET;
			cbp->cb_cmd = ZDC_RESET;
			NUDGE_ZDC(ctlrp, cbp, s_ipl);
			return;
		}

		/*
		 * Exceeded retry count.
		 * Shutdown the drive.
		 */
		zd_hard_error(cbp, blkno);
		zddumpstatus(cbp);
		v_lock(&zdcprlock, s_ipl);
		zdshutdrive(up);
		break;

	case ZDC_SNF:			/* Revector request */
		if (zdrevector(cbp, up, ctlrp))
			return;
		/* else fall into... */
	case ZDC_HDR_ECC:		/* Reset and retry */
	case ZDC_SO:
	case ZDC_NDS:
		s_ipl = p_lock(&zdcprlock, SPLZD);
		if (cbp->cb_errcnt == zdcretry)
			zd_hard_error(cbp, blkno);
		else {
#ifdef	DEBUG
			zddumpcb(cbp);
#endif	DEBUG
			printf("zd%d%c: Error (%s); cmd 0x%x at (%d, %d, %d).\n",
				cbp->cb_unit, PARTCHR(cbp->cb_bp->b_dev),
				zd_compcodes[cbp->cb_compcode],
				cbp->cb_cmd, cbp->cb_cyl, cbp->cb_head,
				cbp->cb_sect);
			printf("zd%d%c: Filesystem blkno = %d.\n", cbp->cb_unit,
				PARTCHR(cbp->cb_bp->b_dev), blkno);
		}
		zddumpstatus(cbp);
		v_lock(&zdcprlock, s_ipl);
		cbp->cb_state |= ZD_RESET;
		cbp->cb_cmd = ZDC_RESET;
		NUDGE_ZDC(ctlrp, cbp, s_ipl);
		return;

	} /* end of switch */

	/*
	 * Fail this I/O request
	 */
	donebp = cbp->cb_bp;
	donebp->b_flags |= B_ERROR;
	donebp->b_error = EIO;

donext:
#ifdef	DEBUG
	if (zddebug > 2)
		zddumpcb(cbp);
#endif	DEBUG
	if (donebp->b_flags & B_IOCTL) {
		bcopy((caddr_t)cbp, (caddr_t)&up->zu_ioctlcb, FWCBSIZE);
	}
	/*
	 * If more requests - start them.
	 */
	s_ipl = p_lock(&up->zu_lock, SPLZD);
	cbp->cb_bp = NULL;
	if (up->zu_bhead.av_forw != NULL) {
		zdstart(up);
	} else {
		if (up->zu_cbptr->cb_bp == NULL
		&&  up->zu_cbptr[1].cb_bp == NULL) {
			/*
			 * Going idle.
			 * Get elapsed time and decrement I/O activity.
			 */
#ifndef	MACH
			if (up->zu_dkstats) {
				struct timeval elapsed;

				elapsed = time;
				timevalsub(&elapsed, &up->zu_starttime);
				timevaladd(&up->zu_dkstats->dk_time, &elapsed);
			}
#endif	MACH
			if (fp_lights) {
				s_ipl = splhi();
				FP_IO_INACTIVE;
				splx(s_ipl);
			}
		}
	}
	v_lock(&up->zu_lock, s_ipl);

	/*
	 * Gather stats
	 * Note: no attempt to mutex...
	 */
#ifndef	MACH
	if (up->zu_dkstats) {
		up->zu_dkstats->dk_xfer++;
		up->zu_dkstats->dk_blks +=
			btodb(donebp->b_bcount - donebp->b_resid);
	}
#endif	MACH
	biodone(donebp);
}

/*
 * zd_hard_error
 *	report hard error
 */
zd_hard_error(cbp, blkno)
	register struct cb *cbp;
	daddr_t	blkno;
{
#ifdef	DEBUG
	zddumpcb(cbp);
#endif	DEBUG
	printf("zd%d%c: Hard Error (%s); cmd 0x%x at (%d, %d, %d).\n",
		cbp->cb_unit, PARTCHR(cbp->cb_bp->b_dev),
		zd_compcodes[cbp->cb_compcode], cbp->cb_cmd,
		cbp->cb_cyl, cbp->cb_head, cbp->cb_sect);
	printf("zd%d%c: Filesystem blkno = %d.\n", cbp->cb_unit,
		PARTCHR(cbp->cb_bp->b_dev), blkno);
}

/*
 * zdgetrpl()
 *	Search bad block list for replacement sector for bad sector (SNF).
 *
 * Return:
 *	pointer to replacement sector
 *	otherwise NULL pointer if no replacement sector found.
 *	
 */
static struct diskaddr *
zdgetrpl(badsect, zdp)
	register struct diskaddr *badsect;
	register struct zdbad	 *zdp;
{
	register struct bz_bad	 *bb;

	bb = zdp->bz_bad;
	for (bb = zdp->bz_bad; bb < &zdp->bz_bad[zdp->bz_nsnf]; bb++) {
		if (bb->bz_cyl < badsect->da_cyl)
			continue;
		if (bb->bz_cyl > badsect->da_cyl)
			break;
		/* cylinder matched */
		if (bb->bz_head < badsect->da_head)
			continue;
		if (bb->bz_head > badsect->da_head)
			break;
		/* head matched */
		if (bb->bz_sect == badsect->da_sect)
			return (&bb->bz_rpladdr);
	}
	return (NULL);
}

/*
 * zdrevector
 *	Attempt to revector the sector-not-found.
 * If the sector-not-found is in the bad block list perform the I/O on
 * its replacement sector. Then continue with the following sector if
 * necessary.
 * 
 * Return values:
 *	 0 - No replacement sector in bad block list.
 *	 1 - Replacement sector found in bad block list.
 */
int
zdrevector(cbp, up, ctlrp)
	register struct cb *cbp;
	struct	zd_unit	*up;
	struct zdc_ctlr *ctlrp;
{
	register struct zdcdd	 *dd;		/* disk description */
	register struct diskaddr *replcmnt;	/* replacement address */
	register int	count;			/* # bytes of iovecs */
	int	transfrd;			/* bytes already transferred */
	int	cbcount;
	u_int	*from;
	spl_t	s_ipl;

#ifdef	DEBUG
	if (zddebug)
		printf("zd%d: revectoring (%d, %d, %d)", cbp->cb_unit,
			cbp->cb_cyl, cbp->cb_head, cbp->cb_sect);
#endif	DEBUG
	/*
	 * Determine if bad block is in bad block list.
	 */
	replcmnt = zdgetrpl(&cbp->cb_diskaddr, up->zu_zdbad);
	if (replcmnt == NULL) {
		/*
		 * Not in bad block list. Caller should retry
		 * bad block.
		 */
#ifdef	DEBUG
		if (zddebug)
			printf(": not in table.\n");
#endif	DEBUG
		return (0);
	}

	/*
	 * Save state of current request, do new request for replacement.
	 */
	transfrd = cbp->cb_bp->b_bcount - cbp->cb_bp->b_resid;
	/* adjust for previously transferred sectors */
	transfrd -= cbp->cb_transfrd;
	from = cbp->cb_iovstart
		+ i386_btop((cbp->cb_addr & (I386_PGBYTES-1)) + transfrd);
	cbp->cb_transfrd += transfrd;
	cbp->cb_addr += transfrd;

	/*
	 * figure number of iovecs to copy down.
	 */
	count = 0;
	cbcount = cbp->cb_count;
	if (cbp->cb_addr & (I386_PGBYTES-1)) {
		++count;
		cbcount -= I386_PGBYTES - (cbp->cb_addr & (I386_PGBYTES-1));
	}
	count += i386_btop(cbcount + (I386_PGBYTES-1));
	count *= sizeof(u_int *);
	cbp->cb_iovec = KVTOPHYS(cbp->cb_iovstart, u_int *);
	bcopy((caddr_t)from, (caddr_t)cbp->cb_iovstart, (unsigned)count);

	dd = (up->zu_drive & 1) ? &ctlrp->zdc_chanB : &ctlrp->zdc_chanA;
	if (cbp->cb_count != DEV_BSIZE) {
		cbp->cb_state |= ZD_REVECTOR;
		if (++cbp->cb_sect == dd->zdd_sectors) {
			cbp->cb_sect = 0;
			if (++cbp->cb_head == dd->zdd_tracks) {
				cbp->cb_head = 0;
				cbp->cb_cyl++;
			}
		}
		*(int *)&cbp->cb_contaddr = *(int *)&cbp->cb_diskaddr;
		cbp->cb_contiovsz = count;
	}
	cbp->cb_count = DEV_BSIZE;
	cbp->cb_diskaddr = *replcmnt;
	cbp->cb_psect = zdgetpsect(&cbp->cb_diskaddr, dd);
#ifdef	DEBUG
	if (zddebug)
		printf(" to (%d, %d, %d).\n",
			cbp->cb_cyl, cbp->cb_head, cbp->cb_sect);
#endif	DEBUG

	/*
	 * Nudge controller.
	 */
	NUDGE_ZDC(ctlrp, cbp, s_ipl);
	return (1);
}

/*
 * zdcontinue
 *	Continue rest of I/O request after the revectored sector.
 */
zdcontinue(cbp, up, ctlrp)
	register struct cb	 *cbp;
	struct zd_unit *up;
	register struct	zdc_ctlr *ctlrp;
{
	u_int	*from;
	spl_t	s_ipl;

	/*
	 * Revectoring completed successfully.
	 * Restart rest of I/O request.
	 */
	cbp->cb_state &= ~ZD_REVECTOR;
	cbp->cb_bp->b_resid -= DEV_BSIZE;
	cbp->cb_transfrd += DEV_BSIZE;
	cbp->cb_errcnt = 0;
	cbp->cb_count = cbp->cb_bp->b_resid;
	cbp->cb_iovec = KVTOPHYS(cbp->cb_iovstart, u_int *);
	from = cbp->cb_iovstart
		+ i386_btop((cbp->cb_addr & (I386_PGBYTES-1)) + DEV_BSIZE);
	cbp->cb_addr += DEV_BSIZE;
	if (from != cbp->cb_iovstart) {
		/*
		 * Must start with next iovector.
		 */
		bcopy((caddr_t)from, (caddr_t)cbp->cb_iovstart,
			(unsigned)(cbp->cb_contiovsz - sizeof(u_int *)));
	}
	*(int *)&cbp->cb_diskaddr = *(int *)&cbp->cb_contaddr;
	cbp->cb_psect = zdgetpsect(&cbp->cb_diskaddr,
					(up->zu_drive & 1) ? &ctlrp->zdc_chanB
							   : &ctlrp->zdc_chanA);
	/*
	 * Nudge controller.
	 */
	NUDGE_ZDC(ctlrp, cbp, s_ipl);
}

/*
 * zdretry
 *	Retry the request.
 * If the request is a normal read/write request then retry the command
 * from where the error occurred. If B_IOCTL request, retry from the
 * initial CB.
 */
zdretry(cbp, up, ctlrp)
	register struct cb *cbp;
	struct	zd_unit  *up;
	struct	zdc_ctlr *ctlrp;
{
	register int count;		/* # iovecs remaining */
	register int transfrd;		/* # bytes transferred */
	register int cbcount;
	u_int	*from;			/* 1st retry iovec */
	spl_t	s_ipl;

	/*
	 * Retry job
	 */
	if (cbp->cb_bp->b_flags & B_IOCTL) {
		/*
		 * copy 1st half of cb (what fw will see)
		 */
		bcopy((caddr_t)&up->zu_ioctlcb, (caddr_t)cbp, FWCBSIZE);
	} else {
		cbp->cb_count = cbp->cb_bp->b_resid;
		cbp->cb_cmd = (cbp->cb_bp->b_flags & B_READ) ? ZDC_READ : ZDC_WRITE;
		cbp->cb_sect &= ~ZD_AUTOBIT;	 /* drop Auto-revetor bit */
		cbp->cb_psect = zdgetpsect(&cbp->cb_diskaddr,
					(up->zu_drive & 1) ? &ctlrp->zdc_chanB
							   : &ctlrp->zdc_chanA);
		transfrd = cbp->cb_bp->b_bcount - cbp->cb_count;
		transfrd -= cbp->cb_transfrd;
		from = cbp->cb_iovstart
			+ i386_btop((cbp->cb_addr & (I386_PGBYTES-1)) + transfrd);
		cbp->cb_transfrd += transfrd;
		cbp->cb_addr += transfrd;

		/*
		 * determine number of iovecs remaining.
		 */
		count = 0;
		cbcount = cbp->cb_count;
		if (cbp->cb_addr & (I386_PGBYTES-1)) {
			++count;
			cbcount -= I386_PGBYTES - (cbp->cb_addr & (I386_PGBYTES-1));
		}
		count += i386_btop(cbcount + (I386_PGBYTES-1));

		/*
		 * Copy down iovecs to 32 byte aligned boundary.
		 * That is, cb_iovstart.
		 */
		bcopy((caddr_t)from, (caddr_t)cbp->cb_iovstart,
			(unsigned)(count * sizeof(u_int *)));
	}
	cbp->cb_iovec = KVTOPHYS(cbp->cb_iovstart, u_int *);

	/*
	 * Nudge controller.
	 */
	NUDGE_ZDC(ctlrp, cbp, s_ipl);
}

/*
 * zdc_cb_cleanup
 *	Cleanup (cancel) jobs active on disabled controller.
 * Called as timeout routine to allow completed CB's to drain.
 * That is, those for which a completion interrupt was sent just
 * before the error interrupt occurred.
 */
zdc_cb_cleanup(ctlrp)
	register struct zdc_ctlr *ctlrp;
{
	register struct cb	 *cbp;
	register struct buf	 *bp;
	spl_t	s_ipl;

	/*
	 * Controller dead. Fail all active I/O requests.
	 */
	cbp = ctlrp->zdc_cbp;
	for (; cbp < &ctlrp->zdc_cbp[NCBPERZDC]; cbp++) {
		if (cbp->cb_bp != NULL) {
			bp = cbp->cb_bp;
			bp->b_flags |= B_ERROR;
			bp->b_error = EIO;
			bp->b_resid = bp->b_bcount;
			biodone(bp);
			bp = NULL;
			/*
			 * Decrement I/O activity
			 */
			if (fp_lights) {
				s_ipl = splhi();
				FP_IO_INACTIVE;
				splx(s_ipl);
			}
		}
	}
}

/*
 * zdc_error
 *	Controller error interrupt.
 */
zdc_error(vector)
	u_char	vector;
{
	register int	val;
	register struct zdc_ctlr *ctlrp;
	spl_t	s_ipl;

	ctlrp = &zdctrlr[vector - base_err_intr];
	s_ipl = splhi();	/* In case zdc_err_bin not splhi() */
	val = rdslave(ctlrp->zdc_slicaddr, SL_Z_STATUS);
	splx(s_ipl);
	if (((val & SLB_ZPARERR) != SLB_ZPARERR) &&
	    ((val & ZDC_READY) == ZDC_READY)) {
		printf("zdc%d - stray error interrupt!\n", ctlrp - zdctrlr);
		return;
	}
	printf("zdc%d: controller interrupt - SL_Z_STATUS == 0x%x.\n",
		ctlrp - zdctrlr, val);
	if ((val & ZDC_ERRMASK) == ZDC_OBCB) {
		/*
		 * Assume stray interrupt happened and tell fw to continue.
		 */
		s_ipl = splhi();	/* in case zdc_err_bin not splhi() */
		mIntr(ctlrp->zdc_slicaddr, CLRERRBIN, 0xbb);
		splx(s_ipl);
		return;
	}

	splx(SPLZD);		/* Do the rest at SPLZD */

	/*
	 * Fail all queued I/O requests.
	 */
	zdshutctlr(ctlrp);

	/*
	 * Allow completed CBs to drain then
	 * fail all other "active" I/O requests.
	 */
	timeout(zdc_cb_cleanup, (caddr_t)ctlrp, 5 * hz);
}

/*
 * zdshutctlr
 *	Fail all I/O requests queued to the drives on this controller.
 *	Currently active I/O requests will finish asyncronously.
 *	The controller is marked as ZDC_DEAD and all units on the
 *	controller are marked as ZU_BAD.
 */
zdshutctlr(ctlrp)
	register struct zdc_ctlr *ctlrp;
{
	register struct cb *cbp;

	printf("zdc%d: Controller disabled.\n", ctlrp - zdctrlr);

	cbp = ctlrp->zdc_cbp;
	for (; cbp < &ctlrp->zdc_cbp[NCBPERZDC]; cbp += NCBPERDRIVE) {
		if (cbp->cb_unit >= 0)
			zdshutdrive(&zdunit[cbp->cb_unit]);
	}
	ctlrp->zdc_state = ZDC_DEAD;
}

/*
 * zdshutchan
 *	Fail all I/O requests queued to the drives on this channel.
 *	Currently active I/O requests will finish asyncronously.
 *	All units on the channel are marked as ZU_BAD.
 */
zdshutchan(ctlrp, channel)
	register struct zdc_ctlr *ctlrp;
	int	channel;		/* 0 == Channel A, 1 == channel B */
{
	register struct cb *cbp;

	printf("zdc%d: Channel %c disabled.\n", ctlrp - zdctrlr,
			(channel == 0) ? 'A' : 'B');

	cbp = &ctlrp->zdc_cbp[channel * NCBPERDRIVE];
	for (; cbp < &ctlrp->zdc_cbp[NCBPERZDC]; cbp += 2 * NCBPERDRIVE) {
		if (cbp->cb_unit >= 0)
			zdshutdrive(&zdunit[cbp->cb_unit]);
	}
}

/*
 * zdshutdrive
 *	Fail all I/O requests queued to the drive.
 *	Currently active I/O requests will finish asyncronously.
 *	The drive is marked as ZU_BAD.
 */
zdshutdrive(up)
	register struct	zd_unit *up;
{
	register struct	buf	*bp, *nextbp;
	spl_t	s_ipl;

	/*
	 * Lock the device and fail all jobs currently queued.
	 */
	s_ipl = p_lock(&up->zu_lock, SPLZD);
	if (up->zu_state == ZU_BAD) {
		v_lock(&up->zu_lock, s_ipl);
		return;
	}
	printf("zd%d: Drive disabled.\n", up - zdunit);
	bp = up->zu_bhead.av_forw;
	while (bp != NULL) {
		bp->b_flags |= B_ERROR;
		bp->b_error = EIO;
		bp->b_resid = bp->b_bcount;
		nextbp = bp->av_forw;
		biodone(bp);
		bp = nextbp;
	}
	up->zu_bhead.av_forw = NULL;
	up->zu_state = ZU_BAD;
	v_lock(&up->zu_lock, s_ipl);
	/*
	 * Turn on error light on front panel.
	 */
	if (fp_lights) {
		s_ipl = splhi();
		FP_IO_ERROR;
		splx(s_ipl);
	}
	/*
	 * It had been online and usable.
	 */
	disk_offline();
}

/*
 * zdminphys - correct for too large a request.
 *
 * Note correction for non-cluster-aligned transfers.
 */
zdminphys(bp)
	struct buf *bp;
{
	if (bp->b_bcount > ((zdc_iovpercb - 1) * I386_PGBYTES))
		bp->b_bcount = (zdc_iovpercb - 1) * I386_PGBYTES;
}

zdread(dev, uio)
	dev_t	dev;
	struct uio *uio;
{
#ifndef	MACH
	int err, diff;
	off_t lim;

	lim = zdparts[zdunit[ZDUNIT(dev)].zu_drive_type][ZDPART(dev)].p_length;
	lim = dbtob(lim);
	err = physck(lim, uio, B_READ, &diff);
	if (err != 0) {
		if (err == -1)	/* not an error, but request of 0 bytes */
			err = 0;
		return (err);
	}
	err = physio(zdstrat, (struct buf *)0, dev, B_READ, zdminphys, uio);
	uio->uio_resid += diff;
	return (err);
#else	MACH
	return physio(zdstrat, &zdbuf, dev, B_READ, zdminphys, uio);
#endif	MACH
}

zdwrite(dev, uio)
	dev_t	dev;
	struct uio *uio;
{
#ifndef	MACH
	int err, diff;
	off_t lim;

	lim = zdparts[zdunit[ZDUNIT(dev)].zu_drive_type][ZDPART(dev)].p_length;
	lim = dbtob(lim);
	err = physck(lim, uio, B_WRITE, &diff);
	if (err != 0) {
		if (err == -1)	/* not an error, but request of 0 bytes */
			err = 0;
		return (err);
	}
	err = physio(zdstrat, (struct buf *)0, dev, B_WRITE, zdminphys, uio);
	uio->uio_resid += diff;
	return (err);
#else	MACH
	return physio(zdstrat, &zdbuf, dev, B_WRITE, zdminphys, uio);
#endif	MACH
}

/*
 * zdioctl
 *	Currently the only ZIOCBCMDs that are supported are ZDC_READ_LRAM and
 *	ZDC_WRITE_LRAM. The ioctl mechanism could be used to support
 *	more functions to facilitate online formatting.
 */
/*ARGSUSED*/
zdioctl(dev, cmd, data, flag)
	dev_t	dev;
	int	cmd;
	caddr_t data;
	int	flag;
{
#ifndef	MACH
	register struct cb *cbp;	/* cb argument */
	register struct buf *bp;	/* ioctl buffer */
	register struct zd_unit *up;	/* unit structure */
	int	error;
	spl_t	s_ipl;

	up = &zdunit[ZDUNIT(dev)];
	bp = &up->zu_ioctl;

	switch(cmd) {

	case ZIOCBCMD:
		cbp = (struct cb *)data;
		switch(cbp->cb_cmd) {

		case ZDC_GET_CHANCFG:
			if (cbp->cb_count != sizeof(struct zdcdd) ||
			    (cbp->cb_addr & (ADDRALIGN-1)) != 0)
				return (EINVAL);
			if (!useracc((char *)cbp->cb_addr,
						(u_int)cbp->cb_count, B_WRITE))
				return (EFAULT);
			bufalloc(bp);
			bp->b_flags = B_READ;
			bp->b_bcount = cbp->cb_count;
			bp->b_un.b_addr = (caddr_t)cbp->cb_addr;
			break;

		case ZDC_READ_SS:
			if (cbp->cb_count != ZDD_SS_SIZE ||
			    (cbp->cb_addr & (ADDRALIGN-1)) != 0 ||
			    cbp->cb_diskaddr.da_cyl != 0 ||
			    cbp->cb_diskaddr.da_head != 0 ||
			    cbp->cb_diskaddr.da_sect >= ZDD_NDDSECTORS)
				return (EINVAL);
			if (!useracc((char *)cbp->cb_addr,
						(u_int)cbp->cb_count, B_WRITE))
				return (EFAULT);
			bufalloc(bp);
			bp->b_flags = B_READ;
			bp->b_bcount = cbp->cb_count;
			bp->b_un.b_addr = (caddr_t)cbp->cb_addr;
			break;

		case ZDC_WRITE_LRAM:
			if (!suser())
				return (EPERM);
			/* Fall into */
		case ZDC_READ_LRAM:
			/*
			 * Check if LRAM address reasonable.
			 */
			if (cbp->cb_addr >= (ZDC_LRAMSZ / sizeof(int)))
				return (EINVAL);
			bufalloc(bp);
			bp->b_flags = (cbp->cb_cmd == ZDC_READ_LRAM) ? B_READ
								     : B_WRITE;
			bp->b_bcount = 0;		/* data in CB */
			bp->b_un.b_addr = NULL;
			break;

		default:
			return (EINVAL);
		}

		/*
		 * Do ioctl.
		 */
		bcopy((caddr_t)cbp, (caddr_t)&up->zu_ioctlcb, FWCBSIZE);
		bp->b_flags |= B_IOCTL;
		bp->b_dev = dev;
		bp->b_blkno = 0;
		bp->b_error = 0;
		bp->b_proc = u.u_procp;
		bp->b_iotype = B_RAWIO;
		BIODONE(bp) = 0;
		++u.u_procp->p_noswap;
		if (bp->b_bcount > 0) {
			/*
			 * lock down pages
			 */
			vslock(bp->b_un.b_addr, (int)bp->b_bcount,
				(bool_t)(bp->b_flags & B_READ));
		}
		s_ipl = p_lock(&up->zu_lock, SPLZD);
		if (up->zu_state == ZU_BAD) {
			v_lock(&up->zu_lock, s_ipl);
			/*
			 * Controller/channel/Drive has gone bad!
			 */
			buffree(bp);
			--u.u_procp->p_noswap;
			return (EIO);
		}
		/*
		 * Insert at head of list and zdstart!
		 */
		bp->av_forw = up->zu_bhead.av_forw;
		up->zu_bhead.av_forw = bp;
		zdstart(up);
		v_lock(&up->zu_lock, s_ipl);
		biowait(bp);
		--u.u_procp->p_noswap;			/* re-swappable */
		error = geterror(bp);
		bcopy((caddr_t)&up->zu_ioctlcb, (caddr_t)cbp, FWCBSIZE);
		buffree(bp);
		return (error);

	default:
		return (EINVAL);
	}
#else	MACH
	return EINVAL;
#endif	MACH
}

#ifdef	MACH_KERNEL
/*
 * Get status routine
 */
zdgetstat(dev, flavor, status, count)
	dev_t	dev;
	int	*status, *count;
{
	if (flavor == DEV_GET_SIZE) {
		unsigned int	length;

		length = zdparts[
			   zdunit[ ZDUNIT(dev) ].zu_drive_type
			 ] [ ZDPART(dev) ].p_length;
		status[DEV_GET_SIZE_DEVICE_SIZE] = length * DEV_BSIZE;
		status[DEV_GET_SIZE_RECORD_SIZE] = DEV_BSIZE;
		*count = DEV_GET_SIZE_COUNT;
		
		return D_SUCCESS;
	} else return D_INVALID_OPERATION;
}
#endif	/*MACH_KERNEL*/

/*
 *	Routine to return information to kernel.
 */
int
zd_devinfo(dev, flavor, info)
	int	dev;
	int	flavor;
	char	*info;
{
	register int	result;

	result = D_SUCCESS;

	switch (flavor) {
	case D_INFO_BLOCK_SIZE:
		*((int *) info) = DEV_BSIZE;
		break;
	default:
		result = D_INVALID_OPERATION;
	}

	return(result);
}

/*
 * zdsize()
 *	Used for swap-space partition calculation.
 */
zdsize(dev)
	register dev_t dev;
{
	register struct zd_unit *up;

	up = &zdunit[ZDUNIT(dev)];

	if (ZDUNIT(dev) >= zdc_conf->zc_nent
	||  up->zu_state != ZU_GOOD
	||  ((up->zu_cfg & (ZD_FORMATTED|ZD_MATCH)) != (ZD_FORMATTED|ZD_MATCH))
	||  zdparts[up->zu_drive_type][ZDPART(dev)].p_length == 0)
			return (-1);
	return (zdparts[up->zu_drive_type][ZDPART(dev)].p_length);
}

/*
 * Print out status bytes when appropriate.
 */
zddumpstatus(cbp)
	register struct cb *cbp;
{
	register int i;

	if (cbp->cb_cmd == ZDC_INIT || cbp->cb_cmd == ZDC_PROBE ||
	    cbp->cb_cmd == ZDC_PROBEDRIVE)
		return;

	switch(cbp->cb_compcode) {

	case ZDC_DRVPROT:
	case ZDC_DRVFLT:
	case ZDC_SEEKERR:
	case ZDC_HDR_ECC:
	case ZDC_SOFTECC:
	case ZDC_CORRECC:
	case ZDC_ECC:
	case ZDC_SNF:
	case ZDC_REVECT:
	case ZDC_SO:
	case ZDC_NDS:
	case ZDC_FDL:
	case ZDC_DDC_STAT:
		break;

	default:
		if (cbp->cb_cmd == ZDC_READ_LRAM || cbp->cb_cmd == ZDC_WRITE_LRAM)
			break;
		return;
	}
	if (cbp->cb_bp != NULL)
		printf("zd%d%c: cb_status:", cbp->cb_unit,
			PARTCHR(cbp->cb_bp->b_dev));
	else
		printf("zd%d: cbstatus:", cbp->cb_unit);
	for (i=0; i < NSTATBYTES; i++)
		printf(" 0x%x", cbp->cb_status[i]);
	printf("\n");
}

#ifdef	DEBUG
/*
 * Print out the contents of a cb.
 */
zddumpcb(cbp)
	register struct cb *cbp;
{
	register struct init_cb *icbp;
	register int i;
	register int count;
	int	cbcount;

	printf("zd%d: cb at 0x%x, cb_cmd 0x%x, cb_compcode 0x%x\n",
		cbp->cb_unit, cbp, cbp->cb_cmd, cbp->cb_compcode);
	if (cbp->cb_cmd == ZDC_INIT) {
		icbp = (struct init_cb *)cbp;
		printf("icb_ctrl 0x%x, icb_pagesize 0x%x, icb_dumpaddr 0x%x\n",
			icbp->icb_ctrl, icbp->icb_pagesize, icbp->icb_dumpaddr);
		printf("icb_dest 0x%x, icb_bin 0x%x, icb_vecbase 0x%x\n",
			icbp->icb_dest, icbp->icb_bin, icbp->icb_vecbase);
		printf("icb_errdest 0x%x, icb_errbin 0x%x, icb_errvector 0x%x\n\n",
			icbp->icb_errdest, icbp->icb_errbin,
			icbp->icb_errvector);
		return;
	}
	if (cbp->cb_cmd == ZDC_PROBE || cbp->cb_cmd == ZDC_PROBEDRIVE) {
		printf("pcb_drivecfg:");
		for (i = 0; i < ZDC_MAXDRIVES; i++)
			printf(" 0x%x",
				 ((struct probe_cb *)cbp)->pcb_drivecfg[i]);
		printf("\n\n");
		return;
	}
	printf("cb_mod 0x%x, cb_diskaddr (%d, %d, %d), cb_psect 0x%x\n",
		cbp->cb_mod, cbp->cb_cyl, cbp->cb_head, cbp->cb_sect,
		cbp->cb_psect);
	printf("cb_addr 0x%x, cb_count 0x%x, cb_iovec 0x%x, cb_reqstat 0x%x\n",
		cbp->cb_addr, cbp->cb_count, cbp->cb_iovec, cbp->cb_reqstat);
	zddumpstatus(cbp);
	printf("cb_bp 0x%x, cb_errcnt 0x%x, cb_iovstart 0x%x, cb_state 0x%x\n",
		cbp->cb_bp, cbp->cb_errcnt, cbp->cb_iovstart, cbp->cb_state);
	printf("cb_contaddr 0x%x, cb_contiovsz 0x%x, cb_transfrd 0x%x\n",
		cbp->cb_contaddr, cbp->cb_contiovsz, cbp->cb_transfrd);
	if (cbp->cb_iovec != NULL) {
		count = 0;
		cbcount = cbp->cb_count;
#ifdef	MACH_KERNEL
		if (cbp->cb_addr != trunc_page(cbp->cb_addr)) {
			++count;
			cbcount -= PAGE_SIZE -
				   (cbp->cb_addr - trunc_page(cbp->cb_addr));
		}
		count += atop(round_page(cbcount));
#else	/* MACH_KERNEL */
		if (cbp->cb_addr & CLOFSET) {
			++count;
			cbcount -= CLBYTES - (cbp->cb_addr & CLOFSET);
		}
		count += (cbcount + CLOFSET) >> CLSHIFT;
#endif	/* MACH_KERNEL */
		printf("iovecs:");
		for (i = 0; i < count; i++) {
			printf(" 0x%x", cbp->cb_iovstart[i]);
			if ((i % 8) == 7)
				printf("\n\t");
		}
		printf("\n");
	}
	printf("\n");
}
#endif	DEBUG
