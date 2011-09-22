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
 * $Log:	dev.h,v $
 * Revision 2.5  91/05/14  17:17:36  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:38:41  mrt
 * 	Added author notices
 * 	[91/02/04  11:09:35  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:04:20  mrt]
 * 
 * Revision 2.3  90/12/05  23:29:27  af
 * 	Created.
 * 	[90/12/02            af]
 * 
 */
/*
 *	File: dev.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	12/90
 *
 */
struct dev_t {
	int		handle;
	unsigned int	first_block;
	unsigned int	last_block;
};
