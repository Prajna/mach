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
 * $Log:	a.out.h,v $
 * Revision 2.1.1.1  94/06/01  10:19:34  rvb
 * 	From BSDSS
 * 
 * Revision 2.3  93/12/14  14:10:18  rvb
 * 	Accept "magic" in natural OR network order.
 * 	[93/10/12            rvb]
 * 
 * Revision 2.2  93/05/11  11:41:52  rvb
 *	Gnu code originally used by Mach I386 release -> BSDSS
 * 	[93/05/05  15:21:13  rvb]
 * 
 *
 */
/*
 *  Note: the copyright above only applies to the MACH changes in the file.
 *
 *	The files below were changed from the orginals provided in binutils.
 *  All changes are in #ifdef NOT_MACH ... #else ... #endif conditionals.
 *  The changes are intended for use with the i386 MACH 2.5 operating 
 *  environment.
 *
 *	NOTE:  MACH ld has a few major changes from the base ld:
 *		 The struct exec is part of the file.  It is loaded
 *		with the text.
 *		 The text start address is 0x10000.  The low 64k is
 *		never mapped.  The pagesize is always 4k.
 */
/*
	Makefile
	ar.c
	a.out.h
	ld.c
	nm.c
*/

#ifndef __A_OUT_GNU_H__
#define __A_OUT_GNU_H__

#define __GNU_EXEC_MACROS__

#ifdef	NOT_MACH
#else	NOT_MACH
#include <machine/vmparam.h>
#ifdef i386
#undef __STRUCT_EXEC_OVERRIDE__
#endif	i386
#endif	NOT_MACH


#ifndef __STRUCT_EXEC_OVERRIDE__

struct exec
{
  unsigned long a_info;		/* Use macros N_MAGIC, etc for access */
  unsigned a_text;		/* length of text, in bytes */
  unsigned a_data;		/* length of data, in bytes */
  unsigned a_bss;		/* length of uninitialized data area for file, in bytes */
  unsigned a_syms;		/* length of symbol table data in file, in bytes */
  unsigned a_entry;		/* start address */
  unsigned a_trsize;		/* length of relocation info for text, in bytes */
  unsigned a_drsize;		/* length of relocation info for data, in bytes */
};

#endif /* __STRUCT_EXEC_OVERRIDE__ */

/* these go in the N_MACHTYPE field */
enum machine_type {
  M_OLDSUN2 = 0,
  M_68010 = 1,
  M_68020 = 2,
  M_SPARC = 3,
  /* skip a bunch so we don't run into any of sun's numbers */
  M_386 = 100,
};

#define N_MAGIC(exec) ((exec).a_info & 0xffff)
#define BLK_MAGIC(exec) (ntohl((exec).a_info) & 0xffff)
#define N_MACHTYPE(exec) ((enum machine_type)(((exec).a_info >> 16) & 0xff))
#define N_FLAGS(exec) (((exec).a_info >> 24) & 0xff)
#define N_SET_INFO(exec, magic, type, flags) \
	((exec).a_info = ((magic) & 0xffff) \
	 | (((int)(type) & 0xff) << 16) \
	 | (((flags) & 0xff) << 24))
#define N_SET_MAGIC(exec, magic) \
	((exec).a_info = (((exec).a_info & 0xffff0000) | ((magic) & 0xffff)))

#define N_SET_MACHTYPE(exec, machtype) \
	((exec).a_info = \
	 ((exec).a_info&0xff00ffff) | ((((int)(machtype))&0xff) << 16))

#define N_SET_FLAGS(exec, flags) \
	((exec).a_info = \
	 ((exec).a_info&0x00ffffff) | (((flags) & 0xff) << 24))

/* Code indicating object file or impure executable.  */
#define OMAGIC 0407
/* Code indicating pure executable.  */
#define NMAGIC 0410
/* Code indicating demand-paged executable.  */
#define ZMAGIC 0413

#define N_BADMAG(x)					\
 (N_MAGIC(x) != OMAGIC && N_MAGIC(x) != NMAGIC		\
  && N_MAGIC(x) != ZMAGIC &&				\
  BLK_MAGIC(x) != OMAGIC && BLK_MAGIC(x) != NMAGIC \
  && BLK_MAGIC(x) != ZMAGIC)

#define _N_BADMAG(x)					\
 (N_MAGIC(x) != OMAGIC && N_MAGIC(x) != NMAGIC		\
  && N_MAGIC(x) != ZMAGIC)

#if	defined(i386) && ! defined(NOT_MACH)
#define _N_HDROFF(x) 0

#define N_TXTOFF(x) sizeof (struct exec)
#else	defined(i386) && ! defined(NOT_MACH)
#define _N_HDROFF(x) (1024 - sizeof (struct exec))

#define N_TXTOFF(x) \
 (N_MAGIC(x) == ZMAGIC ? _N_HDROFF((x)) + sizeof (struct exec) : sizeof (struct exec))
#endif	defined(i386) && ! defined(NOT_MACH)

#define N_DATOFF(x) (N_TXTOFF(x) + (x).a_text)

#define N_TRELOFF(x) (N_DATOFF(x) + (x).a_data)

#define N_DRELOFF(x) (N_TRELOFF(x) + (x).a_trsize)

#define N_SYMOFF(x) (N_DRELOFF(x) + (x).a_drsize)

#define N_STROFF(x) (N_SYMOFF(x) + (x).a_syms)

/* Address of text segment in memory after it is loaded.  */
#if	defined(i386) && ! defined(NOT_MACH)
#define N_TXTADDR(x) (0x10000 + sizeof(struct exec)) 
/* #define N_TXTADDR(x) (USRTEXT + sizeof(struct exec)) */
#else	defined(i386) && ! defined(NOT_MACH)
#define N_TXTADDR(x) 0
#endif	defined(i386) && ! defined(NOT_MACH)

/* Address of data segment in memory after it is loaded.
   Note that it is up to you to define SEGMENT_SIZE
   on machines not listed here.  */
#ifdef vax
#define SEGMENT_SIZE page_size
#endif
#ifdef is68k
#define SEGMENT_SIZE 0x20000
#endif
#if	defined(i386) && ! defined(NOT_MACH)
#define SEGMENT_SIZE 0x1000
#endif	defined(i386) && ! defined(NOT_MACH)

#ifndef N_DATADDR
#define N_DATADDR(x) \
    (N_MAGIC(x)==OMAGIC? (N_TXTADDR(x)+(x).a_text) \
     : (SEGMENT_SIZE + ((N_TXTADDR(x)+(x).a_text-1) & ~(SEGMENT_SIZE-1))))
#endif

/* Address of bss segment in memory after it is loaded.  */
#define N_BSSADDR(x) (N_DATADDR(x) + (x).a_data)

struct nlist {
  union {
    char *n_name;
    struct nlist *n_next;
    long n_strx;
  } n_un;
  unsigned char n_type;
  char n_other;
  short n_desc;
  unsigned long n_value;
};

#define N_UNDF 0
#define N_ABS 2
#define N_TEXT 4
#define N_DATA 6
#define N_BSS 8
#define N_FN 15

#define N_EXT 1
#define N_TYPE 036
#define N_STAB 0340

/* The following type indicates the definition of a symbol as being
   an indirect reference to another symbol.  The other symbol
   appears as an undefined reference, immediately following this symbol.

   Indirection is asymmetrical.  The other symbol's value will be used
   to satisfy requests for the indirect symbol, but not vice versa.
   If the other symbol does not have a definition, libraries will
   be searched to find a definition.  */
#define N_INDR 0xa

/* The following symbols refer to set elements.
   All the N_SET[ATDB] symbols with the same name form one set.
   Space is allocated for the set in the text section, and each set
   element's value is stored into one word of the space.
   The first word of the space is the length of the set (number of elements).

   The address of the set is made into an N_SETV symbol
   whose name is the same as the name of the set.
   This symbol acts like a N_DATA global symbol
   in that it can satisfy undefined external references.  */

/* These appear as input to LD, in a .o file.  */
#define	N_SETA	0x14		/* Absolute set element symbol */
#define	N_SETT	0x16		/* Text set element symbol */
#define	N_SETD	0x18		/* Data set element symbol */
#define	N_SETB	0x1A		/* Bss set element symbol */

/* This is output from LD.  */
#define N_SETV	0x1C		/* Pointer to set vector in data area.  */

/* This structure describes a single relocation to be performed.
   The text-relocation section of the file is a vector of these structures,
   all of which apply to the text section.
   Likewise, the data-relocation section applies to the data section.  */

struct relocation_info
{
  /* Address (within segment) to be relocated.  */
  int r_address;
  /* The meaning of r_symbolnum depends on r_extern.  */
  unsigned int r_symbolnum:24;
  /* Nonzero means value is a pc-relative offset
     and it should be relocated for changes in its own address
     as well as for changes in the symbol or section specified.  */
  unsigned int r_pcrel:1;
  /* Length (as exponent of 2) of the field to be relocated.
     Thus, a value of 2 indicates 1<<2 bytes.  */
  unsigned int r_length:2;
  /* 1 => relocate with value of symbol.
          r_symbolnum is the index of the symbol
	  in file's the symbol table.
     0 => relocate with the address of a segment.
          r_symbolnum is N_TEXT, N_DATA, N_BSS or N_ABS
	  (the N_EXT bit may be set also, but signifies nothing).  */
  unsigned int r_extern:1;
  /* Four bits that aren't used, but when writing an object file
     it is desirable to clear them.  */
  unsigned int r_pad:4;
};


#endif /* __A_OUT_GNU_H__ */
