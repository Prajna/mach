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
 * $Log:	lock.h,v $
 * Revision 2.12  93/02/05  07:51:21  danner
 * 	Reorganization of lock structure.
 * 	Recursion_depth is now a bitfield,
 * 	and the thread pointer is the first field.
 * 	[93/02/04            danner]
 * 
 * Revision 2.11  93/01/24  13:19:22  danner
 * 	Add include of mach/machine/vm_types.h
 * 
 * Revision 2.10  93/01/19  09:00:57  danner
 * 	Underlying simple lock type is now volatile natural_t.
 * 	[93/01/19            danner]
 * 
 * Revision 2.9  93/01/14  17:35:00  danner
 * 	Added ANSI function prototypes.  Made simple lock data
 * 	volatile.
 * 	[92/12/30            dbg]
 * 	ANSI-fied cpp directives.
 * 	[92/12/01            af]
 * 
 * Revision 2.8  91/11/12  11:51:58  rvb
 * 	Added simple_lock_pause.
 * 	[91/11/12            rpd]
 * 
 * Revision 2.7  91/05/18  14:32:17  rpd
 * 	Added check_simple_locks.
 * 	[91/03/31            rpd]
 * 
 * Revision 2.6  91/05/14  16:43:51  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/05/08  12:47:17  dbg
 * 	When actually using the locks (on multiprocessors), import the
 * 	machine-dependent simple_lock routines from machine/lock.h.
 * 	[91/04/26  14:42:23  dbg]
 * 
 * Revision 2.4  91/02/05  17:27:37  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:14:39  mrt]
 * 
 * Revision 2.3  90/11/05  14:31:18  rpd
 * 	Added simple_lock_taken.
 * 	[90/11/04            rpd]
 * 
 * Revision 2.2  90/01/11  11:43:26  dbg
 * 	Upgraded to match mainline:
 * 	 	Added decl_simple_lock_data, simple_lock_addr macros.
 * 	 	Rearranged complex lock structure to use decl_simple_lock_data
 * 	 	for the interlock field and put it last (except on ns32000).
 * 	 	[89/01/15  15:16:47  rpd]
 * 
 * 	Made all machines use the compact field layout.
 * 
 * Revision 2.1  89/08/03  15:49:42  rwd
 * Created.
 * 
 * Revision 2.2  88/07/20  16:49:35  rpd
 * Allow for sanity-checking of simple locking on uniprocessors,
 * controlled by new option MACH_LDEBUG.  Define composite
 * MACH_SLOCKS, which is true iff simple locking calls expand
 * to code.  It can be used to #if-out declarations, etc, that
 * are only used when simple locking calls are real.
 * 
 *  3-Nov-87  David Black (dlb) at Carnegie-Mellon University
 *	Use optimized lock structure for multimax also.
 *
 * 27-Oct-87  Robert Baron (rvb) at Carnegie-Mellon University
 *	Use optimized lock "structure" for balance now that locks are
 *	done inline.
 *
 * 26-Jan-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Invert logic of no_sleep to can_sleep.
 *
 * 29-Dec-86  David Golub (dbg) at Carnegie-Mellon University
 *	Removed BUSYP, BUSYV, adawi, mpinc, mpdec.  Defined the
 *	interlock field of the lock structure to be a simple-lock.
 *
 *  9-Nov-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added "unsigned" to fields in vax case, for lint.
 *
 * 21-Oct-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added fields for sleep/recursive locks.
 *
 *  7-Oct-86  David L. Black (dlb) at Carnegie-Mellon University
 *	Merged Multimax changes.
 *
 * 26-Sep-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Removed reference to "caddr_t" from BUSYV/P.  I really
 *	wish we could get rid of these things entirely.
 *
 * 24-Sep-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Changed to directly import boolean declaration.
 *
 *  1-Aug-86  David Golub (dbg) at Carnegie-Mellon University
 *	Added simple_lock_try, sleep locks, recursive locking.
 *
 * 11-Jun-86  Bill Bolosky (bolosky) at Carnegie-Mellon University
 *	Removed ';' from definitions of locking macros (defined only
 *	when NCPU < 2). so as to make things compile.
 *
 * 28-Feb-86  Bill Bolosky (bolosky) at Carnegie-Mellon University
 *	Defined adawi to be add when not on a vax.
 *
 * 07-Nov-85  Michael Wayne Young (mwyoung) at Carnegie-Mellon University
 *	Overhauled from previous version.
 */
/*
 *	File:	kern/lock.h
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *	Date:	1985
 *
 *	Locking primitives definitions
 */

#ifndef	_KERN_LOCK_H_
#define	_KERN_LOCK_H_

#include <cpus.h>
#include <mach_ldebug.h>

#include <mach/boolean.h>
#include <mach/machine/vm_types.h>

#define MACH_SLOCKS	((NCPUS > 1) || MACH_LDEBUG)

/*
 *	A simple spin lock.
 */

struct slock {
	volatile natural_t lock_data;	/* in general 1 bit is sufficient */
};

typedef struct slock	simple_lock_data_t;
typedef struct slock	*simple_lock_t;

#if	MACH_SLOCKS
/*
 *	Use the locks.
 */

#define	decl_simple_lock_data(class,name) \
class	simple_lock_data_t	name;

#define	simple_lock_addr(lock)	(&(lock))

#if	(NCPUS > 1)

/*
 *	Import the definitions from machine-dependent code.
 */

#include <machine/lock.h>

/*
 *	The single-CPU debugging routines are not valid
 *	on a multiprocessor.
 */
#define	simple_lock_taken(lock)		(1)	/* always succeeds */
#define check_simple_locks()

#else	/* NCPUS > 1 */
/*
 *	Use our single-CPU locking test routines.
 */

extern void		simple_lock_init(simple_lock_t);
extern void		simple_lock(simple_lock_t);
extern void		simple_unlock(simple_lock_t);
extern boolean_t	simple_lock_try(simple_lock_t);

#define simple_lock_pause()
#define simple_lock_taken(lock)		((lock)->lock_data)

extern void		check_simple_locks(void);

#endif	/* NCPUS > 1 */

#else	/* MACH_SLOCKS */
/*
 * Do not allocate storage for locks if not needed.
 */
#define	decl_simple_lock_data(class,name)
#define	simple_lock_addr(lock)		((simple_lock_t)0)

/*
 *	No multiprocessor locking is necessary.
 */
#define simple_lock_init(l)
#define simple_lock(l)
#define simple_unlock(l)
#define simple_lock_try(l)	(TRUE)	/* always succeeds */
#define simple_lock_taken(l)	(1)	/* always succeeds */
#define check_simple_locks()
#define simple_lock_pause()

#endif	/* MACH_SLOCKS */

/*
 *	The general lock structure.  Provides for multiple readers,
 *	upgrading from read to write, and sleeping until the lock
 *	can be gained.
 *
 *	On some architectures, assembly language code in the 'inline'
 *	program fiddles the lock structures.  It must be changed in
 *	concert with the structure layout.
 *
 *	Only the "interlock" field is used for hardware exclusion;
 *	other fields are modified with normal instructions after
 *	acquiring the interlock bit.
 */
struct lock {
	struct thread	*thread;	/* Thread that has lock, if
					   recursive locking allowed */
	unsigned int	read_count:16,	/* Number of accepted readers */
	/* boolean_t */	want_upgrade:1,	/* Read-to-write upgrade waiting */
	/* boolean_t */	want_write:1,	/* Writer is waiting, or
					   locked for write */
	/* boolean_t */	waiting:1,	/* Someone is sleeping on lock */
	/* boolean_t */	can_sleep:1,	/* Can attempts to lock go to sleep? */
			recursion_depth:12, /* Depth of recursion */
			:0; 
	decl_simple_lock_data(,interlock)
					/* Hardware interlock field.
					   Last in the structure so that
					   field offsets are the same whether
					   or not it is present. */
};

typedef struct lock	lock_data_t;
typedef struct lock	*lock_t;

/* Sleep locks must work even if no multiprocessing */

extern void		lock_init(lock_t, boolean_t);
extern void		lock_sleepable(lock_t, boolean_t);
extern void		lock_write(lock_t);
extern void		lock_read(lock_t);
extern void		lock_done(lock_t);
extern boolean_t	lock_read_to_write(lock_t);
extern void		lock_write_to_read(lock_t);
extern boolean_t	lock_try_write(lock_t);
extern boolean_t	lock_try_read(lock_t);
extern boolean_t	lock_try_read_to_write(lock_t);

#define	lock_read_done(l)	lock_done(l)
#define	lock_write_done(l)	lock_done(l)

extern void		lock_set_recursive(lock_t);
extern void		lock_clear_recursive(lock_t);

#endif	/* _KERN_LOCK_H_ */
