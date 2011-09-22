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
 * $Log:	param.h,v $
 * Revision 2.6  91/05/14  16:00:44  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:10:13  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:30:33  mrt]
 * 
 * Revision 2.4  90/12/05  23:27:45  af
 * 
 * 
 * Revision 2.3  90/12/05  20:42:08  af
 * 	Fixed missing parenthesis.
 * 	[90/11/29            af]
 * 
 * Revision 2.2  90/08/27  21:55:37  dbg
 * 	Re-wrote from scratch.
 * 	[90/07/16            dbg]
 * 
 */
/*
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date: 	7/90
 */

#ifndef	_DEVICE_PARAM_H_
#define	_DEVICE_PARAM_H_

/*
 * Compatibility definitions for disk IO.
 */

/*
 * Disk devices do all IO in 512-byte blocks.
 */
#define	DEV_BSIZE	512

/*
 * Conversion between bytes and disk blocks.
 */
#define	btodb(byte_offset)	((byte_offset) >> 9)
#define	dbtob(block_number)	((block_number) << 9)

#endif	/* _DEVICE_PARAM_H_ */
