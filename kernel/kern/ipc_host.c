/* 
 * Mach Operating System
 * Copyright (c) 1992,1991,1990,1989,1988 Carnegie Mellon University
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
 * $Log:	ipc_host.c,v $
 * Revision 2.14  93/05/15  18:55:15  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.13  93/01/14  17:34:21  danner
 * 	Added function prototypes.  Combined ipc_processor_init and
 * 	ipc_processor_enable.  Removed ipc_processor_disable and
 * 	ipc_processor_terminate; processor ports are never removed.
 * 	Fixed lock ordering bugs in processor and processor_set code.
 * 	[92/11/17            dbg]
 * 	Added function prototypes.  Combined ipc_processor_init and
 * 	ipc_processor_enable.  Removed ipc_processor_disable and
 * 	ipc_processor_terminate; processor ports are never removed.
 * 	Fixed lock ordering bugs in processor and processor_set code.
 * 	[92/11/17            dbg]
 * 
 * Revision 2.12  92/08/03  17:37:13  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.11  92/05/21  17:13:39  jfriedl
 * 	tried prototypes.
 * 	[92/05/20            jfriedl]
 * 
 * Revision 2.10  91/08/03  18:18:50  jsb
 * 	Removed NORMA hooks.
 * 	[91/07/17  23:05:10  jsb]
 * 
 * Revision 2.9  91/06/25  10:28:20  rpd
 * 	Changed the convert_foo_to_bar functions
 * 	to use ipc_port_t instead of mach_port_t.
 * 	[91/05/27            rpd]
 * 
 * Revision 2.8  91/06/17  15:46:59  jsb
 * 	Renamed NORMA conditionals.
 * 	[91/06/17  10:49:34  jsb]
 * 
 * Revision 2.7  91/06/06  17:06:59  jsb
 * 	Redid host port initialization under NORMA_IPC.
 * 	[91/05/13  17:37:21  jsb]
 * 
 * Revision 2.6  91/05/14  16:41:36  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:26:24  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:12:32  mrt]
 * 
 * Revision 2.4  90/09/09  14:32:08  rpd
 * 	Don't take out extra references in ipc_pset_init.
 * 	[90/08/30            rpd]
 * 
 * Revision 2.3  90/06/19  22:58:46  rpd
 * 	Changed convert_port_to_pset_name to allow
 * 	both IKOT_PSET and IKOT_PSET_NAME ports.
 * 	Changed convert_port_to_host to allow
 * 	both IKOT_HOST and IKOT_HOST_PRIV ports.
 * 	[90/06/18            rpd]
 * 
 * 	Fixed bug in convert_processor_to_port.
 * 	[90/06/16            rpd]
 * 
 * Revision 2.2  90/06/02  14:53:59  rpd
 * 	Created for new IPC.
 * 	[90/03/26  23:46:28  rpd]
 * 
 * 	Move includes.
 * 	[89/08/02            dlb]
 * 	Remove interrupt protection from pset locks.
 * 	[89/06/14            dlb]
 * 	Use port_alloc instead of xxx_port_allocate.
 * 	[89/02/21            dlb]
 * 	Reformat includes.
 * 	[89/01/26            dlb]
 * 
 * 	Break processor_set_default into two pieces.
 * 	[88/12/21            dlb]
 * 
 * 	Move host_self, host_priv_self to ipc_ptraps.c
 * 	Rewrite processor_set_default to return both ports
 * 	[88/11/30            dlb]
 * 
 * 	Created.
 * 	[88/10/29            dlb]
 * 
 * Revision 2.4  89/12/22  15:52:20  rpd
 * 	Take out extra reference on new processor set ports in
 * 	ipc_pset_init for reply message; these ports are now
 * 	returned untranslated.  Assume caller of ipc_pset_disable
 * 	has pset locked as well as referenced.
 * 	[89/12/15            dlb]
 * 
 * Revision 2.3  89/10/15  02:04:29  rpd
 * 	Minor cleanups.
 * 
 * Revision 2.2  89/10/11  14:07:11  dlb
 * 	Fix includes.
 * 	Remove interrupt protection from pset locks.
 * 
 */

/*
 *	kern/ipc_host.c
 *
 *	Routines to implement host ports.
 */

#include <mach/message.h>
#include <kern/host.h>
#include <kern/processor.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <kern/ipc_host.h>
#include <kern/ipc_kobject.h>
#include <ipc/ipc_port.h>
#include <ipc/ipc_space.h>

#include <machine/machspl.h>	/* for spl */



/*
 *	ipc_host_init: set up various things.
 */

void ipc_host_init(void)
{
	ipc_port_t	port;
	/*
	 *	Allocate and set up the two host ports.
	 */
	port = ipc_port_alloc_kernel();
	if (port == IP_NULL)
		panic("ipc_host_init");

	ipc_kobject_set(port, (ipc_kobject_t) &realhost, IKOT_HOST);
	realhost.host_self = port;

	port = ipc_port_alloc_kernel();
	if (port == IP_NULL)
		panic("ipc_host_init");

	ipc_kobject_set(port, (ipc_kobject_t) &realhost, IKOT_HOST_PRIV);
	realhost.host_priv_self = port;

	/*
	 *	Set up ipc for default processor set.
	 */
	ipc_pset_init(&default_pset);
	ipc_pset_enable(&default_pset);

	/*
	 *	And for master processor
	 */
	ipc_processor_init(master_processor);
}

/*
 *	Routine:	mach_host_self [mach trap]
 *	Purpose:
 *		Give the caller send rights for his own host port.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_PORT_NULL if there are any resource failures
 *		or other errors.
 */

mach_port_t
mach_host_self(void)
{
	ipc_port_t sright;

	sright = ipc_port_make_send(realhost.host_self);
	return ipc_port_copyout_send(sright, current_space());
}

#if	MACH_IPC_COMPAT

/*
 *	Routine:	host_self [mach trap]
 *	Purpose:
 *		Give the caller send rights for his own host port.
 *		If new, the send right is marked with IE_BITS_COMPAT.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_PORT_NULL if there are any resource failures
 *		or other errors.
 */

port_name_t
host_self(void)
{
	ipc_port_t sright;

	sright = ipc_port_make_send(realhost.host_self);
	return (port_name_t)
		ipc_port_copyout_send_compat(sright, current_space());
}

#endif	MACH_IPC_COMPAT

/*
 *	ipc_processor_init:
 *
 *	Initialize ipc access to processor by allocating port.
 *	Enable ipc control of processor by setting port object.
 */

void
ipc_processor_init(
	processor_t	processor)
{
	ipc_port_t	port;

	port = ipc_port_alloc_kernel();
	if (port == IP_NULL)
		panic("ipc_processor_init");
	processor->processor_self = port;
	ipc_kobject_set(port, (ipc_kobject_t) processor, IKOT_PROCESSOR);
}


/*
 *	ipc_pset_init:
 *
 *	Initialize ipc control of a processor set by allocating its ports.
 */

void
ipc_pset_init(
	processor_set_t	pset)
{
	ipc_port_t	port;

	port = ipc_port_alloc_kernel();
	if (port == IP_NULL)
		panic("ipc_pset_init");
	pset->pset_self = port;

	port = ipc_port_alloc_kernel();
	if (port == IP_NULL)
		panic("ipc_pset_init");
	pset->pset_name_self = port;
}

/*
 *	ipc_pset_enable:
 *
 *	Enable ipc access to a processor set.
 */
void
ipc_pset_enable(
	processor_set_t	pset)
{
	pset_lock(pset);
	if (pset->active) {
		ipc_kobject_set(pset->pset_self,
				(ipc_kobject_t) pset, IKOT_PSET);
		ipc_kobject_set(pset->pset_name_self,
				(ipc_kobject_t) pset, IKOT_PSET_NAME);
		pset_ref_lock(pset);
		pset->ref_count += 2;
		pset_ref_unlock(pset);
	}
	pset_unlock(pset);
}

/*
 *	ipc_pset_disable:
 *
 *	Disable ipc access to a processor set by clearing the port objects.
 *	Caller must hold pset lock and a reference to the pset.  Ok to
 *	just decrement pset reference count as a result.
 */
void
ipc_pset_disable(
	processor_set_t	pset)
{
	ipc_kobject_set(pset->pset_self, IKO_NULL, IKOT_NONE);
	ipc_kobject_set(pset->pset_name_self, IKO_NULL, IKOT_NONE);

	pset_ref_lock(pset);
	pset->ref_count -= 2;
	pset_ref_unlock(pset);
}

/*
 *	ipc_pset_terminate:
 *
 *	Processor set is dead.  Deallocate the ipc control structures.
 */
void
ipc_pset_terminate(
	processor_set_t	pset)
{
	ipc_port_dealloc_kernel(pset->pset_self);
	ipc_port_dealloc_kernel(pset->pset_name_self);
}

/*
 *	processor_set_default, processor_set_default_priv:
 *
 *	Return ports for manipulating default_processor set.  MiG code
 *	differentiates between these two routines.
 */
kern_return_t
processor_set_default(
	host_t		host,
	processor_set_t	*pset)
{
	if (host == HOST_NULL)
		return KERN_INVALID_ARGUMENT;

	*pset = &default_pset;
	pset_reference(*pset);
	return KERN_SUCCESS;
}

kern_return_t
xxx_processor_set_default_priv(
	host_t		host,
	processor_set_t	*pset)
{
	if (host == HOST_NULL)
		return KERN_INVALID_ARGUMENT;

	*pset = &default_pset;
	pset_reference(*pset);
	return KERN_SUCCESS;
}

/*
 *	Routine:	convert_port_to_host
 *	Purpose:
 *		Convert from a port to a host.
 *		Doesn't consume the port ref; the host produced may be null.
 *	Conditions:
 *		Nothing locked.
 */

host_t
convert_port_to_host(ipc_port_t port)
{
	host_t host = HOST_NULL;

	if (IP_VALID(port)) {
		ip_lock(port);
		if (ip_active(port) &&
		    ((ip_kotype(port) == IKOT_HOST) ||
		     (ip_kotype(port) == IKOT_HOST_PRIV)))
			host = (host_t) port->ip_kobject;
		ip_unlock(port);
	}

	return host;
}

/*
 *	Routine:	convert_port_to_host_priv
 *	Purpose:
 *		Convert from a port to a host.
 *		Doesn't consume the port ref; the host produced may be null.
 *	Conditions:
 *		Nothing locked.
 */

host_t
convert_port_to_host_priv(ipc_port_t port)
{
	host_t host = HOST_NULL;

	if (IP_VALID(port)) {
		ip_lock(port);
		if (ip_active(port) &&
		    (ip_kotype(port) == IKOT_HOST_PRIV))
			host = (host_t) port->ip_kobject;
		ip_unlock(port);
	}

	return host;
}

/*
 *	Routine:	convert_port_to_processor
 *	Purpose:
 *		Convert from a port to a processor.
 *		Doesn't consume the port ref;
 *		the processor produced may be null.
 *	Conditions:
 *		Nothing locked.
 */

processor_t
convert_port_to_processor(ipc_port_t port)
{
	processor_t processor = PROCESSOR_NULL;

	if (IP_VALID(port)) {
		ip_lock(port);
		if (ip_active(port) &&
		    (ip_kotype(port) == IKOT_PROCESSOR))
			processor = (processor_t) port->ip_kobject;
		ip_unlock(port);
	}

	return processor;
}

/*
 *	Routine:	convert_port_to_pset
 *	Purpose:
 *		Convert from a port to a pset.
 *		Doesn't consume the port ref; produces a pset ref,
 *		which may be null.
 *	Conditions:
 *		Nothing locked.
 */

processor_set_t
convert_port_to_pset(ipc_port_t port)
{
	processor_set_t pset = PROCESSOR_SET_NULL;

	if (IP_VALID(port)) {
		ip_lock(port);
		if (ip_active(port) &&
		    (ip_kotype(port) == IKOT_PSET)) {
			pset = (processor_set_t) port->ip_kobject;
			pset_reference(pset);
		}
		ip_unlock(port);
	}

	return pset;
}

/*
 *	Routine:	convert_port_to_pset_name
 *	Purpose:
 *		Convert from a port to a pset.
 *		Doesn't consume the port ref; produces a pset ref,
 *		which may be null.
 *	Conditions:
 *		Nothing locked.
 */

processor_set_t
convert_port_to_pset_name(ipc_port_t port)
{
	processor_set_t pset = PROCESSOR_SET_NULL;

	if (IP_VALID(port)) {
		ip_lock(port);
		if (ip_active(port) &&
		    ((ip_kotype(port) == IKOT_PSET) ||
		     (ip_kotype(port) == IKOT_PSET_NAME))) {
			pset = (processor_set_t) port->ip_kobject;
			pset_reference(pset);
		}
		ip_unlock(port);
	}

	return pset;
}

/*
 *	Routine:	convert_host_to_port
 *	Purpose:
 *		Convert from a host to a port.
 *		Produces a naked send right which is always valid.
 *	Conditions:
 *		Nothing locked.
 */

ipc_port_t
convert_host_to_port(host_t host)
{
	ipc_port_t port;

	port = ipc_port_make_send(host->host_self);

	return port;
}

/*
 *	Routine:	convert_processor_to_port
 *	Purpose:
 *		Convert from a processor to a port.
 *		Produces a naked send right which is always valid.
 *	Conditions:
 *		Nothing locked.
 */

ipc_port_t
convert_processor_to_port(processor_t processor)
{
	ipc_port_t port;

	port = ipc_port_make_send(processor->processor_self);

	return port;
}

/*
 *	Routine:	convert_pset_to_port
 *	Purpose:
 *		Convert from a pset to a port.
 *		Consumes a pset ref; produces a naked send right
 *		which may be invalid.
 *	Conditions:
 *		Nothing locked.
 */

ipc_port_t
convert_pset_to_port(processor_set_t pset)
{
	ipc_port_t port;

	pset_lock(pset);
	if (pset->active)
		port = ipc_port_make_send(pset->pset_self);
	else
		port = IP_NULL;
	pset_unlock(pset);

	pset_deallocate(pset);
	return port;
}

/*
 *	Routine:	convert_pset_name_to_port
 *	Purpose:
 *		Convert from a pset to a port.
 *		Consumes a pset ref; produces a naked send right
 *		which may be invalid.
 *	Conditions:
 *		Nothing locked.
 */

ipc_port_t
convert_pset_name_to_port(processor_set_t pset)
{
	ipc_port_t port;

	pset_lock(pset);
	if (pset->active)
		port = ipc_port_make_send(pset->pset_name_self);
	else
		port = IP_NULL;
	pset_unlock(pset);

	pset_deallocate(pset);
	return port;
}
