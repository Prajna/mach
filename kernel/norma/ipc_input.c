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
 * $Log:	ipc_input.c,v $
 * Revision 2.14  93/05/15  19:34:02  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.13  92/03/10  16:27:30  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:49:08  jsb]
 * 
 * Revision 2.11.2.4  92/02/21  11:24:15  jsb
 * 	Store msgh_id in global variable in norma_ipc_finish_receiving,
 * 	for debugging purposes.
 * 	[92/02/20  10:32:54  jsb]
 * 
 * Revision 2.11.2.3  92/01/21  21:51:09  jsb
 * 	Removed global_msgh_id.
 * 	[92/01/17  14:35:50  jsb]
 * 
 * 	More de-linting.
 * 	[92/01/17  11:39:24  jsb]
 * 
 * 	More de-linting.
 * 	[92/01/16  22:10:41  jsb]
 * 
 * 	De-linted.
 * 	[92/01/13  10:15:12  jsb]
 * 
 * 	Fix from dlb to increment receiver->ip_seqno in thread_go case.
 * 	[92/01/11  17:41:46  jsb]
 * 
 * 	Moved netipc_ack status demultiplexing here.
 * 	[92/01/11  17:08:19  jsb]
 * 
 * Revision 2.11.2.2  92/01/09  18:45:18  jsb
 * 	Turned off copy object continuation debugging.
 * 	[92/01/09  15:37:46  jsb]
 * 
 * 	Added support for copy object continuations.
 * 	[92/01/09  13:18:50  jsb]
 * 
 * 	Replaced spls with netipc_thread_{lock,unlock}.
 * 	[92/01/08  10:14:14  jsb]
 * 
 * 	Made out-of-line ports work.
 * 	[92/01/05  17:51:28  jsb]
 * 
 * 	Parameter copy_npages replaced by page_last in norma_deliver_page.
 * 	Removed continuation panic since continuations are coming soon.
 * 	[92/01/05  15:58:34  jsb]
 * 
 * Revision 2.11.2.1  92/01/03  16:37:18  jsb
 * 	Replaced norma_ipc_ack_failure with norma_ipc_ack_{dead,not_found}.
 * 	[91/12/29  16:01:41  jsb]
 * 
 * 	Added type parameter to norma_ipc_receive_migrating_dest.
 * 	Added debugging code to remember msgh_id when creating proxies.
 * 	[91/12/28  18:07:18  jsb]
 * 
 * 	Pass remote node via kmsg->ikm_source_node to norma_ipc_receive_port
 * 	on its way to norma_ipc_receive_rright. Now that we have a real
 * 	ikm_source_node kmsg field, we can get rid of the ikm_remote hack.
 * 	[91/12/27  21:37:36  jsb]
 * 
 * 	Removed unused msgid (not msgh_id) parameters.
 * 	[91/12/27  17:08:39  jsb]
 * 
 * 	Queue migrated messages on atrium port.
 * 	[91/12/26  20:37:49  jsb]
 * 
 * 	Moved translation of local port to norma_receive_complex_ports.
 * 	Moved norma_receive_complex_ports call to norma_ipc_finish_receiving.
 * 	Added code for MACH_MSGH_BITS_MIGRATED, including call to new routine
 * 	norma_ipc_receive_migrating_dest. 
 * 	[91/12/25  16:54:50  jsb]
 * 
 * 	Made large kmsgs work correctly. Corrected log.
 * 	Added check for null local port in norma_deliver_kmsg.
 * 	[91/12/24  14:33:18  jsb]
 * 
 * Revision 2.11  91/12/15  17:31:06  jsb
 * 	Almost made large kmsgs work... now it leaks but does not crash.
 * 	Changed debugging printfs.
 * 
 * Revision 2.10  91/12/15  10:47:09  jsb
 * 	Added norma_ipc_finish_receiving to support large in-line msgs.
 * 	Small clean-up of norma_deliver_page.
 * 
 * Revision 2.9  91/12/14  14:34:11  jsb
 * 	Removed private assert definition.
 * 
 * Revision 2.8  91/12/13  13:55:01  jsb
 * 	Fixed check for end of last copy object in norma_deliver_page.
 * 	Moved norma_ipc_ack_xxx calls to safer places.
 * 
 * Revision 2.7  91/12/10  13:26:00  jsb
 * 	Added support for moving receive rights.
 * 	Use norma_ipc_ack_* upcalls (downcalls?) instead of return values
 * 	from norma_deliver_kmsg and _page.
 * 	Merged dlb check for continuation-needing copy objects in
 * 	norma_deliver_page.
 * 	Added (untested) support for multiple copy objects per message.
 * 	[91/12/10  11:26:32  jsb]
 * 
 * Revision 2.6  91/11/19  09:40:50  rvb
 *	Added new_remote argument to norma_deliver_kmsg to support
 *	migrating receive rights.
 * 	[91/11/00            jsb]
 * 
 * Revision 2.5  91/11/14  16:51:39  rpd
 * 	Replaced norma_ipc_get_proxy with norma_ipc_receive_{port,dest}.
 *	Added check that destination port can accept message.
 * 	Added checks on type of received rights.
 * 	[91/09/19  13:51:21  jsb]
 * 
 * Revision 2.4  91/08/28  11:16:00  jsb
 * 	Mark received pages as dirty and not busy.
 * 	Initialize copy->cpy_cont and copy->cpy_cont_args.
 * 	[91/08/16  10:44:19  jsb]
 * 
 * 	Fixed reference to norma_ipc_kobject_send.
 * 	[91/08/15  08:42:23  jsb]
 * 
 * 	Renamed clport things to norma things.
 * 	[91/08/14  21:34:13  jsb]
 * 
 * 	Fixed norma_ipc_handoff code.
 * 	Added splon/sploff redefinition hack.
 * 	[91/08/14  19:11:07  jsb]
 * 
 * Revision 2.3  91/08/03  18:19:19  jsb
 * 	Use MACH_MSGH_BITS_COMPLEX_DATA instead of null msgid to determine
 * 	whether data follows kmsg.
 * 	[91/08/01  21:57:37  jsb]
 * 
 * 	Eliminated remaining old-style page list code.
 * 	Cleaned up and corrected clport_deliver_page.
 * 	[91/07/27  18:47:08  jsb]
 * 
 * 	Moved MACH_MSGH_BITS_COMPLEX_{PORTS,DATA} to mach/message.h.
 * 	[91/07/04  13:10:48  jsb]
 * 
 * 	Use vm_map_copy_t's instead of old style page_lists.
 * 	Still need to eliminate local conversion between formats.
 * 	[91/07/04  10:18:11  jsb]
 * 
 * Revision 2.2  91/06/17  15:47:41  jsb
 * 	Moved here from ipc/ipc_clinput.c.
 * 	[91/06/17  11:02:28  jsb]
 * 
 * Revision 2.2  91/06/06  17:05:18  jsb
 * 	Fixed copyright.
 * 	[91/05/24  13:18:23  jsb]
 * 
 * 	First checkin.
 * 	[91/05/14  13:29:10  jsb]
 * 
 */
/*
 *	File:	norma/ipc_input.c
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
#include <norma/ipc_net.h>

extern zone_t vm_map_copy_zone;

extern vm_map_copy_t netipc_copy_grab();
extern void norma_ipc_kobject_send();

extern ipc_mqueue_t	norma_ipc_handoff_mqueue;
extern ipc_kmsg_t	norma_ipc_handoff_msg;
extern mach_msg_size_t	norma_ipc_handoff_max_size;
extern mach_msg_size_t	norma_ipc_handoff_msg_size;

extern ipc_port_t	norma_ipc_receive_port();

ipc_kmsg_t norma_kmsg_complete;
ipc_kmsg_t norma_kmsg_incomplete;

int jc_handoff_fasthits = 0;
int jc_handoff_hits = 0;
int jc_handoff_misses = 0;
int jc_handoff_m2 = 0;		/* XXX very rare (0.1 %) */
int jc_handoff_m3 = 0;
int jc_handoff_m4 = 0;
int jc_netipc_ast = 0;

#define	MACH_MSGH_BITS_COMPLEX_ANYTHING	\
	(MACH_MSGH_BITS_COMPLEX_DATA | MACH_MSGH_BITS_COMPLEX_PORTS)

/*
 * Called from a thread context, by the receiving thread.
 * May replace kmsg with new kmsg.
 *
 * (What if the message stays on the queue forever, hogging resources?)
 *
 * The only places Rich and I can think of where messages are received are:
 *	after calling ipc_mqueue_receive
 *	in exception handling path
 *	in kobject server
 */
int input_msgh_id = 0;
norma_ipc_finish_receiving(kmsgp)
	ipc_kmsg_t *kmsgp;
{
	mach_msg_header_t *msgh;
	mach_msg_bits_t mbits;

	/*
	 * Common case: not a norma message.
	 */
	if ((*kmsgp)->ikm_size != IKM_SIZE_NORMA) {
		return;
	}

	/*
	 * Translate local port, if one exists.
	 */
	msgh = &(*kmsgp)->ikm_header;
	input_msgh_id = msgh->msgh_id;
	mbits = msgh->msgh_bits;
	if (msgh->msgh_local_port) {
		/*
		 * We could call the correct form directly,
		 * eliminating the need to pass ikm_source_node.
		 */
		assert(MACH_MSGH_BITS_LOCAL(mbits) !=
		       MACH_MSG_TYPE_PORT_RECEIVE);
		msgh->msgh_local_port = (mach_port_t)
		    norma_ipc_receive_port((unsigned long)
					   msgh->msgh_local_port,
					   MACH_MSGH_BITS_LOCAL(mbits),
					   (*kmsgp)->ikm_source_node);
	}

	/*
	 * Common case: nothing left to do.
	 */
	if ((mbits & MACH_MSGH_BITS_COMPLEX_ANYTHING) == 0) {
		return;
	}

	/*
	 * Do we need to assemble a large message?
	 */
	if (mbits & MACH_MSGH_BITS_COMPLEX_DATA) {
		norma_ipc_receive_complex_data(kmsgp);
	}

	/*
	 * Do we need to process some ports?
	 *
	 * XXX local port handling should always be done here
	 */
	if (mbits & MACH_MSGH_BITS_COMPLEX_PORTS) {
		norma_ipc_receive_complex_ports(*kmsgp);
	}
}

/*
 * Replace fragmented kmsg with contiguous kmsg.
 */
norma_ipc_receive_complex_data(kmsgp)
	ipc_kmsg_t *kmsgp;
{
	ipc_kmsg_t old_kmsg = *kmsgp, kmsg;
	vm_map_copy_t copy;
	int i;

	/*
	 * Assemble kmsg pages into one large kmsg.
	 *
	 * XXX
	 * For now, we do so by copying the pages.
	 * We could remap the kmsg instead.
	 */
	kmsg = ikm_alloc(old_kmsg->ikm_header.msgh_size);
	if (kmsg == IKM_NULL) {
		panic("norma_ipc_finish_receiving: ikm_alloc\n");
		return;
	}

	/*
	 * Copy and deallocate the first page.
	 */
	assert(old_kmsg->ikm_size == IKM_SIZE_NORMA);
	assert(old_kmsg->ikm_header.msgh_size + IKM_OVERHEAD > PAGE_SIZE);
	bcopy((char *) old_kmsg, (char *) kmsg, (int) PAGE_SIZE);
	norma_kmsg_put(old_kmsg);
	ikm_init(kmsg, kmsg->ikm_header.msgh_size);
	kmsg->ikm_header.msgh_bits &= ~MACH_MSGH_BITS_COMPLEX_DATA;

	/*
	 * Copy the other pages.
	 */
	copy = kmsg->ikm_copy;
	for (i = 0; i < copy->cpy_npages; i++) {
		int length;
		vm_page_t m;
		char *page;

		m = copy->cpy_page_list[i];
		if (i == copy->cpy_npages - 1) {
			length = copy->size - i * PAGE_SIZE;
		} else {
			length = PAGE_SIZE;
		}
		assert(length <= PAGE_SIZE);
		assert((i+1) * PAGE_SIZE + length <=
		       ikm_plus_overhead(kmsg->ikm_header.msgh_size));
		page = (char *) phystokv(m->phys_addr);
		bcopy((char *) page, (char *) kmsg + (i+1) * PAGE_SIZE,
		      (int) length);
	}

	/*
	 * Deallocate pages; release copy object.
	 */
	netipc_thread_lock();
	for (i = 0; i < copy->cpy_npages; i++) {
		netipc_page_put(copy->cpy_page_list[i]);
	}
	netipc_copy_ungrab(copy);
	netipc_thread_unlock();

	/*
	 * Return kmsg.
	 */
	*kmsgp = kmsg;
}

vm_offset_t
copy_to_kalloc(copy)
	vm_map_copy_t copy;
{
	vm_offset_t k;
	int i;

	k = kalloc(copy->size);
	assert(k);

	/*
	 * Copy the other pages.
	 */
	for (i = 0; i < copy->cpy_npages; i++) {
		int length;
		vm_page_t m;
		char *page;

		m = copy->cpy_page_list[i];
		if (i == copy->cpy_npages - 1) {
			length = copy->size - i * PAGE_SIZE;
		} else {
			length = PAGE_SIZE;
		}
		assert(length <= PAGE_SIZE);
		page = (char *) phystokv(m->phys_addr);
		bcopy((char *) page, (char *) k + i * PAGE_SIZE, (int) length);
	}

	/*
	 * Deallocate pages; release copy object.
	 */
	netipc_thread_lock();
	for (i = 0; i < copy->cpy_npages; i++) {
		netipc_page_put(copy->cpy_page_list[i]);
	}
	netipc_copy_ungrab(copy);
	netipc_thread_unlock();

	return k;
}

/*
 * Translate ports. Don't do anything with data.
 */
norma_ipc_receive_complex_ports(kmsg)
	ipc_kmsg_t kmsg;
{
	mach_msg_header_t *msgh = &kmsg->ikm_header;
	vm_offset_t saddr = (vm_offset_t) (msgh + 1);
	vm_offset_t eaddr = (vm_offset_t) msgh + msgh->msgh_size;

	msgh->msgh_bits &= ~MACH_MSGH_BITS_COMPLEX_PORTS;
	while (saddr < eaddr) {
		mach_msg_type_long_t *type;
		mach_msg_type_size_t size;
		mach_msg_type_number_t number;
		boolean_t is_inline, longform;
		mach_msg_type_name_t type_name;
		vm_size_t length;

		type = (mach_msg_type_long_t *) saddr;
		is_inline = type->msgtl_header.msgt_inline;
		longform = type->msgtl_header.msgt_longform;
		if (longform) {
			type_name = type->msgtl_name;
			size = type->msgtl_size;
			number = type->msgtl_number;
			saddr += sizeof(mach_msg_type_long_t);
		} else {
			type_name = type->msgtl_header.msgt_name;
			size = type->msgtl_header.msgt_size;
			number = type->msgtl_header.msgt_number;
			saddr += sizeof(mach_msg_type_t);
		}

		/* calculate length of data in bytes, rounding up */
		length = ((number * size) + 7) >> 3;

		if (MACH_MSG_TYPE_PORT_ANY(type_name)) {
			ipc_port_t *ports;
			mach_msg_type_number_t i;

			if (is_inline) {
				ports = (ipc_port_t *) saddr;
			} else if (number > 0) {
				vm_map_copy_t copy = * (vm_map_copy_t *) saddr;
				* (vm_offset_t *) saddr = copy_to_kalloc(copy);
				ports = (ipc_port_t *) * (vm_offset_t *) saddr;
			}
			for (i = 0; i < number; i++) {
				if (type_name == MACH_MSG_TYPE_PORT_RECEIVE) {
					mumble("rright 0x%x\n", ports[i]);
				}
				ports[i] = (ipc_port_t)
				    norma_ipc_receive_port((unsigned long)
							   ports[i],
							   type_name,
							   kmsg->
							   ikm_source_node);
			}
		}

		if (is_inline) {
			saddr += (length + 3) &~ 3;
		} else {
			saddr += sizeof(vm_offset_t);
		}
	}
}

/*
 * Called in ast-mode, where it is safe to execute ipc code but not to block.
 * (This can actually be in an ast, or from an interrupt handler when the
 * processor was in the idle thread or spinning on norma_ipc_handoff_mqueue.)
 *
 * xxx verify port locking
 */
norma_handoff_kmsg(kmsg)
	ipc_kmsg_t kmsg;
{
	ipc_port_t port;
	ipc_mqueue_t mqueue;
	ipc_pset_t pset;
	ipc_thread_t receiver;
	ipc_thread_queue_t receivers;
	
	jc_handoff_fasthits++;

	/*
	 * Change meaning of complex_data bits to mean a kmsg that
	 * must be made contiguous.
	 */
	if (kmsg->ikm_copy == VM_MAP_COPY_NULL) {
		kmsg->ikm_header.msgh_bits &= ~MACH_MSGH_BITS_COMPLEX_DATA;
	} else {
		kmsg->ikm_header.msgh_bits |= MACH_MSGH_BITS_COMPLEX_DATA;
	}

	/*
	 * We must check to see whether this message is destined for a
	 * kernel object. If it is, and if we were to call ipc_mqueue_send,
	 * we would execute the kernel operation, possibly blocking,
	 * which would be bad. Instead, we hand the kmsg off to a kserver
	 * thread which does the delivery and associated kernel operation.
	 */
	port = (ipc_port_t) kmsg->ikm_header.msgh_remote_port;
	assert(IP_VALID(port));
	if (port->ip_receiver == ipc_space_kernel) {
		norma_ipc_kobject_send(kmsg);
		return;
	}

	/*
	 * If this is a migrating message, then just stick it
	 * directly on the queue, and return.
	 */
	if (kmsg->ikm_header.msgh_bits & MACH_MSGH_BITS_MIGRATED) {
		port = port->ip_norma_atrium;
		port->ip_msgcount++;
		ipc_kmsg_enqueue_macro(&port->ip_messages.imq_messages, kmsg);
		return;
	}

	/*
	 * If there is no one spinning waiting for a message,
	 * then queue this kmsg via the normal mqueue path.
	 *
	 * We don't have to check queue length here (or in mqueue_send)
	 * because we already checked it in receive_dest_*.
	 */
	if (norma_ipc_handoff_mqueue == IMQ_NULL) {
		ipc_mqueue_send_always(kmsg);
		return;
	}

	/*
	 * Find the queue associated with this port.
	 */
	ip_lock(port);
	port->ip_msgcount++;
	assert(port->ip_msgcount > 0);
	pset = port->ip_pset;
	if (pset == IPS_NULL) {
		mqueue = &port->ip_messages;
	} else {
		mqueue = &pset->ips_messages;
	}

	/*
	 * If someone is spinning on this queue, we must release them.
	 * However, if the message is too large for them to successfully
	 * receive it, we continue below to find a receiver.
	 */
	if (mqueue == norma_ipc_handoff_mqueue) {
		norma_ipc_handoff_msg = kmsg;
		if (kmsg->ikm_header.msgh_size <= norma_ipc_handoff_max_size) {
			ip_unlock(port);
			return;
		}
		norma_ipc_handoff_msg_size = kmsg->ikm_header.msgh_size;
	}
	
	imq_lock(mqueue);
	receivers = &mqueue->imq_threads;
	ip_unlock(port);
	
	for (;;) {
		receiver = ipc_thread_queue_first(receivers);
		if (receiver == ITH_NULL) {
			/* no receivers; queue kmsg */
			
			ipc_kmsg_enqueue_macro(&mqueue->imq_messages, kmsg);
			imq_unlock(mqueue);
			return;
		}
		
		ipc_thread_rmqueue_first_macro(receivers, receiver);
		assert(ipc_kmsg_queue_empty(&mqueue->imq_messages));
		
		if (kmsg->ikm_header.msgh_size <= receiver->ith_msize) {
			/* got a successful receiver */
			
			receiver->ith_state = MACH_MSG_SUCCESS;
			receiver->ith_kmsg = kmsg;
			receiver->ith_seqno = port->ip_seqno++;
			imq_unlock(mqueue);
			
			thread_go(receiver);
			return;
		}
		
		receiver->ith_state = MACH_RCV_TOO_LARGE;
		receiver->ith_msize = kmsg->ikm_header.msgh_size;
		thread_go(receiver);
	}
}

/*
 * Called from a thread context where it's okay to lock but not to block.
 */
void netipc_ast()
{
	ipc_kmsg_t kmsg;

	netipc_thread_lock();
	while (kmsg = norma_kmsg_complete) {
		norma_kmsg_complete = kmsg->ikm_next;
		norma_handoff_kmsg(kmsg);
	}
	ast_off(cpu_number(), AST_NETIPC);
	netipc_thread_unlock();
}

norma_deliver_kmsg(kmsg, remote)
	ipc_kmsg_t kmsg;
	unsigned long remote;
{
	register mach_msg_header_t *msgh;
	kern_return_t kr;

	assert(netipc_intr_locked());

	/*
	 * Translate remote_port, and check that it can accept a message.
	 */
	kmsg->ikm_copy = VM_MAP_COPY_NULL;
	kmsg->ikm_source_node = remote;
	msgh = (mach_msg_header_t *) &kmsg->ikm_header;
	if (msgh->msgh_bits & MACH_MSGH_BITS_MIGRATED) {
		kr = norma_ipc_receive_migrating_dest(
			(unsigned long) msgh->msgh_remote_port,
			MACH_MSGH_BITS_REMOTE(msgh->msgh_bits),
			(ipc_port_t *) &msgh->msgh_remote_port);
	} else {
		kr = norma_ipc_receive_dest(
			(unsigned long) msgh->msgh_remote_port,
			MACH_MSGH_BITS_REMOTE(msgh->msgh_bits),
			(ipc_port_t *) &msgh->msgh_remote_port);
	}

	/*
	 * If failure, then acknowledge failure now.
	 *
	 * Should this work be done in receive_dest???
	 */
	if (kr != KERN_SUCCESS) {
		norma_ipc_ack(kr, (unsigned long) msgh->msgh_remote_port);
		return;
	}

	/*
	 * Mark kmsg as a norma kmsg so that it gets return to norma pool.
	 * This is also used by norma_ipc_finish_receiving to detect that
	 * it is a norma kmsg.
	 */
	kmsg->ikm_size = IKM_SIZE_NORMA;

	/*
	 * If the message is incomplete, put it on the incomplete list.
	 */
	if (msgh->msgh_bits & MACH_MSGH_BITS_COMPLEX_DATA) {
		kmsg->ikm_next = norma_kmsg_incomplete;
		norma_kmsg_incomplete = kmsg;
		norma_ipc_ack(KERN_SUCCESS, 0L);
		return;
	}
	/*
	 * The message is complete.
	 * If it safe to process it now, do so.
	 */
	if (norma_ipc_handoff_mqueue) {
		norma_ipc_ack(KERN_SUCCESS, 0L);
		norma_handoff_kmsg(kmsg);
		return;
	}
	/*
	 * It is not safe now to process the complete message,
	 * so place it on the list of completed messages,
	 * and post an ast.
	 * XXX
	 * 1. should be conditionalized on whether we really
	 *	are called at interrupt level
	 * 2. should check flag set by *all* idle loops
	 * 3. this comment applies as well to deliver_page
	 */
	{
		register ipc_kmsg_t *kmsgp;
		
		kmsg->ikm_next = IKM_NULL;
		if (norma_kmsg_complete) {
			for (kmsgp = &norma_kmsg_complete;
			     (*kmsgp)->ikm_next;
			     kmsgp = &(*kmsgp)->ikm_next) {
				continue;
			}
			(*kmsgp)->ikm_next = kmsg;
		} else {
			norma_kmsg_complete = kmsg;
		}
	}
	jc_handoff_misses++;
	ast_on(cpu_number(), AST_NETIPC);
	norma_ipc_ack(KERN_SUCCESS, 0L);
}

kern_return_t
norma_deliver_page_continuation(cont_args, copy_result)
	char *cont_args;
	vm_map_copy_t *copy_result;
{
	boolean_t abort;

	abort = (copy_result == (vm_map_copy_t *) 0);
	if (abort) {
		/*
		 * XXX need to handle this
		 */
		panic("norma_deliver_page_continuation: abort!\n");
		return KERN_SUCCESS;
	} else {
		*copy_result = (vm_map_copy_t) cont_args;
		return KERN_SUCCESS;
	}
}

norma_deliver_page(page, copy_msgh_offset, remote, page_first, page_last,
		   copy_last, copy_size, copy_offset)
	vm_page_t page;
	unsigned long copy_msgh_offset;
	unsigned long remote;
	boolean_t page_first;
	boolean_t page_last;
	boolean_t copy_last;
	unsigned long copy_size;
	unsigned long copy_offset;
{
	ipc_kmsg_t kmsg, *kmsgp;
	vm_map_copy_t copy, *copyp, new_copy;

	assert(netipc_intr_locked());

	/*
	 * Find appropriate kmsg.
	 * XXX consider making this an array?
	 */
	for (kmsgp = &norma_kmsg_incomplete; ; kmsgp = &kmsg->ikm_next) {
		if (! (kmsg = *kmsgp)) {
			panic("norma_deliver_page: kmsg not found");
			return;
		}
		if (kmsg->ikm_source_node == remote) {
			break;
		}
	}

	/*
	 * Find the location of the copy within the kmsg.
	 */
	if (copy_msgh_offset == 0) {
		copyp = &kmsg->ikm_copy;
	} else {
		copyp = (vm_map_copy_t *)
		    ((vm_offset_t) &kmsg->ikm_header + copy_msgh_offset);
	}

	/*
	 * If this is the first page, create a copy object.
	 */
	if (page_first) {
		copy = netipc_copy_grab();
		if (copy == VM_MAP_COPY_NULL) {
			norma_ipc_drop();
			return;
		}
		copy->type = VM_MAP_COPY_PAGE_LIST;
		copy->cpy_npages = 1;
		copy->offset = copy_offset;
		copy->size = copy_size;
		copy->cpy_page_list[0] = page;
		copy->cpy_cont = ((kern_return_t (*)()) 0);
		copy->cpy_cont_args = (char *) VM_MAP_COPYIN_ARGS_NULL;
		*copyp = copy;
		goto test_for_completion;
	}

	/*
	 * There is a preexisting copy object.
	 * If we are in the first page list, things are simple.
	 */
	copy = *copyp;
	if (copy->cpy_npages < VM_MAP_COPY_PAGE_LIST_MAX) {
		copy->cpy_page_list[copy->cpy_npages++] = page;
		goto test_for_completion;
	}

	/*
	 * We are beyond the first page list.
	 * Chase list of copy objects until we are in the last one.
	 */
	printf3("deliver_page: npages=%d\n", copy->cpy_npages);
	while (vm_map_copy_has_cont(copy)) {
		copy = (vm_map_copy_t) copy->cpy_cont_args;
	}

	/*
	 * Will we fit in this page list?
	 * Note: this may still be the first page list,
	 * but in that case the test will fail.
	 */
	if (copy->cpy_npages < VM_MAP_COPY_PAGE_LIST_MAX) {
		copy->cpy_page_list[copy->cpy_npages++] = page;
		(*copyp)->cpy_npages++;
		goto test_for_completion;
	}

	/*
	 * We won't fit; we have to create a continuation.
	 */
	printf3("deliver_page: new cont, copy=0x%x\n", copy);
	assert(copy->cpy_cont_args == (char *) 0);
	if (copy != *copyp) {
		/*
		 * Only first copy object has fake (grand total) npages.
		 * Only first copy object has unalligned offset.
		 */
		assert(copy->cpy_npages == VM_MAP_COPY_PAGE_LIST_MAX);
		assert(copy->offset == 0);
	}
	new_copy = netipc_copy_grab();
	if (new_copy == VM_MAP_COPY_NULL) {
		norma_ipc_drop();
		return;
	}
	new_copy->cpy_page_list[0] = page;
	new_copy->cpy_npages = 1;
	new_copy->cpy_cont = ((kern_return_t (*)()) 0);
	new_copy->cpy_cont_args = (char *) VM_MAP_COPYIN_ARGS_NULL;
	new_copy->size = copy->size -
	    (PAGE_SIZE * VM_MAP_COPY_PAGE_LIST_MAX - copy->offset);
	assert(trunc_page(copy->offset) == 0);
	new_copy->offset = 0;
	copy->cpy_cont = norma_deliver_page_continuation;
	copy->cpy_cont_args = (char *) new_copy;
	(*copyp)->cpy_npages++;

test_for_completion:
	/*
	 * Mark page dirty (why?) and not busy.
	 */
	assert(! page->tabled);
	page->busy = FALSE;
	page->dirty = TRUE;

	/*
	 * We were able to put the page in a page list somewhere.
	 * We therefore know at this point that this call will succeed,
	 * so acknowledge the page.
	 */
	norma_ipc_ack(KERN_SUCCESS, 0L);

	/*
	 * If this is the last page in the copy object, then
	 * correct copy->cpy_npages. If this is not the last page,
	 * then the message is not yet complete, so return now.
	 */
	if (page_last) {
		if ((*copyp)->cpy_npages > VM_MAP_COPY_PAGE_LIST_MAX) {
			(*copyp)->cpy_npages = VM_MAP_COPY_PAGE_LIST_MAX;
		}
	} else {
		return;
	}

	/*
	 * If this is not the last copy object, then the message is
	 * not yet complete, so return.
	 */
	if (! copy_last) {
		return;
	}

	/*
	 * The message is complete. Take it off the list.
	 */
	*kmsgp = kmsg->ikm_next;

	/*
	 * If it safe for us to process the message, do so.
	 * XXX
	 * 1. should be conditionalized on whether we really
	 *	are called at interrupt level
	 * 2. should check flag set by *all* idle loops
	 * 3. this comment applies as well to deliver_kmsg
	 */
	if (norma_ipc_handoff_mqueue) {
		norma_handoff_kmsg(kmsg);
		return;
	}

	/*
	 * It is not safe for us to process the message, so post an ast.
	 * XXX
	 * Should use queue macros
	 */
	kmsg->ikm_next = IKM_NULL;
	if (norma_kmsg_complete) {
		for (kmsgp = &norma_kmsg_complete;
		     (*kmsgp)->ikm_next;
		     kmsgp = &(*kmsgp)->ikm_next) {
			continue;
		}
		(*kmsgp)->ikm_next = kmsg;
	} else {
		norma_kmsg_complete = kmsg;
	}
	jc_handoff_misses++;
	ast_on(cpu_number(), AST_NETIPC);
}
