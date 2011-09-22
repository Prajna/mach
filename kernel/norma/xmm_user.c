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
 * $Log:	xmm_user.c,v $
 * Revision 2.6  92/03/10  16:38:42  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:53:39  jsb]
 * 
 * Revision 2.4.3.4  92/03/03  16:24:12  jeffreyh
 * 	Pick up fix from jsb to call K_SET_READY with MAY_CACHE_FALSE. This
 * 	fixes a memory leak.
 * 	[92/02/26  12:25:25  jeffreyh]
 * 
 * Revision 2.4.3.3  92/02/21  11:28:30  jsb
 * 	Copy send right to memory object, so that each kernel can release its
 * 	own send right to memory object.
 * 	[92/02/20  13:57:24  jsb]
 * 
 * 	Reference mobj on port to mobj conversion; release when done.
 * 	[92/02/20  10:54:28  jsb]
 * 
 * 	Removed initialized flag and corresponding code to detect multiple
 * 	initialization, since xmm_kobj_link now handles such detection.
 * 	[92/02/18  08:47:44  jsb]
 * 
 * 	Changed reply->mobj to reply->kobj.
 * 	[92/02/16  18:22:26  jsb]
 * 
 * 	Explicitly provide name parameter to xmm_decl macro.
 * 	Changes for reference counting termination protocol.
 * 	[92/02/16  15:54:47  jsb]
 * 
 * 	Removed is_internal_memory_object_call; xmm internal objects now
 * 	create their own xmm stacks and objects and thus will never be
 * 	seen here. Use new MEMORY_OBJECT_COPY_TEMPORARY strategy instead
 * 	of MEMORY_OBJECT_COPY_NONE for setting internal objects ready.
 * 	Removed ipc_kobject_set of memory_object; this was a hack for when
 * 	xmm_server.c stored a pointer to the svm mobj stack in the
 * 	memory_object kobject. We now use a separate port (the xmm object
 * 	port) for this association, and break that association elsewhere.
 * 	[92/02/11  11:22:23  jsb]
 * 
 * 	Remove vm_object_lookup_by_pager.
 * 	[92/02/10  09:41:36  jsb]
 * 
 * 	Use new xmm_decl, and new memory_object_name and deallocation protocol.
 * 	Let mig do automatic conversion of memory_control port into user obj.
 * 	Cleaned up memory_object_create support.
 * 	[92/02/09  14:01:13  jsb]
 * 
 * Revision 2.4.3.2  92/01/21  21:55:05  jsb
 * 	De-linted. Supports new (dlb) memory object routines.
 * 	Supports arbitrary reply ports to lock_request, etc.
 * 	Converted mach_port_t (and port_t) to ipc_port_t.
 * 	[92/01/20  17:48:19  jsb]
 * 
 * Revision 2.4.3.1  92/01/03  16:39:03  jsb
 * 	Picked up temporary fix to m_user_terminate from dlb.
 * 	[91/12/24  14:31:09  jsb]
 * 
 * Revision 2.4  91/08/28  11:16:30  jsb
 * 	Added definition for xxx_memory_object_lock_request, and temporary
 * 	stubs for data_supply, object_ready, and change_attributes.
 * 	[91/08/16  14:22:37  jsb]
 * 
 * 	Added check for internal memory objects to xmm_user_create.
 * 	[91/08/15  10:14:19  jsb]
 * 
 * Revision 2.3  91/07/01  08:26:46  jsb
 * 	Collect garbage. Support memory_object_create.
 * 	Disassociate kobj from memory_control before calling
 * 	memory_object_terminate to prevent upcalls on terminated kobj.
 * 	[91/06/29  15:51:50  jsb]
 * 
 * Revision 2.2  91/06/17  15:48:48  jsb
 * 	Renamed xmm_vm_object_lookup.
 * 	[91/06/17  13:20:06  jsb]
 * 
 * 	First checkin.
 * 	[91/06/17  11:02:47  jsb]
 * 
 */
/*
 *	File:	norma/xmm_user.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Interface between memory managers and xmm system.
 */

#include <norma/xmm_obj.h>
#include <norma/xmm_user_rename.h>
#include <ipc/ipc_space.h>
#include <ipc/ipc_port.h>
#include <vm/memory_object.h>
#include <vm/vm_fault.h>
#include <vm/vm_map.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <vm/vm_pageout.h>

/*
 * Since we ALWAYS have an SVM module above us,
 * we NEVER have more than one memory_control per memory_object.
 * Thus we can combine mobj and kobj.
 */

struct mobj {
	struct xmm_obj	obj;
	ipc_port_t	memory_object;
	ipc_port_t	memory_control;
	ipc_port_t	memory_object_name;
};

#undef  KOBJ
#define KOBJ    ((struct mobj *) kobj)

#define	m_user_deallocate		m_interpose_deallocate

#define	k_user_data_unavailable		k_invalid_data_unavailable
#define	k_user_get_attributes		k_invalid_get_attributes
#define	k_user_lock_request		k_invalid_lock_request
#define	k_user_data_error		k_invalid_data_error
#define	k_user_set_ready		k_invalid_set_ready
#define	k_user_destroy			k_invalid_destroy
#define	k_user_data_supply		k_invalid_data_supply

xmm_decl(user, "user", sizeof(struct mobj));

/*
 * Translate from memory_control to kobj. Take a reference.
 */
xmm_obj_t
xmm_kobj_lookup(memory_control)
	ipc_port_t memory_control;
{
	register xmm_obj_t kobj;

	if (memory_control == IP_NULL) {
		return XMM_OBJ_NULL;
	}
	ip_lock(memory_control);
	if (ip_kotype(memory_control) == IKOT_PAGING_REQUEST) {
		kobj = (xmm_obj_t) memory_control->ip_kobject;
		xmm_obj_reference(kobj);
	} else {
		kobj = XMM_OBJ_NULL;
	}
	ip_unlock(memory_control);
	return kobj;
}

/*
 * We create our own memory_control and memory_object_name ports.
 * This is easier and less confusing than each kernel allocating
 * its own ports, particularly for name ports, since everyone should
 * see the same name port for the same object.
 */
kern_return_t
xmm_user_create(memory_object, new_mobj)
	ipc_port_t memory_object;
	xmm_obj_t *new_mobj;
{
	ipc_port_t memory_control;
	ipc_port_t memory_object_name;
	kern_return_t kr;
	xmm_obj_t mobj;

	/*
	 * Allocate request port.
	 */
	memory_control = ipc_port_alloc_kernel();
	if (memory_control == IP_NULL) {
		panic("xmm_user_create: memory_control");
	}

	/*
	 * Allocate name port.
	 */
	memory_object_name = ipc_port_alloc_kernel();
	if (memory_object_name == IP_NULL) {
		panic("xmm_user_create: memory_object_name");
	}

	/*
	 * Allocate mobj.
	 */
	kr = xmm_obj_allocate(&user_class, XMM_OBJ_NULL, &mobj);
	if (kr != KERN_SUCCESS) {
		return kr;
	}

	MOBJ->memory_object = ipc_port_copy_send(memory_object);
	MOBJ->memory_control = memory_control;
	MOBJ->memory_object_name = memory_object_name;

	/*
	 * Grab a reference for mobj and associate it with memory_control port.
	 */
	xmm_obj_reference(mobj);
	ipc_kobject_set(memory_control, (ipc_kobject_t) mobj,
			IKOT_PAGING_REQUEST);
	*new_mobj = mobj;
	return KERN_SUCCESS;
}

m_user_init(mobj, k_kobj, pagesize, internal, size)
	xmm_obj_t mobj;
	xmm_obj_t k_kobj;
	vm_size_t pagesize;
	boolean_t internal;
	vm_size_t size;
{
	xmm_obj_t kobj = mobj;

#ifdef	lint
	M_INIT(mobj, k_kobj, pagesize, internal, size);
#endif	lint
	xmm_kobj_link(kobj, k_kobj);

	assert(MOBJ->memory_object != IP_NULL);
	if (internal) {
		/* acquire a naked send right for the default pager */
		ipc_port_t default_pager = memory_manager_default_reference();

		/* consumes the naked send right for default_pager */
		(void) k_memory_object_create(default_pager,
					      MOBJ->memory_object, size,
					      MOBJ->memory_control,
					      MOBJ->memory_object_name,
					      PAGE_SIZE);

		/* call set_ready, since default pager won't */
		return K_SET_READY(mobj, OBJECT_READY_TRUE, MAY_CACHE_FALSE,
				   MEMORY_OBJECT_COPY_TEMPORARY,
				   USE_OLD_PAGEOUT_TRUE,
				   ipc_port_make_send(MOBJ->
						      memory_object_name),
				   XMM_REPLY_NULL);
	} else {
		(void) memory_object_init(MOBJ->memory_object,
					  MOBJ->memory_control,
					  MOBJ->memory_object_name,
					  PAGE_SIZE);
	}
	return KERN_SUCCESS;
}

m_user_terminate(mobj, kobj)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
{
	kern_return_t kr;

#ifdef	lint
	M_TERMINATE(mobj, kobj);
#endif	lint
	ipc_kobject_set(MOBJ->memory_control, IKO_NULL, IKOT_NONE);
	kr = memory_object_terminate(MOBJ->memory_object,
				     MOBJ->memory_control,
				     MOBJ->memory_object_name);
	xmm_obj_release(mobj);
	return kr;
}

m_user_copy(mobj, kobj, offset, length, new_mobj)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	xmm_obj_t new_mobj;
{
#ifdef	lint
	M_COPY(mobj, kobj, offset, length, new_mobj);
#endif	lint
	panic("m_user_copy");
	/* NOTREACHED */
}

m_user_data_request(mobj, kobj, offset, length, desired_access)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
#ifdef	lint
	M_DATA_REQUEST(mobj, kobj, offset, length, desired_access);
#endif	lint
	return memory_object_data_request(MOBJ->memory_object,
					  KOBJ->memory_control,
					  offset,
					  length,
					  desired_access);
}

m_user_data_unlock(mobj, kobj, offset, length, desired_access)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
#ifdef	lint
	M_DATA_UNLOCK(mobj, kobj, offset, length, desired_access);
#endif	lint
	return memory_object_data_unlock(MOBJ->memory_object,
					 KOBJ->memory_control,
					 offset,
					 length,
					 desired_access);
}

m_user_data_write(mobj, kobj, offset, data, length)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_offset_t data;
	vm_size_t length;
{
#ifdef	lint
	M_DATA_WRITE(mobj, kobj, offset, data, length);
#endif	lint
	return memory_object_data_write(MOBJ->memory_object,
					KOBJ->memory_control,
					offset,
					data,
					length);
}

m_user_lock_completed(reply, offset, length)
	xmm_reply_t reply;
	vm_offset_t offset;
	vm_size_t length;
{
	xmm_obj_t kobj;
	kern_return_t kr;

#ifdef	lint
	M_LOCK_COMPLETED(reply, offset, length);
#endif	lint
	kobj = reply->kobj;
	assert(kobj->class == &user_class);
	kr = memory_object_lock_completed(reply->reply_to,
					  reply->reply_to_type,
					  KOBJ->memory_control,
					  offset,
					  length);
	xmm_reply_deallocate(reply);
	return kr;
}

m_user_supply_completed(reply, offset, length, result, error_offset)
	xmm_reply_t reply;
	vm_offset_t offset;
	vm_size_t length;
	kern_return_t result;
	vm_offset_t error_offset;
{
	xmm_obj_t kobj;
	kern_return_t kr;

#ifdef	lint
	M_SUPPLY_COMPLETED(reply, offset, length, result, error_offset);
#endif	lint
	kobj = reply->kobj;
	assert(kobj->class == &user_class);
	kr = memory_object_supply_completed(reply->reply_to,
					    reply->reply_to_type,
					    KOBJ->memory_control,
					    offset,
					    length,
					    result,
					    error_offset);
	xmm_reply_deallocate(reply);
	return kr;
}

m_user_data_return(mobj, kobj, offset, data, length)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_offset_t data;
	vm_size_t length;
{
#ifdef	lint
	M_DATA_RETURN(mobj, kobj, offset, data, length);
#endif	lint
	return memory_object_data_return(MOBJ->memory_object,
					 KOBJ->memory_control,
					 offset,
					 data,
					 length);
}

m_user_change_completed(reply, may_cache, copy_strategy)
	xmm_reply_t reply;
	boolean_t may_cache;
	memory_object_copy_strategy_t copy_strategy;
{
	kern_return_t kr;

#ifdef	lint
	M_CHANGE_COMPLETED(reply, may_cache, copy_strategy);
#endif	lint
	kr = memory_object_change_completed(reply->reply_to,
					    reply->reply_to_type,
					    may_cache,
					    copy_strategy);
	xmm_reply_deallocate(reply);
	return kr;
}

kern_return_t
memory_object_data_provided(kobj, offset, data, length, lock_value)
	xmm_obj_t	kobj;
	vm_offset_t	offset;
	pointer_t	data;
	unsigned int	length;
	vm_prot_t	lock_value;
{
	kern_return_t kr;

	if (kobj == XMM_OBJ_NULL) {
		return KERN_INVALID_ARGUMENT;
	}
	kr = K_DATA_SUPPLY(kobj, offset, data, length, lock_value,
			   PRECIOUS_FALSE, XMM_REPLY_NULL);
	xmm_obj_release(kobj);
	return kr;
}

kern_return_t
memory_object_data_unavailable(kobj, offset, length)
	xmm_obj_t	kobj;
	vm_offset_t	offset;
	vm_size_t	length;
{
	kern_return_t kr;

	if (kobj == XMM_OBJ_NULL) {
		return KERN_INVALID_ARGUMENT;
	}
	kr = K_DATA_UNAVAILABLE(kobj, offset, length);
	xmm_obj_release(kobj);
	return kr;
}

kern_return_t
memory_object_get_attributes(kobj, object_ready, may_cache, copy_strategy)
	xmm_obj_t	kobj;
	boolean_t	*object_ready;
	boolean_t	*may_cache;
	memory_object_copy_strategy_t *copy_strategy;
{
	kern_return_t kr;

	if (kobj == XMM_OBJ_NULL) {
		return KERN_INVALID_ARGUMENT;
	}
	kr = K_GET_ATTRIBUTES(kobj, object_ready, may_cache, copy_strategy);
	xmm_obj_release(kobj);
	return kr;
}

kern_return_t
memory_object_lock_request(kobj, offset, length, should_return, should_flush,
			   prot, reply_to, reply_to_type)
	xmm_obj_t	kobj;
	vm_offset_t	offset;
	vm_size_t	length;
	int		should_return;
	boolean_t	should_flush;
	vm_prot_t	prot;
	ipc_port_t	reply_to;
	mach_msg_type_name_t reply_to_type;
{
	xmm_reply_t reply;
	kern_return_t kr;
	
	if (kobj == XMM_OBJ_NULL) {
		return KERN_INVALID_ARGUMENT;
	}
	kr = xmm_reply_allocate(kobj, reply_to, reply_to_type, &reply);
	if (kr != KERN_SUCCESS) {
		return kr;
	}
	kr = K_LOCK_REQUEST(kobj, offset, length, should_return, should_flush,
			    prot, reply);
	xmm_obj_release(kobj);
	return kr;
}

kern_return_t
xxx_memory_object_lock_request(kobj, offset, size, should_clean, should_flush,
			       prot, reply_to, reply_to_type)
	xmm_obj_t	kobj;
	vm_offset_t	offset;
	vm_size_t	size;
	boolean_t	should_clean;
	boolean_t	should_flush;
	vm_prot_t	prot;
	ipc_port_t	reply_to;
	mach_msg_type_name_t reply_to_type;
{
	int should_return;

	if (should_clean) {
		should_return = MEMORY_OBJECT_RETURN_DIRTY;
	} else {
		should_return = MEMORY_OBJECT_RETURN_NONE;
	}
	return memory_object_lock_request(kobj, offset, size, should_return,
					  should_flush, prot, reply_to,
					  reply_to_type);
}

kern_return_t
memory_object_data_error(kobj, offset, length, error_value)
	xmm_obj_t	kobj;
	vm_offset_t	offset;
	vm_size_t	length;
	kern_return_t	error_value;
{
	kern_return_t kr;

	if (kobj == XMM_OBJ_NULL) {
		return KERN_INVALID_ARGUMENT;
	}
	kr = K_DATA_ERROR(kobj, offset, length, error_value);
	xmm_obj_release(kobj);
	return kr;
}

kern_return_t
memory_object_set_attributes(kobj, object_ready, may_cache, copy_strategy)
	xmm_obj_t	kobj;
	boolean_t	object_ready;
	boolean_t	may_cache;
	memory_object_copy_strategy_t copy_strategy;
{
	kern_return_t kr;

	if (kobj == XMM_OBJ_NULL) {
		return KERN_INVALID_ARGUMENT;
	}
	kr = K_SET_READY(kobj, object_ready, may_cache, copy_strategy,
			 USE_OLD_PAGEOUT_TRUE,
			 ipc_port_make_send(KOBJ->memory_object_name),
			 XMM_REPLY_NULL);
	xmm_obj_release(kobj);
	return kr;
}

kern_return_t
memory_object_destroy(kobj, reason)
	xmm_obj_t	kobj;
	kern_return_t	reason;
{
	kern_return_t kr;

	if (kobj == XMM_OBJ_NULL) {
		return KERN_INVALID_ARGUMENT;
	}
	kr = K_DESTROY(kobj, reason);
	xmm_obj_release(kobj);
	return kr;
}

kern_return_t
memory_object_data_supply(kobj, offset, data, length, lock_value, precious,
			  reply_to, reply_to_type)
	xmm_obj_t	kobj;
	vm_offset_t	offset;
	pointer_t	data;
	unsigned int	length;
	vm_prot_t	lock_value;
	boolean_t	precious;
	ipc_port_t	reply_to;
	mach_msg_type_name_t reply_to_type;
{
	xmm_reply_t reply;
	kern_return_t kr;
	
	if (kobj == XMM_OBJ_NULL) {
		return KERN_INVALID_ARGUMENT;
	}
	kr = xmm_reply_allocate(kobj, reply_to, reply_to_type, &reply);
	if (kr != KERN_SUCCESS) {
		return kr;
	}
	kr = K_DATA_SUPPLY(kobj, offset, data, length, lock_value, precious,
			   reply);
	xmm_obj_release(kobj);
	return kr;
}

kern_return_t
memory_object_ready(kobj, may_cache, copy_strategy)
	xmm_obj_t	kobj;
	boolean_t	may_cache;
	memory_object_copy_strategy_t copy_strategy;
{
	kern_return_t kr;

	if (kobj == XMM_OBJ_NULL) {
		return KERN_INVALID_ARGUMENT;
	}
	kr = K_SET_READY(kobj, OBJECT_READY_TRUE, may_cache, copy_strategy,
			 USE_OLD_PAGEOUT_FALSE,
			 ipc_port_make_send(KOBJ->memory_object_name),
			 XMM_REPLY_NULL);
	xmm_obj_release(kobj);
	return kr;
}

kern_return_t
memory_object_change_attributes(kobj, may_cache, copy_strategy, reply_to,
				reply_to_type)
	xmm_obj_t	kobj;
	boolean_t	may_cache;
	memory_object_copy_strategy_t copy_strategy;
	ipc_port_t	reply_to;
	mach_msg_type_name_t reply_to_type;
{
	xmm_reply_t reply;
	kern_return_t kr;

	if (kobj == XMM_OBJ_NULL) {
		return KERN_INVALID_ARGUMENT;
	}
	kr = xmm_reply_allocate(kobj, reply_to, reply_to_type, &reply);
	if (kr != KERN_SUCCESS) {
		return kr;
	}
	kr = K_SET_READY(kobj, OBJECT_READY_TRUE, may_cache, copy_strategy,
			 USE_OLD_PAGEOUT_FALSE,
			 ipc_port_make_send(KOBJ->memory_object_name), reply);
	xmm_obj_release(kobj);
	return kr;
}
