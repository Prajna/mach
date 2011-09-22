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
 * Teledebug debugging header file.
 *
 * HISTORY:
 * $Log:	ttd_debug.h,v $
 * Revision 2.2  93/05/10  23:24:42  rvb
 * 	Checkin for MK80 branch.
 * 	[93/05/10  15:08:12  grm]
 * 
 * Revision 2.1.2.1  93/03/03  14:37:46  grm
 * 	Gratuitous checkin for branch reasons.
 * 	[93/03/03            grm]
 * 
 * Revision 2.1.1.1  93/01/28  15:21:00  grm
 * 	Initial version.
 * 
 * 
 */

#ifndef	_TTD_DEBUG_H_
#define	_TTD_DEBUG_H_

#include <mach/vm_param.h>

extern	vm_offset_t	virtual_end;

#define	KERN_ADDR(m)	((m >= (ttd_address)VM_MIN_KERNEL_ADDRESS) && (m < virtual_end))

#define TTD_ASSERT(t,m) if (!(t)) {printf(m); while(1);}

#endif	/* _TTD_DEBUG_H_ */
