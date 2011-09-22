/* 
 * Mach Operating System
 * Copyright (c) 1991,1992 Carnegie Mellon University
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
 * $Log:	ipc_net.c,v $
 * Revision 2.12  93/01/14  17:53:58  danner
 * 	64bit cleanup. Proper spl typing.
 * 	[92/12/01            af]
 * 
 * Revision 2.11  92/03/10  16:27:47  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:49:32  jsb]
 * 
 * Revision 2.10.2.5  92/02/21  11:24:34  jsb
 * 	Moved spl{on,off} definitions earlier in the file (before all uses).
 * 	[92/02/18  08:03:01  jsb]
 * 
 * 	Removed accidently included lint declarations of panic, etc.
 * 	[92/02/16  11:17:48  jsb]
 * 
 * 	Eliminated netipc_thread_wakeup/netipc_replenish race.
 * 	[92/02/09  14:16:24  jsb]
 * 
 * Revision 2.10.2.4  92/02/18  19:14:43  jeffreyh
 * 	[intel] added support for callhere debug option of iPSC.
 * 	[92/02/13  13:02:21  jeffreyh]
 * 
 * Revision 2.10.2.3  92/01/21  21:51:30  jsb
 * 	From sjs@osf.org: moved node_incarnation declaration here from
 * 	ipc_ether.c.
 * 	[92/01/17  14:37:03  jsb]
 * 
 * 	New implementation of netipc lock routines which uses sploff/splon
 * 	and which releases spl before calling interrupt handlers (but after
 * 	taking netipc lock).
 * 	[92/01/16  22:20:30  jsb]
 * 
 * 	Removed panic in netipc_copy_grab: callers can now deal with failure.
 * 	[92/01/14  21:59:10  jsb]
 * 
 * 	In netipc_drain_intr_request, decrement request counts before calling
 * 	interrupt routines, not after. This preserves assertion that there is
 * 	at most one outstanding send or receive interrupt.
 * 	[92/01/13  20:17:18  jsb]
 * 
 * 	De-linted.
 * 	[92/01/13  10:15:50  jsb]
 * 
 * 	Now contains locking, allocation, and debugging printf routines.
 * 	Purged log.
 * 	[92/01/11  17:38:28  jsb]
 * 
 */ 
/*
 *	File:	norma/ipc_net.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Routines for reliable delivery and flow control for NORMA_IPC.
 */

#include <norma/ipc_net.h>

#if	i386 || i860
#else
#define	sploff()	splhigh()
#define	splon(s)	splx(s)
#endif

/*
 * Not proven to be multiprocessor-safe
 */

unsigned long node_incarnation = 1;		/* should never be zero */

decl_simple_lock_data(,netipc_lock)

thread_t netipc_lock_owner = THREAD_NULL;
#define	THREAD_INTR	((thread_t) 1)

int send_intr_request = 0;
int recv_intr_request = 0;
int timeout_intr_request = 0;

#if	iPSC386 || iPSC860
extern void	netipc_called_here();
#endif	iPSC386 || iPSC860

/*
 * Called with interrupts not explicitly disabled but with lock held.
 * Returns with interrupts as they were and with lock released.
 */
int
netipc_drain_intr_request()
{
	spl_t	s;

	s = sploff();
#if	iPSC386 || iPSC860
	netipc_called_here(__FILE__, __LINE__, "netipc_drain_intr_request");
#endif	iPSC836 || iPSC860
	assert(netipc_lock_owner != THREAD_NULL);
	while (send_intr_request > 0 ||
	       recv_intr_request > 0 ||
	       timeout_intr_request > 0) {
		/*
		 * Send and receive interrupts are counting interrupts.
		 * Many timeout interrupts map into one.
		 */
		netipc_lock_owner = THREAD_INTR;
		if (send_intr_request > 0) {
			send_intr_request--;
			splon(s);
#if	iPSC386 || iPSC860
			{ int spl = spldcm();
#endif	iPSC836 || iPSC860
			_netipc_send_intr();
#if	iPSC386 || iPSC860
			splx(spl);}
#endif	iPSC836 || iPSC860
			s = sploff();
		} else if (recv_intr_request > 0) {
			recv_intr_request--;
			splon(s);
#if	iPSC386 || iPSC860
			{ int spl = spldcm();
#endif	iPSC836 || iPSC860
			_netipc_recv_intr();
#if	iPSC386 || iPSC860
			splx(spl);}
#endif	iPSC836 || iPSC860
			s = sploff();
		} else {
			assert(timeout_intr_request > 0);
			timeout_intr_request = 0;
			splon(s);
#if	iPSC386 || iPSC860
			{ int spl = splclock();
#endif	iPSC836 || iPSC860
			_netipc_timeout_intr();
#if	iPSC386 || iPSC860
			splx(spl);}
#endif	iPSC836 || iPSC860
			s = sploff();
		}
	}
	netipc_lock_owner = THREAD_NULL;
	splon(s);
}

/*
 * XXX
 * These testing functions should have spls.
 */

boolean_t
netipc_thread_locked()
{
	return (netipc_lock_owner == current_thread());
}

boolean_t
netipc_intr_locked()
{
	return (netipc_lock_owner == THREAD_INTR);
}

boolean_t
netipc_locked()
{
	return (netipc_lock_owner != THREAD_NULL);
}

boolean_t
netipc_unlocked()
{
	return (netipc_lock_owner == THREAD_NULL);
}

void
netipc_thread_lock()
{
	spl_t s;

	/*
	 * Threads fight among themselves.
	 */
	simple_lock(&netipc_lock);

	/*
	 * A single thread fights against interrupt handler.
	 */
	s = sploff();
	assert(netipc_unlocked());
	netipc_lock_owner = current_thread();
	splon(s);
}

void
netipc_thread_unlock()
{
	assert(netipc_thread_locked());

	/*
	 * Process queued interrupts, and release simple lock.
	 */
	netipc_drain_intr_request();
	simple_unlock(&netipc_lock);
}

netipc_send_intr()
{
	spl_t	s;

	s = sploff();
	if (netipc_lock_owner == THREAD_NULL) {
		netipc_lock_owner = THREAD_INTR;
		splon(s);
#if	iPSC386 || iPSC860
		{ int spl = spldcm();
#endif	iPSC836 || iPSC860
		_netipc_send_intr();
#if	iPSC386 || iPSC860
		splx(spl);}
#endif	iPSC836 || iPSC860
		netipc_drain_intr_request();
	} else {
		assert(send_intr_request == 0);
		send_intr_request++;
		splon(s);
	}
}

netipc_recv_intr()
{
	spl_t s;

	s = sploff();
	if (netipc_lock_owner == THREAD_NULL) {
		netipc_lock_owner = THREAD_INTR;
		splon(s);
#if	iPSC386 || iPSC860
		{ int spl = spldcm();
#endif	iPSC836 || iPSC860
		_netipc_recv_intr();
#if	iPSC386 || iPSC860
		splx(spl);}
#endif	iPSC836 || iPSC860
		netipc_drain_intr_request();
	} else {
		assert(recv_intr_request == 0);
		recv_intr_request++;
		splon(s);
	}
}

netipc_timeout_intr()
{
	spl_t s;

	s = sploff();
	if (netipc_lock_owner == THREAD_NULL) {
		netipc_lock_owner = THREAD_INTR;
		splon(s);
#if	iPSC386 || iPSC860
		{ int spl = splclock();
#endif	iPSC836 || iPSC860
		_netipc_timeout_intr();
#if	iPSC386 || iPSC860
		splx(spl);}
#endif	iPSC836 || iPSC860
		netipc_drain_intr_request();
	} else {
		timeout_intr_request = 1;
		splon(s);
	}
}

extern int	netipc_self_stopped;

boolean_t	netipc_thread_awake = FALSE;
boolean_t	netipc_thread_reawaken = FALSE;
int		netipc_thread_awaken = 0;
vm_page_t	netipc_page_list = VM_PAGE_NULL;
int		netipc_page_list_count = 0;
int		netipc_page_list_low = 20;
int		netipc_page_list_high = 30;

extern zone_t	vm_map_copy_zone;
vm_map_copy_t	netipc_vm_map_copy_list = VM_MAP_COPY_NULL;
int		netipc_vm_map_copy_count = 0;

netipc_page_put(m)
	vm_page_t m;
{
	assert(netipc_locked());

	* (vm_page_t *) &m->pageq.next = netipc_page_list;
	netipc_page_list = m;
	netipc_page_list_count++;
	if (netipc_self_stopped) {
		netipc_self_unstop();
	}
}

vm_map_copy_t
netipc_copy_grab()
{
	vm_map_copy_t copy;

	assert(netipc_locked());
	copy = netipc_vm_map_copy_list;
	if (copy != VM_MAP_COPY_NULL) {
		netipc_vm_map_copy_list = (vm_map_copy_t) copy->type;
		netipc_vm_map_copy_count--;
		copy->type = VM_MAP_COPY_PAGE_LIST;
	}
	return copy;
}

void
netipc_copy_ungrab(copy)
	vm_map_copy_t copy;
{
	assert(netipc_locked());
	copy->type = (int) netipc_vm_map_copy_list;
	netipc_vm_map_copy_list = copy;
	netipc_vm_map_copy_count++;
}

netipc_thread_wakeup()
{
	assert(netipc_locked());
	if (netipc_thread_awake) {
		netipc_thread_reawaken = TRUE;
	} else {
		thread_wakeup((vm_offset_t) &netipc_thread_awake);
	}
}

/*
 * XXX
 * The wakeup protocol for this loop is not quite correct...
 *
 * XXX
 * We should move the lists out all at once, not one elt at a time.
 *
 * XXX
 * The locking here is farcical.
 */
netipc_replenish(always)
	boolean_t always;
{
	vm_page_t m;

	assert(netipc_unlocked());
	netipc_output_replenish();	/* XXX move somewhere else */
	while (netipc_vm_map_copy_count < 300) {
		vm_map_copy_t copy;

		copy = (vm_map_copy_t) zalloc(vm_map_copy_zone);
		netipc_thread_lock();
		copy->type = (int) netipc_vm_map_copy_list;
		netipc_vm_map_copy_list = copy;
		netipc_vm_map_copy_count++;
		netipc_thread_unlock();
	}
	if (current_thread()->vm_privilege) {
		return;	/* we might allocate from the reserved pool */
	}
	while (netipc_page_list_count < netipc_page_list_high) {
		m = vm_page_grab();
		if (m == VM_PAGE_NULL) {
			break;
		}
		m->tabled = FALSE;
		vm_page_init(m, m->phys_addr);

		netipc_thread_lock();
		* (vm_page_t *) &m->pageq.next = netipc_page_list;
		netipc_page_list = m;
		netipc_page_list_count++;
		if (netipc_self_stopped) {
			netipc_self_unstop();
		}
		netipc_thread_unlock();
	}
	while (always && netipc_page_list_count < netipc_page_list_low) {
		while ((m = vm_page_grab()) == VM_PAGE_NULL) {
			vm_page_wait(0);
		}
		m->tabled = FALSE;
		vm_page_init(m, m->phys_addr);

		netipc_thread_lock();
		* (vm_page_t *) &m->pageq.next = netipc_page_list;
		netipc_page_list = m;
		netipc_page_list_count++;
		if (netipc_self_stopped) {
			netipc_self_unstop();
		}
		netipc_thread_unlock();
	}
}

/*
 * Grab a vm_page_t at interrupt level. May return VM_PAGE_NULL.
 */
vm_page_t
netipc_page_grab()
{
	vm_page_t m;

	assert(netipc_locked());
	if ((m = netipc_page_list) != VM_PAGE_NULL) {
		netipc_page_list = (vm_page_t) m->pageq.next;
		netipc_page_list_count--;
	} else {
		netipc_thread_wakeup();
	}
	return m;
}

void
netipc_thread_continue()
{
	netipc_thread_lock();
	for (;;) {
		/*
		 * Record that we are awake.
		 * Look out for new awaken requests while we are out working.
		 */
		netipc_thread_awaken++;
		netipc_thread_awake = TRUE;
		netipc_thread_reawaken = FALSE;

		/*
		 * Call netipc_replenish with netipc lock unlocked.
		 */
		netipc_thread_unlock();
		netipc_replenish(TRUE);
		netipc_thread_lock();

		/*
		 * If we don't yet have enough pages, or someone
		 * came up with something new for us to do, then
		 * do more work before going to sleep.
		 */
		if (netipc_page_list_count < netipc_page_list_low ||
		    netipc_thread_reawaken) {
			continue;
		}

		/*
		 * Nothing left for us to do right now.  Go to sleep.
		 */
		netipc_thread_awake = FALSE;
		assert_wait((vm_offset_t) &netipc_thread_awake, FALSE);
		(void) netipc_thread_unlock();
		thread_block(netipc_thread_continue);
		netipc_thread_lock();
	}
}

void
netipc_thread()
{
	thread_set_own_priority(0);	/* high priority */
	netipc_thread_continue();
}

int Noise0 = 0;	/* print netipc packets */	
int Noise1 = 0;	/* notification and migration debugging */
int Noise2 = 0;	/* synch and timeout printfs */
int Noise3 = 0;	/* copy object continuation debugging */
int Noise4 = 0;	/* multiple out-of-line section debugging */
int Noise5 = 0;	/* obsolete acks */
int Noise6 = 0;	/* short print of rcvd packets, including msgh_id */

extern cnputc();

/* VARARGS */
printf1(fmt, va_alist)
	char* fmt;
	va_dcl
{
	va_list	listp;

	if (Noise1) {
		va_start(listp);
		_doprnt(fmt, &listp, cnputc, 0);
		va_end(listp);
	}
}

/* VARARGS */
printf2(fmt, va_alist)
	char* fmt;
	va_dcl
{
	va_list	listp;

	if (Noise2) {
		va_start(listp);
		_doprnt(fmt, &listp, cnputc, 0);
		va_end(listp);
	}
}

/* VARARGS */
printf3(fmt, va_alist)
	char* fmt;
	va_dcl
{
	va_list	listp;

	if (Noise3) {
		va_start(listp);
		_doprnt(fmt, &listp, cnputc, 0);
		va_end(listp);
	}
}

/* VARARGS */
printf4(fmt, va_alist)
	char* fmt;
	va_dcl
{
	va_list	listp;

	if (Noise4) {
		va_start(listp);
		_doprnt(fmt, &listp, cnputc, 0);
		va_end(listp);
	}
}

/* VARARGS */
printf5(fmt, va_alist)
	char* fmt;
	va_dcl
{
	va_list	listp;

	if (Noise5) {
		va_start(listp);
		_doprnt(fmt, &listp, cnputc, 0);
		va_end(listp);
	}
}

/* VARARGS */
printf6(fmt, va_alist)
	char* fmt;
	va_dcl
{
	va_list	listp;

	if (Noise6) {
		va_start(listp);
		_doprnt(fmt, &listp, cnputc, 0);
		va_end(listp);
	}
}

#if	iPSC386 || iPSC860
#define	MAX_CALLS	256
char	*called_here_filename_buffer[MAX_CALLS];
char	*called_here_notation_buffer[MAX_CALLS];
int	called_here_line_buffer[MAX_CALLS];
int	called_here_next = 0;

void netipc_called_here(filename, line, notation)
	char	*filename, *notation;
	int	line;
{
	spl_t	s;
	int	i;

	s = sploff();
	i = called_here_next++;
	if (called_here_next >= MAX_CALLS) {
		called_here_next = 0;
	}
	called_here_filename_buffer[called_here_next] = 0;
	called_here_filename_buffer[i] = filename;
	called_here_notation_buffer[i] = notation;
	called_here_line_buffer[i] = line;
	splon(s);
}


void db_show_netipc_called_here()
{
	int	i, j;
	char	*s, *slash;

	kdbprintf(" #   Line File\n");
	j = called_here_next - 1;
	for (i = 0; i < MAX_CALLS; i++) {
		if (j < 0) {
			j = MAX_CALLS - 1;
		}
		if (called_here_filename_buffer[j]) {
			slash = 0;
			for (s = called_here_filename_buffer[j]; *s; s++) {
				if (*s == '/')
					slash = s + 1;
			}
			kdbprintf("%3d %5d %s\t\t%s\n", j,
				called_here_line_buffer[j],
				slash ? slash : called_here_filename_buffer[j],
				called_here_notation_buffer[j]);
			j--;
		} else {
			return;
		}
	}
}

#endif	iPSC386 || iPSC860
