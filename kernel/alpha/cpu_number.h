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
 * $Log:	cpu_number.h,v $
 * Revision 2.2  93/01/14  17:12:30  danner
 * 	Created.
 * 	[92/12/10  14:55:43  af]
 * 
 */
/*
 *	File: cpu_number.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Machine-dependent definitions for cpu identification.
 *
 *	On the ALPHA, cpu_number() is the WHAMI IPR.
 */

#ifndef	_ALPHA_CPU_NUMBER_H_
#define	_ALPHA_CPU_NUMBER_H_

extern unsigned long	cpu_number();

#endif	/* _ALPHA_CPU_NUMBER_H_ */
