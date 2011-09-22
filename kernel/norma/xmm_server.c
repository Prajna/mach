/* 
 * Mach Operating System
 * Copyright (c) 1991, 1992 Carnegie Mellon University
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
 * $Log:	xmm_server.c,v $
 * Revision 2.6  92/03/10  16:29:35  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:51:42  jsb]
 * 
 * Revision 2.5.1.4  92/03/03  16:24:06  jeffreyh
 * 	Pick up fix from dlb to add missing vm_object_dealocate to the
 * 	 object->internal case of k_server_set_ready().
 * 	[92/02/29            jeffreyh]
 * 
 * Revision 2.5.1.3  92/02/21  11:28:01  jsb
 * 	Release send right to memory object in memory_object_terminate, now
 * 	that the xmm_user layer keeps a separate send right.
 * 	[92/02/20  14:02:57  jsb]
 * 
 * 	Explicitly provide name parameter to xmm_decl macro.
 * 	Changed termination for new reference counting implementation.
 * 	[92/02/16  15:53:26  jsb]
 * 
 * 	In memory_object_terminate, don't release_send memory_object_name
 * 	if it is null. Do call xmm_object_by_memory_object_release.
 * 	[92/02/11  18:23:15  jsb]
 * 
 * 	Changed xmm_memory_object_init to use xmm_object_by_memory_object
 * 	instead of xmm_lookup. Removed xmm_lookup.
 * 	[92/02/10  17:02:39  jsb]
 * 
 * 	Instead of holding a vm_object pointer, always do a vm_object_lookup
 * 	on pager to obtain vm_object. This allows us to notice when vm_object.c
 * 	has removed (as in vm_object_terminate) or changed (vm_object_collapse)
 * 	the port to object associations. Removed now unneeded xmm_object_set.
 * 	Declare second parameter of memory_object_* calls as xmm_obj_t, thanks
 * 	to new pager_request_t declaration of object->pager_request.
 * 	[92/02/10  09:47:16  jsb]
 * 
 * 	Use new xmm_decl, and new memory_object_name and deallocation protocol.
 * 	Removed svm exceptions; this is now handled by xmm_vm_object_lookup.
 * 	Changed xmm_lookup to not use memory_object kobject to hold
 * 	both mobj and vm_object; we now use memory_object->ip_norma_xmm_object
 * 	which is migrated upon memory_object port migration.
 * 	Don't defined memory_object_{init,create}; instead, vm/vm_object.c
 * 	calls new routine xmm_memory_object_init routine which passes
 * 	internal and size parameters down the xmm layers.
 * 	[92/02/09  13:54:41  jsb]
 * 
 * Revision 2.5.1.2  92/01/21  21:54:46  jsb
 * 	De-linted. Supports new (dlb) memory object routines.
 * 	Supports arbitrary reply ports to lock_request, etc.
 * 	Converted mach_port_t (and port_t) to ipc_port_t.
 * 	[92/01/20  17:28:58  jsb]
 * 
 * Revision 2.5.1.1  92/01/03  17:13:19  jsb
 * 	MACH_PORT_NULL -> IP_NULL.
 * 
 * Revision 2.5  91/08/28  11:16:24  jsb
 * 	Added temporary definition for memory_object_change_completed.
 * 	[91/08/16  14:21:20  jsb]
 * 
 * 	Added comment to xmm_lookup about read-only pagers.
 * 	[91/08/15  10:12:12  jsb]
 * 
 * Revision 2.4  91/08/03  18:19:40  jsb
 * 	Changed mach_port_t to ipc_port_t whereever appropriate.
 * 	[91/07/17  14:07:08  jsb]
 * 
 * Revision 2.3  91/07/01  08:26:29  jsb
 * 	Added support for memory_object_create.
 * 	Now export normal memory_object_init with standard arguments.
 * 	Improved object initialization logic.
 * 	Added garbage collection.
 * 	[91/06/29  15:39:01  jsb]
 * 
 * Revision 2.2  91/06/17  15:48:33  jsb
 * 	First checkin.
 * 	[91/06/17  11:05:10  jsb]
 * 
 */
/*
 *	File:	norma/xmm_server.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Interface between kernel and xmm system.
 */

#include <norma/xmm_obj.h>
#include <norma/xmm_server_rename.h>
#include <ipc/ipc_space.h>
#include <ipc/ipc_port.h>
#include <vm/memory_object.h>
#include <vm/vm_fault.h>
#include <vm/vm_map.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <vm/vm_pageout.h>
#include <kern/host.h>
#include <kern/ipc_kobject.h>

struct mobj {
	struct xmm_obj	obj;
	ipc_port_t	pager;
};

#undef  KOBJ
#define KOBJ    ((struct mobj *) kobj)

#define	m_server_init			m_invalid_init
#define	m_server_terminate		m_invalid_terminate
#define	m_server_copy			m_invalid_copy
#define	m_server_data_request		m_invalid_data_request
#define	m_server_data_unlock		m_invalid_data_unlock
#define	m_server_data_write		m_invalid_data_write
#define	m_server_lock_completed		m_invalid_lock_completed
#define	m_server_supply_completed	m_invalid_supply_completed
#define	m_server_data_return		m_invalid_data_return
#define	m_server_change_completed	m_invalid_change_completed

xmm_decl(server, "server", sizeof(struct mobj));

extern ipc_port_t xmm_object_by_memory_object();

k_server_data_unavailable(kobj, offset, length)
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
{
#ifdef	lint
	K_DATA_UNAVAILABLE(kobj, offset, length);
#endif	lint
	return memory_object_data_unavailable(vm_object_lookup(KOBJ->pager),
					      offset, length);
}

k_server_get_attributes(kobj, object_ready, may_cache, copy_strategy)
	xmm_obj_t kobj;
	boolean_t *object_ready;
	boolean_t *may_cache;
	memory_object_copy_strategy_t *copy_strategy;
{
#ifdef	lint
	K_GET_ATTRIBUTES(kobj, object_ready, may_cache, copy_strategy);
#endif	lint
	return memory_object_get_attributes(vm_object_lookup(KOBJ->pager),
					    object_ready, may_cache,
					    copy_strategy);
}

k_server_lock_request(kobj, offset, length, should_clean, should_flush,
		      lock_value, reply)
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	boolean_t should_clean;
	boolean_t should_flush;
	vm_prot_t lock_value;
	xmm_reply_t reply;
{
#ifdef	lint
	K_LOCK_REQUEST(kobj, offset, length, should_clean, should_flush,
		       lock_value, reply);
#endif	lint
	return memory_object_lock_request(vm_object_lookup(KOBJ->pager),
					  offset, length, should_clean,
					  should_flush, lock_value,
					  (ipc_port_t) reply,
					  MACH_MSG_TYPE_PORT_SEND_ONCE);
}

k_server_data_error(kobj, offset, length, error_value)
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	kern_return_t error_value;
{
#ifdef	lint
	K_DATA_ERROR(kobj, offset, length, error_value);
#endif	lint
	return memory_object_data_error(vm_object_lookup(KOBJ->pager),
					offset, length, error_value);
}

k_server_set_ready(kobj, object_ready, may_cache, copy_strategy,
		   use_old_pageout, memory_object_name, reply)
	xmm_obj_t kobj;
	boolean_t object_ready;
	boolean_t may_cache;
	memory_object_copy_strategy_t copy_strategy;
	boolean_t use_old_pageout;
	ipc_port_t memory_object_name;
	xmm_reply_t reply;
{
	vm_object_t object;
	kern_return_t kr;

#ifdef	lint
	K_SET_READY(kobj, object_ready, may_cache, copy_strategy,
		    use_old_pageout, memory_object_name, reply);
#endif	lint

	/*
	 * Remember pager_name. Only keep one send right for it.
	 */
	object = vm_object_lookup(KOBJ->pager);
	vm_object_lock(object);
	if (object->pager_name == IP_NULL) {
		object->pager_name = memory_object_name;
	} else {
		assert(object->pager_name == memory_object_name);
		ipc_port_release_send(memory_object_name);
	}

	/*
	 * If we are internal, we don't need to call set_attributes_common.
	 */
	if (object->internal) {
		assert(object->pager_ready);
		assert(reply == XMM_REPLY_NULL);
		vm_object_unlock(object);
		vm_object_deallocate(object);
		return KERN_SUCCESS;
	}

	/*
	 * Call set_attributes_common.
	 */
	vm_object_unlock(object);
	kr = memory_object_set_attributes_common(object, object_ready,
						 may_cache, copy_strategy,
						 use_old_pageout);

	/*
	 * Send a reply if one was requested.
	 */
	if (reply != XMM_REPLY_NULL) {
		M_CHANGE_COMPLETED(reply, may_cache, copy_strategy);
	}
	return kr;
}

k_server_destroy(kobj, reason)
	xmm_obj_t kobj;
	kern_return_t reason;
{
#ifdef	lint
	K_DESTROY(kobj, reason);
#endif	lint
	return memory_object_destroy(vm_object_lookup(KOBJ->pager), reason);
}

k_server_data_supply(kobj, offset, data, length, lock_value, precious, reply)
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_offset_t data;
	vm_size_t length;
	vm_prot_t lock_value;
	boolean_t precious;
	xmm_reply_t reply;
{
#ifdef	lint
	K_DATA_SUPPLY(kobj, offset, data, length, lock_value, precious, reply);
#endif	lint
	return memory_object_data_supply(vm_object_lookup(KOBJ->pager),
					 offset, (vm_map_copy_t) data,
					 length, lock_value, precious,
					 (ipc_port_t) reply,
					 MACH_MSG_TYPE_PORT_SEND_ONCE);
}

xmm_memory_object_init(object)
	vm_object_t object;
{
	ipc_port_t xmm_object;
	xmm_obj_t mobj;
	kern_return_t kr;

	/*
	 * Find or create xmm_object corresponding to memory_object.
	 * Once created, the xmm_object for a memory_object remains
	 * the same until the memory_object port dies.
	 *
	 * XXX
	 * This isn't right -- what about memory_object_destroy()?
	 *
	 * Maybe at that point the xmm_object port is destroyed.
	 *
	 * XXX
	 * Check for multiple inits? Or is this handled well enough
	 * by vm_object_enter. A few asserts might be worthwhile...
	 */
	xmm_object = xmm_object_by_memory_object(object->pager);
	assert(xmm_object != IP_NULL);	/* XXX */

	/*
	 * If xmm_object is local, then so is the svm stack, which will
	 * be stored as xmm_object's kobject. Otherwise, we need to
	 * create an import mobj.
	 */
	if (IP_NORMA_IS_PROXY(xmm_object)) {
		kr = xmm_import_create(xmm_object, &mobj);
		if (kr != KERN_SUCCESS) {
			panic("xmm_memory_object_init: xmm_import_create");
			return kr;
		}
	} else {
		assert(ip_kotype(xmm_object) == IKOT_XMM_OBJECT);
		mobj = (xmm_obj_t) xmm_object->ip_kobject;
		ipc_port_release_send(xmm_object);
	}

	/*
	 * Create a server layer on top.
	 */
	kr = xmm_obj_allocate(&server_class, mobj, &mobj);
	if (kr != KERN_SUCCESS) {
		printf("xmm_memory_object_init: xmm_obj_allocate: %x\n", kr);
		return kr;
	}

	/*
	 * Associate server mobj with vm object, and pager with mobj.
	 *
	 * The reason that we don't store the vm object directly in the
	 * mobj is that the vm object can change, for example as a result
	 * of vm_object_collapse.
	 */
	MOBJ->pager = object->pager;
	object->pager_request = mobj;

	/*
	 * Intialize the mobj for this kernel.
	 */
	M_INIT(mobj, mobj, PAGE_SIZE, (boolean_t) object->internal,
	       object->size);
	return KERN_SUCCESS;
}

memory_object_terminate(memory_object, mobj, memory_object_name)
	ipc_port_t memory_object;
	xmm_obj_t mobj;
	ipc_port_t memory_object_name;
{
	if (memory_object != IP_NULL) {
		ipc_port_release_send(memory_object);
	}
	if (memory_object_name != IP_NULL) {
		ipc_port_release_send(memory_object_name);
	}
	xmm_obj_release(mobj);
	return M_TERMINATE(mobj, mobj);
}

void
m_server_deallocate(mobj)
	xmm_obj_t mobj;
{
#if 666
	/*
	 * XXX should release pager --
	 * but need to coordinate with xmm_user.c
	 */
#else
	ipc_port_release_send(MOBJ->pager);
#endif
}

/* ARGSUSED */
memory_object_copy(memory_object, mobj, offset, length, new_memory_object)
	ipc_port_t memory_object;
	xmm_obj_t mobj;
	vm_offset_t offset;
	vm_size_t length;
	ipc_port_t new_memory_object;
{
	panic("xmm_server: memory_object_copy\n");
	return KERN_FAILURE;
}

memory_object_data_request(memory_object, mobj, offset, length, desired_access)
	ipc_port_t memory_object;
	xmm_obj_t mobj;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
#ifdef	lint
	memory_object++;
#endif	lint
	return M_DATA_REQUEST(mobj, mobj, offset, length, desired_access);
}

memory_object_data_unlock(memory_object, mobj, offset, length, desired_access)
	ipc_port_t memory_object;
	xmm_obj_t mobj;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
#ifdef	lint
	memory_object++;
#endif	lint
	return M_DATA_UNLOCK(mobj, mobj, offset, length, desired_access);
}

memory_object_data_write(memory_object, mobj, offset, data, length)
	ipc_port_t memory_object;
	xmm_obj_t mobj;
	vm_offset_t offset;
	vm_offset_t data;
	vm_size_t length;
{
#ifdef	lint
	memory_object++;
#endif	lint
	return M_DATA_WRITE(mobj, mobj, offset, data, length);
}

#ifdef	KERNEL
/* ARGSUSED */
memory_object_data_initialize(memory_object, mobj, offset, data, length)
	ipc_port_t memory_object;
	xmm_obj_t mobj;
	vm_offset_t offset;
	vm_offset_t data;
	vm_size_t length;
{
	/*
	 * Probably need to add M_DATA_INITIALIZE, or perhaps
	 * an 'initial' parameter to memory_object_data_write.
	 */
	panic("memory_object_data_initialize");
}
#endif	KERNEL

memory_object_lock_completed(reply_to, reply_to_type, mobj, offset, length)
	ipc_port_t reply_to;
	mach_msg_type_name_t reply_to_type;
	xmm_obj_t mobj;
	vm_offset_t offset;
	vm_size_t length;
{
	xmm_reply_t reply = (xmm_reply_t) reply_to;

#ifdef	lint
	mobj++;
#endif
	assert(reply != XMM_REPLY_NULL);
	assert(reply_to_type == MACH_MSG_TYPE_PORT_SEND_ONCE);
	return M_LOCK_COMPLETED(reply, offset, length);
}

memory_object_supply_completed(reply_to, reply_to_type, mobj, offset, length,
			       result, error_offset)
	ipc_port_t reply_to;
	mach_msg_type_name_t reply_to_type;
	xmm_obj_t mobj;
	vm_offset_t offset;
	vm_size_t length;
	kern_return_t result;
	vm_offset_t error_offset;
{
	xmm_reply_t reply = (xmm_reply_t) reply_to;

#ifdef	lint
	mobj++;
#endif
	assert(reply != XMM_REPLY_NULL);
	assert(reply_to_type == MACH_MSG_TYPE_PORT_SEND_ONCE);
	return M_SUPPLY_COMPLETED(reply, offset, length, result, error_offset);
}

memory_object_data_return(memory_object, mobj, offset, data, length)
	ipc_port_t memory_object;
	xmm_obj_t mobj;
	vm_offset_t offset;
	vm_offset_t data;
	vm_size_t length;
{
#ifdef	lint
	memory_object++;
#endif	lint
	return M_DATA_RETURN(mobj, mobj, offset, data, length);
}
