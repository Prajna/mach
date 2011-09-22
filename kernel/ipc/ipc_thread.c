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
 * $Log:	ipc_thread.c,v $
 * Revision 2.6  92/08/03  17:35:55  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.5  92/05/21  17:12:02  jfriedl
 * 	tried prototypes.
 * 	[92/05/20            jfriedl]
 * 
 * Revision 2.4  91/05/14  16:38:05  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:24:23  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:52:29  mrt]
 * 
 * Revision 2.2  90/06/02  14:52:06  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:04:48  rpd]
 * 
 */
/*
 *	File:	ipc/ipc_thread.c
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	IPC operations on threads.
 */

#include <ipc/ipc_thread.h>



/*
 *	Routine:	ipc_thread_enqueue
 *	Purpose:
 *		Enqueue a thread.
 */

void
ipc_thread_enqueue(queue, thread)
	ipc_thread_queue_t queue;
	ipc_thread_t thread;
{
	ipc_thread_enqueue_macro(queue, thread);
}

/*
 *	Routine:	ipc_thread_dequeue
 *	Purpose:
 *		Dequeue and return a thread.
 */

ipc_thread_t
ipc_thread_dequeue(queue)
	ipc_thread_queue_t queue;
{
	ipc_thread_t first;

	first = ipc_thread_queue_first(queue);

	if (first != ITH_NULL)
		ipc_thread_rmqueue_first_macro(queue, first);

	return first;
}

/*
 *	Routine:	ipc_thread_rmqueue
 *	Purpose:
 *		Pull a thread out of a queue.
 */

void
ipc_thread_rmqueue(queue, thread)
	ipc_thread_queue_t queue;
	ipc_thread_t thread;
{
	ipc_thread_t next, prev;

	assert(queue->ithq_base != ITH_NULL);

	next = thread->ith_next;
	prev = thread->ith_prev;

	if (next == thread) {
		assert(prev == thread);
		assert(queue->ithq_base == thread);

		queue->ithq_base = ITH_NULL;
	} else {
		if (queue->ithq_base == thread)
			queue->ithq_base = next;

		next->ith_prev = prev;
		prev->ith_next = next;
		ipc_thread_links_init(thread);
	}
}
