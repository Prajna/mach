/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
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
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	vm_debug.c,v $
 * Revision 2.13  93/01/14  18:00:49  danner
 * 	64bit cleanup.
 * 	[92/12/01            af]
 * 
 * Revision 2.12  92/08/03  18:00:06  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.11  92/05/21  17:25:28  jfriedl
 * 	Added stuff to quiet gcc warnings.
 * 	Also removed unused var 'kr' in mach_vm_region_info().
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.10  92/02/25  14:17:42  elf
 * 	Added line to mach_vm_region_info to reflect sharing.
 * 	Note: vm_region_info needs to be updated. 
 * 	[92/02/24            elf]
 * 
 * Revision 2.9  92/02/23  19:51:55  elf
 * 	Eliminate keep_wired argument from vm_map_copyin().
 * 	[92/02/21  10:14:21  dlb]
 * 
 * 	No more sharing maps.
 * 	[92/01/07  11:02:46  dlb]
 * 
 * Revision 2.7.8.1  92/02/18  19:20:54  jeffreyh
 * 	Include thread.h when compiling with VM_OBJECT_DEBUG
 * 	[91/09/09            bernadat]
 * 
 * Revision 2.8  92/01/14  16:47:39  rpd
 * 	Changed host_virtual_physical_table_info and
 * 	mach_vm_object_pages for CountInOut.
 * 	[92/01/14            rpd]
 * 
 * 	Removed <mach_debug/page_info.h>.
 * 	[92/01/08            rpd]
 * 	Fixed mach_vm_region_info for submaps and share maps.
 * 	[92/01/02            rpd]
 * 
 * 	Replaced the old mach_vm_region_info with new mach_vm_region_info,
 * 	mach_vm_object_info, mach_vm_object_pages calls.
 * 	Removed vm_mapped_pages_info.
 * 	[91/12/30            rpd]
 * 
 * Revision 2.7  91/08/28  11:17:57  jsb
 * 	single_use --> use_old_pageout in vm_object.
 * 	[91/08/05  17:44:32  dlb]
 * 
 * Revision 2.6  91/05/14  17:48:16  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:57:39  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:31:12  mrt]
 * 
 * Revision 2.4  91/01/08  16:44:26  rpd
 * 	Added host_virtual_physical_table_info.
 * 	[91/01/02            rpd]
 * 
 * Revision 2.3  90/10/12  13:05:13  rpd
 * 	Removed copy_on_write field.
 * 	[90/10/08            rpd]
 * 
 * Revision 2.2  90/06/02  15:10:26  rpd
 * 	Moved vm_mapped_pages_info here from vm/vm_map.c.
 * 	[90/05/31            rpd]
 * 
 * 	Created.
 * 	[90/04/20            rpd]
 * 
 */
/*
 *	File:	vm/vm_debug.c.
 *	Author:	Rich Draves
 *	Date:	March, 1990
 *
 *	Exported kernel calls.  See mach_debug/mach_debug.defs.
 */

#include <kern/thread.h>
#include <mach/kern_return.h>
#include <mach/machine/vm_types.h>
#include <mach/memory_object.h>
#include <mach/vm_prot.h>
#include <mach/vm_inherit.h>
#include <mach/vm_param.h>
#include <mach_debug/vm_info.h>
#include <mach_debug/hash_info.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <vm/vm_object.h>
#include <kern/task.h>
#include <kern/host.h>
#include <ipc/ipc_port.h>



/*
 *	Routine:	vm_object_real_name
 *	Purpose:
 *		Convert a VM object to a name port.
 *	Conditions:
 *		Takes object and port locks.
 *	Returns:
 *		A naked send right for the object's name port,
 *		or IP_NULL if the object or its name port is null.
 */

ipc_port_t
vm_object_real_name(object)
	vm_object_t object;
{
	ipc_port_t port = IP_NULL;

	if (object != VM_OBJECT_NULL) {
		vm_object_lock(object);
		if (object->pager_name != IP_NULL)
			port = ipc_port_make_send(object->pager_name);
		vm_object_unlock(object);
	}

	return port;
}

/*
 *	Routine:	mach_vm_region_info [kernel call]
 *	Purpose:
 *		Retrieve information about a VM region,
 *		including info about the object chain.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Retrieve region/object info.
 *		KERN_INVALID_TASK	The map is null.
 *		KERN_NO_SPACE		There is no entry at/after the address.
 */

kern_return_t
mach_vm_region_info(map, address, regionp, portp)
	vm_map_t map;
	vm_offset_t address;
	vm_region_info_t *regionp;
	ipc_port_t *portp;
{
	vm_map_t cmap;		/* current map in traversal */
	vm_map_t nmap;		/* next map to look at */
	vm_map_entry_t entry;	/* entry in current map */
	vm_object_t object;

	if (map == VM_MAP_NULL)
		return KERN_INVALID_TASK;

	/* find the entry containing (or following) the address */

	vm_map_lock_read(map);
	for (cmap = map;;) {
		/* cmap is read-locked */

		if (!vm_map_lookup_entry(cmap, address, &entry)) {
			entry = entry->vme_next;
			if (entry == vm_map_to_entry(cmap)) {
				if (map == cmap) {
					vm_map_unlock_read(cmap);
					return KERN_NO_SPACE;
				}

				/* back out to top-level & skip this submap */

				address = vm_map_max(cmap);
				vm_map_unlock_read(cmap);
				vm_map_lock_read(map);
				cmap = map;
				continue;
			}
		}

		if (entry->is_sub_map) {
			/* move down to the sub map */

			nmap = entry->object.sub_map;
			vm_map_lock_read(nmap);
			vm_map_unlock_read(cmap);
			cmap = nmap;
			continue;
		} else {
			break;
		}
		/*NOTREACHED*/
	}


	assert(entry->vme_start < entry->vme_end);

	regionp->vri_start = entry->vme_start;
	regionp->vri_end = entry->vme_end;

	/* attributes from the real entry */

	regionp->vri_protection = entry->protection;
	regionp->vri_max_protection = entry->max_protection;
	regionp->vri_inheritance = entry->inheritance;
	regionp->vri_wired_count = entry->wired_count;
	regionp->vri_user_wired_count = entry->user_wired_count;

	object = entry->object.vm_object;
	*portp = vm_object_real_name(object);
	regionp->vri_object = (vm_offset_t) object;
	regionp->vri_offset = entry->offset;
	regionp->vri_needs_copy = entry->needs_copy;

	regionp->vri_sharing = entry->is_shared;

	vm_map_unlock_read(cmap);
	return KERN_SUCCESS;
}

/*
 *	Routine:	mach_vm_object_info [kernel call]
 *	Purpose:
 *		Retrieve information about a VM object.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Retrieved object info.
 *		KERN_INVALID_ARGUMENT	The object is null.
 */

kern_return_t
mach_vm_object_info(object, infop, shadowp, copyp)
	vm_object_t object;
	vm_object_info_t *infop;
	ipc_port_t *shadowp;
	ipc_port_t *copyp;
{
	vm_object_info_t info;
	vm_object_info_state_t state;
	ipc_port_t shadow, copy;

	if (object == VM_OBJECT_NULL)
		return KERN_INVALID_ARGUMENT;

	/*
	 *	Because of lock-ordering/deadlock considerations,
	 *	we can't use vm_object_real_name for the copy object.
	 */

    retry:
	vm_object_lock(object);
	copy = IP_NULL;
	if (object->copy != VM_OBJECT_NULL) {
		if (!vm_object_lock_try(object->copy)) {
			vm_object_unlock(object);
			simple_lock_pause();	/* wait a bit */
			goto retry;
		}

		if (object->copy->pager_name != IP_NULL)
			copy = ipc_port_make_send(object->copy->pager_name);
		vm_object_unlock(object->copy);
	}
	shadow = vm_object_real_name(object->shadow);

	info.voi_object = (vm_offset_t) object;
	info.voi_pagesize = PAGE_SIZE;
	info.voi_size = object->size;
	info.voi_ref_count = object->ref_count;
	info.voi_resident_page_count = object->resident_page_count;
	info.voi_absent_count = object->absent_count;
	info.voi_copy = (vm_offset_t) object->copy;
	info.voi_shadow = (vm_offset_t) object->shadow;
	info.voi_shadow_offset = object->shadow_offset;
	info.voi_paging_offset = object->paging_offset;
	info.voi_copy_strategy = object->copy_strategy;
	info.voi_last_alloc = object->last_alloc;
	info.voi_paging_in_progress = object->paging_in_progress;

	state = 0;
	if (object->pager_created)
		state |= VOI_STATE_PAGER_CREATED;
	if (object->pager_initialized)
		state |= VOI_STATE_PAGER_INITIALIZED;
	if (object->pager_ready)
		state |= VOI_STATE_PAGER_READY;
	if (object->can_persist)
		state |= VOI_STATE_CAN_PERSIST;
	if (object->internal)
		state |= VOI_STATE_INTERNAL;
	if (object->temporary)
		state |= VOI_STATE_TEMPORARY;
	if (object->alive)
		state |= VOI_STATE_ALIVE;
	if (object->lock_in_progress)
		state |= VOI_STATE_LOCK_IN_PROGRESS;
	if (object->lock_restart)
		state |= VOI_STATE_LOCK_RESTART;
	if (object->use_old_pageout)
		state |= VOI_STATE_USE_OLD_PAGEOUT;
	info.voi_state = state;
	vm_object_unlock(object);

	*infop = info;
	*shadowp = shadow;
	*copyp = copy;
	return KERN_SUCCESS;
}

#define VPI_STATE_NODATA	(VPI_STATE_BUSY|VPI_STATE_FICTITIOUS| \
				 VPI_STATE_PRIVATE|VPI_STATE_ABSENT)

/*
 *	Routine:	mach_vm_object_pages [kernel call]
 *	Purpose:
 *		Retrieve information about the pages in a VM object.
 *	Conditions:
 *		Nothing locked.  Obeys CountInOut protocol.
 *	Returns:
 *		KERN_SUCCESS		Retrieved object info.
 *		KERN_INVALID_ARGUMENT	The object is null.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory.
 */

kern_return_t
mach_vm_object_pages(object, pagesp, countp)
	vm_object_t object;
	vm_page_info_array_t *pagesp;
	natural_t *countp;
{
	vm_size_t size;
	vm_offset_t addr;
	vm_page_info_t *pages;
	unsigned int potential, actual, count;
	vm_page_t p;
	kern_return_t kr;

	if (object == VM_OBJECT_NULL)
		return KERN_INVALID_ARGUMENT;

	/* start with in-line memory */

	pages = *pagesp;
	potential = *countp;

	for (size = 0;;) {
		vm_object_lock(object);
		actual = object->resident_page_count;
		if (actual <= potential)
			break;
		vm_object_unlock(object);

		if (pages != *pagesp)
			kmem_free(ipc_kernel_map, addr, size);

		size = round_page(actual * sizeof *pages);
		kr = kmem_alloc(ipc_kernel_map, &addr, size);
		if (kr != KERN_SUCCESS)
			return kr;

		pages = (vm_page_info_t *) addr;
		potential = size/sizeof *pages;
	}
	/* object is locked, we have enough wired memory */

	count = 0;
	queue_iterate(&object->memq, p, vm_page_t, listq) {
		vm_page_info_t *info = &pages[count++];
		vm_page_info_state_t state = 0;

		info->vpi_offset = p->offset;
		info->vpi_phys_addr = p->phys_addr;
		info->vpi_wire_count = p->wire_count;
		info->vpi_page_lock = p->page_lock;
		info->vpi_unlock_request = p->unlock_request;

		if (p->busy)
			state |= VPI_STATE_BUSY;
		if (p->wanted)
			state |= VPI_STATE_WANTED;
		if (p->tabled)
			state |= VPI_STATE_TABLED;
		if (p->fictitious)
			state |= VPI_STATE_FICTITIOUS;
		if (p->private)
			state |= VPI_STATE_PRIVATE;
		if (p->absent)
			state |= VPI_STATE_ABSENT;
		if (p->error)
			state |= VPI_STATE_ERROR;
		if (p->dirty)
			state |= VPI_STATE_DIRTY;
		if (p->precious)
			state |= VPI_STATE_PRECIOUS;
		if (p->overwriting)
			state |= VPI_STATE_OVERWRITING;

		if (((state & (VPI_STATE_NODATA|VPI_STATE_DIRTY)) == 0) &&
		    pmap_is_modified(p->phys_addr)) {
			state |= VPI_STATE_DIRTY;
			p->dirty = TRUE;
		}

		vm_page_lock_queues();
		if (p->inactive)
			state |= VPI_STATE_INACTIVE;
		if (p->active)
			state |= VPI_STATE_ACTIVE;
		if (p->laundry)
			state |= VPI_STATE_LAUNDRY;
		if (p->free)
			state |= VPI_STATE_FREE;
		if (p->reference)
			state |= VPI_STATE_REFERENCE;

		if (((state & (VPI_STATE_NODATA|VPI_STATE_REFERENCE)) == 0) &&
		    pmap_is_referenced(p->phys_addr)) {
			state |= VPI_STATE_REFERENCE;
			p->reference = TRUE;
		}
		vm_page_unlock_queues();

		info->vpi_state = state;
	}

	if (object->resident_page_count != count)
		panic("mach_vm_object_pages");
	vm_object_unlock(object);

	if (pages == *pagesp) {
		/* data fit in-line; nothing to deallocate */

		*countp = actual;
	} else if (actual == 0) {
		kmem_free(ipc_kernel_map, addr, size);

		*countp = 0;
	} else {
		vm_size_t size_used, rsize_used;
		vm_map_copy_t copy;

		/* kmem_alloc doesn't zero memory */

		size_used = actual * sizeof *pages;
		rsize_used = round_page(size_used);

		if (rsize_used != size)
			kmem_free(ipc_kernel_map,
				  addr + rsize_used, size - rsize_used);

		if (size_used != rsize_used)
			bzero((char *) (addr + size_used),
			      rsize_used - size_used);

		kr = vm_map_copyin(ipc_kernel_map, addr, rsize_used,
				   TRUE, &copy);
		assert(kr == KERN_SUCCESS);

		*pagesp = (vm_page_info_t *) copy;
		*countp = actual;
	}

	return KERN_SUCCESS;
}

/*
 *	Routine:	host_virtual_physical_table_info
 *	Purpose:
 *		Return information about the VP table.
 *	Conditions:
 *		Nothing locked.  Obeys CountInOut protocol.
 *	Returns:
 *		KERN_SUCCESS		Returned information.
 *		KERN_INVALID_HOST	The host is null.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory.
 */

kern_return_t
host_virtual_physical_table_info(host, infop, countp)
	host_t host;
	hash_info_bucket_array_t *infop;
	natural_t *countp;
{
	vm_offset_t addr;
	vm_size_t size = 0;/* '=0' to quiet gcc warnings */
	hash_info_bucket_t *info;
	unsigned int potential, actual;
	kern_return_t kr;

	if (host == HOST_NULL)
		return KERN_INVALID_HOST;

	/* start with in-line data */

	info = *infop;
	potential = *countp;

	for (;;) {
		actual = vm_page_info(info, potential);
		if (actual <= potential)
			break;

		/* allocate more memory */

		if (info != *infop)
			kmem_free(ipc_kernel_map, addr, size);

		size = round_page(actual * sizeof *info);
		kr = kmem_alloc_pageable(ipc_kernel_map, &addr, size);
		if (kr != KERN_SUCCESS)
			return KERN_RESOURCE_SHORTAGE;

		info = (hash_info_bucket_t *) addr;
		potential = size/sizeof *info;
	}

	if (info == *infop) {
		/* data fit in-line; nothing to deallocate */

		*countp = actual;
	} else if (actual == 0) {
		kmem_free(ipc_kernel_map, addr, size);

		*countp = 0;
	} else {
		vm_map_copy_t copy;
		vm_size_t used;

		used = round_page(actual * sizeof *info);

		if (used != size)
			kmem_free(ipc_kernel_map, addr + used, size - used);

		kr = vm_map_copyin(ipc_kernel_map, addr, used,
				   TRUE, &copy);
		assert(kr == KERN_SUCCESS);

		*infop = (hash_info_bucket_t *) copy;
		*countp = actual;
	}

	return KERN_SUCCESS;
}
