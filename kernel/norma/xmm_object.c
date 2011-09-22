/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
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
 * $Log:	xmm_object.c,v $
 * Revision 2.2  92/03/10  16:29:28  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:51:37  jsb]
 * 
 * Revision 2.1.2.1  92/02/21  11:27:52  jsb
 * 	Use xmm_object_destroy in xmm_object_notify to release mobj and
 * 	deallocate xmm object port. Added hack to get rid of send-once
 * 	right to xmm object port before we deallocate the port, since
 * 	ipc_port_release_sonce won't do so after we deallocate it.
 * 	[92/02/20  14:00:36  jsb]
 * 
 * 	Fixed reference counting on xmm objs. A reference is now held by
 * 	xmm object, which is copied along with send right to xmm object.
 * 	[92/02/18  17:15:33  jsb]
 * 
 * 	Lots of changes. First reasonably working version.
 * 	[92/02/16  15:58:03  jsb]
 * 
 * 	Added missing line to xmm_memory_manager_export.
 * 	[92/02/12  05:58:07  jsb]
 * 
 * 	Added xmm_object_allocate routine to replace replicated xmm object
 * 	creation and initialization logic.
 * 	Added xmm_object_by_memory_object_release which disassociates
 * 	xmm object from memory object, possibly resulting in the deallocation
 * 	of mobj associated with xmm object (via no-senders).
 * 	Moved all responsibility for tearing down stack in case of xmm
 * 	object creation race to no-senders notification handler.
 * 	[92/02/11  18:38:36  jsb]
 * 
 * 	Updated explanatory text. Fixed send right management. Added
 * 	xmm_memory_manager_export routine for xmm-internal memory managers.
 * 	[92/02/11  11:24:42  jsb]
 * 
 * 	Added xmm_object_notify.
 * 	[92/02/10  17:27:44  jsb]
 * 
 * 	First checkin.
 * 	[92/02/10  17:05:02  jsb]
 * 
 */
/*
 *	File:	norma/xmm_object.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Routines to manage xmm object to memory object association.
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
#include <mach/notify.h>

extern void xmm_svm_destroy();

/*
 * The structure here is:
 *
 *	memory object		-> xmm object		[ send right ]
 *	xmm object		-> top of mobj stack	[ xmm obj ref ]
 *	bottom of mobj stack	-> memory object	[ send right ]
 *
 * The xmm object and mobj stack are colocated. They are originally created
 * on the same node as the memory object, so that we can atomically set
 * memory_object->ip_norma_xmm_object on principal port for memory object,
 * which is the central synchronizing point for creating and finding an
 * mobj stack for a memory object.
 * 
 * After the stack is created, the memory object may migrated away from
 * the stack. The port migration mechanism is responsible for maintaining
 * the association between the memory object port and the xmm object port.
 * (In the current implementation, this means moving the send right to
 * xmm object and setting it as ip_norma_xmm_object in the new principal
 * for memory object.)
 *
 * This doesn't seem right... of course the real basic problem is
 * we are designing around a couple unclean things:
 *
 *	1. pointers from ports to objects
 *	2. transparent port interposition
 *
 * So I guess it's natural that if an object moves, and we've associated
 * data with that object, and that object doesn't know about it, then
 * we have to migrate that data ourselves. I guess a netmsgserver could
 * export a kobject (port data) interface, in which case it would know
 * to migrate the data when the port was migrated.
 *
 * Right now the policy is to create the layer at the current home of the
 * memory object. We could create it at the home of the first mapper.
 * This might make sense if we expect to often not need to talk to the
 * pager from the svm layer, for example in shadowing cases.
 * We might even want to migrate the layer. There's a lot of flexibility
 * here now that memory object has a port pointing to the svm layer.
 *
 * We could get rid of ip_norma_xmm_object and replace it with a hash table
 * (and a set of routines to manipulate it). The advantage would be the
 * space savings of eliminating the field, which for most ports will be
 * unused. Note that port migration must still migrate xmm object assocication.
 */

xmm_object_set(memory_object, xmm_object, make_copy)
	ipc_port_t memory_object;
	ipc_port_t xmm_object;
	boolean_t make_copy;
{
	assert(! IP_NORMA_IS_PROXY(xmm_object));
	memory_object->ip_norma_xmm_object = ipc_port_make_send(xmm_object);
	if (make_copy) {
		assert(ip_kotype(xmm_object) == IKOT_XMM_OBJECT);
		xmm_obj_reference((xmm_obj_t) xmm_object->ip_kobject);
		memory_object->ip_norma_xmm_object_refs = 1;
		ipc_port_copy_send(xmm_object);
	} else {
		memory_object->ip_norma_xmm_object_refs = 0;
	}
}

ipc_port_t
xmm_object_copy(memory_object)
	ipc_port_t memory_object;
{
	register ipc_port_t xmm_object;

	assert(! IP_NORMA_IS_PROXY(memory_object));
	xmm_object = memory_object->ip_norma_xmm_object;
	if (xmm_object == IP_NULL) {
		return IP_NULL;
	}
	assert(ip_kotype(xmm_object) == IKOT_XMM_OBJECT);
	xmm_obj_reference((xmm_obj_t) xmm_object->ip_kobject);
	memory_object->ip_norma_xmm_object_refs++;
	return ipc_port_copy_send(xmm_object);
}

void
xmm_object_release_local(memory_object)
	ipc_port_t memory_object;
{
	assert(! IP_NORMA_IS_PROXY(memory_object));
	if (--memory_object->ip_norma_xmm_object_refs == 0) {
		/*
		 * We use no-senders because it's snazzier, but we could
		 * use a call that did a move_send instead. The receiver
		 * would then deallocate the send and the no-senders
		 * notification would be done locally (if at all).
		 * Using no-senders might help deal with node failure
		 */
		ipc_port_release_send(memory_object->ip_norma_xmm_object);
		memory_object->ip_norma_xmm_object = IP_NULL;
	}
}

/*
 * Only called internally.
 * Allocate an xmm_object port with a no-senders notification request.
 * The xmm_object takes the mobj reference.
 */
ipc_port_t
xmm_object_allocate(mobj)
	xmm_obj_t mobj;
{
	ipc_port_t xmm_object;
	ipc_port_t old_nsrequest;

	/*
	 * Create an xmm object port.
	 */
	xmm_object = ipc_port_alloc_kernel();
	if (xmm_object == IP_NULL) {
		return IP_NULL;
	}

	/*
	 * Associate the xmm obj with the xmm object port.
	 * We keep the xmm obj reference returned by xmm_svm_create.
	 */
	ipc_kobject_set(xmm_object, (ipc_kobject_t) mobj, IKOT_XMM_OBJECT);

	/*
	 * Request a no-senders notification.
	 */
	ipc_port_nsrequest(xmm_object, 1, ipc_port_make_sonce(xmm_object),
			   &old_nsrequest);
	assert(old_nsrequest == IP_NULL);

	/*
	 * Return the port.
	 */
	return xmm_object;
}

/*
 * Called when we lose a race to associate a newly created xmm object
 * with a memory object. Also called by xmm_object_notify.
 */
xmm_object_destroy(xmm_object, mobj)
	ipc_port_t xmm_object;
	xmm_obj_t mobj;
{
#if 666
	/*
	 * XXX
	 * A temporary fix. ipc_port_release_sonce won't decr sorights
	 * on a dead port, and ipc_kobject.c calls ipc_port_release_sonce
	 * after we've killed it, and norma_port_remove won't happen
	 * until send-once rights drop to 0. The correct fix is probably
	 * to change ipc_port_release_sonce.
	 */
	assert(xmm_object->ip_sorights > 0);
	xmm_object->ip_sorights--;
#endif

	/*
	 * Destroy xmm object port (and its no-senders notification request).
	 */
	ipc_port_dealloc_kernel(xmm_object);

	/*
	 * Lose reference to mobj, and explicitly destroy it.
	 */
	xmm_obj_release(mobj);
	xmm_svm_destroy(mobj);
}

/*
 * Handle notifications. We only care about no-senders notifications.
 */
boolean_t
xmm_object_notify(msg)
	mach_msg_header_t *msg;
{
	ipc_port_t xmm_object;
	xmm_obj_t mobj;

	/*
	 * Only process no-senders notifications.
	 */
	if (msg->msgh_id != MACH_NOTIFY_NO_SENDERS) {
		printf("xmm_object_notify: strange notification %d\n",
		       msg->msgh_id);
		return FALSE;
	}

	/*
	 * Extract xmm_object port from notification message.
	 */
	xmm_object = (ipc_port_t) msg->msgh_remote_port;

	/*
	 * Get and disassociate mobj from xmm object port.
	 */
	assert(ip_kotype(xmm_object) == IKOT_XMM_OBJECT);
	mobj = (xmm_obj_t) xmm_object->ip_kobject;
	ipc_kobject_set(xmm_object, IKO_NULL, IKOT_NONE);

	/*
	 * Destroy xmm object port and mobj.
	 */
	xmm_object_destroy(xmm_object, mobj);
	return TRUE;
}

/*
 * Called with memory_object locked. Unlocks memory_object.
 */
ipc_port_t
xmm_object_by_memory_object_remote(memory_object)
	ipc_port_t memory_object;
{
	unsigned long node;
	kern_return_t kr;
	ipc_port_t xmm_object;
	
	node = ipc_port_node(memory_object);
	assert(node != node_self());
	ip_unlock(memory_object);
	kr = r_norma_xmm_object_by_memory_object(remote_host_priv(node),
						 memory_object,
						 &xmm_object);
	assert(kr == KERN_SUCCESS);
	return xmm_object;
}

/*
 * Return send right for xmm object corresponding to memory object.
 * This is to be consumed when using xmm object to set up init,
 * either via move_send dest in proxy_init, or explicit deallocation
 * in local case.
 * Also returns one xmm_object ref, to be given to svm layer and
 * released there upon termination via xmm_object_release.
 * Also returns one xmm obj ref, to be consumed by xmm_obj_allocate
 * in either _proxy_init or xmm_memory_object_init.
 *
 * Create xmm object if necessary.
 * Memory object holds a send right to xmm object as well, which is released
 * when xmm object refs drop to 0. No-senders then triggers
 * svm deallocation.
 */
ipc_port_t
xmm_object_by_memory_object(memory_object)
	ipc_port_t memory_object;
{
	ipc_port_t xmm_object, old_xmm_object;
	xmm_obj_t mobj;
	kern_return_t kr;

	/*
	 * We always create the svm stack at the current location of the
	 * memory object. We may have to chase it down if it's migrating.
	 *
	 * The memory_object principal node is the one true source
	 * of knowledge about whether an svm stack exists.
	 */
	ip_lock(memory_object);
	if (IP_NORMA_IS_PROXY(memory_object)) {
		/* the following call inherits the lock */
		return xmm_object_by_memory_object_remote(memory_object);
	}

	/*
	 * If there is already an xmm_object associated with this
	 * memory_object, return it, after taking a send-right reference
	 * which will be given (moved, if necessary) to the caller.
	 */
	xmm_object = xmm_object_copy(memory_object);
	if (xmm_object != IP_NULL) {
		ip_unlock(memory_object);
		return xmm_object;
	}

	/*
	 * Check kobject type, to foil attempts to map in inappropriate
	 * kernel objects (like task ports).
	 */
	if (ip_kotype(memory_object) != IKOT_NONE &&
	    ip_kotype(memory_object) != IKOT_PAGER) {
		ip_unlock(memory_object);
		return IP_NULL;
	}

	/*
	 * No xmm object is currently associcated with memory object.
	 * Unlock memory object port, and create an xmm obj stack.
	 * and a corresponding xmm obj stack.
	 *
	 * XXX
	 * Should deallocate things if this call fails part-way.
	 */
	ip_unlock(memory_object);
	kr = xmm_user_create(memory_object, &mobj);
	if (kr != KERN_SUCCESS) {
		panic("xmm_mo_create: xmm_user_create: %x\n", kr);
		return IP_NULL;
	}
	kr = xmm_split_create(mobj, &mobj);
	if (kr != KERN_SUCCESS) {
		panic("xmm_mo_create: xmm_split_create: %x\n", kr);
		return IP_NULL;
	}
	kr = xmm_svm_create(mobj, memory_object, &mobj);
	if (kr != KERN_SUCCESS) {
		panic("xmm_mo_create: xmm_svm_create: %x\n", kr);
		return IP_NULL;
	}

	/*
	 * Create an xmm object and associate it with stack.
	 * It will have one send right and a no-senders notification request.
	 */
	xmm_object = xmm_object_allocate(mobj);
	if (xmm_object == IP_NULL) {
		panic("xmm_mo_create: xmm_object_allocate: %x\n", kr);
		return IP_NULL;
	}

	/*
	 * Now that we have a stack to associate with the memory object,
	 * make sure we still want it. If we don't, then just release
	 * the send right, and the no-senders notification handler
	 * will take care of deallocation.
	 *
	 * First, make sure that the memory object has not migrated.
	 */
	ip_lock(memory_object);
	if (IP_NORMA_IS_PROXY(memory_object)) {
		xmm_object_destroy(xmm_object, mobj);
		/* the following call inherits the lock */
		return xmm_object_by_memory_object_remote(memory_object);
	}

	/*
	 * If we lost the race to create the stack, discard ours
	 * and use the one already created. Otherwise, associate
	 * our xmm object and stack with the memory object,
	 * by giving the memory object the send right to the xmm object.
	 */
	old_xmm_object = xmm_object_copy(memory_object);
	if (old_xmm_object != IP_NULL) {
		xmm_object_destroy(xmm_object, mobj);
		xmm_object = old_xmm_object;
	} else {
		xmm_object_set(memory_object, xmm_object, TRUE);
	}

	/*
	 * Unlock memory object and return the xmm object send right.
	 */
	ip_unlock(memory_object);
	return xmm_object;
}

/*
 * Remote, protected cover routine for xmm_object_by_memory_object.
 * Requires host_priv.
 */
kern_return_t
norma_xmm_object_by_memory_object(host, memory_object, xmm_object)
	host_t host;
	ipc_port_t memory_object;
	ipc_port_t *xmm_object;
{
	/*
	 * Check host port validity.
	 */
	if (host == HOST_NULL) {
		return KERN_INVALID_ARGUMENT;
	}

	/*
	 * Obtain xmm_object, perhaps recursively.
	 */
	*xmm_object = xmm_object_by_memory_object(memory_object);

	/*
	 * Discard send right to memory_object given to us by our caller.
	 */
	ipc_port_release_send(memory_object);
	return KERN_SUCCESS;
}

/*
 * Called with memory_object locked. Unlocks memory_object.
 */
void
xmm_object_release_remote(memory_object)
	ipc_port_t memory_object;
{
	unsigned long node;

	node = ipc_port_node(memory_object);
	assert(node != node_self());
	ip_unlock(memory_object);
	r_norma_xmm_object_release(remote_host_priv(node), memory_object);
}

/*
 * If there are no real references to xmm object, then break its
 * association with memory object.
 */
void
xmm_object_release(memory_object)
	ipc_port_t memory_object;
{
	/*
	 * Use local or remote form as appropriate.
	 */
	ip_lock(memory_object);
	if (IP_NORMA_IS_PROXY(memory_object)) {
		/* the following call inherits the lock */
		xmm_object_release_remote(memory_object);
	} else {
		xmm_object_release_local(memory_object);
		ip_unlock(memory_object);
	}
}

/*
 * Remote, protected cover routine for xmm_object_release.
 * Requires host_priv.
 */
kern_return_t
norma_xmm_object_release(host, memory_object)
	host_t host;
	ipc_port_t memory_object;
{
	/*
	 * Check host port validity.
	 */
	if (host == HOST_NULL) {
		return KERN_INVALID_ARGUMENT;
	}

	/*
	 * Release xmm object.
	 */
	xmm_object_release(memory_object);

	/*
	 * Discard send right to memory_object given to us by our caller.
	 */
	ipc_port_release_send(memory_object);
	return KERN_SUCCESS;
}

/*
 * Create an xmm object and a stack for an xmm-internal memory manager.
 */
ipc_port_t
xmm_memory_manager_export(mobj)
	xmm_obj_t mobj;
{
	kern_return_t kr;
	ipc_port_t xmm_object;
	ipc_port_t memory_object;

	/*
	 * Create a memory object port for the memory manager.
	 */
	memory_object = ipc_port_alloc_kernel();
	if (memory_object == IP_NULL) {
		panic("xmm_memory_manager_export: memory_object");
		return IP_NULL;
	}

	/*
	 * Create an svm stack on top of mobj.
	 */
	kr = xmm_split_create(mobj, &mobj);
	if (kr != KERN_SUCCESS) {
		panic("xmm_memory_manager_export: xmm_split_create: %x\n", kr);
		return IP_NULL;
	}
	kr = xmm_svm_create(mobj, memory_object, &mobj);
	if (kr != KERN_SUCCESS) {
		panic("xmm_memory_manager_export: xmm_svm_create: %x\n", kr);
		return IP_NULL;
	}

	/*
	 * Create an xmm object and associate it with stack.
	 * It will have one send right and a no-senders notification request.
	 */
	xmm_object = xmm_object_allocate(mobj);
	if (xmm_object == IP_NULL) {
		panic("xmm_memory_manager_export: xmm_object_allocate");
	}

	/*
	 * Associate the xmm object with a memory object,
	 * and return a send right for the memory object.
	 */
	xmm_object_set(memory_object, xmm_object, FALSE);
	return ipc_port_make_send(memory_object);
}
