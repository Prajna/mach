/*
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
 * $Log:	thread_swap.c,v $
 * Revision 2.12  93/05/15  18:56:04  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.11  93/01/14  17:37:17  danner
 * 	Corrected casts for thread_wakeup and assert_wait.
 * 	[93/01/12            danner]
 * 	64bit cleanup. Proper spl typing.
 * 	[92/12/01            af]
 * 
 * Revision 2.10  92/08/03  17:40:14  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.9  92/05/21  17:16:48  jfriedl
 * 	tried prototypes.
 * 	[92/05/20            jfriedl]
 * 
 * Revision 2.8  91/07/31  17:50:04  dbg
 * 	Revise scheduling state machine.
 * 	[91/07/30  17:07:03  dbg]
 * 
 * Revision 2.7  91/05/14  16:49:08  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/03/16  14:53:09  rpd
 * 	Removed thread_swapout.
 * 	[91/02/24            rpd]
 * 	Added swapin_thread_continue.
 * 	Simplified the state machine.  Now it uses only
 * 	TH_SW_IN, TH_SW_OUT, TH_SW_COMING_IN.
 * 	[91/01/20            rpd]
 * 
 * 	Simplified thread_swapin.
 * 	[91/01/17            rpd]
 * 
 * Revision 2.5  91/02/05  17:30:40  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:20:14  mrt]
 * 
 * Revision 2.4  91/01/08  15:18:20  rpd
 * 	Added continuation argument to thread_block.
 * 	[90/12/08            rpd]
 * 
 * 	Removed swapout_thread, swapout_threads,
 * 	swapout_scan, thread_swapout.
 * 	[90/11/11            rpd]
 * 
 * Revision 2.3  90/06/02  14:57:18  rpd
 * 	In thread_swapout, free the thread's cached message buffer.
 * 	[90/04/23            rpd]
 * 	Converted to new processor set technology.
 * 	[90/03/26  22:26:32  rpd]
 * 
 * Revision 2.2  89/12/08  19:52:35  rwd
 * 	Added call to zone_gc()
 * 	[89/11/21            rwd]
 * 
 * Revision 2.1  89/08/03  15:48:24  rwd
 * Created.
 * 
 * Revision 2.4  88/10/27  10:50:40  rpd
 * 	Changed includes to the new style.  Removed extraneous semis
 * 	from the swapper_lock/swapper_unlock macros.
 * 	[88/10/26  14:49:09  rpd]
 * 
 * 15-Jun-88  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Fix improper handling of swapper_lock() in swapin_thread().
 *	Problem discovery and elegant recoding due to Richard Draves.
 *
 *  4-May-88  David Golub (dbg) at Carnegie-Mellon University
 *	Remove vax-specific code.
 *
 *  1-Mar-88  David Black (dlb) at Carnegie-Mellon University
 *	Logic change due to replacement of wait_time field in thread
 *	with sched_stamp.  Extra argument to thread_setrun().
 *
 * 25-Jan-88  Richard Sanzi (sanzi) at Carnegie-Mellon University
 *	Notify pcb module that pcb is about to be unwired by calling
 *	pcb_synch(thread).
 *	
 * 21-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Fix lots more race conditions (thread_swapin called during
 *	swapout, thread_swapin called twice) by adding a swapper state
 *	machine to each thread.  Moved thread_swappable here from
 *	kern/thread.c.
 *
 * 12-Nov-87  David Golub (dbg) at Carnegie-Mellon University
 *	Fix race condition in thread_swapout: mark thread as swapped
 *	before swapping out its stack, so that an intervening wakeup
 *	will put it on the swapin list.
 *
 *  5-Oct-87  David Golub (dbg) at Carnegie-Mellon University
 *	Changed to new scheduling state machine.
 *
 * 15-Sep-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	De-linted.
 *
 *  5-Sep-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added check for THREAD_SWAPPABLE in swapout_scan().
 *
 * 14-Jul-87  David Golub (dbg) at Carnegie-Mellon University
 *	Truncate the starting address and round up the size given to
 *	vm_map_pageable, when wiring/unwiring kernel stacks.
 *	KERNEL_STACK_SIZE is not necessarily a multiple of page_size; if
 *	it isn't, forgetting to round the address and size to page
 *	boundaries results in panic.  Kmem_alloc and kmem_free, used in
 *	thread.c to allocate and free kernel stacks, already round to
 *	page boundaries.
 *
 * 26-Jun-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Add thread_swapout_allowed flag to make it easy to turn
 *	off swapping when debugging.
 *
 *  4-Jun-87  David Golub (dbg) at Carnegie-Mellon University
 *	Pass correct number of parameters to lock_init - initialize
 *	swap_lock as sleepable instead of calling lock_sleepable
 *	separately.
 *
 *  1-Apr-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Include vm_param.h to pick up KERNEL_STACK_SIZE definition.
 *
 * 20-Mar-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Lower ipl before calling thread_swapout().
 *
 * 19-Mar-87  David Golub (dbg) at Carnegie-Mellon University
 *	Fix one race condition in this (not so buggy) version - since
 *	thread_swapin can be called from interrupts, must raise IPL when
 *	locking swapper_lock.
 *
 * 09-Mar-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Created, based somewhat loosely on the earlier (which was a highly
 *	buggy, race condition filled version).
 *
 */
/*
 *
 *	File:	kern/thread_swap.c
 *	Author:	Avadis Tevanian, Jr.
 *	Date:	1987
 *
 *	Mach thread swapper:
 *		Find idle threads to swap, freeing up kernel stack resources
 *		at the expense of allowing them to execute.
 *
 *		Swap in threads that need to be run.  This is done here
 *		by the swapper thread since it cannot be done (in general)
 *		when the kernel tries to place a thread on a run queue.
 *
 *	Note: The act of swapping a thread in Mach does not mean that
 *	its memory gets forcibly swapped to secondary storage.  The memory
 *	for the task corresponding to a swapped thread is paged out
 *	through the normal paging mechanism.
 *
 */

#include <ipc/ipc_kmsg.h>
#include <kern/counters.h>
#include <kern/thread.h>
#include <kern/lock.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <mach/vm_param.h>
#include <kern/sched_prim.h>
#include <kern/processor.h>
#include <kern/thread_swap.h>
#include <machine/machspl.h>		/* for splsched */



queue_head_t		swapin_queue;
decl_simple_lock_data(,	swapper_lock_data)

#define swapper_lock()		simple_lock(&swapper_lock_data)
#define swapper_unlock()	simple_unlock(&swapper_lock_data)

/*
 *	swapper_init: [exported]
 *
 *	Initialize the swapper module.
 */
void swapper_init()
{
	queue_init(&swapin_queue);
	simple_lock_init(&swapper_lock_data);
}

/*
 *	thread_swapin: [exported]
 *
 *	Place the specified thread in the list of threads to swapin.  It
 *	is assumed that the thread is locked, therefore we are at splsched.
 *
 *	We don't bother with stack_alloc_try to optimize swapin;
 *	our callers have already tried that route.
 */

void thread_swapin(thread)
	thread_t	thread;
{
	switch (thread->state & TH_SWAP_STATE) {
	    case TH_SWAPPED:
		/*
		 *	Swapped out - queue for swapin thread.
		 */
		thread->state = (thread->state & ~TH_SWAP_STATE)
				| TH_SW_COMING_IN;
		swapper_lock();
		enqueue_tail(&swapin_queue, (queue_entry_t) thread);
		swapper_unlock();
		thread_wakeup((event_t) &swapin_queue);
		break;

	    case TH_SW_COMING_IN:
		/*
		 *	Already queued for swapin thread, or being
		 *	swapped in.
		 */
		break;

	    default:
		/*
		 *	Already swapped in.
		 */
		panic("thread_swapin");
	}
}

/*
 *	thread_doswapin:
 *
 *	Swapin the specified thread, if it should be runnable, then put
 *	it on a run queue.  No locks should be held on entry, as it is
 *	likely that this routine will sleep (waiting for stack allocation).
 */
void thread_doswapin(thread)
	register thread_t thread;
{
	spl_t	s;

	/*
	 *	Allocate the kernel stack.
	 */

	stack_alloc(thread, thread_continue);

	/*
	 *	Place on run queue.  
	 */

	s = splsched();
	thread_lock(thread);
	thread->state &= ~(TH_SWAPPED | TH_SW_COMING_IN);
	if (thread->state & TH_RUN)
		thread_setrun(thread, TRUE);
	thread_unlock(thread);
	(void) splx(s);
}

/*
 *	swapin_thread: [exported]
 *
 *	This procedure executes as a kernel thread.  Threads that need to
 *	be swapped in are swapped in by this thread.
 */
void swapin_thread_continue()
{
	for (;;) {
		register thread_t thread;
		spl_t s;

		s = splsched();
		swapper_lock();

		while ((thread = (thread_t) dequeue_head(&swapin_queue))
							!= THREAD_NULL) {
			swapper_unlock();
			(void) splx(s);

			thread_doswapin(thread);		/* may block */

			s = splsched();
			swapper_lock();
		}

		assert_wait((event_t) &swapin_queue, FALSE);
		swapper_unlock();
		(void) splx(s);
		counter(c_swapin_thread_block++);
		thread_block(swapin_thread_continue);
	}
}

void swapin_thread()
{
	stack_privilege(current_thread());

	swapin_thread_continue();
	/*NOTREACHED*/
}
