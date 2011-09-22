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
 * $Log:	xmm_svm.c,v $
 * Revision 2.5  92/03/10  16:29:50  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:51:57  jsb]
 * 
 * Revision 2.4.3.3  92/02/21  11:28:23  jsb
 * 	Replaced xmm_reply_allocate_mobj with m_svm_do_request, which now takes
 * 	a reference to mobj. m_svm_lock_completed now deallocates reply as well
 * 	as reference to mobj.
 * 	[92/02/18  17:31:27  jsb]
 * 
 * 	Cosmetic changes, including vm_page_size -> PAGE_SIZE.
 * 	[92/02/18  08:01:18  jsb]
 * 
 * 	Explicitly provide name parameter to xmm_decl macro.
 * 	Added MOBJ_STATE_TERMINATED to detect init/terminate race.
 * 	Added memory_object parameter to xmm_svm_create, and memory_object
 * 	field to struct mobj, so that m_svm_terminate can call
 * 	xmm_object_release on memory_object. Move M_TERMINATE call
 * 	to new routine xmm_svm_destroy, which is called from xmm_object
 * 	module only when there are no references to xmm object.
 * 	This fixes race between xmm_object_by_memory_object (where someone
 * 	decides to use our existing svm stack) and m_svm_terminate
 * 	(where we used to tear down the stack as soon as all the kernels
 * 	we knew about had terminated the object).
 * 	[92/02/16  15:50:31  jsb]
 * 
 * 	Changed copy strategy management to handle (naively)
 * 	MEMORY_OBJECT_COPY_TEMPORARY (by passing it up unchanged).
 * 	This will break in its current form when we enable VM_INHERIT_SHARE.
 * 	(Added appropriate checks to panic in this case.) Removed dead
 * 	routines xmm_svm_{set_access,initialize}. Changed debugging printfs.
 * 	[92/02/11  11:32:58  jsb]
 * 
 * 	Use new xmm_decl, and new memory_object_name and deallocation protocol.
 * 	Use xmm_buffer layer to buffer data writes of migrating pages.
 * 	General cleanup.
 * 	[92/02/09  13:58:24  jsb]
 * 
 * Revision 2.4.3.1  92/01/21  21:54:57  jsb
 * 	De-linted. Supports new (dlb) memory object routines.
 * 	Supports arbitrary reply ports to lock_request, etc.
 * 	Converted mach_port_t (and port_t) to ipc_port_t.
 * 	[92/01/20  17:46:55  jsb]
 * 
 * 	Fixes from OSF.
 * 	[92/01/17  14:15:53  jsb]
 * 
 * Revision 2.4.1.1  92/01/15  12:17:45  jeffreyh
 * 	Deallocate memory_object_name port when not propagating
 *	termination. (dlb)
 * 
 * Revision 2.4  91/08/03  18:19:47  jsb
 * 	Added missing type cast.
 * 	[91/07/17  14:07:46  jsb]
 * 
 * Revision 2.3  91/07/01  08:26:40  jsb
 * 	Now allow objects to grow in size (as temporary objects do).
 * 	Merged user_t and kobj structures. Do garbage collection.
 * 	Now pass up all set_attribute calls, not just first.
 * 	Use zone for request structures.
 * 	[91/06/29  15:43:16  jsb]
 * 
 * Revision 2.2  91/06/17  15:48:43  jsb
 * 	First checkin.
 * 	[91/06/17  11:04:03  jsb]
 * 
 */
/*
 *	File:	norma/xmm_svm.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Xmm layer providing consistent shared virtual memory.
 */

#ifdef	KERNEL
#include <norma/xmm_obj.h>
#include <mach/vm_param.h>
#include <ipc/ipc_port.h>
#include <ipc/ipc_space.h>
#else	KERNEL
#include <xmm_obj.h>
#endif	KERNEL

#define	dprintf	xmm_svm_dprintf

#define	USE_XMM_BUFFER	1

typedef struct request *	request_t;

#define	REQUEST_NULL		((request_t) 0)

#define	MOBJ_STATE_UNCALLED	0
#define	MOBJ_STATE_CALLED	1
#define	MOBJ_STATE_READY	2
#define	MOBJ_STATE_TERMINATED	3

#define	DATA_NONE		((vm_offset_t) 0)
#define	DATA_UNAVAILABLE	((vm_offset_t) 1)
#define	DATA_ERROR		((vm_offset_t) 2)

#define	K			((struct kobj *) k)

/*
 * lock[] is set when pager gives us a message.
 * prot[] is set when we send message to kernels;
 * it should simply reflect max of all kobj->prot.
 */
struct mobj {
	struct xmm_obj	obj;
	xmm_obj_t	kobj_list;
	int		state;
	unsigned int	num_pages;
	request_t	request;
	vm_prot_t	*prot;			/* kernel access */
	vm_prot_t	*lock;			/* lock by pager */
	boolean_t	may_cache;
	ipc_port_t	memory_object;		/* for xmm_object_release */
	ipc_port_t	memory_object_name;	/* at most one send right */
	memory_object_copy_strategy_t
			copy_strategy;
};

union who {
	xmm_obj_t	kobj;
	xmm_reply_t	reply;
};

/*
 * XXX some of these fields could be aliased to save space
 * XXX eg: needs_data,should_clean; lock_value,desired_access
 *
 * XXX should probably add ref counts to kobjs....
 */
struct request {
	union who	who;
	int		m_count;		/* -> m_yield_count */
	int		k_count;		/* -> m_yield_count */
	boolean_t	is_kernel;
	boolean_t	needs_data;		/* ours alone */
	boolean_t	should_clean;		/* same as needs_data? */
	boolean_t	should_flush;
	vm_prot_t	desired_access;
	vm_prot_t	lock_value;
	vm_offset_t	offset;			/* -> page */
	request_t	next_eq;
	request_t	next_ne;
};

struct kobj {
	struct xmm_obj	obj;
	unsigned int	num_pages;		/* needed for deallocation */
	vm_prot_t	*prot;
	xmm_obj_t	next;
};

#define	m_msvm_init		m_svm_init
#define	m_msvm_terminate	m_svm_terminate
#define	m_msvm_copy		m_invalid_copy
#define	m_msvm_data_request	m_svm_data_request
#define	m_msvm_data_unlock	m_svm_data_unlock
#define	m_msvm_data_write	m_svm_data_write
#define	m_msvm_lock_completed	m_invalid_lock_completed
#define	m_msvm_supply_completed	m_invalid_supply_completed
#define	m_msvm_data_return	m_invalid_data_return
#define	m_msvm_change_completed	m_invalid_change_completed
#define	k_msvm_data_unavailable	k_svm_data_unavailable
#define	k_msvm_get_attributes	k_invalid_get_attributes
#define	k_msvm_lock_request	k_svm_lock_request
#define	k_msvm_data_error	k_svm_data_error
#define	k_msvm_set_ready	k_svm_set_ready
#define	k_msvm_destroy		k_svm_destroy
#define	k_msvm_data_supply	k_svm_data_supply
#define	m_msvm_deallocate	m_svm_deallocate

xmm_decl(msvm, "msvm", sizeof(struct mobj));

#define	m_ksvm_init		m_invalid_init
#define	m_ksvm_terminate	m_invalid_terminate
#define	m_ksvm_copy		m_invalid_copy
#define	m_ksvm_data_request	m_invalid_data_request
#define	m_ksvm_data_unlock	m_invalid_data_unlock
#define	m_ksvm_data_write	m_invalid_data_write
#define	m_ksvm_lock_completed	m_svm_lock_completed
#define	m_ksvm_supply_completed	m_invalid_supply_completed
#define	m_ksvm_data_return	m_invalid_data_return
#define	m_ksvm_change_completed	m_invalid_change_completed
#define	k_ksvm_data_unavailable	k_svm_data_unavailable
#define	k_ksvm_get_attributes	k_invalid_get_attributes
#define	k_ksvm_lock_request	k_svm_lock_request
#define	k_ksvm_data_error	k_svm_data_error
#define	k_ksvm_set_ready	k_svm_set_ready
#define	k_ksvm_destroy		k_svm_destroy
#define	k_ksvm_data_supply	k_svm_data_supply
#define	m_ksvm_deallocate	k_svm_deallocate

xmm_decl(ksvm, "ksvm", sizeof(struct kobj));

extern void	xmm_object_release();

boolean_t	m_svm_add_request();
request_t	m_svm_lookup_request();
void		m_svm_satisfy_request();
void		m_svm_satisfy_kernel_request();
void		m_svm_satisfy_pager_request();
void		m_svm_process_request();
void		m_svm_process_kernel_request();
void		m_svm_process_pager_request();

zone_t		xmm_svm_request_zone;

int C_mobj_prot = 0;
int C_mobj_lock = 0;
int C_user_prot = 0;

/* XXX should be implemented by kalloc.c */
/* XXX should kalloc have asm help for round-to-power-of-two? */
krealloc(old_buf_p, old_size, new_size, counter)
	char **old_buf_p;
	vm_size_t old_size;
	vm_size_t new_size;
	int *counter;
{
	char *new_buf;

	new_buf = (char *) kalloc(new_size);
	if (new_buf == (char *) 0) {
		panic("krealloc");
	}
	if (old_size > 0) {
		bcopy(*old_buf_p, new_buf, old_size);
		kfree(*old_buf_p, old_size);
	}
	*counter += (new_size - old_size);
	*old_buf_p = new_buf;
}

void
m_svm_extend(mobj, new_num_pages)
	xmm_obj_t mobj;
	unsigned int new_num_pages;
{
	xmm_obj_t kobj;

	int page, i;
	unsigned int old_num_pages = MOBJ->num_pages;

	for (i = 4; i < new_num_pages; i += i) {
		continue;
	}
	new_num_pages = i;
	MOBJ->num_pages = new_num_pages;
/*	assert(new_num_pages > old_num_pages);*/
	krealloc((char **) &MOBJ->prot,
		 old_num_pages * sizeof(vm_prot_t),
		 new_num_pages * sizeof(vm_prot_t),
		 &C_mobj_prot);
	krealloc((char **) &MOBJ->lock,
		 old_num_pages * sizeof(vm_prot_t),
		 new_num_pages * sizeof(vm_prot_t),
		 &C_mobj_lock);
	for (kobj = MOBJ->kobj_list; kobj; kobj = KOBJ->next) {
		assert(KOBJ->num_pages == old_num_pages);
		KOBJ->num_pages = new_num_pages;
		krealloc((char **) &KOBJ->prot,
			 old_num_pages * sizeof(vm_prot_t),
			 new_num_pages * sizeof(vm_prot_t),
			 &C_user_prot);
	}
	for (page = old_num_pages; page < new_num_pages; page++) {
		MOBJ->prot[page] = VM_PROT_NONE;
		MOBJ->lock[page] = VM_PROT_ALL;
		for (kobj = MOBJ->kobj_list; kobj; kobj = KOBJ->next) {
			KOBJ->prot[page] = VM_PROT_NONE;
		}
	}
}

kern_return_t
xmm_svm_create(old_mobj, memory_object, new_mobj)
	xmm_obj_t old_mobj;
	ipc_port_t memory_object;
	xmm_obj_t *new_mobj;
{
	xmm_obj_t mobj;
	kern_return_t kr;

#if	USE_XMM_BUFFER
	kr = xmm_buffer_create(old_mobj, &old_mobj);
	if (kr != KERN_SUCCESS) {
		return kr;
	}
#endif	USE_XMM_BUFFER
	kr = xmm_obj_allocate(&msvm_class, old_mobj, &mobj);
	if (kr != KERN_SUCCESS) {
		return kr;
	}
	MOBJ->num_pages = 0;
	MOBJ->kobj_list = XMM_OBJ_NULL;
	MOBJ->prot = (vm_prot_t *) 0;
	MOBJ->lock = (vm_prot_t *) 0;
	MOBJ->request = REQUEST_NULL;
	MOBJ->memory_object = memory_object;
	MOBJ->memory_object_name = IP_NULL;
	*new_mobj = mobj;
	return KERN_SUCCESS;
}

m_svm_init(mobj, k_kobj, pagesize, internal, size)
	xmm_obj_t mobj;
	xmm_obj_t k_kobj;
	vm_size_t pagesize;
	boolean_t internal;
	vm_size_t size;
{
	xmm_obj_t kobj;

#ifdef	lint
	M_INIT(mobj, k_kobj, pagesize, internal, size);
#endif	lint
	assert(pagesize == PAGE_SIZE);
	if (xmm_obj_allocate(&ksvm_class, XMM_OBJ_NULL, &kobj)) {
		panic("m_svm_init");
	}
	xmm_kobj_link(kobj, k_kobj);

	KOBJ->num_pages = MOBJ->num_pages;
	KOBJ->prot = (vm_prot_t *) kalloc(KOBJ->num_pages * sizeof(vm_prot_t));
#if 9
	C_user_prot += (KOBJ->num_pages * sizeof(vm_prot_t));
#endif
	if (! KOBJ->prot) {
		panic("m_svm_init");
	}
	bzero((char *) KOBJ->prot,
	      (int) (KOBJ->num_pages * sizeof(vm_prot_t)));

	KOBJ->next = MOBJ->kobj_list;
	MOBJ->kobj_list = kobj;

	/*
	 * If there are multiple kernels, then we had better be
	 * using MEMORY_OBJECT_COPY_NONE, at least until we get
	 * trickier about changing copy strategies.
	 */
	if (MOBJ->kobj_list && ((struct kobj *)MOBJ->kobj_list)->next &&
	    MOBJ->copy_strategy != MEMORY_OBJECT_COPY_NONE) {
		panic("losing big on multiple copies of temporary object");
	}

	if (MOBJ->state == MOBJ_STATE_READY) {
		assert(MOBJ->memory_object_name != IP_NULL);
		ipc_port_copy_send(MOBJ->memory_object_name);
		K_SET_READY(kobj, OBJECT_READY_TRUE, MOBJ->may_cache,
			    MOBJ->copy_strategy, USE_OLD_PAGEOUT_TRUE,
			    MOBJ->memory_object_name, XMM_REPLY_NULL);
	} else if (MOBJ->state == MOBJ_STATE_UNCALLED) {
		MOBJ->state = MOBJ_STATE_CALLED;
		M_INIT(mobj, mobj, PAGE_SIZE, internal, size);
	} else {
		assert(MOBJ->state == MOBJ_STATE_TERMINATED);
		panic("m_svm_init: terminate/lookup race");
	}

	return KERN_SUCCESS;
}

m_svm_terminate(mobj, kobj)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
{
	xmm_obj_t kobj_terminated, *kp;

#ifdef	lint
	M_TERMINATE(mobj, kobj);
#endif	lint
	/*
	 * Remove kobj from list and free its resources.
	 * Return if there are more kobjs.
	 */
	kobj_terminated = kobj;
	for (kp = &MOBJ->kobj_list; kobj = *kp; kp = &KOBJ->next) {
		if (kobj == kobj_terminated) {
			*kp = KOBJ->next;
			break;
		}
	}
	xmm_obj_release(kobj_terminated);

	/*
	 * Release one reference to xmm object. If there are no
	 * more references, then xmm_svm_destroy will be called.
	 */
	xmm_object_release(MOBJ->memory_object);
	return KERN_SUCCESS;
}

void
xmm_svm_destroy(mobj)
	xmm_obj_t mobj;
{
	assert(mobj->class == &msvm_class);
	MOBJ->state = MOBJ_STATE_TERMINATED;
	(void) M_TERMINATE(mobj, mobj);
}

void
k_svm_deallocate(kobj)
	xmm_obj_t kobj;
{
	/*
	 * Free kobj's resources.
	 */
	if (KOBJ->num_pages > 0) {
		kfree((char *) KOBJ->prot,
		      KOBJ->num_pages * sizeof(vm_prot_t));
#if 9
		C_user_prot -= (KOBJ->num_pages * sizeof(vm_prot_t));
#endif
	}
}

void
m_svm_deallocate(mobj)
	xmm_obj_t mobj;
{
	/*
	 * Free mobj's resources.
	 */
	if (MOBJ->num_pages > 0) {
		kfree((char *) MOBJ->prot,
		      MOBJ->num_pages * sizeof(vm_prot_t));
		kfree((char *) MOBJ->lock,
		      MOBJ->num_pages * sizeof(vm_prot_t));
#if 9
		C_mobj_prot -= (MOBJ->num_pages * sizeof(vm_prot_t));
		C_mobj_lock -= (MOBJ->num_pages * sizeof(vm_prot_t));
#endif
	}
	if (MOBJ->memory_object_name != IP_NULL) {
		ipc_port_release_send(MOBJ->memory_object_name);
	}
}

void
m_svm_request(mobj, r)
	xmm_obj_t mobj;
	request_t r;
{
	assert(mobj->class == &msvm_class);
	if((unsigned long)atop(r->offset) >= MOBJ->num_pages) {
		m_svm_extend(mobj, atop(r->offset) + 1);
	}
	if (m_svm_add_request(mobj, r)) {
		m_svm_process_kernel_request(mobj, r);
	}
}

m_svm_data_request(mobj, kobj, offset, length, desired_access)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
	request_t r;

#ifdef	lint
	M_DATA_REQUEST(mobj, kobj, offset, length, desired_access);
#endif	lint
	assert(mobj->class == &msvm_class);
	assert(kobj->class == &ksvm_class);
	if (length != PAGE_SIZE) {
		K_DATA_ERROR(kobj, offset, length, KERN_FAILURE);
		return KERN_FAILURE;
	}
	r = (request_t) zalloc(xmm_svm_request_zone);
	r->who.kobj = kobj;
	r->is_kernel = TRUE;
	r->m_count = 0;
	r->k_count = 0;
	r->needs_data = TRUE;
	r->should_clean = FALSE;
	r->should_flush = FALSE;
	r->desired_access = desired_access;
	r->offset = offset;
	r->next_ne = 0;
	r->next_eq = 0;
	m_svm_request(mobj, r);
	return KERN_SUCCESS;
}

m_svm_data_unlock(mobj, kobj, offset, length, desired_access)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
	request_t r;

#ifdef	lint
	M_DATA_UNLOCK(mobj, kobj, offset, length, desired_access);
#endif	lint
	assert(mobj->class == &msvm_class);
	assert(kobj->class == &ksvm_class);
	if (length != PAGE_SIZE) {
		K_DATA_ERROR(kobj, offset, length, KERN_FAILURE);
		return KERN_FAILURE;
	}
	r = (request_t) zalloc(xmm_svm_request_zone);
	r->who.kobj = kobj;
	r->is_kernel = TRUE;
	r->m_count = 0;
	r->k_count = 0;
	r->needs_data = FALSE;
	r->should_clean = FALSE;
	r->should_flush = FALSE;
	r->desired_access = desired_access;
	r->offset = offset;
	r->next_ne = 0;
	r->next_eq = 0;
	m_svm_request(mobj, r);
	return KERN_SUCCESS;
}

m_svm_data_write(mobj, kobj, offset, data, length)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_offset_t data;
	vm_size_t length;
{
	request_t r;

#ifdef	lint
	M_DATA_WRITE(mobj, kobj, offset, data, length);
#endif	lint
#if	USE_XMM_BUFFER
	assert(mobj->class == &msvm_class);
	assert(kobj->class == &ksvm_class);
	/* make sanity checks */
	r = m_svm_lookup_request(mobj, offset);
	if (r == REQUEST_NULL || ! r->is_kernel) {
		/*
		 * If there is no request, then this is an unsolicited
		 * pageout. We don't want to buffer this, since no one
		 * wants it.
		 *
		 * If this is not a kernel request, then it is a pager
		 * request, and thus the pager wants this page. We
		 * don't want to buffer the page in this case either.
		 */
		return M_DATA_WRITE(mobj, mobj, offset, data, length);
	} else {
		/*
		 * To avoid deadlock, pager requests have priority.
		 * Thus, if first request is a kernel, then all are.
		 * Therefore this pageout is wanted by kernels and
		 * not by the memory manager. This is case in which
		 * we want to buffer the page.
		 */
		return M_BUFFERED_DATA_WRITE(mobj, mobj, offset, data, length);
	}
#else	USE_XMM_BUFFER
	return M_DATA_WRITE(mobj, mobj, offset, data, length);
#endif	USE_XMM_BUFFER
}

m_svm_do_lock_request(k, should_clean, should_flush, lock_value, r, mobj)
	xmm_obj_t k;
	boolean_t should_clean;
	boolean_t should_flush;
	vm_prot_t lock_value;
	request_t r;
	xmm_obj_t mobj;

{
	kern_return_t kr;
	xmm_reply_t reply;

	xmm_obj_reference(mobj);
	kr = xmm_reply_allocate(k, (ipc_port_t) mobj, XMM_SVM_REPLY, &reply);
	if (kr != KERN_SUCCESS) {
		panic("m_svm_do_lock_request: xmm_reply_allocate: %d\n", kr);
	}
	K_LOCK_REQUEST(k, r->offset, PAGE_SIZE, should_clean, should_flush,
		       lock_value, reply);
}	

m_svm_lock_completed(reply, offset, length)
	xmm_reply_t reply;
	vm_offset_t offset;
	vm_size_t length;
{
	request_t r;
	xmm_obj_t mobj;

#ifdef	lint
	M_LOCK_COMPLETED(reply, offset, length);
#endif	lint
	/* XXX should make sanity checks */
	/* XXX should store r in reply */
	assert(reply->reply_to_type == XMM_SVM_REPLY);
	mobj = (xmm_obj_t) reply->reply_to;
	xmm_reply_deallocate(reply);
	assert(mobj->class == &msvm_class);
	r = m_svm_lookup_request(mobj, offset);
	if (r == REQUEST_NULL) {
		panic("m_svm_lock_completed: missing request");
	}
	if (--r->k_count == 0 && r->m_count == 0) {
		m_svm_satisfy_request(mobj, r, DATA_NONE);
	}
	xmm_obj_release(mobj);	/* reference obtained by do_lock_request */
	return KERN_SUCCESS;
}

k_svm_data_supply(mobj, offset, data, length, lock_value, precious, reply)
	xmm_obj_t mobj;
	vm_offset_t offset;
	vm_offset_t data;
	vm_size_t length;
	vm_prot_t lock_value;
	boolean_t precious;
	xmm_reply_t reply;
{
	request_t r;

#ifdef	lint
	K_DATA_SUPPLY(mobj, offset, data, length, lock_value, precious, reply);
#endif	lint
	assert(mobj->class == &msvm_class);
	/* make sanity checks */

	if (precious) {
		panic("k_svm_data_supply: precious");
	}
	if (reply != XMM_REPLY_NULL) {
		panic("k_svm_data_supply: reply");
	}

	/*
	 * XXX what do we do if this restricts access???
	 * XXX should probably do whatever lock_request does.
	 */
	if (lock_value & ~MOBJ->lock[atop(offset)]) {
		printf("XXX data_supply: lock=0x%x -> 0x%x\n",
		       MOBJ->lock[atop(offset)], lock_value);
	}

	MOBJ->lock[atop(offset)] = lock_value;

	r = m_svm_lookup_request(mobj, offset);
	if (r == REQUEST_NULL) {
		printf("how strange, data_supply for nothing!\n");
		return KERN_FAILURE;
	}
	if (--r->m_count == 0 && r->k_count == 0) {
		m_svm_satisfy_request(mobj, r, data);
	} else {
		printf("how strange, data provided but still other things\n");
		return KERN_FAILURE;
	}
	return KERN_SUCCESS;
}

k_svm_data_unavailable(mobj, offset, length)
	xmm_obj_t mobj;
	vm_offset_t offset;
	vm_size_t length;
{
	request_t r;

#ifdef	lint
	K_DATA_UNAVAILABLE(mobj, offset, length);
#endif	lint
	assert(mobj->class == &msvm_class);
	/* make sanity checks */

	/* XXX is this absolutely correct? */
	MOBJ->lock[atop(offset)] = VM_PROT_NONE;

	r = m_svm_lookup_request(mobj, offset);
	if (r == REQUEST_NULL) {
		printf("how strange, data_unavailable for nothing!\n");
		return KERN_FAILURE;
	}
	if (--r->m_count == 0 && r->k_count == 0) {
		m_svm_satisfy_request(mobj, r, DATA_UNAVAILABLE);
	}
	return KERN_SUCCESS;
}

k_svm_lock_request(mobj, offset, length, should_clean, should_flush,
		   lock_value, reply)
	xmm_obj_t mobj;
	vm_offset_t offset;
	vm_size_t length;
	boolean_t should_clean;
	boolean_t should_flush;
	vm_prot_t lock_value;
	xmm_reply_t reply;
{
	request_t r, r0;

#ifdef	lint
	K_LOCK_REQUEST(mobj, offset, length, should_clean, should_flush,
		       lock_value, reply);
#endif	lint
	assert(mobj->class == &msvm_class);
	dprintf("k_svm_lock_request!\n");

	if (length != PAGE_SIZE) {
		if (length > PAGE_SIZE) {
			panic("k_svm_lock_request: %d > PAGE_SIZE\n", length);
		}
		length = PAGE_SIZE;
	}
	if((unsigned long)atop(offset) >= MOBJ->num_pages) {
		m_svm_extend(mobj, atop(offset) + 1);
	}

	r0 = m_svm_lookup_request(mobj, offset);

	/*
	 * If we are not increasing lock value, flushing, or cleaning,
	 * then we set simply set lock value, without creating a request.
	 * However, we do need to see whether we can satisfy a kernel request.
	 */
	if (! (lock_value & ~MOBJ->lock[atop(offset)])
	    && ! should_clean && ! should_flush) {
		MOBJ->lock[atop(offset)] = lock_value;
		if (r0
		    && r0->is_kernel
		    && !(lock_value & r0->desired_access)
		    && r0->m_count > 0 && --r0->m_count == 0
		    && r0->k_count == 0) {
			m_svm_satisfy_kernel_request(mobj, r0, DATA_NONE);
		}
		return KERN_SUCCESS;
	}

	/*
	 * We need to submit a request. Create the request.
	 */
	dprintf("** lock_request: submitting request\n");
	r = (request_t) zalloc(xmm_svm_request_zone);
	r->who.reply = reply;
	r->is_kernel = FALSE;
	r->m_count = 0;
	r->k_count = 0;
	r->needs_data = FALSE;
	r->should_clean = should_clean;
	r->should_flush = should_flush;
	r->lock_value = lock_value;
	r->offset = offset;
	r->next_ne = 0;
	r->next_eq = 0;

	/*
	 * If there are no requests, then add new request and process it.
	 */
	if (! r0) {
		dprintf("- no reqs\n");
		(void) m_svm_add_request(mobj, r); /* will be true */
		(void) m_svm_process_pager_request(mobj, r);
		return KERN_SUCCESS;
	}

	/*
	 * If first request is pager request, then place new request
	 * after all pager requests, but before any kernel requests.
	 */
	if (! r0->is_kernel) {
		dprintf("- only pager reqs\n");
		while (r0->next_eq && ! r0->next_eq->is_kernel) {
			r0 = r0->next_eq;
		}
		r->next_eq = r0->next_eq;
		r0->next_eq = r;
		return KERN_SUCCESS;
	}

	/*
	 * First request is a kernel request.
	 * To avoid deadlock, pager requests have priority.
	 * Thus, if first request is a kernel, then all are.
	 * In this case, we place new request at the top
	 * (before all kernel requests) and process it immediately.
	 *
	 * XXXO
	 * This is slightly pessimal because we just ignore any
	 * request that the kernel request made to the other kernels.
	 */
	if (r0->is_kernel) {
		request_t *rp;
		for (rp = &MOBJ->request; r0 = *rp; rp = &r0->next_ne) {
			if (r0->offset == offset) {
				break;
			}
		}
		if (! r0) {
			printf("oops, oh my\n");
			return KERN_FAILURE;
		}
		*rp = r;
		r->next_ne = r0->next_ne;
		r->next_eq = r0;
		if (r0->m_count) {
			printf("This could get confusing\n");
		}
		r->m_count = r0->m_count;
		r->k_count = r0->k_count;
		r0->m_count = 0;
		r0->k_count = 0;	/* XXXO */
		(void) m_svm_process_pager_request(mobj, r);
		return KERN_SUCCESS;
	}
	return KERN_SUCCESS;
}

k_svm_data_error(mobj, offset, length, error_value)
	xmm_obj_t mobj;
	vm_offset_t offset;
	vm_size_t length;
	kern_return_t error_value;
{
	request_t r;

#ifdef	lint
	K_DATA_ERROR(mobj, offset, length, error_value);
#endif	lint
	assert(mobj->class == &msvm_class);
	/* make sanity checks */

	/* XXX certainly questionable! */
	MOBJ->lock[atop(offset)] = VM_PROT_NONE;

	r = m_svm_lookup_request(mobj, offset);
	if (r == REQUEST_NULL) {
		printf("how strange, data_unavailable for nothing!\n");
		return KERN_FAILURE;
	}
	if (--r->m_count == 0 && r->k_count == 0) {
		m_svm_satisfy_request(mobj, r, DATA_ERROR);
	}
	/* XXX should keep and return error_value */
	printf("k_svm_data_error: Gack(%d)!\n", error_value);
	return KERN_SUCCESS;
}

k_svm_set_ready(mobj, object_ready, may_cache, copy_strategy, use_old_pageout,
		memory_object_name, reply)
	xmm_obj_t mobj;
	boolean_t object_ready;
	boolean_t may_cache;
	memory_object_copy_strategy_t copy_strategy;
	boolean_t use_old_pageout;
	ipc_port_t memory_object_name;
	xmm_reply_t reply;
{
	xmm_obj_t kobj;

#ifdef	lint
	K_SET_READY(mobj, object_ready, may_cache, copy_strategy,
		    use_old_pageout, memory_object_name, reply);
#endif	lint
	assert(mobj->class == &msvm_class);
	MOBJ->may_cache = may_cache;

	/*
	 * Compute our copy strategy based on that of underlying pager.
	 *
	 * XXX
	 * Right now, we always use COPY_NONE, except if underlying pager
	 * specifies COPY_TEMPORARY, in which case we use COPY_DELAY.
	 * What this means is that we don't have any intelligent way
	 * of dealing with sharing, but that if it's a temporary object
	 * (either a vm internal object, created via memory_object_create,
	 * or an xmm internal object, created via norma_copy_create),
	 * then we don't expect any sharing, so we can use a lazy copy.
	 *
	 * THIS WILL BREAK IN ITS CURRENT FORM WHEN WE ENABLE VM_INHERIT_SHARE
	 */
	if (copy_strategy == MEMORY_OBJECT_COPY_TEMPORARY) {
		MOBJ->copy_strategy = MEMORY_OBJECT_COPY_TEMPORARY;
	} else {
		MOBJ->copy_strategy = MEMORY_OBJECT_COPY_NONE;
	}

	if (MOBJ->memory_object_name == IP_NULL) {
		MOBJ->memory_object_name = memory_object_name;
	} else {
		assert(MOBJ->memory_object_name == memory_object_name);
		ipc_port_release_send(memory_object_name);
	}
	if (object_ready) {
		MOBJ->state = MOBJ_STATE_READY;
	} else if (MOBJ->state == MOBJ_STATE_READY) {
		/* XXX What should we do here? */
		printf("k_svm_set_ready: ready -> not ready ?\n");
	}
	if (! use_old_pageout) {
		panic("k_svm_set_ready: use_old_pageout=FALSE!");
	}
	if (reply != XMM_REPLY_NULL) {
		panic("k_svm_set_ready: reply!\n");
	}

	/*
	 * If there are multiple kernels, then we had better be
	 * using MEMORY_OBJECT_COPY_NONE, at least until we get
	 * trickier about changing copy strategies.
	 */
	if (MOBJ->kobj_list && ((struct kobj *)MOBJ->kobj_list)->next &&
	    MOBJ->copy_strategy != MEMORY_OBJECT_COPY_NONE) {
		panic("losing big on multiple copies of temporary object");
	}

	/*
	 * Let all kernels know we're ready
	 */
	for (kobj = MOBJ->kobj_list; kobj; kobj = KOBJ->next) {
		assert(MOBJ->memory_object_name != IP_NULL);
		ipc_port_copy_send(MOBJ->memory_object_name);
		K_SET_READY(kobj, object_ready, may_cache, MOBJ->copy_strategy,
			    USE_OLD_PAGEOUT_TRUE, MOBJ->memory_object_name,
			    XMM_REPLY_NULL);
	}
	return KERN_SUCCESS;
}

k_svm_destroy(mobj, reason)
	xmm_obj_t mobj;
	kern_return_t reason;
{
#ifdef	lint
	K_DESTROY(mobj, reason);
#endif	lint
	assert(mobj->class == &msvm_class);
	printf("k_svm_destroy: Gack!\n");
}

/*
 * Place request at end of appropriate queue.
 * Return TRUE if first request in queue for this page.
 */
boolean_t
m_svm_add_request(mobj, r0)
	xmm_obj_t mobj;
	request_t r0;
{
	request_t r, *rp;

	assert(mobj->class == &msvm_class);
	dprintf("m_svm_add_request(0x%x, 0x%x)\n", mobj, r0);
	for (rp = &MOBJ->request; r = *rp; rp = &r->next_ne) {
	dprintf("m_svm_add_request: 0x%x 0x%x\n", r, r0);
		if (r->offset == r0->offset) {
			for (; r->next_eq; r = r->next_eq) {
				continue;
			}
			r->next_eq = r0;
			return FALSE;
		}
	}
	r0->next_ne = MOBJ->request;
	MOBJ->request = r0;
	return TRUE;
}

/*
 * Look for first request for given offset.
 * If we find such a request, move it to front of list
 * since we expect to remove it soon.
 */
request_t
m_svm_lookup_request(mobj, offset)
	xmm_obj_t mobj;
	vm_offset_t offset;
{
	request_t r, *rp;

	assert(mobj->class == &msvm_class);
	for (rp = &MOBJ->request; r = *rp; rp = &r->next_ne) {
		if (r->offset == offset) {
			*rp = r->next_ne;
			r->next_ne = MOBJ->request;
			MOBJ->request = r;
			return r;
		}
	}
	return REQUEST_NULL;
}

/*
 * Remove first request for given offset.
 * Return next request for same offset, if any.
 */
request_t
m_svm_remove_request(mobj, offset)
	xmm_obj_t mobj;
	vm_offset_t offset;
{
	request_t r, *rp;

	assert(mobj->class == &msvm_class);
	for (rp = &MOBJ->request; r = *rp; rp = &r->next_ne) {
		if (r->offset == offset) {
			if (r->next_eq) {
				r = r->next_eq;
				r->next_ne = (*rp)->next_ne;
				*rp = r;
				return r;
			} else {
				*rp = r->next_ne;
				return REQUEST_NULL;
			}
		}
	}
	printf("m_svm_remove_request: request not found!\n");
	return REQUEST_NULL;
}

/*
 * All the real work takes place in m_svm_process_request and
 * m_svm_satisfy_request.
 *
 * m_svm_process_request takes a request for a page that does not already have
 * outstanding requests and generates the appropriate K/M_ requests.
 * If, after generating all apropriate K/M_ requests, there are no outstanding
 * K/M_ requests (either because no K/M_ requests were required, or because
 * they were all satisfied by the time we check), we call
 * m_svm_satisfy_request.
 *
 * m_svm_satisfy_request takes a request for a page that has had its last
 * outstanding K/M_ request satisfied, and sends the appropriate K/M_ reply
 * to the entity (kernel or memory manager) that generated the request. If more
 * requests follow the request being satisfied, m_svm_satisfy_request calls
 * m_svm_process_request on the first such request.
 */

/*
 * This routine does not worry about lock[page]; m_svm_satisfy_request does.
 */
void
m_svm_process_kernel_request(mobj, r)
	xmm_obj_t mobj;
	request_t r;
{
	int page;
	xmm_obj_t kobj, k;

	assert(mobj->class == &msvm_class);
	page = atop(r->offset);
	kobj = r->who.kobj;

	/*
	 * If requesting kernel wants to write, we must flush and lock
	 * all kernels (either readers or a single writer).
	 */
	if (r->desired_access & VM_PROT_WRITE) {
		boolean_t writing = !! (MOBJ->prot[page] & VM_PROT_WRITE);
		MOBJ->prot[page] = VM_PROT_NONE;
		r->k_count++;
		for (k = MOBJ->kobj_list; k; k = K->next) {
			if (k == kobj || K->prot[page] == VM_PROT_NONE) {
				continue;
			}
			r->k_count++;
			K->prot[page] = VM_PROT_NONE;
			m_svm_do_lock_request(k, writing, TRUE, VM_PROT_ALL,
					      r, mobj);
			if (writing) {
				break;
			}
		}
		if (--r->k_count == 0 && r->m_count == 0) {
			m_svm_satisfy_kernel_request(mobj, r, DATA_NONE);
		}
		return;
	}

	/*
	 * If requesting kernel wants to read, but the page is being written,
	 * then we must clean and lock the writer.
	 */
	if (r->desired_access && (MOBJ->prot[page] & VM_PROT_WRITE)) {
		if (KOBJ->prot[page] & VM_PROT_WRITE) {
			/*
			 * What could the writer be doing asking us for read?
			 *
			 * This can happen if page was cleaned and flushed,
			 * or (more commonly?) cleaned and then paged out.
			 *
			 * Should we give this kernel read (more concurrency)
			 * or write (on the assumption that he will want
			 * to write again)?
			 *
			 * For now, we just give him read.
			 * We have to correct our notion of how this page is
			 * used. Note that there is no problem giving
			 * him either read or write, since there is nobody
			 * else to evict.
			 */
			KOBJ->prot[page] = r->desired_access;
			MOBJ->prot[page] = r->desired_access;
			m_svm_satisfy_kernel_request(mobj, r, DATA_NONE);
			return;
		}
		for (k = MOBJ->kobj_list; k; k = K->next) {
			if (K->prot[page] & VM_PROT_WRITE) {
				break;
			}
		}
		if (k == XMM_OBJ_NULL) {
			printf("x lost writer!\n");
			return;
		}
		MOBJ->prot[page] = VM_PROT_READ;
		K->prot[page] = VM_PROT_READ;
		r->k_count++;
		m_svm_do_lock_request(k, TRUE, FALSE, VM_PROT_WRITE, r, mobj);
		return;
	}

	/*
	 * No current kernel use conflicts with requesting kernel's
	 * desired use. Call m_svm_satisfy_kernel_request, which
	 * will handle any requests that need to be made of the pager.
	 */
	m_svm_satisfy_kernel_request(mobj, r, DATA_NONE);
}

void
m_svm_process_pager_request(mobj, r)
	xmm_obj_t mobj;
	request_t r;
{
	int page;
	xmm_obj_t k;

	assert(mobj->class == &msvm_class);
	page = atop(r->offset);

	/*
	 * Locking against non-write implies locking all access.
	 * Is this a bug, or universal truth?
	 * Beware: code below and elsewhere depends on this mapping.
	 */
	if (r->lock_value & ~VM_PROT_WRITE) {
		r->lock_value = VM_PROT_ALL;
	}

	/*
	 * XXX we can't yet represent
	 *	(lock=write but dirty)
	 * or
	 *	(lock=all but resident)
	 *
	 * Thus we force lock=write into clean,
	 * and lock=all into flush.
	 */
	if (r->lock_value == VM_PROT_WRITE) {
		r->should_clean = TRUE;
	} else if (r->lock_value) {
		r->should_clean = TRUE;
		r->should_flush = TRUE;
	}

	/*
	 * If we need to flush, or lock all access, then we must talk
	 * to all kernels.
	 */
	if (r->should_flush || r->lock_value == VM_PROT_ALL) {
		r->k_count++;
		MOBJ->prot[page] &= ~r->lock_value;
		for (k = MOBJ->kobj_list; k; k = K->next) {
			if (K->prot[page] == VM_PROT_NONE) {
				continue;
			}
			r->k_count++;
			K->prot[page] &= ~r->lock_value;
			m_svm_do_lock_request(k, r->should_clean,
					      r->should_flush,
					      r->lock_value, r, mobj);
		}
		if (--r->k_count == 0 && r->m_count == 0) {
			m_svm_satisfy_request(mobj, r, DATA_NONE);
		}
		return;
	}

	/*
	 * If we need to clean, or lock write access, and there is in fact
	 * a writer, then we must talk to that writer.
	 */
	if ((r->should_clean || r->lock_value == VM_PROT_WRITE)
	    && (MOBJ->prot[page] & VM_PROT_WRITE)) {
		MOBJ->prot[page] &= ~r->lock_value;
		for (k = MOBJ->kobj_list; k; k = K->next) {
			if (K->prot[page] & VM_PROT_WRITE) {
				break;
			}
		}
		if (k == XMM_OBJ_NULL) {
			printf("y lost writer!\n");
			return;
		}
		K->prot[page] &= ~r->lock_value;
		r->k_count++;
		m_svm_do_lock_request(k, r->should_clean, FALSE, r->lock_value,
				      r, mobj);
		return;
	}

	/*
	 * We didn't need to flush, clean, or lock.
	 */
	m_svm_satisfy_pager_request(mobj, r);
}

void
m_svm_process_request(mobj, r)
	xmm_obj_t mobj;
	request_t r;
{
	assert(mobj->class == &msvm_class);
	if (r->is_kernel) {
		m_svm_process_kernel_request(mobj, r);
	} else {
		m_svm_process_pager_request(mobj, r);
	}
}

void
m_svm_satisfy_kernel_request(mobj, r, data)
	xmm_obj_t mobj;
	request_t r;
	vm_offset_t data;
{
	xmm_obj_t kobj;
	request_t r_next;

	kobj = r->who.kobj;
	assert(mobj->class == &msvm_class);
	assert(r->is_kernel);
	assert(r->k_count == 0);
	assert(r->m_count == 0);

	/*
	 * If we need an unlock or data from the pager, make the request now.
	 */
	if ((MOBJ->lock[atop(r->offset)] & r->desired_access)
	    || (r->needs_data && data == DATA_NONE)) {
		if (data) {
			M_DATA_WRITE(mobj, mobj, r->offset, data,
				     PAGE_SIZE);
		}
		r->m_count++;
		if (r->needs_data) {
			M_DATA_REQUEST(mobj, mobj, r->offset, PAGE_SIZE,
				       r->desired_access);
		} else {
			M_DATA_UNLOCK(mobj, mobj, r->offset, PAGE_SIZE,
				      r->desired_access);
		}
		return;
	}

	/*
	 * We have everything we need. Satisfy the kernel request.
	 */
	if (! r->needs_data) {
		K_LOCK_REQUEST(r->who.kobj, r->offset, PAGE_SIZE, FALSE,
			       FALSE, r->desired_access ^ VM_PROT_ALL,
			       XMM_REPLY_NULL);
	} else if (data == DATA_UNAVAILABLE) {
		K_DATA_UNAVAILABLE(r->who.kobj, r->offset, PAGE_SIZE);
		r->desired_access = VM_PROT_ALL;	/* XXX */
	} else if (data == DATA_ERROR) {
		K_DATA_ERROR(r->who.kobj, r->offset, PAGE_SIZE,
			     KERN_FAILURE);
		/* XXX start killing object? */
	} else {
		K_DATA_SUPPLY(r->who.kobj, r->offset, data, PAGE_SIZE,
			      r->desired_access ^ VM_PROT_ALL, FALSE,
			      XMM_REPLY_NULL);
	}

	/*
	 * Update KOBJ->prot[] and MOBJ->prot[] values.
	 */
	MOBJ->prot[atop(r->offset)] = r->desired_access;
	KOBJ->prot[atop(r->offset)] = r->desired_access;

	/*
	 * Remove and free request.
	 */
	r_next = m_svm_remove_request(mobj, r->offset);
	zfree(xmm_svm_request_zone, (vm_offset_t) r);

	/*
	 * If there is another request, process it now.
	 */
	if (r_next) {
		m_svm_process_request(mobj, r_next);
	}
}

void
m_svm_satisfy_pager_request(mobj, r)
	xmm_obj_t mobj;
	request_t r;
{
	request_t r_next;

	assert(mobj->class == &msvm_class);
	assert(! r->is_kernel);
	assert(r->k_count == 0);
	assert(r->m_count == 0);

#if	USE_XMM_BUFFER
	/*
	 * Flush or clean any buffered data if necessary.
	 */
	if (r->should_flush || r->should_clean) {
		M_UNBUFFER_DATA(mobj, r->offset, PAGE_SIZE,
				r->should_clean, r->should_flush);
	}
#endif	USE_XMM_BUFFER

	/*
	 * We have everything we need. Satisfy the pager request.
	 */
	if (r->who.reply != XMM_REPLY_NULL) {
		M_LOCK_COMPLETED(r->who.reply, r->offset, PAGE_SIZE);
	}

	/*
	 * Update MOBJ->lock[] value.
	 */
	MOBJ->lock[atop(r->offset)] = r->lock_value;

	/*
	 * Remove and free request.
	 */
	r_next = m_svm_remove_request(mobj, r->offset);
	zfree(xmm_svm_request_zone, (vm_offset_t) r);

	/*
	 * If there is another request, process it now.
	 */
	if (r_next) {
		m_svm_process_request(mobj, r_next);
	}
}

void
m_svm_satisfy_request(mobj, r, data)
	xmm_obj_t mobj;
	request_t r;
	vm_offset_t data;
{
	assert(mobj->class == &msvm_class);
	if (r->is_kernel) {
		m_svm_satisfy_kernel_request(mobj, r, data);
	} else {
		m_svm_satisfy_pager_request(mobj, r);
	}
}

xmm_svm_init()
{
	xmm_svm_request_zone = zinit(sizeof(struct request), 512*1024,
				     sizeof(struct request), FALSE,
				     "xmm.svm.request");
}

#include <sys/varargs.h>

int xmm_svm_debug = 0;

/* VARARGS */
xmm_svm_dprintf(fmt, va_alist)
	char *fmt;
	va_dcl
{
	va_list	listp;

	if (xmm_svm_debug) {
		va_start(listp);
		printf(fmt, &listp);
		va_end(listp);
	}
}
