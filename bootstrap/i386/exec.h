/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
 * $Log:	exec.h,v $
 * Revision 2.4  92/03/05  23:11:32  rpd
 * 	Moved from kernel to bootstrap.
 * 	Removed sun conditionals.
 * 	Changed a_info to a_magic.
 * 	[92/03/01            rpd]
 * 
 * Revision 2.4  91/05/14  16:07:10  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:11:34  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:31:52  mrt]
 * 
 * Revision 2.2  90/05/03  15:25:11  dbg
 * 	Adapted for i386 a.out format.
 * 	[90/02/15            dbg]
 * 
 * 12-Sep-86  Jonathan J. Chew (jjc) at Carnegie-Mellon University
 *	Changed first long word to two shorts for the Sun to give
 *	the machine type along with the magic number.
 *	Added machine types for Sun.
 *	Made file includable more than once.
 *
 */
/*
 * exec stucture in an a.out file derived from FSF's
 * a.out.gnu.h file.
 */

#ifndef	_I386_EXEC_H_
#define	_I386_EXEC_H_

/*
 * Header prepended to each a.out file.
 */
struct exec
{
  long a_magic;			/* Use macros N_MAGIC, etc for access */
  unsigned long a_text;		/* length of text, in bytes */
  unsigned long a_data;		/* length of data, in bytes */
  unsigned long a_bss;		/* length of uninitialized data area for file, in bytes */
  unsigned long a_syms;		/* length of symbol table data in file, in bytes */
  unsigned long a_entry;	/* start address */
  unsigned long a_trsize;	/* length of relocation info for text, in bytes */
  unsigned long a_drsize;	/* length of relocation info for data, in bytes */
};

/* Code indicating object file or impure executable.  */
#define OMAGIC 0407
/* Code indicating pure executable.  */
#define NMAGIC 0410
/* Code indicating demand-paged executable.  */
#define ZMAGIC 0413

#endif	_I386_EXEC_H_
