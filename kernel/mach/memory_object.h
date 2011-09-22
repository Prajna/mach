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
 * $Log:	memory_object.h,v $
 * Revision 2.9  93/01/14  17:44:47  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 
 * Revision 2.8  92/05/21  17:22:24  jfriedl
 * 	     Removed coment starter from within comments to shut up gcc warnings.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.7  92/03/10  16:27:05  jsb
 * 	Added MEMORY_OBJECT_COPY_TEMPORARY.
 * 	[92/02/11  07:56:35  jsb]
 * 
 * Revision 2.6  91/08/28  11:15:22  jsb
 * 	Add defs for memory_object_return_t.
 * 	[91/07/03  14:06:26  dlb]
 * 
 * Revision 2.5  91/05/18  14:35:05  rpd
 * 	Removed memory_manager_default.
 * 	[91/03/22            rpd]
 * 
 * Revision 2.4  91/05/14  16:55:55  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:34:01  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:19:11  mrt]
 * 
 * Revision 2.2  90/06/02  14:58:56  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  22:35:26  rpd]
 * 
 * Revision 2.1  89/08/03  16:02:52  rwd
 * Created.
 * 
 * Revision 2.5  89/02/25  18:38:23  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.4  89/02/07  00:54:07  mwyoung
 * Relocated from vm/memory_object.h
 * 
 * Revision 2.3  89/01/30  22:08:42  rpd
 * 	Updated includes to the new style.  Fixed log.
 * 	Made variable declarations use "extern".
 * 	[89/01/25  15:25:20  rpd]
 */
/*
 *	File:	memory_object.h
 *	Author:	Michael Wayne Young
 *
 *	External memory management interface definition.
 */

#ifndef	_MACH_MEMORY_OBJECT_H_
#define _MACH_MEMORY_OBJECT_H_

/*
 *	User-visible types used in the external memory
 *	management interface:
 */

#include <mach/port.h>

typedef	mach_port_t	memory_object_t;
					/* Represents a memory object ... */
					/*  Used by user programs to specify */
					/*  the object to map; used by the */
					/*  kernel to retrieve or store data */

typedef	mach_port_t	memory_object_control_t;
					/* Provided to a memory manager; ... */
					/*  used to control a memory object */

typedef	mach_port_t	memory_object_name_t;
					/* Used to describe the memory ... */
					/*  object in vm_regions() calls */

typedef	int		memory_object_copy_strategy_t;
					/* How memory manager handles copy: */
#define		MEMORY_OBJECT_COPY_NONE		0
					/* ... No special support */
#define		MEMORY_OBJECT_COPY_CALL		1
					/* ... Make call on memory manager */
#define		MEMORY_OBJECT_COPY_DELAY 	2
					/* ... Memory manager doesn't ... */
					/*     change data externally. */
#define		MEMORY_OBJECT_COPY_TEMPORARY 	3
					/* ... Memory manager doesn't ... */
					/*     change data externally, and */
					/*     doesn't need to see changes. */

typedef	int		memory_object_return_t;
					/* Which pages to return to manager
					   this time (lock_request) */
#define		MEMORY_OBJECT_RETURN_NONE	0
					/* ... don't return any. */
#define		MEMORY_OBJECT_RETURN_DIRTY	1
					/* ... only dirty pages. */
#define		MEMORY_OBJECT_RETURN_ALL	2
					/* ... dirty and precious pages. */

#define		MEMORY_OBJECT_NULL	MACH_PORT_NULL

#endif	/* _MACH_MEMORY_OBJECT_H_ */
