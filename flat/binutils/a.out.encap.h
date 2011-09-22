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
 * $Log:	a.out.encap.h,v $
 * Revision 2.1.1.1  94/06/01  10:19:31  rvb
 * 	From BSDSS
 * 
 * Revision 2.2  93/05/11  11:41:44  rvb
 * 		Gnu code originally used by Mach I386 release -> BSDSS
 * 	[93/05/05  15:21:08  rvb]
 * 
 *
 */
/* Another try at encapsulating bsd object files in coff.
   Copyright (C) 1988, 1989, Free Software Foundation, Inc.
   Written by Pace Willisson 12/9/88

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */
   
/*
 * This time, we will only use the coff headers to tell the kernel
 * how to exec the file.  Therefore, the only fields that need to 
 * be filled in are the scnptr and vaddr for the text and data
 * sections, and the vaddr for the bss.  As far as coff is concerned,
 * there is no symbol table, relocation, or line numbers.
 *
 * A normal bsd header (struct exec) is placed after the coff headers,
 * and before the real text.  I defined a the new fields 'a_machtype'
 * and a_flags.  If a_machtype is M_386, and a_flags & A_ENCAP is
 * true, then the bsd header is preceeded by a coff header.  Macros
 * like N_TXTOFF and N_TXTADDR use this field to find the bsd header.
 * 
 * The only problem is to track down the bsd exec header.  The
 * macros HEADER_OFFSET, etc do this.  Look at nm.c, dis.c, etc
 * for examples.
 */

#include "a.out.gnu.h"

#define N_FLAGS_COFF_ENCAPSULATE 0x20 /* coff header precedes bsd header */

/* Describe the COFF header used for encapsulation.  */

struct coffheader
{
  /* filehdr */
  unsigned short f_magic;
  unsigned short f_nscns;
  long f_timdat;
  long f_symptr;
  long f_nsyms;
  unsigned short f_opthdr;
  unsigned short f_flags;
  /* aouthdr */
  short magic;
  short vstamp;
  long tsize;
  long dsize;
  long bsize;
  long entry;
  long text_start;
  long data_start;
  struct coffscn
    {
      char s_name[8];
      long s_paddr;
      long s_vaddr;
      long s_size;
      long s_scnptr;
      long s_relptr;
      long s_lnnoptr;
      unsigned short s_nreloc;
      unsigned short s_nlnno;
      long s_flags;
    } scns[3];
};

/* Describe some of the parameters of the encapsulation,
   including how to find the encapsulated BSD header.  */

#ifdef i386
#define COFF_MAGIC 0514 /* I386MAGIC */
#endif

#if defined(i386)
short __header_offset_temp;
#define HEADER_OFFSET(f) \
	(__header_offset_temp = 0, \
	 fread ((char *)&__header_offset_temp, sizeof (short), 1, (f)), \
	 fseek ((f), -sizeof (short), 1), \
	 __header_offset_temp==COFF_MAGIC ? sizeof(struct coffheader) : 0)

#define HEADER_OFFSET_FD(fd) \
	(__header_offset_temp = 0, \
	 read ((fd), (char *)&__header_offset_temp, sizeof (short)), \
	 lseek ((fd), -sizeof (short), 1), \
	 __header_offset_temp==COFF_MAGIC ? sizeof(struct coffheader) : 0)


#else
#define HEADER_OFFSET(f) 0
#define HEADER_OFFSET_FD(fd) 0
#endif

#define HEADER_SEEK(f) (fseek ((f), HEADER_OFFSET((f)), 1))
#define HEADER_SEEK_FD(fd) (lseek ((fd), HEADER_OFFSET_FD((fd)), 1))


/* Describe the characteristics of the BSD header
   that appears inside the encapsulation.  */

#undef _N_HDROFF
#undef N_TXTADDR
#undef N_DATADDR

#define _N_HDROFF(x) ((N_FLAGS(x) & N_FLAGS_COFF_ENCAPSULATE) ? \
		      sizeof (struct coffheader) : 0)

/* Address of text segment in memory after it is loaded.  */
#define N_TXTADDR(x) \
	((N_FLAGS(x) & N_FLAGS_COFF_ENCAPSULATE) ? \
	 sizeof (struct coffheader) + sizeof (struct exec) : 0)

#define SEGMENT_SIZE 0x400000
#define N_DATADDR(x) \
	((N_FLAGS(x) & N_FLAGS_COFF_ENCAPSULATE) ? \
	 (SEGMENT_SIZE + ((N_TXTADDR(x)+(x).a_text-1) & ~(SEGMENT_SIZE-1))) : \
	 (N_TXTADDR(x)+(x).a_text))
