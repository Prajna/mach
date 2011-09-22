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
 * $Log:	device_init.c,v $
 * Revision 2.14  93/02/01  09:46:41  danner
 * 	Added call to chario_init.
 * 	[93/01/25            danner]
 * 
 * Revision 2.13  92/08/03  17:33:28  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.12  92/05/21  17:09:11  jfriedl
 * 	tried prototypes.
 * 	[92/05/20            jfriedl]
 * 
 * Revision 2.11  91/08/03  18:17:30  jsb
 * 	Removed NORMA hooks.
 * 	[91/07/17  23:06:14  jsb]
 * 
 * Revision 2.10  91/06/17  15:43:55  jsb
 * 	Renamed NORMA conditionals.
 * 	[91/06/17  09:58:32  jsb]
 * 
 * Revision 2.9  91/06/06  17:03:56  jsb
 * 	NORMA_BOOT: master_device_port initialization change.
 * 	[91/05/13  16:51:18  jsb]
 * 
 * Revision 2.8  91/05/14  15:42:08  mrt
 * 	Correcting copyright
 * 
 * Revision 2.7  91/03/16  14:42:48  rpd
 * 	Changed net_rcv_msg_thread to net_thread.
 * 	[91/02/13            rpd]
 * 
 * Revision 2.6  91/02/05  17:08:54  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:27:56  mrt]
 * 
 * Revision 2.5  90/12/14  10:59:35  jsb
 * 	Added NORMA_BOOT support.
 * 	[90/12/13  21:06:19  jsb]
 * 
 * Revision 2.4  90/08/27  21:55:11  dbg
 * 	Removed call to cinit.
 * 	[90/07/09            dbg]
 * 
 * Revision 2.3  90/06/02  14:47:29  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  21:51:02  rpd]
 * 
 * Revision 2.2  89/09/08  11:23:33  dbg
 * 	Created.
 * 	[89/08/02            dbg]
 * 
 */
/*
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date: 	8/89
 *
 * 	Initialize device service as part of kernel task.
 */
#include <ipc/ipc_port.h>
#include <ipc/ipc_space.h>
#include <kern/task.h>

#include <device/device_types.h>
#include <device/device_port.h>



extern void	ds_init();
extern void	dev_lookup_init();
extern void	net_io_init();
extern void	device_pager_init();
extern void	chario_init(void);

extern void	io_done_thread();
extern void	net_thread();

ipc_port_t	master_device_port;

void
device_service_create()
{
	master_device_port = ipc_port_alloc_kernel();
	if (master_device_port == IP_NULL)
	    panic("can't allocate master device port");

	ds_init();
	dev_lookup_init();
	net_io_init();
	device_pager_init();
	chario_init();

	(void) kernel_thread(kernel_task, io_done_thread, 0);
	(void) kernel_thread(kernel_task, net_thread, 0);
}
