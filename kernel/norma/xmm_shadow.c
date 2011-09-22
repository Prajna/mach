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
 * $Log:	xmm_shadow.c,v $
 * Revision 2.2  92/03/10  16:29:43  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:51:50  jsb]
 * 
 * Revision 2.1.3.3  92/02/21  11:28:09  jsb
 * 	Explicitly provide name parameter to xmm_decl macro.
 * 	[92/02/16  14:24:15  jsb]
 * 
 * 	Disabled; use the vm system to manage copies.
 * 	[92/02/11  11:08:14  jsb]
 * 
 * 	Removed *_internal_memory_object garbage.
 * 	[92/02/11  11:00:23  jsb]
 * 
 * 	First real implementation. Moved internal_memory_object routines here.
 * 	[92/02/09  14:19:11  jsb]
 * 
 */
/*
 *	File:	norma/xmm_shadow.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1992
 *
 *	Xmm layer to handle writes to a read-only object.
 *	Interim solution until we can do lazy copies of svm memory.
 */

#if	0

#include <norma/xmm_obj.h>
#include <mach/vm_param.h>
#include <ipc/ipc_space.h>
#include <ipc/ipc_port.h>

struct mobj {
	struct xmm_obj	obj;
	ipc_port_t	memory_object;	/* must be second field */
	xmm_obj_t	next;		/* must be third field */
	vm_size_t	size;
	boolean_t	*present;
	vm_map_copy_t	*page;		/* XXX should create internal pager */
};

#undef  KOBJ
#define KOBJ    ((struct mobj *) kobj)

kern_return_t m_shadow_terminate();
kern_return_t m_shadow_data_request();
kern_return_t m_shadow_data_write();

xmm_decl(shadow_class,
	/* m_init		*/	m_interpose_init,
	/* m_terminate		*/	m_shadow_terminate,
	/* m_copy		*/	m_interpose_copy,
	/* m_data_request	*/	m_shadow_data_request,
	/* m_data_unlock	*/	m_interpose_data_unlock,
	/* m_data_write		*/	m_shadow_data_write,
	/* m_lock_completed	*/	m_interpose_lock_completed,
	/* m_supply_completed	*/	m_interpose_supply_completed,
	/* m_data_return	*/	m_interpose_data_return,
	/* m_change_completed	*/	m_interpose_change_completed,

	/* k_data_unavailable	*/	k_interpose_data_unavailable,
	/* k_get_attributes	*/	k_interpose_get_attributes,
	/* k_lock_request	*/	k_interpose_lock_request,
	/* k_data_error		*/	k_interpose_data_error,
	/* k_set_ready		*/	k_interpose_set_ready,
	/* k_destroy		*/	k_interpose_destroy,
	/* k_data_supply	*/	k_interpose_data_supply,

	/* name			*/	"shadow",
	/* size			*/	sizeof(struct mobj)
);

kern_return_t
xmm_shadow_create(old_mobj, size, new_mobj)
	xmm_obj_t old_mobj;
	vm_size_t size;
	xmm_obj_t *new_mobj;
{
	xmm_obj_t mobj;
	kern_return_t kr;

	assert(page_aligned(size));
	kr = xmm_obj_allocate(&shadow_class, old_mobj, &mobj);
	if (kr != KERN_SUCCESS) {
		return kr;
	}
	MOBJ->size = size;
	MOBJ->present = (boolean_t *) kalloc(atop(size) * sizeof(boolean_t));
	bzero(MOBJ->present, atop(size) * sizeof(boolean_t));
	MOBJ->page = (vm_map_copy_t *)
	    kalloc(atop(MOBJ->size) * sizeof(vm_map_copy_t));
	bzero(MOBJ->page, atop(MOBJ->size) * sizeof(vm_map_copy_t));
	*new_mobj = mobj;
	return KERN_SUCCESS;
}

m_shadow_terminate(mobj, kobj, memory_object_name)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	ipc_port_t memory_object_name;
{
	unsigned long page;
	kern_return_t kr;

	for (page = 0; page < atop(MOBJ->size); page++) {
		if (MOBJ->page[page]) {
			vm_map_copy_discard(MOBJ->page[page]);
		}
	}
	kfree(MOBJ->present, atop(MOBJ->size) * sizeof(boolean_t));
	remove_internal_memory_object(mobj);
	kr = M_TERMINATE(mobj, mobj, memory_object_name);
	xmm_obj_deallocate(mobj);
	return kr;
}

m_shadow_data_request(mobj, kobj, offset, length, desired_access)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
	extern zone_t vm_map_copy_zone;
	vm_map_copy_t copy;
	unsigned long page;
	
	assert(page_aligned(offset));
	assert(page_aligned(length));
	assert(length == PAGE_SIZE);
	assert(offset + length <= MOBJ->size);

	/*
	 * If the page is not shadowed, pass request down to source obj.
	 */
	page = atop(offset);
	if (! MOBJ->present[page]) {
		return M_DATA_REQUEST(mobj, kobj, offset, length,
				      desired_access);
	}

	/*
	 * Copy the shadowed page.
	 * MOBJ->page[page] is a object flavor copy object.
	 * We need to keep a copy here (for multiple pageins),
	 * and we need a page list copy object anyway.
	 */
	if (MOBJ->page[page] == VM_MAP_COPY_NULL) {
		panic("m_shadow_data_request: absent page");
	}
	copy = (vm_map_copy_t) xmm_buffer_copy(MOBJ->page[page]);

	/*
	 * Provide the page list copy object containing the page
	 * to the kernel.
	 */
	return K_DATA_SUPPLY(kobj, offset, (vm_offset_t) copy, length,
			     VM_PROT_NONE, FALSE, XMM_REPLY_NULL);
}

kern_return_t
m_shadow_data_write(mobj, kobj, offset, data, length)
	xmm_obj_t	mobj;
	xmm_obj_t	kobj;
	vm_offset_t	offset;
	vm_offset_t	data;
	vm_size_t	length;
{
	unsigned long page;
	vm_map_copy_t copy = (vm_map_copy_t) data;

	assert(page_aligned(offset));
	assert(page_aligned(length));
	assert(length == PAGE_SIZE);
	assert(offset + length <= MOBJ->size);

	page = atop(offset);
	if (MOBJ->present[page]) {
		assert(MOBJ->page[page]);
		vm_map_copy_discard(MOBJ->page[page]);
	}
	MOBJ->present[page] = TRUE;
	MOBJ->page[page] = copy;
	return KERN_SUCCESS;
}

/* ---------------------------------------------------------------------- */

#if	666
/*
 * This should live somewhere else
 */
xmm_obj_t	internal_mobj_list;

boolean_t
is_internal_memory_object(memory_object, new_mobj)
	ipc_port_t memory_object;
	xmm_obj_t *new_mobj;
{
	xmm_obj_t mobj;

	for (mobj = internal_mobj_list; mobj; mobj = MOBJ->next) {
		if (MOBJ->memory_object == memory_object) {
			*new_mobj = mobj;
			return TRUE;
		}
	}
	return FALSE;
}

add_internal_memory_object(mobj, memory_object_p)
	xmm_obj_t mobj;
	ipc_port_t *memory_object_p;
{
	MOBJ->memory_object = ipc_port_alloc_kernel();
	if (MOBJ->memory_object == IP_NULL) {
		panic("add_internal_memory_object: ipc_port_alloc_kernel");
	}
	MOBJ->memory_object = ipc_port_make_send(MOBJ->memory_object);

	MOBJ->next = internal_mobj_list;
	internal_mobj_list = mobj;

	*memory_object_p = MOBJ->memory_object;
}

#define MP    ((struct mobj *) (*mp))

remove_internal_memory_object(mobj)
	xmm_obj_t mobj;
{
	xmm_obj_t *mp;

	for (mp = &internal_mobj_list; *mp; mp = &MP->next) {
		if (*mp == mobj) {
			/* XXX deallocate MOBJ->memory_object? */
			*mp = MOBJ->next;
			return;
		}
	}
	assert(0);
}
#endif	666

#endif	0
