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
 * $Log:	ipc_migrate.c,v $
 * Revision 2.6  92/03/10  16:27:43  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:49:26  jsb]
 * 
 * Revision 2.5.2.4  92/02/21  11:24:29  jsb
 * 	Don't convert migrated kmsgs to network format.
 * 	[92/02/21  09:06:25  jsb]
 * 
 * 	Changed norma_ipc_send_rright to return uid.
 * 	[92/02/20  17:14:09  jsb]
 * 
 * Revision 2.5.2.3  92/01/21  21:51:25  jsb
 * 	From dlb@osf.org: Fixed ip_kotype assertions and stransit logic.
 * 	[92/01/17  14:39:09  jsb]
 * 
 * 	More de-linting.
 * 	[92/01/17  11:39:52  jsb]
 * 
 * 	De-linted.
 * 	[92/01/16  22:11:26  jsb]
 * 
 * Revision 2.5.2.2  92/01/09  18:45:27  jsb
 * 	Corrected setting of ip_norma_stransit for migrated-from port.
 * 	[92/01/09  13:17:34  jsb]
 * 
 * 	Use remote_host_priv() instead of norma_get_special_port().
 * 	[92/01/04  18:24:21  jsb]
 * 
 * Revision 2.5.2.1  92/01/03  16:37:33  jsb
 * 	Use ipc_port_release instead of ip_release to allow port deallocation.
 * 	Changed ndproxy macros to nsproxy.
 * 	[91/12/31  21:35:52  jsb]
 * 
 * 	Added code to use norma_port_tabled function.
 * 	Added missing ip_reference when receiving a receive right.
 * 	[91/12/31  17:19:11  jsb]
 * 
 * 	Changed printfs to debugging printfs.
 * 	Changes for IP_NORMA_REQUEST macros being renamed to ip_ndproxy{,m,p}.
 * 	[91/12/31  12:20:36  jsb]
 * 
 * 	Fixed reference counting. No-senders now works for migrated ports.
 * 	[91/12/28  18:44:05  jsb]
 * 
 * 	Added source_node parameter to norma_ipc_receive_rright so that
 * 	we know exactly where to send norma_ipc_pull_receive request.
 * 	[91/12/27  21:32:31  jsb]
 * 
 * 	Norma_ipc_pull_receive now uses a temporary proxy for forwarded
 * 	messages, since norma_ipc_send now uses port queue.
 * 	Still need to wait for proxy to drain, and receive_rright still
 * 	needs to not clobber any proxy-queued messages when moving over
 * 	messages received on atrium.
 * 	[91/12/27  17:07:10  jsb]
 * 
 * 	A new implementation that migrates queued messages and port state
 * 	(such as qlimit) when migrating receive rights. Contains reference
 * 	counting workarounds; no-senders does not yet work on migrated ports.
 * 	This implementation does not change the migrated port's uid.
 * 	[91/12/26  18:23:25  jsb]
 * 
 * 	Corrected log. Removed uses of obsolete ipc_port fields.
 * 	Added a debugging printf in place of an assertion failure.
 * 	[91/12/24  14:24:46  jsb]
 * 
 * Revision 2.4  91/12/13  14:00:04  jsb
 * 	Changed debugging printf routines.
 * 
 * Revision 2.3  91/12/10  13:26:05  jsb
 * 	Make sure a port has a uid before migrating its receive right.
 * 	Set ip_receiver_name and ip_destination for migrated right.
 * 	[91/12/10  11:28:53  jsb]
 * 
 * Revision 2.2  91/11/14  16:45:56  rpd
 * 	Created.
 * 
 */
/*
 *	File:	norma/ipc_migrate.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Functions for migration of receive rights between nodes.
 */

#include <norma_vm.h>
#include <norma_ether.h>

#include <vm/vm_kern.h>
#include <mach/vm_param.h>
#include <mach/port.h>
#include <mach/message.h>
#include <kern/assert.h>
#include <kern/host.h>
#include <kern/sched_prim.h>
#include <kern/ipc_sched.h>
#include <kern/ipc_kobject.h>
#include <kern/zalloc.h>
#include <device/device_port.h>
#include <ipc/ipc_mqueue.h>
#include <ipc/ipc_thread.h>
#include <ipc/ipc_kmsg.h>
#include <ipc/ipc_port.h>
#include <ipc/ipc_pset.h>
#include <ipc/ipc_space.h>
#include <ipc/ipc_marequest.h>
#include <norma/ipc_node.h>

extern ipc_port_t norma_port_lookup();
extern ipc_port_t remote_host_priv();
extern unsigned long norma_new_uid();
extern void norma_port_insert();
extern void norma_port_remove();

/*
 * Called whenever a receive right is sent to another node.
 * Port must be locked. Port must be a local port.
 */
unsigned long
norma_ipc_send_rright(port)
	ipc_port_t port;
{
	unsigned long uid;

	/*
	 * If this port has never been exported (for either srights or
	 * sorights), assign it a uid and place it on the norma port list.
	 */
	if (port->ip_norma_uid == 0) {
		port->ip_norma_uid = norma_new_uid();
		norma_port_insert(port);
	} else if (! norma_port_tabled(port)) {
		norma_port_insert(port);
	}
	assert(! port->ip_norma_is_proxy);
	uid = port->ip_norma_uid;

	/*
	 * The only reference after this will be one in the norma port table.
	 */
	ipc_port_release(port);

	/*
	 * For now, we continue to accept messages here.
	 * We turn into a proxy only when we have a dest_node to give senders,
	 * which will be sent by the receiver of this receive right.
	 * (We may not yet know where dest_node is, since the message
	 * carrying this receive right may be indirected...)
	 */
	return uid;
}

/*
 * When a node receives a migrated receive right, it sends this message
 * to the source of the receive right. The source then starts sending
 * queued messages until it runs out...
 * Note of course that if this port is busy enough, the migration will
 * never happen. Not easy to fix this, but it's not a correctness problem
 * and I don't think it will be a problem in practice.
 *
 * This call is executed in a kserver thread context.
 */
norma_ipc_pull_receive(host, uid, dest_node, stransit, sotransit, nsrequest,
		       pdrequest, dnrequest_list, dnrequest_count,
		       seqno, qlimit)
	host_t host;
	unsigned long uid;
	unsigned long dest_node;
	int *stransit;
	int *sotransit;
	ipc_port_t *nsrequest;
	ipc_port_t *pdrequest;
	ipc_port_t *dnrequest_list;
	int *dnrequest_count;
	int *seqno;
	int *qlimit;
{
	ipc_port_t port, proxy;
	ipc_mqueue_t mqueue;
	ipc_kmsg_queue_t kmqueue;
	ipc_kmsg_t kmsg;
	int proxy_stransit;

	printf1("norma_ipc_pull_receive(uid=%x dest=%d)\n", uid, dest_node);
	/*
	 * Guard against random bozos calling this routine.
	 */
	if (host == HOST_NULL) {
		return KERN_INVALID_ARGUMENT;
	}

	/*
	 * Find the port.
	 */
	port = norma_port_lookup(uid);
	assert(port != IP_NULL);
	assert(! port->ip_norma_is_proxy);

	/*
	 * Create a private proxy for forwarding messages via norma_ipc_send.
	 * XXX
	 * Need to wait for it to drain!
	 * XXXO
	 * Could move initial queue of messages from port to proxy
	 */
	proxy = ipc_port_alloc_special(ipc_space_remote);
	if (proxy == IP_NULL) {
		panic("norma_ipc_pull_receive: ipc_port_alloc_special");
	}
	proxy->ip_norma_uid = uid;
	proxy->ip_norma_dest_node = dest_node;
	proxy->ip_norma_is_proxy = TRUE;

	/*
	 * Migrate messages to destination.
	 * New messages may show up. We could turn
	 * them off, perhaps, by adjusting queue
	 * limit, but it's probably not worth it,
	 * and what do we do about SEND_ALWAYS?
	 *
	 * The lack of port locking is particularly conspicuous here.
	 */
	printf1("norma_ipc_pull_receive: moving %d msgs\n", port->ip_msgcount);
	mqueue = &port->ip_messages;
	imq_lock(mqueue);
	assert(ipc_thread_queue_empty(&mqueue->imq_threads));
	kmqueue = &mqueue->imq_messages;
	while ((kmsg = ipc_kmsg_dequeue(kmqueue)) != IKM_NULL) {
		imq_unlock(mqueue);
		assert(kmsg->ikm_header.msgh_remote_port == (mach_port_t)port);

		/*
		 * Undo norma_ipc_receive_dest so that we can compute
		 * local ip_srights below.
		 */
		if (MACH_MSGH_BITS_REMOTE(kmsg->ikm_header.msgh_bits)
		    == MACH_MSG_TYPE_PORT_SEND) {
			port->ip_srights--;
			ipc_port_release(port);
		}

		/*
		 * Send the message.
		 */
		kmsg->ikm_header.msgh_remote_port = (mach_port_t) proxy;
		kmsg->ikm_header.msgh_bits |= MACH_MSGH_BITS_MIGRATED;

		(void) norma_ipc_send(kmsg);
		printf1("sent    kmsg 0x%x\n", kmsg);
		imq_lock(mqueue);
	}
	imq_unlock(mqueue);

	/*
	 * Copy out port state.
	 * Some assertions came from ipc_port_clear_receiver.
	 */
	assert(port->ip_mscount == 0);
	assert(port->ip_seqno == 0);
	assert((ip_kotype(port) == IKOT_NONE) ||
	       (ip_kotype(port) == IKOT_PAGER));
	assert(port->ip_pset == IPS_NULL);
	*nsrequest = port->ip_nsrequest;
	*pdrequest = port->ip_pdrequest;
	*dnrequest_list = (ipc_port_t) 0;	/* XXX */
	*dnrequest_count = 0;			/* XXX */
	*seqno = port->ip_seqno;
	*qlimit = port->ip_qlimit;

	/*
	 * If there are any local send rights, increment stransit by 1
	 * so that recipient of receiver right knows they exist.
	 *
	 * Note that any send rights associated with messages
	 * (as destinations) have already been accounted for above.
	 *
	 * The true number of local send rights is
	 * ip_srights - ip_norma_stransit.
	 */
	assert(port->ip_srights >= port->ip_norma_stransit);
	if (port->ip_srights - port->ip_norma_stransit > 0) {
		proxy_stransit = 1;
	} else {
		proxy_stransit = 0;
	}
	*stransit = port->ip_norma_stransit + proxy_stransit;

	/*
	 * Send count of non-local send-once rights.
	 *
	 * The true number of local send-once rights is
	 * ip_sorights - ip_norma_sotransit.
	 */
	*sotransit = port->ip_norma_sotransit - port->ip_sorights;

	/*
	 * Change port into a proxy.
	 */
	assert((ip_kotype(port) == IKOT_NONE) ||
	       (ip_kotype(port) == IKOT_PAGER));
	port->ip_receiver = ipc_space_remote;
	port->ip_receiver_name = 1; /* name used by ipc_port_alloc_special */

	port->ip_mscount = 0;
	port->ip_srights -= port->ip_norma_stransit;
	port->ip_sorights -= port->ip_norma_sotransit;
	port->ip_references -= (port->ip_norma_stransit +
				port->ip_norma_sotransit);

	port->ip_nsrequest = ip_nsproxym(port);
	port->ip_pdrequest = IP_NULL;
	port->ip_dnrequests = IPR_NULL;

	port->ip_pset = IPS_NULL;
	port->ip_seqno = 0;
	port->ip_msgcount = 0;
	port->ip_qlimit = MACH_PORT_QLIMIT_DEFAULT;

	port->ip_norma_stransit = -proxy_stransit;
	port->ip_norma_sotransit = 0;
	port->ip_norma_dest_node = dest_node;
	port->ip_norma_is_proxy = TRUE;
	port->ip_norma_is_special = FALSE;

	/*
	 * XXX
	 * What do we do with:
	 *	ip_blocked?
	 *
	 */

	/*
	 * We could probably have done this earlier...
	 */
	if (port->ip_srights == 0 && port->ip_sorights == 0) {
		printf1("pull_receive: destroying 0x%x:%x\n",
		       port, port->ip_norma_uid);
		norma_port_remove(port);
	}

	return KERN_SUCCESS;
}

/*
 * Receive a receive right.
 * We create a port and export it, giving the uid that we pick
 * to the node that sent us the receive right. We enter a forwarding
 * from its uid to ours in our forwarding table. We will first receive
 * all the messages queued at its port, then it will stop queueing
 * messages and instead tell nodes to send messages to us.
 *
 * What happens if we had a proxy for this uid?
 * Note that we still have to send messages to the old uid
 * until the real migration happens.
 *
 * This routine is always called from a thread context.
 */
ipc_port_t
norma_ipc_receive_rright(uid, source_node)
	unsigned long uid;
	unsigned long source_node;
{
	kern_return_t kr;
	ipc_port_t port, atrium;
	ipc_port_t *dnrequest_list = (ipc_port_t *) 0;
	int dnrequest_count = 0;
	long stransit, sotransit;

	mumble("receive_rright %x\n", uid);
	assert(source_node != node_self());
	/*
	 * Find or allocate proxy for this uid.
	 * XXX
	 * What keeps this port from being deallocated?
	 * Perhaps we should hack up another sright/reference.
	 */
	port = norma_port_lookup(uid);
	if (port == IP_NULL) {
		mumble("receive_rright: new proxy\n");
		port = ipc_port_alloc_special(ipc_space_remote);
		if (port == IP_NULL) {
			panic("receive_rright: ipc_port_alloc_special");
		}
		port->ip_nsrequest = ip_nsproxym(port);
		port->ip_norma_uid = uid;
		port->ip_norma_dest_node = source_node;
		port->ip_norma_is_proxy = TRUE;
		norma_port_insert(port);
	} else {
		mumble("receive_rright: old proxy\n");
		assert(ip_active(port));
		assert(port->ip_norma_is_proxy);
	}

	/*
	 * I believe there is one port reference associated with
	 * the receive right itself. This ip_reference matches the
	 * ipc_port_release in norma_ipc_send_rright.
	 */
	ip_reference(port);


	/*
	 * Allocate an atrium port to hold forwarded messages.
	 * This gives us one queue for forwarded messages and another
	 * for proxy usage.
	 */
	atrium = ip_alloc();
	printf1("ip_alloc:atrium=0x%x\n", atrium);
	if (atrium == IP_NULL) {
		panic("receive_rright: ip_alloc");
	}
	atrium->ip_receiver = ipc_space_remote;
	atrium->ip_msgcount = 0;
	ipc_kmsg_queue_init(&atrium->ip_messages.imq_messages);
	port->ip_norma_atrium = atrium;

	/*
	 * Pull over the receive right (and all of its messages).
	 */
	printf1("about to call r_norma_ipc_pull_receive\n");
	kr = r_norma_ipc_pull_receive(remote_host_priv(source_node),
				      uid,
				      node_self(),
				      &stransit,
				      &sotransit,
				      &port->ip_nsrequest,
				      &port->ip_pdrequest,
				      &dnrequest_list,
				      &dnrequest_count,
				      &port->ip_seqno,
				      &port->ip_qlimit);
	printf1("r_norma_ipc_pull_receive returns %d/%x\n", kr, kr);
	assert(kr == KERN_SUCCESS);

	/*
	 * Should block in case not all messages have arrived!
	 * (If that's possible, and I don't think it is.)
	 */

	/*
	 *
	 */
	port->ip_norma_stransit = stransit + -(port->ip_norma_stransit);
	port->ip_norma_sotransit = sotransit + -(port->ip_norma_sotransit);

	/*
	 * Set any remaining fields.
	 * XXX Does ip_receiver's value matter here?
	 */
	port->ip_norma_is_proxy = FALSE;
	port->ip_receiver_name = MACH_PORT_NULL;
	port->ip_destination = IP_NULL;
	port->ip_srights += port->ip_norma_stransit;
	port->ip_sorights += port->ip_norma_sotransit;
	port->ip_references += (port->ip_norma_stransit +
				port->ip_norma_sotransit);

	/*
	 * Move messages from atrium to new principal.
	 * Deallocate atrium.
	 *
	 * XXX
	 * Should not lose messages still queued on proxy!
	 */
	printf1("port=0x%x, ->atrium=0x%x atrium=0x%x count=%d\n",
	       port,
	       port->ip_norma_atrium,
	       atrium,
	       atrium->ip_msgcount);
	printf1("--- old count=%d\n", port->ip_msgcount);
	port->ip_msgcount = atrium->ip_msgcount;
	port->ip_messages.imq_messages = atrium->ip_messages.imq_messages;
	ip_free(atrium);
	port->ip_norma_atrium = IP_NULL;

	/*
	 * Return port.
	 */
	mumble("receive_rright: returning port 0x%x\n", port);
	return port;
}
