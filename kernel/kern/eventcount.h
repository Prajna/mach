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
 * $Log:	eventcount.h,v $
 * Revision 2.6  93/08/10  15:12:05  mrt
 * 	Changed ev_id to natural_t to match the changes in
 * 	eventcount.c
 * 	[93/08/03            mrt]
 * 
 * Revision 2.5  93/01/24  13:18:53  danner
 * 	Prototypes, evc_notify_abort.
 * 	[93/01/22            danner]
 * 
 * Revision 2.4  93/01/14  17:34:03  danner
 * 	Allow count to go negative.
 * 	[92/12/15            af]
 * 
 * Revision 2.3  92/01/03  20:40:10  dbg
 * 	Added id field to make user-safe.  Since we have not
 * 	decided what the user-interface will look like (and
 * 	we are using it nonetheless) do not define anything.
 * 	[91/12/27            af]
 * 
 * Revision 2.2  91/12/13  14:54:47  jsb
 * 	Created.
 * 	[91/11/01            af]
 * 
 */
/*
 *	File:	eventcount.c
 *	Author:	Alessandro Forin
 *	Date:	10/91
 *
 *	Eventcounters, for user-level drivers synchronization
 *
 */

#ifndef	_KERN_EVENTCOUNT_H_
#define	_KERN_EVENTCOUNT_H_	1

/* kernel visible only */

typedef struct evc {
	int		count;
	thread_t	waiting_thread;
	natural_t	ev_id;
	struct evc	*sanity;
	decl_simple_lock_data(,	lock)
} *evc_t;

extern	void	evc_init(evc_t ev),
		evc_destroy(evc_t ev),
		evc_signal(evc_t ev),
  		evc_notify_abort(thread_t thread);

/* kernel and user visible */

extern	kern_return_t	evc_wait(natural_t ev_id);

#endif	/* _KERN_EVENTCOUNT_H_ */
