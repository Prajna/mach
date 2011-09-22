/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * Copyright (c) 1991 Sequent Computer Systems
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND SEQUENT COMPUTER SYSTEMS ALLOW FREE USE OF
 * THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * SEQUENT COMPUTER SYSTEMS DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	mutex.h,v $
 * Revision 2.3  91/07/31  18:03:14  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  12:58:12  dbg
 * 	Adapted for pure kernel.  Removed gate code, since Symmetry
 * 	doesn't use it.  Re-implemented Dynix semaphores.
 * 	[90/09/24            dbg]
 * 
 */

/*
 * $Header: mutex.h,v 2.3 91/07/31 18:03:14 dbg Exp $
 *
 * mutex.h
 *	Implementation dependent mutual-exclusion interface definitions.
 *
 * i386 version.
 */

/*
 * Revision 1.1  89/07/05  13:15:39  kak
 * Initial revision
 * 
 * Revision 2.25  89/02/27  10:38:58  djg
 * increased slic sychrinisation timing to 22Mhz (=12 cycles)
 * 
 * Revision 2.24  89/02/20  08:02:20  djg
 * corrected comments on SLICSYNC timeing and added a missing
 * #if MHz=20 ... nop
 * 
 * Revision 2.23  88/11/10  08:25:17  djg
 * bak242 
 * 
 */

#ifndef	_SQT_MUTEX_H_
#define	_SQT_MUTEX_H_

/*
 * i386 does not use gates.  Leave typedef here for compilation.
 */
typedef	unsigned char	gate_t;		/* gate number */

/*
 * Use Mach simple_lock_data_t instead of Dynix lock_t.
 */
#include <kern/lock.h>

/*
 * Semaphores.
 * Very simple mono-processor implementation.  See sqt/sema.c.
 *
 * These semaphores assume non-signalable sleep priorities, initial value
 * == 0 or 1.  Implements "random" queueing.
 *
 * This is sufficient for (eg) zd driver, sm, misc others.  Anything
 * more fancy need to be done in driver itself.
 */

struct sema {
	char		lock;
	char		waiting;
	short		count;
};
typedef struct sema	sema_t;

/*
 * Processor interrupt masking (lowest level mutex).
 */

#if	!defined(GENASSYM) && !defined(lint)

#define	DISABLE() \
	asm volatile("cli")

#define	ENABLE() \
	asm volatile("sti")

#endif	!defined(GENASSYM) && !defined(lint)

/*
 * Locks.
 *
 * init_lock() interface provided as a macro to avoid need to represent
 * SLIC gates numbers.
 */

#define	L_UNLOCKED	0
#define	L_LOCKED	1

#define	CPLOCKFAIL	-1		/* illegal SPL value */

#define	init_lock(l,g)	simple_lock_init(l)

#define	v_lock(l,p)	(simple_unlock(l), splx(p))

#endif	_SQT_MUTEX_H_
