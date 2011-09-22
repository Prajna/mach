/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	xmm_server_rename.h,v $
 * Revision 2.4  92/03/10  16:29:41  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:51:47  jsb]
 * 
 * Revision 2.3.2.1  92/02/21  11:28:06  jsb
 * 	Removed rename of cover and backward compatibility routines.
 * 	[92/02/09  13:56:07  jsb]
 * 
 * Revision 2.3  91/08/28  11:16:28  jsb
 * 	Added xxx_memory_object_lock_request, memory_object_data_supply,
 * 	memory_object_ready, and memory_object_change_attributes.
 * 	[91/08/16  14:32:26  jsb]
 * 
 * Revision 2.2  91/06/17  15:48:38  jsb
 * 	First checkin.
 * 	[91/06/17  11:09:08  jsb]
 * 
 */
/*
 *	File:	norma/xmm_server_rename.h
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Renames to interpose on memory_object.defs server side.
 */

#include <norma_vm.h>


/*
 * The following routines:
 *
 *	memory_object_set_attributes
 *	memory_object_change_attributes
 *	memory_object_ready
 *
 * are just cover routines for memory_object_set_attributes_common.
 * We don't rename these, but rather conditionalize them out, since
 * we have our own cover routines which call K_SET_READY directly.
 *
 * The routines:
 *
 *	memory_object_data_provided
 *	xxx_memory_object_lock_request
 *
 * are also cover routines for which we have our own versions.
 */

#if	NORMA_VM
#define	memory_object_data_unavailable	k_memory_object_data_unavailable
#define	memory_object_get_attributes	k_memory_object_get_attributes
#define	memory_object_lock_request	k_memory_object_lock_request
#define	memory_object_data_error	k_memory_object_data_error
#define	memory_object_destroy		k_memory_object_destroy
#define	memory_object_data_supply	k_memory_object_data_supply
#endif	NORMA_VM
