/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * $Log:	ipc_kserver.c,v $
 * Revision 2.12  93/05/15  19:33:23  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.11  93/01/14  17:53:50  danner
 * 	64bit cleanup. Proper spl typing.
 * 	[92/12/01            af]
 * 
 * Revision 2.10  92/03/10  16:27:35  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:49:17  jsb]
 * 
 * Revision 2.9.2.4  92/02/21  11:24:20  jsb
 * 	In norma_kserver_deliver, don't convert reply to network format.
 * 	[92/02/21  09:04:29  jsb]
 * 
 * Revision 2.9.2.3  92/01/21  21:51:15  jsb
 * 	De-linted.
 * 	[92/01/17  12:20:44  jsb]
 * 
 * Revision 2.9.2.2  92/01/09  18:45:24  jsb
 * 	Added kernel_kmsg_lock. Use splhigh/splx instead of sploff/splon.
 * 	[92/01/08  10:03:53  jsb]
 * 
 * Revision 2.9.2.1  92/01/03  16:37:23  jsb
 * 	Corrected log.
 * 	[91/12/24  14:34:11  jsb]
 * 
 * Revision 2.9  91/12/15  10:42:15  jsb
 * 	Added norma_ipc_finish_receiving call to support large in-line msgs.
 * 
 * Revision 2.8  91/12/14  14:34:23  jsb
 * 	Removed ipc_fields.h hack.
 * 
 * Revision 2.7  91/12/10  13:26:03  jsb
 * 	Use ipc_kmsg_copyout_to_network instead of ipc_kmsg_copyin_from_kernel.
 * 	[91/12/10  11:27:15  jsb]
 * 
 * Revision 2.6  91/11/14  16:52:24  rpd
 * 	Added ipc_fields.h hack.
 *	Use IP_NORMA_IS_PROXY macro instead of ipc_space_remote.
 *	Added missing argument to kernel_thread().
 * 	[91/11/00            jsb]
 * 
 * Revision 2.5  91/08/28  11:16:03  jsb
 * 	As a hack to avoid printfs from i860ipsc/spl.c,
 * 	defined sploff/splon as splsched/splx.
 * 	[91/08/27  21:59:34  jsb]
 * 
 * 	Renamed clport things to norma things.
 * 	[91/08/15  09:11:36  jsb]
 * 
 * Revision 2.4  91/08/03  18:19:22  jsb
 * 	Replaced spldcm/splx with sploff/splon.
 * 	[91/07/28  20:52:22  jsb]
 * 
 * 	Removed obsolete includes and vm and kmsg munging operations.
 * 	[91/07/17  14:14:11  jsb]
 * 
 * 	Moved MACH_MSGH_BITS_COMPLEX_{PORTS,DATA} to mach/message.h.
 * 	[91/07/04  13:12:09  jsb]
 * 
 * 	Use vm_map_copy_t page_lists instead of old style page_lists.
 * 	[91/07/04  10:20:35  jsb]
 * 
 * Revision 2.3  91/07/01  08:25:30  jsb
 * 	Changes for new vm_map_copy_t definition.
 * 	[91/06/29  16:38:27  jsb]
 * 
 * Revision 2.2  91/06/17  15:47:44  jsb
 * 	Moved here from ipc/ipc_clkobject.c.
 * 	[91/06/17  11:05:35  jsb]
 * 
 * Revision 2.2  91/06/06  17:05:23  jsb
 * 	First checkin.
 * 	[91/05/24  13:10:00  jsb]
 * 
 */
/*
 *	File:	norma/ipc_kserver.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Functions to support ipc between nodes in a single Mach cluster.
 */

#include <machine/machspl.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>
#include <mach/vm_param.h>
#include <mach/port.h>
#include <mach/message.h>
#include <kern/assert.h>
#include <kern/host.h>
#include <kern/sched_prim.h>
#include <kern/ipc_sched.h>
#include <kern/ipc_kobject.h>
#include <kern/zalloc.h>
#include <ipc/ipc_mqueue.h>
#include <ipc/ipc_thread.h>
#include <ipc/ipc_kmsg.h>
#include <ipc/ipc_port.h>
#include <ipc/ipc_pset.h>
#include <ipc/ipc_space.h>
#include <ipc/ipc_marequest.h>

#define	NUM_KSERVER_THREADS	4

/*
 * Problems:
 *	Kserver_awake should be used but isn't.
 *	We used to replenish here; should we still?
 *	Kernel_kmsg list is ugly; we should use a queue.
 */

decl_simple_lock_data(,kernel_kmsg_lock)
ipc_kmsg_t kernel_kmsg = (ipc_kmsg_t) 0;
boolean_t kserver_awake = FALSE;
int kserver_awaken = 0;

/*
 * Service request, perhaps blocking; send reply, if any.
 */
norma_kserver_deliver(kmsg)
	ipc_kmsg_t kmsg;
{
	ipc_port_t port;

	norma_ipc_finish_receiving(&kmsg);
	kmsg = ipc_kobject_server(kmsg);
	if (kmsg != IKM_NULL) {
		port = (ipc_port_t) kmsg->ikm_header.msgh_remote_port;
		if (IP_NORMA_IS_PROXY(port)) {
			(void) norma_ipc_send(kmsg);
		} else {
			ipc_mqueue_send_always(kmsg);
		}
	}
}

void
kserver_continue()
{
	spl_t s;
	ipc_kmsg_t kmsg;

	for (;;) {
		kserver_awaken++;

		simple_lock(&kernel_kmsg_lock);
		s = splhigh();
		while (kernel_kmsg) {
			kmsg = kernel_kmsg;
			kernel_kmsg = kmsg->ikm_next;
			splx(s);
			simple_unlock(&kernel_kmsg_lock);
			norma_kserver_deliver(kmsg);
			s = splhigh();
			simple_lock(&kernel_kmsg_lock);
		}

		kserver_awake = FALSE;
		assert_wait((vm_offset_t) &kserver_awake, FALSE);
		(void) splx(s);
		simple_unlock(&kernel_kmsg_lock);
		thread_block(kserver_continue);
	}
}

void
kserver_thread()
{
	spl_t	s;
	
	thread_set_own_priority(0);	/* high priority */

	s = splhigh();
	kserver_awake = FALSE;
	assert_wait((vm_offset_t) &kserver_awake, FALSE);
	(void) splx(s);

	thread_block(kserver_continue);
	kserver_continue();
	/*NOTREACHED*/
}

void
norma_ipc_kobject_send(kmsg)
	ipc_kmsg_t kmsg;
{
	if (kernel_kmsg) {
		ipc_kmsg_t km;
		for (km = kernel_kmsg; km->ikm_next; ) {
			km = km->ikm_next;
		}
		km->ikm_next = kmsg;
	} else {
		kernel_kmsg = kmsg;
	}
	kmsg->ikm_next = 0;
#if 0
	if (! kserver_awake) {
		thread_wakeup_one((vm_offset_t) &kserver_awake);
	}
#else
	thread_wakeup_one((vm_offset_t) &kserver_awake);
#endif
}

norma_kserver_startup()
{
	int i;

	for (i = 0; i < NUM_KSERVER_THREADS; i++) {
		(void) kernel_thread(kernel_task, kserver_thread, (char *) 0);
	}
}
