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
 * $Log:	ipc_net.h,v $
 * Revision 2.4  93/05/15  20:00:59  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.3  92/03/10  16:27:58  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:49:55  jsb]
 * 
 * Revision 2.2.2.1  92/01/21  21:51:40  jsb
 * 	Added file/author/date comment.
 * 	[92/01/21  19:44:13  jsb]
 * 
 * 	Conditionalized NETIPC_CHECKSUM on NORMA_ETHER. Removed MAXVEC
 * 	definition. Removed lint-inspired includes of ipc_{pset,space}.h.
 * 	[92/01/16  22:09:32  jsb]
 * 
 * 	Added NETIPC_CHECKSUM conditional and netipc_hdr checksum field.
 * 	[92/01/14  21:32:29  jsb]
 * 
 * 	Changed ctl_status type.
 * 	[92/01/13  19:34:52  jsb]
 * 
 * 	De-linted.
 * 	[92/01/13  10:14:47  jsb]
 * 
 * 	Moved protocol dependent definitions into norma/ipc_unreliable.c.
 * 	Added ctl_status to netipc_hdr (which shouldn't be exported anyway).
 * 	[92/01/11  17:07:26  jsb]
 * 
 * 	Old contents moved to norma/ipc_netvec.h.
 * 	Now contains definitions shared by files split from norma/ipc_net.c.
 * 	[92/01/10  20:38:53  jsb]
 * 
 */ 
/*
 *	File:	norma/ipc_net.h
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Definitions for reliable delivery and flow control for NORMA_IPC.
 */

#include <norma_ether.h>

#include <machine/machspl.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>
#include <mach/vm_param.h>
#include <kern/assert.h>
#include <kern/lock.h>
#include <ipc/ipc_port.h>
#include <ipc/ipc_kmsg.h>
#include <norma/ipc_node.h>
#include <norma/ipc_netvec.h>
#include <sys/varargs.h>

#if	NORMA_ETHER
#define	NETIPC_CHECKSUM	1
#else
/*
 * XXX
 * The checksumming code is not likely to work on the ipsc until
 * the netipc_recv routines for i386 and i860 are changed to set
 * the vector sizes to reflect the size of the incoming data.
 */
#define	NETIPC_CHECKSUM	0
#endif

#define	NETIPC_TYPE_INVALID	0x00000000L
#define	NETIPC_TYPE_KMSG	0xabcdef00L
#define	NETIPC_TYPE_PAGE	0xabcdef01L
#define	NETIPC_TYPE_CTL		0xabcdef02L

struct pginfo {
	unsigned long	pg_msgh_offset;
	boolean_t	pg_page_first;
	boolean_t	pg_page_last;
	boolean_t	pg_copy_last;
	unsigned long	pg_copy_offset;
	unsigned long	pg_copy_size;
};

struct netipc_hdr {
#if	NETIPC_CHECKSUM
	unsigned long	checksum;
#endif	NETIPC_CHECKSUM
	unsigned long	type;
	unsigned long	seqid;
	struct pginfo	pg;
	unsigned long	remote;
	unsigned long	ctl;
	unsigned long	ctl_seqid;
	kern_return_t	ctl_status;
	unsigned long	ctl_data;
	unsigned long	incarnation;
};

/*
 * Some devices want virtual addresses, others want physical addresses.
 *
 * KVTODEV:	Kernel virtual address to device address
 * DEVTOKV:	Device address to kernel virtual address
 * VPTODEV:	vm_page_t to device address
 *
 * XXX These should be defined somewhere else.
 */
#if	NORMA_ETHER	|| i860
/*
 * Device uses virtual addresses.
 */
#define	KVTODEV(addr)	((unsigned long) (addr))
#define	DEVTOKV(addr)	((unsigned long) (addr))
#define	VPTODEV(m)	(VPTOKV(m))
#else	/*NORMA_ETHER*/
/*
 * Device uses physical addresses.
 */
#define	KVTODEV(addr)	((unsigned long) kvtophys(addr))
#define	DEVTOKV(addr)	((unsigned long) phystokv(addr))
#define	VPTODEV(m)	((m)->phys_addr)
#endif	/*NORMA_ETHER*/

#define	VPTOKV(m)	phystokv((m)->phys_addr)

extern void netipc_thread_lock();
extern void netipc_thread_unlock();
extern void netipc_copy_ungrab();
