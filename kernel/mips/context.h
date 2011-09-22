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
 * $Log:	context.h,v $
 * Revision 2.5  91/05/14  17:33:02  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:47:34  mrt
 * 	Added author notices
 * 	[91/02/04  11:21:32  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:25:05  mrt]
 * 
 * Revision 2.3  91/01/08  15:48:53  rpd
 * 	Removed TLB_SAFE_KSTACK, TLB_SAFE_KSTACK1.
 * 	[90/12/08            rpd]
 * 
 * Revision 2.2  89/11/29  14:12:51  af
 * 	Define usage of safe tlb entries for kernel stack pages.
 * 	[89/11/03  16:37:45  af]
 * 
 * 	Moved here definitions about use of the TLB 'safe' entries.
 * 	Which is now just the first, as the kernel stack shrinked
 * 	to just one page.
 * 	[89/10/29  15:15:42  af]
 * 
 * 04-Oct-89  Alessandro Forin (af) at Carnegie-Mellon University
 *	Created.
 *
 */

/*
 *	File: context.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Register save definitions for non-local goto's
 */

#ifndef	ASSEMBLER
typedef struct {
	int	s0;
	int	s1;
	int	s2;
	int	s3;
	int	s4;
	int	s5;
	int	s6;
	int	s7;
	int	sp;
	int	fp;
	int	pc;
	int	sr;
} jmp_buf;

#endif	ASSEMBLER
