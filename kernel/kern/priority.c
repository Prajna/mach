/* 
 * Mach Operating System
 * Copyright (c) 1993,1991,1990,1989,1988,1987 Carnegie Mellon University
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
 * $Log:	priority.c,v $
 * Revision 2.8  93/05/15  18:56:43  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.7  93/01/14  17:35:42  danner
 * 	Added check to thread_quantum_update for processor not in a
 * 	processor set (when being reassigned).  This should be fixed in
 * 	processor assignment code instead.
 * 	[92/10/23            dbg]
 * 	Removed unnecessary include of machine/mach_param.h
 * 	[92/12/16            af]
 * 	Proper spl typing.
 * 	[92/12/01            af]
 * 
 * 	Added check thread_quantum_update for processor not in a
 * 	processor set (when being reassigned).  This should be fixed in
 * 	processor assignment code instead.
 * 	[92/10/23            dbg]
 * 	Proper spl typing.
 * 	[92/12/01            af]
 * 
 * 	Added check thread_quantum_update for processor not in a
 * 	processor set (when being reassigned).  This should be fixed in
 * 	processor assignment code instead.
 * 	[92/10/23            dbg]
 * 
 * Revision 2.6  92/08/03  17:38:38  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.5  92/05/21  17:15:03  jfriedl
 * 	tried prototypes.
 * 	[92/05/20            jfriedl]
 * 
 * Revision 2.4  91/05/14  16:45:17  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:28:22  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:15:50  mrt]
 * 
 * Revision 2.2  90/06/02  14:55:24  rpd
 * 	Updated to new scheduling technology.
 * 	[90/03/26  22:13:58  rpd]
 * 
 * Revision 2.1  89/08/03  15:45:28  rwd
 * Created.
 * 
 * 24-Mar-89  David Golub (dbg) at Carnegie-Mellon University
 *	Added thread_set_priority.
 *
 * 14-Jan-89  David Golub (dbg) at Carnegie-Mellon University
 *	Split into two new files: mach_clock (for timing) and priority
 *	(for priority calculation).
 *
 *  9-Aug-88  David Black (dlb) at Carnegie-Mellon University
 *	thread->first_quantum replaces runrun.
 *
 *  4-May-88  David Black (dlb) at Carnegie-Mellon University
 *	MACH_TIME_NEW is now standard.
 *	Do ageing here on clock interrupts instead of in
 *	recompute_priorities.  Do accurate usage calculations.
 *
 * 18-Nov-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Delete previous history.
 */ 
/*
 *	File:	clock_prim.c
 *	Author:	Avadis Tevanian, Jr.
 *	Date:	1986
 *
 *	Clock primitives.
 */

#include <cpus.h>

#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/machine.h>
#include <kern/host.h>
#include <kern/mach_param.h>
#include <kern/sched.h>
#include <kern/thread.h>
#include <kern/time_out.h>
#include <kern/time_stamp.h>
#include <machine/machspl.h>



/*
 *	USAGE_THRESHOLD is the amount by which usage must change to
 *	cause a priority shift that moves a thread between run queues.
 */

#ifdef	PRI_SHIFT_2
#if	PRI_SHIFT_2 > 0
#define	USAGE_THRESHOLD (((1 << PRI_SHIFT) + (1 << PRI_SHIFT_2)) << (2 + SCHED_SHIFT))
#else	/* PRI_SHIFT_2 > 0 */
#define	USAGE_THRESHOLD (((1 << PRI_SHIFT) - (1 << -(PRI_SHIFT_2))) << (2 + SCHED_SHIFT))
#endif	/* PRI_SHIFT_2 > 0 */
#else	/* PRI_SHIFT_2 */
#define USAGE_THRESHOLD	(1 << (PRI_SHIFT + 2 + SCHED_SHIFT))
#endif	/* PRI_SHIFT_2 */

/*
 *	thread_quantum_update:
 *
 *	Recalculate the quantum and priority for a thread.
 *	The number of ticks that has elapsed since we were last called
 *	is passed as "nticks."
 */

void thread_quantum_update(mycpu, thread, nticks, state)
	register int		mycpu;
	register thread_t	thread;
	int			nticks;
	int			state;
{
	register int			quantum;
	register processor_t		myprocessor;
#if	NCPUS > 1
	register processor_set_t	pset;
#endif
	spl_t				s;

	myprocessor = cpu_to_processor(mycpu);
#if	NCPUS > 1
	pset = myprocessor->processor_set;
	if (pset == 0) {
	    /*
	     * Processor is being reassigned.
	     * Should rewrite processor assignment code to
	     * block clock interrupts.
	     */
	    return;
	}
#endif	/* NCPUS > 1 */

	/*
	 *	Account for thread's utilization of these ticks.
	 *	This assumes that there is *always* a current thread.
	 *	When the processor is idle, it should be the idle thread.
	 */

	/*
	 *	Update set_quantum and calculate the current quantum.
	 */
#if	NCPUS > 1
	pset->set_quantum = pset->machine_quantum[
		((pset->runq.count > pset->processor_count) ?
		  pset->processor_count : pset->runq.count)];

	if (myprocessor->runq.count != 0)
		quantum = min_quantum;
	else
		quantum = pset->set_quantum;
#else	/* NCPUS > 1 */
	quantum = min_quantum;
	default_pset.set_quantum = quantum;
#endif	/* NCPUS > 1 */
		
	/*
	 *	Now recompute the priority of the thread if appropriate.
	 */

	if (state != CPU_STATE_IDLE) {
		myprocessor->quantum -= nticks;
#if	NCPUS > 1
		/*
		 *	Runtime quantum adjustment.  Use quantum_adj_index
		 *	to avoid synchronizing quantum expirations.
		 */
		if ((quantum != myprocessor->last_quantum) &&
		    (pset->processor_count > 1)) {
			myprocessor->last_quantum = quantum;
			simple_lock(&pset->quantum_adj_lock);
			quantum = min_quantum + (pset->quantum_adj_index *
				(quantum - min_quantum)) / 
					(pset->processor_count - 1);
			if (++(pset->quantum_adj_index) >=
			    pset->processor_count)
				pset->quantum_adj_index = 0;
			simple_unlock(&pset->quantum_adj_lock);
		}
#endif	/* NCPUS > 1 */
		if (myprocessor->quantum <= 0) {
			s = splsched();
			thread_lock(thread);
			if (thread->sched_stamp != sched_tick) {
				update_priority(thread);
			}
			else {
			    if (
#if	MACH_FIXPRI
				(thread->policy == POLICY_TIMESHARE) &&
#endif	/* MACH_FIXPRI */
				(thread->depress_priority < 0)) {
				    thread_timer_delta(thread);
				    thread->sched_usage +=
					thread->sched_delta;
				    thread->sched_delta = 0;
				    compute_my_priority(thread);
			    }
			}
			thread_unlock(thread);
			(void) splx(s);
			/*
			 *	This quantum is up, give this thread another.
			 */
			myprocessor->first_quantum = FALSE;
#if	MACH_FIXPRI
			if (thread->policy == POLICY_TIMESHARE) {
#endif	/* MACH_FIXPRI */
				myprocessor->quantum += quantum;
#if	MACH_FIXPRI
			}
			else {
				/*
				 *    Fixed priority has per-thread quantum.
				 *    
				 */
				myprocessor->quantum += thread->sched_data;
			}
#endif	/* MACH_FIXPRI */
		}
		/*
		 *	Recompute priority if appropriate.
		 */
		else {
		    s = splsched();
		    thread_lock(thread);
		    if (thread->sched_stamp != sched_tick) {
			update_priority(thread);
		    }
		    else {
			if (
#if	MACH_FIXPRI
			    (thread->policy == POLICY_TIMESHARE) &&
#endif	/* MACH_FIXPRI */
			    (thread->depress_priority < 0)) {
				thread_timer_delta(thread);
				if (thread->sched_delta >= USAGE_THRESHOLD) {
				    thread->sched_usage +=
					thread->sched_delta;
				    thread->sched_delta = 0;
				    compute_my_priority(thread);
				}
			}
		    }
		    thread_unlock(thread);
		    (void) splx(s);
		}
		/*
		 * Check for and schedule ast if needed.
		 */
		ast_check();
	}
}

