/* 
 * Mach Operating System
 * Copyright (c) 1994-1987 Carnegie Mellon University
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
 * 31-Aug-94  David Golub (dbg) at Carnegie-Mellon University
 *	Clear the entire pc_sample structure when creating a task.
 *	PC sampling code keys on pc_sample.sampletypes.
 *
 * $Log:	thread.c,v $
 * Revision 2.33  93/08/10  15:11:43  mrt
 * 	Conditionalized atm hooks.
 * 	[93/07/30            cmaeda]
 * 	Included hooks for network interface.
 * 	[93/06/09  15:44:22  jcb]
 * 
 * Revision 2.32  93/05/15  18:55:33  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.31  93/01/24  13:20:13  danner
 * 	Add call to evc_notify_abort to thread_abort; correct call in
 * 	 thread_terminate. 
 * 	[93/01/22            danner]
 * 
 * 	We must explicitly set "new_thread->pc_sample.buffer = 0;" so
 * 	that we don't think we have a sampling buffer.
 * 	[93/01/13            rvb]
 * 
 * Revision 2.30  93/01/21  12:22:49  danner
 * 	Add call in thread_deallocate to evc_notify_thread_destroy to
 * 	deal with threads terminating while in evc_wait.
 * 	[93/01/20            bershad]
 * 
 * Revision 2.29  93/01/14  17:36:55  danner
 * 	Added ANSI function prototypes.
 * 	[92/12/29            dbg]
 * 
 * 	Changed thread_create to hold both the pset and task locks while
 * 	inserting the new thread in the lists.
 * 	[92/11/20            dbg]
 * 
 * 	Fixed pset lock ordering.  Pset lock must be taken before task
 * 	and thread locks.
 * 	[92/10/28            dbg]
 * 	64bit cleanup. Proper spl typing.
 * 	[92/12/01            af]
 * 
 * 	Changed thread_create to hold both the pset and task locks while
 * 	inserting the new thread in the lists.
 * 	[92/11/20            dbg]
 * 
 * 	Fixed pset lock ordering.  Pset lock must be taken before task
 * 	and thread locks.
 * 	[92/10/28            dbg]
 * 
 * Revision 2.28  92/08/03  17:39:56  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.27  92/05/21  17:16:33  jfriedl
 * 	Appended 'U' to constants that would otherwise be signed.
 * 	Changed name of one of the two local 'thread' variables in
 * 	processor_set_stack_usage() to 'tmp_thread' for cleanness.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.26  92/04/01  19:33:34  rpd
 * 	Restored continuation in AST_TERMINATE case of thread_halt_self.
 * 	[92/03/22            rpd]
 * 
 * Revision 2.25  92/03/10  16:24:22  jsb
 * 	Remove continuation from AST_TERMINATE case of thread_halt_self 
 * 	so that "zombie walks" is reachable.
 * 	[92/02/25            dlb]
 * 
 * Revision 2.24  92/02/19  16:07:03  elf
 * 	Change calls to compute_priority.
 * 	[92/01/19            rwd]
 * 
 * Revision 2.23  92/01/03  20:18:56  dbg
 * 	Make thread_wire really reserve a kernel stack.
 * 	[91/12/18            dbg]
 * 
 * Revision 2.22  91/12/13  14:54:53  jsb
 * 	Removed thread_resume_from_kernel.
 * 	[91/12/12  17:40:12  af]
 * 
 * Revision 2.21  91/08/28  11:14:43  jsb
 * 	Fixed thread_halt, mach_msg_interrupt interaction.
 * 	Added checks for thread_exception_return, thread_bootstrap_return.
 * 	[91/08/03            rpd]
 * 
 * Revision 2.20  91/07/31  17:49:10  dbg
 * 	Call pcb_module_init from thread_init.
 * 	[91/07/26            dbg]
 * 
 * 	When halting a thread: if it is waiting at a continuation with a
 * 	known cleanup routine, call the cleanup routine instead of
 * 	resuming the thread.
 * 	[91/06/21            dbg]
 * 
 * 	Revise scheduling state machine.
 * 	[91/05/22            dbg]
 * 
 * 	Add thread_wire.
 * 	[91/05/14            dbg]
 * 
 * Revision 2.19  91/06/25  10:29:48  rpd
 * 	Picked up dlb's thread_doassign no-op fix.
 * 	[91/06/23            rpd]
 * 
 * Revision 2.18  91/05/18  14:34:09  rpd
 * 	Fixed stack_alloc to use kmem_alloc_aligned.
 * 	[91/05/14            rpd]
 * 	Added argument to kernel_thread.
 * 	[91/04/03            rpd]
 * 
 * 	Changed thread_deallocate to reset timer and depress_timer.
 * 	[91/03/31            rpd]
 * 
 * 	Replaced stack_free_reserved and swap_privilege with stack_privilege.
 * 	[91/03/30            rpd]
 * 
 * Revision 2.17  91/05/14  16:48:35  mrt
 * 	Correcting copyright
 * 
 * Revision 2.16  91/05/08  12:49:14  dbg
 * 	Add volatile declarations.
 * 	[91/04/26  14:44:13  dbg]
 * 
 * Revision 2.15  91/03/16  14:52:40  rpd
 * 	Fixed the initialization of stack_lock_data.
 * 	[91/03/11            rpd]
 * 	Updated for new kmem_alloc interface.
 * 	[91/03/03            rpd]
 * 	Removed ith_saved.
 * 	[91/02/16            rpd]
 * 
 * 	Added stack_alloc_max.
 * 	[91/02/10            rpd]
 * 	Added active_stacks.
 * 	[91/01/28            rpd]
 * 	Can't use thread_dowait on the current thread now.
 * 	Added reaper_thread_continue.
 * 	Added stack_free_reserved.
 * 	[91/01/20            rpd]
 * 
 * 	Removed thread_swappable.
 * 	Allow swapped threads on the run queues.
 * 	Changed the AST interface.
 * 	[91/01/17            rpd]
 * 
 * Revision 2.14  91/02/05  17:30:14  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:19:30  mrt]
 * 
 * Revision 2.13  91/01/08  15:17:57  rpd
 * 	Added KEEP_STACKS support.
 * 	[91/01/06            rpd]
 * 	Added consider_thread_collect, thread_collect_scan.
 * 	[91/01/03            rpd]
 * 
 * 	Added locking to the stack package.  Added stack_collect.
 * 	[90/12/31            rpd]
 * 	Changed thread_dowait to discard the stacks of suspended threads.
 * 	[90/12/22            rpd]
 * 	Added continuation argument to thread_block.
 * 	[90/12/08            rpd]
 * 
 * 	Changed thread_create to make new threads be swapped.
 * 	Changed kernel_thread to swapin the threads.
 * 	[90/11/20            rpd]
 * 
 * 	Removed stack_free/stack_alloc/etc.
 * 	[90/11/12            rpd]
 * 
 * 	Changed thread_create to let pcb_init handle all pcb initialization.
 * 	[90/11/11            rpd]
 * 
 * Revision 2.12  90/12/05  23:29:09  af
 * 
 * 
 * Revision 2.11  90/12/05  20:42:19  af
 * 	Added (temporarily, until we define the right new primitive with
 * 	the RTMach guys) thread_resume_from_kernel() for internal kernel
 * 	use only.
 * 
 * 	Revision 2.7.1.1  90/09/25  13:06:04  dlb
 * 	Inline thread_hold in thread_suspend and thread_release in
 * 	thread_resume (while still holding the thread lock in both).
 * 	This eliminates a long-standing MP bug.
 * 	[90/09/20            dlb]
 * 
 * Revision 2.10  90/11/05  14:31:46  rpd
 * 	Unified untimeout and untimeout_try.
 * 	[90/10/29            rpd]
 * 
 * Revision 2.9  90/10/25  14:45:37  rwd
 * 	Added host_stack_usage and processor_set_stack_usage.
 * 	[90/10/22            rpd]
 * 
 * 	Removed pausing code in stack_alloc/stack_free.
 * 	It was broken and it isn't needed.
 * 	Added code to check how much stack is actually used.
 * 	[90/10/21            rpd]
 * 
 * Revision 2.8  90/10/12  12:34:37  rpd
 * 	Fixed HW_FOOTPRINT code in thread_create.
 * 	Fixed a couple bugs in thread_policy.
 * 	[90/10/09            rpd]
 * 
 * Revision 2.7  90/08/27  22:04:03  dbg
 * 	Fixed thread_info to return the correct count.
 * 	[90/08/23            rpd]
 * 
 * Revision 2.6  90/08/27  11:52:25  dbg
 * 	Remove unneeded mode parameter to thread_start.
 * 	Remove u_zone, thread_deallocate_interrupt.
 * 	[90/07/17            dbg]
 * 
 * Revision 2.5  90/08/07  17:59:04  rpd
 * 	Removed tmp_address, tmp_object fields.
 * 	Picked up depression abort functionality in thread_abort.
 * 	Picked up initialization of max_priority in kernel_thread.
 * 	Picked up revised priority computation in thread_create.
 * 	Removed in-transit check from thread_max_priority.
 * 	Picked up interprocessor-interrupt optimization in thread_dowait.
 * 	[90/08/07            rpd]
 * 
 * Revision 2.4  90/06/02  14:56:54  rpd
 * 	Converted to new IPC and scheduling technology.
 * 	[90/03/26  22:24:06  rpd]
 * 
 * Revision 2.3  90/02/22  20:04:12  dbg
 * 	Initialize per-thread global VM variables.
 * 	Clean up global variables in thread_deallocate().
 * 		[89/04/29	mwyoung]
 * 
 * Revision 2.2  89/09/08  11:26:53  dbg
 * 	Allocate thread kernel stack with kmem_alloc_wired to get
 * 	separate vm_object for it.
 * 	[89/08/18            dbg]
 * 
 * 19-Aug-88  David Golub (dbg) at Carnegie-Mellon University
 *	Removed all non-MACH code.
 *
 * 11-Aug-88  David Black (dlb) at Carnegie-Mellon University
 *	Rewrote exit logic to use new ast mechanism.  Includes locking
 *	bug fix to thread_terminate().
 *
 *  9-Aug-88  David Black (dlb) at Carnegie-Mellon University
 *	first_quantum replaces preempt_pri.
 *
 * Revision 2.3  88/08/06  18:26:41  rpd
 * Changed to use ipc_thread_lock/ipc_thread_unlock macros.
 * Eliminated use of kern/mach_ipc_defs.h.
 * Added definitions of all_threads, all_threads_lock.
 * 
 *  4-May-88  David Golub (dbg) and David Black (dlb) at CMU
 *	Remove vax-specific code.  Add register declarations.
 *	MACH_TIME_NEW now standard.  Moved thread_read_times to timer.c.
 *	SIMPLE_CLOCK: clock drift compensation in cpu_usage calculation.
 *	Initialize new fields in thread_create().  Implemented cpu usage
 *	calculation in thread_info().  Added argument to thread_setrun.
 *	Initialization changes for MACH_TIME_NEW.
 *
 * 13-Apr-88  David Black (dlb) at Carnegie-Mellon University
 *	Rewrite kernel stack retry code to eliminate races and handle
 *	termination correctly.
 *
 * 19-Feb-88  David Kirschen (kirschen) at Encore Computer Corporation
 *      Retry if kernel stacks exhausted on thread_create
 *
 * 12-Feb-88  David Black (dlb) at Carnegie-Mellon University
 *	Fix MACH_TIME_NEW code.
 *
 *  1-Feb-88  David Golub (dbg) at Carnegie-Mellon University
 *	In thread_halt: mark the victim thread suspended/runnable so that it
 *	will notify the caller when it hits the next interruptible wait.
 *	The victim may not immediately proceed to a clean point once it
 *	is awakened.
 *
 * 21-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Use new swapping state machine.  Moved thread_swappable to
 *	thread_swap.c.
 *
 * 17-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Added new thread interfaces: thread_suspend, thread_resume,
 *	thread_get_state, thread_set_state, thread_abort,
 *	thread_info.  Old interfaces remain (temporarily) for binary
 *	compatibility, prefixed with 'xxx_'.
 *
 * 29-Dec-87  David Golub (dbg) at Carnegie-Mellon University
 *	Delinted.
 *
 * 15-Dec-87  David Golub (dbg) at Carnegie-Mellon University
 *	Made thread_reference and thread_deallocate check for null
 *	threads.  Call pcb_terminate when a thread is deallocated.
 *	Call pcb_init with thread pointer instead of pcb pointer.
 *	Add missing case to thread_dowait.
 *
 *  9-Dec-87  David Golub (dbg) at Carnegie-Mellon University
 *	Rewrote thread termination to have a terminating thread clean up
 *	after itself.
 *
 *  9-Dec-87  David Black (dlb) at Carnegie-Mellon University
 *	Moved reaper invocation to thread_terminate from
 *	thread_deallocate.  [XXX temporary pending rewrite.]
 *
 *  8-Dec-87  David Black (dlb) at Carnegie-Mellon University
 *	Added call to ipc_thread_disable.
 *
 *  4-Dec-87  David Black (dlb) at Carnegie-Mellon University
 *	Set ipc_kernel in kernel_thread().
 *
 *  3-Dec-87  David Black (dlb) at Carnegie-Mellon University
 *	Rewrote thread_create().  thread_terminate() must throw away
 *	an extra reference if called on the current thread [ref is
 *	held by caller who will not be returned to.]  Locking bug fix
 *	to thread_status.
 *
 * 19-Nov-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Eliminated TT conditionals.
 *
 * 30-Oct-87  David Golub (dbg) at Carnegie-Mellon University
 *	Fix race condition in thread_deallocate for thread terminating
 *	itself.
 *
 * 23-Oct-87  David Golub (dbg) at Carnegie-Mellon University
 *	Correctly set thread_statistics fields.
 *
 * 13-Oct-87  David Black (dlb) at Carnegie-Mellon University
 *	Use counts for suspend and resume primitives.
 *
 *  5-Oct-87  David Golub (dbg) at Carnegie-Mellon University
 *	MACH_TT: Completely replaced scheduling state machine.
 *
 * 30-Sep-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added initialization of thread->flags in thread_create().
 *	Added thread_swappable().
 *	De-linted.
 *
 * 30-Sep-87  David Black (dlb) at Carnegie-Mellon University
 *	Rewrote thread_dowait to more effectively stop threads.
 *
 * 11-Sep-87  Robert Baron (rvb) at Carnegie-Mellon University
 *	Initialize thread fields and unix_lock.
 *
 *  9-Sep-87  David Black (dlb) at Carnegie-Mellon University
 *	Changed thread_dowait to count a thread as stopped if it is
 *	sleeping and will stop immediately when woken up.  [i.e. is
 *	sleeping interruptibly].  Corresponding change to
 *	thread_terminate().
 *
 *  4-Aug-87  David Golub (dbg) at Carnegie-Mellon University
 *	Moved ipc_thread_terminate to thread_terminate (from
 *	thread_deallocate), to shut out other threads that are
 *	manipulating the thread via its thread_port.
 *
 * 29-Jul-87  David Golub (dbg) at Carnegie-Mellon University
 *	Make sure all deallocation calls are outside of locks - they may
 *	block.  Moved task_deallocate from thread_deallocate to
 *	thread_destroy, since thread may blow up if task disappears while
 *	thread is running.
 *
 * 26-Jun-87  David Black (dlb) at Carnegie-Mellon University
 *	Added update_priority() call to thread_release() for any thread
 *	actually released.
 *
 * 23-Jun-87  David Black (dlb) at Carnegie-Mellon University
 *	Initialize thread priorities in thread_create() and kernel_thread().
 *
 * 10-Jun-87  Karl Hauth (hauth) at Carnegie-Mellon University
 *	Added code to fill in the thread_statistics structure.
 *
 *  1-Jun-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Added thread_statistics stub.
 *
 * 21-May-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Clear the thread u-area upon creation of a thread to keep
 *	consistent.
 *
 *  4-May-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Call uarea_init to initialize u-area stuff.
 *
 * 29-Apr-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Moved call to ipc_thread_terminate into the MACH_TT only branch
 *	to prevent problems with non-TT systems.
 *
 * 28-Apr-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Support the thread status information as a MiG refarray.
 *	[NOTE: turned off since MiG is still too braindamaged.]
 *
 * 23-Apr-87  Rick Rashid (rfr) at Carnegie-Mellon University
 *	Moved ipc_thread_terminate to thread_deallocate from
 *	thread_destroy to eliminate having the reaper call it after
 *	the task has been freed.
 *
 * 18-Mar-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Added reaper thread for deallocating threads that cannot
 *	deallocate themselves (some time ago).
 *
 * 17-Mar-87  David Golub (dbg) at Carnegie-Mellon University
 *	De-linted.
 *
 * 14-Mar-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Panic if no space left in the kernel map for stacks.
 *
 *  6-Mar-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Add kernel_thread routine which starts up kernel threads.
 *
 *  4-Mar-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Make thread_terminate work.
 *
 *  2-Mar-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	New kernel stack allocation mechanism.
 *
 * 27-Feb-87  David L. Black (dlb) at Carnegie-Mellon University
 *	MACH_TIME_NEW: Added timer inits to thread_create().
 *
 * 24-Feb-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Rewrote thread_suspend/thread_hold and added thread_wait for new
 *	user synchronization paradigm.
 *
 * 24-Feb-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Reorded locking protocol in thread_deallocate for the
 *	all_threads_lock (this allows one to reference a thread then
 *	release the all_threads_lock when scanning the thread list).
 *
 * 31-Jan-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Merged in my changes for real thread implementation.
 *
 * 30-Sep-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Make floating u-area work, maintain list of threads per task.
 *
 *  1-Aug-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added initialization for Mach-style IPC.
 *
 *  7-Jul-86  Rick Rashid (rfr) at Carnegie-Mellon University
 *	Added thread_in_use_chain to keep track of threads which
 *	have been created but not yet destroyed.
 *
 * 31-May-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Initialize thread state field to THREAD_WAITING.  Some general
 *	cleanup.
 *
 */
/*
 *	File:	kern/thread.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young, David Golub
 *	Date:	1986
 *
 *	Thread management primitives implementation.
 */

#include <cpus.h>
#include <hw_footprint.h>
#include <mach_host.h>
#include <mach_fixpri.h>
#include <mach_pcsample.h>
#include <simple_clock.h>
#include <mach_debug.h>
#include <net_atm.h>

#include <mach/std_types.h>
#include <mach/policy.h>
#include <mach/thread_info.h>
#include <mach/thread_special_ports.h>
#include <mach/thread_status.h>
#include <mach/time_value.h>
#include <mach/vm_param.h>
#include <kern/ast.h>
#include <kern/counters.h>
#include <kern/ipc_tt.h>
#include <kern/mach_param.h>
#include <kern/processor.h>
#include <kern/queue.h>
#include <kern/sched.h>
#include <kern/sched_prim.h>
#include <kern/thread.h>
#include <kern/thread_swap.h>
#include <kern/host.h>
#include <kern/zalloc.h>
#include <vm/vm_kern.h>
#include <ipc/ipc_kmsg.h>
#include <ipc/ipc_port.h>
#include <ipc/mach_msg.h>
#include <machine/machspl.h>		/* for splsched */
#include <machine/thread.h>		/* for MACHINE_STACK */

#if	NET_ATM
#include <chips/nw_mk.h>
#endif

thread_t active_threads[NCPUS];
vm_offset_t active_stacks[NCPUS];

struct zone *thread_zone;

queue_head_t		reaper_queue;
decl_simple_lock_data(,	reaper_lock)

extern int		tick;

extern void		pcb_module_init(void);

/* private */
struct thread	thread_template;

#if	MACH_DEBUG
void stack_init(vm_offset_t stack);	/* forward */
void stack_finalize(vm_offset_t stack);	/* forward */

#define	STACK_MARKER	0xdeadbeefU
boolean_t		stack_check_usage = FALSE;
decl_simple_lock_data(,	stack_usage_lock)
vm_size_t		stack_max_usage = 0;
#endif	/* MACH_DEBUG */

/*
 *	Machine-dependent code must define:
 *		pcb_init
 *		pcb_terminate
 *		pcb_collect
 *
 *	The thread->pcb field is reserved for machine-dependent code.
 */

#ifdef	MACHINE_STACK
/*
 *	Machine-dependent code must define:
 *		stack_alloc_try
 *		stack_alloc
 *		stack_free
 *		stack_handoff
 *		stack_collect
 *	and if MACH_DEBUG:
 *		stack_statistics
 */
#else	/* MACHINE_STACK */
/*
 *	We allocate stacks from generic kernel VM.
 *	Machine-dependent code must define:
 *		stack_attach
 *		stack_detach
 *		stack_handoff
 *
 *	The stack_free_list can only be accessed at splsched,
 *	because stack_alloc_try/thread_invoke operate at splsched.
 */

decl_simple_lock_data(, stack_lock_data)/* splsched only */
#define stack_lock()	simple_lock(&stack_lock_data)
#define stack_unlock()	simple_unlock(&stack_lock_data)

vm_offset_t stack_free_list;		/* splsched only */
unsigned int stack_free_count = 0;	/* splsched only */
unsigned int stack_free_limit = 1;	/* patchable */

unsigned int stack_alloc_hits = 0;	/* debugging */
unsigned int stack_alloc_misses = 0;	/* debugging */
unsigned int stack_alloc_max = 0;	/* debugging */

/*
 *	The next field is at the base of the stack,
 *	so the low end is left unsullied.
 */

#define stack_next(stack) (*((vm_offset_t *)((stack) + KERNEL_STACK_SIZE) - 1))

/*
 *	stack_alloc_try:
 *
 *	Non-blocking attempt to allocate a kernel stack.
 *	Called at splsched with the thread locked.
 */

boolean_t stack_alloc_try(
	thread_t	thread,
	void		(*resume)(thread_t))
{
	register vm_offset_t stack;

	stack_lock();
	stack = stack_free_list;
	if (stack != 0) {
		stack_free_list = stack_next(stack);
		stack_free_count--;
	} else {
		stack = thread->stack_privilege;
	}
	stack_unlock();

	if (stack != 0) {
		stack_attach(thread, stack, resume);
		stack_alloc_hits++;
		return TRUE;
	} else {
		stack_alloc_misses++;
		return FALSE;
	}
}

/*
 *	stack_alloc:
 *
 *	Allocate a kernel stack for a thread.
 *	May block.
 */

void stack_alloc(
	thread_t	thread,
	void		(*resume)(thread_t))
{
	vm_offset_t stack;
	spl_t s;

	/*
	 *	We first try the free list.  It is probably empty,
	 *	or stack_alloc_try would have succeeded, but possibly
	 *	a stack was freed before the swapin thread got to us.
	 */

	s = splsched();
	stack_lock();
	stack = stack_free_list;
	if (stack != 0) {
		stack_free_list = stack_next(stack);
		stack_free_count--;
	}
	stack_unlock();
	(void) splx(s);

	if (stack == 0) {
		/*
		 *	Kernel stacks should be naturally aligned,
		 *	so that it is easy to find the starting/ending
		 *	addresses of a stack given an address in the middle.
		 */

		if (kmem_alloc_aligned(kernel_map, &stack, KERNEL_STACK_SIZE)
							!= KERN_SUCCESS)
			panic("stack_alloc");

#if	MACH_DEBUG
		stack_init(stack);
#endif	/* MACH_DEBUG */
	}

	stack_attach(thread, stack, resume);
}

/*
 *	stack_free:
 *
 *	Free a thread's kernel stack.
 *	Called at splsched with the thread locked.
 */

void stack_free(
	thread_t thread)
{
	register vm_offset_t stack;

	stack = stack_detach(thread);

	if (stack != thread->stack_privilege) {
		stack_lock();
		stack_next(stack) = stack_free_list;
		stack_free_list = stack;
		if (++stack_free_count > stack_alloc_max)
			stack_alloc_max = stack_free_count;
		stack_unlock();
	}
}

/*
 *	stack_collect:
 *
 *	Free excess kernel stacks.
 *	May block.
 */

void stack_collect(void)
{
	register vm_offset_t stack;
	spl_t s;

	s = splsched();
	stack_lock();
	while (stack_free_count > stack_free_limit) {
		stack = stack_free_list;
		stack_free_list = stack_next(stack);
		stack_free_count--;
		stack_unlock();
		(void) splx(s);

#if	MACH_DEBUG
		stack_finalize(stack);
#endif	/* MACH_DEBUG */
		kmem_free(kernel_map, stack, KERNEL_STACK_SIZE);

		s = splsched();
		stack_lock();
	}
	stack_unlock();
	(void) splx(s);
}
#endif	/* MACHINE_STACK */

/*
 *	stack_privilege:
 *
 *	stack_alloc_try on this thread must always succeed.
 */

void stack_privilege(
	register thread_t thread)
{
	/*
	 *	This implementation only works for the current thread.
	 */

	if (thread != current_thread())
		panic("stack_privilege");

	if (thread->stack_privilege == 0)
		thread->stack_privilege = current_stack();
}

void thread_init(void)
{
	thread_zone = zinit(
			sizeof(struct thread),
			THREAD_MAX * sizeof(struct thread),
			THREAD_CHUNK * sizeof(struct thread),
			FALSE, "threads");

	/*
	 *	Fill in a template thread for fast initialization.
	 *	[Fields that must be (or are typically) reset at
	 *	time of creation are so noted.]
	 */

	/* thread_template.links (none) */
	thread_template.runq = RUN_QUEUE_NULL;

	/* thread_template.task (later) */
	/* thread_template.thread_list (later) */
	/* thread_template.pset_threads (later) */

	/* thread_template.lock (later) */
	/* one ref for being alive; one for the guy who creates the thread */
	thread_template.ref_count = 2;

	thread_template.pcb = (pcb_t) 0;		/* (reset) */
	thread_template.kernel_stack = (vm_offset_t) 0;
	thread_template.stack_privilege = (vm_offset_t) 0;

	thread_template.wait_event = 0;
	/* thread_template.suspend_count (later) */
	thread_template.wait_result = KERN_SUCCESS;
	thread_template.wake_active = FALSE;
	thread_template.state = TH_SUSP | TH_SWAPPED;
	thread_template.swap_func = thread_bootstrap_return;

/*	thread_template.priority (later) */
	thread_template.max_priority = BASEPRI_USER;
/*	thread_template.sched_pri (later - compute_priority) */
#if	MACH_FIXPRI
	thread_template.sched_data = 0;
	thread_template.policy = POLICY_TIMESHARE;
#endif	/* MACH_FIXPRI */
	thread_template.depress_priority = -1;
	thread_template.cpu_usage = 0;
	thread_template.sched_usage = 0;
	/* thread_template.sched_stamp (later) */

	thread_template.recover = (vm_offset_t) 0;
	thread_template.vm_privilege = FALSE;

	thread_template.user_stop_count = 1;

	/* thread_template.<IPC structures> (later) */

	timer_init(&(thread_template.user_timer));
	timer_init(&(thread_template.system_timer));
	thread_template.user_timer_save.low = 0;
	thread_template.user_timer_save.high = 0;
	thread_template.system_timer_save.low = 0;
	thread_template.system_timer_save.high = 0;
	thread_template.cpu_delta = 0;
	thread_template.sched_delta = 0;

	thread_template.active = FALSE; /* reset */
	thread_template.ast = AST_ZILCH;

	/* thread_template.processor_set (later) */
	thread_template.bound_processor = PROCESSOR_NULL;
#if	MACH_HOST
	thread_template.may_assign = TRUE;
	thread_template.assign_active = FALSE;
#endif	/* MACH_HOST */

#if	NCPUS > 1
	/* thread_template.last_processor  (later) */
#endif	/* NCPUS > 1 */

	/*
	 *	Initialize other data structures used in
	 *	this module.
	 */

	queue_init(&reaper_queue);
	simple_lock_init(&reaper_lock);

#ifndef	MACHINE_STACK
	simple_lock_init(&stack_lock_data);
#endif	/* MACHINE_STACK */

#if	MACH_DEBUG
	simple_lock_init(&stack_usage_lock);
#endif	/* MACH_DEBUG */

	/*
	 *	Initialize any machine-dependent
	 *	per-thread structures necessary.
	 */

	pcb_module_init();
}

kern_return_t thread_create(
	register task_t	parent_task,
	thread_t	*child_thread)		/* OUT */
{
	register thread_t	new_thread;
	register processor_set_t	pset;

	if (parent_task == TASK_NULL)
		return KERN_INVALID_ARGUMENT;

	/*
	 *	Allocate a thread and initialize static fields
	 */

	new_thread = (thread_t) zalloc(thread_zone);

	if (new_thread == THREAD_NULL)
		return KERN_RESOURCE_SHORTAGE;

	*new_thread = thread_template;

	/*
	 *	Initialize runtime-dependent fields
	 */

	new_thread->task = parent_task;
	simple_lock_init(&new_thread->lock);
	new_thread->sched_stamp = sched_tick;
	thread_timeout_setup(new_thread);

	/*
	 *	Create a pcb.  The kernel stack is created later,
	 *	when the thread is swapped-in.
	 */
	pcb_init(new_thread);

	ipc_thread_init(new_thread);

#if	NET_ATM
	new_thread->nw_ep_waited = 0;
#endif

	/*
	 *	Find the processor set for the parent task.
	 */
	task_lock(parent_task);
	pset = parent_task->processor_set;
	pset_reference(pset);
	task_unlock(parent_task);

	/*
	 *	Lock both the processor set and the task,
	 *	so that the thread can be added to both
	 *	simultaneously.  Processor set must be
	 *	locked first.
	 */

    Restart:
	pset_lock(pset);
	task_lock(parent_task);

	/*
	 *	If the task has changed processor sets,
	 *	catch up (involves lots of lock juggling).
	 */
	{
	    processor_set_t	cur_pset;

	    cur_pset = parent_task->processor_set;
	    if (!cur_pset->active)
		cur_pset = &default_pset;

	    if (cur_pset != pset) {
		pset_reference(cur_pset);
		task_unlock(parent_task);
		pset_unlock(pset);
		pset_deallocate(pset);
		pset = cur_pset;
		goto Restart;
	    }
	}

	/*
	 *	Set the thread`s priority from the pset and task.
	 */

	new_thread->priority = parent_task->priority;
	if (pset->max_priority > new_thread->max_priority)
		new_thread->max_priority = pset->max_priority;
	if (new_thread->max_priority > new_thread->priority)
		new_thread->priority = new_thread->max_priority;
	/*
	 *	Don't need to lock thread here because it can't
	 *	possibly execute and no one else knows about it.
	 */
	compute_priority(new_thread, TRUE);

	/*
	 *	Thread is suspended if the task is.  Add 1 to
	 *	suspend count since thread is created in suspended
	 *	state.
	 */
	new_thread->suspend_count = parent_task->suspend_count + 1;

	/*
	 *	Add the thread to the processor set.
	 *	If the pset is empty, suspend the thread again.
	 */

	pset_add_thread(pset, new_thread);
	if (pset->empty)
		new_thread->suspend_count++;

#if	HW_FOOTPRINT
	/*
	 *	Need to set last_processor, idle processor would be best, but
	 *	that requires extra locking nonsense.  Go for tail of
	 *	processors queue to avoid master.
	 */
	if (!pset->empty) {
		new_thread->last_processor = 
			(processor_t)queue_first(&pset->processors);
	}
	else {
		/*
		 *	Thread created in empty processor set.  Pick
		 *	master processor as an acceptable legal value.
		 */
		new_thread->last_processor = master_processor;
	}
#else	/* HW_FOOTPRINT */
	/*
	 *	Don't need to initialize because the context switch
	 *	code will set it before it can be used.
	 */
#endif	/* HW_FOOTPRINT */

#if	MACH_PCSAMPLE
	new_thread->pc_sample.buffer = 0;
	new_thread->pc_sample.seqno = 0;
	new_thread->pc_sample.sampletypes = 0;
#endif	/* MACH_PCSAMPLE */

	/*
	 *	Add the thread to the task`s list of threads.
	 *	The new thread holds another reference to the task.
	 */

	parent_task->ref_count++;

	parent_task->thread_count++;
	queue_enter(&parent_task->thread_list, new_thread, thread_t,
					thread_list);

	/*
	 *	Finally, mark the thread active.
	 */

	new_thread->active = TRUE;

	if (!parent_task->active) {
		task_unlock(parent_task);
		pset_unlock(pset);
		(void) thread_terminate(new_thread);
		/* release ref we would have given our caller */
		thread_deallocate(new_thread);
		return KERN_FAILURE;
	}
	task_unlock(parent_task);
	pset_unlock(pset);

	ipc_thread_enable(new_thread);

	*child_thread = new_thread;
	return KERN_SUCCESS;
}

unsigned int thread_deallocate_stack = 0;

void thread_deallocate(
	register thread_t	thread)
{
	spl_t		s;
	register task_t	task;
	register processor_set_t	pset;

	time_value_t	user_time, system_time;

	if (thread == THREAD_NULL)
		return;

	/*
	 *	First, check for new count > 0 (the common case).
	 *	Only the thread needs to be locked.
	 */
	s = splsched();
	thread_lock(thread);
	if (--thread->ref_count > 0) {
		thread_unlock(thread);
		(void) splx(s);
		return;
	}

	/*
	 *	Count is zero.  However, the task's and processor set's
	 *	thread lists have implicit references to
	 *	the thread, and may make new ones.  Their locks also
	 *	dominate the thread lock.  To check for this, we
	 *	temporarily restore the one thread reference, unlock
	 *	the thread, and then lock the other structures in
	 *	the proper order.
	 */
	thread->ref_count = 1;
	thread_unlock(thread);
	(void) splx(s);

	pset = thread->processor_set;
	pset_lock(pset);

#if	MACH_HOST
	/*
	 *	The thread might have moved.
	 */
	while (pset != thread->processor_set) {
	    pset_unlock(pset);
	    pset = thread->processor_set;
	    pset_lock(pset);
	}
#endif	/* MACH_HOST */

	task = thread->task;
	task_lock(task);

	s = splsched();
	thread_lock(thread);

	if (--thread->ref_count > 0) {
		/*
		 *	Task or processor_set made extra reference.
		 */
		thread_unlock(thread);
		(void) splx(s);
		task_unlock(task);
		pset_unlock(pset);
		return;
	}

	/*
	 *	Thread has no references - we can remove it.
	 */

	/*
	 *	Remove pending timeouts.
	 */
	reset_timeout_check(&thread->timer);

	reset_timeout_check(&thread->depress_timer);
	thread->depress_priority = -1;

	/*
	 *	Accumulate times for dead threads in task.
	 */
	thread_read_times(thread, &user_time, &system_time);
	time_value_add(&task->total_user_time, &user_time);
	time_value_add(&task->total_system_time, &system_time);

	/*
	 *	Remove thread from task list and processor_set threads list.
	 */
	task->thread_count--;
	queue_remove(&task->thread_list, thread, thread_t, thread_list);

	pset_remove_thread(pset, thread);

	thread_unlock(thread);		/* no more references - safe */
	(void) splx(s);
	task_unlock(task);
	pset_unlock(pset);
	pset_deallocate(pset);

	/*
	 *	A couple of quick sanity checks
	 */

	if (thread == current_thread()) {
	    panic("thread deallocating itself");
	}
	if ((thread->state & ~(TH_RUN | TH_HALTED | TH_SWAPPED)) != TH_SUSP)
		panic("unstopped thread destroyed!");

	/*
	 *	Deallocate the task reference, since we know the thread
	 *	is not running.
	 */
	task_deallocate(thread->task);			/* may block */

	/*
	 *	Clean up any machine-dependent resources.
	 */
	if ((thread->state & TH_SWAPPED) == 0) {
		spl_t _s_ = splsched();
		stack_free(thread);
		(void) splx(s);
		thread_deallocate_stack++;
	}
	/*
	 * Rattle the event count machinery (gag)
	 */
	evc_notify_abort(thread);

	pcb_terminate(thread);
	zfree(thread_zone, (vm_offset_t) thread);
}

void thread_reference(
	register thread_t	thread)
{
	spl_t		s;

	if (thread == THREAD_NULL)
		return;

	s = splsched();
	thread_lock(thread);
	thread->ref_count++;
	thread_unlock(thread);
	(void) splx(s);
}

/*
 *	thread_terminate:
 *
 *	Permanently stop execution of the specified thread.
 *
 *	A thread to be terminated must be allowed to clean up any state
 *	that it has before it exits.  The thread is broken out of any
 *	wait condition that it is in, and signalled to exit.  It then
 *	cleans up its state and calls thread_halt_self on its way out of
 *	the kernel.  The caller waits for the thread to halt, terminates
 *	its IPC state, and then deallocates it.
 *
 *	If the caller is the current thread, it must still exit the kernel
 *	to clean up any state (thread and port references, messages, etc).
 *	When it exits the kernel, it then terminates its IPC state and
 *	queues itself for the reaper thread, which will wait for the thread
 *	to stop and then deallocate it.  (A thread cannot deallocate itself,
 *	since it needs a kernel stack to execute.)
 */
kern_return_t thread_terminate(
	register thread_t	thread)
{
	register thread_t	cur_thread = current_thread();
	register task_t		cur_task;
	spl_t			s;

	if (thread == THREAD_NULL)
		return KERN_INVALID_ARGUMENT;

	/*
	 *	Break IPC control over the thread.
	 */
	ipc_thread_disable(thread);

	if (thread == cur_thread) {

	    /*
	     *	Current thread will queue itself for reaper when
	     *	exiting kernel.
	     */
	    s = splsched();
	    thread_lock(thread);
	    if (thread->active) {
		    thread->active = FALSE;
		    thread_ast_set(thread, AST_TERMINATE);
	    }
	    thread_unlock(thread);
	    ast_on(cpu_number(), AST_TERMINATE);
	    splx(s);
	    return KERN_SUCCESS;
	}

	/*
	 *	Lock both threads and the current task
	 *	to check termination races and prevent deadlocks.
	 */
	cur_task = current_task();
	task_lock(cur_task);
	s = splsched();
	if ((vm_offset_t)thread < (vm_offset_t)cur_thread) {
		thread_lock(thread);
		thread_lock(cur_thread);
	}
	else {
		thread_lock(cur_thread);
		thread_lock(thread);
	}

	/*
	 *	If the current thread is being terminated, help out.
	 */
	if ((!cur_task->active) || (!cur_thread->active)) {
		thread_unlock(cur_thread);
		thread_unlock(thread);
		(void) splx(s);
		task_unlock(cur_task);
		thread_terminate(cur_thread);
		return KERN_FAILURE;
	}
    
	thread_unlock(cur_thread);
	task_unlock(cur_task);

	/*
	 *	Terminate victim thread.
	 */
	if (!thread->active) {
		/*
		 *	Someone else got there first.
		 */
		thread_unlock(thread);
		(void) splx(s);
		return KERN_FAILURE;
	}

	thread->active = FALSE;

	thread_unlock(thread);
	(void) splx(s);

#if	MACH_HOST
	/*
	 *	Reassign thread to default pset if needed.
	 */
	thread_freeze(thread);
	if (thread->processor_set != &default_pset) {
		thread_doassign(thread, &default_pset, FALSE);
	}
#endif	/* MACH_HOST */

	/*
	 *	Halt the victim at the clean point.
	 */
	(void) thread_halt(thread, TRUE);
#if	MACH_HOST
	thread_unfreeze(thread);
#endif	/* MACH_HOST */
	/*
	 *	Shut down the victims IPC and deallocate its
	 *	reference to itself.
	 */
	ipc_thread_terminate(thread);
#if	NET_ATM
	mk_waited_collect(thread);
#endif
	thread_deallocate(thread);
	return KERN_SUCCESS;
}

/*
 *	thread_force_terminate:
 *
 *	Version of thread_terminate called by task_terminate.  thread is
 *	not the current thread.  task_terminate is the dominant operation,
 *	so we can force this thread to stop.
 */
void
thread_force_terminate(
	register thread_t	thread)
{
	boolean_t	deallocate_here = FALSE;
	spl_t s;

	ipc_thread_disable(thread);

#if	MACH_HOST
	/*
	 *	Reassign thread to default pset if needed.
	 */
	thread_freeze(thread);
	if (thread->processor_set != &default_pset)
		thread_doassign(thread, &default_pset, FALSE);
#endif	/* MACH_HOST */

	s = splsched();
	thread_lock(thread);
	deallocate_here = thread->active;
	thread->active = FALSE;
	thread_unlock(thread);
	(void) splx(s);

	(void) thread_halt(thread, TRUE);
	ipc_thread_terminate(thread);
#if	NET_ATM
	mk_waited_collect(thread);
#endif

#if	MACH_HOST
	thread_unfreeze(thread);
#endif	/* MACH_HOST */

	if (deallocate_here)
		thread_deallocate(thread);
}


/*
 *	Halt a thread at a clean point, leaving it suspended.
 *
 *	must_halt indicates whether thread must halt.
 *
 */
kern_return_t thread_halt(
	register thread_t	thread,
	boolean_t		must_halt)
{
	register thread_t	cur_thread = current_thread();
	register kern_return_t	ret;
	spl_t	s;

	if (thread == cur_thread)
		panic("thread_halt: trying to halt current thread.");
	/*
	 *	If must_halt is FALSE, then a check must be made for
	 *	a cycle of halt operations.
	 */
	if (!must_halt) {
		/*
		 *	Grab both thread locks.
		 */
		s = splsched();
		if ((vm_offset_t)thread < (vm_offset_t)cur_thread) {
			thread_lock(thread);
			thread_lock(cur_thread);
		}
		else {
			thread_lock(cur_thread);
			thread_lock(thread);
		}

		/*
		 *	If target thread is already halted, grab a hold
		 *	on it and return.
		 */
		if (thread->state & TH_HALTED) {
			thread->suspend_count++;
			thread_unlock(cur_thread);
			thread_unlock(thread);
			(void) splx(s);
			return KERN_SUCCESS;
		}

		/*
		 *	If someone is trying to halt us, we have a potential
		 *	halt cycle.  Break the cycle by interrupting anyone
		 *	who is trying to halt us, and causing this operation
		 *	to fail; retry logic will only retry operations
		 *	that cannot deadlock.  (If must_halt is TRUE, this
		 *	operation can never cause a deadlock.)
		 */
		if (cur_thread->ast & AST_HALT) {
			thread_wakeup_with_result((event_t)&cur_thread->wake_active,
				THREAD_INTERRUPTED);
			thread_unlock(thread);
			thread_unlock(cur_thread);
			(void) splx(s);
			return KERN_FAILURE;
		}

		thread_unlock(cur_thread);
	
	}
	else {
		/*
		 *	Lock thread and check whether it is already halted.
		 */
		s = splsched();
		thread_lock(thread);
		if (thread->state & TH_HALTED) {
			thread->suspend_count++;
			thread_unlock(thread);
			(void) splx(s);
			return KERN_SUCCESS;
		}
	}

	/*
	 *	Suspend thread - inline version of thread_hold() because
	 *	thread is already locked.
	 */
	thread->suspend_count++;
	thread->state |= TH_SUSP;

	/*
	 *	If someone else is halting it, wait for that to complete.
	 *	Fail if wait interrupted and must_halt is false.
	 */
	while ((thread->ast & AST_HALT) && (!(thread->state & TH_HALTED))) {
		thread->wake_active = TRUE;
		thread_sleep((event_t) &thread->wake_active,
			simple_lock_addr(thread->lock), TRUE);

		if (thread->state & TH_HALTED) {
			(void) splx(s);
			return KERN_SUCCESS;
		}
		if ((current_thread()->wait_result != THREAD_AWAKENED)
		    && !(must_halt)) {
			(void) splx(s);
			thread_release(thread);
			return KERN_FAILURE;
		}
		thread_lock(thread);
	}

	/*
	 *	Otherwise, have to do it ourselves.
	 */
		
	thread_ast_set(thread, AST_HALT);

	while (TRUE) {
	  	/*
		 *	Wait for thread to stop.
		 */
		thread_unlock(thread);
		(void) splx(s);

		ret = thread_dowait(thread, must_halt);

		/*
		 *	If the dowait failed, so do we.  Drop AST_HALT, and
		 *	wake up anyone else who might be waiting for it.
		 */
		if (ret != KERN_SUCCESS) {
			s = splsched();
			thread_lock(thread);
			thread_ast_clear(thread, AST_HALT);
			thread_wakeup_with_result((event_t)&thread->wake_active,
				THREAD_INTERRUPTED);
			thread_unlock(thread);
			(void) splx(s);

			thread_release(thread);
			return ret;
		}

		/*
		 *	Clear any interruptible wait.
		 */
		clear_wait(thread, THREAD_INTERRUPTED, TRUE);

		/*
		 *	If the thread's at a clean point, we're done.
		 *	Don't need a lock because it really is stopped.
		 */
		if (thread->state & TH_HALTED) {
			return KERN_SUCCESS;
		}

		/*
		 *	If the thread is at a nice continuation,
		 *	or a continuation with a cleanup routine,
		 *	call the cleanup routine.
		 */
		if ((((thread->swap_func == mach_msg_continue) ||
		      (thread->swap_func == mach_msg_receive_continue)) &&
		     mach_msg_interrupt(thread)) ||
		    (thread->swap_func == thread_exception_return) ||
		    (thread->swap_func == thread_bootstrap_return)) {
			s = splsched();
			thread_lock(thread);
			thread->state |= TH_HALTED;
			thread_ast_clear(thread, AST_HALT);
			thread_unlock(thread);
			splx(s);

			return KERN_SUCCESS;
		}

		/*
		 *	Force the thread to stop at a clean
		 *	point, and arrange to wait for it.
		 *
		 *	Set it running, so it can notice.  Override
		 *	the suspend count.  We know that the thread
		 *	is suspended and not waiting.
		 *
		 *	Since the thread may hit an interruptible wait
		 *	before it reaches a clean point, we must force it
		 *	to wake us up when it does so.  This involves some
		 *	trickery:
		 *	  We mark the thread SUSPENDED so that thread_block
		 *	will suspend it and wake us up.
		 *	  We mark the thread RUNNING so that it will run.
		 *	  We mark the thread UN-INTERRUPTIBLE (!) so that
		 *	some other thread trying to halt or suspend it won't
		 *	take it off the run queue before it runs.  Since
		 *	dispatching a thread (the tail of thread_invoke) marks
		 *	the thread interruptible, it will stop at the next
		 *	context switch or interruptible wait.
		 */

		s = splsched();
		thread_lock(thread);
		if ((thread->state & TH_SCHED_STATE) != TH_SUSP)
			panic("thread_halt");
		thread->state |= TH_RUN | TH_UNINT;
		thread_setrun(thread, FALSE);

		/*
		 *	Continue loop and wait for thread to stop.
		 */
	}
}

void	walking_zombie(void)
{
	panic("the zombie walks!");
}

/*
 *	Thread calls this routine on exit from the kernel when it
 *	notices a halt request.
 */
void	thread_halt_self(void)
{
	register thread_t	thread = current_thread();
	spl_t	s;

	if (thread->ast & AST_TERMINATE) {
		/*
		 *	Thread is terminating itself.  Shut
		 *	down IPC, then queue it up for the
		 *	reaper thread.
		 */
		ipc_thread_terminate(thread);
#if	NET_ATM
		mk_waited_collect(thread);
#endif

		thread_hold(thread);

		s = splsched();
		simple_lock(&reaper_lock);
		enqueue_tail(&reaper_queue, (queue_entry_t) thread);
		simple_unlock(&reaper_lock);

		thread_lock(thread);
		thread->state |= TH_HALTED;
		thread_unlock(thread);
		(void) splx(s);

		thread_wakeup((event_t)&reaper_queue);
		counter(c_thread_halt_self_block++);
		thread_block(walking_zombie);
		/*NOTREACHED*/
	} else {
		/*
		 *	Thread was asked to halt - show that it
		 *	has done so.
		 */
		s = splsched();
		thread_lock(thread);
		thread->state |= TH_HALTED;
		thread_ast_clear(thread, AST_HALT);
		thread_unlock(thread);
		splx(s);
		counter(c_thread_halt_self_block++);
		thread_block(thread_exception_return);
		/*
		 *	thread_release resets TH_HALTED.
		 */
	}
}

/*
 *	thread_hold:
 *
 *	Suspend execution of the specified thread.
 *	This is a recursive-style suspension of the thread, a count of
 *	suspends is maintained.
 */
void thread_hold(
	register thread_t	thread)
{
	spl_t			s;

	s = splsched();
	thread_lock(thread);
	thread->suspend_count++;
	thread->state |= TH_SUSP;
	thread_unlock(thread);
	(void) splx(s);
}

/*
 *	thread_dowait:
 *
 *	Wait for a thread to actually enter stopped state.
 *
 *	must_halt argument indicates if this may fail on interruption.
 *	This is FALSE only if called from thread_abort via thread_halt.
 */
kern_return_t
thread_dowait(
	register thread_t	thread,
	boolean_t		must_halt)
{
	register boolean_t	need_wakeup;
	register kern_return_t	ret = KERN_SUCCESS;
	spl_t			s;

	if (thread == current_thread())
		panic("thread_dowait");

	/*
	 *	If a thread is not interruptible, it may not be suspended
	 *	until it becomes interruptible.  In this case, we wait for
	 *	the thread to stop itself, and indicate that we are waiting
	 *	for it to stop so that it can wake us up when it does stop.
	 *
	 *	If the thread is interruptible, we may be able to suspend
	 *	it immediately.  There are several cases:
	 *
	 *	1) The thread is already stopped (trivial)
	 *	2) The thread is runnable (marked RUN and on a run queue).
	 *	   We pull it off the run queue and mark it stopped.
	 *	3) The thread is running.  We wait for it to stop.
	 */

	need_wakeup = FALSE;
	s = splsched();
	thread_lock(thread);

	for (;;) {
	    switch (thread->state & TH_SCHED_STATE) {
		case			TH_SUSP:
		case	      TH_WAIT | TH_SUSP:
		    /*
		     *	Thread is already suspended, or sleeping in an
		     *	interruptible wait.  We win!
		     */
		    break;

		case TH_RUN	      | TH_SUSP:
		    /*
		     *	The thread is interruptible.  If we can pull
		     *	it off a runq, stop it here.
		     */
		    if (rem_runq(thread) != RUN_QUEUE_NULL) {
			thread->state &= ~TH_RUN;
			need_wakeup = thread->wake_active;
			thread->wake_active = FALSE;
			break;
		    }
#if	NCPUS > 1
		    /*
		     *	The thread must be running, so make its
		     *	processor execute ast_check().  This
		     *	should cause the thread to take an ast and
		     *	context switch to suspend for us.
		     */
		    cause_ast_check(thread->last_processor);
#endif	/* NCPUS > 1 */

		    /*
		     *	Fall through to wait for thread to stop.
		     */

		case TH_RUN	      | TH_SUSP | TH_UNINT:
		case TH_RUN | TH_WAIT | TH_SUSP:
		case TH_RUN | TH_WAIT | TH_SUSP | TH_UNINT:
		case	      TH_WAIT | TH_SUSP | TH_UNINT:
		    /*
		     *	Wait for the thread to stop, or sleep interruptibly
		     *	(thread_block will stop it in the latter case).
		     *	Check for failure if interrupted.
		     */
		    thread->wake_active = TRUE;
		    thread_sleep((event_t) &thread->wake_active,
				simple_lock_addr(thread->lock), TRUE);
		    thread_lock(thread);
		    if ((current_thread()->wait_result != THREAD_AWAKENED) &&
			    !must_halt) {
			ret = KERN_FAILURE;
			break;
		    }

		    /*
		     *	Repeat loop to check thread`s state.
		     */
		    continue;
	    }
	    /*
	     *	Thread is stopped at this point.
	     */
	    break;
	}

	thread_unlock(thread);
	(void) splx(s);

	if (need_wakeup)
	    thread_wakeup((event_t) &thread->wake_active);

	return ret;
}

void thread_release(
	register thread_t	thread)
{
	spl_t			s;

	s = splsched();
	thread_lock(thread);
	if (--thread->suspend_count == 0) {
		thread->state &= ~(TH_SUSP | TH_HALTED);
		if ((thread->state & (TH_WAIT | TH_RUN)) == 0) {
			/* was only suspended */
			thread->state |= TH_RUN;
			thread_setrun(thread, TRUE);
		}
	}
	thread_unlock(thread);
	(void) splx(s);
}

kern_return_t thread_suspend(
	register thread_t	thread)
{
	register boolean_t	hold;
	spl_t			spl;

	if (thread == THREAD_NULL)
		return KERN_INVALID_ARGUMENT;

	hold = FALSE;
	spl = splsched();
	thread_lock(thread);
	if (thread->user_stop_count++ == 0) {
		hold = TRUE;
		thread->suspend_count++;
		thread->state |= TH_SUSP;
	}
	thread_unlock(thread);
	(void) splx(spl);

	/*
	 *	Now  wait for the thread if necessary.
	 */
	if (hold) {
		if (thread == current_thread()) {
			/*
			 *	We want to call thread_block on our way out,
			 *	to stop running.
			 */
			spl = splsched();
			ast_on(cpu_number(), AST_BLOCK);
			(void) splx(spl);
		} else
			(void) thread_dowait(thread, TRUE);
	}
	return KERN_SUCCESS;
}


kern_return_t thread_resume(
	register thread_t	thread)
{
	register kern_return_t	ret;
	spl_t			s;

	if (thread == THREAD_NULL)
		return KERN_INVALID_ARGUMENT;

	ret = KERN_SUCCESS;

	s = splsched();
	thread_lock(thread);
	if (thread->user_stop_count > 0) {
	    if (--thread->user_stop_count == 0) {
		if (--thread->suspend_count == 0) {
		    thread->state &= ~(TH_SUSP | TH_HALTED);
		    if ((thread->state & (TH_WAIT | TH_RUN)) == 0) {
			    /* was only suspended */
			    thread->state |= TH_RUN;
			    thread_setrun(thread, TRUE);
		    }
		}
	    }
	}
	else {
		ret = KERN_FAILURE;
	}

	thread_unlock(thread);
	(void) splx(s);

	return ret;
}

/*
 *	Return thread's machine-dependent state.
 */
kern_return_t thread_get_state(
	register thread_t	thread,
	int			flavor,
	thread_state_t		old_state,	/* pointer to OUT array */
	natural_t		*old_state_count)	/*IN/OUT*/
{
	kern_return_t		ret;

	if (thread == THREAD_NULL || thread == current_thread()) {
		return KERN_INVALID_ARGUMENT;
	}

	thread_hold(thread);
	(void) thread_dowait(thread, TRUE);

	ret = thread_getstatus(thread, flavor, old_state, old_state_count);

	thread_release(thread);
	return ret;
}

/*
 *	Change thread's machine-dependent state.
 */
kern_return_t thread_set_state(
	register thread_t	thread,
	int			flavor,
	thread_state_t		new_state,
	natural_t		new_state_count)
{
	kern_return_t		ret;

	if (thread == THREAD_NULL || thread == current_thread()) {
		return KERN_INVALID_ARGUMENT;
	}

	thread_hold(thread);
	(void) thread_dowait(thread, TRUE);

	ret = thread_setstatus(thread, flavor, new_state, new_state_count);

	thread_release(thread);
	return ret;
}

kern_return_t thread_info(
	register thread_t	thread,
	int			flavor,
	thread_info_t		thread_info_out,    /* pointer to OUT array */
	natural_t		*thread_info_count) /*IN/OUT*/
{
	int			state, flags;
	spl_t			s;

	if (thread == THREAD_NULL)
		return KERN_INVALID_ARGUMENT;

	if (flavor == THREAD_BASIC_INFO) {
	    register thread_basic_info_t	basic_info;

	    if (*thread_info_count < THREAD_BASIC_INFO_COUNT) {
		return KERN_INVALID_ARGUMENT;
	    }

	    basic_info = (thread_basic_info_t) thread_info_out;

	    s = splsched();
	    thread_lock(thread);

	    /*
	     *	Update lazy-evaluated scheduler info because someone wants it.
	     */
	    if ((thread->state & TH_RUN) == 0 &&
		thread->sched_stamp != sched_tick)
		    update_priority(thread);

	    /* fill in info */

	    thread_read_times(thread,
			&basic_info->user_time,
			&basic_info->system_time);
	    basic_info->base_priority	= thread->priority;
	    basic_info->cur_priority	= thread->sched_pri;

	    /*
	     *	To calculate cpu_usage, first correct for timer rate,
	     *	then for 5/8 ageing.  The correction factor [3/5] is
	     *	(1/(5/8) - 1).
	     */
	    basic_info->cpu_usage = thread->cpu_usage /
					(TIMER_RATE/TH_USAGE_SCALE);
	    basic_info->cpu_usage = (basic_info->cpu_usage * 3) / 5;
#if	SIMPLE_CLOCK
	    /*
	     *	Clock drift compensation.
	     */
	    basic_info->cpu_usage =
		(basic_info->cpu_usage * 1000000)/sched_usec;
#endif	/* SIMPLE_CLOCK */

	    if (thread->state & TH_SWAPPED)
		flags = TH_FLAGS_SWAPPED;
	    else if (thread->state & TH_IDLE)
		flags = TH_FLAGS_IDLE;
	    else
		flags = 0;

	    if (thread->state & TH_HALTED)
		state = TH_STATE_HALTED;
	    else
	    if (thread->state & TH_RUN)
		state = TH_STATE_RUNNING;
	    else
	    if (thread->state & TH_UNINT)
		state = TH_STATE_UNINTERRUPTIBLE;
	    else
	    if (thread->state & TH_SUSP)
		state = TH_STATE_STOPPED;
	    else
	    if (thread->state & TH_WAIT)
		state = TH_STATE_WAITING;
	    else
		state = 0;		/* ? */

	    basic_info->run_state = state;
	    basic_info->flags = flags;
	    basic_info->suspend_count = thread->user_stop_count;
	    if (state == TH_STATE_RUNNING)
		basic_info->sleep_time = 0;
	    else
		basic_info->sleep_time = sched_tick - thread->sched_stamp;

	    thread_unlock(thread);
	    splx(s);

	    *thread_info_count = THREAD_BASIC_INFO_COUNT;
	    return KERN_SUCCESS;
	}
	else if (flavor == THREAD_SCHED_INFO) {
	    register thread_sched_info_t	sched_info;

	    if (*thread_info_count < THREAD_SCHED_INFO_COUNT) {
		return KERN_INVALID_ARGUMENT;
	    }

	    sched_info = (thread_sched_info_t) thread_info_out;

	    s = splsched();
	    thread_lock(thread);

#if	MACH_FIXPRI
	    sched_info->policy = thread->policy;
	    if (thread->policy == POLICY_FIXEDPRI) {
		sched_info->data = (thread->sched_data * tick)/1000;
	    }
	    else {
		sched_info->data = 0;
	    }
#else	/* MACH_FIXPRI */
	    sched_info->policy = POLICY_TIMESHARE;
	    sched_info->data = 0;
#endif	/* MACH_FIXPRI */

	    sched_info->base_priority = thread->priority;
	    sched_info->max_priority = thread->max_priority;
	    sched_info->cur_priority = thread->sched_pri;
	    
	    sched_info->depressed = (thread->depress_priority >= 0);
	    sched_info->depress_priority = thread->depress_priority;

	    thread_unlock(thread);
	    splx(s);

	    *thread_info_count = THREAD_SCHED_INFO_COUNT;
	    return KERN_SUCCESS;
	}

	return KERN_INVALID_ARGUMENT;
}

kern_return_t	thread_abort(
	register thread_t	thread)
{
	if (thread == THREAD_NULL || thread == current_thread()) {
		return KERN_INVALID_ARGUMENT;
	}

	/*
	 *
         *	clear it of an event wait 
         */
	evc_notify_abort(thread);

	/*
	 *	Try to force the thread to a clean point
	 *	If the halt operation fails return KERN_ABORTED.
	 *	ipc code will convert this to an ipc interrupted error code.
	 */
	if (thread_halt(thread, FALSE) != KERN_SUCCESS)
		return KERN_ABORTED;

	/*
	 *	If the thread was in an exception, abort that too.
	 */
	mach_msg_abort_rpc(thread);

	/*
	 *	Then set it going again.
	 */
	thread_release(thread);

	/*
	 *	Also abort any depression.
	 */
	if (thread->depress_priority != -1)
		thread_depress_abort(thread);

	return KERN_SUCCESS;
}

/*
 *	thread_start:
 *
 *	Start a thread at the specified routine.
 *	The thread must	be in a swapped state.
 */

void
thread_start(
	thread_t	thread,
	continuation_t	start)
{
	thread->swap_func = start;
}

/*
 *	kernel_thread:
 *
 *	Start up a kernel thread in the specified task.
 */

thread_t kernel_thread(
	task_t		task,
	continuation_t	start,
	void *		arg)
{
	thread_t	thread;

	(void) thread_create(task, &thread);
	/* release "extra" ref that thread_create gave us */
	thread_deallocate(thread);
	thread_start(thread, start);
	thread->ith_other = arg;

	/*
	 *	We ensure that the kernel thread starts with a stack.
	 *	The swapin mechanism might not be operational yet.
	 */
	thread_doswapin(thread);
	thread->max_priority = BASEPRI_SYSTEM;
	thread->priority = BASEPRI_SYSTEM;
	thread->sched_pri = BASEPRI_SYSTEM;
	(void) thread_resume(thread);
	return thread;
}

/*
 *	reaper_thread:
 *
 *	This kernel thread runs forever looking for threads to destroy
 *	(when they request that they be destroyed, of course).
 */
void reaper_thread_continue(void)
{
	for (;;) {
		register thread_t thread;
		spl_t s;

		s = splsched();
		simple_lock(&reaper_lock);

		while ((thread = (thread_t) dequeue_head(&reaper_queue))
							!= THREAD_NULL) {
			simple_unlock(&reaper_lock);
			(void) splx(s);

			(void) thread_dowait(thread, TRUE);	/* may block */
			thread_deallocate(thread);		/* may block */

			s = splsched();
			simple_lock(&reaper_lock);
		}

		assert_wait((event_t) &reaper_queue, FALSE);
		simple_unlock(&reaper_lock);
		(void) splx(s);
		counter(c_reaper_thread_block++);
		thread_block(reaper_thread_continue);
	}
}

void reaper_thread(void)
{
	reaper_thread_continue();
	/*NOTREACHED*/
}

#if	MACH_HOST
/*
 *	thread_assign:
 *
 *	Change processor set assignment.
 *	Caller must hold an extra reference to the thread (if this is
 *	called directly from the ipc interface, this is an operation
 *	in progress reference).  Caller must hold no locks -- this may block.
 */

kern_return_t
thread_assign(
	thread_t	thread,
	processor_set_t	new_pset)
{
	if (thread == THREAD_NULL || new_pset == PROCESSOR_SET_NULL) {
		return KERN_INVALID_ARGUMENT;
	}

	thread_freeze(thread);
	thread_doassign(thread, new_pset, TRUE);

	return KERN_SUCCESS;
}

/*
 *	thread_freeze:
 *
 *	Freeze thread's assignment.  Prelude to assigning thread.
 *	Only one freeze may be held per thread.  
 */
void
thread_freeze(
	thread_t	thread)
{
	spl_t	s;
	/*
	 *	Freeze the assignment, deferring to a prior freeze.
	 */
	s = splsched();
	thread_lock(thread);
	while (thread->may_assign == FALSE) {
		thread->assign_active = TRUE;
		thread_sleep((event_t) &thread->assign_active,
			simple_lock_addr(thread->lock), FALSE);
		thread_lock(thread);
	}
	thread->may_assign = FALSE;
	thread_unlock(thread);
	(void) splx(s);

}

/*
 *	thread_unfreeze: release freeze on thread's assignment.
 */
void
thread_unfreeze(
	thread_t	thread)
{
	spl_t 	s;

	s = splsched();
	thread_lock(thread);
	thread->may_assign = TRUE;
	if (thread->assign_active) {
		thread->assign_active = FALSE;
		thread_wakeup((event_t)&thread->assign_active);
	}
	thread_unlock(thread);
	splx(s);
}

/*
 *	thread_doassign:
 *
 *	Actually do thread assignment.  thread_will_assign must have been
 *	called on the thread.  release_freeze argument indicates whether
 *	to release freeze on thread.
 */

void
thread_doassign(
	register thread_t		thread,
	register processor_set_t	new_pset,
	boolean_t			release_freeze)
{
	register processor_set_t	pset;
	register boolean_t		old_empty, new_empty;
	boolean_t			recompute_pri = FALSE;
	spl_t				s;
	
	/*
	 *	Check for silly no-op.
	 */
	pset = thread->processor_set;
	if (pset == new_pset) {
		if (release_freeze)
			thread_unfreeze(thread);
		return;
	}
	/*
	 *	Suspend the thread and stop it if it's not the current thread.
	 */
	thread_hold(thread);
	if (thread != current_thread())
		(void) thread_dowait(thread, TRUE);

	/*
	 *	Lock both psets now, use ordering to avoid deadlocks.
	 */
Restart:
	if ((vm_offset_t)pset < (vm_offset_t)new_pset) {
	    pset_lock(pset);
	    pset_lock(new_pset);
	}
	else {
	    pset_lock(new_pset);
	    pset_lock(pset);
	}

	/*
	 *	Check if new_pset is ok to assign to.  If not, reassign
	 *	to default_pset.
	 */
	if (!new_pset->active) {
	    pset_unlock(pset);
	    pset_unlock(new_pset);
	    new_pset = &default_pset;
	    goto Restart;
	}

	pset_reference(new_pset);

	/*
	 *	Grab the thread lock and move the thread.
	 *	Then drop the lock on the old pset and the thread's
	 *	reference to it.
	 */
	s = splsched();
	thread_lock(thread);

	thread_change_psets(thread, pset, new_pset);

	old_empty = pset->empty;
	new_empty = new_pset->empty;

	pset_unlock(pset);

	/*
	 *	Reset policy and priorities if needed.
	 */
#if	MACH_FIXPRI
	if (thread->policy & new_pset->policies == 0) {
	    thread->policy = POLICY_TIMESHARE;
	    recompute_pri = TRUE;
	}
#endif	/* MACH_FIXPRI */

	if (thread->max_priority < new_pset->max_priority) {
	    thread->max_priority = new_pset->max_priority;
	    if (thread->priority < thread->max_priority) {
		thread->priority = thread->max_priority;
		recompute_pri = TRUE;
	    }
	    else {
		if ((thread->depress_priority >= 0) &&
		    (thread->depress_priority < thread->max_priority)) {
			thread->depress_priority = thread->max_priority;
		}
	    }
	}

	pset_unlock(new_pset);

	if (recompute_pri)
		compute_priority(thread, TRUE);

	if (release_freeze) {
		thread->may_assign = TRUE;
		if (thread->assign_active) {
			thread->assign_active = FALSE;
			thread_wakeup((event_t)&thread->assign_active);
		}
	}

	thread_unlock(thread);
	splx(s);

	pset_deallocate(pset);

	/*
	 *	Figure out hold status of thread.  Threads assigned to empty
	 *	psets must be held.  Therefore:
	 *		If old pset was empty release its hold.
	 *		Release our hold from above unless new pset is empty.
	 */

	if (old_empty)
		thread_release(thread);
	if (!new_empty)
		thread_release(thread);

	/*
	 *	If current_thread is assigned, context switch to force
	 *	assignment to happen.  This also causes hold to take
	 *	effect if the new pset is empty.
	 */
	if (thread == current_thread()) {
		s = splsched();
		ast_on(cpu_number(), AST_BLOCK);
		(void) splx(s);
	}
}
#else	/* MACH_HOST */
kern_return_t
thread_assign(
	thread_t	thread,
	processor_set_t	new_pset)
{
	return KERN_FAILURE;
}
#endif	/* MACH_HOST */

/*
 *	thread_assign_default:
 *
 *	Special version of thread_assign for assigning threads to default
 *	processor set.
 */
kern_return_t
thread_assign_default(
	thread_t	thread)
{
	return thread_assign(thread, &default_pset);
}

/*
 *	thread_get_assignment
 *
 *	Return current assignment for this thread.
 */	    
kern_return_t thread_get_assignment(
	thread_t	thread,
	processor_set_t	*pset)
{
	*pset = thread->processor_set;
	pset_reference(*pset);
	return KERN_SUCCESS;
}

/*
 *	thread_priority:
 *
 *	Set priority (and possibly max priority) for thread.
 */
kern_return_t
thread_priority(
	thread_t	thread,
	int		priority,
	boolean_t	set_max)
{
    spl_t		s;
    kern_return_t	ret = KERN_SUCCESS;

    if ((thread == THREAD_NULL) || invalid_pri(priority))
	return KERN_INVALID_ARGUMENT;

    s = splsched();
    thread_lock(thread);

    /*
     *	Check for violation of max priority
     */
    if (priority < thread->max_priority) {
	ret = KERN_FAILURE;
    }
    else {
	/*
	 *	Set priorities.  If a depression is in progress,
	 *	change the priority to restore.
	 */
	if (thread->depress_priority >= 0) {
	    thread->depress_priority = priority;
	}
	else {
	    thread->priority = priority;
	    compute_priority(thread, TRUE);
	}

	if (set_max)
	    thread->max_priority = priority;
    }
    thread_unlock(thread);
    (void) splx(s);

    return ret;
}

/*
 *	thread_set_own_priority:
 *
 *	Internal use only; sets the priority of the calling thread.
 *	Will adjust max_priority if necessary.
 */
void
thread_set_own_priority(
	int	priority)
{
    spl_t	s;
    thread_t	thread = current_thread();

    s = splsched();
    thread_lock(thread);

    if (priority < thread->max_priority)
	thread->max_priority = priority;
    thread->priority = priority;
    compute_priority(thread, TRUE);

    thread_unlock(thread);
    (void) splx(s);
}

/*
 *	thread_max_priority:
 *
 *	Reset the max priority for a thread.
 */
kern_return_t
thread_max_priority(
	thread_t	thread,
	processor_set_t	pset,
	int		max_priority)
{
    spl_t		s;
    kern_return_t	ret = KERN_SUCCESS;

    if ((thread == THREAD_NULL) || (pset == PROCESSOR_SET_NULL) ||
    	invalid_pri(max_priority))
	    return KERN_INVALID_ARGUMENT;

    s = splsched();
    thread_lock(thread);

#if	MACH_HOST
    /*
     *	Check for wrong processor set.
     */
    if (pset != thread->processor_set) {
	ret = KERN_FAILURE;
    }
    else {
#endif	/* MACH_HOST */
	thread->max_priority = max_priority;

	/*
	 *	Reset priority if it violates new max priority
	 */
	if (max_priority > thread->priority) {
	    thread->priority = max_priority;

	    compute_priority(thread, TRUE);
	}
	else {
	    if (thread->depress_priority >= 0 &&
		max_priority > thread->depress_priority)
		    thread->depress_priority = max_priority;
	    }
#if	MACH_HOST
    }
#endif	/* MACH_HOST */

    thread_unlock(thread);
    (void) splx(s);

    return ret;
}

/*
 *	thread_policy:
 *
 *	Set scheduling policy for thread.
 */
kern_return_t
thread_policy(
	thread_t	thread,
	int		policy,
	int		data)
{
#if	MACH_FIXPRI
	register kern_return_t	ret = KERN_SUCCESS;
	register int	temp;
	spl_t		s;
#endif	/* MACH_FIXPRI */

	if ((thread == THREAD_NULL) || invalid_policy(policy))
		return KERN_INVALID_ARGUMENT;

#if	MACH_FIXPRI
	s = splsched();
	thread_lock(thread);

	/*
	 *	Check if changing policy.
	 */
	if (policy == thread->policy) {
	    /*
	     *	Just changing data.  This is meaningless for
	     *	timesharing, quantum for fixed priority (but
	     *	has no effect until current quantum runs out).
	     */
	    if (policy == POLICY_FIXEDPRI) {
		temp = data * 1000;
		if (temp % tick)
			temp += tick;
		thread->sched_data = temp/tick;
	    }
	}
	else {
	    /*
	     *	Changing policy.  Check if new policy is allowed.
	     */
	    if ((thread->processor_set->policies & policy) == 0) {
		    ret = KERN_FAILURE;
	    }
	    else {
		/*
		 *	Changing policy.  Save data and calculate new
		 *	priority.
		 */
		thread->policy = policy;
		if (policy == POLICY_FIXEDPRI) {
			temp = data * 1000;
			if (temp % tick)
				temp += tick;
			thread->sched_data = temp/tick;
		}
		compute_priority(thread, TRUE);
	    }
	}
	thread_unlock(thread);
	(void) splx(s);

	return ret;
#else	/* MACH_FIXPRI */
	if (policy == POLICY_TIMESHARE)
		return KERN_SUCCESS;
	else
		return KERN_FAILURE;
#endif	/* MACH_FIXPRI */
}

/*
 *	thread_wire:
 *
 *	Specify that the target thread must always be able
 *	to run and to allocate memory.
 */
kern_return_t
thread_wire(
	host_t		host,
	thread_t	thread,
	boolean_t	wired)
{
	spl_t		s;

	if (host == HOST_NULL)
	    return KERN_INVALID_ARGUMENT;

	if (thread == THREAD_NULL)
	    return KERN_INVALID_ARGUMENT;

	/*
	 * This implementation only works for the current thread.
	 * See stack_privilege.
	 */
	if (thread != current_thread())
	    return KERN_INVALID_ARGUMENT;

	s = splsched();
	thread_lock(thread);

	if (wired) {
	    thread->vm_privilege = TRUE;
	    stack_privilege(thread);
	}
	else {
	    thread->vm_privilege = FALSE;
/*XXX	    stack_unprivilege(thread); */
	    thread->stack_privilege = 0;
	}

	thread_unlock(thread);
	splx(s);

	return KERN_SUCCESS;
}

/*
 *	thread_collect_scan:
 *
 *	Attempt to free resources owned by threads.
 *	pcb_collect doesn't do anything yet.
 */

void thread_collect_scan(void)
{
#if	0
	register thread_t	thread, prev_thread;
	processor_set_t		pset, prev_pset;

	prev_thread = THREAD_NULL;
	prev_pset = PROCESSOR_SET_NULL;

	simple_lock(&all_psets_lock);
	queue_iterate(&all_psets, pset, processor_set_t, all_psets) {
		pset_lock(pset);
		queue_iterate(&pset->threads, thread, thread_t, pset_threads) {
			spl_t	s = splsched();
			thread_lock(thread);

			/*
			 *	Only collect threads which are
			 *	not runnable and are swapped.
			 */

			if ((thread->state & (TH_RUN|TH_SWAPPED))
							== TH_SWAPPED) {
				thread->ref_count++;
				thread_unlock(thread);
				(void) splx(s);
				pset->ref_count++;
				pset_unlock(pset);
				simple_unlock(&all_psets_lock);

				pcb_collect(thread);

				if (prev_thread != THREAD_NULL)
					thread_deallocate(prev_thread);
				prev_thread = thread;

				if (prev_pset != PROCESSOR_SET_NULL)
					pset_deallocate(prev_pset);
				prev_pset = pset;

				simple_lock(&all_psets_lock);
				pset_lock(pset);
			} else {
				thread_unlock(thread);
				(void) splx(s);
			}
		}
		pset_unlock(pset);
	}
	simple_unlock(&all_psets_lock);

	if (prev_thread != THREAD_NULL)
		thread_deallocate(prev_thread);
	if (prev_pset != PROCESSOR_SET_NULL)
		pset_deallocate(prev_pset);
#endif	/* 0 */
}

boolean_t thread_collect_allowed = TRUE;
unsigned thread_collect_last_tick = 0;
unsigned thread_collect_max_rate = 0;		/* in ticks */

/*
 *	consider_thread_collect:
 *
 *	Called by the pageout daemon when the system needs more free pages.
 */

void consider_thread_collect(void)
{
	/*
	 *	By default, don't attempt thread collection more frequently
	 *	than once a second.
	 */

	if (thread_collect_max_rate == 0)
		thread_collect_max_rate = hz;

	if (thread_collect_allowed &&
	    (sched_tick >
	     (thread_collect_last_tick + thread_collect_max_rate))) {
		thread_collect_last_tick = sched_tick;
		thread_collect_scan();
	}
}

#if	MACH_DEBUG

vm_size_t stack_usage(
	register vm_offset_t stack)
{
	int i;

	for (i = 0; i < KERNEL_STACK_SIZE/sizeof(unsigned int); i++)
	    if (((unsigned int *)stack)[i] != STACK_MARKER)
		break;

	return KERNEL_STACK_SIZE - i * sizeof(unsigned int);
}

/*
 *	Machine-dependent code should call stack_init
 *	before doing its own initialization of the stack.
 */

void stack_init(
	register vm_offset_t stack)
{
	if (stack_check_usage) {
	    int i;

	    for (i = 0; i < KERNEL_STACK_SIZE/sizeof(unsigned int); i++)
		((unsigned int *)stack)[i] = STACK_MARKER;
	}
}

/*
 *	Machine-dependent code should call stack_finalize
 *	before releasing the stack memory.
 */

void stack_finalize(
	register vm_offset_t stack)
{
	if (stack_check_usage) {
	    vm_size_t used = stack_usage(stack);

	    simple_lock(&stack_usage_lock);
	    if (used > stack_max_usage)
		stack_max_usage = used;
	    simple_unlock(&stack_usage_lock);
	}
}

#ifndef	MACHINE_STACK
/*
 *	stack_statistics:
 *
 *	Return statistics on cached kernel stacks.
 *	*maxusagep must be initialized by the caller.
 */

void stack_statistics(
	natural_t *totalp,
	vm_size_t *maxusagep)
{
	spl_t	s;

	s = splsched();
	stack_lock();
	if (stack_check_usage) {
		vm_offset_t stack;

		/*
		 *	This is pretty expensive to do at splsched,
		 *	but it only happens when someone makes
		 *	a debugging call, so it should be OK.
		 */

		for (stack = stack_free_list; stack != 0;
		     stack = stack_next(stack)) {
			vm_size_t usage = stack_usage(stack);

			if (usage > *maxusagep)
				*maxusagep = usage;
		}
	}

	*totalp = stack_free_count;
	stack_unlock();
	(void) splx(s);
}
#endif	/* MACHINE_STACK */

kern_return_t host_stack_usage(
	host_t		host,
	vm_size_t	*reservedp,
	unsigned int	*totalp,
	vm_size_t	*spacep,
	vm_size_t	*residentp,
	vm_size_t	*maxusagep,
	vm_offset_t	*maxstackp)
{
	unsigned int total;
	vm_size_t maxusage;

	if (host == HOST_NULL)
		return KERN_INVALID_HOST;

	simple_lock(&stack_usage_lock);
	maxusage = stack_max_usage;
	simple_unlock(&stack_usage_lock);

	stack_statistics(&total, &maxusage);

	*reservedp = 0;
	*totalp = total;
	*spacep = *residentp = total * round_page(KERNEL_STACK_SIZE);
	*maxusagep = maxusage;
	*maxstackp = 0;
	return KERN_SUCCESS;
}

kern_return_t processor_set_stack_usage(
	processor_set_t	pset,
	unsigned int	*totalp,
	vm_size_t	*spacep,
	vm_size_t	*residentp,
	vm_size_t	*maxusagep,
	vm_offset_t	*maxstackp)
{
	unsigned int total;
	vm_size_t maxusage;
	vm_offset_t maxstack;

	register thread_t *threads;
	register thread_t tmp_thread;

	unsigned int actual;	/* this many things */
	unsigned int i;

	vm_size_t size, size_needed;
	vm_offset_t addr;

	if (pset == PROCESSOR_SET_NULL)
		return KERN_INVALID_ARGUMENT;

	size = 0; addr = 0;

	for (;;) {
		pset_lock(pset);
		if (!pset->active) {
			pset_unlock(pset);
			return KERN_INVALID_ARGUMENT;
		}

		actual = pset->thread_count;

		/* do we have the memory we need? */

		size_needed = actual * sizeof(thread_t);
		if (size_needed <= size)
			break;

		/* unlock the pset and allocate more memory */
		pset_unlock(pset);

		if (size != 0)
			kfree(addr, size);

		assert(size_needed > 0);
		size = size_needed;

		addr = kalloc(size);
		if (addr == 0)
			return KERN_RESOURCE_SHORTAGE;
	}

	/* OK, have memory and the processor_set is locked & active */

	threads = (thread_t *) addr;
	for (i = 0, tmp_thread = (thread_t) queue_first(&pset->threads);
	     i < actual;
	     i++,
	     tmp_thread = (thread_t) queue_next(&tmp_thread->pset_threads)) {
		thread_reference(tmp_thread);
		threads[i] = tmp_thread;
	}
	assert(queue_end(&pset->threads, (queue_entry_t) tmp_thread));

	/* can unlock processor set now that we have the thread refs */
	pset_unlock(pset);

	/* calculate maxusage and free thread references */

	total = 0;
	maxusage = 0;
	maxstack = 0;
	for (i = 0; i < actual; i++) {
		thread_t thread = threads[i];
		vm_offset_t stack = 0;

		/*
		 *	thread->kernel_stack is only accurate if the
		 *	thread isn't swapped and is not executing.
		 *
		 *	Of course, we don't have the appropriate locks
		 *	for these shenanigans.
		 */

		if ((thread->state & TH_SWAPPED) == 0) {
			int cpu;

			stack = thread->kernel_stack;

			for (cpu = 0; cpu < NCPUS; cpu++)
				if (active_threads[cpu] == thread) {
					stack = active_stacks[cpu];
					break;
				}
		}

		if (stack != 0) {
			total++;

			if (stack_check_usage) {
				vm_size_t usage = stack_usage(stack);

				if (usage > maxusage) {
					maxusage = usage;
					maxstack = (vm_offset_t) thread;
				}
			}
		}

		thread_deallocate(thread);
	}

	if (size != 0)
		kfree(addr, size);

	*totalp = total;
	*residentp = *spacep = total * round_page(KERNEL_STACK_SIZE);
	*maxusagep = maxusage;
	*maxstackp = maxstack;
	return KERN_SUCCESS;
}

/*
 *	Useful in the debugger:
 */
void
thread_stats(void)
{
	register thread_t thread;
	int total = 0, rpcreply = 0;

	queue_iterate(&default_pset.threads, thread, thread_t, pset_threads) {
		total++;
		if (thread->ith_rpc_reply != IP_NULL)
			rpcreply++;
	}

	printf("%d total threads.\n", total);
	printf("%d using rpc_reply.\n", rpcreply);
}
#endif	/* MACH_DEBUG */
