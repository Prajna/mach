/* 
 * Mach Operating System
 * Copyright (c) 1990 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	loader_info.h,v $
 * Revision 2.2  91/05/08  13:09:49  dbg
 * 	Version for build_boot.  Removed function declarations and format.
 * 	Added sym_offset, sym_size, str_size.
 * 	[90/11/20            dbg]
 * 
 * Revision 2.1.1.1  91/02/26  11:18:04  dbg
 * 	Version for build_boot.  Removed function declarations and format.
 * 	Added sym_offset, sym_size, str_size.
 * 	[90/11/20            dbg]
 * 
 * Revision 2.2  90/08/27  21:46:12  dbg
 * 	Added error return codes and imported function declarations.
 * 	[90/07/18            dbg]
 * 
 */

#ifndef	_LOADER_INFO_H_
#define	_LOADER_INFO_H_

/*
 *	Data structures for bootstrap program loader.
 */

struct loader_info {
	vm_offset_t	text_start;	/* text start in memory */
	vm_size_t	text_size;
	vm_offset_t	text_offset;	/* text offset in file */
	vm_offset_t	data_start;	/* data+bss start in memory */
	vm_size_t	data_size;
	vm_offset_t	data_offset;	/* data offset in file */
	vm_size_t	bss_size;
	vm_offset_t	entry_1;	/* 2 words for entry address */
	vm_offset_t	entry_2;
	vm_offset_t	sym_offset;
	vm_size_t	sym_size;
	vm_size_t	str_size;
} ;

#endif	/* _LOADER_INFO_H_ */
