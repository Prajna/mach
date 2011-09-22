/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
 * $Log:	mach_param.h,v $
 * Revision 2.5  91/06/19  11:56:19  rvb
 * 	#ifdef PMAX -> #ifdef DECSTATION and we include <platforms.h>
 * 	[91/06/12  14:08:46  rvb]
 * 
 * Revision 2.4  91/05/14  17:34:57  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:48:58  mrt
 * 	Added author notices
 * 	[91/02/04  11:22:56  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:26:37  mrt]
 * 
 * Revision 2.2  89/11/29  14:14:18  af
 * 	Created.
 * 	[89/10/04            af]
 * 
 */
/*
 *	File: mach_param.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Machine-dependent parameters for MIPS.
 */
#include <platforms.h>

#ifdef	DECSTATION
#define	HZ	(64)
				/* clock tick each 8 ms. (circa) */
#else	/*DECSTATION*/
#define	HZ	(100)
				/* clock tick each 10 ms. */
#endif	/*DECSTATION*/
