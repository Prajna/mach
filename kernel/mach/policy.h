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
 * $Log:	policy.h,v $
 * Revision 2.5  93/01/14  17:46:25  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 
 * Revision 2.4  91/05/14  16:58:29  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:35:22  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:20:11  mrt]
 * 
 * Revision 2.2  90/06/02  14:59:37  rpd
 * 	Created for new host/processor technology.
 * 	[90/03/26  23:51:22  rpd]
 * 
 * 	Cleanup changes.
 * 	[89/08/02            dlb]
 * 	Created.
 * 	[89/07/25  18:47:00  dlb]
 * 
 * Revision 2.3  89/10/15  02:05:50  rpd
 * 	Minor cleanups.
 * 
 * Revision 2.2  89/10/11  14:40:53  dlb
 * 	Cleanup changes.
 * 	[89/08/02            dlb]
 * 
 */

#ifndef	_MACH_POLICY_H_
#define _MACH_POLICY_H_

/*
 *	mach/policy.h
 *
 *	Definitions for scheduing policy.
 */

/*
 *	Policy definitions.  Policies must be powers of 2.
 */
#define	POLICY_TIMESHARE	1
#define POLICY_FIXEDPRI		2
#define POLICY_LAST		2

#define invalid_policy(policy)	(((policy) <= 0) || ((policy) > POLICY_LAST))

#endif /* _MACH_POLICY_H_ */
