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
 * $Log:	mach.h,v $
 * Revision 2.5  91/08/28  11:19:13  jsb
 * 	Synced up with USER area mach.h by removing exc.h, mach_host.h,
 * 	memory_object_default.h, and memory_object_user.h.
 * 	[91/08/27  16:39:43  jsb]
 * 
 * Revision 2.4  91/05/14  17:52:53  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/14  14:17:30  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:44:03  mrt]
 * 
 * Revision 2.2  90/06/02  15:12:22  rpd
 * 	Created
 * 	[89/05/24            mrt]
 * 
 */
/* 
 *  Includes all the types that a normal user
 *  of Mach programs should need
 */

#ifndef	_MACH_H_
#define	_MACH_H_

#include <mach/mach_types.h>
#include <mach/mach_interface.h>
#include <mach/mach_port.h>
#include <mach_init.h>

#endif	_MACH_H_
