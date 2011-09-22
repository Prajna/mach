/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
 * $Log:	startup.c,v $
 * Revision 2.26  93/05/15  18:55:03  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.25  93/01/14  17:36:31  danner
 * 	Invoke printf_init() for multiP interlock setup.
 * 	[92/12/01            af]
 * 
 * Revision 2.24  92/08/03  17:39:15  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.23  92/05/21  17:15:47  jfriedl
 * 	tried prototypes.
 * 	[92/05/20            jfriedl]
 * 
 * Revision 2.22  92/01/27  16:43:18  rpd
 * 	Fixed to finish vm/ipc initialization before calling machine_init.
 * 	[92/01/25            rpd]
 * 
 * Revision 2.21  92/01/14  16:44:57  rpd
 * 	Split vm_mem_init into vm_mem_bootstrap and vm_mem_init.
 * 	[91/12/29            rpd]
 * 
 * Revision 2.20  91/09/12  16:38:00  bohman
 * 	Changed launch_first_thread() to set active_stacks[] before
 * 	starting the first thread.
 * 	[91/09/11  17:08:23  bohman]
 * 
 * Revision 2.19  91/08/28  11:14:37  jsb
 * 	Start NORMA ipc and vm systems before user bootstrap.
 * 	[91/08/15  08:33:05  jsb]
 * 
 * Revision 2.18  91/08/03  18:18:59  jsb
 * 	Changed NORMA specific startup again.
 * 	[91/07/24  22:32:51  jsb]
 * 
 * Revision 2.17  91/07/31  17:48:00  dbg
 * 	Remove interrupt_stack_alloc - not all multiprocessors use it.
 * 
 * 	Revised scheduling state machine.
 * 	[91/07/30  17:05:11  dbg]
 * 
 * Revision 2.16  91/06/17  15:47:15  jsb
 * 	Renamed NORMA conditionals.
 * 	[91/06/17  10:51:38  jsb]
 * 
 * Revision 2.15  91/06/06  17:07:27  jsb
 * 	Changed NORMA_IPC specific startup.
 * 	[91/05/14  09:19:55  jsb]
 * 
 * Revision 2.14  91/05/18  14:33:30  rpd
 * 	Added argument to kernel_thread.
 * 	[91/04/03            rpd]
 * 
 * Revision 2.13  91/05/14  16:47:00  mrt
 * 	Correcting copyright
 * 
 * Revision 2.12  91/03/16  14:51:45  rpd
 * 	Updated for new kmem_alloc interface.
 * 	[91/03/03            rpd]
 * 	Removed thread_swappable.
 * 	[91/01/18            rpd]
 * 
 * Revision 2.11  91/02/05  17:29:18  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:17:40  mrt]
 * 
 * Revision 2.10  91/01/08  15:17:00  rpd
 * 	Swapin the startup thread and idle threads.
 * 	[90/11/20            rpd]
 * 
 * 	Removed swapout_thread.
 * 	[90/11/11            rpd]
 * 
 * Revision 2.9  90/10/12  18:07:34  rpd
 * 	Fixed call to thread_bind in start_kernel_threads.
 * 	Fix from Philippe Bernadat.
 * 	[90/10/10            rpd]
 * 
 * Revision 2.8  90/09/28  16:55:37  jsb
 * 	Added NORMA_IPC support.
 * 	[90/09/28  14:05:00  jsb]
 * 
 * Revision 2.7  90/09/09  14:32:47  rpd
 * 	Fixed setup_main to call pset_sys_init.
 * 	[90/08/30            rpd]
 * 
 * Revision 2.6  90/08/27  22:03:40  dbg
 * 	Pass processor argument to choose_thread.
 * 	Rename cpu_start to cpu_launch_first_thread to avoid conflict
 * 	with processor code.
 * 	[90/07/17            dbg]
 * 
 * Revision 2.5  90/06/02  14:56:12  rpd
 * 	Start sched_thread.  On multiprocessors, start action_thread
 * 	instead of shutdown_thread.
 * 	[90/04/28            rpd]
 * 	Updated to new scheduling technology.
 * 	[90/03/26  22:19:00  rpd]
 * 
 * Revision 2.4  90/01/11  11:44:11  dbg
 * 	If multiprocessor, start shutdown thread.  If uniprocessor,
 * 	don't allocate interrupt stacks.
 * 	[89/12/19            dbg]
 * 
 * 	Add call to start other CPUs if multiprocessor.
 * 	[89/12/01            dbg]
 * 
 * Revision 2.3  89/09/08  11:26:32  dbg
 * 	Move kalloc initialization to vm_init.
 * 	[89/09/06            dbg]
 * 
 * 	Move bootstrap initialization to kern/bootstrap.c.  Initialize
 * 	device service here.
 * 	[89/08/02            dbg]
 * 
 * 	Initialize kalloc package.
 * 	[89/07/11            dbg]
 * 
 * Revision 2.2  89/08/05  16:07:22  rwd
 * 	Call mappable_time_init.
 * 	[89/08/04  18:31:35  rwd]
 * 
 * 23-May-89  David Golub (dbg) at Carnegie-Mellon University
 *	Thread creation must be done by a running thread;
 *	thread_setrun (called by thread_resume & kernel_thread) may look
 *	at current thread.
 *
 * 19-May-89  David Golub (dbg) at Carnegie-Mellon University
 *	Don't export first_task and first_thread; they don't last very
 *	long.
 *
 * 15-Aug-88  David Golub (dbg) at Carnegie-Mellon University
 *	Initial threads are now part of the kernel task, not first-task.
 *	Create the bootstrap task as a kernel task.
 *
 *  8-Aug-88  David Golub (dbg) at Carnegie-Mellon University
 *	Modified for stand-alone MACH kernel.
 *
 *  1-Jul-88  David Golub (dbg) at Carnegie-Mellon University
 *	Created.
 *
 */

/*
 *	Mach kernel startup.
 */


#include <xpr_debug.h>
#include <cpus.h>
#include <mach_host.h>
#include <norma_ipc.h>
#include <norma_vm.h>

#include <mach/boolean.h>
#include <mach/machine.h>
#include <mach/task_special_ports.h>
#include <mach/vm_param.h>
#include <ipc/ipc_init.h>
#include <kern/cpu_number.h>
#include <kern/processor.h>
#include <kern/sched_prim.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <kern/thread_swap.h>
#include <kern/time_out.h>
#include <kern/timer.h>
#include <kern/zalloc.h>
#include <vm/vm_kern.h>
#include <vm/vm_map.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <machine/machspl.h>
#include <machine/pmap.h>
#include <sys/version.h>



extern void	vm_mem_init();
extern void	vm_mem_bootstrap();
extern void	init_timeout();
extern void	machine_init();

extern void	idle_thread();
extern void	vm_pageout();
extern void	reaper_thread();
extern void	swapin_thread();
extern void	sched_thread();

extern void	bootstrap_create();
extern void	device_service_create();

void cpu_launch_first_thread();		/* forward */
void start_kernel_threads();	/* forward */

#if	NCPUS > 1
extern void	start_other_cpus();
extern void	action_thread();
#endif	NCPUS > 1

/*
 *	Running in virtual memory, on the interrupt stack.
 *	Does not return.  Dispatches initial thread.
 *
 *	Assumes that master_cpu is set.
 */
void setup_main()
{
	thread_t		startup_thread;

	panic_init();
	printf_init();

	sched_init();
	vm_mem_bootstrap();
	ipc_bootstrap();
	vm_mem_init();
	ipc_init();

	/*
	 * As soon as the virtual memory system is up, we record
	 * that this CPU is using the kernel pmap.
	 */
	PMAP_ACTIVATE_KERNEL(master_cpu);

	init_timers();
	init_timeout();

#if	XPR_DEBUG
	xprbootstrap();
#endif	XPR_DEBUG

	timestamp_init();

	mapable_time_init();

	machine_init();

	machine_info.max_cpus = NCPUS;
	machine_info.memory_size = mem_size;
	machine_info.avail_cpus = 0;
	machine_info.major_version = KERNEL_MAJOR_VERSION;
	machine_info.minor_version = KERNEL_MINOR_VERSION;

	/*
	 *	Initialize the IPC, task, and thread subsystems.
	 */
	task_init();
	thread_init();
	swapper_init();
#if	MACH_HOST
	pset_sys_init();
#endif	MACH_HOST

	/*
	 *	Kick off the time-out driven routines by calling
	 *	them the first time.
	 */
	recompute_priorities();
	compute_mach_factor();
	
	/*
	 *	Create a kernel thread to start the other kernel
	 *	threads.  Thread_resume (from kernel_thread) calls
	 *	thread_setrun, which may look at current thread;
	 *	we must avoid this, since there is no current thread.
	 */

	/*
	 * Create the thread, and point it at the routine.
	 */
	(void) thread_create(kernel_task, &startup_thread);
	thread_start(startup_thread, start_kernel_threads);

	/*
	 * Give it a kernel stack.
	 */
	thread_doswapin(startup_thread);

	/*
	 * Pretend it is already running, and resume it.
	 * Since it looks as if it is running, thread_resume
	 * will not try to put it on the run queues.
	 *
	 * We can do all of this without locking, because nothing
	 * else is running yet.
	 */
	startup_thread->state |= TH_RUN;
	(void) thread_resume(startup_thread);

	/*
	 * Start the thread.
	 */
	cpu_launch_first_thread(startup_thread);
	/*NOTREACHED*/
}

/*
 * Now running in a thread.  Create the rest of the kernel threads
 * and the bootstrap task.
 */
void start_kernel_threads()
{
	register int	i;

	/*
	 *	Create the idle threads and the other
	 *	service threads.
	 */
	for (i = 0; i < NCPUS; i++) {
	    if (machine_slot[i].is_cpu) {
		thread_t	th;

		(void) thread_create(kernel_task, &th);
		thread_bind(th, cpu_to_processor(i));
		thread_start(th, idle_thread);
		thread_doswapin(th);
		(void) thread_resume(th);
	    }
	}

	(void) kernel_thread(kernel_task, reaper_thread, (char *) 0);
	(void) kernel_thread(kernel_task, swapin_thread, (char *) 0);
	(void) kernel_thread(kernel_task, sched_thread, (char *) 0);

#if	NCPUS > 1
	/*
	 *	Create the shutdown thread.
	 */
	(void) kernel_thread(kernel_task, action_thread, (char *) 0);

	/*
	 *	Allow other CPUs to run.
	 */
	start_other_cpus();
#endif	NCPUS > 1

	/*
	 *	Create the device service.
	 */
	device_service_create();

	/*
	 *	Initialize NORMA ipc system.
	 */
#if	NORMA_IPC
	norma_ipc_init();
#endif	NORMA_IPC

	/*
	 *	Initialize NORMA vm system.
	 */
#if	NORMA_VM
	norma_vm_init();
#endif	NORMA_VM

	/*
	 *	Start the user bootstrap.
	 */
	bootstrap_create();

#if	XPR_DEBUG
	xprinit();		/* XXX */
#endif	XPR_DEBUG

	/*
	 *	Become the pageout daemon.
	 */
	(void) spl0();
	vm_pageout();
	/*NOTREACHED*/
}

#if	NCPUS > 1

extern void	slave_machine_init();

void slave_main()
{
	slave_machine_init();

	cpu_launch_first_thread(THREAD_NULL);
}
#endif	NCPUS > 1

/*
 *	Start up the first thread on a CPU.
 *	First thread is specified for the master CPU.
 */
void cpu_launch_first_thread(th)
	register thread_t	th;
{
	register int	mycpu;

	mycpu = cpu_number();

	cpu_up(mycpu);

	start_timer(&kernel_timer[mycpu]);

	/*
	 * Block all interrupts for choose_thread.
	 */
	(void) splhigh();

	if (th == THREAD_NULL)
	    th = choose_thread(cpu_to_processor(mycpu));
	if (th == THREAD_NULL)
	    panic("cpu_launch_first_thread");

	startrtclock();		/* needs an active thread */
	PMAP_ACTIVATE_KERNEL(mycpu);

	active_threads[mycpu] = th;
	active_stacks[mycpu] = th->kernel_stack;
	thread_lock(th);
	th->state &= ~TH_UNINT;
	thread_unlock(th);
	timer_switch(&th->system_timer);

	PMAP_ACTIVATE_USER(vm_map_pmap(th->task->map), th, mycpu);

	load_context(th);
	/*NOTREACHED*/
}
