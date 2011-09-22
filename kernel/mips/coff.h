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
 * $Log:	coff.h,v $
 * Revision 2.6  91/05/14  17:32:44  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:47:24  mrt
 * 	Added author notices
 * 	[91/02/04  11:21:18  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:24:53  mrt]
 * 
 * Revision 2.4  90/12/05  23:37:05  af
 * 
 * 
 * Revision 2.3  90/12/05  20:49:33  af
 * 	Added definition of exechdr, which appeared just about
 * 	everywhere, sooo...
 * 	[90/12/02            af]
 * 
 * Revision 2.2  89/11/29  14:12:46  af
 * 	Created, for pure Mach kernel.
 * 	[89/10/09            af]
 * 
 */
/*
 *	File: coff.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Structure definitions for COFF headers
 */

struct filehdr {
	unsigned short	f_magic;	/* magic number */
	unsigned short	f_nscns;	/* number of sections */
	long		f_timdat;	/* time & date stamp */
	long		f_symptr;	/* file pointer to symtab */
	long		f_nsyms;	/* number of symtab entries */
	unsigned short	f_opthdr;	/* sizeof(optional hdr) */
	unsigned short	f_flags;	/* flags */
};

#define  F_EXEC		0000002

#if	BYTE_MSF
#define MIPSMAGIC	0540
#else
#define MIPSMAGIC	0542
#endif


struct scnhdr {
	char		s_name[8];	/* section name */
	long		s_paddr;	/* physical address */
	long		s_vaddr;	/* virtual address */
	long		s_size;		/* section size */
	long		s_scnptr;	/* file ptr to raw data for section */
	long		s_relptr;	/* file ptr to relocation */
	long		s_lnnoptr;	/* file ptr to line numbers */
	unsigned short	s_nreloc;	/* number of relocation entries */
	unsigned short	s_nlnno;	/* number of line number entries */
	long		s_flags;	/* flags */
};



struct aouthdr {
	short	magic;		/* see magic.h				*/
	short	vstamp;		/* version stamp			*/
	long	tsize;		/* text size in bytes, padded to FW
				   bdry					*/
	long	dsize;		/* initialized data "  "		*/
	long	bsize;		/* uninitialized data "   "		*/
	long	entry;		/* entry point, value of "start"	*/
	long	text_start;	/* base of text used for this file	*/
	long	data_start;	/* base of data used for this file	*/
	long	bss_start;	/* base of bss used for this file	*/
	long	gprmask;	/* general purpose register mask	*/
	long	cprmask[4];	/* co-processor register masks		*/
	long	gp_value;	/* the gp value used for this object    */
};


#define OMAGIC	0407		/* old impure format */
#define NMAGIC	0410		/* read-only text */
#define ZMAGIC	0413		/* demand load format */

#define	N_BADMAG(a) \
  ((a).magic != OMAGIC && (a).magic != NMAGIC && (a).magic != ZMAGIC)

#define SCNROUND ((long)16)
#define OSCNRND  ((long)8)

struct exechdr {
	struct filehdr	f;
	struct aouthdr	a;
};

#define N_TXTOFF(f, a) \
 ((a).magic == ZMAGIC ? 0 : \
  ((a).vstamp < 23 ? \
   ((sizeof(struct filehdr) + sizeof(struct aouthdr) + \
     (f).f_nscns * sizeof(struct scnhdr) + 7) & ~7) : \
   ((sizeof(struct filehdr) + sizeof(struct aouthdr) + \
     (f).f_nscns * sizeof(struct scnhdr) + 15) & ~15)))
