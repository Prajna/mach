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
 * $Log:	xmm_split.c,v $
 * Revision 2.4  92/03/10  16:29:47  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:51:54  jsb]
 * 
 * Revision 2.3.2.2  92/02/21  11:28:13  jsb
 * 	Changed reply->mobj to reply->kobj. Discard reply in lock_completed.
 * 	[92/02/16  18:24:34  jsb]
 * 
 * 	Explicitly provide name parameter to xmm_decl macro.
 * 	Changed debugging printf name and declaration.
 * 	[92/02/16  14:25:05  jsb]
 * 
 * 	Use new xmm_decl, and new memory_object_name and deallocation protocol.
 * 	[92/02/09  14:17:57  jsb]
 * 
 * Revision 2.3.2.1  92/01/21  21:54:54  jsb
 * 	De-linted. Supports new (dlb) memory object routines.
 * 	Supports arbitrary reply ports to lock_request, etc.
 * 	Converted mach_port_t (and port_t) to ipc_port_t.
 * 	[92/01/20  17:30:57  jsb]
 * 
 * Revision 2.3  91/07/01  08:26:34  jsb
 * 	Use zone for requests.
 * 	[91/06/29  15:34:20  jsb]
 * 
 * Revision 2.2  91/06/17  15:48:40  jsb
 * 	First checkin.
 * 	[91/06/17  11:04:50  jsb]
 * 
 */
/*
 *	File:	norma/xmm_split.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Xmm layer to split multi-page lock_requests.
 */

#ifdef	KERNEL
#include <norma/xmm_obj.h>
#include <mach/vm_param.h>
#else	KERNEL
#include <xmm_obj.h>
#endif	KERNEL

#define	dprintf	xmm_split_dprintf

typedef struct request *request_t;

#define	REQUEST_NULL	((request_t) 0)

struct mobj {
	struct xmm_obj	obj;
	request_t	r_head;
	request_t	r_tail;
};

struct request {
	vm_offset_t	r_offset;
	vm_size_t	r_length;
	vm_offset_t	r_start;
	vm_size_t	r_count;
	vm_size_t	r_resid;
	request_t	r_next;
};

#undef  KOBJ
#define KOBJ    ((struct mobj *) kobj)

#define	m_split_init			m_interpose_init
#define	m_split_terminate		m_interpose_terminate
#define	m_split_deallocate		m_interpose_deallocate
#define	m_split_copy			m_interpose_copy
#define	m_split_data_request		m_interpose_data_request
#define	m_split_data_unlock		m_interpose_data_unlock
#define	m_split_data_write		m_interpose_data_write
#define	m_split_supply_completed	m_interpose_supply_completed
#define	m_split_data_return		m_interpose_data_return
#define	m_split_change_completed	m_interpose_change_completed

#define	k_split_data_unavailable	k_interpose_data_unavailable
#define	k_split_get_attributes		k_interpose_get_attributes
#define	k_split_data_error		k_interpose_data_error
#define	k_split_set_ready		k_interpose_set_ready
#define	k_split_destroy			k_interpose_destroy
#define	k_split_data_supply		k_interpose_data_supply

xmm_decl(split, "split", sizeof(struct mobj));

zone_t		xmm_split_request_zone;

xmm_reply_t
xmm_reply_allocate_reply(mobj, real_reply)
	xmm_obj_t mobj;
	xmm_reply_t real_reply;
{
	kern_return_t kr;
	xmm_reply_t reply;

	if (real_reply == XMM_REPLY_NULL) {
		return XMM_REPLY_NULL;
	}
	kr = xmm_reply_allocate(mobj, (ipc_port_t) real_reply, XMM_SPLIT_REPLY,
				&reply);
	if (kr != KERN_SUCCESS) {
		panic("xmm_reply_allocate_mobj: xmm_reply_allocate: %d\n", kr);
	}
	return reply;
}

kern_return_t
xmm_split_create(old_mobj, new_mobj)
	xmm_obj_t old_mobj;
	xmm_obj_t *new_mobj;
{
	xmm_obj_t mobj;
	kern_return_t kr;

	kr = xmm_obj_allocate(&split_class, old_mobj, &mobj);
	if (kr != KERN_SUCCESS) {
		return kr;
	}
	MOBJ->r_head = REQUEST_NULL;
	MOBJ->r_tail = REQUEST_NULL;
	*new_mobj = mobj;
	return KERN_SUCCESS;
}

k_split_lock_request(kobj, offset, length, should_clean, should_flush,
		     lock_value, reply)
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	boolean_t should_clean;
	boolean_t should_flush;
	vm_prot_t lock_value;
	xmm_reply_t reply;
{
	request_t r;

#ifdef	lint
	K_LOCK_REQUEST(kobj, offset, length, should_clean, should_flush,
		       lock_value, reply);
#endif	lint
	/*
	 * If we don't have to split the request,
	 * then just pass it through.
	 */
	if (length == 0) {
		return M_LOCK_COMPLETED(reply, offset, length);
	}
	if (length <= PAGE_SIZE) {
		return K_LOCK_REQUEST(kobj, offset, length, should_clean,
				      should_flush, lock_value, reply);
	}
	if (reply != XMM_REPLY_NULL) {
		r = (request_t) zalloc(xmm_split_request_zone);
		r->r_offset = offset;
		r->r_length = length;
		r->r_start = (offset + PAGE_SIZE - 1) / PAGE_SIZE;
		r->r_count = (length + PAGE_SIZE - 1) / PAGE_SIZE;
		r->r_resid = r->r_count;
		r->r_next = REQUEST_NULL;
		if (KOBJ->r_head) {
			KOBJ->r_tail->r_next = r;
		} else {
			KOBJ->r_head = r;
		}
		KOBJ->r_tail = r;
		dprintf("@r start=%d count=%d\n", r->r_start, r->r_count);
	}
	while (length > PAGE_SIZE) {
		K_LOCK_REQUEST(kobj, offset, PAGE_SIZE, should_clean,
			       should_flush, lock_value,
			       xmm_reply_allocate_reply(kobj, reply));
		offset += PAGE_SIZE;
		length -= PAGE_SIZE;
	}
	if (length > 0) {
		K_LOCK_REQUEST(kobj, offset, length, should_clean,
			       should_flush, lock_value,
			       xmm_reply_allocate_reply(kobj, reply));
	}
	/* XXX check return values above??? */
	return KERN_SUCCESS;
}

m_split_lock_completed(reply, offset, length)
	xmm_reply_t reply;
	vm_offset_t offset;
	vm_size_t length;	/* XXX ignored */
{
	vm_offset_t start;
	request_t r, *rp, r_prev = REQUEST_NULL;
	xmm_obj_t kobj;
	xmm_reply_t real_reply;

#ifdef	lint
	M_LOCK_COMPLETED(reply, offset, length);
#endif	lint
	/*
	 * If reply is not a private xmm_split reply, then
	 * just pass it through.
	 */
	if (reply->reply_to_type != XMM_SPLIT_REPLY) {
		return M_LOCK_COMPLETED(reply, offset, length);
	}

	/*
	 * Retrieve kobj and real reply from private reply,
	 * and discard private reply.
	 */
	kobj = reply->kobj;
	assert(kobj->class == &split_class);
	real_reply = (xmm_reply_t) reply->reply_to;
	xmm_reply_deallocate(reply);

	/*
	 * Look for request structure.
	 * XXX reply->reply_to really should be r, not reply.
	 * XXX that would eliminate this stupid search.
	 * XXX same thing goes for xmm_svm module.
	 */
	start = (offset + PAGE_SIZE - 1) / PAGE_SIZE;
	dprintf("#r start=%d\n", start);
	for (rp = &KOBJ->r_head; r = *rp; rp = &r->r_next) {
		dprintf(":r start=%d..%d resid=%d\n",
		       r->r_start, r->r_start + r->r_count - 1, r->r_resid);
		if (start >= r->r_start && start < r->r_start + r->r_count) {
			break;
		}
		r_prev = r;
	}
	if (r == REQUEST_NULL) {
		panic("k_split_lock_completed: lost request\n");
	}
	dprintf("#r found\n");

	/*
	 * If this is the reply that we are waiting for, then
	 * send the real unsplit reply. Otherwise, just return.
	 */
	if (--r->r_resid > 0) {
		return KERN_SUCCESS;
	}
	if (r == KOBJ->r_tail) {
		KOBJ->r_tail = r_prev;
	}
	*rp = r->r_next;
	M_LOCK_COMPLETED(real_reply, r->r_offset, r->r_length);
	zfree(xmm_split_request_zone, (vm_offset_t) r);
	return KERN_SUCCESS;
}

xmm_split_init()
{
	xmm_split_request_zone = zinit(sizeof(struct request), 512*1024,
				       sizeof(struct request), FALSE,
				       "xmm.split.request");
}

#include <sys/varargs.h>

int xmm_split_debug = 0;

/* VARARGS */
xmm_split_dprintf(fmt, va_alist)
	char *fmt;
	va_dcl
{
	va_list	listp;

	if (xmm_split_debug) {
		va_start(listp);
		printf(fmt, &listp);
		va_end(listp);
	}
}
