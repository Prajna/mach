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
 * $Log:	queue.c,v $
 * Revision 2.8  93/03/09  10:55:20  danner
 * 	Fixed remque not to typecast. ANSI C.
 * 	[93/03/05            af]
 * 
 * Revision 2.7  93/01/27  09:35:03  danner
 * 	Remove ifdef sun around insque and remque.
 * 	[93/01/25            danner]
 * 
 * Revision 2.6  92/08/03  17:38:50  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.5  92/05/21  17:15:19  jfriedl
 * 	Added void to fcns that still needed it.
 * 	[92/05/20            jfriedl]
 * 
 * Revision 2.4  91/05/14  16:45:45  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/05/08  12:48:22  dbg
 * 	Compile queue routines on vax.
 * 	[91/03/26            dbg]
 * 
 * Revision 2.2  91/02/05  17:28:38  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:16:22  mrt]
 * 
 * Revision 2.1  89/08/03  15:51:47  rwd
 * Created.
 * 
 * 17-Mar-87  David Golub (dbg) at Carnegie-Mellon University
 *	Created from routines written by David L. Black.
 *
 */ 

/*
 *	Routines to implement queue package.
 */

#include <kern/queue.h>



/*
 *	Insert element at head of queue.
 */
void enqueue_head(
	register queue_t	que,
	register queue_entry_t	elt)
{
	elt->next = que->next;
	elt->prev = que;
	elt->next->prev = elt;
	que->next = elt;
}

/*
 *	Insert element at tail of queue.
 */
void enqueue_tail(
	register queue_t	que,
	register queue_entry_t	elt)
{
	elt->next = que;
	elt->prev = que->prev;
	elt->prev->next = elt;
	que->prev = elt;
}

/*
 *	Remove and return element at head of queue.
 */
queue_entry_t dequeue_head(
	register queue_t	que)
{
	register queue_entry_t	elt;

	if (que->next == que)
		return((queue_entry_t)0);

	elt = que->next;
	elt->next->prev = que;
	que->next = elt->next;
	return(elt);
}

/*
 *	Remove and return element at tail of queue.
 */
queue_entry_t dequeue_tail(
	register queue_t	que)
{
	register queue_entry_t	elt;

	if (que->prev == que)
		return((queue_entry_t)0);

	elt = que->prev;
	elt->prev->next = que;
	que->prev = elt->prev;
	return(elt);
}

/*
 *	Remove arbitrary element from queue.
 *	Does not check whether element is on queue - the world
 *	will go haywire if it isn't.
 */

/*ARGSUSED*/
void remqueue(
	queue_t			que,
	register queue_entry_t	elt)
{
	elt->next->prev = elt->prev;
	elt->prev->next = elt->next;
}

/*
 *	Routines to directly imitate the VAX hardware queue
 *	package.
 */
void insque(
	register struct queue_entry *entry,
	register struct queue_entry *pred)
{
	entry->next = pred->next;
	entry->prev = pred;
	(pred->next)->prev = entry;
	pred->next = entry;
}

struct queue_entry
*remque(
	register struct queue_entry *elt)
{
	(elt->next)->prev = elt->prev;
	(elt->prev)->next = elt->next;
	return(elt);
}

