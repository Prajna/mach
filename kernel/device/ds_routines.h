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
 * $Log:	ds_routines.h,v $
 * Revision 2.5  91/08/28  11:11:22  jsb
 * 	Page list support: device_write_dealloc returns a boolean.
 * 	[91/08/05  17:32:28  dlb]
 * 
 * Revision 2.4  91/05/14  15:47:56  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:09:33  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:29:15  mrt]
 * 
 * Revision 2.2  89/09/08  11:24:24  dbg
 * 	Created.
 * 	[89/08/04            dbg]
 * 
 */
/*
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date: 	8/89
 *
 *	Device service utility routines.
 */

#ifndef	DS_ROUTINES_H
#define	DS_ROUTINES_H

#include <vm/vm_map.h>
#include <device/device_types.h>

/*
 * Map for device IO memory.
 */
vm_map_t	device_io_map;

kern_return_t	device_read_alloc();
kern_return_t	device_write_get();
boolean_t	device_write_dealloc();

boolean_t	ds_open_done();
boolean_t	ds_read_done();
boolean_t	ds_write_done();

#endif	DS_ROUTINES_H
