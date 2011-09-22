/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
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
 * Revision 2.2  93/01/14  17:13:23  danner
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:15:30  af]
 * 
 * 	Created.
 * 	[92/05/31            af]
 * 
 */
/*
 *	File: mach_param.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	5/92
 *
 *	Machine-dependent parameters for Alpha.
 */
#ifndef	_ALPHA_MACH_PARAM_H_
#define	_ALPHA_MACH_PARAM_H_ 1

/*
 * The clock frequency, unfortunately, was not defined once
 * and forall in the architecture.  This definition is here
 * to tell MI code we might have a high frequency clock.
 * Autoconf code takes care of fixing up the relevant vars
 * in kern/mach_clock.c.
 */

#define	HZ	(1000)	/* clock tick each 1 ms. */

#endif	/* _ALPHA_MACH_PARAM_H_ */
