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
 * $Log:	xmm_obj.h,v $
 * Revision 2.5  92/03/10  16:29:25  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:51:33  jsb]
 * 
 * Revision 2.4.2.2  92/02/21  11:27:30  jsb
 * 	Changed mobj field in xmm_reply to kobj, and added kobj_held field.
 * 	Changed xmm_reply_allocate_send_once macro accordingly.
 * 	[92/02/16  18:23:36  jsb]
 * 
 * 	Added explicit name paramater to xmm_decl, since not all cpps
 * 	recoginize arguments inside quotes. Sigh.
 * 	[92/02/16  14:14:38  jsb]
 * 
 * 	Defined more robust xmm_decl.
 * 	[92/02/09  12:55:40  jsb]
 * 
 * Revision 2.4.2.1  92/01/21  21:54:33  jsb
 * 	Moved IKOT_XMM_* definitions to kern/ipc_kobject.h.
 * 	[92/01/21  18:21:45  jsb]
 * 
 * 	De-linted. Supports new (dlb) memory object routines.
 * 	Supports arbitrary reply ports to lock_request, etc.
 * 	Converted mach_port_t (and port_t) to ipc_port_t.
 * 	Added XMM_USE_MACROS conditional, off by default,
 * 	which toggles between macros and functions for invocation
 * 	routines. (Right now only the functions are provided.)
 * 	[92/01/20  17:27:35  jsb]
 * 
 * Revision 2.4  91/07/01  08:26:25  jsb
 * 	Removed malloc, free definitions.
 * 	Added xmm_decl macro.
 * 	Renamed Xobj_allocate to xmm_obj_allocate.
 * 	Added zone element to xmm_class structure.
 * 	[91/06/29  14:41:25  jsb]
 * 
 * Revision 2.3  91/06/18  20:53:00  jsb
 * 	Removed bogus include.
 * 	[91/06/18  19:06:29  jsb]
 * 
 * Revision 2.2  91/06/17  15:48:28  jsb
 * 	First checkin.
 * 	[91/06/17  11:03:46  jsb]
 * 
 */
/*
 *	File:	norma/xmm_obj.h
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Common definitions for xmm system.
 */

#ifndef	_NORMA_XMM_OBJ_H_
#define	_NORMA_XMM_OBJ_H_

#ifdef	KERNEL
#include <mach/std_types.h>	/* For pointer_t */
#include <mach/mach_types.h>
#include <mach/kern_return.h>
#include <mach/boolean.h>
#include <mach/vm_prot.h>
#include <mach/message.h>
#include <kern/zalloc.h>
#include <kern/assert.h>
#else	KERNEL
#include <mach.h>
#include <xmm_hash.h>
#endif	KERNEL

typedef kern_return_t		(*kern_routine_t)();
typedef void			(*void_routine_t)();

typedef struct xmm_class	*xmm_class_t;
typedef struct xmm_obj		*xmm_obj_t;
typedef struct xmm_reply	*xmm_reply_t;

#define	XMM_CLASS_NULL		((xmm_class_t) 0)
#define	XMM_OBJ_NULL		((xmm_obj_t) 0)
#define	XMM_REPLY_NULL		((xmm_reply_t) 0)

#define	MOBJ			((struct mobj *) mobj)
#define	KOBJ			((struct kobj *) kobj)

struct xmm_class {
	kern_routine_t	m_init;
	kern_routine_t	m_terminate;
	void_routine_t	m_deallocate;
	kern_routine_t	m_copy;
	kern_routine_t	m_data_request;
	kern_routine_t	m_data_unlock;
	kern_routine_t	m_data_write;
	kern_routine_t	m_lock_completed;
	kern_routine_t	m_supply_completed;
	kern_routine_t	m_data_return;
	kern_routine_t	m_change_completed;

	kern_routine_t	k_data_unavailable;
	kern_routine_t	k_get_attributes;
	kern_routine_t	k_lock_request;
	kern_routine_t	k_data_error;
	kern_routine_t	k_set_ready;
	kern_routine_t	k_destroy;
	kern_routine_t	k_data_supply;

	char *		c_name;
	int		c_size;
	zone_t		c_zone;
};

#if	__STDC__

#define xmm_decl(class, name, size)				\
extern kern_return_t m_ ## class ## _init();			\
extern kern_return_t m_ ## class ## _terminate();		\
extern void	     m_ ## class ## _deallocate();		\
extern kern_return_t m_ ## class ## _copy();			\
extern kern_return_t m_ ## class ## _data_request();		\
extern kern_return_t m_ ## class ## _data_unlock();		\
extern kern_return_t m_ ## class ## _data_write();		\
extern kern_return_t m_ ## class ## _lock_completed();		\
extern kern_return_t m_ ## class ## _supply_completed();	\
extern kern_return_t m_ ## class ## _data_return();		\
extern kern_return_t m_ ## class ## _change_completed();	\
extern kern_return_t k_ ## class ## _data_unavailable();	\
extern kern_return_t k_ ## class ## _get_attributes();		\
extern kern_return_t k_ ## class ## _lock_request();		\
extern kern_return_t k_ ## class ## _data_error();		\
extern kern_return_t k_ ## class ## _set_ready();		\
extern kern_return_t k_ ## class ## _destroy();			\
extern kern_return_t k_ ## class ## _data_supply();		\
								\
struct xmm_class class ## _class = {				\
	m_ ## class ## _init,					\
	m_ ## class ## _terminate,				\
	m_ ## class ## _deallocate,				\
	m_ ## class ## _copy,					\
	m_ ## class ## _data_request,				\
	m_ ## class ## _data_unlock,				\
	m_ ## class ## _data_write,				\
	m_ ## class ## _lock_completed,				\
	m_ ## class ## _supply_completed,			\
	m_ ## class ## _data_return,				\
	m_ ## class ## _change_completed,			\
								\
	k_ ## class ## _data_unavailable,			\
	k_ ## class ## _get_attributes,				\
	k_ ## class ## _lock_request,				\
	k_ ## class ## _data_error,				\
	k_ ## class ## _set_ready,				\
	k_ ## class ## _destroy,				\
	k_ ## class ## _data_supply,				\
								\
	name,							\
	size,							\
	ZONE_NULL,						\
}

#else	__STDC__

#define xmm_decl(class, name, size)				\
extern kern_return_t m_/**/class/**/_init();			\
extern kern_return_t m_/**/class/**/_terminate();		\
extern void	     m_/**/class/**/_deallocate();		\
extern kern_return_t m_/**/class/**/_copy();			\
extern kern_return_t m_/**/class/**/_data_request();		\
extern kern_return_t m_/**/class/**/_data_unlock();		\
extern kern_return_t m_/**/class/**/_data_write();		\
extern kern_return_t m_/**/class/**/_lock_completed();		\
extern kern_return_t m_/**/class/**/_supply_completed();	\
extern kern_return_t m_/**/class/**/_data_return();		\
extern kern_return_t m_/**/class/**/_change_completed();	\
extern kern_return_t k_/**/class/**/_data_unavailable();	\
extern kern_return_t k_/**/class/**/_get_attributes();		\
extern kern_return_t k_/**/class/**/_lock_request();		\
extern kern_return_t k_/**/class/**/_data_error();		\
extern kern_return_t k_/**/class/**/_set_ready();		\
extern kern_return_t k_/**/class/**/_destroy();			\
extern kern_return_t k_/**/class/**/_data_supply();		\
								\
struct xmm_class class/**/_class = {				\
	m_/**/class/**/_init,					\
	m_/**/class/**/_terminate,				\
	m_/**/class/**/_deallocate,				\
	m_/**/class/**/_copy,					\
	m_/**/class/**/_data_request,				\
	m_/**/class/**/_data_unlock,				\
	m_/**/class/**/_data_write,				\
	m_/**/class/**/_lock_completed,				\
	m_/**/class/**/_supply_completed,			\
	m_/**/class/**/_data_return,				\
	m_/**/class/**/_change_completed,			\
								\
	k_/**/class/**/_data_unavailable,			\
	k_/**/class/**/_get_attributes,				\
	k_/**/class/**/_lock_request,				\
	k_/**/class/**/_data_error,				\
	k_/**/class/**/_set_ready,				\
	k_/**/class/**/_destroy,				\
	k_/**/class/**/_data_supply,				\
								\
	name,							\
	size,							\
	ZONE_NULL,						\
}

#endif	__STDC__

struct xmm_obj {
	int		refcount;
	xmm_class_t	class;
	xmm_obj_t	m_mobj;
	xmm_obj_t	m_kobj;
	xmm_obj_t	k_mobj;
	xmm_obj_t	k_kobj;
};

struct xmm_reply {
	ipc_port_t	reply_to;
	mach_msg_type_name_t
	    		reply_to_type;
	xmm_obj_t	kobj;
	xmm_obj_t	kobj_held;
	ipc_port_t	reply_proxy;
};

xmm_reply_t	xmm_k_reply();
xmm_class_t	xmm_m_reply();

kern_return_t	m_interpose_init();
kern_return_t	m_interpose_terminate();
kern_return_t	m_interpose_copy();
kern_return_t	m_interpose_data_request();
kern_return_t	m_interpose_data_unlock();
kern_return_t	m_interpose_data_write();
kern_return_t	m_interpose_lock_completed();
kern_return_t	m_interpose_supply_completed();
kern_return_t	m_interpose_data_return();
kern_return_t	m_interpose_change_completed();
kern_return_t	k_interpose_data_unavailable();
kern_return_t	k_interpose_get_attributes();
kern_return_t	k_interpose_lock_request();
kern_return_t	k_interpose_data_error();
kern_return_t	k_interpose_set_ready();
kern_return_t	k_interpose_destroy();
kern_return_t	k_interpose_data_supply();

kern_return_t	m_invalid_init();
kern_return_t	m_invalid_terminate();
kern_return_t	m_invalid_copy();
kern_return_t	m_invalid_data_request();
kern_return_t	m_invalid_data_unlock();
kern_return_t	m_invalid_data_write();
kern_return_t	m_invalid_lock_completed();
kern_return_t	m_invalid_supply_completed();
kern_return_t	m_invalid_data_return();
kern_return_t	m_invalid_change_completed();
kern_return_t	k_invalid_data_unavailable();
kern_return_t	k_invalid_get_attributes();
kern_return_t	k_invalid_lock_request();
kern_return_t	k_invalid_data_error();
kern_return_t	k_invalid_set_ready();
kern_return_t	k_invalid_destroy();
kern_return_t	k_invalid_data_supply();

kern_return_t	xmm_obj_allocate();
kern_return_t	xmm_reply_allocate();
xmm_reply_t	convert_port_to_reply();

extern ipc_port_t remote_host_priv();

/*
 * Central repository of magic reply_to_types uses for non-port reply_tos.
 */
#define	XMM_SVM_REPLY		(MACH_MSG_TYPE_LAST + 1)
#define	XMM_SPLIT_REPLY		(MACH_MSG_TYPE_LAST + 2)
#define	XMM_BUFFER_REPLY	(MACH_MSG_TYPE_LAST + 3)

#define	xmm_reply_allocate_send_once(kobj, reply_to, replyp)\
xmm_reply_allocate((kobj), (reply_to), MACH_MSG_TYPE_PORT_SEND_ONCE, (replyp))

/*
 * More meaningful parameter constants for memory_object calls.
 */
#define	OBJECT_READY_FALSE	FALSE
#define	MAY_CACHE_FALSE		FALSE
#define	USE_OLD_PAGEOUT_FALSE	FALSE
#define	PRECIOUS_FALSE		FALSE

#define	OBJECT_READY_TRUE	TRUE
#define	MAY_CACHE_TRUE		TRUE
#define	USE_OLD_PAGEOUT_TRUE	TRUE
#define	PRECIOUS_TRUE		TRUE

#ifndef	KERNEL
char	*malloc();
int	free();

/*
 * XXX should find or define vm_page_shift
 */
#define	atop(addr)	((addr) / vm_page_size)
#define	ptoa(page)	((page) * vm_page_size)
#endif	KERNEL

#define	XMM_USE_MACROS	0

#ifdef	lint
#undef	XMM_USE_MACROS
#define	XMM_USE_MACROS	0
#endif

#if	XMM_USE_MACROS

@@@_need_to_rewrite_macros_before_this_option_can_be_enabled_@@@;

#else	XMM_USE_MACROS

#endif	XMM_USE_MACROS

#endif	_NORMA_XMM_OBJ_H_
