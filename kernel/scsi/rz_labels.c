/* 
 * Mach Operating System
 * Copyright (c) 1993,1992,1991,1990,1989 Carnegie Mellon University
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
 * $Log:	rz_labels.c,v $
 * Revision 2.11  93/11/17  18:44:36  dbg
 * 	Need to always keep d_partitions[MAXPARTITIONS] correct, so
 * 	always reset it ourselves.
 * 	[93/10/08            rvb]
 * 
 * Revision 2.10  93/08/10  15:19:51  mrt
 * 	We now use bsd labels internally and convert from "local labels"
 * 	if we find them on the disk.
 * 	[93/08/03  20:36:46  rvb]
 * 
 * Revision 2.9  93/05/10  21:22:41  rvb
 * 	Removed depends on DEV_BSIZE.
 * 	[93/05/06  10:09:09  af]
 * 
 * Revision 2.8  93/03/09  11:18:35  danner
 * 	If there is no recognizable label, we must set p_size of the
 * 	MAXPARTITION slice.  We use MAXPARTITION to do ABSOLUTE reads.
 * 	[93/02/23            rvb]
 * 
 * Revision 2.7  92/08/03  17:53:58  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.6  92/05/21  17:23:47  jfriedl
 * 	tried prototypes.
 * 	[92/05/20            jfriedl]
 * 
 * Revision 2.5  92/04/03  12:10:09  rpd
 * 	Let "grab_bob_label" return failure so we take the default.
 * 	[92/04/01            rvb]
 * 
 * Revision 2.4  92/02/23  22:44:31  elf
 * 	Actually, scan for all possible (bogus) label formats.
 * 	This makes it possible to cross-mount disks even if
 * 	they do not have the standard BSD label.
 * 	[92/02/22            af]
 * 
 * Revision 2.3  91/08/24  12:28:06  af
 * 	Created, splitting out DEC-specific code from rz_disk.c and
 * 	adding some more.
 * 	[91/06/26            af]
 * 
 */
/*
 *	File: rz_labels.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/91
 *
 *	Routines for various vendor's disk label formats.
 */

#include <platforms.h>

#include <mach/std_types.h>
#include <scsi/compat_30.h>
#include <scsi/scsi_defs.h>
#include <scsi/rz_labels.h>

#define LABEL_DEBUG(x,y) if (label_flag&x) y


/*
 * Find and convert from a DEC label to BSD
 */
boolean_t
rz_dec_label(
	target_info_t		*tgt,
	struct disklabel	*label,
	io_req_t		ior)
{
	/* here look for a DEC label */
	register scsi_dec_label_t	*part;
	char				*data;
	int				i, dev_bsize = tgt->block_size;

	ior->io_count = dev_bsize;
	ior->io_op = IO_READ;
	tgt->ior = ior;
	scdisk_read( tgt, DEC_LABEL_BYTE_OFFSET/dev_bsize, ior);
	iowait(ior);
	data = (char *)ior->io_data;
	part = (scsi_dec_label_t*)&data[DEC_LABEL_BYTE_OFFSET%dev_bsize];
	if (part->magic == DEC_LABEL_MAGIC) {
		if (scsi_debug)
		printf("{Using DEC label}");
		for (i = 0; i < 8; i++) {
			label->d_partitions[i].p_size = part->partitions[i].n_sectors;
			label->d_partitions[i].p_offset = part->partitions[i].offset;
		}
		return TRUE;
	}
	return FALSE;
}

/*
 * Find and convert from a Omron label to BSD
 */
boolean_t
rz_omron_label(
	target_info_t		*tgt,
	struct disklabel	*label,
	io_req_t		ior)
{
	/* here look for an Omron label */
	register scsi_omron_label_t	*part;
	char				*data;
	int				i, dev_bsize = tgt->block_size;

	ior->io_count = dev_bsize;
	ior->io_op = IO_READ;
	tgt->ior = ior;
	scdisk_read( tgt, OMRON_LABEL_BYTE_OFFSET/dev_bsize, ior);
	iowait(ior);
	data = (char *)ior->io_data;
	part = (scsi_omron_label_t*)&data[OMRON_LABEL_BYTE_OFFSET%dev_bsize];
	if (part->magic == OMRON_LABEL_MAGIC) {
		if (scsi_debug)
		printf("{Using OMRON label}");
		for (i = 0; i < 8; i++) {
			label->d_partitions[i].p_size = part->partitions[i].n_sectors;
			label->d_partitions[i].p_offset = part->partitions[i].offset;
		}
		bcopy(part->packname, label->d_packname, 16);
		label->d_ncylinders = part->ncyl;
		label->d_acylinders = part->acyl;
		label->d_ntracks = part->nhead;
		label->d_nsectors = part->nsect;
		/* Many disks have this wrong, therefore.. */
#if 0
		label->d_secperunit = part->maxblk;
#else
		label->d_secperunit = label->d_ncylinders * label->d_ntracks *
					label->d_nsectors;
#endif
		return TRUE;
	}
	return FALSE;
}

/*
 * Find and convert from a Intel BIOS label to BSD
 */
extern int label_flag;
extern int OS;
#define BSDOS 165
#define LBLLOC 1

boolean_t
rz_bios_bsd_label(
	target_info_t		*tgt,
	struct disklabel	*label,
	io_req_t		ior,
	struct bios_partition_info *ospart)
{
	struct disklabel	*dlp;
	int			dev_bsize = tgt->block_size;
	int			i;
	extern int 		allow_bsd_label;

	ior->io_count = dev_bsize;
	ior->io_op = IO_READ;
	tgt->ior = ior;
	scdisk_read( tgt, (ospart->offset + LBLLOC) * 512 / dev_bsize, ior);
	iowait(ior);
	dlp = (struct disklabel *)ior->io_data;

	if (dlp->d_magic  != DISKMAGIC || dlp->d_magic2 != DISKMAGIC) {
		if (OS == BSDOS) {
			printf("NO BSD LABEL!!\n");
		}
		return FALSE;
	}

	printf("BSD LABEL ");
	LABEL_DEBUG(2,print_bsd_label(dlp, "dk:"));
	/*
	 * here's where we dump label
	 */
	if (allow_bsd_label == 0)
		return FALSE;

	i = label->d_secperunit;
	*label = *dlp;
	label->d_secperunit = i;
		/*
		 * The disklabel program should insure the invariant below
		 * when it writes a label.  But it does not seem to.
		 */
        label->d_partitions[MAXPARTITIONS].p_size = -1;
        label->d_partitions[MAXPARTITIONS].p_offset = 0;
        label->d_checksum = dkcksum(label);

	tgt->dev_info.disk.labeloffset = 0;
	tgt->dev_info.disk.labelsector = (ospart->offset + LBLLOC) * 512 / dev_bsize;

	LABEL_DEBUG(0x20,print_bsd_label(dlp, "   "));
	return TRUE;
}

boolean_t
rz_bios_label(
	target_info_t		*tgt,
	struct disklabel	*label,
	io_req_t		ior)
{
	/* here look for a BIOS label */
	char				*data;
	int				i, dev_bsize = tgt->block_size;
	scsi_bios_label_t		part;

	ior->io_count = dev_bsize;
	ior->io_op = IO_READ;
	tgt->ior = ior;
	scdisk_read( tgt, BIOS_LABEL_BYTE_OFFSET/dev_bsize, ior);
	iowait(ior);
	data = (char *)ior->io_data;
	/*
	 * It's safer to just copy the data here and not worry about
	 * scdisk_read's down the line will clobber the buffer.
	 */
	part = *(scsi_bios_label_t*)&data[BIOS_LABEL_BYTE_OFFSET%dev_bsize];
	if (((scsi_bios_label_t*)&data[BIOS_LABEL_BYTE_OFFSET%dev_bsize])->magic 
	     == BIOS_LABEL_MAGIC) {
		struct bios_partition_info *bpart, *ospart = (struct bios_partition_info *)0;
		if (scsi_debug)
			printf("{Using BIOS label}");
		for (i = 0; i < 4; i++) {
			bpart = &(((struct bios_partition_info *)part.partitions)[i]);
			LABEL_DEBUG(1,print_dos_partition(i, bpart));
			label->d_partitions[i].p_size = bpart->n_sectors;
			label->d_partitions[i].p_offset = bpart->offset;
			if (bpart->systid == UNIXOS || bpart->systid == BSDOS) {
				if (ospart != 0) {
					if (bpart->bootid == BIOS_BOOTABLE)
						ospart = bpart;
				} else
					ospart = bpart;
			}
		}
		tgt->dev_info.disk.l.d_partitions[MAXPARTITIONS].p_offset= 0;
		tgt->dev_info.disk.l.d_partitions[MAXPARTITIONS].p_size  = -1;

		if (ospart == 0)
			return FALSE;
		OS = ospart->systid;
		if (OS == UNIXOS)
			printf("sd%d: MACH OS ", tgt->target_id);
		if (OS == BSDOS)
			printf("sd%d: XXXBSD OS ", tgt->target_id);

		if (rz_bios_bsd_label(tgt, label, ior, ospart))
			return TRUE;

#ifdef	AT386	/* this goes away real fast */
		if (!grab_bob_label(tgt, label, ior,
				    ((struct bios_partition_info *)part.partitions)))
			return FALSE;		/* take default setup of "a" and "c" */
#else
		label->d_npartitions = 4;
#endif
		label->d_bbsize = BIOS_BOOT0_SIZE;
		return TRUE;
	}
	return FALSE;
}

/*
 * Try all of the above
 */
boolean_t rz_vendor_label(
	target_info_t		*tgt,
	struct disklabel	*label,
	io_req_t		ior)
{
	/* If for any reason there might be problems someday.. */
#ifdef	VENDOR_LABEL
	if (VENDOR_LABEL( tgt, label, ior)) return TRUE;
#endif	/*VENDOR_LABEL*/

	if (rz_dec_label( tgt, label, ior)) return TRUE;
	if (rz_bios_label( tgt, label, ior)) return TRUE;
	if (rz_omron_label( tgt, label, ior)) return TRUE;
	return FALSE;
}

