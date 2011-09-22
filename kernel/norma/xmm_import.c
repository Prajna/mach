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
 * $Log:	xmm_import.c,v $
 * Revision 2.6  92/03/10  16:29:11  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:51:19  jsb]
 * 
 * Revision 2.5.2.5  92/02/21  11:25:54  jsb
 * 	Reference mobj on port to mobj conversion; release when done.
 * 	[92/02/20  10:53:45  jsb]
 * 
 * 	Deallocate all resources upon termination.
 * 	Deallocate allocated replies.
 * 	Changed MACH_PORT_NULL uses to IP_NULL.
 * 	[92/02/18  07:56:15  jsb]
 * 
 * 	Explicitly provide name parameter to xmm_decl macro.
 * 	Handle m_import_terminate being called before proxy_set_ready has been.
 * 	This can happen because the vm system may terminate an object after
 * 	it has been initialized but before it is ready.
 * 	[92/02/16  15:26:31  jsb]
 * 
 * 	Don't call proxy_terminate on a null port.
 * 	[92/02/11  18:21:03  jsb]
 * 
 * 	Added ipc_kobject_set to null to break port/mobj association.
 * 	Accordingly, removed dead field and associated logic.
 * 	[92/02/11  13:22:39  jsb]
 * 
 * 	Renamed xmm_import_notify to xmm_kernel_notify.
 * 	[92/02/10  17:26:50  jsb]
 * 
 * 	Use xmm object instead of <guessed host, memory_object> pair in
 * 	proxy_init. Renamed {mobj,kobj}_port to xmm_{pager,kernel}.
 * 	[92/02/10  16:56:20  jsb]
 * 
 * 	Use new xmm_decl, and new memory_object_name and deallocation protocol.
 * 	[92/02/09  12:51:12  jsb]
 * 
 * Revision 2.5.2.4  92/01/21  22:22:24  jsb
 * 	18-Jan-92 David L. Black (dlb) at Open Software Foundation
 * 	Add dead field to mobj and use to synchronize termination
 * 	against other operations.
 * 
 * Revision 2.5.2.3  92/01/21  21:54:12  jsb
 * 	Added xmm_import_notify stub.
 * 	[92/01/21  18:20:54  jsb]
 * 
 * 	Use ports instead of pointers when communicating with xmm_export.c.
 * 	De-linted. Supports new (dlb) memory object routines.
 * 	Supports arbitrary reply ports to lock_request, etc.
 * 	Converted mach_port_t (and port_t) to ipc_port_t.
 * 	[92/01/20  17:22:22  jsb]
 * 
 * 	Fixes from OSF.
 * 	[92/01/17  14:15:01  jsb]
 * 
 * Revision 2.5.2.2.1.1  92/01/15  12:16:26  jeffreyh
 * 	Pass memory_object_name port to proxy terminate. (dlb)
 * 
 * Revision 2.5.2.2  92/01/09  18:46:13  jsb
 * 	Use remote_host_priv() instead of norma_get_special_port().
 * 	[92/01/04  18:33:18  jsb]
 * 
 * Revision 2.5.2.1  92/01/03  16:38:50  jsb
 * 	Corrected log.
 * 	[91/12/24  14:33:29  jsb]
 * 
 * Revision 2.5  91/12/10  13:26:24  jsb
 * 	Added missing third parameter in call to proxy_terminate.
 * 	[91/12/10  12:48:52  jsb]
 * 
 * Revision 2.4  91/11/14  16:52:34  rpd
 * 	Replaced master_device_port_at_node call with calls to
 *	norma_get_special_port and norma_port_location_hint.
 * 	[91/11/00            jsb]
 * 
 * Revision 2.3  91/07/01  08:26:12  jsb
 * 	Fixed object importation protocol. Return valid return values.
 * 	[91/06/29  15:30:10  jsb]
 * 
 * Revision 2.2  91/06/17  15:48:18  jsb
 * 	First checkin.
 * 	[91/06/17  11:03:28  jsb]
 * 
 */
/*
 *	File:	norma/xmm_import.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Xmm layer for mapping a remote object.
 */

#include <norma/xmm_obj.h>
#include <norma/ipc_node.h>
#include <ipc/ipc_space.h>
#include <ipc/ipc_port.h>
#include <mach/notify.h>
#include <mach/proxy.h>

struct mobj {
	struct xmm_obj	obj;
	ipc_port_t	xmm_object;
	ipc_port_t	xmm_pager;
	ipc_port_t	xmm_kernel;
	boolean_t	terminated;
};

#undef  KOBJ
#define KOBJ    ((struct mobj *) kobj)

#define	m_import_deallocate		m_interpose_deallocate
#define	k_import_data_unavailable	k_invalid_data_unavailable
#define	k_import_get_attributes		k_invalid_get_attributes
#define	k_import_lock_request		k_invalid_lock_request
#define	k_import_data_error		k_invalid_data_error
#define	k_import_set_ready		k_invalid_set_ready
#define	k_import_destroy		k_invalid_destroy
#define	k_import_data_supply		k_invalid_data_supply

xmm_decl(import, "import", sizeof(struct mobj));

kern_return_t
xmm_import_create(xmm_object, new_mobj)
	ipc_port_t xmm_object;
	xmm_obj_t *new_mobj;
{
	xmm_obj_t mobj;
	kern_return_t kr;

	kr = xmm_obj_allocate(&import_class, XMM_OBJ_NULL, &mobj);
	if (kr != KERN_SUCCESS) {
		return kr;
	}

	MOBJ->xmm_object = xmm_object;
	MOBJ->xmm_pager = IP_NULL;
	MOBJ->xmm_kernel = IP_NULL;
	MOBJ->terminated = FALSE;

	*new_mobj = mobj;
	return KERN_SUCCESS;
}

m_import_init(mobj, k_kobj, pagesize, internal, size)
	xmm_obj_t mobj;
	xmm_obj_t k_kobj;
	vm_size_t pagesize;
	boolean_t internal;
	vm_size_t size;
{
	kern_return_t kr;
	xmm_obj_t kobj = mobj;
	
#ifdef	lint
	M_INIT(mobj, k_kobj, pagesize, internal, size);
#endif	lint
	xmm_kobj_link(kobj, k_kobj);

	MOBJ->xmm_kernel = ipc_port_alloc_kernel();
	if (MOBJ->xmm_kernel == IP_NULL) {
		panic("m_import_init: allocate xmm_kernel");
	}
	xmm_obj_reference(mobj);
	ipc_kobject_set(MOBJ->xmm_kernel, (ipc_kobject_t) mobj,
			IKOT_XMM_KERNEL);

	kr = proxy_init(MOBJ->xmm_object, MOBJ->xmm_kernel, pagesize,
			internal, size);
#if 1
	if (kr) {
		panic("m_import_init: proxy_init returns %x\n", kr);
	}
#endif
	return KERN_SUCCESS;
}

m_import_terminate(mobj, kobj)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
{
	kern_return_t kr;

#ifdef	lint
	M_TERMINATE(mobj, kobj);
#endif	lint
	assert(MOBJ->xmm_kernel != IP_NULL);
	if (MOBJ->xmm_pager == IP_NULL) {
		/*
		 * This can happen because the vm system only waits
		 * until the object is initialized before terminating
		 * it; it does not wait until the object is ready.
		 * We must therefore wait for _proxy_set_ready ourselves
		 * before terminating the object. We don't even have the
		 * option of calling proxy_terminate on xmm_object, since
		 * we no longer have send rights for xmm_object.
		 *
		 * We need to retain the xmm_kernel to mobj association
		 * so that we can process _proxy_set_ready. Fortunately,
		 * there is no need to break this association, since the
		 * only call that we should receive is _proxy_set_ready.
		 * Any spurious calls to anything else will be caught
		 * at the xmm_server level.
		 */
		printf("m_import_terminate on unready object 0x%x\n", mobj);
		MOBJ->terminated = TRUE;
		return KERN_SUCCESS;
	}
	/*
	 * Deallocate resources associated with mobj.
	 * MOBJ->xmm_object was deallocated via proxy_init.
	 * MOBJ->xmm_kernel will be deallocated via proxy_terminate.
	 * We must explicitly deallocate MOBJ->xmm_pager
	 * (to make mobj inaccessible) and then release mobj.
	 */
	ipc_kobject_set(MOBJ->xmm_kernel, IKO_NULL, IKOT_NONE);
	ipc_port_dealloc_kernel(MOBJ->xmm_kernel);
	kr = proxy_terminate(MOBJ->xmm_pager);
	assert(kr == KERN_SUCCESS);
	xmm_obj_release(mobj);
	return KERN_SUCCESS;
}

m_import_copy(mobj, kobj, offset, length, new_mobj)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	xmm_obj_t new_mobj;
{
#ifdef	lint
	M_COPY(mobj, kobj, offset, length, new_mobj);
#endif	lint
	panic("m_import_copy\n");
}

/*
 *	VM system should handle ready synchronization for everything else
 */

m_import_data_request(mobj, kobj, offset, length, desired_access)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
#ifdef	lint
	M_DATA_REQUEST(mobj, kobj, offset, length, desired_access);
#endif	lint
	return proxy_data_request(MOBJ->xmm_pager, offset, length,
				  desired_access);
}

m_import_data_unlock(mobj, kobj, offset, length, desired_access)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
#ifdef	lint
	M_DATA_UNLOCK(mobj, kobj, offset, length, desired_access);
#endif	lint
	return proxy_data_unlock(MOBJ->xmm_pager, offset, length,
				 desired_access);
}

m_import_data_write(mobj, kobj, offset, data, length)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_offset_t data;
	vm_size_t length;
{
#ifdef	lint
	M_DATA_WRITE(mobj, kobj, offset, data, length);
#endif	lint
	return proxy_data_write(MOBJ->xmm_pager, offset, data, length);
}

m_import_lock_completed(reply, offset, length)
	xmm_reply_t reply;
	vm_offset_t offset;
	vm_size_t length;
{
	ipc_port_t reply_to = reply->reply_to;

#ifdef	lint
	M_LOCK_COMPLETED(reply, offset, length);
#endif	lint
	assert(reply->reply_to_type == MACH_MSG_TYPE_PORT_SEND_ONCE);
	xmm_reply_deallocate(reply);
	return proxy_lock_completed(reply_to, offset, length);
}

m_import_supply_completed(reply, offset, length, result, error_offset)
	xmm_reply_t reply;
	vm_offset_t offset;
	vm_size_t length;
	kern_return_t result;
	vm_offset_t error_offset;
{
	ipc_port_t reply_to = reply->reply_to;

#ifdef	lint
	M_SUPPLY_COMPLETED(reply, offset, length, result, error_offset);
#endif	lint
	assert(reply->reply_to_type == MACH_MSG_TYPE_PORT_SEND_ONCE);
	xmm_reply_deallocate(reply);
	return proxy_supply_completed(reply_to, offset, length, result,
				      error_offset);
}

m_import_data_return(mobj, kobj, offset, data, length)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_offset_t data;
	vm_size_t length;
{
#ifdef	lint
	M_DATA_RETURN(mobj, kobj, offset, data, length);
#endif	lint
	return proxy_data_return(MOBJ->xmm_pager, offset, data, length);
}

m_import_change_completed(reply, may_cache, copy_strategy)
	xmm_reply_t reply;
	boolean_t may_cache;
	memory_object_copy_strategy_t copy_strategy;
{
	ipc_port_t reply_to = reply->reply_to;

#ifdef	lint
	M_CHANGE_COMPLETED(reply, may_cache, copy_strategy);
#endif	lint
	assert(reply->reply_to_type == MACH_MSG_TYPE_PORT_SEND_ONCE);
	xmm_reply_deallocate(reply);
	return proxy_change_completed(reply_to,  may_cache, copy_strategy);
}

xmm_obj_t
convert_xmm_kernel_to_kobj(xmm_kernel)
	ipc_port_t xmm_kernel;
{
	xmm_obj_t kobj = XMM_OBJ_NULL;

	if (IP_VALID(xmm_kernel)) {
		ip_lock(xmm_kernel);
		if (ip_active(xmm_kernel) &&
		    ip_kotype(xmm_kernel) == IKOT_XMM_KERNEL) {
			kobj = (xmm_obj_t) xmm_kernel->ip_kobject;
			xmm_obj_reference(kobj);
		}
		ip_unlock(xmm_kernel);
	}
	return kobj;
}

boolean_t
xmm_kernel_notify(msg)
	mach_msg_header_t *msg;
{
	return FALSE;
}

_proxy_data_unavailable(xmm_kernel, offset, length)
	ipc_port_t	xmm_kernel;
	vm_offset_t	offset;
	vm_size_t	length;
{
	xmm_obj_t kobj;
	kern_return_t kr;

	kobj = convert_xmm_kernel_to_kobj(xmm_kernel);
	if (kobj == XMM_OBJ_NULL) {
		return KERN_FAILURE;
	}
	kr = K_DATA_UNAVAILABLE(kobj, offset, length);
	xmm_obj_release(kobj);
	return kr;
}

_proxy_get_attributes(xmm_kernel, object_ready, may_cache, copy_strategy)
	ipc_port_t	xmm_kernel;
	boolean_t	*object_ready;
	boolean_t	*may_cache;
	memory_object_copy_strategy_t *copy_strategy;
{
	xmm_obj_t kobj;
	kern_return_t kr;

	kobj = convert_xmm_kernel_to_kobj(xmm_kernel);
	if (kobj == XMM_OBJ_NULL) {
		return KERN_FAILURE;
	}
	kr = K_GET_ATTRIBUTES(kobj, object_ready, may_cache, copy_strategy);
	xmm_obj_release(kobj);
	return kr;
}

_proxy_lock_request(xmm_kernel, offset, length, should_clean, should_flush,
		    prot, reply_to)
	ipc_port_t	xmm_kernel;
	vm_offset_t	offset;
	vm_size_t	length;
	boolean_t	should_clean;
	boolean_t	should_flush;
	vm_prot_t	prot;
	ipc_port_t	reply_to;
{
	kern_return_t kr;
	xmm_reply_t reply;
	xmm_obj_t kobj;

	kobj = convert_xmm_kernel_to_kobj(xmm_kernel);
	if (kobj == XMM_OBJ_NULL) {
		/*
		 * XXX	What about reply message???
		 * XXX	Printf here for now.
		 * XXX  Same thing goes for other calls that ask for replies.
		 */
		printf("Rejecting proxy_lock_request on dead object\n");
		return KERN_FAILURE;
	}
	kr = xmm_reply_allocate_send_once(kobj, reply_to, &reply);
	if (kr != KERN_SUCCESS) {
		return kr;
	}
	kr = K_LOCK_REQUEST(kobj, offset, length, should_clean, should_flush,
			    prot, reply);
	xmm_obj_release(kobj);
	return kr;
}

_proxy_data_error(xmm_kernel, offset, length, error_value)
	ipc_port_t	xmm_kernel;
	vm_offset_t	offset;
	vm_size_t	length;
	kern_return_t	error_value;
{
	xmm_obj_t kobj;
	kern_return_t kr;

	kobj = convert_xmm_kernel_to_kobj(xmm_kernel);
	if (kobj == XMM_OBJ_NULL) {
		return KERN_FAILURE;
	}
	kr = K_DATA_ERROR(kobj, offset, length, error_value);
	xmm_obj_release(kobj);
	return kr;
}

_proxy_set_ready(xmm_kernel, xmm_pager, object_ready, may_cache, copy_strategy,
		 error_value, use_old_pageout, memory_object_name, reply_to)
	ipc_port_t	xmm_kernel;
	ipc_port_t	xmm_pager;
	boolean_t	object_ready;
	boolean_t	may_cache;
	memory_object_copy_strategy_t copy_strategy;
	kern_return_t	error_value;
	boolean_t	use_old_pageout;
	ipc_port_t	memory_object_name;
	ipc_port_t	reply_to;
{
	xmm_obj_t kobj;
	xmm_reply_t reply;
	kern_return_t kr;

	if (error_value) {
		/* destroy? or should export have done that? */
		printf("proxy_set_ready loses\n");
	}
	kobj = convert_xmm_kernel_to_kobj(xmm_kernel);
	if (kobj == XMM_OBJ_NULL) {
		return KERN_FAILURE;
	}
	KOBJ->xmm_pager = xmm_pager;
	if (KOBJ->terminated) {
		/*
		 * Now that we have xmm_pager, we can process the
		 * pending m_import_terminate.
		 *
		 * XXX what should we do with reply?
		 */
		printf("_proxy_set_ready on terminated 0x%x\n", kobj);
		kr = m_import_terminate(kobj, kobj);
		xmm_obj_release(kobj);
		return kr;
	}
	kr = xmm_reply_allocate_send_once(kobj, reply_to, &reply);
	if (kr != KERN_SUCCESS) {
		return kr;
	}
	kr = K_SET_READY(kobj, object_ready, may_cache, copy_strategy,
			 use_old_pageout, memory_object_name, reply);
	xmm_obj_release(kobj);
	return kr;
}

_proxy_destroy(xmm_kernel, reason)
	ipc_port_t	xmm_kernel;
	kern_return_t	reason;
{
	xmm_obj_t kobj;
	kern_return_t kr;

	kobj = convert_xmm_kernel_to_kobj(xmm_kernel);
	if (kobj == XMM_OBJ_NULL) {
		return KERN_FAILURE;
	}
	kr = K_DESTROY(kobj, reason);
	xmm_obj_release(kobj);
	return kr;
}

_proxy_data_supply(xmm_kernel, offset, data, length, lock_value, precious,
		   reply_to)
	ipc_port_t	xmm_kernel;
	vm_offset_t	offset;
	pointer_t	data;
	unsigned int	length;
	vm_prot_t	lock_value;
	ipc_port_t	reply_to;
{
	xmm_obj_t kobj;
	xmm_reply_t reply;
	kern_return_t kr;

	kobj = convert_xmm_kernel_to_kobj(xmm_kernel);
	if (kobj == XMM_OBJ_NULL) {
		return KERN_FAILURE;
	}
	kr = xmm_reply_allocate_send_once(kobj, reply_to, &reply);
	if (kr != KERN_SUCCESS) {
		return kr;
	}
	kr = K_DATA_SUPPLY(kobj, offset, data, length, lock_value, precious,
			   reply);
	xmm_obj_release(kobj);
	return kr;
}
