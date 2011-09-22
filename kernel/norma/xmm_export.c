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
 * $Log:	xmm_export.c,v $
 * Revision 2.5  92/03/10  16:29:05  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:51:14  jsb]
 * 
 * Revision 2.4.2.3  92/02/21  11:25:50  jsb
 * 	In _proxy_terminate, deallocate xmm_pager and release xmm_kernel.
 * 	[92/02/20  15:46:35  jsb]
 * 
 * 	Reference mobj on port to mobj conversion; release when done.
 * 	[92/02/20  10:54:05  jsb]
 * 
 * 	Changed MACH_PORT use to IP_NULL. Use m_interpose_deallocate.
 * 	[92/02/18  17:13:28  jsb]
 * 
 * 	Changed reply->mobj to reply->kobj.
 * 	[92/02/16  18:22:12  jsb]
 * 
 * 	Explicitly provide name parameter to xmm_decl macro.
 * 	Hide and release mobj in _proxy_terminate.
 * 	[92/02/16  15:51:50  jsb]
 * 
 * 	Renamed xmm_export_notify to xmm_pager_notify.
 * 	[92/02/10  17:27:15  jsb]
 * 
 * 	Changed proxy_init to use xmm object instead of
 * 	<guessed host, memory_object> pair.
 * 	Renamed mobj_port to xmm_pager, and xmm_control to xmm_kernel.
 * 	[92/02/10  17:01:03  jsb]
 * 
 * 	Use new xmm_decl, and new memory_object_name and deallocation protocol.
 * 	[92/02/09  12:51:49  jsb]
 * 
 * Revision 2.4.2.2  92/01/21  21:54:06  jsb
 * 	Added xmm_export_notify stub.
 * 	[92/01/21  18:22:48  jsb]
 * 
 * 	Use ports instead of pointers when communicating with xmm_import.c.
 * 	De-linted. Supports new (dlb) memory object routines.
 * 	Supports arbitrary reply ports to lock_request, etc.
 * 	Converted mach_port_t (and port_t) to ipc_port_t.
 * 	[92/01/20  17:21:43  jsb]
 * 
 * 	Fixes from OSF.
 * 	[92/01/17  14:14:46  jsb]
 * 
 * Revision 2.4.2.1.1.1  92/01/15  12:15:33  jeffreyh
 * 	Deallocate memory object name port on termination. (dlb)
 * 
 * Revision 2.4.2.1  92/01/03  16:38:45  jsb
 * 	Added missing type cast.
 * 	[91/12/27  21:29:32  jsb]
 * 
 * 	Cleaned up debugging printf.
 * 	[91/12/24  14:30:28  jsb]
 * 
 * Revision 2.4  91/11/15  14:10:03  rpd
 * 	Use ipc_port_copy_send in _proxy_init for import_master.
 * 	[91/09/23  09:14:28  jsb]
 * 
 * Revision 2.3  91/07/01  08:26:07  jsb
 * 	Fixed object importation protocol.
 * 	Corrected declaration of _proxy_lock_completed.
 * 	[91/06/29  15:28:46  jsb]
 * 
 * Revision 2.2  91/06/17  15:48:15  jsb
 * 	First checkin.
 * 	[91/06/17  11:06:11  jsb]
 * 
 */
/*
 *	File:	norma/xmm_export.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Xmm layer for allowing remote kernels to map a local object.
 */

#include <norma/xmm_obj.h>
#include <kern/host.h>
#include <ipc/ipc_space.h>
#include <ipc/ipc_port.h>
#include <mach/notify.h>
#include <mach/proxy.h>

struct mobj {
	struct xmm_obj	obj;
	ipc_port_t	xmm_pager;
	ipc_port_t	xmm_kernel;
};

#undef  KOBJ
#define KOBJ    ((struct mobj *) kobj)

#define	m_export_init			m_invalid_init
#define	m_export_terminate		m_invalid_terminate
#define	m_export_deallocate		m_interpose_deallocate
#define	m_export_copy			m_invalid_copy
#define	m_export_data_request		m_invalid_data_request
#define	m_export_data_unlock		m_invalid_data_unlock
#define	m_export_data_write		m_invalid_data_write
#define	m_export_lock_completed		m_invalid_lock_completed
#define	m_export_supply_completed	m_invalid_supply_completed
#define	m_export_data_return		m_invalid_data_return
#define	m_export_change_completed	m_invalid_change_completed

xmm_decl(export, "export", sizeof(struct mobj));

extern ipc_port_t xmm_object_by_memory_object();

k_export_data_unavailable(kobj, offset, length)
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
{
#ifdef	lint
	K_DATA_UNAVAILABLE(kobj, offset, length);
#endif	lint
	return proxy_data_unavailable(KOBJ->xmm_kernel, offset, length);
}

k_export_get_attributes(kobj, object_ready, may_cache, copy_strategy)
	xmm_obj_t kobj;
	boolean_t *object_ready;
	boolean_t *may_cache;
	memory_object_copy_strategy_t *copy_strategy;
{
#ifdef	lint
	K_GET_ATTRIBUTES(kobj, object_ready, may_cache, copy_strategy);
#endif	lint
	return proxy_get_attributes(KOBJ->xmm_kernel, object_ready, may_cache,
				    copy_strategy);
}

k_export_lock_request(kobj, offset, length, should_clean, should_flush,
		      lock_value, reply)
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	boolean_t should_clean;
	boolean_t should_flush;
	vm_prot_t lock_value;
	xmm_reply_t reply;
{
	kern_return_t kr;

#ifdef	lint
	K_LOCK_REQUEST(kobj, offset, length, should_clean, should_flush,
		       lock_value, reply);
#endif	lint
	if (reply == XMM_REPLY_NULL) {
		return proxy_lock_request(KOBJ->xmm_kernel, offset, length,
					  should_clean, should_flush,
					  lock_value, IP_NULL);
	}
	kr = xmm_reply_allocate_proxy(reply);
	if (kr != KERN_SUCCESS) {
		return kr;
	}
	assert(reply != XMM_REPLY_NULL && reply->kobj == kobj);
	return proxy_lock_request(KOBJ->xmm_kernel, offset, length,
				  should_clean, should_flush, lock_value,
				  reply->reply_proxy);
}

k_export_data_error(kobj, offset, length, error_value)
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	kern_return_t error_value;
{
#ifdef	lint
	K_DATA_ERROR(kobj, offset, length, error_value);
#endif	lint
	return proxy_data_error(KOBJ->xmm_kernel, offset, length, error_value);
}

k_export_set_ready(kobj, object_ready, may_cache, copy_strategy,
		   use_old_pageout, memory_object_name, reply)
	xmm_obj_t kobj;
	boolean_t object_ready;
	boolean_t may_cache;
	memory_object_copy_strategy_t copy_strategy;
	boolean_t use_old_pageout;
	ipc_port_t memory_object_name;
	xmm_reply_t reply;
{
	kern_return_t kr;

#ifdef	lint
	K_SET_READY(kobj, object_ready, may_cache, copy_strategy,
		    use_old_pageout, memory_object_name, reply);
#endif	lint
	if (reply == XMM_REPLY_NULL) {
		return proxy_set_ready(KOBJ->xmm_kernel, KOBJ->xmm_pager,
				       object_ready, may_cache, copy_strategy,
				       KERN_SUCCESS, use_old_pageout,
				       memory_object_name, IP_NULL);
	}
	kr = xmm_reply_allocate_proxy(reply);
	if (kr != KERN_SUCCESS) {
		return kr;
	}
	assert(reply != XMM_REPLY_NULL && reply->kobj == kobj);
	return proxy_set_ready(KOBJ->xmm_kernel, KOBJ->xmm_pager,
			       object_ready, may_cache, copy_strategy,
			       KERN_SUCCESS, use_old_pageout,
			       memory_object_name, reply->reply_proxy);
}

k_export_destroy(kobj, reason)
	xmm_obj_t kobj;
	kern_return_t reason;
{
#ifdef	lint
	K_DESTROY(kobj, reason);
#endif	lint
	return proxy_destroy(KOBJ->xmm_kernel, reason);
}

k_export_data_supply(kobj, offset, data, length, lock_value, precious, reply)
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_offset_t data;
	vm_size_t length;
	vm_prot_t lock_value;
	boolean_t precious;
	xmm_reply_t reply;
{
	kern_return_t kr;

#ifdef	lint
	K_DATA_SUPPLY(kobj, offset, data, length, lock_value, precious, reply);
#endif	lint
	if (reply == XMM_REPLY_NULL) {
		return proxy_data_supply(KOBJ->xmm_kernel, offset, data, length,
					 lock_value, precious, IP_NULL);
	}
	kr = xmm_reply_allocate_proxy(reply);
	if (kr != KERN_SUCCESS) {
		return kr;
	}
	assert(reply != XMM_REPLY_NULL && reply->kobj == kobj);
	return proxy_data_supply(KOBJ->xmm_kernel, offset, data, length,
				 lock_value, precious, reply->reply_proxy);
}

xmm_obj_t
convert_xmm_pager_to_mobj(xmm_pager)
	ipc_port_t xmm_pager;
{
	xmm_obj_t mobj = XMM_OBJ_NULL;

	if (IP_VALID(xmm_pager)) {
		ip_lock(xmm_pager);
		if (ip_active(xmm_pager) &&
		    ip_kotype(xmm_pager) == IKOT_XMM_PAGER) {
			mobj = (xmm_obj_t) xmm_pager->ip_kobject;
			xmm_obj_reference(mobj);
		}
		ip_unlock(xmm_pager);
	}
	return mobj;
}

boolean_t
xmm_pager_notify(msg)
	mach_msg_header_t *msg;
{
	return FALSE;
}

_proxy_init(xmm_object, xmm_kernel, pagesize, internal, size)
	ipc_port_t xmm_object;
	ipc_port_t xmm_kernel;
	vm_size_t pagesize;
	boolean_t internal;
	vm_size_t size;
{
	xmm_obj_t mobj;
	kern_return_t kr;

	/*
	 * XXX
	 * Check for multiple inits and/or reuse of memory_object.
	 * XXX
	 * Should use proxy_set_ready to return errors.
	 */
	ip_lock(xmm_object);
	if (ip_kotype(xmm_object) != IKOT_XMM_OBJECT) {
		ip_unlock(xmm_object);
		return KERN_INVALID_ARGUMENT;
	}
	mobj = (xmm_obj_t) xmm_object->ip_kobject;
	ip_unlock(xmm_object);

	kr = xmm_obj_allocate(&export_class, mobj, &mobj);
	if (kr != KERN_SUCCESS) {
		printf("_proxy_init: xmm_obj_allocate: %x\n", kr);
		return kr;
	}

	MOBJ->xmm_pager = ipc_port_alloc_kernel();
	if (MOBJ->xmm_pager == IP_NULL) {
		panic("m_import_init: allocate xmm_pager");
	}
	ipc_kobject_set(MOBJ->xmm_pager, (ipc_kobject_t) mobj, IKOT_XMM_PAGER);
	MOBJ->xmm_kernel = xmm_kernel;

	xmm_obj_reference(mobj);
	kr = M_INIT(mobj, mobj, pagesize, internal, size);
	xmm_obj_release(mobj);
	return kr;
}

_proxy_terminate(xmm_pager)
	ipc_port_t xmm_pager;
{
	xmm_obj_t mobj;
	kern_return_t kr;

	mobj = convert_xmm_pager_to_mobj(xmm_pager);
	if (mobj == XMM_OBJ_NULL) {
		return KERN_FAILURE;
	}
	ipc_kobject_set(MOBJ->xmm_pager, IKO_NULL, IKOT_NONE);
	ipc_port_dealloc_kernel(MOBJ->xmm_pager);
	ipc_port_release_send(MOBJ->xmm_kernel);
	xmm_obj_release(mobj);
	kr = M_TERMINATE(mobj, mobj);
	xmm_obj_release(mobj);
	return kr;
}

_proxy_copy(xmm_pager, offset, length, new_memory_object)
	ipc_port_t xmm_pager;
	vm_offset_t offset;
	vm_size_t length;
	memory_object_t new_memory_object;
{
	xmm_obj_t mobj;

#ifdef	lint
	offset++;
	length++;
	new_memory_object++;
#endif
	mobj = convert_xmm_pager_to_mobj(xmm_pager);
	if (mobj == XMM_OBJ_NULL) {
		return KERN_FAILURE;
	}
	panic("_proxy_copy: not implemented\n");
	xmm_obj_release(mobj);
	return KERN_FAILURE;
}

_proxy_data_request(xmm_pager, offset, length, desired_access)
	ipc_port_t xmm_pager;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
	xmm_obj_t mobj;
	kern_return_t kr;

	mobj = convert_xmm_pager_to_mobj(xmm_pager);
	if (mobj == XMM_OBJ_NULL) {
		return KERN_FAILURE;
	}
	kr = M_DATA_REQUEST(mobj, mobj, offset, length, desired_access);
	xmm_obj_release(mobj);
	return kr;
}

_proxy_data_unlock(xmm_pager, offset, length, desired_access)
	ipc_port_t xmm_pager;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
	xmm_obj_t mobj;
	kern_return_t kr;

	mobj = convert_xmm_pager_to_mobj(xmm_pager);
	if (mobj == XMM_OBJ_NULL) {
		return KERN_FAILURE;
	}
	kr = M_DATA_UNLOCK(mobj, mobj, offset, length, desired_access);
	xmm_obj_release(mobj);
	return kr;
}

_proxy_data_write(xmm_pager, offset, data, length)
	ipc_port_t xmm_pager;
	vm_offset_t offset;
	vm_offset_t data;
	vm_size_t length;
{
	xmm_obj_t mobj;
	kern_return_t kr;

	mobj = convert_xmm_pager_to_mobj(xmm_pager);
	if (mobj == XMM_OBJ_NULL) {
		return KERN_FAILURE;
	}
	kr = M_DATA_WRITE(mobj, mobj, offset, data, length);
	xmm_obj_release(mobj);
	return kr;
}

_proxy_lock_completed(reply_to, offset, length)
	ipc_port_t reply_to;
	vm_offset_t offset;
	vm_size_t length;
{
	xmm_reply_t reply;

	reply = convert_port_to_reply(reply_to);
	if (reply == XMM_REPLY_NULL) {
		return KERN_FAILURE;
	}
	return M_LOCK_COMPLETED(reply, offset, length);
}

_proxy_supply_completed(reply_to, offset, length, result, error_offset)
	ipc_port_t reply_to;
	vm_offset_t offset;
	vm_size_t length;
	kern_return_t result;
	vm_offset_t error_offset;
{
	xmm_reply_t reply;

	reply = convert_port_to_reply(reply_to);
	if (reply == XMM_REPLY_NULL) {
		return KERN_FAILURE;
	}
	return M_SUPPLY_COMPLETED(reply, offset, length, result, error_offset);
}

_proxy_data_return(xmm_pager, offset, data, length)
	ipc_port_t xmm_pager;
	vm_offset_t offset;
	vm_offset_t data;
	vm_size_t length;
{
	xmm_obj_t mobj;
	kern_return_t kr;

	mobj = convert_xmm_pager_to_mobj(xmm_pager);
	if (mobj == XMM_OBJ_NULL) {
		return KERN_FAILURE;
	}
	kr = M_DATA_RETURN(mobj, mobj, offset, data, length);
	xmm_obj_release(mobj);
	return kr;
}

_proxy_change_completed(reply_to, may_cache, copy_strategy)
	ipc_port_t reply_to;
	boolean_t may_cache;
	memory_object_copy_strategy_t copy_strategy;
{
	xmm_reply_t reply;

	reply = convert_port_to_reply(reply_to);
	if (reply == XMM_REPLY_NULL) {
		return KERN_FAILURE;
	}
	return M_CHANGE_COMPLETED(reply, may_cache, copy_strategy);
}
