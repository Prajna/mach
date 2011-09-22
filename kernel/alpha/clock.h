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
 * $Log:	clock.h,v $
 * Revision 2.2  93/03/10  09:03:38  danner
 * 	Copied from mips.
 * 	[93/03/09  18:10:16  af]
 * 
 * Revision 2.4  91/05/14  17:32:37  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:47:15  mrt
 * 	Added author notices
 * 	[91/02/04  11:21:14  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:24:48  mrt]
 * 
 * Revision 2.2  89/11/29  14:12:42  af
 * 	Adapted for pure kernel, removed BSD remnants.
 * 	[89/10/29            af]
 * 
 * Revision 2.1  89/05/30  12:55:27  rvb
 * Created.
 * 
 *  4-Jan-89  Alessandro Forin (af) at Carnegie-Mellon University
 *	Created.
 */

/*
 *	File: clock.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	1/89
 *
 *	General definitions for clock devices
 *
 */

/*
 * General time definitions
 */
#define	SECMIN	((unsigned)60)			/* seconds per minute */
#define	SECHOUR	((unsigned)(60*SECMIN))		/* seconds per hour */
#define	SECDAY	((unsigned)(24*SECHOUR))	/* seconds per day */
#define	SECYR	((unsigned)(365*SECDAY))	/* sec per reg year */

#define	YRREF		1970
#define	LEAPYEAR(x)	(((x) % 4) == 0)

