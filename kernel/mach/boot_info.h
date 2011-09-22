/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * $Log:	boot_info.h,v $
 * Revision 2.3  93/01/14  17:41:27  danner
 * 	Added magic number, for sanity.
 * 	[92/12/01            af]
 * 
 * Revision 2.2  92/01/03  20:19:42  dbg
 * 	Created.
 * 	[91/09/06            dbg]
 * 
 */

#ifndef	_MACH_BOOT_INFO_H_
#define	_MACH_BOOT_INFO_H_

/*
 * Structure of Mach kernel boot file.
 */
#include <mach/machine/vm_types.h>

/*
 *	A Mach kernel boot file consists of the Mach
 *	kernel image and the bootstrap image, glued
 *	together.
 *
 *	The first part of the file is a normal executable
 *	(bootable) file: a.out, coff, or whatever.  The
 *	text and data sizes are set to include the entire
 *	file.  (Some machines do not allow a zero-length
 *	data segment).
 *
 *	The rest of the file sits where the kernel BSS
 *	should be.  A boot_info record describes the
 *	sizes of the next 3 sections.  Following this
 *	are the kernel symbol table, the bootstrap image
 *	(including its symbol table), and the loader
 *	information for the bootstrap image.  Each
 *	of these sections is padded to an integer (4-byte)
 *	boundary.
 *
 *	When the file is loaded into memory, the kernel
 *	text and data segments are at their normal locations.
 *
 *	The boot_info structure appears at the start of
 *	the bss (at 'edata[]'):
 */

struct boot_info {
	vm_size_t	magic_number;
#	define	MACH_BOOT_INFO_MAGIC	0x15beef15
	vm_size_t	sym_size;		/* size of kernel symbols */
	vm_size_t	boot_size;		/* size of bootstrap image */
	vm_size_t	load_info_size;		/* size of loader information
						   for bootstrap image */
};

/*
 *	The 3 sections must be moved out of BSS for the kernel to run:
 *
 *	The kernel symbol table follows the BSS (at 'end[]').
 *
 *	The bootstrap image is on the first page boundary (machine page
 *	size) following the kernel symbol table.
 *
 *	The loader information immediately follows the bootstrap image.
 */

/*
 *	Loader information for bootstrap image:
 */

struct loader_info {
	vm_offset_t	text_start;	/* text start in memory */
	vm_size_t	text_size;	/* text size */
	vm_offset_t	text_offset;	/* text offset in file */
	vm_offset_t	data_start;	/* data+bss start in memory */
	vm_size_t	data_size;	/* data size */
	vm_offset_t	data_offset;	/* data offset in file */
	vm_size_t	bss_size;	/* BSS size */
	vm_offset_t	sym_offset;	/* symbol table offset in file */
	vm_size_t	sym_size;	/* symbol table size */
	vm_offset_t	entry_1;	/* 2 words for entry address */
	vm_offset_t	entry_2;
} ;

#endif	/* _MACH_BOOT_INFO_H_ */
