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
 * $Log:	blkio.c,v $
 * Revision 2.12  93/03/09  10:54:01  danner
 * 	block_io_mmap() must return a vm_offset_t.
 * 	[93/03/07            af]
 * 
 * Revision 2.11  92/08/03  17:32:43  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.10  92/05/21  17:08:27  jfriedl
 * 	Cleanup to quiet gcc warnings.
 * 	Function args to block_io are void.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.9  91/08/28  11:11:05  jsb
 * 	From rpd: documented what (*max_count)() does in block_io.
 * 	[91/08/22  15:30:18  jsb]
 * 
 * 	Unlimited size is for write only.
 * 	[91/08/12  17:44:31  dlb]
 * 
 * 	Support synchronous waits for writes with vm continuations.
 * 	Remove limit on size of operations.
 * 	[91/08/12  17:22:59  dlb]
 * 
 * Revision 2.8  91/07/30  15:45:38  rvb
 * 	From rpd: temp(!!) jack up MAXPHYS to 256K to make newfs
 * 	work.  rpd is working on having blkio loop, doing NBYTES
 * 	per interation, rather than having a fixed max size.
 * 	[91/07/30            rvb]
 * 
 * Revision 2.7  91/05/14  15:38:36  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/05/13  06:02:15  af
 * 	Do not trim io_count to DEV_BSIZE, we know how to cope
 * 	and this is used for tapes also [which might be block
 * 	devices or not, depends].
 * 	[91/05/12  15:48:48  af]
 * 
 * Revision 2.5  91/02/05  17:07:46  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:26:03  mrt]
 * 
 * Revision 2.4  90/08/27  21:54:15  dbg
 * 	Add disk sort routine.
 * 	[90/06/20            dbg]
 * 
 * Revision 2.3  89/09/08  11:22:56  dbg
 * 	Converted to run in kernel task.
 * 	[89/07/27            dbg]
 * 
 */
/*
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date: 	7/89
 *
 * 	Block IO driven from generic kernel IO interface.
 */
#include <mach/kern_return.h>

#include <device/param.h>
#include <device/device_types.h>
#include <device/io_req.h>
#include <device/ds_routines.h>



io_return_t block_io(strat, max_count, ior)
	void			(*strat)();
	void			(*max_count)();
	register io_req_t	ior;
{
	register kern_return_t	rc;
	boolean_t		wait = FALSE;

	/*
	 * Make sure the size is not too large by letting max_count
	 * change io_count.  If we are doing a write, then io_alloc_size
	 * preserves the original io_count.
	 */
	(*max_count)(ior);

	/*
	 * If reading, allocate memory.  If writing, wire
	 * down the incoming memory.
	 */
	if (ior->io_op & IO_READ)
	    rc = device_read_alloc(ior, (vm_size_t)ior->io_count);
	else
	    rc = device_write_get(ior, &wait);

	if (rc != KERN_SUCCESS)
	    return (rc);

	/*
	 * Queue the operation for the device.
	 */
	(*strat)(ior);

	/*
	 * The io is now queued.  Wait for it if needed.
	 */
	if (wait) {
		iowait(ior);
		return(D_SUCCESS);
	}

	return (D_IO_QUEUED);
}

/*
 * 'standard' max_count routine.  VM continuations mean that this
 * code can cope with arbitrarily-sized write operations (they won't be
 * atomic, but any caller that cares will do the op synchronously).
 */
#define MAX_PHYS        (256 * 1024)

void minphys(ior)
	register io_req_t	ior;
{
	if ((ior->io_op & (IO_WRITE | IO_READ | IO_OPEN)) == IO_WRITE)
	    return;

        if (ior->io_count > MAX_PHYS)
            ior->io_count = MAX_PHYS;
}

/*
 * Dummy routine placed in device switch entries to indicate that
 * block device may be mapped.
 */
vm_offset_t block_io_mmap()
{
	return (0);
}

/*
 * Disk sort routine.
 *
 * We order the disk request chain so that the disk head will sweep
 * back and forth across the disk.  The chain is divided into two
 * pieces, with requests ordered in opposite directions.  Assume that
 * the first part of the chain holds increasing cylinder numbers.
 * If a new request has a higher cylinder number than the head of
 * the chain, the disk head has not yet reached it; the new request
 * can go in the first part of the chain.  If the new request has
 * a lower cylinder number, the disk head has already passed it and
 * must catch it on the way back; so the new request goes in the
 * second (descending) part of the chain.
 * When all of the requests in the ascending portion are filled,
 * the descending chain becomes the first chain, and requests above
 * the first now go in the second part of the chain (ascending).
 */

#define	io_cylinder	io_residual
				/* Disk drivers put cylinder here */
#define	h_head		io_next
#define	h_tail		io_prev
				/* IORs are chained here */

void disksort(head, ior)
	io_req_t	head;	/* (sort of) */
	io_req_t	ior;
{
	register int		cylinder = ior->io_cylinder;
	register io_req_t	next, prev;

	next = head->h_head;
	if (next == 0) {
	    head->h_head = ior;
	    head->h_tail = ior;
	    ior->io_next = 0;
	    return;
	}

	do {
	    prev = next;
	    next = prev->io_next;
	} while (next != 0 && prev->io_cylinder == next->io_cylinder);

	if (next == 0) {
	    prev->io_next = ior;
	    head->h_tail = ior;
	    ior->io_next = 0;
	    return;
	}

	if (prev->io_cylinder < next->io_cylinder) {
	    /*
	     * Ascending list first.
	     */
	    if (prev->io_cylinder <= cylinder) {
		/*
		 * Insert in ascending list.
		 */
		while (next != 0 &&
			next->io_cylinder <= cylinder &&
			prev->io_cylinder <= next->io_cylinder)
		{
		    prev = next;
		    next = prev->io_next;
		}
	    }
	    else {
		/*
		 * Insert in descending list
		 */
		do {
		    prev = next;
		    next = prev->io_next;
		} while (next != 0 &&
			prev->io_cylinder <= next->io_cylinder);

		while (next != 0 &&
			next->io_cylinder >= cylinder)
		{
		    prev = next;
		    next = prev->io_next;
		}
	    }
	}
	else {
	    /*
	     * Descending first.
	     */
	    if (prev->io_cylinder >= cylinder) {
		/*
		 * Insert in descending list.
		 */
		while (next != 0 &&
			next->io_cylinder >= cylinder &&
			prev->io_cylinder >= next->io_cylinder)
		{
		    prev = next;
		    next = prev->io_next;
		}
	    }
	    else {
		/*
		 * Insert in ascending list
		 */
		do {
		    prev = next;
		    next = prev->io_next;
		} while (next != 0 &&
			prev->io_cylinder >= next->io_cylinder);
		while (next != 0 &&
			next->io_cylinder <= cylinder)
		{
		    prev = next;
		    next = prev->io_next;
		}
	    }
	}
	/*
	 * Insert between prev and next.
	 */
	prev->io_next = ior;
	ior->io_next = next;
	if (next == 0) {
	    /* At tail of list. */
	    head->h_tail = ior;
	}
}

