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
 * $Log:	xmm_copy.c,v $
 * Revision 2.4  92/03/10  16:28:57  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:51:05  jsb]
 * 
 * Revision 2.3.3.3  92/02/21  11:25:37  jsb
 * 	Deallocate memory_object and memory_object_name ports.
 * 	[92/02/20  10:56:16  jsb]
 * 
 * 	Use newer xmm_decl macro with explicit name parameter.
 * 	[92/02/16  14:06:22  jsb]
 * 
 * 	Eliminated shadow obj creation, since we now let the vm system
 * 	handle copies for us.
 * 	[92/02/11  11:07:52  jsb]
 * 
 * 	Separate xmm_copy_create from norma_copy_create.
 * 	Replaced *_internal_memory_object garbage with call to
 * 	xmm_memory_manager_export. Use new MEMORY_OBJECT_COPY_TEMPORARY
 * 	strategy instead of MEMORY_OBJECT_COPY_DELAY, since we don't
 * 	need (or want) to see changes made to our object.
 * 	[92/02/11  11:05:04  jsb]
 * 
 * 	Use new xmm_decl, and new memory_object_name and deallocation protocol.
 * 	Removed *_internal_memory_object routines. Removed bogus data_write
 * 	implementation. Added shadow layer creation.
 * 	[92/02/09  12:49:43  jsb]
 * 
 * 	Obtain reference to map upon creation; release on termination.
 * 	[92/01/22  10:35:58  jsb]
 * 
 * Revision 2.3.3.1  92/01/21  21:53:50  jsb
 * 	De-linted. Supports new (dlb) memory object routines.
 * 	Supports arbitrary reply ports to lock_request, etc.
 * 	Converted mach_port_t (and port_t) to ipc_port_t.
 * 	[92/01/20  17:19:14  jsb]
 * 
 * 	Fixes from OSF.
 * 	[92/01/17  14:13:56  jsb]
 * 
 * Revision 2.3.1.1  92/01/15  12:13:44  jeffreyh
 * 	Deallocate memory object name port on termination. (dlb)
 * 
 * Revision 2.3  91/11/15  14:09:53  rpd
 * 	Use ipc_port_make_send in norma_copy_create for returned memory_object.
 * 	[91/09/23  09:09:25  jsb]
 * 
 * Revision 2.2  91/08/28  11:16:22  jsb
 * 	In m_copy_data_request: removed dead code, and added missing
 * 	is_continuation parameter to vm_map_copyin_page_list.
 * 	[91/08/16  14:25:04  jsb]
 * 
 * 	First checkin.
 * 	[91/08/15  13:03:06  jsb]
 * 
 */
/*
 *	File:	norma/xmm_copy.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
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

struct mobj {
	struct xmm_obj	obj;
	vm_map_t	map;
	vm_offset_t	start;
	vm_size_t	size;
	ipc_port_t	memory_object_name;
	ipc_port_t	memory_object;
};

#undef  KOBJ
#define KOBJ    ((struct mobj *) kobj)

#define	m_copy_copy			m_invalid_copy
#define	m_copy_data_unlock		m_invalid_data_unlock
#define	m_copy_data_write		m_invalid_data_write
#define	m_copy_lock_completed		m_invalid_lock_completed
#define	m_copy_supply_completed		m_invalid_supply_completed
#define	m_copy_data_return		m_invalid_data_return
#define	m_copy_change_completed		m_invalid_change_completed

#define	k_copy_data_unavailable		k_invalid_data_unavailable
#define	k_copy_get_attributes		k_invalid_get_attributes
#define	k_copy_lock_request		k_invalid_lock_request
#define	k_copy_data_error		k_invalid_data_error
#define	k_copy_set_ready		k_invalid_set_ready
#define	k_copy_destroy			k_invalid_destroy
#define	k_copy_data_supply		k_invalid_data_supply

xmm_decl(copy, "copy", sizeof(struct mobj));

kern_return_t
xmm_copy_create(map, start, size, new_mobj)
	vm_map_t map;
	vm_offset_t start;
	vm_size_t size;
	xmm_obj_t *new_mobj;
{
	xmm_obj_t mobj;
	kern_return_t kr;

	assert(page_aligned(start));
	assert(page_aligned(size));
	kr = xmm_obj_allocate(&copy_class, XMM_OBJ_NULL, &mobj);
	if (kr != KERN_SUCCESS) {
		return kr;
	}
	vm_map_reference(map);
	MOBJ->map = map;
	MOBJ->start = start;
	MOBJ->size = size;
	MOBJ->memory_object_name = ipc_port_alloc_kernel();
	if (MOBJ->memory_object_name == IP_NULL) {
		panic("add_internal_memory_object: ipc_port_alloc_kernel");
	}
	MOBJ->memory_object = IP_NULL;
	*new_mobj = mobj;
	return KERN_SUCCESS;
}

norma_copy_create(map, start, size, memory_object_p)
	vm_map_t map;
	vm_offset_t start;
	vm_size_t size;
	ipc_port_t *memory_object_p;
{
	xmm_obj_t mobj;
	kern_return_t kr;
	ipc_port_t xmm_memory_manager_export();

	/*
	 * Create a read-only, xmm-internal memory manager for map.
	 */
	kr = xmm_copy_create(map, start, size, &mobj);
	if (kr != KERN_SUCCESS) {
		return kr;
	}

	/*
	 * Create an svm stack and an xmm object, and save memory object.
	 */
	MOBJ->memory_object = xmm_memory_manager_export(mobj);

	/*
	 * Return memory object.
	 */
	*memory_object_p = MOBJ->memory_object;
	return KERN_SUCCESS;
}

m_copy_init(mobj, k_kobj, pagesize, internal, size)
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
	return K_SET_READY(kobj, OBJECT_READY_TRUE, MAY_CACHE_FALSE,
			   MEMORY_OBJECT_COPY_TEMPORARY, USE_OLD_PAGEOUT_TRUE,
			   ipc_port_make_send(MOBJ->memory_object_name),
			   XMM_REPLY_NULL);
}

m_copy_terminate(mobj, kobj)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
{
	return KERN_SUCCESS;
}

void
m_copy_deallocate(mobj)
	xmm_obj_t mobj;
{
	vm_map_deallocate(MOBJ->map);
	ipc_port_dealloc_kernel(MOBJ->memory_object_name);
	if (MOBJ->memory_object != IP_NULL) {
		ipc_port_dealloc_kernel(MOBJ->memory_object);
	}
}

m_copy_data_request(mobj, kobj, offset, length, desired_access)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
	extern zone_t vm_map_copy_zone;
	vm_map_copy_t copy;
	kern_return_t kr;
	
#ifdef	lint
	M_DATA_REQUEST(mobj, kobj, offset, length, desired_access);
#endif	lint
	assert(page_aligned(offset));
	assert(page_aligned(length));
	assert(length == PAGE_SIZE);
	assert(offset + length <= MOBJ->size);

	kr = vm_map_copyin_page_list(MOBJ->map,
				     MOBJ->start + offset,
				     PAGE_SIZE,
				     FALSE,/* src_destroy */
				     TRUE,/* steal pages */
				     &copy,
				     FALSE/* is continuation */);
	if (kr) {
		fret("xmm_copy_data_request 0x%x 0x%x 0x%x: %x\n",
		     MOBJ->start, offset, length, kr);
		return K_DATA_ERROR(kobj, offset, length, kr);
	}
	/* XXX should only return appropriate access */
	return K_DATA_SUPPLY(kobj, offset, (vm_offset_t) copy, length,
			     VM_PROT_NONE, FALSE, XMM_REPLY_NULL);
}
