/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * Copyright (c) 1991 Sequent Computer Systems
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND SEQUENT COMPUTER SYSTEMS ALLOW FREE USE OF
 * THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * SEQUENT COMPUTER SYSTEMS DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	stubs.c,v $
 * Revision 2.4  92/01/03  20:28:13  dbg
 * 	Add iopl_device.
 * 	[91/11/26            dbg]
 * 
 * Revision 2.3  91/07/31  18:04:39  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:01:00  dbg
 * 	Created.
 * 	[91/04/26  15:00:31  dbg]
 * 
 */

/*
 * Stubs for random routines and data.
 */
#include <mach/port.h>
#include <device/dev_hdr.h>

mach_port_t	iopl_device_port = MACH_PORT_NULL;
device_t	iopl_device = DEVICE_NULL;

unsigned int	microdata = 50;	/* loop count for 10 microseconds */

cpusboot()
{
}

disk_offline()
{
}

disk_online()
{
}

todclock()
{
}

