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
 * $Log:	ipc_deadname.c,v $
 * Revision 2.2  93/01/14  17:53:31  danner
 * 	64bit cleanup.
 * 	[92/12/01            af]
 * 
 * 	Created.
 * 	[92/05/25            dlb]
 * 
 * Revision 2.1  92/05/27  01:00:30  jeffreyh
 * Created.
 * 
 */

#include <ipc/ipc_port.h>
#include <mach/kern_return.h>
#include <norma/ipc_node.h>
#include <kern/host.h>

extern ipc_port_t norma_port_lookup();
/*
 *	File:	norma/ipc_dnrequest.c
 *	Author:	David L. Black
 *	Date:	1992
 *
 *	Functions for managing NORMA IPC dead name requests.
 */

/*
 *	Dead name notifications have a two-level structure: NORMA
 *	code tracks which nodes have to be notified, and the proxy
 *	structures on those nodes track the actual notifications.
 *	The existence of a dead name notification causes destruction
 *	of the corresponding proxies to be eagerly evaluated.  A special
 *	value for the notify port is used to redirect dead name
 *	notifications for remote proxies into the norma system.
 */

/*
 *	norma_ipc_dnrequest_init:
 *
 *	Called by the ipc system the first time a dead name request
 *	is queued for a port.  If the port is a proxy, ask the principal
 *	to tell us when it dies.
 */

void
norma_ipc_dnrequest_init(port)
ipc_port_t	port;
{
	kern_return_t result;

	if (!IP_NORMA_IS_PROXY(port))
		return;

	result = r_norma_ipc_node_dnrequest(
		remote_host_priv(port->ip_norma_dest_node),
		port->ip_norma_uid, node_self());

	if (result != KERN_SUCCESS) {
		/*
		 *	Principal is already dead, so kill this proxy.
		 *	Caller will discover port is dead when it
		 *	rechecks after return from ipc_port_dngrow();
		 */
		norma_ipc_dead_destination(port);
	}
}

/*
 *	norma_ipc_node_dnrequest:
 *
 *	Called on principal's node by a proxy that wants to be
 *	notified when the receive right dies (so that it can process
 *	its dead name requests).  Queue a norma fake dead name notification
 *	so that we get called at that time.
 */

kern_return_t
norma_ipc_node_dnrequest(host_priv, uid, node)
	host_t		host_priv;
	natural_t	uid;
	natural_t	node;
{
	ipc_port_t	port;
	ipc_port_request_index_t	junk_request;
	kern_return_t			kr;

	if (host_priv == HOST_NULL)
		return(KERN_INVALID_ARGUMENT);

	port = norma_port_lookup(uid);

        if ((port == IP_NULL) || !ip_active(port)) {
		/*
		 *	The receive right is already dead.
		 */
	        printf("Stale dn request node %d uid %d\n");
		r_norma_ipc_node_dnnotify(remote_host_priv(node), uid);
		return(KERN_SUCCESS);
	}

        if (port->ip_norma_is_proxy) {
                /*
                 * Send this along to the principal.  This is an
		 * asynchronous message.
                 */
		r_norma_ipc_node_dnrequest(
				remote_host_priv(port->ip_norma_dest_node),
                                uid, node);
		return(KERN_SUCCESS);
	}

	/*
	 *	Place norma dnrequest.
	 */
        kr = ipc_port_dnrequest(port, NODE_TO_DNREQUEST_NAME(node),
				IP_NORMA_FAKE_DNREQUEST,
				&junk_request);
	while (kr != KERN_SUCCESS) {
		kr = ipc_port_dngrow(port);
                if (kr != KERN_SUCCESS)
		    panic("norma_ipc_node_dnrequest: ipc_port_dngrow failure");

	        kr = ipc_port_dnrequest(port, NODE_TO_DNREQUEST_NAME(node),
					IP_NORMA_FAKE_DNREQUEST,
					&junk_request);
	}

	return(KERN_SUCCESS);
}

/*
 *	norma_ipc_notify_dead_name:
 *
 *	The receive right for a port has died and the proxy on some node
 *	wants to know about it.  Tell it.
 *
 *	NOTE: This routine depends on norma_ipc_port_destroy NOT clobbering
 *		the port's uid.
 */

void	
norma_ipc_notify_dead_name(port, name)
	ipc_port_t	port;
	natural_t	name;
{
	assert(!IP_NORMA_IS_PROXY(port));

	r_norma_ipc_node_dnnotify(
		remote_host_priv(DNREQUEST_NAME_TO_NODE(name)),
		port->ip_norma_uid);
}

/*
 *	norma_ipc_node_dnnotify:
 *
 *	Called on proxy node when principal has died.  Kill proxy to
 *	trigger its dead name notifications.  Proxy may already be
 *	gone for other reasons, so just ignore this if we can't find one.
 */

void
norma_ipc_node_dnnotify(host_priv, uid)
host_t		host_priv;
natural_t	uid;
{
	ipc_port_t port;

	port = norma_port_lookup(uid);

	assert(port == IP_NULL || IP_NORMA_IS_PROXY(port));

	/*
	 *	Ignore this if we can't find the proxy.  If the
	 *	port is queued, the send will fail and destroy the
	 *	proxy anyway; destroying it prematurely will just
	 *	confuse the send logic.
	 */
	if ((port != IP_NULL) && !netipc_port_is_queued(port))
		norma_ipc_dead_destination(port);
}
