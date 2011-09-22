/* 
 * Mach Operating System
 * Copyright (c) 1993,1991,1990,1989 Carnegie Mellon University
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
 * $Log:	ds_routines.c,v $
 * Revision 2.35  93/08/10  15:10:38  mrt
 * 	Added support for device write kernel traps.
 * 	[93/06/11            cmaeda]
 * 
 * Revision 2.34  93/05/15  18:53:00  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.33  93/01/14  17:27:00  danner
 * 	64bit cleanup.
 * 	[92/11/30            af]
 * 
 * 	Return D_INVALID_SIZE instead of panicking in write routines when
 * 	there is no data. This is the case when device_write() is given
 * 	zero as the data length.
 * 	[92/10/08            jvh]
 * 
 * Revision 2.32  92/08/03  17:33:33  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.31  92/05/21  17:09:22  jfriedl
 * 	Cleanup to quiet gcc warnings.
 * 	[92/05/20            jfriedl]
 * 
 * Revision 2.30  92/04/01  19:31:44  rpd
 * 	Calculate minimum transfer size in device_write_get and
 * 	pass it to kmem_io_map_copyout.  This makes large block
 * 	sizes work with tapes.
 * 
 * 	Increase DEVICE_IO_MAP_SIZE to 2 megs
 * 	[92/03/03  13:53:25  jeffreyh]
 * 
 * Revision 2.29  92/03/10  16:25:21  jsb
 * 	Check protection argument to device_map.
 * 	[92/02/22  17:03:11  dlb]
 *
 * 	Merged in norma branch changes as of NORMA_MK7:
 * 	Added ds_notify and ds_no_senders routines in preparation for using
 * 	no-senders notifications to close unreferenced devices.
 * 	Use remote_device() instead of norma_get_special_port().
 * 
 * Revision 2.28  92/03/05  18:54:14  rpd
 * 	Undid norma changes that were inadvertently picked up in 2.27.
 * 	[92/03/05            rpd]
 * 
 * Revision 2.27  92/02/23  19:49:11  elf
 * 	Change panic message to match previous change.
 * 	[92/02/21  10:11:55  dlb]
 * 
 * 	Use page lists for device reads going to default pager.
 * 	[92/02/20  15:16:19  dlb]
 * 
 * 	Temporary change to return data read from devices as a page
 * 	list instead of an entry list.  The keep_wired logic has
 * 	to be updated to convert the default pager to this mechanism
 * 	when making this change permanent.
 * 	[92/02/19  17:36:50  dlb]
 * 
 * Revision 2.26  91/12/10  13:25:33  jsb
 * 	Change ds_read_done to call vm_map_copyin_page_list directly
 * 	if destination of reply is remote.
 * 
 * Revision 2.25  91/11/14  16:52:41  rpd
 * 	Replaced master_device_port_at_node call with
 *	call to norma_get_special_port.
 * 	[91/11/00  00:00:00  jsb]
 * 
 * Revision 2.24  91/10/09  16:05:37  af
 * 	Fixed device_write_get to check kmem_io_map_copyout return code.
 * 	Enabled wait_for_space in device_io_map.
 * 	[91/09/17            rpd]
 * 
 * Revision 2.23  91/09/12  16:37:22  bohman
 * 	Changed device_write_inband() to not require a reply port.
 * 	Fixed device_write_get() to allow inband calls.  In this case,
 * 	an io_inband buffer is allocated and the data is copied into it.
 * 	Fixed device_write_dealloc() to correctly deallocate io_inband
 * 	buffers.
 * 	Fixed ds_read_done() to free io_inband buffers only if one was
 * 	actually allocated.
 * 	[91/09/11  17:06:50  bohman]
 * 
 * Revision 2.22  91/08/28  11:11:16  jsb
 * 	From rpd: increased DEVICE_IO_MAP_SIZE; documented why
 * 	device_write_get cannot be used for in-band data.
 * 	[91/08/22  15:28:19  jsb]
 * 
 * 	In device_write_get, always set the wait parameter to something;
 * 	by default, it's FALSE.
 * 	[91/08/16  14:19:31  jsb]
 * 
 * 	Support synchronous wait by writers when vm continuations are present.
 * 	Optimize device_write_dealloc.  Fix MP bug in iowait/iodone.
 * 	Convert from bsize to dev_info entry in device op vector.
 * 	[91/08/12  17:27:15  dlb]
 * 
 * 	Page lists working reliably: delete old code.
 * 	[91/08/06  17:16:09  dlb]
 * 
 * 	Clean up and add continuation support for device_write page lists.
 * 	[91/08/05  17:30:38  dlb]
 * 
 * 	First version of support for vm page lists in device_write.
 * 	Still needs cleanup and continuation support.  Old code left
 * 	under #ifdef 0.  
 * 	[91/07/31  14:42:24  dlb]
 * 
 * Revision 2.21  91/08/24  11:55:43  af
 * 	Spls definitions.
 * 	[91/08/02  02:44:45  af]
 * 
 * Revision 2.20  91/08/03  18:17:33  jsb
 * 	Device_write_get doesn't need to do anything for loaned ior's.
 * 	[91/08/02  12:13:15  jsb]
 * 
 * 	Create the right flavor of copy object in ds_read_done.
 * 	Replace NORMA_BOOT conditionals with NORMA_DEVICE.
 * 	Free loaned ior's directly in iodone().
 * 	[91/07/27  22:45:09  jsb]
 * 
 * Revision 2.19  91/06/25  10:26:57  rpd
 * 	Changed mach_port_t to ipc_port_t where appropriate.
 * 	Removed device_reply_search and device_reply_terminate.
 * 	[91/05/28            rpd]
 * 
 * Revision 2.18  91/06/17  15:43:58  jsb
 * 	Renamed NORMA conditionals.
 * 	[91/06/17  09:58:59  jsb]
 * 
 * Revision 2.17  91/05/18  14:29:52  rpd
 * 	Added vm/memory_object.h.
 * 	[91/03/22            rpd]
 * 
 * Revision 2.16  91/05/14  15:47:34  mrt
 * 	Correcting copyright
 * 
 * Revision 2.15  91/03/16  14:43:02  rpd
 * 	Updated for new kmem_alloc interface.
 * 	[91/03/03            rpd]
 * 	Added io_done_thread_continue.
 * 	[91/02/13            rpd]
 * 	Removed thread_swappable.
 * 	[91/01/18            rpd]
 * 
 * Revision 2.14  91/02/05  17:09:25  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:28:57  mrt]
 * 
 * Revision 2.13  91/01/08  15:09:38  rpd
 * 	Added continuation argument to thread_block.
 * 	[90/12/08            rpd]
 * 
 * Revision 2.12  90/12/14  10:59:39  jsb
 * 	Moved mechanism for mapping global to local device names
 * 	to the machine-dependent function dev_forward_name.
 * 	[90/12/14  09:37:18  jsb]
 * 
 * 	Added device request forwarding to support inter-node device access.
 * 	[90/12/14  08:30:53  jsb]
 * 
 * Revision 2.11  90/10/25  14:44:32  rwd
 * 	Let ds_device_write proceed w/o a valid reply port.  This is used
 * 	by the unix server ether_output routine.
 * 	[90/10/22            rwd]
 * 	Fixed ds_write_done to use ds_device_write_inband_reply
 * 	when appropriate.
 * 	[90/10/18            rpd]
 * 	Check for invalid reply ports.
 * 	[90/10/17            rwd]
 * 
 * Revision 2.10  90/09/09  14:31:27  rpd
 * 	Use decl_simple_lock_data.
 * 	[90/08/30            rpd]
 * 
 * Revision 2.9  90/06/02  14:48:00  rpd
 * 	Cleaned up check for default pager in ds_read_done.
 * 	[90/04/29            rpd]
 * 
 * 	Fixed ds_read_done to leave memory wired if the read reply
 * 	is being sent to the default pager.
 * 	[90/04/05            rpd]
 * 	Converted to new IPC.  Purged MACH_XP_FPD.
 * 	[90/03/26  21:55:28  rpd]
 * 
 * Revision 2.8  90/02/22  20:02:12  dbg
 * 	Use vm_map_copy routines.
 * 	[90/01/25            dbg]
 * 
 * Revision 2.7  90/01/11  11:42:01  dbg
 * 	De-lint.
 * 	[89/12/06            dbg]
 * 
 * Revision 2.6  89/11/29  14:08:54  af
 * 	iodone() should set the IO_DONE flag.
 * 	[89/11/03  16:58:16  af]
 * 
 * Revision 2.5  89/11/14  10:28:19  dbg
 * 	Make read and write handle zero-length transfers correctly (used
 * 	to implement select).
 * 	[89/10/27            dbg]
 * 
 * Revision 2.4  89/09/08  11:24:17  dbg
 * 	Converted to run in kernel context.
 * 	Add list of wired pages to tail of IOR allocated for write.
 * 	Reorganized file: moved open/close to beginning, map to end.
 * 	[89/08/23            dbg]
 * 
 * Revision 2.3  89/08/31  16:18:46  rwd
 * 	Added ds_read_inband and support
 * 	[89/08/15            rwd]
 * 
 * Revision 2.2  89/08/05  16:06:39  rwd
 * 	Added ds_write_inband for use by tty and ds_device_map_device.
 * 	[89/07/17            rwd]
 * 
 * 12-Apr-89  David Golub (dbg) at Carnegie-Mellon University
 *	Added device_reply_terminate.
 *
 *  3-Mar-89  David Golub (dbg) at Carnegie-Mellon University
 *	Created.
 *
 */
/*
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date: 	3/89
 */

#include <norma_device.h>

#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/mig_errors.h>
#include <mach/port.h>
#include <mach/vm_param.h>
#include <mach/notify.h>
#include <machine/machspl.h>		/* spl definitions */

#include <ipc/ipc_port.h>
#include <ipc/ipc_space.h>

#include <kern/ast.h>
#include <kern/counters.h>
#include <kern/queue.h>
#include <kern/zalloc.h>
#include <kern/thread.h>
#include <kern/task.h>
#include <kern/sched_prim.h>

#include <vm/memory_object.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>

#include <device/device_types.h>
#include <device/dev_hdr.h>
#include <device/conf.h>
#include <device/io_req.h>
#include <device/ds_routines.h>
#include <device/net_status.h>
#include <device/device_port.h>
#include <device/device_reply.h>

#include <machine/machspl.h>

io_return_t
ds_device_open(open_port, reply_port, reply_port_type,
	       mode, name, device_p)
	ipc_port_t	open_port;
	ipc_port_t	reply_port;
	mach_msg_type_name_t reply_port_type;
	dev_mode_t	mode;
	char *		name;
	device_t	*device_p;	/* out */
{
	register device_t	device;
	register kern_return_t	result;
	register io_req_t	ior;
	char			namebuf[64];
	ipc_port_t		notify;

	/*
	 * Open must be called on the master device port.
	 */
	if (open_port != master_device_port)
	    return (D_INVALID_OPERATION);

	/*
	 * There must be a reply port.
	 */
	if (!IP_VALID(reply_port)) {
	    printf("ds_* invalid reply port\n");
	    Debugger("ds_* reply_port");
	    return (MIG_NO_REPLY);	/* no sense in doing anything */
	}

#if	NORMA_DEVICE
	/*
	 * Map global device name to <node> + local device name.
	 */
	if (name[0] != '<') {
		extern char *dev_forward_name();

		name = dev_forward_name(name, namebuf, sizeof(namebuf));
	}
	/*
	 * Look for explicit node specifier, e.g., <2>sd0a.
	 * If found, then forward request to correct device server.
	 * If not found, then remove '<n>' and process locally.
	 *
	 * XXX should handle send-right reply_port as well as send-once XXX
	 */
	if (name[0] == '<') {
		char *n;
		int node = 0;

		for (n = &name[1]; *n != '>'; n++) {
			if (*n >= '0' && *n <= '9') {
				node = 10 * node + (*n - '0');
			} else {
				return (D_NO_SUCH_DEVICE);
			}
		}
		if (node == node_self()) {
			name = &n[1];	/* skip trailing '>' */
		} else {
			forward_device_open_send(remote_device(node),
						 reply_port, mode, name);
			return (MIG_NO_REPLY);
		}
	}
#endif	NORMA_DEVICE

	/*
	 * Find the device.
	 */
	device = device_lookup(name);
	if (device == DEVICE_NULL)
	    return (D_NO_SUCH_DEVICE);

	/*
	 * If the device is being opened or closed,
	 * wait for that operation to finish.
	 */
	device_lock(device);
	while (device->state == DEV_STATE_OPENING ||
		device->state == DEV_STATE_CLOSING) {
	    device->io_wait = TRUE;
	    thread_sleep((event_t)device, simple_lock_addr(device->lock), TRUE);
	    device_lock(device);
	}

	/*
	 * If the device is already open, increment the open count
	 * and return.
	 */
	if (device->state == DEV_STATE_OPEN) {

	    if (device->flag & D_EXCL_OPEN) {
		/*
		 * Cannot open a second time.
		 */
		device_unlock(device);
		device_deallocate(device);
		return (D_ALREADY_OPEN);
	    }

	    device->open_count++;
	    device_unlock(device);
	    *device_p = device;
	    return (D_SUCCESS);
	    /*
	     * Return deallocates device reference while acquiring
	     * port.
	     */
	}

	/*
	 * Allocate the device port and register the device before
	 * opening it.
	 */
	device->state = DEV_STATE_OPENING;
	device_unlock(device);

	/*
	 * Allocate port, keeping a reference for it.
	 */
	device->port = ipc_port_alloc_kernel();
	if (device->port == IP_NULL) {
	    device_lock(device);
	    device->state = DEV_STATE_INIT;
	    device->port = IP_NULL;
	    if (device->io_wait) {
		device->io_wait = FALSE;
		thread_wakeup((event_t)device);
	    }
	    device_unlock(device);
	    device_deallocate(device);
	    return (KERN_RESOURCE_SHORTAGE);
	}

	dev_port_enter(device);

	/*
	 * Request no-senders notifications on device port.
	 */
	notify = ipc_port_make_sonce(device->port);
	ip_lock(device->port);
	ipc_port_nsrequest(device->port, 1, notify, &notify);
	assert(notify == IP_NULL);

	/*
	 * Open the device.
	 */
	io_req_alloc(ior, 0);

	ior->io_device	= device;
	ior->io_unit	= device->dev_number;
	ior->io_op	= IO_OPEN | IO_CALL;
	ior->io_mode	= mode;
	ior->io_error	= 0;
	ior->io_done	= ds_open_done;
	ior->io_reply_port = reply_port;
	ior->io_reply_port_type = reply_port_type;

	result = (*device->dev_ops->d_open)(device->dev_number, (int)mode, ior);
	if (result == D_IO_QUEUED)
	    return (MIG_NO_REPLY);

	/*
	 * Return result via ds_open_done.
	 */
	ior->io_error = result;
	(void) ds_open_done(ior);

	io_req_free(ior);

	return (MIG_NO_REPLY);	/* reply already sent */
}

boolean_t
ds_open_done(ior)
	register io_req_t	ior;
{
	kern_return_t		result;
	register device_t	device;

	device = ior->io_device;
	result = ior->io_error;

	if (result != D_SUCCESS) {
	    /*
	     * Open failed.  Deallocate port and device.
	     */
	    dev_port_remove(device);
	    ipc_port_dealloc_kernel(device->port);
	    device->port = IP_NULL;

	    device_lock(device);
	    device->state = DEV_STATE_INIT;
	    if (device->io_wait) {
		device->io_wait = FALSE;
		thread_wakeup((event_t)device);
	    }
	    device_unlock(device);

	    device_deallocate(device);
	    device = DEVICE_NULL;
	}
	else {
	    /*
	     * Open succeeded.
	     */
	    device_lock(device);
	    device->state = DEV_STATE_OPEN;
	    device->open_count = 1;
	    if (device->io_wait) {
		device->io_wait = FALSE;
		thread_wakeup((event_t)device);
	    }
	    device_unlock(device);

	    /* donate device reference to get port */
	}
	/*
	 * Must explicitly convert device to port, since
	 * device_reply interface is built as 'user' side
	 * (thus cannot get translation).
	 */
	if (IP_VALID(ior->io_reply_port)) {
		(void) ds_device_open_reply(ior->io_reply_port,
					    ior->io_reply_port_type,
					    result,
					    convert_device_to_port(device));
	} else
		device_deallocate(device);

	return (TRUE);
}

io_return_t
ds_device_close(device)
	register device_t	device;
{
	if (device == DEVICE_NULL)
	    return (D_NO_SUCH_DEVICE);

	device_lock(device);

	/*
	 * If device will remain open, do nothing.
	 */
	if (--device->open_count > 0) {
	    device_unlock(device);
	    return (D_SUCCESS);
	}

	/*
	 * If device is being closed, do nothing.
	 */
	if (device->state == DEV_STATE_CLOSING) {
	    device_unlock(device);
	    return (D_SUCCESS);
	}

	/*
	 * Mark device as closing, to prevent new IO.
	 * Outstanding IO will still be in progress.
	 */
	device->state = DEV_STATE_CLOSING;
	device_unlock(device);

	/*
	 * ? wait for IO to end ?
	 *   only if device wants to
	 */

	/*
	 * Remove the device-port association.
	 */
	dev_port_remove(device);
	ipc_port_dealloc_kernel(device->port);

	/*
	 * Close the device
	 */
	(*device->dev_ops->d_close)(device->dev_number);

	/*
	 * Finally mark it closed.  If someone else is trying
	 * to open it, the open can now proceed.
	 */
	device_lock(device);
	device->state = DEV_STATE_INIT;
	if (device->io_wait) {
	    device->io_wait = FALSE;
	    thread_wakeup((event_t)device);
	}
	device_unlock(device);

	return (D_SUCCESS);
}

/*
 * Write to a device.
 */
io_return_t
ds_device_write(device, reply_port, reply_port_type, mode, recnum,
		data, data_count, bytes_written)
	register device_t	device;
	ipc_port_t		reply_port;
	mach_msg_type_name_t	reply_port_type;
	dev_mode_t		mode;
	recnum_t		recnum;
	io_buf_ptr_t		data;
	unsigned int		data_count;
	int			*bytes_written;	/* out */
{
	register io_req_t	ior;
	register io_return_t	result;

	/*
	 * Refuse if device is dead or not completely open.
	 */
	if (device == DEVICE_NULL)
	    return (D_NO_SUCH_DEVICE);

	if (device->state != DEV_STATE_OPEN)
	    return (D_NO_SUCH_DEVICE);

	if (data == 0)
	   return (D_INVALID_SIZE);

	/*
	 * XXX Need logic to reject ridiculously big requests.
	 */

	/* XXX note that a CLOSE may proceed at any point */

	/*
	 * Package the write request for the device driver
	 */
	io_req_alloc(ior, data_count);

	ior->io_device		= device;
	ior->io_unit		= device->dev_number;
	ior->io_op		= IO_WRITE | IO_CALL;
	ior->io_mode		= mode;
	ior->io_recnum		= recnum;
	ior->io_data		= data;
	ior->io_count		= data_count;
	ior->io_total		= data_count;
	ior->io_alloc_size	= 0;
	ior->io_residual	= 0;
	ior->io_error		= 0;
	ior->io_done		= ds_write_done;
	ior->io_reply_port	= reply_port;
	ior->io_reply_port_type	= reply_port_type;
	ior->io_copy		= VM_MAP_COPY_NULL;

	/*
	 * The ior keeps an extra reference for the device.
	 */
	device_reference(device);

	/*
	 * And do the write ...
	 *
	 * device_write_dealoc returns false if there's more
	 * to do; it has updated the ior appropriately and expects
	 * its caller to reinvoke it on the device.
	 */

	do {

		result = (*device->dev_ops->d_write)(device->dev_number, ior);

		/*
		 * If the IO was queued, delay reply until it is finished.
		 */
		if (result == D_IO_QUEUED)
		    return (MIG_NO_REPLY);

		/*
		 * Discard the local mapping of the data.
		 */

	} while (!device_write_dealloc(ior));

	/*
	 * Return the number of bytes actually written.
	 */
	*bytes_written = ior->io_total - ior->io_residual;

	/*
	 * Remove the extra reference.
	 */
	device_deallocate(device);

	io_req_free(ior);
	return (result);
}

/*
 * Write to a device, but memory is in message.
 */
io_return_t
ds_device_write_inband(device, reply_port, reply_port_type, mode, recnum,
		       data, data_count, bytes_written)
	register device_t	device;
	ipc_port_t		reply_port;
	mach_msg_type_name_t	reply_port_type;
	dev_mode_t		mode;
	recnum_t		recnum;
	io_buf_ptr_inband_t	data;
	unsigned int		data_count;
	int			*bytes_written; /* out */
{
	register io_req_t	ior;
	register io_return_t	result;

	/*
	 * Refuse if device is dead or not completely open.
	 */
	if (device == DEVICE_NULL)
	    return (D_NO_SUCH_DEVICE);

	if (device->state != DEV_STATE_OPEN)
	    return (D_NO_SUCH_DEVICE);

	if (data == 0)
	   return (D_INVALID_SIZE);

	/* XXX note that a CLOSE may proceed at any point */

	/*
	 * Package the write request for the device driver.
	 */
	io_req_alloc(ior, 0);

	ior->io_device		= device;
	ior->io_unit		= device->dev_number;
	ior->io_op		= IO_WRITE | IO_CALL | IO_INBAND;
	ior->io_mode		= mode;
	ior->io_recnum		= recnum;
	ior->io_data		= data;
	ior->io_count		= data_count;
	ior->io_total		= data_count;
	ior->io_alloc_size	= 0;
	ior->io_residual	= 0;
	ior->io_error		= 0;
	ior->io_done		= ds_write_done;
	ior->io_reply_port	= reply_port;
	ior->io_reply_port_type = reply_port_type;

	/*
	 * The ior keeps an extra reference for the device.
	 */
	device_reference(device);

	/*
	 * And do the write.
	 */
	result = (*device->dev_ops->d_write)(device->dev_number, ior);

	/*
	 * If the IO was queued, delay reply until it is finished.
	 */
	if (result == D_IO_QUEUED)
	    return (MIG_NO_REPLY);

	/*
	 * Return the number of bytes actually written.
	 */
	*bytes_written = ior->io_total - ior->io_residual;

	/*
	 * Remove the extra reference.
	 */
	device_deallocate(device);

	io_req_free(ior);
	return (result);
}

/*
 * Wire down incoming memory to give to device.
 */
kern_return_t
device_write_get(ior, wait)
	register io_req_t	ior;
	boolean_t		*wait;
{
	vm_map_copy_t		io_copy;
	vm_offset_t		new_addr;
	register kern_return_t	result;
	int			bsize;
	vm_size_t		min_size;

	/*
	 * By default, caller does not have to wait.
	 */
	*wait = FALSE;

	/*
	 * Nothing to do if no data.
	 */
	if (ior->io_count == 0)
	    return (KERN_SUCCESS);

	/*
	 * Loaned iors already have valid data.
	 */
	if (ior->io_op & IO_LOANED)
	    return (KERN_SUCCESS);

	/*
	 * Inband case.
	 */
	if (ior->io_op & IO_INBAND) {
	    assert(ior->io_count <= sizeof (io_buf_ptr_inband_t));
	    new_addr = zalloc(io_inband_zone);
	    bcopy((void*)ior->io_data, (void*)new_addr, ior->io_count);
	    ior->io_data = (io_buf_ptr_t)new_addr;
	    ior->io_alloc_size = sizeof (io_buf_ptr_inband_t);

	    return (KERN_SUCCESS);
	}

	/*
	 *	Figure out how much data to move this time.  If the device
	 *	won't return a block size, then we have to do the whole
	 *	request in one shot (ditto if this is a block fragment),
	 *	otherwise, move at least one block's worth.
	 */
	result = (*ior->io_device->dev_ops->d_dev_info)(
					ior->io_device->dev_number,
					D_INFO_BLOCK_SIZE,
					&bsize);

	if (result != KERN_SUCCESS || ior->io_count < (vm_size_t) bsize)
		min_size = (vm_size_t) ior->io_count;
	else
		min_size = (vm_size_t) bsize;

	/*
	 *	Map the pages from this page list into memory.
	 *	io_data records location of data.
	 *	io_alloc_size is the vm size of the region to deallocate.
	 */
	io_copy = (vm_map_copy_t) ior->io_data;
	result = kmem_io_map_copyout(device_io_map,
			(vm_offset_t*)&ior->io_data, &new_addr,
		        &ior->io_alloc_size, io_copy, min_size);
	if (result != KERN_SUCCESS)
	    return (result);

	if ((ior->io_data + ior->io_count) > 
	    (((char *)new_addr) + ior->io_alloc_size)) {

		/*
		 *	Operation has to be split.  Reset io_count for how
		 *	much we can do this time.
		 */
		assert(vm_map_copy_has_cont(io_copy));
		assert(ior->io_count == io_copy->size);
		ior->io_count = ior->io_alloc_size -
			(ior->io_data - ((char *)new_addr));

		/*
		 *	Caller must wait synchronously.
		 */
		ior->io_op &= ~IO_CALL;
		*wait = TRUE;		
	}

	ior->io_copy = io_copy;			/* vm_map_copy to discard */
	return (KERN_SUCCESS);
}

/*
 * Clean up memory allocated for IO.
 */
boolean_t
device_write_dealloc(ior)
	register io_req_t	ior;
{
	vm_map_copy_t	new_copy = VM_MAP_COPY_NULL;
	register
	vm_map_copy_t	io_copy;
	kern_return_t	result;
	vm_offset_t	size_to_do;
	int		bsize;	

	if (ior->io_alloc_size == 0)
	    return (TRUE);

	/*
	 * Inband case.
	 */
	if (ior->io_op & IO_INBAND) {
	    zfree(io_inband_zone, (vm_offset_t)ior->io_data);

	    return (TRUE);
	}
	
	if ((io_copy = ior->io_copy) == VM_MAP_COPY_NULL)
	    return (TRUE);

	/*
	 *	To prevent a possible deadlock with the default pager,
	 *	we have to release space in the device_io_map before
	 *	we allocate any memory.  (Which vm_map_copy_invoke_cont
	 *	might do.)  See the discussion in ds_init.
	 */

	kmem_io_map_deallocate(device_io_map,
		     	 trunc_page(ior->io_data),
			(vm_size_t) ior->io_alloc_size);

	if (vm_map_copy_has_cont(io_copy)) {

		/*
		 *	Remember how much is left, then 
		 *	invoke or abort the continuation.
		 */
		size_to_do = io_copy->size - ior->io_count;
		if (ior->io_error == 0) {
			vm_map_copy_invoke_cont(io_copy, &new_copy, &result);
		}
		else {
			vm_map_copy_abort_cont(io_copy);
			result = KERN_FAILURE;
		}

		if (result == KERN_SUCCESS && new_copy != VM_MAP_COPY_NULL) {
			register int	res;

			/*
			 *	We have a new continuation, reset the ior to
			 *	represent the remainder of the request.  Must
			 *	adjust the recnum because drivers assume
			 *	that the residual is zero.
			 */
			ior->io_op &= ~IO_DONE;
			ior->io_op |= IO_CALL;

			res = (*ior->io_device->dev_ops->d_dev_info)(
					ior->io_device->dev_number,
					D_INFO_BLOCK_SIZE,
					&bsize);

			if (res != D_SUCCESS)
				panic("device_write_dealloc: No block size");
			
			ior->io_recnum += ior->io_count/bsize;
			ior->io_count = new_copy->size;
		}
		else {

			/*
			 *	No continuation.  Add amount we didn't get
			 *	to into residual.
			 */
			ior->io_residual += size_to_do;
		}
	}

	/*
	 *	Clean up the state for the IO that just completed.
	 */
        vm_map_copy_discard(ior->io_copy);
	ior->io_copy = VM_MAP_COPY_NULL;
	ior->io_data = (char *) new_copy;

	/*
	 *	Return FALSE if there's more IO to do.
	 */

	return(new_copy == VM_MAP_COPY_NULL);
}

/*
 * Send write completion message to client, and discard the data.
 */
boolean_t
ds_write_done(ior)
	register io_req_t	ior;
{
	/*
	 *	device_write_dealloc discards the data that has been
	 *	written, but may decide that there is more to write.
	 */
	while (!device_write_dealloc(ior)) {
		register io_return_t	result;
		register device_t	device;

		/*
		 *     More IO to do -- invoke it.
		 */
		device = ior->io_device;
		result = (*device->dev_ops->d_write)(device->dev_number, ior);

		/*
		 * If the IO was queued, return FALSE -- not done yet.
		 */
		if (result == D_IO_QUEUED)
		    return (FALSE);
	}

	/*
	 *	Now the write is really complete.  Send reply.
	 */

	if (IP_VALID(ior->io_reply_port)) {
	    (void) (*((ior->io_op & IO_INBAND) ?
		      ds_device_write_reply_inband :
		      ds_device_write_reply))(ior->io_reply_port,
					      ior->io_reply_port_type,
					      ior->io_error,
					      (int) (ior->io_total -
						     ior->io_residual));
	}
	device_deallocate(ior->io_device);

	return (TRUE);
}

/*
 * Read from a device.
 */
io_return_t
ds_device_read(device, reply_port, reply_port_type, mode, recnum,
	       bytes_wanted, data, data_count)
	register device_t	device;
	ipc_port_t		reply_port;
	mach_msg_type_name_t	reply_port_type;
	dev_mode_t		mode;
	recnum_t		recnum;
	int			bytes_wanted;
	io_buf_ptr_t		*data;		/* out */
	unsigned int		*data_count;	/* out */
{
	register io_req_t	ior;
	register io_return_t	result;

#ifdef lint
	*data = *data;
	*data_count = *data_count;
#endif lint

	/*
	 * Refuse if device is dead or not completely open.
	 */
	if (device == DEVICE_NULL)
	    return (D_NO_SUCH_DEVICE);

	if (device->state != DEV_STATE_OPEN)
	    return (D_NO_SUCH_DEVICE);

	/* XXX note that a CLOSE may proceed at any point */

	/*
	 * There must be a reply port.
	 */
	if (!IP_VALID(reply_port)) {
	    printf("ds_* invalid reply port\n");
	    Debugger("ds_* reply_port");
	    return (MIG_NO_REPLY);	/* no sense in doing anything */
	}

	/*
	 * Package the read request for the device driver
	 */
	io_req_alloc(ior, 0);

	ior->io_device		= device;
	ior->io_unit		= device->dev_number;
	ior->io_op		= IO_READ | IO_CALL;
	ior->io_mode		= mode;
	ior->io_recnum		= recnum;
	ior->io_data		= 0;		/* driver must allocate data */
	ior->io_count		= bytes_wanted;
	ior->io_alloc_size	= 0;		/* no data allocated yet */
	ior->io_residual	= 0;
	ior->io_error		= 0;
	ior->io_done		= ds_read_done;
	ior->io_reply_port	= reply_port;
	ior->io_reply_port_type	= reply_port_type;

	/*
	 * The ior keeps an extra reference for the device.
	 */
	device_reference(device);

	/*
	 * And do the read.
	 */
	result = (*device->dev_ops->d_read)(device->dev_number, ior);

	/*
	 * If the IO was queued, delay reply until it is finished.
	 */
	if (result == D_IO_QUEUED)
	    return (MIG_NO_REPLY);

	/*
	 * Return result via ds_read_done.
	 */
	ior->io_error = result;
	(void) ds_read_done(ior);
	io_req_free(ior);

	return (MIG_NO_REPLY);	/* reply has already been sent. */
}

/*
 * Read from a device, but return the data 'inband.'
 */
io_return_t
ds_device_read_inband(device, reply_port, reply_port_type, mode, recnum,
		      bytes_wanted, data, data_count)
	register device_t	device;
	ipc_port_t		reply_port;
	mach_msg_type_name_t	reply_port_type;
	dev_mode_t		mode;
	recnum_t		recnum;
	int			bytes_wanted;
	char			*data;		/* pointer to OUT array */
	unsigned int		*data_count;	/* out */
{
	register io_req_t	ior;
	register io_return_t	result;

#ifdef lint
	*data = *data;
	*data_count = *data_count;
#endif lint

	/*
	 * Refuse if device is dead or not completely open.
	 */
	if (device == DEVICE_NULL)
	    return (D_NO_SUCH_DEVICE);

	if (device->state != DEV_STATE_OPEN)
	    return (D_NO_SUCH_DEVICE);

	/* XXX note that a CLOSE may proceed at any point */

	/*
	 * There must be a reply port.
	 */
	if (!IP_VALID(reply_port)) {
	    printf("ds_* invalid reply port\n");
	    Debugger("ds_* reply_port");
	    return (MIG_NO_REPLY);	/* no sense in doing anything */
	}

	/*
	 * Package the read for the device driver
	 */
	io_req_alloc(ior, 0);

	ior->io_device		= device;
	ior->io_unit		= device->dev_number;
	ior->io_op		= IO_READ | IO_CALL | IO_INBAND;
	ior->io_mode		= mode;
	ior->io_recnum		= recnum;
	ior->io_data		= 0;		/* driver must allocate data */
	ior->io_count		= 
	    ((bytes_wanted < sizeof(io_buf_ptr_inband_t)) ?
		bytes_wanted : sizeof(io_buf_ptr_inband_t));
	ior->io_alloc_size	= 0;		/* no data allocated yet */
	ior->io_residual	= 0;
	ior->io_error		= 0;
	ior->io_done		= ds_read_done;
	ior->io_reply_port	= reply_port;
	ior->io_reply_port_type	= reply_port_type;

	/*
	 * The ior keeps an extra reference for the device.
	 */
	device_reference(device);

	/*
	 * Do the read.
	 */
	result = (*device->dev_ops->d_read)(device->dev_number, ior);

	/*
	 * If the io was queued, delay reply until it is finished.
	 */
	if (result == D_IO_QUEUED)
	    return (MIG_NO_REPLY);

	/*
	 * Return result, via ds_read_done.
	 */
	ior->io_error = result;
	(void) ds_read_done(ior);
	io_req_free(ior);

	return (MIG_NO_REPLY);	/* reply has already been sent. */
}


/*
 * Allocate wired-down memory for device read.
 */
kern_return_t device_read_alloc(ior, size)
	register io_req_t	ior;
	register vm_size_t	size;
{
	vm_offset_t		addr;
	kern_return_t		kr;

	/*
	 * Nothing to do if no data.
	 */
	if (ior->io_count == 0)
	    return (KERN_SUCCESS);

	if (ior->io_op & IO_INBAND) {
	    ior->io_data = (io_buf_ptr_t) zalloc(io_inband_zone);
	    ior->io_alloc_size = sizeof(io_buf_ptr_inband_t);
	} else {
	    size = round_page(size);
	    kr = kmem_alloc(kernel_map, &addr, size);
	    if (kr != KERN_SUCCESS)
		return (kr);

	    ior->io_data = (io_buf_ptr_t) addr;
	    ior->io_alloc_size = size;
	}

	return (KERN_SUCCESS);
}

boolean_t ds_read_done(ior)
	io_req_t	ior;
{
	vm_offset_t		start_data, end_data;
	vm_offset_t		start_sent, end_sent;
	register vm_size_t	size_read;

	if (ior->io_error)
	    size_read = 0;
	else
	    size_read = ior->io_count - ior->io_residual;

	start_data  = (vm_offset_t)ior->io_data;
	end_data    = start_data + size_read;

	start_sent  = (ior->io_op & IO_INBAND) ? start_data :
						trunc_page(start_data);
	end_sent    = (ior->io_op & IO_INBAND) ? 
		start_data + ior->io_alloc_size : round_page(end_data);

	/*
	 * Zero memory that the device did not fill.
	 */
	if (start_sent < start_data)
	    bzero((char *)start_sent, start_data - start_sent);
	if (end_sent > end_data)
	    bzero((char *)end_data, end_sent - end_data);


	/*
	 * Touch the data being returned, to mark it dirty.
	 * If the pages were filled by DMA, the pmap module
	 * may think that they are clean.
	 */
	{
	    register vm_offset_t	touch;
	    register int		c;

	    for (touch = start_sent; touch < end_sent; touch += PAGE_SIZE) {
		c = *(char *)touch;
		*(char *)touch = c;
	    }
	}

	/*
	 * Send the data to the reply port - this
	 * unwires and deallocates it.
	 */
	if (ior->io_op & IO_INBAND) {
	    (void)ds_device_read_reply_inband(ior->io_reply_port,
					      ior->io_reply_port_type,
					      ior->io_error,
					      (char *) start_data,
					      size_read);
	} else {
	    vm_map_copy_t copy;
	    kern_return_t kr;

	    kr = vm_map_copyin_page_list(kernel_map, start_data,
						 size_read, TRUE, TRUE,
						 &copy, FALSE);

	    if (kr != KERN_SUCCESS)
		panic("read_done: vm_map_copyin_page_list failed");

	    (void)ds_device_read_reply(ior->io_reply_port,
				       ior->io_reply_port_type,
				       ior->io_error,
				       (char *) copy,
				       size_read);
	}

	/*
	 * Free any memory that was allocated but not sent.
	 */
	if (ior->io_count != 0) {
	    if (ior->io_op & IO_INBAND) {
		if (ior->io_alloc_size > 0)
		    zfree(io_inband_zone, (vm_offset_t)ior->io_data);
	    } else {
		register vm_offset_t	end_alloc;

		end_alloc = start_sent + round_page(ior->io_alloc_size);
		if (end_alloc > end_sent)
		    (void) vm_deallocate(kernel_map,
					 end_sent,
					 end_alloc - end_sent);
	    }
	}

	device_deallocate(ior->io_device);

	return (TRUE);
}

io_return_t
ds_device_set_status(device, flavor, status, status_count)
	register device_t	device;
	dev_flavor_t		flavor;
	dev_status_t		status;
	mach_msg_type_number_t	status_count;
{
	/*
	 * Refuse if device is dead or not completely open.
	 */
	if (device == DEVICE_NULL)
	    return (D_NO_SUCH_DEVICE);

	if (device->state != DEV_STATE_OPEN)
	    return (D_NO_SUCH_DEVICE);

	/* XXX note that a CLOSE may proceed at any point */

	return ((*device->dev_ops->d_setstat)(device->dev_number,
					      flavor,
					      status,
					      status_count));
}

io_return_t
ds_device_get_status(device, flavor, status, status_count)
	register device_t	device;
	dev_flavor_t		flavor;
	dev_status_t		status;		/* pointer to OUT array */
	mach_msg_type_number_t	*status_count;	/* out */
{
	/*
	 * Refuse if device is dead or not completely open.
	 */
	if (device == DEVICE_NULL)
	    return (D_NO_SUCH_DEVICE);

	if (device->state != DEV_STATE_OPEN)
	    return (D_NO_SUCH_DEVICE);

	/* XXX note that a CLOSE may proceed at any point */

	return ((*device->dev_ops->d_getstat)(device->dev_number,
					      flavor,
					      status,
					      status_count));
}

io_return_t
ds_device_set_filter(device, receive_port, priority, filter, filter_count)
	register device_t	device;
	ipc_port_t		receive_port;
	int			priority;
	filter_t		filter[];	/* pointer to IN array */
	unsigned int		filter_count;
{
	/*
	 * Refuse if device is dead or not completely open.
	 */
	if (device == DEVICE_NULL)
	    return (D_NO_SUCH_DEVICE);

	if (device->state != DEV_STATE_OPEN)
	    return (D_NO_SUCH_DEVICE);

	/* XXX note that a CLOSE may proceed at any point */

	/*
	 * Request is absurd if no receive port is specified.
	 */
	if (!IP_VALID(receive_port))
	    return (D_INVALID_OPERATION);

	return ((*device->dev_ops->d_async_in)(device->dev_number,
					       receive_port,
					       priority,
					       filter,
					       filter_count));
}

io_return_t
ds_device_map(device, protection, offset, size, pager, unmap)
	register device_t	device;
	vm_prot_t		protection;
	vm_offset_t		offset;
	vm_size_t		size;
	ipc_port_t		*pager;	/* out */
	boolean_t		unmap;	/* ? */
{
#ifdef	lint
	unmap = unmap;
#endif	lint
	if (protection & ~VM_PROT_ALL)
		return (KERN_INVALID_ARGUMENT);
	/*
	 * Refuse if device is dead or not completely open.
	 */
	if (device == DEVICE_NULL)
	    return (D_NO_SUCH_DEVICE);

	if (device->state != DEV_STATE_OPEN)
	    return (D_NO_SUCH_DEVICE);

	/* XXX note that a CLOSE may proceed at any point */

	return (device_pager_setup(device, protection, offset, size,
				    (mach_port_t*)pager));
}

/*
 * Doesn't do anything (yet).
 */
void ds_no_senders(notification)
	mach_no_senders_notification_t *notification;
{
	printf("ds_no_senders called! device_port=0x%x count=%d\n",
	       notification->not_header.msgh_remote_port,
	       notification->not_count);
}

boolean_t
ds_notify(msg)
	mach_msg_header_t *msg;
{
	switch (msg->msgh_id) {
		case MACH_NOTIFY_NO_SENDERS:
		ds_no_senders((mach_no_senders_notification_t *) msg);
		return TRUE;

		default:
		printf("ds_notify: strange notification %d\n", msg->msgh_id);
		return FALSE;
	}
}

queue_head_t		io_done_list;
decl_simple_lock_data(,	io_done_list_lock)

#define	splio	splsched	/* XXX must block ALL io devices */

void iodone(ior)
	register io_req_t	ior;
{
	register spl_t	s;

	/*
	 * If this ior was loaned to us, return it directly.
	 */
	if (ior->io_op & IO_LOANED) {
		(*ior->io_done)(ior);
		return;
	}
	/*
	 * If !IO_CALL, some thread is waiting for this.  Must lock
	 * structure to interlock correctly with iowait().  Else can
	 * toss on queue for io_done thread to call completion.
	 */
	s = splio();
	if ((ior->io_op & IO_CALL) == 0) {
	    ior_lock(ior);
	    ior->io_op |= IO_DONE;
	    ior->io_op &= ~IO_WANTED;
	    ior_unlock(ior);
	    thread_wakeup((event_t)ior);
	} else {
	    ior->io_op |= IO_DONE;
	    simple_lock(&io_done_list_lock);
	    enqueue_tail(&io_done_list, (queue_entry_t)ior);
	    thread_wakeup((event_t)&io_done_list);
	    simple_unlock(&io_done_list_lock);
	}
	splx(s);
}

void io_done_thread_continue()
{
	for (;;) {
	    register spl_t	s;
	    register io_req_t	ior;

	    s = splio();
	    simple_lock(&io_done_list_lock);
	    while ((ior = (io_req_t)dequeue_head(&io_done_list)) != 0) {
		simple_unlock(&io_done_list_lock);
		(void) splx(s);

		if ((*ior->io_done)(ior)) {
		    /*
		     * IO done - free io_req_elt
		     */
		    io_req_free(ior);
		}
		/* else routine has re-queued it somewhere */

		s = splio();
		simple_lock(&io_done_list_lock);
	    }

	    assert_wait(&io_done_list, FALSE);
	    simple_unlock(&io_done_list_lock);
	    (void) splx(s);
	    counter(c_io_done_thread_block++);
	    thread_block(io_done_thread_continue);
	}
}

void io_done_thread()
{
	/*
	 * Set thread privileges and highest priority.
	 */
	current_thread()->vm_privilege = TRUE;
	stack_privilege(current_thread());
	thread_set_own_priority(0);

	io_done_thread_continue();
	/*NOTREACHED*/
}

#define	DEVICE_IO_MAP_SIZE	(2 * 1024 * 1024)

extern void ds_trap_init(void);		/* forward */

void ds_init()
{
	vm_offset_t	device_io_min, device_io_max;

	queue_init(&io_done_list);
	simple_lock_init(&io_done_list_lock);

	device_io_map = kmem_suballoc(kernel_map,
				      &device_io_min,
				      &device_io_max,
				      DEVICE_IO_MAP_SIZE,
				      FALSE);
	/*
	 *	If the kernel receives many device_write requests, the
	 *	device_io_map might run out of space.  To prevent
	 *	device_write_get from failing in this case, we enable
	 *	wait_for_space on the map.  This causes kmem_io_map_copyout
	 *	to block until there is sufficient space.
	 *	(XXX Large writes may be starved by small writes.)
	 *
	 *	There is a potential deadlock problem with this solution,
	 *	if a device_write from the default pager has to wait
	 *	for the completion of a device_write which needs to wait
	 *	for memory allocation.  Hence, once device_write_get
	 *	allocates space in device_io_map, no blocking memory
	 *	allocations should happen until device_write_dealloc
	 *	frees the space.  (XXX A large write might starve
	 *	a small write from the default pager.)
	 */
	device_io_map->wait_for_space = TRUE;

	io_inband_zone = zinit(sizeof(io_buf_ptr_inband_t),
			    1000 * sizeof(io_buf_ptr_inband_t),
			    10 * sizeof(io_buf_ptr_inband_t),
			    FALSE,
			    "io inband read buffers");

	ds_trap_init();
}

void iowait(ior)
    io_req_t ior;
{
    spl_t s;

    s = splio();
    ior_lock(ior);
    while ((ior->io_op&IO_DONE)==0) {
	assert_wait((event_t)ior, FALSE);
	ior_unlock(ior);
	thread_block((void (*)()) 0);
        ior_lock(ior);
    }
    ior_unlock(ior);
    splx(s);
}


/*
 * Device trap support.
 */

/*
 * Memory Management
 *
 * This currently has a single pool of 2k wired buffers
 * since we only handle writes to an ethernet device.
 * Should be more general.
 */
#define IOTRAP_REQSIZE 2048

zone_t io_trap_zone;

/*
 * Initialization.  Called from ds_init().
 */
void
ds_trap_init(void)
{
	io_trap_zone = zinit(IOTRAP_REQSIZE,
			     256 * IOTRAP_REQSIZE,
			     16 * IOTRAP_REQSIZE,
			     FALSE,
			     "wired device trap buffers");
}

/*
 * Allocate an io_req_t.
 * Currently zalloc's from io_trap_zone.
 *
 * Could have lists of different size zones.
 * Could call a device-specific routine.
 */
io_req_t
ds_trap_req_alloc(device_t device, vm_size_t data_size)
{
	return (io_req_t) zalloc(io_trap_zone);
}

/*
 * Called by iodone to release ior.
 */
boolean_t
ds_trap_write_done(io_req_t ior)
{
	register device_t dev;

	dev = ior->io_device;

	/*
	 * Should look at reply port and maybe send a message.
	 */
	zfree(io_trap_zone, ior);
	
	/*
	 * Give up device reference from ds_write_trap.
	 */
	device_deallocate(dev);
	return TRUE;
}

/*
 * Like device_write except that data is in user space.
 */
io_return_t
ds_device_write_trap(device_t		device,
		     dev_mode_t		mode,
		     recnum_t		recnum,
		     vm_offset_t	data,
		     vm_size_t		data_count)
{
	io_req_t ior;
	io_return_t result;

	/*
	 * Refuse if device is dead or not completely open.
	 */
	if (device == DEVICE_NULL)
		return (D_NO_SUCH_DEVICE);
 
	if (device->state != DEV_STATE_OPEN)
		return (D_NO_SUCH_DEVICE);
 
	/* XXX note that a CLOSE may proceed at any point */
 
	/*
	 * Get a buffer to hold the ioreq.
	 */
	ior = ds_trap_req_alloc(device, data_count);
 
	/*
	 * Package the write request for the device driver.
	 */

	ior->io_device          = device;
	ior->io_unit            = device->dev_number;
	ior->io_op              = IO_WRITE | IO_CALL | IO_LOANED;
	ior->io_mode            = mode;
	ior->io_recnum          = recnum;
	ior->io_data            = (io_buf_ptr_t)
		(vm_offset_t)ior + sizeof(struct io_req);
	ior->io_count           = data_count;
	ior->io_total           = data_count;
	ior->io_alloc_size      = 0;
	ior->io_residual        = 0;
	ior->io_error           = 0;
	ior->io_done            = ds_trap_write_done;
	ior->io_reply_port      = IP_NULL;	/* XXX */
	ior->io_reply_port_type = 0;		/* XXX */

	/*
	 * Copy the data from user space.
	 */
	if (data_count > 0)
		copyin((char *)data, (char *)ior->io_data, data_count);
 
	/*
	 * The ior keeps an extra reference for the device.
	 */
	device_reference(device);
 
	/*
	 * And do the write.
	 */
	result = (*device->dev_ops->d_write)(device->dev_number, ior);
 
	/*
	 * If the IO was queued, delay reply until it is finished.
	 */
	if (result == D_IO_QUEUED)
		return (MIG_NO_REPLY);
 
	/*
	 * Remove the extra reference.
	 */
	device_deallocate(device);
 
	zfree(io_trap_zone, ior);
	return (result);
}

io_return_t
ds_device_writev_trap(device_t		device,
		      dev_mode_t	mode,
		      recnum_t		recnum,
		      io_buf_vec_t	*iovec,
		      vm_size_t		iocount)
{
	io_req_t ior;
	io_return_t result;
	io_buf_vec_t	stack_iovec[16]; /* XXX */
	vm_size_t data_count;
	int i;

	/*
	 * Refuse if device is dead or not completely open.
	 */
	if (device == DEVICE_NULL)
		return (D_NO_SUCH_DEVICE);
 
	if (device->state != DEV_STATE_OPEN)
		return (D_NO_SUCH_DEVICE);
 
	/* XXX note that a CLOSE may proceed at any point */
 
	/*
	 * Copyin user addresses.
	 */
	if (iocount > 16)
		return KERN_INVALID_VALUE; /* lame */
	copyin((char *)iovec,
	       (char *)stack_iovec,
	       iocount * sizeof(io_buf_vec_t));
	for (data_count = 0, i = 0; i < iocount; i++)
		data_count += stack_iovec[i].count;

	/*
	 * Get a buffer to hold the ioreq.
	 */
	ior = ds_trap_req_alloc(device, data_count);
 
	/*
	 * Package the write request for the device driver.
	 */

	ior->io_device          = device;
	ior->io_unit            = device->dev_number;
	ior->io_op              = IO_WRITE | IO_CALL | IO_LOANED;
	ior->io_mode            = mode;
	ior->io_recnum          = recnum;
	ior->io_data            = (io_buf_ptr_t)
		(vm_offset_t)ior + sizeof(struct io_req);
	ior->io_count           = data_count;
	ior->io_total           = data_count;
	ior->io_alloc_size      = 0;
	ior->io_residual        = 0;
	ior->io_error           = 0;
	ior->io_done            = ds_trap_write_done;
	ior->io_reply_port      = IP_NULL;	/* XXX */
	ior->io_reply_port_type = 0;		/* XXX */

	/*
	 * Copy the data from user space.
	 */
	if (data_count > 0) {
		vm_offset_t p;

		p = (vm_offset_t) ior->io_data;
		for (i = 0; i < iocount; i++) {
			copyin((char *) stack_iovec[i].data,
			       (char *) p,
			       stack_iovec[i].count);
			p += stack_iovec[i].count;
		}
	}
 
	/*
	 * The ior keeps an extra reference for the device.
	 */
	device_reference(device);
 
	/*
	 * And do the write.
	 */
	result = (*device->dev_ops->d_write)(device->dev_number, ior);
 
	/*
	 * If the IO was queued, delay reply until it is finished.
	 */
	if (result == D_IO_QUEUED)
		return (MIG_NO_REPLY);
 
	/*
	 * Remove the extra reference.
	 */
	device_deallocate(device);
 
	zfree(io_trap_zone, ior);
	return (result);
}
