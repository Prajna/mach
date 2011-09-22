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
 * $Log:	rz.h,v $
 * Revision 2.9  93/08/03  12:33:48  mrt
 * 	Added rzlun().
 * 	[93/07/29  23:40:26  af]
 * 
 * Revision 2.8  92/04/03  12:09:55  rpd
 * 	Fabricate extra partition info to deal with the
 * 	first alternate partition range and "PARITITON_ABSOLUTE".
 * 	[92/04/01            rvb]
 * 
 * Revision 2.7  91/06/19  11:56:57  rvb
 * 	File moved here from mips/PMAX since it is now "MI" code, also
 * 	used by Vax3100 and soon -- the omron luna88k.
 * 	[91/06/04            rvb]
 * 
 * 	A couple of macros were not fully parenthesized, which screwed
 * 	up the second scsi bus on Vaxen.  [This was the only bug in the
 * 	multi-bus code, amazing].
 * 	[91/05/30            af]
 * 
 * Revision 2.6  91/05/14  17:26:19  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/05/13  06:04:20  af
 * 	Redefined naive macro names to avoid conflicts.
 * 	[91/05/12  16:08:55  af]
 * 
 * Revision 2.4  91/02/05  17:43:42  mrt
 * 	Added author notices
 * 	[91/02/04  11:16:33  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:15:29  mrt]
 * 
 * Revision 2.3  90/12/05  23:33:55  af
 * 
 * 
 * Revision 2.1.1.1  90/11/01  03:43:37  af
 * 	Created.
 * 	[90/10/21            af]
 */
/*
 *	File: rz.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	Mapping between U*x-like indexing and controller+slave
 *	Each controller handles at most 8 slaves, few controllers.
 */

#define	rzcontroller(dev)	(((dev)>>6)&0x3)
#define	rzslave(dev)		(((dev)>>3)&0x7)
#define	rzpartition(dev)	((PARTITION_TYPE(dev)==0xf)?MAXPARTITIONS:((dev)&0x7))
/* To address the full 256 luns use upper bits 8..12 */
/* NOTE: Under U*x this means the next major up.. what a mess */
#define rzlun(dev)		(((dev)&0x7) | (((dev)>>5)&0xf8))

#define PARTITION_TYPE(dev)	(((dev)>>24)&0xf)
#define PARTITION_ABSOLUTE	(0xf<<24)

#ifdef	MACH_KERNEL
#else	/*MACH_KERNEL*/
#define tape_unit(dev)		((((dev)&0xe0)>>3)|((dev)&0x3))
#define	TAPE_UNIT(dev)		((dev)&(~0xff))|(tape_unit((dev))<<3)
#define	TAPE_REWINDS(dev)	(((dev)&0x1c)==0)||(((dev)&0x1c)==8)
#endif	/*MACH_KERNEL*/
