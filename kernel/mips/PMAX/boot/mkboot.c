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
 * $Log:	mkboot.c,v $
 * Revision 2.7  91/08/24  12:18:24  af
 * 	Scsi include files have moved, plus we use BSD label defs
 * 	from disk_status.h
 * 	[91/06/20            af]
 * 
 * Revision 2.6  91/06/20  22:14:44  rvb
 * 	Scsi include files have moved, plus we use BSD label defs
 * 	from disk_status.h
 * 	[91/06/20            af]
 * 
 * Revision 2.5  91/05/14  17:18:16  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:39:03  mrt
 * 	Added author notices
 * 	[91/02/04  11:10:50  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:06:13  mrt]
 * 
 * Revision 2.3  90/12/05  23:30:09  af
 * 	Created.
 * 	[90/12/02            af]
 * 
 */
/*
 *	File: mkboot.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	12/90
 *
 *	Build the 16 sector boot.
 */


#include <mach/std_types.h>
#include <stdio.h>
#include <sys/file.h>
#include <mips/coff.h>
#include <device/device_types.h>
#include <device/disk_status.h>
#include <scsi/rz_labels.h>

#ifndef	DEV_BSIZE
#define	DEV_BSIZE	512
#endif

#define	usage(pgm)	fatal(0,pgm)

boolean_t	nowrite = FALSE;
boolean_t	shutup  = FALSE;

main(argc, argv)
	char	**argv;
{
	char	*boot1, *boot2, *pgm, *labelfile;

	pgm = *argv;
	if (argc < 2)
		usage(pgm);

	/* establish defaults */
	boot1 = "16sec.boot";
	labelfile = 0;

	/* Parse options */
	while (argc > 1) {
		if (argv[1][0] != '-')
			break;

		switch(argv[1][1]) {
		case 'o':
			if ((boot1 = argv[2]) == 0)
				usage(pgm);
			argc--, argv++;
			break;
		case 'u':
			if ((labelfile = argv[2]) == 0)
				usage(pgm);
			argc--, argv++;
			break;
		case 'n':
			nowrite = TRUE;
			break;
		case 's':
			shutup = TRUE;
			break;
		}
		argc--, argv++;
	}
	if ((boot2 = argv[1]) == 0)
		usage(pgm);

	mkit(boot1, boot2, labelfile);
	return 0;
}

/*
 *	This is the one that does the work
 */
mkit(out, in, lab)
	char	*out, *in, *lab;
{
	int			fout, fin, flab;
	register int		i;
	/* This chicanery is so that we can write /dev/rrz0a in block chunks */
	union {
		char		raw[DEV_BSIZE];
		struct {
			scsi_dec_boot0_t	b;
			char	pad[LABELOFFSET-sizeof(scsi_dec_boot0_t)];
			struct disklabel	l;
		} info;
#		define	boot0	block_zero.info.b
#		define	label	block_zero.info.l
	} block_zero;

	bzero(&block_zero, sizeof(block_zero));

	/*
	 *	Open all files
	 */
	if (!nowrite) {
		fout = open(out, O_WRONLY|O_CREAT, 0644);
		if (fout < 0)
			fatal(1,out);
	}

	fin = open(in, O_RDONLY, 0);
	if (fin < 0)
		fatal(1,in);

	/*
	 *	Get the label
	 */
	if (lab == 0) {
		ask_for_label(&label, FALSE);
	} else {
		flab = open(lab, O_RDONLY, 0);
		if (flab < 0)
			fatal(1, lab);
		lseek(flab, 0, 0);
		if (read(flab, &block_zero, DEV_BSIZE) != sizeof(block_zero))
			fatal(2, lab);
	}
	
	print_label(&label);

	while (!shutup && confirm("Would you like to change anything [y/n]: ", 0)) {
		ask_for_label(&label, TRUE);
		print_label(&label);
	}

	printf("That's it then, thanks.\n");
	fflush(stdout);

	if (nowrite)
		exit(0);

	/*
	 *	Get boot0 info
	 */
	get_boot0(fin, &boot0);

	/*
	 *	Put it all together
	 */
	lseek(fout, 0, 0);
	if (write(fout, &block_zero, DEV_BSIZE) != sizeof(block_zero))
		fatal(3, out);

	lseek(fout, boot0.start_sector * DEV_BSIZE, 0);
	for (i = 0; i < boot0.n_sectors; i++) {
		char	buf[DEV_BSIZE];
		int	siz;

		siz = read(fin, buf, DEV_BSIZE);
		if ((siz != DEV_BSIZE) && (i != boot0.n_sectors-1))
			fatal(2, in);

		if (write(fout, buf, DEV_BSIZE) != DEV_BSIZE)
			fatal(3, out);
	}
	close(fout);
}

/*
 *	Compute the checksum for a label
 */
unsigned short
label_sum(l)
	struct disklabel	*l;
{
	register unsigned short	*data, *end;
	register unsigned short	ret;

	data = (unsigned short *)l;
	end = (unsigned short *)&l->d_partitions[l->d_npartitions];

	for (ret = 0; data < end; data++)
		ret ^= *data;
	return ret;
}

/*
 *	Prompt the user for the various disk parameters
 */
ask_for_label(l, updating)
	struct disklabel	*l;
	boolean_t		updating;
{
#	define LSIZ	100
	char            answ[LSIZ];
	char		deflt[LSIZ];
	int             n, i;
	static char    *would_you = "Would you like to change the %s [y/n]: ";


	if (!updating)
	    bzero(l, sizeof(*l));

	/*
	 * Label proper 
	 */
	l->d_magic = l->d_magic2 = DISKMAGIC;
	l->d_type = DTYPE_SCSI;

	if (!updating || confirm(would_you, "Disk type")) {
	    n = get_confirmed(
			      "Disk type name",
			      updating ? l->d_typename : "rz56",
			      "string_16",
			      answ, 16);
	    bcopy(answ, l->d_typename, n);

	    sprintf(deflt, "%d", l->d_subtype);
	    n = get_confirmed(
			      "SCSI protocol version",
			      updating ? deflt : "1",
			      "decimal u_int_32",
			      answ, LSIZ);
	    l->d_subtype = atoi(answ);

	}

	if (!updating || confirm(would_you, "Pack label")) {
	    n = get_confirmed(
			      "Disk pack name",
			      updating ? l->d_packname : "mydisk",
			      "string_16",
			      answ, 16);
	    bcopy(answ, l->d_packname, n);
	}

	/*
	 * disk geometry: 
	 */
	if (!updating || confirm(would_you, "Disk geometry parameters")) {

	    printf("%s%s\n", "Disk geometry parameters. ",
		       "Do not count spares in the following.");

	    sprintf(deflt, "%d", l->d_secsize);
	    n = get_confirmed(
			      "Bytes per sector",
			      updating ? deflt : "512",
			      "decimal u_int_32",
			      answ, LSIZ);
	    l->d_secsize = atoi(answ);

	    sprintf(deflt, "%d", l->d_nsectors);
	    n = get_confirmed(
			      "Sectors per track",
			      updating ? deflt : "54",
			      "decimal u_int_32",
			      answ, LSIZ);
	    l->d_nsectors = atoi(answ);

	    sprintf(deflt, "%d", l->d_ntracks);
	    n = get_confirmed(
			      "Tracks per cylinder",
			      updating ? deflt : "15",
			      "decimal u_int_32",
			      answ, LSIZ);
	    l->d_ntracks = atoi(answ);

	    sprintf(deflt, "%d", l->d_ncylinders);
	    n = get_confirmed(
			      "Number of cylinders",
			      updating ? deflt : "1632",
			      "decimal u_int_32",
			      answ, LSIZ);
	    l->d_ncylinders = atoi(answ);

	    l->d_secpercyl = l->d_nsectors * l->d_ntracks;
	    l->d_secperunit = l->d_secpercyl * l->d_ncylinders;

	    /*
	     * Spares
	     */
	    sprintf(deflt, "%d", l->d_sparespertrack);
	    n = get_confirmed(
			      "Spare sectors per track",
			      updating ? deflt : "1",
			      "decimal u_short_16",
			      answ, LSIZ);
	    l->d_sparespertrack = atoi(answ);

	    sprintf(deflt, "%d", l->d_sparespercyl);
	    n = get_confirmed(
			      "Spare sectors per cylinder",
			      updating ? deflt : "1",
			      "decimal u_short_16",
			      answ, LSIZ);
	    l->d_sparespercyl = atoi(answ);

	    sprintf(deflt, "%d", l->d_acylinders);
	    n = get_confirmed(
			      "Spare cylinders",
			      deflt,
			      "decimal u_int_32",
			      answ, LSIZ);
	    l->d_acylinders = atoi(answ);

	}
	/*
	 * hardware characteristics 
	 */
	if (!updating || confirm(would_you, "Hardware characteristics")) {
	    sprintf(deflt, "%d", l->d_rpm);
	    n = get_confirmed(
			      "Disk Revolutions per minute",
			      updating ? deflt : "3600",
			      "decimal u_short_16",
			      answ, LSIZ);
	    l->d_rpm = atoi(answ);

	    sprintf(deflt, "%d", l->d_interleave);
	    n = get_confirmed(
			      "Interleave factor",
			      updating ? deflt : "1",
			      "decimal u_short_16",
			      answ, LSIZ);
	    l->d_interleave = atoi(answ);

	    sprintf(deflt, "%d", l->d_trackskew);
	    n = get_confirmed(
			      "Track skew",
			      deflt,
			      "decimal u_short_16",
			      answ, LSIZ);
	    l->d_trackskew = atoi(answ);

	    sprintf(deflt, "%d", l->d_cylskew);
	    n = get_confirmed(
			      "Cylinder skew",
			      deflt,
			      "decimal u_short_16",
			      answ, LSIZ);
	    l->d_cylskew = atoi(answ);


	    sprintf(deflt, "%d", l->d_headswitch);
	    n = get_confirmed(
			      "Head switch time (usecs)",
			      deflt,
			      "decimal u_int_32",
			      answ, LSIZ);
	    l->d_headswitch = atoi(answ);

	    sprintf(deflt, "%d", l->d_trkseek);
	    n = get_confirmed(
			      "Track-to-track seek time (usecs)",
			      updating ? deflt : "3000",
			      "decimal u_int_32",
			      answ, LSIZ);
	    l->d_trkseek = atoi(answ);
	}

	if (!updating || confirm(would_you, "Special FLAGS")) {
	    sprintf(deflt, "x%x", l->d_flags);
	    n = get_confirmed(
			      "Special FLAGS, as per rz_label.h",
			      deflt,
			      "decimal u_int_32",
			      answ, LSIZ);
	    l->d_flags = atoi(answ);
	}

	bzero(l->d_drivedata, NDDATA * sizeof(int));
	bzero(l->d_spare, NSPARE * sizeof(int));

	/*
	 * filesystem and partition information 
	 */
	if (!updating || confirm(would_you, "Partitioning")) {
	    printf("Partitioning information.\n");

	    sprintf(deflt, "%d", l->d_npartitions);
	    n = get_confirmed(
			      "Number of partitions",
			      updating ? deflt : "8",
			      "decimal u_short_16",
			      answ, LSIZ);
	    l->d_npartitions = atoi(answ);

	    sprintf(deflt, "%x", l->d_bbsize);
	    n = get_confirmed(
			      "Size of primary boot area in bytes, sector 0 included",
			      updating ? deflt : "2000",
			      "hexadecimal u_int_32",
			      answ, LSIZ);
	    l->d_bbsize = atoh(answ);

	    sprintf(deflt, "%x", l->d_sbsize);
	    n = get_confirmed(
			      "Max superblock size in bytes",
			      updating ? deflt : "2000",
			      "hexadecimal u_int_32",
			      answ, LSIZ);
	    l->d_sbsize = atoh(answ);

	    for (i = 0; i < l->d_npartitions; i++) {
		static char    *p = "Number of sectors in partition X";

	    	n = strlen(p) - 1;
	    	p[n] = 'a' + i;
		sprintf(deflt, "%d", l->d_partitions[i].p_size);
	    	n = get_confirmed(
			    	  p,
				  deflt,
			    	  "decimal u_int_32",
			    	  answ, LSIZ);
	    	l->d_partitions[i].p_size = atoi(answ);

		sprintf(deflt, "%d", l->d_partitions[i].p_offset);
	    	n = get_confirmed(
			         "Starting sector of partition",
				 deflt,
			    	  "decimal u_int_32",
			    	  answ, LSIZ);
	    	l->d_partitions[i].p_offset = atoi(answ);

		printf("File system type on partition: BSD->7, unused->0 ");
		sprintf(deflt, "%d", l->d_partitions[i].p_fstype);
	    	n = get_confirmed(
			    	  "as per rz_label.h",
				  updating ? deflt : "7",
			    	  "decimal u_int_8",
			    	  answ, LSIZ);
	    	l->d_partitions[i].p_fstype = atoi(answ);

		sprintf(deflt, "%d", l->d_partitions[i].p_fsize);
	    	n = get_confirmed(
			          "Filesystem fragment size",
				  updating ? deflt : "1024",
			    	  "decimal u_int_32",
			    	  answ, LSIZ);
	    	l->d_partitions[i].p_fsize = atoi(answ);

		sprintf(deflt, "%d", l->d_partitions[i].p_frag);
	    	n = get_confirmed(
			    	  "Fragments per block",
				  updating ? deflt : "8",
			    	  "decimal u_int_8",
			    	  answ, LSIZ);
		l->d_partitions[i].p_frag = atoi(answ);

		sprintf(deflt, "%d", l->d_partitions[i].p_cpg);
		n = get_confirmed(
			          "Cylinders per cylinder-group",
				  updating ? deflt : "16",
			    	  "decimal u_int_16",
				  answ, LSIZ);
	    	l->d_partitions[i].p_cpg = atoi(answ);
	    }
	}

	/*
	 * Checksum and we are done 
	 */
	l->d_checksum = 0;
	l->d_checksum = label_sum(l);
}


/*
 *	Print a label
 */
print_label(l)
	register struct disklabel	*l;
{
	register int			i;
	static char			*typenames[FS_BSDFFS+1] = {
		"-- unused --", "swap space", "Sixth Edition",
		"Seventh Edition", "System-V", "Seventh Edition (1k blocks)",
		"Eigth Edition", "4.2BSD"
	};

	if (l->d_magic != DISKMAGIC ||
	    l->d_magic2 != DISKMAGIC ||
	    label_sum(l) != 0) {
		if (!confirm("Corrupted label, continue [y/n] ? ", 0))
			fatal(5,0);
	}
	printf("Label for pack: %s\n", l->d_packname);

	printf("Disk type: ");
	if (l->d_type == DTYPE_SCSI)
		printf("SCSI-%d\n", l->d_subtype);
	else
		printf("invalid!\n");
	printf("Disk type name: %s\n", l->d_typename);
	printf("Disk geometry: %d bytes/sec, %d secs/trk, %d trk/cyl, %d cyl/pack\n",
		l->d_secsize, l->d_nsectors, l->d_ntracks, l->d_ncylinders);
	printf("Disk size: %d secs/cyl, %d secs/pack\n",
		l->d_secpercyl, l->d_secperunit);
	printf("Spares: %d sec/trk, %d sec/cyl, %d alternate cyls\n",
		l->d_sparespertrack, l->d_sparespercyl, l->d_acylinders);
	printf("Params: %d rpm, %d:1 interleaved, %d trkskew, %d cylskew,\n",
		l->d_rpm, l->d_interleave, l->d_trackskew, l->d_cylskew);
	printf("\t%d us head switch time, %d us trk/trk seek time\n",
		l->d_headswitch, l->d_trkseek);
	printf("Special FLAGS: x%x\n", l->d_flags);
	printf("Drive-specific data: x%x x%x x%x x%x x%x\n",
		l->d_drivedata[0], l->d_drivedata[1], l->d_drivedata[2],
		l->d_drivedata[3], l->d_drivedata[4]);
	printf("Primary boot size at sec0: x%x bytes\n", l->d_bbsize);
	printf("Max superblock size: x%x bytes\n", l->d_sbsize);
	if (l->d_npartitions == 0)
		return;
	printf("Partition\tBlocks\tStart\tfsiz\tbsiz\tcpg\tFilesystem type\n");
	for (i = 0; i < l->d_npartitions; i++) {
		printf("\t%c\t%d\t%d\t%d\t%d\t%d\t",
			'a' + i, l->d_partitions[i].p_size,
			l->d_partitions[i].p_offset,
			l->d_partitions[i].p_fsize,
			l->d_partitions[i].p_fsize * l->d_partitions[i].p_frag,
			l->d_partitions[i].p_cpg);
		if (l->d_partitions[i].p_fstype <= FS_BSDFFS)
			printf("%s\n", typenames[l->d_partitions[i].p_fstype]);
		else
			printf("?type x%x?\n", l->d_partitions[i].p_fstype);
	}
}

/*
 *	Input tools
 */
get_confirmed(prompt, deflt, type, answ, maxlen)
	char	*prompt, *deflt, *type, *answ;
{
	int             n;

	while (1) {
		char	c = *answ;

		printf("%s {default: %s} [%s]: ", prompt, deflt, type);
		fflush(stdout);
		n = read(0, answ, maxlen);
		if (n <= 0 || n > maxlen)
			continue;
		if (*answ == '\n') {
			*answ = c;	/* could be answ==deflt */
			n = strlen(deflt) + 1;
			bcopy(deflt,answ,n);
		} else
			answ[n - 1] = 0;

		if (confirm("Please confirm {%s} as your choice [y/n]: ", answ))
			break;

	}
	return n;
}

boolean_t
confirm(fmt, arg)
	char	*fmt;
{
	char            yn[2];

	printf(fmt, arg);
	fflush(stdout);

	if (read(0, yn, 2) < 0)
		return FALSE;
	if (*yn == 'y' || *yn == 'Y' || *yn == '\n')
		return TRUE;
	return FALSE;
}

/*
 *	Look at the level-1 boot and build its level-0
 *	boot descriptor to be used by the prom.
 */
/* side-effect: file f is left pointing at text start */
get_boot0(f, b0)
	int			f;
	scsi_dec_boot0_t	*b0;
{
	struct exechdr		hdr;
	char			*in = "bootimage";
	register unsigned int	secs;

	/*
	 *	Read coff header and do mild checks
	 */
	if (read(f, &hdr, sizeof(hdr)) != sizeof(hdr))
		fatal(2, in);

	if (hdr.f.f_magic != MIPSMAGIC)
		fatal(4, in);

	if (hdr.a.magic != 0407)
		fatal(4, in);

	/* defaults to 0 */
	bzero(b0, sizeof(*b0));

	/* leave the file pointer where expected */
	lseek(f, N_TXTOFF(hdr.f,hdr.a), 0);

	/* make up the info */
	b0->magic = DEC_BOOT0_MAGIC;
	b0->phys_base = 
	b0->virt_base = hdr.a.text_start;
	b0->start_sector = 1;
	secs = hdr.a.tsize + hdr.a.dsize;
	b0->n_sectors = (secs + DEV_BSIZE - 1) / DEV_BSIZE;
}


/*
 *	Quitters
 */
fatal(mid, s)
	char	*s;
{
	static char *(fmt[]) = {
	    	"Usage: %s [-o outfile] [-u labelfile] [-nowrite] secondary-bootimage\n",
		"Cannot open %s\n",
		"Short read %s\n",
		"Short write %s\n",
		"Not (right) executable %s\n",
		"Bye.\n",
	};
	quit(mid, fmt[mid], s);
}
