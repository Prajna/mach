/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	rz_labels.h,v $
 * Revision 2.11  93/01/14  17:55:34  danner
 * 	Alpha labels.
 * 	[92/12/01            af]
 * 
 * Revision 2.10  92/02/23  22:44:34  elf
 * 	The vendor-label-searching function is now fixed.
 * 	[92/02/22  19:03:13  af]
 * 
 * Revision 2.9  91/08/24  12:28:10  af
 * 	More vendor's labels.
 * 	[91/08/02  03:50:34  af]
 * 
 * Revision 2.8  91/07/09  23:22:43  danner
 * 	Support for DEC, BSD, Omron labels.
 * 	[91/07/09  11:15:23  danner]
 * 
 * Revision 2.7  91/06/19  11:57:09  rvb
 * 	File moved here from mips/PMAX since it is now "MI" code, also
 * 	used by Vax3100 and soon -- the omron luna88k.
 * 	[91/06/04            rvb]
 * 
 * Revision 2.6  91/05/14  17:26:52  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/05/13  06:04:35  af
 * 	Moved BSD/OSF1 stuff elsewhere, and the copyright that goes with
 * 	it.
 * 	[91/05/03            af]
 * 
 * Revision 2.4  91/02/05  17:43:56  mrt
 * 	Added author notices
 * 	[91/02/04  11:17:04  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:15:43  mrt]
 * 
 * Revision 2.3  90/12/05  23:34:02  af
 * 	Added BSD/OSF1 labels and the copyright that goes with it.
 * 	[90/11/26            af]
 * 
 * Revision 2.1.1.1  90/11/01  03:44:04  af
 * 	Created.
 * 	[90/10/22            af]
 */
/*
 *	File: rz_labels.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/90
 *
 *	Definitions of various vendor's disk label formats.
 */

#ifndef	_SCSI_RZ_LABELS_H_
#define	_SCSI_RZ_LABELS_H_

/*
 * This function looks for, and converts to BSD format
 * a vendor's label.  It is only called if we did not
 * find a standard BSD label on the disk pack.
 */
extern boolean_t	rz_vendor_label();

/*
 * Definition of the DEC disk label,
 * which is located (you guessed it)
 * at the end of the 4.3 superblock.
 */

struct dec_partition_info {
	unsigned int	n_sectors;	/* how big the partition is */
	unsigned int	offset;		/* sector no. of start of part. */
};

typedef struct {
	int	magic;
#	define	DEC_LABEL_MAGIC		0x032957
	int	in_use;
	struct  dec_partition_info partitions[8];
} scsi_dec_label_t;

/*
 * Physical location on disk.
 * This is independent of the filesystem we use,
 * although of course we'll be in trouble if we
 * screwup the 4.3 SBLOCK..
 */

#define	DEC_LABEL_BYTE_OFFSET	((2*8192)-sizeof(scsi_dec_label_t))


/*
 * Definitions for the primary boot information
 * This is common, cuz the prom knows it.
 */

typedef struct {
	int		pad[2];
	unsigned int	magic;
#	define		DEC_BOOT0_MAGIC	0x2757a
	int		mode;
	unsigned int	phys_base;
	unsigned int	virt_base;
	unsigned int	n_sectors;
	unsigned int	start_sector;
} scsi_dec_boot0_t;

typedef struct {
	scsi_dec_boot0_t	vax_boot;
					/* BSD label still fits in pad */
	char			pad[0x1e0-sizeof(scsi_dec_boot0_t)];
	unsigned long		block_count;
	unsigned long		starting_lbn;
	unsigned long		flags;
	unsigned long		checksum; /* add cmpl-2 all but here */
} scsi_alpha_boot0_t;

/*
 * Definition of the Omron disk label,
 * which is located at sector 0. It
 * _is_ sector 0, actually.
 */
struct omron_partition_info {
	unsigned long	offset;
	unsigned long	n_sectors;
};

typedef struct {
	char		packname[128];	/* in ascii */

	char		pad[512-(128+8*8+11*2+4)];

	unsigned short	badchk;	/* checksum of bad track */
	unsigned long	maxblk;	/* # of total logical blocks */
	unsigned short	dtype;	/* disk drive type */
	unsigned short	ndisk;	/* # of disk drives */
	unsigned short	ncyl;	/* # of data cylinders */
	unsigned short	acyl;	/* # of alternate cylinders */
	unsigned short	nhead;	/* # of heads in this partition */
	unsigned short	nsect;	/* # of 512 byte sectors per track */
	unsigned short	bhead;	/* identifies proper label locations */
	unsigned short	ppart;	/* physical partition # */
	struct omron_partition_info
			partitions[8];

	unsigned short	magic;	/* identifies this label format */
#	define	OMRON_LABEL_MAGIC	0xdabe

	unsigned short	cksum;	/* xor checksum of sector */

} scsi_omron_label_t;

/*
 * Physical location on disk.
 */

#define	OMRON_LABEL_BYTE_OFFSET	0


/*
 * Definition of the i386AT disk label, which lives inside sector 0.
 * This is the info the BIOS knows about, which we use for bootstrapping.
 * It is common across all disks known to BIOS, not just SCSI.
 */

struct bios_partition_info {

	unsigned char	bootid;	/* bootable or not */
#	define BIOS_BOOTABLE	128

	unsigned char	beghead;/* beginning head, sector, cylinder */
	unsigned char	begsect;/* begcyl is a 10-bit number. High 2 bits */
	unsigned char	begcyl;	/*     are in begsect. */

	unsigned char	systid;	/* filesystem type */
#	define	UNIXOS		99

	unsigned char	endhead;/* ending head, sector, cylinder */
	unsigned char	endsect;/* endcyl is a 10-bit number.  High 2 bits */
	unsigned char	endcyl;	/*     are in endsect. */

	unsigned long	offset;
	unsigned long	n_sectors;
};

typedef struct {
/*	struct bios_partition_info	bogus compiler alignes wrong
			partitions[4];
*/
	char		partitions[4*sizeof(struct bios_partition_info)];
	unsigned short	magic;
#	define	BIOS_LABEL_MAGIC	0xaa55
} scsi_bios_label_t;

/*
 * Physical location on disk.
 */

#define	BIOS_LABEL_BYTE_OFFSET	446

/*
 * Definitions for the primary boot information
 * This _is_ block 0
 */

#define	BIOS_BOOT0_SIZE	BIOS_LABEL_BYTE_OFFSET

typedef struct {
	char		boot0[BIOS_BOOT0_SIZE];	/* boot code */
/*	scsi_bios_label_t label;	bogus compiler alignes wrong */
	char		label[sizeof(scsi_bios_label_t)];
} scsi_bios_boot0_t;


#endif	_SCSI_RZ_LABELS_H_

