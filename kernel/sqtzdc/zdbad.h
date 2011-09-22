/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * Copyright (c) 1991 Sequent Computer Systems
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND SEQUENT COMPUTER SYSTEMS ALLOW FREE USE OF
 * THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * SEQUENT COMPUTER SYSTEMS DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	zdbad.h,v $
 * Revision 2.3  91/07/31  18:08:16  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:08:08  dbg
 * 	Adapted for pure Mach kernel.
 * 	[90/10/04            dbg]
 * 
 */

/*
 * $Header: zdbad.h,v 2.3 91/07/31 18:08:16 dbg Exp $
 *
 * zdbad.h
 *	ZDC bad block list and MFG defect list structure definitions
 */

/*
 * Revision 1.1  89/07/05  13:21:05  kak
 * Initial revision
 * 
 */
#ifndef	_SQTZDC_ZDBAD_H_
#define	_SQTZDC_ZDBAD_H_

#include <sys/types.h>

#define	BZ_NBADCOPY	5		/* number of bad block list copies */

/*
 * structure of bad block list
 */

struct zdbad {
	long	bz_csn;		/* bad block list checksum */
	u_short	bz_nelem;	/* nbr of elements in list */
	u_short	bz_nsnf;	/* nbr of BZ_SNF entries in list */
	struct	bz_bad {
		struct	bz_diskaddr {
			u_char	bd_sect;	/* sector */
			u_char	bd_head;	/* head */
			u_short	bd_cyl   : 13,	/* cylinder */
				bd_rtype :  2,	/* replacement type */
				bd_ftype :  1;	/* failure type */
		} bz_badaddr;			/* Bad disk address */
		struct diskaddr bz_rpladdr;	/* Replacement disk address */
	} bz_bad[1];		/* size is disk type dependent */
};

#define bz_cyl		bz_badaddr.bd_cyl
#define bz_head		bz_badaddr.bd_head
#define bz_sect		bz_badaddr.bd_sect
#define bz_rtype	bz_badaddr.bd_rtype
#define bz_ftype	bz_badaddr.bd_ftype

/*
 * Replacement types
 */
#define BZ_PHYS		0x0		/* Physical disk address */
#define BZ_AUTOREVECT	0x1		/* Autorevector for replacement */
#define BZ_SNF		0x2		/* SNF - driver must revector */ 

/*
 * Failure types
 */
#define BZ_BADHEAD	0x0
#define BZ_BADDATA	0x1

/*
 * Structure of manufacturer's defect list.
 */

struct bad_mfg {
	long	bm_csn;		/* mfg's defect list checksum */
	u_short	bm_nelem;	/* number of entries in bm_mfgbad */

	/* defect location */
	struct bm_mfgbad {
		u_short	bm_cyl;		/* cylinder */
		u_short	bm_len;		/* length of defect */
		u_int	bm_pos  : 24,	/* bytes from index */
			bm_head :  8;	/* head */
	} bm_mfgbad[1];		/* size is disk type dependent */
};

#endif	/* _SQTZDC_ZDBAD_H_ */
