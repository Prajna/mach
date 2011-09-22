/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 * $Log:	ipc_special.c,v $
 * Revision 2.4  92/03/10  16:28:17  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:50:20  jsb]
 * 
 * Revision 2.3.2.4  92/02/18  19:16:34  jeffreyh
 * 	Use IP_NORMA_IS_PROXY macro instead of checking for norma_uid
 * 	in norma_port_location_hint.  The latter gives the wrong
 * 	answer if the port has been exported and we're not node 0.
 * 	[92/02/14            dlb]
 * 
 * Revision 2.3.2.3  92/01/21  21:52:33  jsb
 * 	More de-linting.
 * 	[92/01/17  11:41:55  jsb]
 * 
 * 	De-linted.
 * 	[92/01/16  22:14:38  jsb]
 * 
 * Revision 2.3.2.2  92/01/09  18:45:51  jsb
 * 	Removed placeholder logic.
 * 	Added remote_{special,device,host,host_priv} routines.
 * 	[92/01/04  18:35:52  jsb]
 * 
 * Revision 2.3.2.1  92/01/03  16:37:58  jsb
 * 	Corrected log. Removed norma_token_startup call. Fixed type clash.
 * 	[91/12/24  14:26:15  jsb]
 * 
 * Revision 2.2  91/11/14  16:48:12  rpd
 * 	Moved from norma/ipc_xxx.c.
 * 
 * Revision 2.5  91/08/28  11:16:15  jsb
 * 	Renamed clport things to norma things.
 * 	Eliminated NORMA_ETHER special case node/lid uid division.
 * 	[91/08/15  09:14:06  jsb]
 * 
 * 	Renamed ipc_clport_foo things to norma_ipc_foo.
 * 	Added host_port_at_node().
 * 	[91/08/14  19:21:16  jsb]
 * 
 * Revision 2.4  91/08/03  18:19:34  jsb
 * 	Conditionalized printfs.
 * 	[91/08/01  22:55:17  jsb]
 * 
 * 	Eliminated dynamically allocated clport structures; this information
 * 	is now stored in ports directly. This simplifies issues of allocation
 * 	at interrupt time.
 * 
 * 	Added definitions for IP_CLPORT_{NODE,LID} appropriate for using the
 * 	low two bytes of an internet address as a node number.
 * 
 * 	Revised norma_special_port mechanism to better handle port death
 * 	and port replacement. Eliminated redundant special port calls.
 * 	Cleaned up associated startup code.
 * 	[91/07/25  19:09:16  jsb]
 * 
 * Revision 2.3  91/07/01  08:25:52  jsb
 * 	Some locking changes.
 * 	[91/06/29  15:10:46  jsb]
 * 
 * Revision 2.2  91/06/17  15:48:05  jsb
 * 	Moved here from ipc/ipc_clport.c.
 * 	[91/06/17  11:07:25  jsb]
 * 
 * Revision 2.7  91/06/06  17:05:35  jsb
 * 	Added norma_get_special_port, norma_set_special_port.
 * 	[91/05/25  10:28:14  jsb]
 * 
 * 	Much code moved to other norma/ files.
 * 	[91/05/13  17:16:24  jsb]
 * 
 */
/*
 *	File:	norma/ipc_special.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1990
 *
 *	Functions to support norma special ports.
 */

#include <norma_vm.h>
#include <norma_ether.h>

#include <vm/vm_kern.h>
#include <mach/vm_param.h>
#include <mach/port.h>
#include <mach/message.h>
#include <mach/norma_special_ports.h>
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

extern void netipc_thread();
extern ipc_port_t norma_port_lookup();
extern void netipc_thread_lock();
extern void netipc_thread_unlock();
extern void norma_port_insert();

ipc_port_t host_special_port_array[MAX_SPECIAL_ID];
#define host_special_port(id) (host_special_port_array[id])

/*
 * The first three special ports are kernel owned.
 * They are DEVICE, HOST, and HOST_PRIV.
 * The kernel has receive rights to these ports.
 * Users are not allowed to change these ports.
 * No-more senders notifications are not requested for these ports.
 */

norma_ipc_init()
{
	/*
	 * Register master device, host, and host_priv ports.
	 */
	assert(master_device_port != IP_NULL);
	master_device_port->ip_norma_is_special = TRUE;
	host_special_port(NORMA_DEVICE_PORT) = master_device_port;

	assert(realhost.host_self != IP_NULL);
	realhost.host_self->ip_norma_is_special = TRUE;
	host_special_port(NORMA_HOST_PORT) = realhost.host_self;

	assert(realhost.host_priv_self != IP_NULL);
	realhost.host_priv_self->ip_norma_is_special = TRUE;
	host_special_port(NORMA_HOST_PRIV_PORT) = realhost.host_priv_self;

	/*
	 * Initialize network subsystem
	 */
	netipc_init();

	/*
	 * Start up netipc thread, kserver module, and token module.
	 * XXX netipc_init should do the former...
	 */
	(void) kernel_thread(kernel_task, netipc_thread, (char *) 0);
	norma_kserver_startup();
}

/*
 * XXX should probably eliminate this -- it's a hack used by the xmm system
 */
ipc_port_node(port)
	ipc_port_t port;
{
	if (!port) return node_self();
	if (!port->ip_norma_uid) return node_self();
	return IP_NORMA_NODE(port->ip_norma_uid);
}

kern_return_t
norma_port_location_hint(task, port, node)
	task_t task;
	ipc_port_t port;
	int *node;
{
	if (port == IP_NULL) {
		return KERN_INVALID_ARGUMENT;
	}
	if (IP_NORMA_IS_PROXY(port)) {
		*node = port->ip_norma_dest_node;
	} else {
		*node = node_self();
	}
	return KERN_SUCCESS;
}

kern_return_t
norma_set_special_port(host, id, port)
	host_t host;
	unsigned long id;
	ipc_port_t port;
{
	ipc_port_t old;

	if (host == HOST_NULL) {
		return KERN_INVALID_HOST;
	}
	if (id <= 0 || id > MAX_SPECIAL_ID) {
		return KERN_INVALID_VALUE;
	}
	if (id <= MAX_SPECIAL_KERNEL_ID) {
		/* these never change */
		return KERN_INVALID_VALUE;
	}
	netipc_thread_lock();
	old = host_special_port(id);
	host_special_port(id) = port;
	port->ip_norma_is_special = TRUE;
	if (IP_VALID(old)) {
		fret("special id #%d replaced\n", id);
		ipc_port_release_send(old);
	}
	netipc_thread_unlock();
	return KERN_SUCCESS;
}

/*
 * Internally called when port is being destroyed.
 * XXX
 * Is it also called when proxy is destroyed???
 */
norma_unset_special_port(port)
	ipc_port_t port;
{
	unsigned long id;

	for (id = 1; id <= MAX_SPECIAL_ID; id++) {
		if (host_special_port(id) == port) {
			assert(id >= MAX_SPECIAL_KERNEL_ID);
			fret("special id #%d died\n", id);
			host_special_port(id) = IP_NULL;
			ipc_port_release_send(port);
		}
	}
	port->ip_norma_is_special = FALSE;
}

ipc_port_t
local_special(id)
	unsigned long id;
{
	assert(IP_NORMA_SPECIAL(id));
	assert(id > 0 && id <= MAX_SPECIAL_KERNEL_ID);
	return ipc_port_make_send(host_special_port(id));
}

ipc_port_t
remote_special(node, id)
	unsigned long node;
	unsigned long id;
{
	ipc_port_t port;
	unsigned long uid = IP_NORMA_UID(node, id);

	assert(node != node_self());
	assert(id > 0 && id <= MAX_SPECIAL_KERNEL_ID);
	port = norma_port_lookup(uid);
	if (port == IP_NULL) {
		port = ipc_port_alloc_special(ipc_space_remote);
		if (port == IP_NULL) {
			panic("remote_special: ipc_port_alloc_special");
		}
		port->ip_nsrequest = ip_nsproxym(port);
		port->ip_srights = 1;
		port->ip_norma_uid = uid;
		port->ip_norma_dest_node = node;
		port->ip_norma_is_proxy = TRUE;
		norma_port_insert(port);
	} else {
		port->ip_srights++;
	}
	port->ip_norma_stransit = -0x70000000;	/* XXX */
	return port;
}

ipc_port_t
remote_device(node)
	unsigned long node;
{
	return remote_special(node, (unsigned long) NORMA_DEVICE_PORT);
}

ipc_port_t
remote_host(node)
	unsigned long node;
{
	return remote_special(node, (unsigned long) NORMA_HOST_PORT);
}

ipc_port_t
remote_host_priv(node)
	unsigned long node;
{
	return remote_special(node, (unsigned long) NORMA_HOST_PRIV_PORT);
}

/*
 * XXX
 * The blocking behavior of this call is now somewhat funny.
 * A remote call will block until the remote node comes up.
 * However, in either local or remote case, a null port may be returned;
 * that is, we do not block for a valid port to show up.
 */
kern_return_t
norma_get_special_port(host, node, id, portp)
	host_t host;
	unsigned long node;
	unsigned long id;
	ipc_port_t *portp;
{
	if (host == HOST_NULL) {
		return KERN_INVALID_HOST;
	}
	if (id <= 0 || id > MAX_SPECIAL_ID) {
		return KERN_INVALID_ARGUMENT;
	}
	if (id <= MAX_SPECIAL_KERNEL_ID) {
		if (node == node_self()) {
			*portp = local_special(id);
		} else {
			*portp = remote_special(node, id);
		}
	} else {
		if (node == node_self()) {
			*portp = ipc_port_copy_send(host_special_port(id));
		} else {
			return r_norma_get_special_port(remote_host_priv(node),
							node, id, portp);
		}
	}
	return KERN_SUCCESS;
}
