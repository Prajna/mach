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
 * $Log:	ipc_object.h,v $
 * Revision 2.7  92/05/21  17:11:06  jfriedl
 * 	Appended 'U' to long constants that would otherwise be signed.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.6  91/10/09  16:09:27  af
 * 	 Revision 2.5.2.1  91/09/16  10:15:46  rpd
 * 	 	Added (unconditional) ipc_object_print declaration.
 * 	 	[91/09/02            rpd]
 * 
 * Revision 2.5.2.1  91/09/16  10:15:46  rpd
 * 	Added (unconditional) ipc_object_print declaration.
 * 	[91/09/02            rpd]
 * 
 * Revision 2.5  91/05/14  16:35:04  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:22:53  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:49:25  mrt]
 * 
 * Revision 2.3  90/11/05  14:29:21  rpd
 * 	Removed ipc_object_reference_macro, ipc_object_release_macro.
 * 	Created new io_reference, io_release macros.
 * 	[90/10/29            rpd]
 * 
 * Revision 2.2  90/06/02  14:51:04  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:00:05  rpd]
 * 
 */
/*
 *	File:	ipc/ipc_object.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Definitions for IPC objects, for which tasks have capabilities.
 */

#ifndef	_IPC_IPC_OBJECT_H_
#define _IPC_IPC_OBJECT_H_

#include <mach_ipc_compat.h>

#include <mach/kern_return.h>
#include <mach/message.h>
#include <kern/lock.h>
#include <kern/macro_help.h>
#include <kern/zalloc.h>

typedef unsigned int ipc_object_refs_t;
typedef unsigned int ipc_object_bits_t;
typedef unsigned int ipc_object_type_t;

typedef struct ipc_object {
	decl_simple_lock_data(,io_lock_data)
	ipc_object_refs_t io_references;
	ipc_object_bits_t io_bits;
} *ipc_object_t;

#define	IO_NULL			((ipc_object_t) 0)
#define	IO_DEAD			((ipc_object_t) -1)

#define	IO_VALID(io)		(((io) != IO_NULL) && ((io) != IO_DEAD))

#define	IO_BITS_KOTYPE		0x0000ffff	/* used by the object */
#define IO_BITS_OTYPE		0x7fff0000 	/* determines a zone */
#define	IO_BITS_ACTIVE		0x80000000U	/* is object alive? */

#define	io_active(io)		((int)(io)->io_bits < 0)	/* hack */

#define	io_otype(io)		(((io)->io_bits & IO_BITS_OTYPE) >> 16)
#define	io_kotype(io)		((io)->io_bits & IO_BITS_KOTYPE)

#define	io_makebits(active, otype, kotype)	\
	(((active) ? IO_BITS_ACTIVE : 0) | ((otype) << 16) | (kotype))

#define	IOT_PORT		0
#define IOT_PORT_SET		1
#define IOT_NUMBER		2		/* number of types used */

extern zone_t ipc_object_zones[IOT_NUMBER];

#define	io_alloc(otype)		\
		((ipc_object_t) zalloc(ipc_object_zones[(otype)]))

#define	io_free(otype, io)	\
		zfree(ipc_object_zones[(otype)], (vm_offset_t) (io))

#define	io_lock_init(io)	simple_lock_init(&(io)->io_lock_data)
#define	io_lock(io)		simple_lock(&(io)->io_lock_data)
#define	io_lock_try(io)		simple_lock_try(&(io)->io_lock_data)
#define	io_unlock(io)		simple_unlock(&(io)->io_lock_data)

#define io_check_unlock(io) 						\
MACRO_BEGIN								\
	ipc_object_refs_t _refs = (io)->io_references;			\
									\
	io_unlock(io);							\
	if (_refs == 0)							\
		io_free(io_otype(io), io);				\
MACRO_END

#define	io_reference(io)						\
MACRO_BEGIN								\
	(io)->io_references++;						\
MACRO_END

#define	io_release(io)							\
MACRO_BEGIN								\
	(io)->io_references--;						\
MACRO_END

extern void
ipc_object_reference(/* ipc_object_t */);

extern void
ipc_object_release(/* ipc_object_t */);

extern kern_return_t
ipc_object_translate(/* ipc_space_t, mach_port_t,
			mach_port_right_t, ipc_object_t * */);

extern kern_return_t
ipc_object_alloc_dead(/* ipc_space_t, mach_port_t * */);

extern kern_return_t
ipc_object_alloc_dead_name(/* ipc_space_t, mach_port_t */);

extern kern_return_t
ipc_object_alloc(/* ipc_space_t, ipc_object_type_t,
		    mach_port_type_t, mach_port_urefs_t,
		    mach_port_t *, ipc_object_t * */);

extern kern_return_t
ipc_object_alloc_name(/* ipc_space_t, ipc_object_type_t,
			 mach_port_type_t, mach_port_urefs_t,
			 mach_port_t, ipc_object_t * */);

extern mach_msg_type_name_t
ipc_object_copyin_type(/* mach_msg_type_name_t */);

extern kern_return_t
ipc_object_copyin(/* ipc_space_t, mach_port_t,
		     mach_msg_type_name_t, ipc_object_t * */);

extern void
ipc_object_copyin_from_kernel(/* ipc_object_t, mach_msg_type_name_t */);

extern void
ipc_object_destroy(/* ipc_object_t, mach_msg_type_name_t */);

extern kern_return_t
ipc_object_copyout(/* ipc_space_t, ipc_object_t,
		      mach_msg_type_name_t, boolean_t, mach_port_t * */);

extern kern_return_t
ipc_object_copyout_name(/* ipc_space_t, ipc_object_t,
			   mach_msg_type_name_t, boolean_t, mach_port_t */);

extern void
ipc_object_copyout_dest(/* ipc_space_t, ipc_object_t,
			   mach_msg_type_name_t, mach_port_t * */);

extern kern_return_t
ipc_object_rename(/* ipc_space_t, mach_port_t, mach_port_t */);

#if	MACH_IPC_COMPAT

extern mach_msg_type_name_t
ipc_object_copyout_type_compat(/* mach_msg_type_name_t */);

extern kern_return_t
ipc_object_copyin_compat(/* ipc_space_t, mach_port_t,
			    mach_msg_type_name_t, boolean_t,
			    ipc_object_t * */);

extern kern_return_t
ipc_object_copyin_header(/* ipc_space_t, mach_port_t,
			    ipc_object_t *, mach_msg_type_name_t * */);

extern kern_return_t
ipc_object_copyout_compat(/* ipc_space_t, ipc_object_t,
			     mach_msg_type_name_t, mach_port_t * */);

extern kern_return_t
ipc_object_copyout_name_compat(/* ipc_space_t, ipc_object_t,
				  mach_msg_type_name_t, mach_port_t */);

#endif	MACH_IPC_COMPAT

extern void
ipc_object_print(/* ipc_object_t */);

#endif	_IPC_IPC_OBJECT_H_
