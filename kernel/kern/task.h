/* 
 * Mach Operating System
 * Copyright (c) 1994-1988 Carnegie Mellon University
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
 * 19-Aug-94  David Golub (dbg) at Carnegie-Mellon University
 *	Include <kern/pc_sample.h> instead of mach/pc_sample.h
 *	for sampling data structures.
 *
 * $Log:	task.h,v $
 * Revision 2.12  93/08/10  15:12:22  mrt
 * 	Conditionalized atm hooks.
 * 	[93/07/30            cmaeda]
 * 	Included hooks for network interface.
 * 	[93/06/09  15:43:43  jcb]
 * 
 * Revision 2.11  93/01/24  13:20:05  danner
 * 	Added sample_control to indicate that all the threads of the task are
 * 	to have their pc's sampled periodically.
 * 	[93/01/12            rvb]
 * 
 * Revision 2.10  93/01/21  12:22:26  danner
 * 	Expanded to allow TASK_FAST_TAS_NRAS ras.
 * 	[93/01/19  16:33:32  bershad]
 * 
 * Revision 2.9  93/01/19  09:01:02  danner
 * 	Corrected argument types.
 * 	[93/01/19            danner]
 * 
 * Revision 2.8  93/01/14  17:36:49  danner
 * 	Moved actual declaration of struct task and task_t to
 * 	kern/kern_types.h, to permit mutually recursive structure
 * 	definitions.  Added ANSI function prototypes.
 * 	[92/12/28            dbg]
 * 	Swapped fields for better alignment.
 * 	[92/12/01            af]
 * 
 * Revision 2.7  92/07/20  13:33:02  cmaeda
 * 	Added fast tas addresses to task structure.
 * 	[92/05/11  14:36:52  cmaeda]
 * 
 * Revision 2.6  91/11/15  14:11:16  rpd
 * 	NORMA_TASK: added child_node field, which determines where child tasks
 * 	are created for the given task. A value of -1 means local node.
 * 	[91/09/23  09:19:35  jsb]
 * 
 * Revision 2.5  91/05/14  16:48:20  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:30:06  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:19:18  mrt]
 * 
 * Revision 2.3  90/06/02  14:56:48  rpd
 * 	Removed kernel_vm_space, keep_wired.  They are no longer needed.
 * 	[90/04/29            rpd]
 * 	Converted to new IPC.
 * 	[90/03/26  22:23:06  rpd]
 * 
 * Revision 2.2  89/09/08  11:26:47  dbg
 * 	Add 'keep_wired' privilege field, to allow out-of-line data
 * 	passed to task to remain wired.  Needed for default pager.
 * 	Remove kernel_vm_space (not used).
 * 	[89/07/17            dbg]
 * 
 * 19-Oct-88  David Golub (dbg) at Carnegie-Mellon University
 *	Removed all non-MACH data structures.
 *
 * Revision 2.6  88/09/25  22:16:41  rpd
 * 	Changed port_cache fields/definitions to obj_cache.
 * 	[88/09/24  18:13:13  rpd]
 * 
 * Revision 2.5  88/08/24  02:46:30  mwyoung
 * 	Adjusted include file references.
 * 	[88/08/17  02:24:13  mwyoung]
 * 
 * Revision 2.4  88/07/20  21:07:49  rpd
 * Added ipc_task_lock/ipc_task_unlock definitions.
 * Changes for port sets.
 * Add ipc_next_name field, used for assigning local port names.
 * 
 * Revision 2.3  88/07/17  18:56:33  mwyoung
 * .
 * 
 * Revision 2.2.2.1  88/06/28  20:02:03  mwyoung
 * Cleaned up.  Replaced task_t->kernel_only with
 * task_t->kernel_ipc_space, task_t->kernel_vm_space, and
 * task_t->ipc_privilege, to prevent overloading errors.
 * 
 * Remove current_task() declaration.
 * Eliminate paging_task.
 * 
 * Revision 2.2.1.2  88/06/26  00:45:49  rpd
 * Changes for port sets.
 * 
 * Revision 2.2.1.1  88/06/23  23:32:38  rpd
 * Add ipc_next_name field, used for assigning local port names.
 * 
 * 21-Jun-88  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Cleaned up.  Replaced task_t->kernel_only with
 *	task_t->kernel_ipc_space, task_t->kernel_vm_space, and
 *	task_t->ipc_privilege, to prevent overloading errors.
 *
 * 19-Apr-88  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Remove current_task() declaration.
 *	Eliminate paging_task.
 *
 * 18-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Removed task_data (now is per_thread).  Added
 *	task_bootstrap_port.  Added new routine declarations.
 *	Removed wake_active (unused).  Added fields to accumulate
 *	user and system time for terminated threads.
 *
 *  19-Feb-88 Douglas Orr (dorr) at Carnegie-Mellon University
 *	Change emulation bit mask into vector of routine  addrs
 *
 *  27-Jan-87 Douglas Orr (dorr) at Carnegie-Mellon University
 *	Add support for user space syscall emulation (bit mask
 *	of enabled user space syscalls and user space emulation
 *	routine).
 *
 *  3-Dec-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Change port cache account for per-task port names.
 *	Should move IPC stuff to a separate file :-).
 *	Add reply port for use by kernel-internal tasks.
 *
 *  2-Dec-87  David Black (dlb) at Carnegie-Mellon University
 *	Added active field.
 *
 * 18-Nov-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Eliminate conditionals, flush history.
 */
/*
 *	File:	task.h
 *	Author:	Avadis Tevanian, Jr.
 *
 *	This file contains the structure definitions for tasks.
 *
 */

#ifndef	_KERN_TASK_H_
#define _KERN_TASK_H_

#include <norma_task.h>
#include <fast_tas.h>
#include <net_atm.h>

#include <mach/boolean.h>
#include <mach/port.h>
#include <mach/time_value.h>
#include <mach/mach_param.h>
#include <mach/task_info.h>
#include <kern/kern_types.h>
#include <kern/lock.h>
#include <kern/queue.h>
#include <kern/pc_sample.h>
#include <kern/processor.h>
#include <kern/syscall_emulation.h>
#include <vm/vm_map.h>

#if	NET_ATM
typedef struct nw_ep_owned {
  unsigned int ep;
  struct nw_ep_owned *next;
} nw_ep_owned_s, *nw_ep_owned_t;
#endif

struct task {
	/* Synchronization/destruction information */
	decl_simple_lock_data(,lock)	/* Task's lock */
	int		ref_count;	/* Number of references to me */
	boolean_t	active;		/* Task has not been terminated */

	/* Miscellaneous */
	vm_map_t	map;		/* Address space description */
	queue_chain_t	pset_tasks;	/* list of tasks assigned to pset */
	int		suspend_count;	/* Internal scheduling only */

	/* Thread information */
	queue_head_t	thread_list;	/* list of threads */
	int		thread_count;	/* number of threads */
	processor_set_t	processor_set;	/* processor set for new threads */
	boolean_t	may_assign;	/* can assigned pset be changed? */
	boolean_t	assign_active;	/* waiting for may_assign */

	/* User-visible scheduling information */
	int		user_stop_count;	/* outstanding stops */
	int		priority;		/* for new threads */

	/* Statistics */
	time_value_t	total_user_time;
				/* total user time for dead threads */
	time_value_t	total_system_time;
				/* total system time for dead threads */

	/* IPC structures */
	decl_simple_lock_data(, itk_lock_data)
	struct ipc_port *itk_self;	/* not a right, doesn't hold ref */
	struct ipc_port *itk_sself;	/* a send right */
	struct ipc_port *itk_exception;	/* a send right */
	struct ipc_port *itk_bootstrap;	/* a send right */
	struct ipc_port *itk_registered[TASK_PORT_REGISTER_MAX];
					/* all send rights */

	struct ipc_space *itk_space;

	/* User space system call emulation support */
	struct 	eml_dispatch	*eml_dispatch;

	sample_control_t pc_sample;

#if	NORMA_TASK
	long		child_node;	/* if != -1, node for new children */
#endif	/* NORMA_TASK */

#if	FAST_TAS
#define TASK_FAST_TAS_NRAS	8	
	vm_offset_t	fast_tas_base[TASK_FAST_TAS_NRAS];
	vm_offset_t	fast_tas_end[TASK_FAST_TAS_NRAS];
#endif	/* FAST_TAS */

#if	NET_ATM
	nw_ep_owned_t   nw_ep_owned;
#endif	/* NET_ATM */
};

#define task_lock(task)		simple_lock(&(task)->lock)
#define task_unlock(task)	simple_unlock(&(task)->lock)

#define	itk_lock_init(task)	simple_lock_init(&(task)->itk_lock_data)
#define	itk_lock(task)		simple_lock(&(task)->itk_lock_data)
#define	itk_unlock(task)	simple_unlock(&(task)->itk_lock_data)

/*
 *	Exported routines/macros
 */

extern kern_return_t	task_create(
	task_t		parent_task,
	boolean_t	inherit_memory,
	task_t		*child_task);
extern kern_return_t	task_terminate(
	task_t		task);
extern kern_return_t	task_suspend(
	task_t		task);
extern kern_return_t	task_resume(
	task_t		task);
extern kern_return_t	task_threads(
	task_t		task,
	thread_array_t	*thread_list,
	natural_t	*count);
extern kern_return_t	task_info(
	task_t		task,
	int		flavor,
	task_info_t	task_info_out,
	natural_t	*task_info_count);
extern kern_return_t	task_get_special_port(
	task_t		task,
	int		which,
	struct ipc_port	**portp);
extern kern_return_t	task_set_special_port(
	task_t		task,
	int		which,
	struct ipc_port	*port);
extern kern_return_t	task_assign(
	task_t		task,
	processor_set_t	new_pset,
	boolean_t	assign_threads);
extern kern_return_t	task_assign_default(
	task_t		task,
	boolean_t	assign_threads);

/*
 *	Internal only routines
 */

extern void		task_init();
extern void		task_reference();
extern void		task_deallocate();
extern kern_return_t	task_hold();
extern kern_return_t	task_dowait();
extern kern_return_t	task_release();
extern kern_return_t	task_halt();

extern kern_return_t	task_suspend_nowait();
extern task_t		kernel_task_create();

extern task_t	kernel_task;

#endif	_KERN_TASK_H_
