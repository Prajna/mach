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
 * $Log:	hash_info.h,v $
 * Revision 2.5  93/01/14  17:49:21  danner
 * 	64bit cleanup.
 * 	[92/12/01            af]
 * 
 * Revision 2.4  91/05/14  17:03:21  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:37:46  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:28:22  mrt]
 * 
 * Revision 2.2  91/01/08  15:18:59  rpd
 * 	Created.
 * 	[91/01/02            rpd]
 * 
 */

#ifndef	_MACH_DEBUG_HASH_INFO_H_
#define _MACH_DEBUG_HASH_INFO_H_

/*
 *	Remember to update the mig type definitions
 *	in mach_debug_types.defs when adding/removing fields.
 */

typedef struct hash_info_bucket {
	natural_t	hib_count;	/* number of records in bucket */
} hash_info_bucket_t;

typedef hash_info_bucket_t *hash_info_bucket_array_t;

#endif	_MACH_DEBUG_HASH_INFO_H_
