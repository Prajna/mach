/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
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
 * $Log:	pc_label.c,v $
 * Revision 2.2  93/08/10  16:00:13  mrt
 * 	Created by Robert Baron.
 * 
 * 
 */

#include <sys/types.h>
#include <mach/std_types.h>
#include <scsi/rz_labels.h>

#include <i386at/disk.h>
#include <i386at/hdreg.h>
#include <device/device_types.h>
#include <device/disk_status.h>

int label_flag = 0;
int allow_bsd_label = 1;
int OS;

print_dos_partition(i, part)
int 				i;
struct bios_partition_info	*part;
{
	printf("%d: id %d(%x), start %d, size %d, Activ = %x\n",
		i, part->systid, part->systid,
		part->offset, part->n_sectors,
		part->bootid);
	printf("    start C%d:H%d:S%d, end C%d:H%d:S%d\n",
		part->begcyl, part->beghead, part->begsect,
		part->endcyl, part->endhead, part->endsect);
}

print_local_label(evp, str)
struct evtoc *evp;
char *str;
{
int i, j;

	printf("%s sectors %d, tracks %d, cylinders %d\n",
		str, i = evp->sectors, evp->tracks, j = evp->cyls);
	i *= evp->tracks;
	j *= i;
	printf("%s secpercyl %d, secperunit %d, npartitions %d\n",
		str, i, j, evp->nparts);

	for (i = 0; i < evp->nparts; i++) {
		printf("%s    %c: size = %d, offset = %d\n",
			str, 'a'+i,
			evp->part[i].p_size,
			evp->part[i].p_start);
	}
}

fudge_bsd_label(label, type, cyls, tracks, sectors, n)
struct disklabel	*label;
int			type, cyls, tracks, sectors, n;
{
int	secunit;

	label->d_ncylinders = cyls;
	label->d_ntracks = tracks;
	label->d_nsectors = sectors;

	secunit = label->d_ntracks * label->d_nsectors;
	label->d_secpercyl = secunit;
	secunit *= label->d_ncylinders;
	label->d_secperunit = secunit;

	label->d_partitions[MAXPARTITIONS].p_offset = 0;
	label->d_partitions[MAXPARTITIONS].p_size = secunit;

	/* ??
	 */
	label->d_secsize = SECSIZE;
	label->d_type = type;
	label->d_subtype = 0xa;
	label->d_rpm = 3600;
	label->d_interleave = 1;
	label->d_bbsize = 0x2000;		/* 8k max boot */
	label->d_sbsize = 0x2000;		/* 8k max SuperBlock */

	label->d_npartitions = n;		/* up to 'c' */

	label->d_checksum = 0;
}
