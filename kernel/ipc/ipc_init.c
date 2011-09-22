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
 * $Log:	ipc_init.c,v $
 * Revision 2.14  92/08/03  17:34:17  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.13  92/05/21  17:10:11  jfriedl
 * 	tried prototypes.
 * 	[92/05/20            jfriedl]
 * 
 * Revision 2.12  91/12/10  13:25:41  jsb
 * 	Removed reference counting bug workaround.
 * 	[91/12/10  11:17:21  jsb]
 * 
 * Revision 2.11  91/11/19  09:54:12  rvb
 * 	Joe's tar.3 changes installed by Jeffrey Heller.
 * 	[91/11/19            rvb]
 * 
 * Revision 2.10  91/08/03  18:18:12  jsb
 * 	Removed call to ipc_clport_init.
 * 	[91/07/24  22:11:04  jsb]
 * 
 * Revision 2.9  91/06/17  15:46:00  jsb
 * 	Renamed NORMA conditionals.
 * 	[91/06/17  10:45:48  jsb]
 * 
 * Revision 2.8  91/06/06  17:05:46  jsb
 * 	Added call to ipc_clport_init.
 * 	[91/05/13  17:17:10  jsb]
 * 
 * Revision 2.7  91/05/14  16:32:33  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/02/05  17:21:37  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:45:02  mrt]
 * 
 * Revision 2.5  91/01/08  15:13:40  rpd
 * 	Changed zchange calls to make the IPC zones non-collectable.
 * 	[90/12/29            rpd]
 * 
 * Revision 2.4  90/12/20  16:38:41  jeffreyh
 * 	Changes to zchange to account for new collectable field. Made all
 * 	ipc zones collectable.
 * 	[90/12/11            jeffreyh]
 * 
 * Revision 2.3  90/09/28  16:54:44  jsb
 * 	Added NORMA_IPC support.
 * 	[90/09/28  14:02:05  jsb]
 * 
 * Revision 2.2  90/06/02  14:49:55  rpd
 * 	Created for new IPC.
 * 	[90/03/26  20:55:13  rpd]
 * 
 */
/*
 *	File:	ipc/ipc_init.c
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Functions to initialize the IPC system.
 */

#include <mach/kern_return.h>
#include <kern/mach_param.h>
#include <kern/ipc_host.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <ipc/ipc_entry.h>
#include <ipc/ipc_space.h>
#include <ipc/ipc_object.h>
#include <ipc/ipc_port.h>
#include <ipc/ipc_pset.h>
#include <ipc/ipc_marequest.h>
#include <ipc/ipc_notify.h>
#include <ipc/ipc_kmsg.h>
#include <ipc/ipc_hash.h>
#include <ipc/ipc_init.h>



vm_map_t ipc_kernel_map;
vm_size_t ipc_kernel_map_size = 1024 * 1024;

int ipc_space_max = SPACE_MAX;
int ipc_tree_entry_max = ITE_MAX;
int ipc_port_max = PORT_MAX;
int ipc_pset_max = SET_MAX;

/*
 *	Routine:	ipc_bootstrap
 *	Purpose:
 *		Initialization needed before the kernel task
 *		can be created.
 */

void
ipc_bootstrap()
{
	kern_return_t kr;

	ipc_port_multiple_lock_init();

	ipc_port_timestamp_lock_init();
	ipc_port_timestamp_data = 0;

	/* all IPC zones should be exhaustible */

	ipc_space_zone = zinit(sizeof(struct ipc_space),
			       ipc_space_max * sizeof(struct ipc_space),
			       sizeof(struct ipc_space),
			       FALSE, "ipc spaces");
	/* make it exhaustible */
	zchange(ipc_space_zone, FALSE, FALSE, TRUE, FALSE);

	ipc_tree_entry_zone =
		zinit(sizeof(struct ipc_tree_entry),
			ipc_tree_entry_max * sizeof(struct ipc_tree_entry),
			sizeof(struct ipc_tree_entry),
			FALSE, "ipc tree entries");
	/* make it exhaustible */
	zchange(ipc_tree_entry_zone, FALSE, FALSE, TRUE, FALSE);

	ipc_object_zones[IOT_PORT] =
		zinit(sizeof(struct ipc_port),
		      ipc_port_max * sizeof(struct ipc_port),
		      sizeof(struct ipc_port),
		      FALSE, "ipc ports");
	/* make it exhaustible */
	zchange(ipc_object_zones[IOT_PORT], FALSE, FALSE, TRUE, FALSE);

	ipc_object_zones[IOT_PORT_SET] =
		zinit(sizeof(struct ipc_pset),
		      ipc_pset_max * sizeof(struct ipc_pset),
		      sizeof(struct ipc_pset),
		      FALSE, "ipc port sets");
	/* make it exhaustible */
	zchange(ipc_object_zones[IOT_PORT_SET], FALSE, FALSE, TRUE, FALSE);

	/* create special spaces */

	kr = ipc_space_create_special(&ipc_space_kernel);
	assert(kr == KERN_SUCCESS);

	kr = ipc_space_create_special(&ipc_space_reply);
	assert(kr == KERN_SUCCESS);

#if	NORMA_IPC
	kr = ipc_space_create_special(&ipc_space_remote);
	assert(kr == KERN_SUCCESS);
#endif	NORMA_IPC

	/* initialize modules with hidden data structures */

	ipc_table_init();
	ipc_notify_init();
	ipc_hash_init();
	ipc_marequest_init();
}

/*
 *	Routine:	ipc_init
 *	Purpose:
 *		Final initialization of the IPC system.
 */

void
ipc_init()
{
	vm_offset_t min, max;

	ipc_kernel_map = kmem_suballoc(kernel_map, &min, &max,
				       ipc_kernel_map_size, TRUE);

	ipc_host_init();
}
