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
 * $Log:	pc_hd_label.c,v $
 * Revision 2.2  93/08/10  15:56:36  mrt
 * 	Created by Robert Baron from hc.c
 * 
 */

#include <hd.h>

#include <sys/types.h>
#define PRIBIO 20
#include <device/buf.h>
#include <device/errno.h>
#include <device/device_types.h>
#include <device/disk_status.h>
#include <sys/ioctl.h>
#include <i386/pio.h>
#include <i386/ipl.h>
#include <i386at/disk.h>
#include <chips/busses.h>
#include <i386at/hdreg.h>

/* From sys/systm.h */
struct buf *geteblk();

extern int hdgotvtoc[NHD];
extern struct alt_info alt_info[NHD];
extern struct disklabel label[NHD];
extern int	   labeloffset[NHD];
extern int	   labelsector[NHD];

extern int label_flag;
extern int allow_bsd_label;

#define LABEL_DEBUG(x,y) if (label_flag&x) y
unsigned dkcksum();


hdreadlabel(unit, badblocks)
unsigned char unit;
{
	struct disklabel	*lp = &label[unit];
	struct evtoc		*evp;
	struct buf		*bp;
	struct ipart		part[FD_NUMPART];
	struct ipart		*bpart,
				*ospart = (struct ipart *)0;
	u_int			i, n;

	if (hdgotvtoc[unit]) {
		if (hdgotvtoc[unit] == -1)
			sleep(&hdgotvtoc[unit],PRIBIO);
		return;
	}
	setcontroller(unit);		/* start sane */
	hdgotvtoc[unit] = -1;
	bp = geteblk(SECSIZE);		/* for evtoc */
	/* make partition 0 the whole disk in case of failure then get pdinfo*/
	lp->d_partitions[PART_DISK].p_offset = 0;
	lp->d_partitions[PART_DISK].p_size = lp->d_secperunit;
	/* get active partition */
	bp->b_flags = B_READ | B_MD1;
	bp->b_blkno = 0;
	bp->b_dev = WHOLE_DISK(unit);	/* C partition */
	bp->b_bcount = SECSIZE;
	hdstrategy(bp);
	biowait(bp);
	if (bp->b_flags & B_ERROR) {
		printf("hd%d: can't read sector 0 boot record\n",unit);
		goto done;
	}
	if (((struct mboot *)bp->b_un.b_addr)->signature != 0xaa55) {
		printf("hd%d: sector 0 corrupt, can't read boot record\n",unit);
		goto done;
	}
	bcopy(((struct mboot *)bp->b_un.b_addr)->parts,
	      part, FD_NUMPART * sizeof (struct ipart));
	/*
	 * In search of the bios partition
	 */
	bpart = part;
#ifdef	old
	{
		int nmach = 0;
		struct ipart		*mabr = (struct ipart *) 0,
					*mbr = (struct ipart *) 0;
		for (i = 0; i < 4; i++, bpart++) {
			if (bpart->sys_ind == UNIXOS) {
				if (!nmach++)
					mabr = bpart;
				if (bpart->boot_ind == ACTIVE)
					mbr = bpart;
			}
		}
		if ((int)mabr)
			bpart = mabr;
		else if (nmach == 1)
			bpart = mbr;
		else if (!nmach)
			bpart = &mempty;
		else {
			printf("More than one mach partition and none active.\n");
			printf("Try to fix this from floppies or DOS.\n");
			for (;;);
		}
	}
#else	/* old */
	for (i = 0; i < 4; i++, bpart++) {
		LABEL_DEBUG(1,print_dos_partition(i, bpart));
		if (bpart->systid == UNIXOS || bpart->systid == BSDOS) {
			if ((int)ospart) {
				if (bpart->bootid == ACTIVE)
					ospart = bpart;
			} else
				ospart = bpart;
		}

	}
	if (!ospart)
		goto done;
#endif	/* old */
	OS = ospart->systid;
	if (OS == UNIXOS)
		printf("hd%d: MACH OS ", unit);
	if (OS == BSDOS)
		printf("hd%d: XXXBSD OS ", unit);
	/* set correct partition information */
	lp->d_partitions[PART_DISK].p_offset = ospart->relsect;
	lp->d_partitions[PART_DISK].p_size = ospart->numsect;
	if (hd_bios_bsd_label(unit, bp, lp, ospart))
		goto done;
	/* get evtoc out of active unix partition */
	bp->b_flags = B_READ;
	bp->b_blkno = PDLOCATION;
	hdstrategy(bp);
	biowait(bp);
	if (bp->b_flags & B_ERROR) {
		printf("hd%d: can't read evtoc\n", unit);
		goto done;
	}
	if ((evp = (struct evtoc *)bp->b_un.b_addr)->sanity != VTOC_SANE) {
		printf("hd%d: evtoc corrupt\n",unit);
		goto done;
	}
	printf("LOCAL LABEL\n");
	LABEL_DEBUG(0x40,print_local_label(evp, "ev:"));

	/* pd info from disk must be more accurate than that in cmos thus
	   override hdparm and re- setcontroller() */
	n = evp->nparts;
	if (n >= MAXPARTITIONS)
		n = MAXPARTITIONS;
	fudge_bsd_label(lp, DTYPE_ESDI, evp->cyls, evp->tracks, evp->sectors, n);
	setcontroller(unit);

	/* copy info on all valid partition, zero the others */
	for (i = 0; i < n; i++) {
		if (i == PART_DISK)
			continue;
		bzero(&(lp->d_partitions[i]), sizeof (struct partition));
		lp->d_partitions[i].p_size   = evp->part[i].p_size;
		lp->d_partitions[i].p_offset = evp->part[i].p_start;
		lp->d_partitions[i].p_fstype = FS_BSDFFS;
	}
	for ( ; i < MAXPARTITIONS; i++) {
		if (i == PART_DISK)
			continue;
		bzero(&(lp->d_partitions[i]), sizeof (struct partition));
	}
	if (badblocks) {
		/* get alternate sectors out of active unix partition */
		bp->b_blkno = evp->alt_ptr/SECSIZE;
		for (i = 0; i < 4; i++) {
			bp->b_flags = B_READ;
			hdstrategy(bp);
			biowait(bp);
			if (bp->b_flags & B_ERROR) {
				printf("hd%d: can't read alternate sectors\n", unit);
				goto done;
			}
			bcopy(bp->b_un.b_addr,(char *)&alt_info[unit]+i*SECSIZE,
			      SECSIZE);
			bp->b_blkno++;
		}
		if (alt_info[unit].alt_sanity != ALT_SANITY)
			printf("hd%d: alternate sectors corrupt\n", unit);
	}
	/*
	 * Just in case
	 */
	labeloffset[unit] = 0;
	labelsector[unit] = (ospart->relsect + LBLLOC);
	lp->d_checksum = dkcksum(lp);
	LABEL_DEBUG(2,print_bsd_label(lp, "   "));

done:
	brelse(bp);
	hdgotvtoc[unit]=1;
	wakeup(&hdgotvtoc[unit]);
}

hd_bios_bsd_label(unit, bp, label, ospart)
	int			unit;
	struct buf		 *bp;
	struct disklabel 	*label;
	struct ipart		*ospart;
{
	struct disklabel	*dlp;
	int			i;

	bp->b_flags = B_READ;
	bp->b_blkno = LBLLOC;
	hdstrategy(bp);
	biowait(bp);
	if (bp->b_flags & B_ERROR) {
		printf("hd%d: can't read bsd label\n", unit);
		return FALSE;
	}
	dlp = (struct disklabel *) bp->b_un.b_addr;
	if (dlp->d_magic  != DISKMAGIC || dlp->d_magic2 != DISKMAGIC) {
		if (ospart->systid == BSDOS) {
			printf("NO BSD LABEL!!\n");
		}
		return FALSE;
	}
	printf("BSD LABEL\n");
	LABEL_DEBUG(2,print_bsd_label(dlp, "dk:"));
	if (dlp->d_flags & D_BADSECT) {
		printf("hd%d: DISK NEEDS BAD144 SUPPORT.  WILL TRY FOR LOCAL LABEL.\n",
			unit);
		return FALSE;
	}

	/*
	 * here's were we dump label
	 */
	if (allow_bsd_label == 0)
		return FALSE;

	*label = *dlp;
	setcontroller(unit);
	bzero(&alt_info[unit], sizeof (struct alt_info));

	labeloffset[unit] = 0;
	labelsector[unit] = (ospart->relsect + LBLLOC);

	LABEL_DEBUG(0x20,print_bsd_label(label, "   "));
	return TRUE;
}

hdwritelabel(unit)
{
	struct buf		*bp;

	if (alt_info[unit].alt_trk.alt_used || alt_info[unit].alt_sec.alt_used) {
		printf("hd%x: can not write label: alt trk = %d, alt sec = %d\n",
			unit, alt_info[unit].alt_trk.alt_used, alt_info[unit].alt_sec.alt_used);
		return (EINVAL);
	}
	printf("hd%x: hdwritelabel replacing label at %d\n", unit, labelsector[unit]);

	bp = geteblk(SECSIZE);
	bp->b_flags = B_READ | B_MD1;	/* MD1 is be absolute */
	bp->b_blkno = labelsector[unit];
	bp->b_dev = WHOLE_DISK(unit);	/* C partition */
	bp->b_bcount = SECSIZE;
	hdstrategy(bp);
	biowait(bp);
	if (bp->b_flags & B_ERROR) {
		printf("hd%d hdwritelabel(): read failure\n", unit);
		brelse(bp);
		return (ENXIO);
	}

	*((struct disklabel *) &(((char *)bp->b_un.b_addr)[labeloffset[unit]]))
		= label[unit];

	bp->b_flags = B_WRITE | B_MD1;	/* MD1 is be absolute */
	bp->b_blkno = labelsector[unit];
	bp->b_dev = WHOLE_DISK(unit);	/* C partition */
	bp->b_bcount = SECSIZE;
	hdstrategy(bp);
	biowait(bp);
	if (bp->b_flags & B_ERROR) {
		printf("hd%d: hdwritelabel() write failure\n", unit);
		brelse(bp);
		return ENXIO;
	}
	brelse(bp);

	return D_SUCCESS;
}

io_return_t
hdsetlabel(dev_t dev, int flavor, int *data, unsigned int count)
{
	io_return_t	errcode = D_SUCCESS;
	int		unit = UNIT(dev);
	struct disklabel*lp = &label[unit];

	switch (flavor) {
	/* set */
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
	}
	return errcode;
}
