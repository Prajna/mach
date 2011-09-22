/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
 * $Log:	dev_lookup.c,v $
 * Revision 2.12  93/05/10  21:18:15  rvb
 * 	Removed depends on DEV_BSIZE, now it is just a default
 * 	for backward compat.
 * 	[93/05/06  11:09:37  af]
 * 
 * Revision 2.11  92/08/03  17:33:06  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.10  92/05/21  17:08:49  jfriedl
 * 	tried prototypes.
 * 	[92/05/20            jfriedl]
 * 
 * Revision 2.9  91/06/25  10:26:31  rpd
 * 	Changed the convert_foo_to_bar functions
 * 	to use ipc_port_t instead of mach_port_t.
 * 	[91/05/29            rpd]
 * 
 * Revision 2.8  91/05/18  14:29:28  rpd
 * 	Fixed device_deallocate to always unlock the reference count.
 * 	[91/04/03            rpd]
 * 
 * Revision 2.7  91/05/14  15:40:44  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/02/05  17:08:25  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:27:17  mrt]
 * 
 * Revision 2.5  90/09/09  14:31:13  rpd
 * 	Use decl_simple_lock_data.
 * 	[90/08/30            rpd]
 * 
 * Revision 2.4  90/08/27  21:55:02  dbg
 * 	Remove obsolete type names and macros.
 * 	[90/07/16            dbg]
 * 
 * Revision 2.3  90/06/02  14:47:14  rpd
 * 	Converted to new IPC.
 * 	Fixed device leak in convert_device_to_port.
 * 	[90/03/26  21:45:09  rpd]
 * 
 * Revision 2.2  89/09/08  11:23:12  dbg
 * 	Modified to run in kernel context.  Moved name search routines
 * 	to dev_name.c.  Reorganized remaining routines.  Added
 * 	correct locking.
 * 	[89/08/01            dbg]
 * 
 *  5-Jun-89  Randall Dean (rwd) at Carnegie-Mellon University
 *	Added dev_change_indirect for use by sun autoconf (ms,kbd,fb)
 *
 * 12-May-89  David Golub (dbg) at Carnegie-Mellon University
 *	Added search through indirection table for certain devices.
 *
 * 12-Apr-89  David Golub (dbg) at Carnegie-Mellon University
 *	Added routine to call a function on each device.
 *
 *  3-Mar-89  David Golub (dbg) at Carnegie-Mellon University
 *	Created.
 *
 */
/*
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date: 	3/89
 */

#include <mach/port.h>
#include <mach/vm_param.h>

#include <kern/queue.h>
#include <kern/zalloc.h>

#include <device/device_types.h>
#include <device/dev_hdr.h>
#include <device/conf.h>
#include <device/param.h>		/* DEV_BSIZE, as default */

#include <ipc/ipc_port.h>
#include <kern/ipc_kobject.h>



/*
 * Device structure routines: reference counting, port->device.
 */

/*
 * Lookup/enter by device number.
 */
#define	NDEVHASH	8
#define	DEV_NUMBER_HASH(dev)	((dev) & (NDEVHASH-1))
queue_head_t	dev_number_hash_table[NDEVHASH];

/*
 * Lock for device-number to device lookup.
 * Must be held before device-ref_count lock.
 */
decl_simple_lock_data(,
		dev_number_lock)

zone_t		dev_hdr_zone;

/*
 * Enter device in the number lookup table.
 * The number table lock must be held.
 */
void
dev_number_enter(device)
	register device_t	device;
{
	register queue_t	q;

	q = &dev_number_hash_table[DEV_NUMBER_HASH(device->dev_number)];
	queue_enter(q, device, device_t, number_chain);
}

/*
 * Remove device from the device-number lookup table.
 * The device-number table lock must be held.
 */
void
dev_number_remove(device)
	register device_t	device;
{
	register queue_t	q;

	q = &dev_number_hash_table[DEV_NUMBER_HASH(device->dev_number)];
	queue_remove(q, device, device_t, number_chain);
}

/*
 * Lookup a device by device operations and minor number.
 * The number table lock must be held.
 */
device_t
dev_number_lookup(ops, devnum)
	dev_ops_t	ops;
	int		devnum;
{
	register queue_t	q;
	register device_t	device;

	q = &dev_number_hash_table[DEV_NUMBER_HASH(devnum)];
	queue_iterate(q, device, device_t, number_chain) {
	    if (device->dev_ops == ops && device->dev_number == devnum) {
		return (device);
	    }
	}
	return (DEVICE_NULL);
}

/*
 * Look up a device by name, and create the device structure
 * if it does not exist.  Enter it in the dev_number lookup
 * table.
 */
device_t
device_lookup(name)
	char *		name;
{
	dev_ops_t	dev_ops;
	int		dev_minor;
	register device_t	device;
	register device_t	new_device;

	/*
	 * Get the device and unit number from the name.
	 */
	if (!dev_name_lookup(name, &dev_ops, &dev_minor))
	    return (DEVICE_NULL);

	/*
	 * Look up the device in the hash table.  If it is
	 * not there, enter it.
	 */
	new_device = DEVICE_NULL;
	simple_lock(&dev_number_lock);
	while ((device = dev_number_lookup(dev_ops, dev_minor))
		== DEVICE_NULL) {
	    /*
	     * Must unlock to allocate the structure.  If
	     * the structure has appeared after we have allocated,
	     * release the new structure.
	     */
	    if (new_device != DEVICE_NULL)
		break;	/* allocated */

	    simple_unlock(&dev_number_lock);

	    new_device = (device_t) zalloc(dev_hdr_zone);
	    simple_lock_init(&new_device->ref_lock);
	    new_device->ref_count = 1;
	    simple_lock_init(&new_device->lock);
	    new_device->state = DEV_STATE_INIT;
	    new_device->flag = 0;
	    new_device->open_count = 0;
	    new_device->io_in_progress = 0;
	    new_device->io_wait = FALSE;
	    new_device->port = IP_NULL;
	    new_device->dev_ops = dev_ops;
	    new_device->dev_number = dev_minor;
	    new_device->bsize = DEV_BSIZE;	/* change later */

	    simple_lock(&dev_number_lock);
	}

	if (device == DEVICE_NULL) {
	    /*
	     * No existing device structure.  Insert the
	     * new one.
	     */
	    assert(new_device != DEVICE_NULL);
	    device = new_device;

	    dev_number_enter(device);
	    simple_unlock(&dev_number_lock);
	}
	else {
	    /*
	     * Have existing device.
	     */
	    device_reference(device);
	    simple_unlock(&dev_number_lock);

	    if (new_device != DEVICE_NULL)
		zfree(dev_hdr_zone, (vm_offset_t)new_device);
	}

	return (device);
}

/*
 * Add a reference to the device.
 */
void
device_reference(device)
	register device_t	device;
{
	simple_lock(&device->ref_lock);
	device->ref_count++;
	simple_unlock(&device->ref_lock);
}

/*
 * Remove a reference to the device, and deallocate the
 * structure if no references are left.
 */
void
device_deallocate(device)
	register device_t	device;
{
	simple_lock(&device->ref_lock);
	if (--device->ref_count > 0) {
	    simple_unlock(&device->ref_lock);
	    return;
	}
	device->ref_count = 1;
	simple_unlock(&device->ref_lock);

	simple_lock(&dev_number_lock);
	simple_lock(&device->ref_lock);
	if (--device->ref_count > 0) {
	    simple_unlock(&device->ref_lock);
	    simple_unlock(&dev_number_lock);
	    return;
	}

	dev_number_remove(device);
	simple_unlock(&device->ref_lock);
	simple_unlock(&dev_number_lock);

	zfree(dev_hdr_zone, (vm_offset_t)device);
}

/*

 */
/*
 * port-to-device lookup routines.
 */
decl_simple_lock_data(,
	dev_port_lock)

/*
 * Enter a port-to-device mapping.
 */
void
dev_port_enter(device)
	register device_t	device;
{
	device_reference(device);
	ipc_kobject_set(device->port, (ipc_kobject_t) device, IKOT_DEVICE);
}

/*
 * Remove a port-to-device mapping.
 */
void
dev_port_remove(device)
	register device_t	device;
{
	ipc_kobject_set(device->port, IKO_NULL, IKOT_NONE);
	device_deallocate(device);
}

/*
 * Lookup a device by its port.
 * Doesn't consume the naked send right; produces a device reference.
 */
device_t
dev_port_lookup(port)
	ipc_port_t	port;
{
	register device_t	device;

	if (!IP_VALID(port))
	    return (DEVICE_NULL);

	ip_lock(port);
	if (ip_active(port) && (ip_kotype(port) == IKOT_DEVICE)) {
	    device = (device_t) port->ip_kobject;
	    device_reference(device);
	}
	else
	    device = DEVICE_NULL;

	ip_unlock(port);
	return (device);
}

/*
 * Get the port for a device.
 * Consumes a device reference; produces a naked send right.
 */
ipc_port_t
convert_device_to_port(device)
	register device_t	device;
{
	register ipc_port_t	port;

	if (device == DEVICE_NULL)
	    return IP_NULL;

	device_lock(device);
	if (device->state == DEV_STATE_OPEN)
	    port = ipc_port_make_send(device->port);
	else
	    port = IP_NULL;
	device_unlock(device);

	device_deallocate(device);
	return port;
}

/*
 * Call a supplied routine on each device, passing it
 * the port as an argument.  If the routine returns TRUE,
 * stop the search and return TRUE.  If none returns TRUE,
 * return FALSE.
 */
boolean_t
dev_map(routine, port)
	boolean_t	(*routine)();
	mach_port_t	port;
{
	register int		i;
	register queue_t	q;
	register device_t	dev, prev_dev;

	for (i = 0, q = &dev_number_hash_table[0];
	     i < NDEVHASH;
	     i++, q++) {
	    prev_dev = DEVICE_NULL;
	    simple_lock(&dev_number_lock);
	    queue_iterate(q, dev, device_t, number_chain) {
		device_reference(dev);
		simple_unlock(&dev_number_lock);
		if (prev_dev != DEVICE_NULL)
		    device_deallocate(prev_dev);

		if ((*routine)(dev, port)) {
		    /*
		     * Done
		     */
		    device_deallocate(dev);
		    return (TRUE);
		}

		simple_lock(&dev_number_lock);
		prev_dev = dev;
	    }
	    simple_unlock(&dev_number_lock);
	    if (prev_dev != DEVICE_NULL)
		device_deallocate(prev_dev);
	}
	return (FALSE);
}

/*
 * Initialization
 */
#define	NDEVICES	256

void
dev_lookup_init()
{
	register int	i;

	simple_lock_init(&dev_number_lock);

	for (i = 0; i < NDEVHASH; i++)
	    queue_init(&dev_number_hash_table[i]);

	simple_lock_init(&dev_port_lock);

	dev_hdr_zone = zinit(sizeof(struct device),
			     sizeof(struct device) * NDEVICES,
			     PAGE_SIZE,
			     FALSE,
			     "open device entry");
}
