/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * Copyright 1988, 1989, 1990, 1991 by Intel Corporation,
 * Santa Clara, California.
 * 
 *                          All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and that
 * both the copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Intel not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 * 
 * INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
 * SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN ACTION OF CONTRACT, NEGLIGENCE, OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */
/*
 * HISTORY
 * $Log:	md.c,v $
 * Revision 2.4  93/01/14  17:32:08  danner
 * 	Proper spl typing.
 * 	[92/12/10  17:54:13  af]
 * 
 * Revision 2.3  92/02/23  22:43:18  elf
 * 	Added md_get_status().
 * 	[92/02/22  19:56:15  af]
 * 
 * Revision 2.2  91/12/10  16:29:59  jsb
 * 	New files from Intel
 * 	[91/12/10  16:11:01  jsb]
 * 
 */ 
/*
 * from prp, SSD Intel
 */
 
#ifdef	MACH_KERNEL

#include <sys/types.h>
#include <vm/vm_kern.h>
#include <device/buf.h>
#include <device/errno.h>

#include <md.h>

#define READ_ONLY 0
#define SECSIZE 512

unsigned char *md_address;
unsigned long md_size = 0;

#define	DEBUG	0
#define	dprintf	if (DEBUG) printf

int mdstrategy();

mdopen(dev, flags)
int dev;
int flags;
{
	unsigned char n;
	int	errcode = 0;
	char 	c;

	dprintf("mdopen md address 0x%08X size 0x%08X\n", md_address, md_size);
	if (md_size == 0)
		errcode = ENXIO;
	return(errcode);
}


mdclose(dev)
{

	dprintf("mdclose\n");
	return;
}


/*
 *	No need to limit IO size to 4096 bytes.
 */
mdread(dev, ior)
dev_t		dev;
io_req_t	ior;
{
	dprintf("mdread\n");
	return(block_io(mdstrategy, minphys, ior));
}

mdwrite(dev, ior)
dev_t		dev;
io_req_t	ior;
{
	dprintf("mdwrite\n");
	return(block_io(mdstrategy, minphys, ior));
}


mdstrategy(bp)
struct	buf	*bp;
{
	struct	buf	*ptr;
	spl_t old_priority;
	unsigned int blkaddr;

	dprintf("mdstrategy %c blk %d len %d buf 0x%08X\n",
		(bp->b_flags & B_READ)? 'R' : 'W',
		bp->b_blkno, bp->b_bcount, bp->b_un.b_addr);

	if (bp->b_bcount == 0) {
		biodone(bp);
		return;
	}

	if ( !(bp->b_flags & B_READ) &&
	     (READ_ONLY)
	   )
	{
		bp->b_flags = B_ERROR;
		bp->b_error = ENXIO;
		biodone(bp);
		return;
	}

	/* if request is off the end or trying to write last block on out */

	blkaddr = SECSIZE*bp->b_blkno;
	if ( (blkaddr >  md_size) ||
	     (blkaddr == md_size & !(bp->b_flags & B_READ))) {
		bp->b_flags = B_ERROR;
		bp->b_error = ENXIO;
		biodone(bp);
		return;
	}

	if (blkaddr == md_size) {
	/* indicate (read) EOF by setting b_resid to b_bcount on last block */ 
		bp->b_resid = bp->b_bcount;
		biodone(bp);
		return;
	}


	old_priority = spl5();

	if (bp->b_flags & B_READ) {
		bcopy(	md_address + blkaddr,
			bp->b_un.b_addr,
			bp->b_bcount);
	} else {
		bcopy(	bp->b_un.b_addr,
			md_address + blkaddr,
			bp->b_bcount);
	}
	bp->b_resid = 0;
	biodone(bp);

	splx(old_priority);
}


mdsize()
{
	printf("mdsize()	-- not implemented\n");
}

mddump()
{
	printf("mddump()	-- not implemented\n");
}

md_get_status(dev, flavor, status, count)
	dev_t	dev;
	int	*status, *count;
{
	if (flavor == DEV_GET_SIZE) {
		status[DEV_GET_SIZE_DEVICE_SIZE] = md_size * SECSIZE;
		status[DEV_GET_SIZE_RECORD_SIZE] = SECSIZE;
		*count = DEV_GET_SIZE_COUNT;
		
		return D_SUCCESS;
	} else return D_INVALID_OPERATION;
}

#endif	MACH_KERNEL
