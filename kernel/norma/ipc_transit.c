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
 * $Log:	ipc_transit.c,v $
 * Revision 2.6  92/03/10  16:28:25  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:50:27  jsb]
 * 
 * Revision 2.5.2.4  92/02/21  11:24:59  jsb
 * 	Norma_ipc_send_{port,soright,sright,rright} now return uids.
 * 	Use new norma_ipc_remove_try routine.
 * 	[92/02/20  17:13:19  jsb]
 * 
 * 	Changed reference counting in norma_ipc_send_*_dest, now that they
 * 	are called only after successful acknowledgement of message.
 * 	Removed calls to db_show_all_uids, now that 'show all uids' exists.
 * 	Added rearm of ip_nsrequest in norma_ipc_notify_no_senders.
 * 	[92/02/18  08:58:56  jsb]
 * 
 * 	Added checks for losing all send rights without stransit becoming zero,
 * 	in which case we must generate a no-local-senders notification.
 * 	[92/02/16  15:22:29  jsb]
 * 
 * Revision 2.5.2.3  92/01/21  21:52:42  jsb
 * 	More de-linting.
 * 	[92/01/17  11:42:23  jsb]
 * 
 * 	De-linted.
 * 	[92/01/16  22:16:55  jsb]
 * 
 * Revision 2.5.2.2  92/01/09  18:45:59  jsb
 * 	Use quieter debugging printf when queue limit exceeded.
 * 	[92/01/09  16:13:18  jsb]
 * 
 * 	Use remote_host_priv() instead of norma_get_special_port().
 * 	Use new routines norma_unset_special_port and {local,remote}_special.
 * 	[92/01/04  18:20:55  jsb]
 * 
 * Revision 2.5.2.1  92/01/03  16:38:04  jsb
 * 	Use ipc_port_release instead of ip_release to allow port deallocation.
 * 	Added ip_reference to norma_ipc_send_migrating_dest so that
 * 	norma_ipc_send can always release a reference.
 * 	Changed ndproxy macros to nsproxy.
 * 	[91/12/31  21:34:24  jsb]
 * 
 * 	Added code to use norma_port_tabled function, which separates the
 * 	issues of whether a port has a uid and whether it is accessible
 * 	via norma_port_lookup.
 * 	Added a hard panic on a failed lookup in norma_ipc_no_local_senders.
 * 	Test for and ignore special uids in norma_ipc_notify_no_senders.
 * 	[91/12/31  17:14:19  jsb]
 * 
 * 	Added code to destroy active principal when no remote refs are left.
 * 	Added forwarding and absorbtion code to norma_ipc_no_local_senders.
 * 	Declared norma_ipc_no_local_senders void; changed returns accordingly.
 * 	Split norma_ipc_send_no_local_senders from norma_ipc_notify_no_senders.
 * 	Changed printfs to debugging printfs.
 * 	[91/12/31  11:57:31  jsb]
 * 
 * 	Changes for IP_NORMA_REQUEST macros being renamed to ip_ndproxy{,m,p}.
 * 	[91/12/30  10:18:08  jsb]
 * 
 * 	Test for proxies in norma_ipc_port_destroy.
 * 	[91/12/29  21:24:18  jsb]
 * 
 * 	Distinguish between dead and unfound ports in norma_ipc_receive_dest.
 * 	[91/12/29  15:58:20  jsb]
 * 
 * 	Use IP_NORMA_NSREQUEST macros. Release proxies in all cases.
 * 	Simulate copyin in receive_migrating_dest to account for
 * 	migrated messages.
 * 	[91/12/28  18:05:27  jsb]
 * 
 * 	Added source_node parameter to norma_ipc_receive_port so that it can
 * 	be passed to norma_ipc_receive_rright.
 * 	[91/12/27  21:33:34  jsb]
 * 
 * 	Norma_ipc_send_migrating_dest now expects a proxy.
 * 	[91/12/27  17:00:19  jsb]
 * 
 * 	Added norma_ipc_{send,receive}_migrating_dest routines.
 * 	Removed migrated parameter from norma_ipc_receive_dest.
 * 	Removed migration/retarget debugging code.
 * 	[91/12/26  20:45:44  jsb]
 * 
 * 	Replaced token-based implementation with stransit_request based one.
 * 	Added norma_ipc_send_dest. Removed norma_port_wire hack.
 * 	Moved norma_port_list routines to norma/ipc_list.c. Corrected log.
 * 	[91/12/24  14:23:14  jsb]
 * 
 * Revision 2.5  91/12/14  14:35:10  jsb
 * 	Removed ipc_fields.h hack.
 * 
 * Revision 2.4  91/12/13  14:09:42  jsb
 * 	Changed printfs to debugging printfs.
 * 
 * Revision 2.3  91/11/19  09:41:28  rvb
 * 	Added norma_port_wire hack, to avoid reference counting bug.
 *	Added code to test receive right migration.
 * 	[91/11/00            jsb]
 * 
 * Revision 2.2  91/11/14  16:46:18  rpd
 * 	Created.
 */
/*
 *	File:	norma/ipc_transit.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Functions for movement of rights between nodes, excluding receive
 *	rights (for which see norma/ipc_migrate.c).
 */

#include <ipc/ipc_port.h>
#include <ipc/ipc_space.h>
#include <kern/host.h>
#include <norma/ipc_node.h>

extern ipc_port_t local_special();
extern ipc_port_t remote_special();
extern ipc_port_t remote_host_priv();
extern unsigned long norma_new_uid();

extern unsigned long norma_ipc_send_port();
extern unsigned long norma_ipc_send_soright();
extern unsigned long norma_ipc_send_sright();
extern unsigned long norma_ipc_send_rright();
extern void norma_ipc_send_dest();
extern void norma_ipc_send_migrating_dest();

extern ipc_port_t norma_ipc_receive_port();
extern ipc_port_t norma_ipc_receive_sright();
extern ipc_port_t norma_ipc_receive_soright();
extern ipc_port_t norma_ipc_receive_rright();
extern kern_return_t norma_ipc_receive_dest();
extern kern_return_t norma_ipc_receive_migrating_dest();

extern ipc_port_t norma_port_lookup();
extern ipc_port_t norma_port_lookup_locked();
extern void norma_port_insert();
extern void norma_port_remove();
extern void norma_port_remove_locked();

/*
 * Note: no decisions should be made based on who you think the receiver
 * will be, since it might not be who you think it will be.
 * (Receive rights move.) The only exception is norma_ipc_send_dest.
 * For example, if you have a send right and you are sending it to
 * who you think the receiver is, XXX finish this comment
 */

/*
 * Called when a port right of any flavor is sent to another node.
 * Port must be unlocked. Port may be a local or proxy port.
 */
unsigned long
norma_ipc_send_port(port, type_name)
	ipc_port_t port;
	mach_msg_type_name_t type_name;
{
	unsigned long uid;

	ip_lock(port);
	if (type_name == MACH_MSG_TYPE_PORT_SEND_ONCE) {
		uid = norma_ipc_send_soright(port);
	} else if (type_name == MACH_MSG_TYPE_PORT_SEND) {
		uid = norma_ipc_send_sright(port);
	} else if (type_name == MACH_MSG_TYPE_PORT_RECEIVE) {
		uid = norma_ipc_send_rright(port);
	} else {
		panic("norma_ipc_send_port: bad type %d\n", type_name);
	}
	ip_unlock(port);
	return uid;
}

/*
 * Called whenever a send right is sent to another node.
 * Port must be locked. Port may be a local or proxy port.
 */
unsigned long
norma_ipc_send_sright(port)
	ipc_port_t port;
{
	unsigned long uid;

	/*
	 * Port must be active.
	 */
	assert(ip_active(port));

	if (port->ip_norma_is_proxy) {
		/*
		 * A proxy must always have a nonpositive stransit,
		 * so that total stransit is no greater than the
		 * principal's stransit.
		 *
		 * Indeed, a proxy must have a negative stransit as
		 * long as it holds send rights, so that the principal
		 * can trust its stransit (in the face of proxies
		 * sending send rights to it).
		 * If we're here, then we must have send rights.
		 */
		mumble("norma_ipc_send_sright: sender sending sright\n");
		assert(port->ip_srights > 0);
		assert(port->ip_norma_stransit < 0);
		if (port->ip_srights == 1) {
			/*
			 * We are about to lose all send rights.
			 * It is therefore okay to let stransit
			 * drop to zero, as an implicit no-local-senders
			 * notification. (If stransit does not drop
			 * to zero, then we have to do an explicit
			 * no-local-senders notification.) This works
			 * since either we are sending to the receiver,
			 * who will be able to figure it out (since
			 * he will decr stransit accordingly), or
			 * we are not sending to receiver, in which
			 * case no-senders isn't true.
			 */
		} else {
			/*
			 * We will still retain some send rights,
			 * and must therefore keep stransit negative.
			 */
			assert(port->ip_srights > 1);
			assert(port->ip_norma_stransit <= -1);
			if (port->ip_norma_stransit == -1) {
				norma_ipc_stransit_wait(port);
			}
		}
		port->ip_norma_stransit++;
		assert(port->ip_norma_stransit <= 0);

		/*
		 * Save uid
		 */
		uid = port->ip_norma_uid;

		/*
		 * Release send right. If this is the last send right,
		 * and stransit is not zero, then we must generate
		 * an explicit no-local-senders notification.
		 */
		if (port->ip_srights == 1 && port->ip_norma_stransit != 0) {
			ipc_port_release_send(port);
		} else {
			port->ip_srights--;
			ipc_port_release(port);
		}
	} else {
		/*
		 * The principal can always immediately increase stransit.
		 * He does not release send right here, but rather when
		 * stransit comes back via no-local-senders notification.
		 * This allows the principal's send right count to be an
		 * upper bound on the true number of send rights in the
		 * system, which among other things keeps the a lack of
		 * senders on the principal's node from triggering a
		 * premature no-senders notification.
		 */
		assert(port->ip_norma_stransit >= 0);
		port->ip_norma_stransit++;
		
		/*
		 * If this port has never been exported, assign it a uid
		 * and place it on the norma port list.
		 */
		if ((uid = port->ip_norma_uid) == 0) {
			uid = port->ip_norma_uid = norma_new_uid();
			norma_port_insert(port);
		} else if (! norma_port_tabled(port)) {
			norma_port_insert(port);
		}
		assert(! port->ip_norma_is_proxy);
	}
	assert(uid != 0);
	return uid;
}

/*
 * Called whenever a send-once right is sent to another node.
 * Port must be locked. Port must be a local port.
 */
unsigned long
norma_ipc_send_soright(port)
	ipc_port_t port;
{
	unsigned long uid;

	/*
	 * Port must be active.
	 */
	assert(ip_active(port));

	if (port->ip_norma_is_proxy) {
		/*
		 * Save uid
		 */
		uid = port->ip_norma_uid;

		/*
		 * A proxy releases the send-once right.
		 * This case occurs when moving a send-once right
		 * to another node.
		 */
		port->ip_sorights--;
		ipc_port_release(port);
		norma_port_remove_try(port);
	} else {
		/*
		 * The principal does not release send-once right here, but
		 * rather when the send-once right is used as a destination
		 * (of either a reply or a send-once notification).
		 * This allows the principal's send-once right count to
		 * be an accurate count of the true number of send-once
		 * rights in the system.
		 */
		port->ip_norma_sotransit++;

		/*
		 * If this port has never been exported, assign it a uid
		 * and place it on the norma port list.
		 */
		if ((uid = port->ip_norma_uid) == 0) {
			uid = port->ip_norma_uid = norma_new_uid();
			norma_port_insert(port);
		} else if (! norma_port_tabled(port)) {
			norma_port_insert(port);
		}
		assert(! port->ip_norma_is_proxy);
	}
	assert(uid != 0);
	return uid;
}

void
norma_ipc_send_dest(port, type_name)
	ipc_port_t port;
	mach_msg_type_name_t type_name;
{
	/*
	 * ipc_kmsg_copyin_header asserts that this is so.
	 */
	assert(type_name == MACH_MSG_TYPE_PORT_SEND ||
	       type_name == MACH_MSG_TYPE_PORT_SEND_ONCE);

	/*
	 * This port must be a proxy!
	 */
	assert(port->ip_norma_uid != 0);
	assert(port->ip_norma_is_proxy);

	/*
	 * This is different from norma_ipc_send_port, because we know
	 * we are sending to receiver. We just need to undo refcount
	 * changes performed by copyin; we don't need to modify
	 * stransit, and neither does the receiver. However, if we lose
	 * all send rights here (e.g., from a move_send destination),
	 * then we must generate an explicit no-local-senders notification.
	 */
	if (type_name == MACH_MSG_TYPE_PORT_SEND) {
		if (port->ip_srights == 1) {
			printf1("norma_ipc_send_dest.s: release %x\n", port);
			ipc_port_release_send(port);
			printf1("norma_ipc_send_dest.s: released %x\n", port);
		} else {
			assert(port->ip_srights > 1);
			ip_release(port);
			port->ip_srights--;
		}
	} else {
		ip_release(port);
		port->ip_sorights--;
		norma_port_remove_try(port);
	}
}

void
norma_ipc_send_migrating_dest(port)
	ipc_port_t port;
{
	/*
	 * This port must be a proxy with a uid.
	 */
	assert(port->ip_norma_uid != 0);
	assert(port->ip_norma_is_proxy);
	assert(port->ip_norma_dest_node != node_self());
}

/*
 * Called when a port right of any flavor is received from another node.
 */
ipc_port_t
norma_ipc_receive_port(uid, type_name, source_node)
	unsigned long uid;
	mach_msg_type_name_t type_name;
	unsigned long source_node;
{
	/*
	 * A null uid maps to a null port.
	 */
	if (uid == 0) {
		return IP_NULL;
	}

	if (type_name == MACH_MSG_TYPE_PORT_SEND_ONCE) {
		return norma_ipc_receive_soright(uid);
	} else if (type_name == MACH_MSG_TYPE_PORT_SEND) {
		return norma_ipc_receive_sright(uid);
	} else if (type_name == MACH_MSG_TYPE_PORT_RECEIVE) {
		return norma_ipc_receive_rright(uid, source_node);
	} else {
		panic("norma_ipc_receive_port: bad type %d\n", type_name);
		return IP_NULL;
	}
}

/*
 * Find or create proxy for given uid, and add a send right reference.
 */
ipc_port_t
norma_ipc_receive_sright(uid)
	unsigned long uid;
{
	ipc_port_t port;

	/*
	 * Try to find the port.
	 */
	port = norma_port_lookup(uid);

	/*
	 * If we don't have a port, then we must create a proxy.
	 */
	if (port == IP_NULL) {
		assert(IP_NORMA_NODE(uid) != node_self());
		port = ipc_port_alloc_special(ipc_space_remote);
		if (port == IP_NULL) {
			panic("receive_sright: ipc_port_alloc_special");
		}
		port->ip_nsrequest = ip_nsproxym(port);
		port->ip_norma_stransit = -1;
		port->ip_srights = 1;
		port->ip_norma_uid = uid;
		port->ip_norma_dest_node = IP_NORMA_NODE(uid);
		port->ip_norma_is_proxy = TRUE;
		norma_port_insert(port);
		return port;
	}

	/*
	 * Is it a proxy?
	 */
	if (port->ip_norma_is_proxy) {
		/*
		 * Just adjust srights and stransit.
		 */
		assert(port->ip_nsrequest != IP_NULL);
		assert(ip_nsproxyp(port->ip_nsrequest));
		assert(ip_active(port)); /* XXX How could it be otherwise? */
		port->ip_srights++;
		port->ip_norma_stransit--;
		ip_reference(port);
		return port;
	}

	/*
	 * It is a principal. Is it dead?
	 */
	if (! ip_active(port)) {
		printf1("norma_ipc_receive_sright: dead port %x\n", uid);
		/*
		 * Adjust stransit, since sender used up an stransit in
		 * sending to us. This may enable us to free the port.
		 * Consume immediately the sright associated with stransit.
		 */
		assert(port->ip_norma_stransit > 0);
		assert(port->ip_srights > 0);
		port->ip_norma_stransit--;
		port->ip_srights--;
		norma_port_remove_try(port);
		return IP_DEAD;
	}

	/*
	 * It is a living principal.
	 * Decrement stransit, since sender incremented stransit,
	 * since it could not be sure that we were still the principal.
	 * Consume sright that was associated with stransit.
	 */
	assert(port->ip_srights > 0);
	assert(port->ip_norma_stransit > 0);
	port->ip_norma_stransit--;
	return port;
}

/*
 * Find or create proxy for given uid, and add a send-once right reference.
 */
ipc_port_t
norma_ipc_receive_soright(uid)
	unsigned long uid;
{
	ipc_port_t port;

	/*
	 * Try to find the port.
	 */
	port = norma_port_lookup(uid);

	/*
	 * If we don't have a port, then we must create a proxy.
	 */
	if (port == IP_NULL) {
		assert(IP_NORMA_NODE(uid) != node_self());
		port = ipc_port_alloc_special(ipc_space_remote);
		if (port == IP_NULL) {
			panic("receive_soright: ipc_port_alloc_special");
		}
		port->ip_nsrequest = ip_nsproxym(port);
		port->ip_sorights = 1;
		port->ip_norma_uid = uid;
		port->ip_norma_dest_node = IP_NORMA_NODE(uid);
		port->ip_norma_is_proxy = TRUE;
		norma_port_insert(port);
		return port;
	}

	/*
	 * Is it a proxy?
	 */
	if (port->ip_norma_is_proxy) {
		/*
		 * Just adjust sorights.
		 * We increment, because we are inheriting a remote count.
		 */
		assert(port->ip_nsrequest != IP_NULL);
		assert(ip_nsproxyp(port->ip_nsrequest));
		assert(ip_active(port)); /* XXX How could it be otherwise? */
		port->ip_sorights++;
		ip_reference(port);
		return port;
	}

	/*
	 * It is a principal. Is it dead?
	 */
	if (! ip_active(port)) {
		/*
		 * We decrement sorights, because it has come home.
		 */
		printf1("norma_ipc_receive_soright: dead port %x\n", uid);
		port->ip_sorights--;
		port->ip_norma_sotransit--;
		assert((long) port->ip_sorights >= 0);
		assert(port->ip_norma_sotransit >= 0);
		norma_port_remove_try(port);
		return IP_DEAD;
	}

	/*
	 * It is a living principal.
	 * Leave sorights alone; we consume the one we left for remote node.
	 */
	port->ip_norma_sotransit--;
	assert((long) port->ip_sorights >= 0);
	assert(port->ip_norma_sotransit >= 0);
	ip_reference(port);
	return port;
}

/*
 * Find destination port for given uid.
 * XXX need to make safe to call from interrupt level!
 * XXX perhaps make_send/sonce should be deferred? (XXX no longer make_send)
 */
kern_return_t
norma_ipc_receive_dest(uid, type_name, remote_port)
	unsigned long uid;
	mach_msg_type_name_t type_name;
	ipc_port_t *remote_port;
{
	ipc_port_t port;

	/*
	 * This can only be some flavor of send or send-once right.
	 */
	assert(type_name == MACH_MSG_TYPE_PORT_SEND ||
	       type_name == MACH_MSG_TYPE_PORT_SEND_ONCE);

	/*
	 * Handle the special port case.
	 */
	if (IP_NORMA_SPECIAL(uid)) {
		if (IP_NORMA_NODE(uid) == node_self()) {
			*remote_port = local_special(IP_NORMA_LID(uid));
		} else {
			*remote_port = remote_special(IP_NORMA_NODE(uid),
						      IP_NORMA_LID(uid));
		}
		return KERN_SUCCESS;
	}

	/*
	 * Find associated port.
	 */
	port = norma_port_lookup_locked(uid);
	if (port == IP_NULL) {
		/*
		 * Must be an invalid uid; otherwise, we would have
		 * found something with norma_port_lookup.
		 */
		panic("norma_ipc_receive_dest: invalid uid %x!\n", uid);
		return KERN_INVALID_NAME;
	}

	/*
	 * Check to see whether we are the correct receiver for this message.
	 * If not, try to say where the correct receiver would be.
	 */
	if (port->ip_norma_is_proxy) {
		/*
		 * Tell sender to use new node.
		 */
		printf1("norma_ipc_receive_dest: migrated dest %d -> %d\n",
		       node_self(), port->ip_norma_dest_node);
		* (unsigned long *) remote_port = port->ip_norma_dest_node;
		return KERN_NOT_RECEIVER;
	}

	/*
	 * We have a local principal. Make sure it is active.
	 */
	if (! ip_active(port)) {
		/*
		 * If it is not active, it is a dead port, kept alive
		 * by remote send and send-once references.
		 */
		printf1("norma_ipc_receive_dest: dead port %x\n", uid);
		if (type_name == MACH_MSG_TYPE_PORT_SEND_ONCE) {
			assert(port->ip_sorights > 0);
			port->ip_norma_sotransit--;
			port->ip_sorights--;
			if (port->ip_sorights == 0 &&
			    port->ip_norma_stransit == 0) {
				/*
				 * The last outside send/send-once reference
				 * has been consumed; release port.
				 */
				printf1("norma_ipc_receive_dest: send_once: ");
				printf1("releasing port %x\n", port);
				norma_port_remove_locked(port);
			}
		}
		return KERN_INVALID_RIGHT;
	}
	
	/*
	 * Check queue limit.
	 */
#if 1
	/*
	 * There are locking issues here that need to be adressed.
	 * In the meantime, don't even bother looking at ip_msgcount.
	 */
#else
	if (port->ip_msgcount >= port->ip_qlimit) {
		mumble("norma_ipc_receive_dest: queue=%d >= limit=%d uid=%x\n",
		       port->ip_msgcount, port->ip_qlimit, uid);
		/*
		 * XXX
		 * Should tell sender to block, and remember to wake him up.
		 */
	}
#endif
	
	/*
	 * Return port. Simulate copyin.
	 */
	if (type_name == MACH_MSG_TYPE_PORT_SEND) {
		/*
		 * Create a send right reference.
		 */
		ip_reference(port);
		port->ip_srights++;
	} else {
		/*
		 * Consume a preexisting send-once reference,
		 * created when send-once right was sent to current sender.
		 */
		assert(type_name == MACH_MSG_TYPE_PORT_SEND_ONCE);
	}
	*remote_port = port;
	return KERN_SUCCESS;
}

norma_ipc_receive_migrating_dest(uid, type_name, remote_port)
	unsigned long uid;
	mach_msg_type_name_t type_name;
	ipc_port_t *remote_port;
{
	ipc_port_t port;

	assert(! IP_NORMA_SPECIAL(uid));

	/*
	 * Find associated port.
	 * It must be a proxy with an atrium.
	 */
	port = norma_port_lookup_locked(uid);
	if (port == IP_NULL) {
		panic("norma_ipc_receive_migrating_dest: invalid uid %x!\n",
		      uid);
		return KERN_INVALID_RIGHT;
	}
	if (! port->ip_norma_is_proxy) {
		panic("norma_ipc_receive_migrating_dest: %x not proxy!\n",
		      uid);
		return KERN_INVALID_RIGHT;
	}
	if (port->ip_norma_atrium == IP_NULL) {
		panic("norma_ipc_receive_migrating_dest: %x not migrating!\n",
		      uid);
		return KERN_INVALID_RIGHT;
	}
	assert(ip_active(port));

	/*
	 * Return port. Simulate copyin.
	 */
	if (type_name == MACH_MSG_TYPE_PORT_SEND) {
		/*
		 * Create a send right reference.
		 */
		ip_reference(port);
		port->ip_srights++;
	} else {
		/*
		 * Consume a preexisting send-once reference,
		 * created when send-once right was sent to current sender.
		 */
		assert(type_name == MACH_MSG_TYPE_PORT_SEND_ONCE);
	}
	*remote_port = port;
	return KERN_SUCCESS;
}

/*
 * Called from ipc_port_destroy.
 */
norma_ipc_port_destroy(port)
	ipc_port_t port;
{
	/*
	 * Don't bother with non-norma-affected ports.
	 */
#if 0
	if (port->ip_norma_uid == 0) {
		return;
	}
#else
	/*
	 * A port can be tabled but not have a uid
	 * if external refcounts dropped to zero.
	 */
	if (! norma_port_tabled(port)) {
		return;
	}
#endif

	/*
	 * Don't deal with proxies here. The only way that this could be
	 * a proxy is if norma_ipc_proxy_destroy called ipc_port_destroy,
	 * in which case we'll let norma_ipc_proxy_destroy deal with it.
	 *
	 * Proxies are destroyed in this module, either in send_dest
	 * or send_soright for send-once rights, or in
	 * norma_ipc_notify_no_local_senders for send rights.
	 */
	if (port->ip_norma_is_proxy) {
		return;
	}

	/*
	 * This routine cannot be called on a migrating port, since
	 * the receive right is held by the kernel.
	 * However, it's hard for us to tell if a port is migrating,
	 * and thus hard to assert anything here.
	 */

	/*
	 * If this is a special port (principal), remove it from the
	 * special port table. Note that it may be in more than one slot.
	 * Release send right reference for each slot.
	 *
	 * No-senders is never true as long as a port is in the list,
	 * since the list holds a send right for each occurance of the
	 * port in the list.
	 */
	if (port->ip_norma_is_special) {
		norma_unset_special_port(port);
		assert(port->ip_norma_is_special == FALSE);
	}

	/*
	 * Remove the port from the norma port list, but
	 * only if there are no outstanding references.
	 * If there are, we will release the port when they
	 * are all used up. We detect this when we receive
	 * a (dead) send-once right or when we receive
	 * a no_local_senders notification.
	 *
	 * If we destroyed the port immediately despite
	 * outstanding references, someone might send us
	 * a send right to it, and we would blindly create
	 * a proxy instead of realizing that the port was dead.
	 *
	 * Now of course, we should also at that point notice
	 * that IP_NORMA_NODE(uid) is node_self() and that we
	 * should have some knowledge of the port. We might be
	 * able to take advantage of this fact.
	 *
	 * Normally, we can check ip_srights intead of ip_norma_stransit.
	 * However, we are about to forcibly trash all local srights...
	 * urg. see ipc_port_destroy for ordering to see what I mean.
	 * XXX fix this comment
	 */
	norma_port_remove_try(port);
	/* XXX port->ip_norma_uid = 0; ??? used to do this if we removed it */
}

void
norma_ipc_send_no_local_senders(dest_node, uid, stransit)
	unsigned long dest_node;
	unsigned long uid;
	int stransit;
{
	kern_return_t kr;

	if (IP_NORMA_SPECIAL(uid)) {
		return;
	}
	kr = r_norma_ipc_no_local_senders(remote_host_priv(dest_node), uid,
					  stransit);
	if (kr != KERN_SUCCESS) {
		panic("norma_ipc_notify_no_senders: no_local_senders: %d/%x\n",
		      kr, kr);
	}
}

void
norma_ipc_notify_no_senders(port)
	ipc_port_t port;
{
	unsigned long dest_node;
	unsigned long uid;
	int stransit;

	assert(port->ip_nsrequest == IP_NULL);
	assert(port->ip_norma_is_proxy);
	assert(port->ip_norma_stransit < 0);
	uid = port->ip_norma_uid;
	dest_node = port->ip_norma_dest_node;
	stransit = -port->ip_norma_stransit;
	port->ip_norma_stransit = 0;
	printf1("notify no_senders(0x%x:%x) s=0 so=%d\n",
	       port, uid, port->ip_sorights);
	assert(port->ip_srights == 0);

	/*
	 * There are no local send rights...
	 * if there are also no local send-once rights, then destroy
	 * the proxy; otherwise, rearm nsrequest in case this proxy
	 * acquires send rights in the future.
	 */
	if (port->ip_sorights == 0) {
		norma_port_remove(port);
	} else {
		port->ip_nsrequest = ip_nsproxym(port);
	}
	if (IP_NORMA_SPECIAL(uid)) {
		/*
		 * Don't generate no-senders notifications for special uids.
		 * Remote node wouldn't even know what to do with one.
		 */
		printf1("norma_ipc_notify_no_senders: special port %x\n", uid);
		return;
	}
	norma_ipc_send_no_local_senders(dest_node, uid, stransit);
}

void
norma_ipc_no_local_senders(host_priv, uid, stransit)
	host_t host_priv;
	unsigned long uid;
	int stransit;
{
	ipc_port_t port;

	if (host_priv == HOST_NULL) {
		fret("norma_ipc_no_local_senders: invalid host\n");
		return;
	}
	port = norma_port_lookup(uid);
	if (port == IP_NULL) {
		if (IP_NORMA_NODE(uid) == node_self()) {
			/*
			 * XXX
			 * This can happen if this node rebooted.
			 * Otherwise, it is supposed to have saved
			 * a port for forwarding purposes.
			 */
			printf("norma_ipc_no_local_senders: failed lookup!\n");
			panic("uid %x stransit %d\n", uid, stransit);
		} else {
			fret("norma_ipc_no_local_senders: trying node %d\n",
			     IP_NORMA_NODE(uid));
			norma_ipc_send_no_local_senders(IP_NORMA_NODE(uid),
							uid, stransit);
		}
		return;
	}
	printf1("norma_ipc_no_local_senders(uid=%x port=0x%x stransit=%d)\n",
	     uid, port, stransit);
	if (port->ip_norma_is_proxy) {
		/*
		 * Instead of forwarding, we simply absorb the stransit.
		 */
		fret("norma_ipc_no_local_senders: absorb(%d) %x!\n",
		     stransit, port);
		assert(port->ip_srights > 0 || port->ip_sorights > 0);
		port->ip_norma_stransit -= stransit;
		return;
	}
	assert(stransit > 0);
	assert(port->ip_norma_stransit >= stransit);
	assert(port->ip_srights >= stransit);
	port->ip_norma_stransit -= stransit;
	port->ip_srights -= (stransit - 1);
	port->ip_references -= (stransit - 1);
	if (ip_active(port)) {
		ipc_port_release_send(port);
	} else {
		port->ip_srights--;
		port->ip_references--;
	}
	norma_port_remove_try(port);
}

/*
 * An obvious improvement here would be to somehow, in the initial
 * sending of send rights from receiver to sender, pass along some
 * initial negative stransit. The current scheme is a lose for send
 * rights that are used as capabilities, i.e., passed in the body
 * of the message instead of being used as a destination.
 */
kern_return_t
norma_ipc_stransit_wait(port)
	ipc_port_t port;
{
	kern_return_t kr;
	int stransit;
	
	kr = r_norma_ipc_stransit_request(remote_host_priv(port->
							   ip_norma_dest_node),
					  port->ip_norma_uid, &stransit);
	if (kr != KERN_SUCCESS) {
		panic("norma_ipc_stransit_wait: stransit_request: %d/%x\n",
		      kr, kr);
		return;
	}
	mumble("ip_stransit_wait: stransit += %d\n", stransit);
	port->ip_norma_stransit -= stransit;
}

kern_return_t
norma_ipc_stransit_request(host_priv, uid, stransitp)
	host_t host_priv;
	unsigned long uid;
	int *stransitp;
{
	ipc_port_t port;

	mumble("norma_ipc_stransit_request: called\n");
	if (host_priv == HOST_NULL) {
		fret("norma_ipc_stransit_request: invalid host\n");
		return KERN_INVALID_HOST;
	}
	port = norma_port_lookup(uid);
	if (port == IP_NULL) {
		fret("norma_ipc_stransit_request: failed lookup %x\n",
		     uid);
		return KERN_INVALID_NAME;
	}
	if (port->ip_norma_is_proxy) {
		/* XXX should probably forward to current principal */
		fret("norma_ipc_stransit_request: is proxy\n");
		return KERN_INVALID_RIGHT;
	}
	if (! ip_active(port)) {
		fret("norma_ipc_stransit_request: is not active\n");
		return KERN_FAILURE;
	}
	*stransitp = 10000;
	port->ip_norma_stransit += *stransitp;
	port->ip_srights += *stransitp;
	port->ip_references += *stransitp;
	mumble("norma_ipc_stransit_request: success\n");
	return KERN_SUCCESS;
}
