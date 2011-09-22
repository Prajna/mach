/*
 * Mach Operating System
 * Copyright (c) 1993-1988 Carnegie Mellon University
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
 * $Log:	thread_status.h,v $
 * Revision 2.4  93/01/14  17:48:07  danner
 * 	Include mach/machine/vm_types.h
 * 	[93/01/14            danner]
 * 
 * 	Standardized include symbol name.
 * 	[92/06/10            pds]
 * 	64bit cleanup.
 * 	[92/12/01            af]
 * 
 * Revision 2.3  91/05/14  17:01:22  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:36:42  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:21:56  mrt]
 * 
 * Revision 2.1  89/08/03  16:06:18  rwd
 * Created.
 * 
 * Revision 2.4  89/02/25  18:41:29  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.3  89/02/07  00:53:47  mwyoung
 * Relocated from mach/thread_status.h
 * 
 * Revision 2.2  88/08/25  18:21:12  mwyoung
 * 	Adjusted include file references.
 * 	[88/08/16  04:16:13  mwyoung]
 * 	
 * 	Add THREAD_STATE_FLAVOR_LIST; remove old stuff.
 * 	[88/08/11  18:49:48  mwyoung]
 *
 */
/*
 *
 *	This file contains the structure definitions for the user-visible
 *	thread state.  This thread state is examined with the thread_get_state
 *	kernel call and may be changed with the thread_set_state kernel call.
 *
 */

#ifndef	_MACH_THREAD_STATUS_H_
#define	_MACH_THREAD_STATUS_H_

/*
 *	The actual structure that comprises the thread state is defined
 *	in the machine dependent module.
 */
#include <mach/machine/vm_types.h>
#include <mach/machine/thread_status.h>

/*
 *	Generic definition for machine-dependent thread status.
 */

typedef natural_t		*thread_state_t;	/* Variable-length array */

#define	THREAD_STATE_MAX	(1024)		/* Maximum array size */
typedef natural_t	thread_state_data_t[THREAD_STATE_MAX];

#define	THREAD_STATE_FLAVOR_LIST	0	/* List of valid flavors */

#endif	/* _MACH_THREAD_STATUS_H_ */
