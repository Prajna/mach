/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	dev_hdr.h,v $
 * Revision 2.8  93/05/10  21:18:11  rvb
 * 	Added block size (bsize) field, replaces DEV_BSIZE.
 * 	[93/05/06  11:10:35  af]
 * 
 * Revision 2.7  91/05/14  15:40:30  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/02/05  17:08:20  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:27:12  mrt]
 * 
 * Revision 2.5  90/09/09  14:31:08  rpd
 * 	Use decl_simple_lock_data.
 * 	[90/08/30            rpd]
 * 
 * Revision 2.4  90/08/27  21:54:45  dbg
 * 	Fix type definitions.
 * 	[90/07/16            dbg]
 * 
 * Revision 2.3  90/06/02  14:47:10  rpd
 * 	Updated for new IPC.
 * 	[90/03/26  21:43:28  rpd]
 * 
 * Revision 2.2  89/09/08  11:23:07  dbg
 * 	Rename to 'struct device' and 'device_t'.  Added open-
 * 	state.  Removed most of old flags.
 * 	[89/08/01            dbg]
 * 
 * 12-Apr-89  David Golub (dbg) at Carnegie-Mellon University
 *	Added routine to call a function on each device.
 *
 *  3-Mar-89  David Golub (dbg) at Carnegie-Mellon University
 *	Created.
 */
/*
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date: 	3/89
 */

#ifndef	_DEVICE_DEV_HDR_H_
#define	_DEVICE_DEV_HDR_H_

#include <mach/port.h>
#include <kern/lock.h>
#include <kern/queue.h>

#include <device/conf.h>

/*
 * Generic device header.  May be allocated with the device,
 * or built when the device is opened.
 */
struct device {
	decl_simple_lock_data(,ref_lock)/* lock for reference count */
	int		ref_count;	/* reference count */
	decl_simple_lock_data(, lock)	/* lock for rest of state */
	short		state;		/* state: */
#define	DEV_STATE_INIT		0	/* not open  */
#define	DEV_STATE_OPENING	1	/* being opened */
#define	DEV_STATE_OPEN		2	/* open */
#define	DEV_STATE_CLOSING	3	/* being closed */
	short		flag;		/* random flags: */
#define	D_EXCL_OPEN		0x0001	/* open only once */
	short		open_count;	/* number of times open */
	short		io_in_progress;	/* number of IOs in progress */
	boolean_t	io_wait;	/* someone waiting for IO to finish */

	struct ipc_port *port;		/* open port */
	queue_chain_t	number_chain;	/* chain for lookup by number */
	int		dev_number;	/* device number */
	int		bsize;		/* replacement for DEV_BSIZE */
	struct dev_ops	*dev_ops;	/* and operations vector */
};
typedef	struct device	*device_t;
#define	DEVICE_NULL	((device_t)0)

/*
 * To find and remove device entries
 */
device_t	device_lookup();	/* by name */

void		device_reference();
void		device_deallocate();

/*
 * To find and remove port-to-device mappings
 */
device_t	dev_port_lookup();
void		dev_port_enter();
void		dev_port_remove();

/*
 * To call a routine on each device
 */
boolean_t	dev_map();

/*
 * To lock and unlock state and open-count
 */
#define	device_lock(device)	simple_lock(&(device)->lock)
#define	device_unlock(device)	simple_unlock(&(device)->lock)

#endif	/* _DEVICE_DEV_HDR_H_ */
