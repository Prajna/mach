/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie Mellon the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	build_boot.c,v $
 * Revision 2.5  93/05/10  17:48:49  rvb
 * 	Fix include to use < vs " for new ode shadowing
 * 	[93/05/10  10:40:41  rvb]
 * 
 * Revision 2.4  93/01/14  17:57:13  danner
 * 	Added cross-endian support (from Ian Dall down under :-)
 * 	Added cross-wordsize support (yours truly)
 * 	[92/12/01            af]
 * 
 * Revision 2.3  92/01/03  20:28:35  dbg
 * 	Remove '-sequent' switch.  Recognize Sequent from
 * 	real-to-protected bootstrap text following a.out header.
 * 	[91/08/30            dbg]
 * 
 * 	Added code to glue the kernel and the boot file together, for
 * 	all machines.
 * 	[91/05/29            dbg]
 * 
 * Revision 2.2  91/05/08  13:09:36  dbg
 * 	Created.
 * 	[91/02/26            dbg]
 * 
 */

/*
 * Build a pure-kernel boot file from a kernel and a bootstrap loader.
 */

/*
 * Cross compilation makes thing a bit more complex,
 * but we are handling files that are smaller than
 * 32 bits anyways so it can be done.
 * There might be some byteorder deps still.
 */

#include <cross_32_to_64.h>

/* careful with includes here */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <mach/boot_info.h>

#include <cross_endian.h>

char *	kernel_name;
char *	bootstrap_name;
char *	boot_file_name = "mach.boot";

int32	kern_file;
int32	boot_file;
int32	out_file;

int32	kern_symbols = 1;
int32	boot_symbols = 1;
int32	no_header = 0;
int32	image_type = 0;
int32	debug = 0;

usage()
{
	printf("usage: build_boot [ -o boot_file ] kernel bootstrap\n");
	exit(1);
}

main(argc, argv)
	int32	argc;
	char	**argv;
{
	argc--, argv++;	/* skip program name */

	if (argc == 0)
	    usage();

	/*
	 * Parse switches.
	 */
	while (argc > 0 && **argv == '-') {
	    char *s;

	    s = *argv;
	    if (s[1] == 'o') {
		/* output file name */
		argc--, argv++;
		if (argc == 0)
		    usage();
		boot_file_name = *argv;
		argc--, argv++;
	    }
	    else if (s[1] == 's') {
		/* strip off initial header */
		no_header++;
		argc--, argv++;
	    }
	    else if (s[1] == 't') {
		/* use alternate image format (coff/aout) */
		image_type++;
		argc--, argv++;
	    }
	    else if (s[1] == 'd') {
		/* debug printouts */
		debug++;
		argc--, argv++;
	    }
	    else {
		printf("unknown switch: %s\n", s);
		exit(1);
	    }
	}

	if (argc != 2)
	    usage();

	kernel_name = argv[0];
	bootstrap_name = argv[1];

	kern_file = check_and_open(kernel_name);
	boot_file = check_and_open(bootstrap_name);

	out_file = creat(boot_file_name, 0777);	/* XXX mode */
	if (out_file < 0) {
	    perror(boot_file_name);
	    exit(2);
	}

	build_boot();

	close(out_file);
	close(kern_file);
	close(boot_file);

	exit(0);
}

int32
check_and_open(fname)
	char *fname;
{
	int32 f;
	struct stat statb;

	if (stat(fname, &statb) < 0) {
	    perror(fname);
	    exit(2);
	}
	if ((statb.st_mode & S_IFMT) != S_IFREG ||
	    (statb.st_mode & (S_IEXEC|(S_IEXEC>>3)|(S_IEXEC>>6))) == 0) {
		printf("build_boot: %s: not an executable file\n",
			fname);
		exit(2);
	}

	f = open(fname, O_RDONLY, 0);
	if (f < 0) {
	    perror(fname);
	    exit(2);
	}
	return (f);
}

/*
 * Create the boot file.
 */
build_boot()
{
	/*
	 * The symbol table is read from the file.
	 * Machine-dependent code can specify an
	 * optional header to be prefixed to the
	 * symbol table, containing information
	 * that cannot directly be read from the file.
	 */
#define	HEADER_MAX	(64*sizeof(int32))

	int32		sym_off;
	int32		sym_size;
	vm_size_t	sym_header[HEADER_MAX];
	int32		sym_header_size = HEADER_MAX * sizeof(vm_size_t);

	struct loader_info	kern_header;
	struct loader_info	boot_header;
	struct loader_info	boot_header_out;
	struct boot_info	boot_info;

	off_t	off;
	off_t	boot_info_off;
	off_t	boot_image_off;

	bzero(&kern_header, sizeof(kern_header));
	bzero(&boot_header, sizeof(boot_header));
	bzero(&boot_info, sizeof(boot_info));

	low32(boot_info.magic_number) = MACH_BOOT_INFO_MAGIC;

	/*
	 * Read in the kernel header.
	 */
	if (!ex_get_header(kern_file, 1, &kern_header,
			sym_header, &sym_header_size)) {
	    printf("%s: not an executable file\n", kernel_name);
	    exit(4);
	}

	/*
	 * Copy its text and data to the text section of the output file.
	 */
	if (!no_header)
	    lseek(out_file, exec_header_size(), L_SET);

	lseek(kern_file, low32(kern_header.text_offset), L_SET);
	file_copy(out_file, kern_file, low32(kern_header.text_size));

	lseek(kern_file, low32(kern_header.data_offset), L_SET);
	file_copy(out_file, kern_file, low32(kern_header.data_size));

	/*
	 * Allocate the boot_info block.
	 */
	boot_info_off = lseek(out_file, (off_t) 0, L_INCR);
	(void) lseek(out_file, (off_t) sizeof(struct boot_info), L_INCR);

	if (debug)
		printf("boot_info_block at %#x\n", boot_info_off);

	/*
	 * Find the kernel symbol table.
	 */
	if (kern_symbols && neq(kern_header.sym_size,zero)) {
	    /*
	     * Copy the header to the output file.
	     */
	    if (sym_header_size)
		write(out_file, sym_header, sym_header_size);

	    /*
	     * Copy the symbol table to the output file.
	     */
	    lseek(kern_file, low32(kern_header.sym_offset), L_SET);
	    file_copy(out_file, kern_file, low32(kern_header.sym_size));

	    if (debug)
		printf("kernel: header %#x, symbols %#x\n",
			sym_header_size, low32(kern_header.sym_size));

	    /*
	     * Round to an integer boundary in the file.
	     */
	    sym_size = sym_header_size + low32(kern_header.sym_size);
	    if (sym_size % sizeof(integer_t) != 0) {
		int32		pad;
		integer_t	zeros;

		zeros = zero;
		pad = sizeof(integer_t) - (sym_size % sizeof(integer_t));
		write(out_file, (char *)&zeros, pad);
		sym_size += pad;
		if (debug)
		    printf("padding: +%#x\n", pad);
	    }
		

	    /*
	     * Remember the symbol table size.
	     */
	    low32(boot_info.sym_size) = sym_size;
	}
	else {
	    low32(boot_info.sym_size) = 0;
	}

	if (debug)
	    printf("boot_info.sym_size: %#x\n", low32(boot_info.sym_size) );

	/*
	 * Remember the start of the bootstrap image.
	 */
	boot_image_off = lseek(out_file, (off_t) 0, L_INCR);

	if (debug)
	    printf("boot image at %#x\n", boot_image_off);

	/*
	 * Read the header for the bootstrap file.
	 */
	if (!ex_get_header(boot_file, 0, &boot_header,
			sym_header, &sym_header_size)) {
	    printf("%s: not an executable file\n", bootstrap_name);
	    exit(4);
	}

	/*
	 * Copy the text
	 */
	lseek(boot_file, low32(boot_header.text_offset), L_SET);
	file_copy(out_file, boot_file, low32(boot_header.text_size));

	/*
	 * And the data
	 */
	lseek(boot_file, low32(boot_header.data_offset), L_SET);
	file_copy(out_file, boot_file, low32(boot_header.data_size));

	/*
	 * Symbols for boot program
	 */
	if (boot_symbols && neq(boot_header.sym_size,zero)) {
	    /*
	     * Copy the header to the output file.
	     */
	    if (sym_header_size)
		write(out_file, sym_header, sym_header_size);

	    /*
	     * Copy the symbol table to the output file.
	     */
	    lseek(boot_file, low32(boot_header.sym_offset), L_SET);
	    file_copy(out_file, boot_file, low32(boot_header.sym_size));

	    /*
	     * Round to an integer boundary in the file.
	     */
	    sym_size = sym_header_size + low32(boot_header.sym_size);
	    if (sym_size % sizeof(integer_t) != 0) {
		int32		pad;
		integer_t	zeros;

		zeros = zero;
		pad = sizeof(integer_t) - (sym_size % sizeof(integer_t));
		write(out_file, (char *)&zeros, pad);
		sym_size += pad;
	    }
		
	    /*
	     * Remember the symbol table size.
	     */
	    low32(boot_header.sym_size) = sym_size;
	}
	else {
	    low32(boot_header.sym_size) = 0;
	}

	if (debug)
	    printf("boot_header.sym_size: %#x\n", low32(boot_header.sym_size) );

	/*
	 * Save the size of the boot image.
	 */
	off = lseek(out_file, (off_t) 0, L_INCR);
	low32(boot_info.boot_size) = off - boot_image_off;

	if (debug)
	    printf("boot_info.boot_size: %#x\n", low32(boot_info.boot_size) );

	/*
	 * Write out a modified copy of the boot header.
	 * Offsets are relative to the end of the boot header.
	 */
	boot_header_out = boot_header;
	boot_header_out.text_offset = zero;
	boot_header_out.data_offset = plus(boot_header_out.text_offset,
					   boot_header.text_size);
	boot_header_out.sym_offset  = plus(boot_header_out.data_offset,
					   boot_header.data_size);

	EXTERNALIZE(boot_header_out.text_start);
	EXTERNALIZE(boot_header_out.text_size);
	EXTERNALIZE(boot_header_out.text_offset);
	EXTERNALIZE(boot_header_out.data_start);
	EXTERNALIZE(boot_header_out.data_size);
	EXTERNALIZE(boot_header_out.data_offset);
	EXTERNALIZE(boot_header_out.bss_size);
	EXTERNALIZE(boot_header_out.sym_offset);
	EXTERNALIZE(boot_header_out.sym_size);
	EXTERNALIZE(boot_header_out.entry_1);
	EXTERNALIZE(boot_header_out.entry_2);
	write(out_file, (char *)&boot_header_out, sizeof(boot_header_out));

	low32(boot_info.load_info_size) = sizeof(boot_header_out);

	if (debug)
	    printf("boot_info.load_info_size: %#x\n", low32(boot_info.load_info_size) );

	/*
	 * Remember the end of the file.
	 */
	off = lseek(out_file, (off_t) 0, L_INCR);

	/*
	 * Go back to the start of the file and write out
	 * the bootable file header.
	 */
	if (!no_header) {
	    lseek(out_file, (off_t) 0, L_SET);
	    write_exec_header(out_file, &kern_header, off);
	}

	/*
	 * Write out the boot_info block.
	 */
	EXTERNALIZE(boot_info.magic_number);
	EXTERNALIZE(boot_info.sym_size);
	EXTERNALIZE(boot_info.boot_size);
	EXTERNALIZE(boot_info.load_info_size);
	lseek(out_file, boot_info_off, L_SET);
	write(out_file, (char *)&boot_info, sizeof(boot_info));

}

check_read(f, addr, size)
	int32	f;
	char *	addr;
	int32	size;
{
	if (read(f, addr, size) != size) {
	    perror("read");
	    exit(6);
	}
}

/*
 * Copy N bytes from in_file to out_file
 */
file_copy(out_f, in_f, size)
	int32	out_f;
	int32	in_f;
	int32	size;
{
	char	buf[4096];

	while (size >= sizeof(buf)) {
	    check_read(in_f, buf, sizeof(buf));
	    write(out_f, buf, sizeof(buf));
	    size -= sizeof(buf);
	}
	if (size > 0) {
	    check_read(in_f, buf, size);
	    write(out_f, buf, size);
	}
}

