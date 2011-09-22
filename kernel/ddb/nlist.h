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
 * $Log:	nlist.h,v $
 * Revision 2.5  93/03/09  10:53:55  danner
 * 	Made explicit padding for alpha, has its uses.
 * 	Made n_other unsigned.  Fixed erroneous def of N_FN.
 * 	[93/02/21            af]
 * 
 * Revision 2.4  91/05/14  15:38:20  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:07:42  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  16:20:26  mrt]
 * 
 * 11-Aug-88  David Golub (dbg) at Carnegie-Mellon University
 *	Added n_un, n_strx definitions for kernel debugger (from
 *	a.out.h).
 *
 */
/*
 *  nlist.h - symbol table entry  structure for an a.out file
 *  derived from FSF's a.out.gnu.h
 *
 */

#ifndef _DDB_NLIST_H_
#define _DDB_NLIST_H_

struct	nlist {
	union n_un {
	    char	*n_name;	/* symbol name */
	    long	n_strx;		/* index into file string table */
	} n_un;
	unsigned char n_type;	/* type flag, i.e. N_TEXT etc; see below */
	unsigned char n_other;	/* machdep uses */
	short	n_desc;		/* see <stab.h> */
#if alpha
	int	n_pad;		/* alignment, used to carry framesize info */
#endif
	vm_offset_t n_value;	/* value of this symbol (or sdb offset) */
};

/*
 * Simple values for n_type.
 */
#define	N_UNDF	0		/* undefined */
#define	N_ABS	2		/* absolute */
#define	N_TEXT	4		/* text */
#define	N_DATA	6		/* data */
#define	N_BSS	8		/* bss */
#define	N_FN	0x1f		/* file name symbol */
#define	N_EXT	1		/* external bit, or'ed in */
#define	N_TYPE	0x1e		/* mask for all the type bits */
#define	N_STAB	0xe0		/* if any of these bits set, a SDB entry */


#endif /* _DDB_NLIST_H_ */
