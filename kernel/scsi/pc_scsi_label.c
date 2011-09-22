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
 * $Log:	pc_scsi_label.c,v $
 * Revision 2.8  93/08/10  16:02:19  mrt
 * 	Moved here from i386at/grab_bob_label.c as the code now works
 * 	on scsi disks on many platforms.
 * 		We now use bsd labels internally and convert from "local labels"
 * 		if we find them on the disk.
 * 	[93/08/05            rvb]
 * 
 * Revision 2.7  93/03/09  11:18:39  danner
 * 	If there is no recognizable label, we must return sensible
 * 	defaults for getstatus or freak out diskutil.  Alternatively,
 * 	we could ship a new diskutil.
 * 	[93/02/23            rvb]
 * 
 * Revision 2.6  92/07/09  22:53:52  rvb
 * 	No need to print out the relation between capacity and stated geometry.
 * 	Also adjust GETPARMS answer's to the adaptec's view of reality.
 * 	[92/06/18            rvb]
 * 	tgt->unit_no is not set up correctly now; should use masterno instead
 * 	[92/05/22  17:31:10  rvb]
 * 
 *	Fixed by Jukka Virtanen <jtv@kampi.hut.fi>
 * 
 * Revision 2.5  92/04/03  12:08:41  rpd
 * 	Reviewed with and blessed (?) by af.
 * 	[92/04/01            rvb]
 * 	Merged bernadat's change to mainline.  Made conditional
 * 	on i386 vs AT386.
 * 	[92/03/16            rvb]
 * 	Add AT specific setstatus/getstatus flavors
 * 	to support vtoc, diskutil, verify ...
 * 	[92/03/04            bernadat]
 * 
 * Revision 2.4  92/02/23  22:43:04  elf
 * 	Dropped first scsi_softc argument.
 * 	[92/02/22  19:58:21  af]
 * 
 * Revision 2.3  92/02/19  16:29:46  elf
 * 	On 25-Jan, did not consider NO ACTIVE mach parition.
 * 	[92/01/31            rvb]
 * 
 * 	Add "BIOS" support -- always boot mach partition NOT active one.
 * 	[92/01/25            rvb]
 * 
 * Revision 2.2  91/08/24  11:57:41  af
 * 	Temporarily created, till we evaporate the religious issues.
 * 	[91/08/02            af]
 * 
 */
/* This goes away as soon as we move it in the Ux server */

#include <mach/std_types.h>
#include <scsi/compat_30.h>
#include <scsi/scsi.h>
#include <scsi/scsi_defs.h>
#include <scsi/rz.h>
#include <scsi/rz_labels.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#define LABEL_DEBUG(x,y) if (label_flag&x) y

#include <i386at/disk.h>
#include <i386at/hdreg.h>
#include <device/device_types.h>
#include <device/disk_status.h>

grab_bob_label(tgt, label, ior, bpart)
	target_info_t		*tgt;
	struct disklabel	*label;
	io_req_t		ior;	
	struct bios_partition_info *bpart;
{
	register int		i, n;
	struct evtoc		*evp;
	struct bios_partition_info *ospart = (struct bios_partition_info *)0;
	struct bios_partition_info *first_part = bpart;
	int			upart;
	int 			dev_bsize = tgt->block_size;
	extern int		label_flag;

#ifdef	old
	{
		int nmach = 0;
		int	mabr = 0,
			mbr = 0;

		for (i = 0; i < 4; i++) {
			if (bpart[i].systid == UNIXOS) {
				if (!nmach++)
					mbr = i;
				if (bpart[i].bootid == BIOS_BOOTABLE)
					mabr = i;
			}
		}
		if (mabr)
			i = mabr;
		else if (nmach == 1)
			i = mbr;
		else if (nmach == 0)
			return 0;	/* DOS, no Mach */
		else {
			printf("Warning: More than one Mach partition and none active.\n");
			printf("Using DOS partition #%d\n", mbr);
			i = mbr;
		}
	}
#else	/* old */
	for (i = 0; i < 4; i++, bpart++) {
		LABEL_DEBUG(1, print_dos_partition(i, bpart));
		if (bpart->systid == UNIXOS || bpart->systid == BSDOS) {
			if ((int) ospart) {
				if (bpart->bootid == ACTIVE)
					ospart = bpart;
			} else
				ospart = bpart;
		}
	}
	if (!((int) ospart))
		return 0;
#endif	/* old */
	OS = ospart->systid;
	if (OS == UNIXOS)
		printf("sd%d: MACH OS ", tgt->target_id);
	if (OS == BSDOS)
		printf("sd%d: XXXBSD OS ", tgt->target_id);
	upart = ospart - first_part;
	tgt->dev_info.disk.labeloffset = 0;
	tgt->dev_info.disk.labelsector = (ospart->offset + LBLLOC) * 512 / dev_bsize;
	/*
	 * In rz_labels:rz_bios_label(), we have set up DOS partitions 0-3 to correspond
	 * to the parittions 0-3 in the label.  Hence we use the "upart" above, in
	 * the line below.
	 */
        ior->io_unit = (tgt->masterno<<6) + (tgt->target_id<<3) + (upart);
	ior->io_count = DEV_BSIZE;
	ior->io_error = 0;
	ior->io_op = IO_READ;
	ior->io_recnum = 29;	/* that's where the vtoc is */
	scdisk_strategy(ior);
	iowait(ior);
	evp = (struct evtoc *)ior->io_data;

	if (evp->sanity != VTOC_SANE) {
		printf("vtoc insane\n");
		return 0;
	}

	printf("LOCAL LABEL ");
	LABEL_DEBUG(4,print_local_label(evp, "ev:"));

	bcopy(evp->label, label->d_packname, 16);/* truncates, too bad */
	n = evp->nparts;
	if (n >= MAXPARTITIONS)
		n = MAXPARTITIONS;
	fudge_bsd_label(label, DTYPE_SCSI, evp->cyls, evp->tracks, evp->sectors, n);
#ifdef	you_care
	if (label->d_secperunit != evp->tracks * evp->sectors * evp->cyls)
		printf("sd%d: capacity (%d) != S*H*C (%d)\n", tgt->target_id,
			label->d_secperunit, evp->tracks * evp->sectors * evp->cyls);
#endif
	/* copy info on first MAXPARTITIONS */
		/* "c" is never read; always calculated */
	label->d_partitions[2].p_size = label->d_partitions[upart].p_size;
	label->d_partitions[2].p_offset = label->d_partitions[upart].p_offset;
	for (i = 0; i < n ; i++) {
		if (i == 2)
			continue;
		bzero(&(label->d_partitions[i]), sizeof (struct partition));
		label->d_partitions[i].p_size = evp->part[i].p_size;
		label->d_partitions[i].p_offset = evp->part[i].p_start;
		label->d_partitions[i].p_fstype = FS_BSDFFS;
	}
	for ( ; i < MAXPARTITIONS; i++) {
		if (i == 2)
			continue;
		bzero(&(label->d_partitions[i]), sizeof (struct partition));
	}
		/* whole disk */
	label->d_partitions[MAXPARTITIONS].p_size = -1;
	label->d_partitions[MAXPARTITIONS].p_offset = 0;
	label->d_checksum = dkcksum(label);
	LABEL_DEBUG(0x20,print_bsd_label(label, "   "));
	return 1;
}

int scsi_abs_sec = -1;
int scsi_abs_count = -1;

scsi_rw_abs(dev, data, rw, sec, count)
	dev_t		dev;
{
	io_req_t	ior;
	io_return_t	error;

	io_req_alloc(ior,0);
	ior->io_next = 0;
	ior->io_unit = dev & (~(MAXPARTITIONS-1));	/* sort of */
	ior->io_unit |= PARTITION_ABSOLUTE;
	ior->io_data = (io_buf_ptr_t)data;
	ior->io_count = count;
	ior->io_recnum = sec;
	ior->io_error = 0;
	if (rw == IO_READ)
		ior->io_op = IO_READ;
	else
		ior->io_op = IO_WRITE;
	scdisk_strategy(ior);
	iowait(ior);
	error = ior->io_error;
	io_req_free(ior);
	return(error);
}

io_return_t
scsi_i386_get_status(dev, tgt, flavor, status, status_count)
int		dev;
target_info_t	*tgt;
int		flavor;
dev_status_t	status;
unsigned int	*status_count;
{

	switch (flavor) {
	case V_GETPARMS: {
		struct disklabel *lp = &tgt->dev_info.disk.l;
		struct disk_parms *dp = (struct disk_parms *)status;
		extern struct disklabel scsi_default_label;
		int part = rzpartition(dev);

		if (*status_count < sizeof (struct disk_parms)/sizeof(int))
			return (D_INVALID_OPERATION);
		dp->dp_type = DPT_WINI; 
		dp->dp_secsiz = lp->d_secsize;
		if (lp->d_nsectors == scsi_default_label.d_nsectors &&
		    lp->d_ntracks == scsi_default_label.d_ntracks &&
		    lp->d_ncylinders == scsi_default_label.d_ncylinders) {
		    	/* I guess there is nothing there */
			/* Well, then, Adaptec's like ... */
			dp->dp_sectors = 32;
			dp->dp_heads = 64;
			dp->dp_cyls = lp->d_secperunit / 64 / 32 ;
		} else {
			dp->dp_sectors = lp->d_nsectors;
			dp->dp_heads = lp->d_ntracks;
			dp->dp_cyls = lp->d_ncylinders;
		}

		dp->dp_dossectors = 32;
		dp->dp_dosheads = 64;
		dp->dp_doscyls = lp->d_secperunit / 64 / 32;
		dp->dp_ptag = 0;
		dp->dp_pflag = 0;
		dp->dp_pstartsec = lp->d_partitions[part].p_offset;
		dp->dp_pnumsec = lp->d_partitions[part].p_size;
		*status_count = sizeof(struct disk_parms)/sizeof(int);
		break;
	}
	case V_RDABS:
		if (*status_count < DEV_BSIZE/sizeof (int)) {
			printf("RDABS bad size %x", *status_count);
			return (D_INVALID_OPERATION);
		}
		if (scsi_rw_abs(dev, status, IO_READ, scsi_abs_sec, DEV_BSIZE) != D_SUCCESS)
			return(D_INVALID_OPERATION);
		*status_count = DEV_BSIZE/sizeof(int);
		break;
	case V_VERIFY: {
		int count = scsi_abs_count * DEV_BSIZE;
		int sec = scsi_abs_sec;
		char *scsi_verify_buf;
#include "vm/vm_kern.h"

		(void) kmem_alloc(kernel_map, &scsi_verify_buf, PAGE_SIZE);

		*status = 0;
		while (count > 0) {
			int xcount = (count < PAGE_SIZE) ? count : PAGE_SIZE;
			if (scsi_rw_abs(dev, scsi_verify_buf, IO_READ, sec, xcount) != D_SUCCESS) {
				*status = BAD_BLK;
				break;
			} else {
				count -= xcount;
				sec += xcount / DEV_BSIZE;
			}
	        }
		(void) kmem_free(kernel_map, scsi_verify_buf, PAGE_SIZE);
		*status_count = 1;
		break;
	}
	default:
		return(D_INVALID_OPERATION);
	}
	return D_SUCCESS;
}

io_return_t
scsi_i386_set_status(dev, tgt, flavor, status, status_count)
int		dev;
target_info_t	*tgt;
int		flavor;
int 		*status;
unsigned int	status_count;
{
	io_req_t	ior;

	switch (flavor) {
	case V_SETPARMS:
		printf("scsdisk_set_status: invalid flavor V_SETPARMS\n");
		return(D_INVALID_OPERATION);
		break;
	case V_REMOUNT:
		tgt->flags &= ~TGT_ONLINE;
		break;
	case V_ABS:
		scsi_abs_sec = status[0];
		if (status_count == 2)
			scsi_abs_count = status[1];
		break;
	case V_WRABS:
		if (status_count < DEV_BSIZE/sizeof (int)) {
			printf("RDABS bad size %x", status_count);
			return (D_INVALID_OPERATION);
		}
		if (scsi_rw_abs(dev, status, IO_WRITE, scsi_abs_sec, DEV_BSIZE) != D_SUCCESS)
			return(D_INVALID_OPERATION);
		break;
	default:
		return(D_INVALID_OPERATION);
	}
	return D_SUCCESS;
}
