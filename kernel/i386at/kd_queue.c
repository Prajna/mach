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
 * $Log:	kd_queue.c,v $
 * Revision 2.5  91/05/14  16:27:39  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:19:24  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:45:50  mrt]
 * 
 * Revision 2.3  90/11/26  14:50:35  rvb
 * 	jsb bet me to XMK34, sigh ...
 * 	[90/11/26            rvb]
 * 	Synched 2.5 & 3.0 at I386q (r1.5.1.3) & XMK35 (r2.3)
 * 	[90/11/15            rvb]
 * 
 * Revision 2.2  90/05/21  13:27:29  dbg
 * 	First checkin.
 * 	[90/05/17  15:43:38  dbg]
 * 
 * Revision 1.5.1.2  90/02/28  15:50:29  rvb
 * 	Fix numerous typo's in Olivetti disclaimer.
 * 	[90/02/28            rvb]
 * 
 * Revision 1.5.1.1  90/01/08  13:30:48  rvb
 * 	Add Olivetti copyright.
 * 	[90/01/08            rvb]
 * 
 * Revision 1.5  89/07/17  10:41:36  rvb
 * 	Olivetti Changes to X79 upto 5/9/89:
 * 	[89/07/11            rvb]
 * 
 * Revision 1.1.1.1  89/04/27  12:35:55  kupfer
 * X79 from CMU.
 * 
 * Revision 1.4  89/03/09  20:06:54  rpd
 * 	More cleanup.
 * 
 * Revision 1.3  89/02/26  12:42:46  gm0w
 * 	Changes for cleanup.
 * 
 */
 
/* **********************************************************************
 File:         kd_queue.c
 Description:  Event queue code for keyboard/display (and mouse) driver.

 $ Header: $

 Copyright Ing. C. Olivetti & C. S.p.A. 1989.
 All rights reserved.
********************************************************************** */
/*
  Copyright 1988, 1989 by Olivetti Advanced Technology Center, Inc.,
Cupertino, California.

		All Rights Reserved

  Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Olivetti
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

  OLIVETTI DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL OLIVETTI BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUR OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/


#include <i386at/kd_queue.h>

/*
 * Notice that when adding an entry to the queue, the caller provides 
 * its own storage, which is copied into the queue.  However, when 
 * removing an entry from the queue, the caller is given a pointer to a 
 * queue element.  This means that the caller must either process the 
 * element or copy it into its own storage before unlocking the queue.
 *
 * These routines should be called only at a protected SPL.
 */

#define q_next(index)	(((index)+1) % KDQSIZE)

boolean_t
kdq_empty(q)
	kd_event_queue *q;
{
	return(q->firstfree == q->firstout);
}

boolean_t
kdq_full(q)
	kd_event_queue *q;
{
	return(q_next(q->firstfree) == q->firstout);
}

void
kdq_put(q, ev)
	kd_event_queue *q;
	kd_event *ev;
{
	kd_event *qp = q->events + q->firstfree;

	qp->type = ev->type;
	qp->time = ev->time;
	qp->value = ev->value;
	q->firstfree = q_next(q->firstfree);
}

kd_event *
kdq_get(q)
	kd_event_queue *q;
{
	kd_event *result = q->events + q->firstout;

	q->firstout = q_next(q->firstout);
	return(result);
}

void
kdq_reset(q)
	kd_event_queue *q;
{
	q->firstout = q->firstfree = 0;
}
