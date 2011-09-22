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
 * $Log:	mips_copyin.s,v $
 * Revision 2.12  92/02/19  15:09:21  elf
 * 	Changed #-style comments, for ANSI cpp.
 * 	[92/02/19  13:11:16  rpd]
 * 
 * Revision 2.11  91/11/12  11:17:02  rvb
 * 	Anti-sneaker test was too strict in copyin/copyout, would
 * 	not let you touch the very last byte of the user's space.
 * 	[91/10/19  08:42:19  af]
 * 
 * Revision 2.10  91/05/14  17:35:28  mrt
 * 	Correcting copyright
 * 
 * Revision 2.9  91/05/13  06:06:06  af
 * 	Added aligned_block_copy
 * 	[91/05/12  16:00:40  af]
 * 
 * Revision 2.8  91/02/05  17:49:19  mrt
 * 	Added author notices
 * 	[91/02/04  11:23:22  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:27:04  mrt]
 * 
 * Revision 2.7  91/01/08  15:50:23  rpd
 * 	Added copyinmsg, copyoutmsg.
 * 	Fixed copyin to check correctly for user space violations.
 * 	[90/12/05            rpd]
 * 
 * Revision 2.6  90/08/27  22:08:33  dbg
 * 	Security leak in copyin/copyout: checking the start address is
 * 	not enough,  the user and kernel space are contiguous.
 * 	[90/08/12            af]
 * 
 * Revision 2.5  90/08/07  22:29:32  rpd
 * 	Sleazy bugs with branch-delay instructions.  This was a tricky one to debug:
 * 	it would leave fast_thread_recover up for the first trap(), and just force
 * 	a function return. [This was the misterious duplicate "paging file ?"]
 * 
 * Revision 2.3.3.1  90/05/30  15:35:49  af
 * 	As part of the 3max support, move handling of buserrors
 * 	down in the first level handler.
 * 
 * Revision 2.3  90/01/22  23:07:19  af
 * 	Moved here check_memory()  Added KDB support functions for
 * 	munging with I/O space.
 * 	[90/01/20  17:07:44  af]
 * 
 * Revision 2.2  89/11/29  14:14:29  af
 * 	Created.
 * 	[89/10/12            af]
 */
/*
 *	File: mips_copyin.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Copy operations that require assembly coding
 *	because they use the thread recover technology.
 *	Besides copyin/copyout, kdb's bottom functions.
 */

#include <mach_kdb.h>

#include <mach/mips/asm.h>
#include <mach/kern_return.h>
#include <mips/thread.h>
#include <mips/mips_cpu.h>
#include <mips/mips_box.h>

#include <assym.s>

	.set	noreorder

/*
 *	Object:
 *		copyin				EXPORTED function
 *
 *		Copy bytes from user space to kernel space
 *
 *	Arguments:
 *		from				char *
 *		to				char *
 *		size				unsigned
 *
 *	Use the thread-recover technology and just call bcopy.
 */
#	define	FRAMESIZE 4*4+8
NESTED(copyin,	FRAMESIZE, zero)
	subu	sp,FRAMESIZE
	sw	ra,FRAMESIZE-4(sp)
	sw	s0,FRAMESIZE-8(sp)
	lw	s0,the_current_thread

	blez	a0,copy_error		/* sneaker */
	addu	v0,a0,a2
	subu	v0,1
	bltz	v0,copy_error

	la	t1,copy_error
	jal	bcopy
	sw	t1,THREAD_RECOVER(s0)

	b	copy_ok
	li	v0,KERN_SUCCESS

copy_error:
	li	v0,1
copy_ok:
	sw	zero,THREAD_RECOVER(s0)	
	lw	ra,FRAMESIZE-4(sp)
	lw	s0,FRAMESIZE-8(sp)
	j	ra
	addu	sp,FRAMESIZE
	END(copyin)

/*
 *	Object:
 *		copyout				EXPORTED function
 *
 *		Copy bytes from kernel space to user space
 *
 *	Arguments:
 *		from				char *
 *		to				char *
 *		size				unsigned
 *
 */
NESTED(copyout,	FRAMESIZE, zero)
	subu	sp,FRAMESIZE
	sw	ra,FRAMESIZE-4(sp)
	sw	s0,FRAMESIZE-8(sp)
	lw	s0,the_current_thread

	blez	a1,copy_error		/* sneaker */
	addu	v0,a1,a2
	subu	v0,1
	bltz	v0,copy_error

	la	t1,copy_error
	jal	bcopy
	sw	t1,THREAD_RECOVER(s0)

	b	copy_ok
	li	v0,KERN_SUCCESS
	END(copyout)


/*
 *	Object:
 *		copyinmsg			EXPORTED function
 *
 *		Copy bytes from user space to kernel space.
 *		For message buffers (integral ints).
 *
 *	Object:
 *		copyoutmsg			EXPORTED function
 *
 *		Copy bytes from kernel space to user space
 *		For message buffers (integral ints).
 *
 *	Arguments:
 *		from				char *
 *		to				char *
 *		size				unsigned
 *		Assumes size & 3 == 0 and size>>2 > 0.
 *
 *	Doesn't use the thread-recover technology.
 *	The trap handler is responsible for fixing up faults,
 *	redirecting us to copymsg_error.
 */

EXPORT(copymsg_start)
LEAF(copyinmsg)
	blez	a0,copymsg_error	/* sneaker */
	addu	v0,a0,a2
	blez	v0,copymsg_error

	/*
	 *	The write buffer on a pmax handles one store/six cycles.
	 *	On a 3max, this loop might be worth unrolling.
	 */

1:	lw	t0,0(a0)
	addu	a0,4
	sw	t0,0(a1)
	subu	a2,4
	bne	a2,zero,1b
	addu	a1,4

	j	ra
	li	v0,0
	END(copyinmsg)

LEAF(copyoutmsg)
	blez	a1,copymsg_error	/* sneaker */
	addu	v0,a1,a2
	blez	v0,copymsg_error

	/*
	 *	The write buffer on a pmax handles one store/six cycles.
	 *	On a 3max, this loop might be worth unrolling.
	 */

1:	lw	t0,0(a0)
	addu	a0,4
	sw	t0,0(a1)
	subu	a2,4
	bne	a2,zero,1b
	addu	a1,4

	j	ra
	li	v0,0
	END(copyoutmsg)
EXPORT(copymsg_end)

LEAF(copymsg_error)
	j	ra
	li	v0,1
	END(copymsg_error)


/*
 *	Object:
 *		aligned_block_copy		EXPORTED function
 *
 *		Copy bytes from word-aligned location
 *		to word-aligned location.
 *
 *	Arguments:
 *		from				int *
 *		to				int *
 *		size				unsigned
 *		If {size & 3 != 0} will overcopy
 *
 *	Unrolled, hyperoptimized copy function.  Looks like
 *	even the choice of temporary registers matters.
 *	Note that the count is in bytes, but the pointers *must*
 *	be aligned or else.
 *
 *	Performance issues:
 *	I wrote this specifically for use on the 3max to copy
 *	to the SCSI buffer, but it is more general than that.
 *	Copying 8k from general memory to general memory
 *	takes 198usecs, or 41.4 Mb/sec.
 */
	.set	noat

LEAF(aligned_block_copy)
	/* we'll do it 8*4 bytes at a time */
	li	AT,-32
	and	t0,a2,AT	/* bytecount for loop */
	addu	a3,a0,t0	/* upto */
	sltu	AT,a0,a3
	beq	AT,zero,copyrest
	subu	a2,t0

	/* loop body */
cp:
	lw	v0,0(a0)
	lw	v1,4(a0)
	lw	t0,8(a0)
	lw	t1,12(a0)
	addu	a0,32
	sw	v0,0(a1)
	sw	v1,4(a1)
	sw	t0,8(a1)
	sw	t1,12(a1)
	lw	t1,-4(a0)
	lw	t0,-8(a0)
	lw	v1,-12(a0)
	lw	v0,-16(a0)
	addu	a1,32
	sw	t1,-4(a1)
	sw	t0,-8(a1)
	sw	v1,-12(a1)
	bne	a0,a3,cp
	sw	v0,-16(a1)

copyrest:
	blez	a2,done
	addu	a1,4
	lw	t1,0(a0)
	addu	a0,4
	sw	t1,-4(a1)
	b	copyrest
	subu	a2,4

done:
	j	ra
	nop
	END(aligned_block_copy)

	.set	AT

/*
 *	Object:
 *		check_memory		EXPORTED function
 *
 *		Check whether there is memory at 'addr'
 *
 *	Arguments:
 *		addr				int *
 *		store				int
 *
 *	Does not save the status register, so we better be
 *	at splhigh() or else.  If "store" is set, store a
 *	zero at addr before reading (might be needed on
 *	certain memory boards to force ecc recomputation)
 */
LEAF(check_memory)
	la	v0,check_memory_recover
	sw	v0,fast_thread_recover
	beq	a1,zero,1f
	nop
	sw	zero,0(a0)		/* might need ECC */
1:	lb	a1,0(a0)
	b	1f
	move	v0,zero
XLEAF(check_memory_recover)
	li	v0,1
1:	sw	zero,fast_thread_recover
	j	ra
	nop
	END(check_memory)

#if	MACH_KDB
/*
 *	Object:
 *		kdb_getiomem			EXPORTED function
 *
 *		Copy a word from kernel I/O space to memory
 *
 *	Arguments:
 *		from				char *
 *
 */
LEAF(kdb_getiomem)
	la	v0,kdb_iomem_recover
	sw	v0,fast_thread_recover
	andi	t0,a0,3
	bne	t0,zero,g_by_shorts
	andi	t0,1
	lw	v0,0(a0)
	b	kdb_iomem_ok
	nop
g_by_shorts:
	bne	t0,zero,g_by_bytes
	nop
	lhu	v0,0(a0)
	lhu	t1,2(a0)
#if	BYTE_MSF
	sll	v0,16
#else	BYTE_MSF
	nop
	sll	t1,16
#endif	BYTE_MSF
	b	kdb_iomem_ok
	or	v0,t1

g_by_bytes:
	lbu	v0,0(a0)
	lbu	t1,1(a0)
	lbu	t2,2(a0)
	lbu	t3,3(a0)
#if	BYTE_MSF
	sll	v0,24
	sll	t1,16
	sll	t2,8
#else	BYTE_MSF
	sll	t1,8
	sll	t2,16
	sll	t3,24
#endif	BYTE_MSF
	or	v0,t1
	or	v0,t2
	b	kdb_iomem_ok
	or	v0,t3

kdb_iomem_recover:
	li	v0,-1
kdb_iomem_ok:
	sw	zero,fast_thread_recover
	j	ra
	nop
	END(kdb_getiomem)
/*
 *	Object:
 *		kdb_putiomem			EXPORTED function
 *
 *		Copy a word from memory to kernel I/O space
 *
 *	Arguments:
 *		to				char *
 *		value				unsigned
 *
 */
LEAF(kdb_putiomem)
	la	v0,kdb_iomem_recover
	sw	v0,fast_thread_recover
	andi	t0,a0,3
	bne	t0,zero,byshorts
	andi	t0,1
	sw	a1,0(a0)
	b	kdb_iomem_ok
	nop
byshorts:
	bne	t0,zero,bybytes
	srl	t0,a1,16
#if	BYTE_MSF
	sh	a1,2(a0)
	sh	t0,0(a0)
#else	BYTE_MSF
	sh	a1,0(a0)
	sh	t0,2(a0)
#endif	BYTE_MSF
	b	kdb_iomem_ok
	nop

bybytes:
#if	BYTE_MSF
	sb	a1,3(a0)
	srl	a1,8
	sb	a1,2(a0)
	sb	t0,1(a0)
	srl	t0,8
	sb	t0,0(a0)
#else	BYTE_MSF
	sb	a1,0(a0)
	srl	a1,8
	sb	a1,1(a0)
	sb	t0,2(a0)
	srl	t0,8
	sb	t0,3(a0)
#endif	BYTE_MSF
	b	kdb_iomem_ok
	nop
	END(kdb_putiomem)
#endif	MACH_KDB
