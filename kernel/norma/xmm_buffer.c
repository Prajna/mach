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
 * $Log:	xmm_buffer.c,v $
 * Revision 2.2  92/03/10  16:28:53  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:51:00  jsb]
 * 
 * Revision 2.1.2.5  92/02/21  11:25:30  jsb
 * 	Let xmm_kobj_link handle multiple init detection.
 * 	[92/02/18  08:04:06  jsb]
 * 
 * 	Explicitly provide name parameter to xmm_decl macro.
 * 	Changed debugging printf. Changed termination logic.
 * 	[92/02/16  15:18:10  jsb]
 * 
 * 	First real implementation.
 * 	[92/02/09  14:17:44  jsb]
 * 
 * Revision 2.1.2.3  92/01/21  21:53:42  jsb
 * 	De-linted. Supports new (dlb) memory object routines.
 * 	Supports arbitrary reply ports to lock_request, etc.
 * 	Converted mach_port_t (and port_t) to ipc_port_t.
 * 	[92/01/20  17:18:38  jsb]
 * 
 * Revision 2.1.2.2  92/01/03  16:38:41  jsb
 * 	First checkin.
 * 	[91/12/31  17:26:56  jsb]
 * 
 * Revision 2.1.2.1  92/01/03  08:57:47  jsb
 * 	First NORMA branch checkin.
 * 
 */
/*
 *	File:	xmm_buffer.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Xmm layer which buffers a small amount of data_written data.
 */

#ifdef	KERNEL
#include <kern/queue.h>
#include <norma/xmm_obj.h>
#include <mach/vm_param.h>
#include <vm/vm_fault.h>
#else	KERNEL
#include <xmm_obj.h>
#endif	KERNEL

#define	dprintf	xmm_buffer_dprintf

typedef struct buffer	*buffer_t;
#define	BUFFER_NULL	((buffer_t) 0)

struct buffer {
	queue_chain_t	lruq;
	queue_chain_t	mobjq;
	vm_map_copy_t	copy;
	xmm_obj_t	mobj;
	vm_offset_t	offset;
	buffer_t	next_free;
};

struct mobj {
	struct xmm_obj	obj;
	queue_head_t	buffers;
	boolean_t	ready;
};

#undef	KOBJ
#define	KOBJ	((struct mobj *) kobj)

#define	m_buffer_copy			m_interpose_copy
#define	m_buffer_data_unlock		m_interpose_data_unlock
#define	m_buffer_lock_completed		m_interpose_lock_completed
#define	m_buffer_supply_completed	m_interpose_supply_completed
#define	m_buffer_data_return		m_interpose_data_return
#define	m_buffer_change_completed	m_interpose_change_completed

#define	k_buffer_data_unavailable	k_interpose_data_unavailable
#define	k_buffer_get_attributes		k_interpose_get_attributes
#define	k_buffer_lock_request		k_interpose_lock_request
#define	k_buffer_data_error		k_interpose_data_error
#define	k_buffer_destroy		k_interpose_destroy
#define	k_buffer_data_supply		k_interpose_data_supply

xmm_decl(buffer, "buffer", sizeof(struct mobj));

#define	XMM_BUFFER_COUNT	16

struct buffer xmm_buffers[XMM_BUFFER_COUNT];
buffer_t xmm_buffer_free_list;
queue_head_t xmm_buffer_lru;

buffer_t
xmm_buffer_alloc()
{
	register buffer_t buffer;
	kern_return_t kr;
	xmm_obj_t mobj;

	/*
	 * First check the free list.
	 */
	buffer = xmm_buffer_free_list;
	if (buffer != BUFFER_NULL) {
		xmm_buffer_free_list = buffer->next_free;
		return buffer;
	}

	/*
	 * There's nothing on the free list, so take the oldest element
	 * from the lru queue, if any.
	 */
	if (queue_empty(&xmm_buffer_lru)) {
		/*
		 * This can happen if all the buffers are being written out.
		 */
		return BUFFER_NULL;
	}

	/*
	 * Remove buffer off of its obj queue as well, and write out its data.
	 *
	 * XXX
	 * Is it right for the caller to have to wait for someone else's
	 * data write to be processed?
	 */
	queue_remove_first(&xmm_buffer_lru, buffer, buffer_t, lruq);
	assert(buffer != BUFFER_NULL);
	mobj = buffer->mobj;
	queue_remove(&MOBJ->buffers, buffer, buffer_t, mobjq);
	kr = M_DATA_WRITE(mobj, mobj, buffer->offset,
			  (vm_offset_t) buffer->copy, PAGE_SIZE);
	if (kr != KERN_SUCCESS) {
		/*
		 * XXX
		 * What do we do here? (eternal buffering problem)
		 */
		printf("xmm_buffer_alloc: kr=%d/0x%x\n", kr, kr);
	}

	/*
	 * The buffer is now free. Return it.
	 */
#if 666
	buffer->mobj = (xmm_obj_t) 0xdd66dd66;
	buffer->offset = 0xcccccccc;
#endif
	return buffer;
}

void
xmm_buffer_free(buffer)
	register buffer_t buffer;
{
	buffer->next_free = xmm_buffer_free_list;
	xmm_buffer_free_list = buffer;
}

xmm_buffer_init()
{
	int i;
	buffer_t buffer;

	queue_init(&xmm_buffer_lru);
	for (i = 0; i < XMM_BUFFER_COUNT; i++) {
		buffer = &xmm_buffers[i];
		queue_init(&buffer->mobjq);
		queue_init(&buffer->lruq);
		xmm_buffer_free(buffer);
	}
}

kern_return_t
xmm_buffer_create(old_mobj, new_mobj)
	xmm_obj_t old_mobj;
	xmm_obj_t *new_mobj;
{
	xmm_obj_t mobj;
	kern_return_t kr;
	
	kr = xmm_obj_allocate(&buffer_class, old_mobj, &mobj);
	if (kr != KERN_SUCCESS) {
		return kr;
	}
	queue_init(&MOBJ->buffers);
	MOBJ->ready = FALSE;
	*new_mobj = mobj;
	return KERN_SUCCESS;
}

kern_return_t
m_buffer_init(mobj, k_kobj, pagesize, internal, size)
	xmm_obj_t	mobj;
	xmm_obj_t	k_kobj;
	vm_size_t	pagesize;
	boolean_t	internal;
	vm_size_t	size;
{
#ifdef	lint
	M_INIT(mobj, k_kobj, pagesize, internal, size);
#endif	lint
	assert(pagesize == PAGE_SIZE);
	xmm_kobj_link(mobj, k_kobj);
	M_INIT(mobj, mobj, pagesize, internal, size);
	return KERN_SUCCESS;
}

kern_return_t
m_buffer_terminate(mobj, kobj)
	xmm_obj_t	mobj;
	xmm_obj_t	kobj;
{
	buffer_t buffer;
	kern_return_t kr;
	vm_offset_t offset;
	vm_map_copy_t copy;

#ifdef	lint
	M_TERMINATE(mobj, kobj);
#endif	lint
	dprintf("xmm_buffer_terminate\n");
	while (! queue_empty(&MOBJ->buffers)) {
		queue_remove_first(&MOBJ->buffers, buffer, buffer_t, mobjq);
		assert(buffer != BUFFER_NULL);
		queue_remove(&xmm_buffer_lru, buffer, buffer_t, lruq);
		offset = buffer->offset;
		copy = buffer->copy;
		dprintf("dealloc 0x%x copy 0x%x\n", offset, copy);
		xmm_buffer_free(buffer);
		kr = M_DATA_WRITE(mobj, kobj, offset, (vm_offset_t) copy,
				  PAGE_SIZE);
		if (kr != KERN_SUCCESS) {
			/*
			 * XXX
			 * What do we do here? (eternal buffering problem)
			 */
			printf("xmm_buffer_terminate: kr=%d/0x%x\n", kr, kr);
		}
	}
	dprintf("terminate done\n");
	return M_TERMINATE(mobj, kobj);
}

void
m_buffer_deallocate(mobj)
	xmm_obj_t mobj;
{
}

buffer_t
xmm_buffer_lookup(mobj, offset)
	xmm_obj_t	mobj;
	vm_offset_t	offset;
{
	buffer_t buffer;

	/*
	 * Search through buffers associated with this mobj.
	 * There are typically very few buffers associated
	 * with any given object, so it's not worth having
	 * a hash table or anything tricky.
	 */
	queue_iterate(&MOBJ->buffers, buffer, buffer_t, mobjq) {
		if (buffer->offset == offset) {
			return buffer;
		}
	}
	return BUFFER_NULL;
}

vm_page_t
xmm_buffer_find_page(object, offset)
	vm_object_t object;
	vm_offset_t offset;
{
	vm_page_t m;
	
	/*
	 *	Try to find the page of data.
	 */
	vm_object_lock(object);
	vm_object_paging_begin(object);
	m = vm_page_lookup(object, offset);
	if ((m != VM_PAGE_NULL) && !m->busy && !m->fictitious &&
	    !m->absent && !m->error) {
	} else {
		vm_prot_t result_prot;
		vm_page_t top_page;
		kern_return_t kr;
		
		for (;;) {
			result_prot = VM_PROT_READ;
			kr = vm_fault_page(object, offset,
					   VM_PROT_READ, FALSE, FALSE,
					   &result_prot, &m, &top_page,
					   FALSE, (void (*)()) 0);
			if (kr == VM_FAULT_MEMORY_SHORTAGE) {
				VM_PAGE_WAIT((void (*)()) 0);
				vm_object_lock(object);
				vm_object_paging_begin(object);
				continue;
			}
			if (kr != VM_FAULT_SUCCESS) {
				/* XXX what about data_error? */
				vm_object_lock(object);
				vm_object_paging_begin(object);
				continue;
			}
			if (top_page != VM_PAGE_NULL) {
				vm_object_lock(object);
				VM_PAGE_FREE(top_page);
				vm_object_paging_end(object);
				vm_object_unlock(object);
			}
			break;
		}
	}
	assert(m);
	assert(! m->busy);
	vm_object_paging_end(object);
	vm_object_unlock(object);
	return m;
}

vm_map_copy_t
xmm_buffer_copy(old_copy)
	vm_map_copy_t old_copy;
{
	vm_map_copy_t new_copy;
	vm_page_t old_m, new_m;
	extern zone_t vm_map_copy_zone;

	dprintf("xmm_buffer_copy 0x%x type %d\n", old_copy, old_copy->type);

	/*
	 * Allocate a new copy object.
	 */
	new_copy = (vm_map_copy_t) zalloc(vm_map_copy_zone);
	if (new_copy == VM_MAP_COPY_NULL) {
		panic("xmm_buffer_copy: zalloc");
	}
	new_copy->type = VM_MAP_COPY_PAGE_LIST;
	new_copy->cpy_npages = 1;
	new_copy->offset = 0;
	new_copy->size = PAGE_SIZE;
	new_copy->cpy_cont = ((kern_return_t (*)()) 0);
	new_copy->cpy_cont_args = (char *) VM_MAP_COPYIN_ARGS_NULL;

	/*
	 * Allocate a new page and insert it into new copy object.
	 */
	new_m = vm_page_grab();
	if (new_m == VM_PAGE_NULL) {
		panic("xmm_buffer_copy: vm_page_grab");
	}
	new_copy->cpy_page_list[0] = new_m;

	/*
	 * Find old page.
	 */
	assert(old_copy->size == PAGE_SIZE);
	assert(old_copy->offset == 0);
	if (old_copy->type == VM_MAP_COPY_PAGE_LIST) {
		old_m = old_copy->cpy_page_list[0];
	} else {
		assert(old_copy->type == VM_MAP_COPY_OBJECT);
		old_m = xmm_buffer_find_page(old_copy->cpy_object, 0);
	}

	/*
	 * Copy old page into new, and return new copy object.
	 */
	pmap_copy_page(old_m->phys_addr, new_m->phys_addr);
	return new_copy;
}

kern_return_t
m_buffer_data_request(mobj, kobj, offset, length, desired_access)
	xmm_obj_t	mobj;
	xmm_obj_t	kobj;
	vm_offset_t	offset;
	vm_size_t	length;
	vm_prot_t	desired_access;
{
	buffer_t buffer;
	vm_map_copy_t copy;

#ifdef	lint
	M_DATA_REQUEST(mobj, kobj, offset, length, desired_access);
#endif	lint
	/*
	 * If this page was not buffered, then pass the data request through.
	 */
	buffer = xmm_buffer_lookup(mobj, offset);
	if (buffer == BUFFER_NULL) {
		return M_DATA_REQUEST(mobj, kobj, offset, length,
				      desired_access);
	}

	/*
	 * The page was buffered. Move it to the front of the lru queue.
	 */
	queue_remove(&xmm_buffer_lru, buffer, buffer_t, lruq);
	queue_enter(&xmm_buffer_lru, buffer, buffer_t, lruq);

	/*
	 * This copy is unfortunate and could be avoided.
	 */
	copy = xmm_buffer_copy(buffer->copy);
	
	/*
	 * Return data.
	 */
	return K_DATA_SUPPLY(kobj, offset, (vm_offset_t) copy, length,
			     VM_PROT_NONE, FALSE, XMM_REPLY_NULL);
}

/*
 * Write data through to memory manager;
 * discard any corresponding buffered data.
 */
kern_return_t
m_buffer_data_write(mobj, kobj, offset, data, length)
	xmm_obj_t	mobj;
	xmm_obj_t	kobj;
	vm_offset_t	offset;
	vm_offset_t	data;
	vm_size_t	length;
{
	buffer_t buffer;

#ifdef	lint
	M_DATA_WRITE(mobj, kobj, offset, data, length);
#endif	lint
	/*
	 * Find and deallocate old data, if any.
	 */
	buffer = xmm_buffer_lookup(mobj, offset);
	if (buffer != BUFFER_NULL) {
		vm_map_copy_t copy = buffer->copy;
		queue_remove(&MOBJ->buffers, buffer, buffer_t, mobjq);
		queue_remove(&xmm_buffer_lru, buffer, buffer_t, lruq);
		xmm_buffer_free(buffer);
		dprintf("discard copy=0x%x type=%d\n", copy, copy->type);
		vm_map_copy_discard(copy);
	}

	/*
	 * Write new data.
	 */
	return M_DATA_WRITE(mobj, kobj, offset, data, PAGE_SIZE);
}

/*
 * Buffer data to be written;
 * replace any preexisting corresponding buffered data.
 */
m_buffer_data_write_buffered(mobj, kobj, offset, data, length)
	xmm_obj_t	mobj;
	xmm_obj_t	kobj;
	vm_offset_t	offset;
	vm_offset_t	data;
	vm_size_t	length;
{
	buffer_t buffer;
	vm_map_copy_t copy = (vm_map_copy_t) data;

#ifdef	lint
	M_DATA_WRITE(mobj, kobj, offset, data, length);
#endif	lint
	/*
	 * Be assertive.
	 */
	assert(mobj == kobj);
	assert(length == PAGE_SIZE);
	assert(copy->type == VM_MAP_COPY_OBJECT ||
	       copy->type == VM_MAP_COPY_PAGE_LIST);
	assert(copy->offset == 0);
	assert(copy->size == PAGE_SIZE);

	/*
	 * Check to see whether we have old data for this page.
	 */
	buffer = xmm_buffer_lookup(mobj, offset);
	if (buffer != BUFFER_NULL) {
		/*
		 * Replace data in buffer.
		 */
		vm_map_copy_t old_copy = buffer->copy;
		buffer->copy = copy;
		dprintf("write_buffered: replace\n");

		/*
		 * Move buffer to head of lru queue.
		 */
		queue_remove(&xmm_buffer_lru, buffer, buffer_t, lruq);
		queue_enter(&xmm_buffer_lru, buffer, buffer_t, lruq);

		/*
		 * Discard data, and return.
		 */
		dprintf("replace copy=0x%x[type=%d] with copy=0x%x[%d]\n",
			 old_copy, old_copy->type,
			 copy, copy->type);
		vm_map_copy_discard(old_copy);
		return KERN_SUCCESS;
	}

	/*
	 * We don't have old data for this page, so allocate a new buffer
	 * and enter it in the queues.
	 */
	dprintf("write_buffered: new\n");
	buffer = xmm_buffer_alloc();
	assert(buffer != BUFFER_NULL);
	buffer->mobj = mobj;
	buffer->copy = copy;
	buffer->offset = offset;
	queue_enter(&MOBJ->buffers, buffer, buffer_t, mobjq);
	queue_enter(&xmm_buffer_lru, buffer, buffer_t, lruq);
	return KERN_SUCCESS;
}

kern_return_t
m_buffer_unbuffer_data(mobj, offset, length, should_clean, should_flush)
	xmm_obj_t mobj;
	vm_offset_t offset;
	vm_size_t length;
	boolean_t should_clean;
	boolean_t should_flush;
{
	buffer_t buffer;
	vm_map_copy_t copy;

	/*
	 * If we have no data for this page, return.
	 */
	buffer = xmm_buffer_lookup(mobj, offset);
	if (buffer == BUFFER_NULL) {
		return KERN_SUCCESS;
	}
	
	/*
	 * Dequeue and free buffer.
	 */
	copy = buffer->copy;
	queue_remove(&MOBJ->buffers, buffer, buffer_t, mobjq);
	queue_remove(&xmm_buffer_lru, buffer, buffer_t, lruq);
	xmm_buffer_free(buffer);

	/*
	 * Clean (write) or flush (deallocate) data.
	 */
	assert(should_clean || should_flush);
	dprintf("unbuffer: clean=%d flush=%d\n", should_clean, should_flush);
	if (should_clean) {
		return M_DATA_WRITE(mobj, mobj, offset, (vm_offset_t) copy,
				    PAGE_SIZE);
	} else {
		vm_map_copy_discard(copy);
		return KERN_SUCCESS;
	}
}

k_buffer_set_ready(kobj, object_ready, may_cache, copy_strategy,
		   use_old_pageout, memory_object_name, reply)
	xmm_obj_t kobj;
	boolean_t object_ready;
	boolean_t may_cache;
	memory_object_copy_strategy_t copy_strategy;
	boolean_t use_old_pageout;
	ipc_port_t memory_object_name;
	xmm_reply_t reply;
{
#ifdef	lint
	K_SET_READY(kobj, object_ready, may_cache, copy_strategy,
		    use_old_pageout, memory_object_name, reply);
#endif	lint
	if (object_ready) {
		KOBJ->ready = TRUE;
	}
	K_SET_READY(kobj, object_ready, may_cache, copy_strategy,
		    use_old_pageout, memory_object_name, reply);
	return KERN_SUCCESS;
}

/*
 * These won't work if we are interposing. Do we care?
 */

kern_return_t
M_BUFFERED_DATA_WRITE(mobj, kobj, offset, data, length)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_offset_t data;
	vm_size_t length;
{
	if (mobj->m_mobj->class == &buffer_class) {
		return m_buffer_data_write_buffered(mobj->m_mobj, kobj->m_kobj,
						    offset, data, length);
	} else {
		return M_DATA_WRITE(mobj, kobj, offset, data, length);
	}
}

kern_return_t
M_UNBUFFER_DATA(mobj, offset, length, should_clean, should_flush)
	xmm_obj_t mobj;
	boolean_t should_clean;
	boolean_t should_flush;
	vm_offset_t offset;
	vm_size_t length;
{
	if (mobj->m_mobj->class == &buffer_class) {
		return m_buffer_unbuffer_data(mobj->m_mobj, offset, length,
					      should_clean, should_flush);
	} else {
		return KERN_SUCCESS;
	}
}

#include <sys/varargs.h>

int xmm_buffer_debug = 0;

/* VARARGS */
xmm_buffer_dprintf(fmt, va_alist)
	char *fmt;
	va_dcl
{
	va_list	listp;

	if (xmm_buffer_debug) {
		va_start(listp);
		printf(fmt, &listp);
		va_end(listp);
	}
}
