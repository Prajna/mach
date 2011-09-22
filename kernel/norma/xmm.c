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
 * $Log:	xmm.c,v $
 * Revision 2.7  92/03/10  16:28:49  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:50:54  jsb]
 * 
 * Revision 2.6.2.5  92/02/21  14:34:51  jsb
 * 	Fixed merge botch.
 * 
 * Revision 2.6.2.4  92/02/21  11:25:25  jsb
 * 	Disassociate and deallocate port in convert_port_to_reply.
 * 	[92/02/18  17:33:30  jsb]
 * 
 * 	Added multiple init detection code to xmm_kobj_link.
 * 	Changed MACH_PORT_NULL uses to IP_NULL.
 * 	[92/02/18  08:45:28  jsb]
 * 
 * 	Changed reply->mobj to reply->kobj.
 * 	Added reference counting to xmm_reply_{allocate,deallocate}.
 * 	[92/02/16  18:25:51  jsb]
 * 
 * 	Do real reference counting (except for xmm_replies).
 * 	[92/02/16  14:11:32  jsb]
 * 
 * 	Moved invocation routines to norma/xmm_invoke.c.
 * 	Changed reply walking routines to use kobj, not mobj.
 * 	Preparation for better reference counting on xmm objs.
 * 	[92/02/09  12:53:36  jsb]
 * 
 * Revision 2.6.2.3  92/02/18  19:17:22  jeffreyh
 * 	Changed reply walking routines to use kobj, not mobj.
 * 	Added xmm_buffer_init to norma_vm_init.
 * 	[92/02/12            jsb]
 * 
 * Revision 2.6.2.2  92/01/21  21:53:31  jsb
 * 	Added xmm_reply_notify and (nonfunctional) xmm_reply_send_once.
 * 	[92/01/21  18:23:51  jsb]
 * 
 * 	De-linted. Added xmm_reply routines. Added functional forms of
 * 	invocation routines. Added xmm_{obj,reply}_print routines.
 * 	[92/01/20  17:13:30  jsb]
 * 
 * Revision 2.6.2.1  92/01/03  16:38:38  jsb
 * 	Corrected log.
 * 	[91/12/24  14:33:43  jsb]
 * 
 * Revision 2.6  91/12/10  13:26:21  jsb
 * 	Added better debugging in xmm_obj_deallocate.
 * 	[91/12/10  11:35:06  jsb]
 * 
 * Revision 2.5  91/11/14  16:52:32  rpd
 *	Added missing argument to bcopy.
 * 	[91/11/00            jsb]
 * 
 * Revision 2.4  91/08/03  18:19:37  jsb
 * 	Renamed xmm_init() to norma_vm_init().
 * 	[91/07/24  23:25:57  jsb]
 * 
 * Revision 2.3  91/07/01  08:25:56  jsb
 * 	Removed non-KERNEL code.
 * 	Replaced Xobj_allocate with xmm_obj_allocate.
 * 	Added xmm_obj_deallocate.
 * 	Use per-class zone for obj allocation.
 * 	[91/06/29  15:21:33  jsb]
 * 
 * Revision 2.2  91/06/17  15:48:10  jsb
 * 	First checkin.
 * 	[91/06/17  10:58:38  jsb]
 * 
 */
/*
 *	File:	norma/xmm.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Common xmm support routines.
 */

#include <norma/xmm_obj.h>
#include <mach/notify.h>
#include <ipc/ipc_space.h>
#include <ipc/ipc_port.h>

zone_t xmm_reply_zone;

#define	OBJ_SETQ(lhs_obj, rhs_obj)\
((rhs_obj)->refcount++, (int)((lhs_obj) = (rhs_obj)))

/*
 * If caller provides old_mobj, then he donates a reference.
 * Returns a reference for new_mobj to caller.
 */
kern_return_t
xmm_obj_allocate(class, old_mobj, new_mobj)
	xmm_class_t class;
	xmm_obj_t old_mobj;
	xmm_obj_t *new_mobj;
{
	xmm_obj_t mobj;

	if (class->c_zone == ZONE_NULL) {
		char *zone_name;

		zone_name = (char *)
		    kalloc((vm_size_t) (strlen(class->c_name) + 5));
		bcopy("xmm.", zone_name, 4);
		bcopy(class->c_name, zone_name + 4, strlen(class->c_name) + 1);
		class->c_zone = zinit(class->c_size, 512*1024, class->c_size,
				      FALSE, zone_name);
	}
	mobj = (xmm_obj_t) zalloc(class->c_zone);
	bzero((char *) mobj, class->c_size);
	mobj->class = class;
	mobj->refcount = 1;
	mobj->m_kobj = XMM_OBJ_NULL;
	mobj->k_kobj = XMM_OBJ_NULL;
	if (old_mobj) {
		OBJ_SETQ(mobj->m_mobj, old_mobj);
		OBJ_SETQ(old_mobj->k_mobj, mobj);
		xmm_obj_release(old_mobj); /* release caller's reference */
	} else {
		mobj->m_mobj = XMM_OBJ_NULL;
	}
	*new_mobj = mobj;
	return KERN_SUCCESS;
}

/*
 * XXX
 * REPLACE THIS COMMENT
 *
 * Termination protocol:
 * Each layer terminates layer beneath it before deallocating.
 * Bottom layers disable outside upcalls before blocking.
 * (For example, xmm_user.c does kobject_set(NULL) before
 * calling memory_object_terminate.)
 */

xmm_kobj_link(kobj, k_kobj)
	xmm_obj_t kobj;
	xmm_obj_t k_kobj;
{
	assert(k_kobj->m_kobj == XMM_OBJ_NULL);
	if (kobj->k_kobj != XMM_OBJ_NULL) {
		panic("xmm_kobj_link: multiple init");
	}
	OBJ_SETQ(k_kobj->m_kobj, kobj);
	OBJ_SETQ(kobj->k_kobj, k_kobj);
}

xmm_obj_unlink(mobj, kobj)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
{
	register xmm_obj_t m_mobj = mobj->m_mobj;
	register xmm_obj_t m_kobj = kobj->m_kobj;

	assert(m_mobj != XMM_OBJ_NULL);
	mobj->m_mobj = XMM_OBJ_NULL;
	assert(m_kobj != XMM_OBJ_NULL);
	kobj->m_kobj = XMM_OBJ_NULL;
	xmm_obj_release(m_mobj);
	xmm_obj_release(m_kobj);
}

xmm_obj_reference(obj)
	xmm_obj_t obj;
{
	obj->refcount++;
}

xmm_obj_release(obj)
	xmm_obj_t obj;
{
	assert(obj->refcount > 0);
	if (--obj->refcount == 0) {
		assert(obj->m_mobj == XMM_OBJ_NULL);
		assert(obj->m_kobj == XMM_OBJ_NULL);
		if (obj->k_mobj) {
			xmm_obj_release(obj->k_mobj);
			obj->k_mobj = XMM_OBJ_NULL;
		}
		if (obj->k_kobj) {
			xmm_obj_release(obj->k_kobj);
			obj->k_kobj = XMM_OBJ_NULL;
		}
		obj->class->m_deallocate(obj);
		zfree(obj->class->c_zone, (vm_offset_t) obj);
	}
}

kern_return_t
xmm_reply_allocate(kobj, reply_to, reply_to_type, new_reply)
	xmm_obj_t kobj;
	ipc_port_t reply_to;
	mach_msg_type_name_t reply_to_type;
	xmm_reply_t *new_reply;
{
	register xmm_reply_t reply;

	if (reply_to == IP_NULL) {
		*new_reply = XMM_REPLY_NULL;
		return KERN_SUCCESS;
	}
	reply = (xmm_reply_t) zalloc(xmm_reply_zone);
	reply->kobj = kobj;
	reply->reply_to = reply_to;
	reply->reply_to_type = reply_to_type;
	reply->reply_proxy = IP_NULL;
	OBJ_SETQ(reply->kobj_held, kobj);
	*new_reply = reply;
	return KERN_SUCCESS;
}

xmm_reply_deallocate(reply)
	xmm_reply_t reply;
{
	xmm_obj_release(reply->kobj_held);
	zfree(xmm_reply_zone, (vm_offset_t) reply);
}

/*
 * XXX
 * The same proxy could be used over and over again. Perhaps should
 * have a pool of xmm_replies, with reply_proxies always allocated,
 * and eliminate this routine.
 */
kern_return_t
xmm_reply_allocate_proxy(reply)
	xmm_reply_t reply;
{
	if (reply == XMM_REPLY_NULL) {
		return KERN_SUCCESS;
	}
	assert(reply->reply_proxy == IP_NULL);
	reply->reply_proxy = ipc_port_alloc_kernel();
	if (reply->reply_proxy == IP_NULL) {
		panic("xmm_reply_allocate_proxy");
		return KERN_FAILURE;
	}
	ipc_kobject_set(reply->reply_proxy, (ipc_kobject_t) reply,
			IKOT_XMM_REPLY);
	return KERN_SUCCESS;
}

/*
 * Finds reply corresponding to port.
 * Disassociates port from reply and deallocates port.
 */
xmm_reply_t
convert_port_to_reply(port)
	ipc_port_t port;
{
	xmm_reply_t reply = XMM_REPLY_NULL;

	if (IP_VALID(port)) {
		ip_lock(port);
		if (ip_active(port) && ip_kotype(port) == IKOT_XMM_REPLY) {
			reply = (xmm_reply_t) port->ip_kobject;
			ipc_kobject_set(port, IKO_NULL, IKOT_NONE);
			assert(reply->reply_proxy == port);
			reply->reply_proxy = IP_NULL;
		}
		ip_unlock(port);
		ipc_port_dealloc_kernel(port);
	}
	return reply;
}

/*
 * XXX
 * This is nice, but how do we find the intended destination of this reply?
 */
xmm_reply_send_once(notification)
	mach_send_once_notification_t *notification;
{
	panic("xmm_reply_no_senders");
}

boolean_t
xmm_reply_notify(msg)
	mach_msg_header_t *msg;
{
	switch (msg->msgh_id) {
		case MACH_NOTIFY_SEND_ONCE:
		xmm_reply_send_once((mach_send_once_notification_t *) msg);
		return TRUE;

		default:
		printf("ds_notify: strange notification %d\n", msg->msgh_id);
		return FALSE;
	}
}

xmm_reply_t
xmm_k_reply(reply)
	xmm_reply_t reply;
{
	if (reply != XMM_REPLY_NULL) {
		assert(reply->kobj != XMM_OBJ_NULL);
		reply->kobj = reply->kobj->k_kobj;
		assert(reply->kobj != XMM_OBJ_NULL);
	}
	return reply;
}

xmm_class_t
xmm_m_reply(reply)
	xmm_reply_t reply;
{
	assert(reply->kobj != XMM_OBJ_NULL);
	reply->kobj = reply->kobj->m_kobj;
	assert(reply->kobj != XMM_OBJ_NULL);
	return reply->kobj->class;
}

norma_vm_init()
{
	xmm_svm_init();
	xmm_split_init();
	xmm_buffer_init();
	xmm_reply_zone = zinit(sizeof(struct xmm_reply), 512*1024,
			       sizeof(struct xmm_reply), FALSE, "xmm_reply");
}

#include <mach_kdb.h>
#if	MACH_KDB
#define	printf	kdbprintf

/*
 *	Routine:	xmm_obj_print
 *	Purpose:
 *		Pretty-print an xmm obj.
 */

void
xmm_obj_print(obj)
	xmm_obj_t obj;
{
	extern int indent;

	printf("xmm obj 0x%x\n", obj);

	indent += 2;

	iprintf("class=0x%x[%s]", obj->class, obj->class->c_name);
	printf(", refcount=%d\n", obj->refcount);

	iprintf("m_mobj=0x%x", obj->m_mobj);
	printf(", m_kobj=0x%x", obj->m_kobj);
	printf(", k_mobj=0x%x", obj->k_mobj);
	printf(", k_kobj=0x%x\n", obj->k_kobj);

	indent -=2;
}

/*
 *	Routine:	xmm_reply_print
 *	Purpose:
 *		Pretty-print an xmm reply.
 */

void
xmm_reply_print(reply)
	xmm_reply_t reply;
{
	extern int indent;

	printf("xmm reply 0x%x\n", reply);

	indent += 2;

	iprintf("reply_to=0x%x", reply->reply_to);
	printf(", reply_to_type=%d[", reply->reply_to_type);
	switch (reply->reply_to_type) {
		case MACH_MSG_TYPE_PORT_SEND:
		printf("send");
		break;

		case MACH_MSG_TYPE_PORT_SEND_ONCE:
		printf("send_once");
		break;

		case XMM_SVM_REPLY:
		printf("svm");
		break;

		case XMM_SPLIT_REPLY:
		printf("split");
		break;

		default:
		printf("???");
		break;
	}
	printf("]\n");

	iprintf("kobj=0x%x", reply->kobj);
	printf(", kobj_held=0x%x", reply->kobj_held);
	printf(", reply_proxy=0x%x\n", reply->reply_proxy);

	indent -=2;
}

#endif	MACH_KDB
