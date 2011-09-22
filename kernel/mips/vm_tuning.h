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
 * $Log:	vm_tuning.h,v $
 * Revision 2.8  91/05/18  14:37:27  rpd
 * 	Removed VM_PAGEOUT_BURST_WAIT.
 * 	[91/04/07            rpd]
 * 
 * Revision 2.7  91/05/14  17:39:36  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/02/05  17:52:27  mrt
 * 	Added author notices
 * 	[91/02/04  11:25:44  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:30:32  mrt]
 * 
 * Revision 2.5  90/10/12  12:38:49  rpd
 * 	Never override the inactive target.  The new software
 * 	reference bit code wants a large inactive target.
 * 	[90/10/09            rpd]
 * 
 * Revision 2.4  90/09/28  16:57:23  jsb
 * 	Only override vm_page_inactive_target when not using reference bits.
 * 	[90/09/21            rpd]
 * 
 * Revision 2.3  90/06/02  15:03:41  rpd
 * 	Corrected comments.
 * 	Added VM_PAGEOUT_BURST_WAIT.
 * 	[90/04/18            rpd]
 * 
 * Revision 2.2  89/11/29  14:15:42  af
 * 	Created
 * 	[89/10/29  15:19:15  af]
 * 
 */
/*
 *	File:	mips/vm_tuning.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	VM tuning parameters for the mips.
 */

#ifndef	_MIPS_VM_TUNING_H_
#define	_MIPS_VM_TUNING_H_

#endif	_MIPS_VM_TUNING_H_
