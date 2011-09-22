/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	vm_external.h,v $
 * Revision 2.5  91/05/14  17:48:31  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:57:53  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:31:29  mrt]
 * 
 * Revision 2.3  90/05/29  18:38:36  rwd
 * 	Picked up rfr changes.
 * 	[90/04/12  13:46:56  rwd]
 * 
 * Revision 2.2  90/01/11  11:47:31  dbg
 * 	Added changes from mainline:
 * 		Remember bitmap size, for safer deallocation.
 * 		[89/10/16  15:32:58  af]
 * 		Declare vm_external_destroy().
 * 		[89/08/08            mwyoung]
 * 
 * Revision 2.1  89/08/03  16:44:46  rwd
 * Created.
 * 
 * Revision 2.3  89/04/18  21:24:53  mwyoung
 * 	Created.
 * 	[89/04/18            mwyoung]
 * 
 */

#ifndef	_VM_VM_EXTERNAL_H_
#define _VM_VM_EXTERNAL_H_

/*
 *	External page management hint technology
 *
 *	The data structure exported by this module maintains
 *	a (potentially incomplete) map of the pages written
 *	to external storage for a range of virtual memory.
 */

/*
 *	The data structure representing the state of pages
 *	on external storage.
 */

typedef struct vm_external {
    	int		existence_size;	/* Size of the following bitmap */
	char		*existence_map;	/* A bitmap of pages that have
					 * been written to backing
					 * storage.
					 */
	int		existence_count;/* Number of bits turned on in
					 * existence_map.
					 */
} *vm_external_t;

#define	VM_EXTERNAL_NULL	((vm_external_t) 0)

#define VM_EXTERNAL_SMALL_SIZE	128
#define VM_EXTERNAL_LARGE_SIZE	8192

/*
 *	The states that may be recorded for a page of external storage.
 */

typedef int	vm_external_state_t;
#define	VM_EXTERNAL_STATE_EXISTS		1
#define	VM_EXTERNAL_STATE_UNKNOWN		2
#define	VM_EXTERNAL_STATE_ABSENT		3


/*
 *	Routines exported by this module.
 */

extern void		vm_external_module_initialize();
						/* Initialize the module */

extern vm_external_t	vm_external_create();	/* Create a vm_external_t */
extern void vm_external_destroy();		/* Destroy one */

extern void		vm_external_state_set();/* Set state of a page. */
#define	vm_external_state_get(e,offset)	(((e) != VM_EXTERNAL_NULL) ? \
					  _vm_external_state_get(e, offset) : \
					  VM_EXTERNAL_STATE_UNKNOWN)
						/* Retrieve the state
						 * for a given page, if known.
						 */
extern vm_external_state_t _vm_external_state_get();
						/* HIDDEN routine */

#endif	_VM_VM_EXTERNAL_H_
