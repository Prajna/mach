/* 
 * Mach Operating System
 * Copyright (c) 1993-1987 Carnegie Mellon University
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
 * $Log:	vm_map.h,v $
 * Revision 2.18  93/08/10  15:13:47  mrt
 * 	Included the projected_on field in VM map entries, to support 
 * 	projected buffers.
 * 	[93/02/16  09:46:01  jcb]
 * 
 * Revision 2.17  93/01/14  18:01:34  danner
 * 	64bit cleanup.
 * 	[92/12/10  20:49:22  af]
 * 
 * Revision 2.16  92/05/21  17:26:14  jfriedl
 * 	Added cast to call of assert_wait in vm_map_entry_wait.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.15  92/03/31  15:18:41  rpd
 * 	Add extend_cont continuation invocation macro to invoke
 * 	continuation without affecting current copy.  Declare
 * 	vm_map_copy_discard_cont.
 * 	[92/03/20  14:15:53  dlb]
 * 
 * Revision 2.14  92/02/23  19:51:00  elf
 * 	Change unused wiring_allowed field to wiring_required
 * 	in vm_map data structure.
 * 	[92/02/20  15:19:12  dlb]
 * 
 * 	Add is_shared bit to map entry to detect sharing.
 * 	[92/02/19  14:26:45  dlb]
 * 
 * 	Remove all sharing map structure elements.
 * 	Make vm_map_verify_done() a macro.
 * 	[92/01/07  11:14:16  dlb]
 * 
 * Revision 2.13  91/12/10  13:27:03  jsb
 * 	Simplify page list continuation abort logic. Temporarily
 * 	increase size of page lists for NORMA_IPC until it supports
 * 	page list continuations.
 * 	[91/12/10  12:50:01  dlb]
 * 
 * 	Fix type of null pointer in continuation invocation in
 * 	vm_map_copy_abort_cont.
 * 
 * Revision 2.12  91/08/28  11:18:33  jsb
 * 	Supplied missing argument to thread_block in vm_map_entry_wait.
 * 	[91/08/16  10:36:21  jsb]
 * 
 * 	Minor cleanups.
 * 	[91/08/06  17:26:24  dlb]
 * 
 * 	Discard pages before invoking or aborting a continuation.
 * 	[91/08/05  17:51:55  dlb]
 * 
 * 	Add declarations for in transition map entries and vm_map_copy
 * 	continuations.
 * 	[91/07/30  14:18:14  dlb]
 * 
 * Revision 2.11  91/07/01  08:27:43  jsb
 * 	Declarations for multiple-format vm map copy support.
 * 	[91/06/29  14:36:42  dlb]
 * 
 * Revision 2.10  91/05/18  14:41:09  rpd
 * 	Added kentry_data and friends, for vm_map_init.
 * 	[91/03/22            rpd]
 * 
 * Revision 2.9  91/05/14  17:50:06  mrt
 * 	Correcting copyright
 * 
 * Revision 2.8  91/03/16  15:06:07  rpd
 * 	Removed vm_map_find.  Added vm_map_find_entry.
 * 	[91/03/03            rpd]
 * 
 * Revision 2.7  91/02/05  17:59:07  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:33:15  mrt]
 * 
 * Revision 2.6  90/10/12  13:06:08  rpd
 * 	Removed copy_on_write field.
 * 	[90/10/08            rpd]
 * 
 * Revision 2.5  90/06/19  23:02:32  rpd
 * 	Picked up vm_submap_object.
 * 	[90/06/08            rpd]
 * 
 * Revision 2.4  90/06/02  15:11:24  rpd
 * 	New vm_map_pageable, with user_wired_count.
 * 	[90/03/26  23:15:58  rpd]
 * 
 * Revision 2.3  90/02/22  20:06:19  dbg
 * 	Combine fields in vm_map and vm_map_copy into a vm_map_header
 * 	structure.
 * 	[90/01/29            dbg]
 * 
 * 	Add changes from mainline:
 * 
 * 		Added documentation for exported routines.
 * 		Add vm_map_t->wait_for_space field.
 * 		Add vm_map_copy_t type, associated routine declarations, and
 * 		documentation.
 * 		Introduced vm_map_links, which contains those map entry fields
 * 		used in the map structure.
 * 		[89/08/31  21:13:56  rpd]
 * 
 * 		Optimization from NeXT:  is_a_map, is_sub_map, copy_on_write,
 * 		needs_copy are now bit-fields.
 * 		[89/08/19  23:44:53  rpd]
 * 
 * Revision 2.2  90/01/22  23:09:35  af
 * 	Added vm_map_machine_attribute() decl.
 * 
 * 	Changes for MACH_KERNEL:
 * 	. Added wiring_allowed to map.
 * 	[89/04/29            dbg]
 * 
 * Revision 2.1  89/08/03  16:45:29  rwd
 * Created.
 * 
 * Revision 2.9  89/04/18  21:26:14  mwyoung
 * 	Reset history.  All relevant material is in the documentation
 * 	here, and in the implementation file ("vm/vm_map.c").
 * 	[89/04/18            mwyoung]
 * 
 */
/*
 *	File:	vm/vm_map.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1985
 *
 *	Virtual memory map module definitions.
 *
 * Contributors:
 *	avie, dlb, mwyoung
 */

#ifndef	_VM_VM_MAP_H_
#define _VM_VM_MAP_H_

#include <mach/kern_return.h>
#include <mach/boolean.h>
#include <mach/machine/vm_types.h>
#include <mach/vm_prot.h>
#include <mach/vm_inherit.h>
#include <vm/pmap.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <kern/lock.h>
#include <kern/macro_help.h>

/*
 *	Types defined:
 *
 *	vm_map_t		the high-level address map data structure.
 *	vm_map_entry_t		an entry in an address map.
 *	vm_map_version_t	a timestamp of a map, for use with vm_map_lookup
 *	vm_map_copy_t		represents memory copied from an address map,
 *				 used for inter-map copy operations
 */

/*
 *	Type:		vm_map_object_t [internal use only]
 *
 *	Description:
 *		The target of an address mapping, either a virtual
 *		memory object or a sub map (of the kernel map).
 */
typedef union vm_map_object {
	struct vm_object	*vm_object;	/* object object */
	struct vm_map		*sub_map;	/* belongs to another map */
} vm_map_object_t;

/*
 *	Type:		vm_map_entry_t [internal use only]
 *
 *	Description:
 *		A single mapping within an address map.
 *
 *	Implementation:
 *		Address map entries consist of start and end addresses,
 *		a VM object (or sub map) and offset into that object,
 *		and user-exported inheritance and protection information.
 *		Control information for virtual copy operations is also
 *		stored in the address map entry.
 */
struct vm_map_links {
	struct vm_map_entry	*prev;		/* previous entry */
	struct vm_map_entry	*next;		/* next entry */
	vm_offset_t		start;		/* start address */
	vm_offset_t		end;		/* end address */
};

struct vm_map_entry {
	struct vm_map_links	links;		/* links to other entries */
#define vme_prev		links.prev
#define vme_next		links.next
#define vme_start		links.start
#define vme_end			links.end
	union vm_map_object	object;		/* object I point to */
	vm_offset_t		offset;		/* offset into object */
	unsigned int
	/* boolean_t */		is_shared:1,	/* region is shared */
	/* boolean_t */		is_sub_map:1,	/* Is "object" a submap? */
	/* boolean_t */		in_transition:1, /* Entry being changed */
	/* boolean_t */		needs_wakeup:1,  /* Waiters on in_transition */
		/* Only used when object is a vm_object: */
	/* boolean_t */		needs_copy:1;    /* does object need to be copied */

		/* Only in task maps: */
	vm_prot_t		protection;	/* protection code */
	vm_prot_t		max_protection;	/* maximum protection */
	vm_inherit_t		inheritance;	/* inheritance */
	unsigned short		wired_count;	/* can be paged if = 0 */
	unsigned short		user_wired_count; /* for vm_wire */
	struct vm_map_entry     *projected_on;  /* 0 for normal map entry
           or persistent kernel map projected buffer entry;
           -1 for non-persistent kernel map projected buffer entry;
           pointer to corresponding kernel map entry for user map
           projected buffer entry */
};

typedef struct vm_map_entry	*vm_map_entry_t;

#define VM_MAP_ENTRY_NULL	((vm_map_entry_t) 0)

/*
 *	Type:		struct vm_map_header
 *
 *	Description:
 *		Header for a vm_map and a vm_map_copy.
 */
struct vm_map_header {
	struct vm_map_links	links;		/* first, last, min, max */
	int			nentries;	/* Number of entries */
	boolean_t		entries_pageable;
						/* are map entries pageable? */
};

/*
 *	Type:		vm_map_t [exported; contents invisible]
 *
 *	Description:
 *		An address map -- a directory relating valid
 *		regions of a task's address space to the corresponding
 *		virtual memory objects.
 *
 *	Implementation:
 *		Maps are doubly-linked lists of map entries, sorted
 *		by address.  One hint is used to start
 *		searches again from the last successful search,
 *		insertion, or removal.  Another hint is used to
 *		quickly find free space.
 */
typedef struct vm_map {
	lock_data_t		lock;		/* Lock for map data */
	struct vm_map_header	hdr;		/* Map entry header */
#define min_offset		hdr.links.start	/* start of range */
#define max_offset		hdr.links.end	/* end of range */
	pmap_t			pmap;		/* Physical map */
	vm_size_t		size;		/* virtual size */
	int			ref_count;	/* Reference count */
	decl_simple_lock_data(,	ref_lock)	/* Lock for ref_count field */
	vm_map_entry_t		hint;		/* hint for quick lookups */
	decl_simple_lock_data(,	hint_lock)	/* lock for hint storage */
	vm_map_entry_t		first_free;	/* First free space hint */
	boolean_t		wait_for_space;	/* Should callers wait
						   for space? */
	boolean_t		wiring_required;/* All memory wired? */
	unsigned int		timestamp;	/* Version number */
} *vm_map_t;

#define		VM_MAP_NULL	((vm_map_t) 0)

#define vm_map_to_entry(map)	((struct vm_map_entry *) &(map)->hdr.links)
#define vm_map_first_entry(map)	((map)->hdr.links.next)
#define vm_map_last_entry(map)	((map)->hdr.links.prev)

/*
 *	Type:		vm_map_version_t [exported; contents invisible]
 *
 *	Description:
 *		Map versions may be used to quickly validate a previous
 *		lookup operation.
 *
 *	Usage note:
 *		Because they are bulky objects, map versions are usually
 *		passed by reference.
 *
 *	Implementation:
 *		Just a timestamp for the main map.
 */
typedef struct vm_map_version {
	unsigned int	main_timestamp;
} vm_map_version_t;

/*
 *	Type:		vm_map_copy_t [exported; contents invisible]
 *
 *	Description:
 *		A map copy object represents a region of virtual memory
 *		that has been copied from an address map but is still
 *		in transit.
 *
 *		A map copy object may only be used by a single thread
 *		at a time.
 *
 *	Implementation:
 * 		There are three formats for map copy objects.  
 *		The first is very similar to the main
 *		address map in structure, and as a result, some
 *		of the internal maintenance functions/macros can
 *		be used with either address maps or map copy objects.
 *
 *		The map copy object contains a header links
 *		entry onto which the other entries that represent
 *		the region are chained.
 *
 *		The second format is a single vm object.  This is used
 *		primarily in the pageout path.  The third format is a
 *		list of vm pages.  An optional continuation provides
 *		a hook to be called to obtain more of the memory,
 *		or perform other operations.  The continuation takes 3
 *		arguments, a saved arg buffer, a pointer to a new vm_map_copy
 *		(returned) and an abort flag (abort if TRUE).
 */

#if	iPSC386 || iPSC860
#define VM_MAP_COPY_PAGE_LIST_MAX	64
#else	iPSC386 || iPSC860
#define VM_MAP_COPY_PAGE_LIST_MAX	8
#endif	iPSC386 || iPSC860

typedef struct vm_map_copy {
	int			type;
#define VM_MAP_COPY_ENTRY_LIST	1
#define VM_MAP_COPY_OBJECT	2
#define VM_MAP_COPY_PAGE_LIST	3
	vm_offset_t		offset;
	vm_size_t		size;
	union {
	    struct vm_map_header	hdr;	/* ENTRY_LIST */
	    struct {				/* OBJECT */
	    	vm_object_t		object;
	    } c_o;
	    struct {				/* PAGE_LIST */
		vm_page_t		page_list[VM_MAP_COPY_PAGE_LIST_MAX];
		int			npages;
		kern_return_t		(*cont)();
		char			*cont_args;
	    } c_p;
	} c_u;
} *vm_map_copy_t;

#define cpy_hdr			c_u.hdr

#define cpy_object		c_u.c_o.object

#define cpy_page_list		c_u.c_p.page_list
#define cpy_npages		c_u.c_p.npages
#define cpy_cont		c_u.c_p.cont
#define cpy_cont_args		c_u.c_p.cont_args

#define	VM_MAP_COPY_NULL	((vm_map_copy_t) 0)

/*
 *	Useful macros for entry list copy objects
 */

#define vm_map_copy_to_entry(copy)		\
		((struct vm_map_entry *) &(copy)->cpy_hdr.links)
#define vm_map_copy_first_entry(copy)		\
		((copy)->cpy_hdr.links.next)
#define vm_map_copy_last_entry(copy)		\
		((copy)->cpy_hdr.links.prev)

/*
 *	Continuation macros for page list copy objects
 */

#define	vm_map_copy_invoke_cont(old_copy, new_copy, result)		\
MACRO_BEGIN								\
	vm_map_copy_page_discard(old_copy);				\
	*result = (*((old_copy)->cpy_cont))((old_copy)->cpy_cont_args,	\
					    new_copy);			\
	(old_copy)->cpy_cont = (kern_return_t (*)()) 0;			\
MACRO_END

#define	vm_map_copy_invoke_extend_cont(old_copy, new_copy, result)	\
MACRO_BEGIN								\
	*result = (*((old_copy)->cpy_cont))((old_copy)->cpy_cont_args,	\
					    new_copy);			\
	(old_copy)->cpy_cont = (kern_return_t (*)()) 0;			\
MACRO_END

#define vm_map_copy_abort_cont(old_copy)				\
MACRO_BEGIN								\
	vm_map_copy_page_discard(old_copy);				\
	(*((old_copy)->cpy_cont))((old_copy)->cpy_cont_args,		\
				  (vm_map_copy_t *) 0);			\
	(old_copy)->cpy_cont = (kern_return_t (*)()) 0;			\
  	(old_copy)->cpy_cont_args = (char *) 0;				\
MACRO_END

#define vm_map_copy_has_cont(copy)					\
    (((copy)->cpy_cont) != (kern_return_t (*)()) 0)

/*
 *	Continuation structures for vm_map_copyin_page_list.
 */

typedef	struct {
	vm_map_t	map;
	vm_offset_t	src_addr;
	vm_size_t	src_len;
	vm_offset_t	destroy_addr;
	vm_size_t	destroy_len;
	boolean_t	steal_pages;
}  vm_map_copyin_args_data_t, *vm_map_copyin_args_t;

#define	VM_MAP_COPYIN_ARGS_NULL	((vm_map_copyin_args_t) 0)

/*
 *	Macros:		vm_map_lock, etc. [internal use only]
 *	Description:
 *		Perform locking on the data portion of a map.
 */

#define vm_map_lock_init(map)			\
MACRO_BEGIN					\
	lock_init(&(map)->lock, TRUE);		\
	(map)->timestamp = 0;			\
MACRO_END

#define vm_map_lock(map)			\
MACRO_BEGIN					\
	lock_write(&(map)->lock);		\
	(map)->timestamp++;			\
MACRO_END

#define vm_map_unlock(map)	lock_write_done(&(map)->lock)
#define vm_map_lock_read(map)	lock_read(&(map)->lock)
#define vm_map_unlock_read(map)	lock_read_done(&(map)->lock)
#define vm_map_lock_write_to_read(map) \
		lock_write_to_read(&(map)->lock)
#define vm_map_lock_read_to_write(map) \
		(lock_read_to_write(&(map)->lock) || (((map)->timestamp++), 0))
#define vm_map_lock_set_recursive(map) \
		lock_set_recursive(&(map)->lock)
#define vm_map_lock_clear_recursive(map) \
		lock_clear_recursive(&(map)->lock)

/*
 *	Exported procedures that operate on vm_map_t.
 */

extern vm_offset_t	kentry_data;
extern vm_offset_t	kentry_data_size;
extern int		kentry_count;
extern void		vm_map_init();		/* Initialize the module */

extern vm_map_t		vm_map_create();	/* Create an empty map */
extern vm_map_t		vm_map_fork();		/* Create a map in the image
						 * of an existing map */

extern void		vm_map_reference();	/* Gain a reference to
						 * an existing map */
extern void		vm_map_deallocate();	/* Lose a reference */

extern kern_return_t	vm_map_enter();		/* Enter a mapping */
extern kern_return_t	vm_map_find_entry();	/* Enter a mapping primitive */
extern kern_return_t	vm_map_remove();	/* Deallocate a region */
extern kern_return_t	vm_map_protect();	/* Change protection */
extern kern_return_t	vm_map_inherit();	/* Change inheritance */

extern void		vm_map_print();		/* Debugging: print a map */

extern kern_return_t	vm_map_lookup();	/* Look up an address */
extern boolean_t	vm_map_verify();	/* Verify that a previous
						 * lookup is still valid */
/* vm_map_verify_done is now a macro -- see below */
extern kern_return_t	vm_map_copyin();	/* Make a copy of a region */
extern kern_return_t	vm_map_copyin_page_list();/* Make a copy of a region
						 * using a page list copy */
extern kern_return_t	vm_map_copyout();	/* Place a copy into a map */
extern kern_return_t	vm_map_copy_overwrite();/* Overwrite existing memory
						 * with a copy */
extern void		vm_map_copy_discard();	/* Discard a copy without
						 * using it */
extern kern_return_t	vm_map_copy_discard_cont();/* Page list continuation
						 * version of previous */

extern kern_return_t	vm_map_machine_attribute();
						/* Add or remove machine-
						   dependent attributes from
						   map regions */

/*
 *	Functions implemented as macros
 */
#define		vm_map_min(map)		((map)->min_offset)
						/* Lowest valid address in
						 * a map */

#define		vm_map_max(map)		((map)->max_offset)
						/* Highest valid address */

#define		vm_map_pmap(map)	((map)->pmap)
						/* Physical map associated
						 * with this address map */

#define		vm_map_verify_done(map, version)    (vm_map_unlock_read(map))
						/* Operation that required
						 * a verified lookup is
						 * now complete */
/*
 *	Pageability functions.  Includes macro to preserve old interface.
 */
extern kern_return_t	vm_map_pageable_common();

#define vm_map_pageable(map, s, e, access)	\
		vm_map_pageable_common(map, s, e, access, FALSE)

#define vm_map_pageable_user(map, s, e, access)	\
		vm_map_pageable_common(map, s, e, access, TRUE)

/*
 *	Submap object.  Must be used to create memory to be put
 *	in a submap by vm_map_submap.
 */
extern vm_object_t	vm_submap_object;

/*
 *	Wait and wakeup macros for in_transition map entries.
 */
#define vm_map_entry_wait(map, interruptible)    	\
        MACRO_BEGIN                                     \
        assert_wait((event_t)&(map)->hdr, interruptible);	\
        vm_map_unlock(map);                             \
	thread_block((void (*)()) 0);			\
        MACRO_END

#define vm_map_entry_wakeup(map)        thread_wakeup((event_t)&(map)->hdr)

#endif	_VM_VM_MAP_H_
