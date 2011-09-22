/*
 * Mach Operating System
 * Copyright (c) 1992, 1991 Carnegie Mellon University
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
 * $Log:	disk.c,v $
 * Revision 2.4.1.1  94/03/18  13:15:50  rvb
 * 	Allow partition (say dos) to be specified
 * 
 * Revision 2.4  93/08/10  15:56:55  mrt
 * 	DEBUG -> LABEL_DEBUG
 * 	"better" scheme for picking os partition.
 * 	[93/08/02            rvb]
 * 	Handle xxxbsd label
 * 	[93/07/09            rvb]
 * 
 * Revision 2.3  93/05/10  17:47:04  rvb
 * 	Use < vs " for include-ed files
 * 	[93/05/05  11:41:20  rvb]
 * 
 * Revision 2.2  92/04/04  11:35:49  rpd
 * 	Fabricated from 3.0 bootstrap and 2.5 boot disk.c:
 * 	with support for scsi
 * 	[92/03/30            mg32]
 * 
 */

#include <boot.h>
#include <i386at/disk.h>
#include <device/device_types.h>
#include <device/disk_status.h>

#define	BIOS_DEV_FLOPPY	0x0
#define	BIOS_DEV_WIN	0x80

#define BPS		512
#define	SPT(di)		((di)&0xff)
#define	HEADS(di)	((((di)>>8)&0xff)+1)
#define	BDEV(dev,unit)	(((dev)==1 ? BIOS_DEV_FLOPPY:BIOS_DEV_WIN)+(unit))

char *devs[] = {"hd", "fd", "wt", "sd", "ha", 0};

struct alt_info alt_info;
int spt, spc;
int OS;

char *iodest;
struct fs *fs;
struct inode inode;
int unit, part, maj, boff, poff, bnum, cnt;

loadpart(idx)
{

	struct ipart ipart;
	int dev = inode.i_dev;
	int i, sector, di = get_diskinfo(BDEV(dev,unit));

	spc = (spt = SPT(di)) * HEADS(di);
	if (dev == 1) {
		boff = 0;
		part = (spt == 15 ? 3 : 1);
		printf("loadpart: floppy: tilt\n");
		return 1;
	} else {
		Bread(dev, unit, 0);
		ipart = ((struct ipart *)((struct mboot *)0)->parts)[idx];

		/* get first block */
		sector = ipart.relsect;
#ifdef	DEBUG
		printf("Bread(dev %d, unit %d, sector %d) -> ",
			dev, unit, sector); 
#endif	/* DEBUG */
#define ADDR ((int*)(0x7c00-0x1000))
		Bread(dev, unit, sector);
		bcopy(0, ADDR, BPS);
#ifdef	DEBUG
		printf("%x\n", *ADDR);
#endif	/* DEBUG */
	}
	return 0;
}

devopen()
{
	char *altptr;
	struct ipart *iptr, *osptr = (struct ipart *)0;
	int dev = inode.i_dev;
	int i, sector, di = get_diskinfo(BDEV(dev,unit));
	struct ipart ipart;
	spc = (spt = SPT(di)) * HEADS(di);
	if (dev == 1) {
		boff = 0;
		part = (spt == 15 ? 3 : 1);
	} else {
		Bread(dev, unit, 0);
		iptr = (struct ipart *)((struct mboot *)0)->parts;
		for (i = 0; i < FD_NUMPART; i++, iptr++) {
#ifdef	LABEL_DEBUG
			if (iflag & I_DOS)
				print_dos_partition(i, iptr);
#endif	/* LABEL_DEBUG */
			if (iptr->systid == UNIXOS || iptr->systid == BSDOS) {
				if ((int) osptr) {
					if (iptr->bootid == ACTIVE)
						osptr = iptr;
				} else
					osptr = iptr;
			}
		}
		if (!((int)osptr))
			return 1;
		OS = osptr->systid; 
		if (OS == UNIXOS)
			printf("MACH OS ");
		if (OS == BSDOS)
			printf("XXXBSD OS ");
		ipart = *osptr;

		/* first choice bsd label */
		sector = ipart.relsect + LBLLOC;
		Bread(dev, unit, sector);
		if (((struct disklabel *)0)->d_magic  != DISKMAGIC ||
		    ((struct disklabel *)0)->d_magic2 != DISKMAGIC) {
			if (OS == BSDOS) {
				printf("NO BSD LABEL!!\n");
				return 1;
			}
		} else {
			struct disklabel *dlp = (struct disklabel *)0;

			printf("BSD LABEL ");
#ifdef	LABEL_DEBUG
			if (iflag & I_LABEL) 
				print_bsd_label(dlp, "   ");
#endif	/* LABEL_DEBUG */
			if (!maj && dlp->d_type == DTYPE_SCSI)
				maj = 3;
			if (dlp->d_flags & D_BADSECT) {
				printf("DISK NEEDS BAD144 SUPPORT.  WILL TRY FOR LOCAL LABEL.\n");
				goto local;
			}
			if (part >= dlp->d_npartitions) {
				printf("INVALID BSD PARTITION Index!!\n");
				return 1;
			}
			boff = dlp->d_partitions[part].p_offset;
			bzero(&alt_info, sizeof(struct alt_info));
			return 0;
		}
local:
		sector = ipart.relsect + HDPDLOC;
		Bread(dev, unit, sector++);
		if (((struct evtoc *)0)->sanity != VTOC_SANE) {
			printf("vtoc insane\n");
			return 1;
		} else {
			struct evtoc *evp = (struct evtoc *)0;
			printf("LOCAL LABEL ");
#ifdef	LABEL_DEBUG
			if (iflag & I_LABEL)
				print_local_label(evp, "   ");
#endif	/* LABEL_DEBUG */
		}
		boff = ((struct evtoc *)0)->part[part].p_start;
		altptr = (char *)(&alt_info);
		for (i = 0; i++ < 4; altptr += BPS, sector++) {
			Bread(dev, unit, sector);
			bcopy(0, altptr, BPS);
		}
		if (alt_info.alt_sanity != ALT_SANITY) {
			printf("Bad alt_sanity\n");
			return 1;
		}
	}
	return 0;
}

devread()
{
	int offset, sector = bnum;
	int dev = inode.i_dev;
	for (offset = 0; offset < cnt; offset += BPS) {
		Bread(dev, unit, badsect(dev, sector++));
		bcopy(0, iodest+offset, BPS);
	}
}

Bread(dev,unit,sector)
     int dev,unit,sector;
{
	int cyl = sector/spc, head = (sector%spc)/spt, secnum = sector%spt;
	while (biosread(BDEV(dev,unit), cyl,head,secnum))
		printf("Error: C:%d H:%d S:%d\n",cyl,head,secnum);
}

badsect(dev, sector)
int dev, sector;
{
	if (OS == UNIXOS)
		return badsect_unixos(dev, sector);
	else
		return badsect_bsd(dev, sector);
}

badsect_bsd(dev, sector)
int dev, sector;
{
/* this is rather wrong */;
		return sector;
/* but it is never called for now */
}

badsect_unixos(dev, sector)
int dev, sector;
{
	int i;
	if (!dev) {
		for (i = 0; i < alt_info.alt_trk.alt_used; i++)
			if (sector/spt == alt_info.alt_trk.alt_bad[i])
				return (alt_info.alt_trk.alt_base + 
					i*spt + sector%spt);
		for (i = 0; i < alt_info.alt_sec.alt_used; i++)
			if (sector == alt_info.alt_sec.alt_bad[i])
				return (alt_info.alt_sec.alt_base + i);
	}
	return sector;
}

#ifdef	LABEL_DEBUG
print_dos_partition(i, part)
int		i;
struct ipart	*part;
{
	printf("%d: id %d(%x), start %d, size %d, Activ = %x\n",
		i, part->systid, part->systid,
		part->relsect, part->numsect,
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

print_bsd_label(lp, str)
struct disklabel	*lp;
char			*str;
{
int i;
	printf("%s sectors %d, tracks %d, cylinders %d\n",
		str, lp->d_nsectors, lp->d_ntracks, lp->d_ncylinders);
	printf("%s secpercyl %d, secperunit %d, npartitions %d\n",
		str, lp->d_secpercyl, lp->d_secperunit, lp->d_npartitions);

	for (i = 0; i < lp->d_npartitions; i++) {
		printf("%s    %c: size = %d, offset = %d\n",
			str, 'a'+i,
			lp->d_partitions[i].p_size,
			lp->d_partitions[i].p_offset);
	}
}
#endif	/* LABEL_DEBUG */
