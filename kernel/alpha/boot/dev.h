/*  
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
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
 * Revision 2.2  93/02/05  08:00:32  danner
 * 	Adapted for alpha, from my mips code.
 * 	[93/02/04            af]
 * 
 */
/*
 *	File: dev.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	12/90
 *
 */
#ifndef	_BOOT_DEV_H_
#define	_BOOT_DEV_H_ 1

struct dev_t {
	int		handle;
	unsigned int	first_block;
	unsigned int	last_block;
};

typedef unsigned int recnum_t;

#endif	/* _BOOT_DEV_H_ */
