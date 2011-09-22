/* 
 * Mach Operating System
 * Copyright (c) 1993,1991,1990,1989,1988 Carnegie Mellon University
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
 * $Log:	subrs.c,v $
 * Revision 2.15  93/03/09  10:54:20  danner
 * 	Removed string routines, and the Berkeley copyright that came
 * 	with them.  The new ones we wrote now live in kern/strings.[ch].
 * 	[93/03/06            af]
 * 
 * Revision 2.14  93/01/14  17:27:17  danner
 * 	64bit cleanup.
 * 	[92/11/30            af]
 * 
 * Revision 2.13  92/08/03  17:33:57  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.12  92/05/21  17:09:49  jfriedl
 * 	Cleanup to quiet gcc warnings. Replaced strlen with a faster one.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.11  91/09/12  16:37:39  bohman
 * 	Added strlen() (BSD version).
 * 	[91/09/11  17:07:25  bohman]
 * 
 * Revision 2.10  91/08/24  11:56:04  af
 * 	Use BSD string functions now, which of course come with a
 * 	copyright.  Also, undef them in case some smarty..
 * 	[91/08/22            af]
 * 
 * Revision 2.9  91/07/31  17:34:27  dbg
 * 	Add strcpy.
 * 	[91/07/30  16:47:37  dbg]
 * 
 * Revision 2.8  91/05/14  16:01:30  mrt
 * 	Correcting copyright
 * 
 * Revision 2.7  91/03/16  14:43:34  rpd
 * 	Updated for new kmem_alloc interface.
 * 	[91/03/03            rpd]
 * 
 * Revision 2.6  91/02/05  17:10:17  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:30:39  mrt]
 * 
 * Revision 2.5  91/01/08  15:10:02  rpd
 * 	Added continuation argument to thread_block.
 * 	[90/12/08            rpd]
 * 
 * Revision 2.4  90/05/03  15:19:27  dbg
 * 	Add compatibility routines for BSD-compatible device drivers:
 * 	sleep, wakeup, geteblk, brelse.
 * 	[90/03/14            dbg]
 * 
 * Revision 2.3  90/01/11  11:42:31  dbg
 * 	De-linted.
 * 	[89/12/15            dbg]
 * 
 * 	Add lock initialization in if_init_queues.
 * 	[89/11/30            dbg]
 * 
 * Revision 2.2  89/11/29  14:08:58  af
 * 	RCS-ed, added strncmp (needed for scsi label comparisons).
 * 	[89/10/28            af]
 * 
 */
/*
 * Random device subroutines and stubs.
 */

#include <vm/vm_kern.h>
#include <device/buf.h>
#include <device/if_hdr.h>
#include <device/if_ether.h>



/*
 * Print out disk name and block number for hard disk errors.
 */
void harderr(bp, cp)
	struct buf *bp;
	char *	cp;
{
	printf("%s%d%c: hard error sn%d ",
	       cp,
	       minor(bp->b_dev) >> 3,
	       'a' + (minor(bp->b_dev) & 0x7),
	       bp->b_blkno);
}

/*
 * Ethernet support routines.
 */
u_char	etherbroadcastaddr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

/*
 * Convert Ethernet address to printable (loggable) representation.
 */
char *
ether_sprintf(ap)
	register u_char *ap;
{
	register i;
	static char etherbuf[18];
	register char *cp = etherbuf;
	static char digits[] = "0123456789abcdef";

	for (i = 0; i < 6; i++) {
		*cp++ = digits[*ap >> 4];
		*cp++ = digits[*ap++ & 0xf];
		*cp++ = ':';
	}
	*--cp = 0;
	return (etherbuf);
}

/*
 * Initialize send and receive queues on an interface.
 */
void if_init_queues(ifp)
	register struct ifnet *ifp;
{
	IFQ_INIT(&ifp->if_snd);
	queue_init(&ifp->if_rcv_port_list);
	simple_lock_init(&ifp->if_rcv_port_list_lock);
}


/*
 * Compatibility with BSD device drivers.
 */
void sleep(channel, priority)
	vm_offset_t	channel;
	int		priority;
{
	assert_wait((event_t) channel, FALSE);	/* not interruptible XXX */
	thread_block((void (*)()) 0);
}

void wakeup(channel)
	vm_offset_t	channel;
{
	thread_wakeup((event_t) channel);
}

struct buf *
geteblk(size)
	int	size;
{
	register io_req_t	ior;

	io_req_alloc(ior, 0);
	ior->io_device = (device_t)0;
	ior->io_unit = 0;
	ior->io_op = 0;
	ior->io_mode = 0;
	ior->io_recnum = 0;
	ior->io_count = size;
	ior->io_residual = 0;
	ior->io_error = 0;

	size = round_page(size);
	ior->io_alloc_size = size;
	if (kmem_alloc(kernel_map, (vm_offset_t *)&ior->io_data, size)
		!= KERN_SUCCESS)
		    panic("geteblk");

	return (ior);
}

void brelse(bp)
	struct buf *bp;
{
	register io_req_t	ior = bp;

	(void) vm_deallocate(kernel_map,
			(vm_offset_t) ior->io_data,
			(vm_size_t) ior->io_alloc_size);
	io_req_free(ior);
}
