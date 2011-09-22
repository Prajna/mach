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
 * $Log:	ipc_clean.c,v $
 * Revision 2.2  92/03/10  16:27:19  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:48:39  jsb]
 * 
 * Revision 2.1.2.3  92/02/21  11:24:11  jsb
 * 	Moved ipc_kmsg_uncopyout_to_network here from ipc/ipc_kmsg.c.
 * 	Separated norma_ipc_uncopyout_all_kmsgs out from
 * 	norma_ipc_dead_destination (renamed from norma_ipc_destroy_proxy).
 * 	[92/02/21  10:38:49  jsb]
 * 
 * 	Eliminated norma_ipc_unsend_*_dest, since norma_ipc_send_*_dest
 * 	is no longer called until and unless message is successfully
 * 	received. Added check to ensure that destroyed proxy has queued
 * 	messages. Added call to norma_port_remove before call to
 * 	ipc_port_destroy. Removed zeroing of ip_seqno.
 * 	[92/02/18  08:44:28  jsb]
 * 
 * Revision 2.1.2.2  92/01/21  21:51:00  jsb
 * 	De-linted.
 * 	[92/01/16  21:31:56  jsb]
 * 
 * Revision 2.1.2.1  92/01/03  08:57:08  jsb
 * 	First NORMA branch checkin.
 * 
 * Revision 2.1.1.3  91/12/31  21:36:40  jsb
 * 	Changed ndproxy macros to nsproxy.
 * 
 * Revision 2.1.1.2  91/12/31  12:11:04  jsb
 * 	Changed remote_port handling in norma_ipc_destroy_proxy to account
 * 	for remote consumption of send-once right (if any) in first kmsg.
 * 	Added ipc_kmsg_uncopyout_to_network, which is given remote_port which
 * 	it in turn hands to norma_ipc_unsend{_migrating,}_dest, which now
 * 	use this port instead of trying to look it up based on uid.
 * 	Changes for IP_NORMA_REQUEST macros being renamed to ip_ndproxy{,m,p}.
 * 
 * Revision 2.1.1.1  91/12/29  21:42:51  jsb
 * 	First checkin.
 * 
 */ 
/*
 *	File:	norma/ipc_clean.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Routines to clean messages sent to the network.
 */

#include <ipc/ipc_port.h>
#include <ipc/ipc_kmsg.h>

extern ipc_port_t norma_port_lookup();
extern ipc_port_t norma_ipc_receive_soright();
extern ipc_port_t norma_ipc_receive_sright();

extern ipc_port_t norma_ipc_unsend_port();
extern ipc_port_t norma_ipc_unsend_soright();
extern ipc_port_t norma_ipc_unsend_sright();
extern ipc_port_t norma_ipc_unsend_rright();
extern void norma_ipc_unsend_dest();
extern void norma_ipc_unsend_migrating_dest();

/*
 * These unsend routines should undo the effects of the send routines above.
 */

ipc_port_t
norma_ipc_unsend_port(uid, type_name)
	unsigned long uid;
	mach_msg_type_name_t type_name;
{
	if (type_name == MACH_MSG_TYPE_PORT_SEND_ONCE) {
		return norma_ipc_unsend_soright(uid);
	} else if (type_name == MACH_MSG_TYPE_PORT_SEND) {
		return norma_ipc_unsend_sright(uid);
	} else if (type_name == MACH_MSG_TYPE_PORT_RECEIVE) {
		return norma_ipc_unsend_rright(uid);
	} else {
		panic("norma_ipc_unsend_port: bad type %d\n", type_name);
		/* NOTREACHED */
	}
}

ipc_port_t
norma_ipc_unsend_soright(uid)
	unsigned long uid;
{
	return norma_ipc_receive_soright(uid);
}

ipc_port_t
norma_ipc_unsend_sright(uid)
	unsigned long uid;
{
	return norma_ipc_receive_sright(uid);
}

ipc_port_t
norma_ipc_unsend_rright(uid)
	unsigned long uid;
{
	ipc_port_t port;

	port = norma_port_lookup(uid);
	if (port == IP_NULL) {
		return IP_NULL;
	}
	if (! ip_active(port)) {
		return IP_DEAD;
	}
	assert(! port->ip_norma_is_proxy);
	ip_reference(port);
	return port;
}

void
ipc_kmsg_uncopyout_header_to_network(msgh, dest)
	mach_msg_header_t *msgh;
	ipc_port_t dest;
{
	/*
	 * Uncopy local port
	 */
	if (msgh->msgh_local_port) {
		register mach_msg_type_name_t type;

		type = MACH_MSGH_BITS_LOCAL(msgh->msgh_bits);
		type = ipc_object_copyin_type(type);
		msgh->msgh_local_port = (mach_port_t)
		    norma_ipc_unsend_port((unsigned long)msgh->msgh_local_port,
					  type);
	}

	/*
	 * Uncopy remote port
	 */
	assert(msgh->msgh_remote_port == (mach_port_t) dest->ip_norma_uid);
	msgh->msgh_remote_port = (mach_port_t) dest;
}

void
ipc_kmsg_uncopyout_to_network(kmsg)
	ipc_kmsg_t kmsg;
{
	vm_offset_t saddr, eaddr;

	if ((kmsg->ikm_header.msgh_bits & MACH_MSGH_BITS_COMPLEX) == 0) {
		return;
	}

	kmsg->ikm_header.msgh_bits &= ~ (MACH_MSGH_BITS_COMPLEX_DATA |
					 MACH_MSGH_BITS_COMPLEX_PORTS |
					 MACH_MSGH_BITS_MIGRATED);

	saddr = (vm_offset_t) (&kmsg->ikm_header + 1);
	eaddr = (vm_offset_t) &kmsg->ikm_header + kmsg->ikm_header.msgh_size;

	while (saddr < eaddr) {
		mach_msg_type_long_t *type;
		mach_msg_type_name_t name;
		mach_msg_type_size_t size;
		mach_msg_type_number_t number;
		boolean_t is_inline, longform, is_port;
		vm_offset_t data;
		vm_size_t length;

		type = (mach_msg_type_long_t *) saddr;
		is_inline = type->msgtl_header.msgt_inline;
		longform = type->msgtl_header.msgt_longform;
		/* type->msgtl_header.msgt_deallocate not used */
		if (longform) {
			name = type->msgtl_name;
			size = type->msgtl_size;
			number = type->msgtl_number;
			saddr += sizeof(mach_msg_type_long_t);
		} else {
			name = type->msgtl_header.msgt_name;
			size = type->msgtl_header.msgt_size;
			number = type->msgtl_header.msgt_number;
			saddr += sizeof(mach_msg_type_t);
		}

		/* calculate length of data in bytes, rounding up */

		length = ((number * size) + 7) >> 3;

		is_port = MACH_MSG_TYPE_PORT_ANY(name);

		if (is_inline) {
			/* inline data sizes round up to int boundaries */

			data = saddr;
			saddr += (length + 3) &~ 3;
		} else if (is_port) {
			/*
			 * Not a copy object, just a kernel address.
			 */
			data = * (vm_offset_t *) saddr;
			saddr += sizeof(vm_offset_t);
		} else {
			/*
			 * This is a copy object.
			 * It is okay to leave copy objects in page-list form.
			 * The netipc module is responsible for leaving them
			 * in a sane state.
			 */
			saddr += sizeof(vm_offset_t);
		}

		if (is_port) {
			ipc_port_t *ports = (ipc_port_t *) data;
			mach_msg_type_number_t i;

			/*
			 * Convert uids into ports and undo refcount action.
			 */
			for (i = 0; i < number; i++) {
				ports[i] =
				    norma_ipc_unsend_port((unsigned long)
							  ports[i], name);
			}
		}
	}
}

/*
 * Convert all kmsgs queued on port from network to internal format.
 */
void
norma_ipc_uncopyout_all_kmsgs(port)
	ipc_port_t port;
{
	register ipc_kmsg_t kmsg;
	register ipc_kmsg_queue_t kmsgs;

	kmsgs = &port->ip_messages.imq_messages;
	kmsg = ipc_kmsg_queue_first(kmsgs);
	if (kmsg != IKM_NULL) do {
		printf1("uncopyout ksmg=0x%x msgh_id=%d; was s=%d so=%d\n",
			kmsg, kmsg->ikm_header.msgh_id,
			port->ip_srights, port->ip_sorights);

		/*
		 * Uncopy header
		 */
		ipc_kmsg_uncopyout_header_to_network(&kmsg->ikm_header, port);

		/*
		 * Uncopy body
		 */
		ipc_kmsg_uncopyout_to_network(kmsg);

		/*
		 * Move to next kmsg
		 */
		kmsg = kmsg->ikm_next;
	} while (kmsg != ipc_kmsg_queue_first(kmsgs));
}

/*
 * Called when a send to a remote port returns a notification that
 * the port is dead.
 */
norma_ipc_dead_destination(port)
	ipc_port_t port;
{
	ipc_kmsg_t kmsg;
	mach_port_rights_t sorights;

	printf1("norma_ipc_dead_destination: 0x%x:%x refs %d\n",
		port, port->ip_norma_uid, port->ip_references);

	assert(port->ip_norma_is_proxy);
	assert(port->ip_references > 0);

	/*
	 * Convert all kmsgs from network to internal format.
	 * XXX
	 * We need to lock to prevent anyone else from
	 * queueing ports from this point.
	 */
	norma_ipc_uncopyout_all_kmsgs(port);

	/*
	 * The remote kernel counts us as having consumed one send-once
	 * right, if our destination right was a send-once right.
	 * This reduces how often we need to call norma_ipc_yield_rights.
	 */
	kmsg = ipc_kmsg_queue_first(&port->ip_messages.imq_messages);
	assert(kmsg != IKM_NULL);
	if (MACH_MSGH_BITS_REMOTE(kmsg->ikm_header.msgh_bits) ==
	    MACH_MSG_TYPE_PORT_SEND_ONCE) {
		sorights = port->ip_sorights - 1;
	} else {
		sorights = port->ip_sorights;
	}

	printf1("*** That's really s=%d so=%d\n", port->ip_srights, sorights);

	if (port->ip_srights > 0 || sorights > 0) {
		printf("dead_destination(0x%x) s=%d so=%d st=%d (leak)\n",
		       port,
		       port->ip_srights,
		       port->ip_sorights,
		       - port->ip_norma_stransit);
#if 0
		/* XXX */
		norma_ipc_yield_rights(0, -port->ip_stransit, sorights);
#endif
	}

	/*
	 * Remove port from proxy list before destroying it.
	 */
	norma_port_remove(port);

	/*
	 * Fiddle with fields to make ipc_port_destroy happy, then call it.
	 */
	assert(ip_nsproxyp(port->ip_nsrequest));
	port->ip_nsrequest = IP_NULL;
	printf1("dead_destination: 0x%x:%x: about to destroy port\n",
		port, port->ip_norma_uid);
	ipc_port_destroy(port);
	printf1("dead_destination: 0x%x:%x: destroyed port\n",
		port, port->ip_norma_uid);
}
