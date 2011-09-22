/* 
 * Mach Operating System
 * Copyright (c) 1993,1992 Carnegie Mellon University
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
 * $Log:	alpha_copyin.s,v $
 * Revision 2.4  93/05/20  21:01:52  mrt
 * 	Changed use of zero to ra in call to NESTED.
 * 	[93/05/18            mrt]
 * 
 * Revision 2.3  93/01/19  08:57:31  danner
 * 	Added fast aligned_block_copy.
 * 	[93/01/19            af]
 * 
 * Revision 2.2  93/01/14  17:10:58  danner
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:10:27  af]
 * 
 * 	Created.
 * 	[92/12/10  14:48:32  af]
 */
/*
 *	File: alpha_copyin.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Copy operations that require assembly coding
 *	because they use the thread recover technology.
 *	Besides copyin/copyout, kdb's bottom functions.
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 */

#include <cpus.h>
#include <mach_kdb.h>

#include <mach/alpha/asm.h>
#include <mach/kern_return.h>
#include <alpha/thread.h>
#include <alpha/alpha_cpu.h>
#include <mach/alpha/alpha_instruction.h>

#include <assym.s>

	.set	noreorder

/* BUGFIX: the assemblers do not seem to grok things like
	lda	$1	,0x000003fe00000000
   so we must put a bit too much knowledge in here about the KUSEG */

#if	__GNU_AS__
#define	load_KUSEG_END(reg)		\
	lda	reg,0x3fe;	\
	sll	reg,32,reg
#else
#define	load_KUSEG_END(reg)		\
	lda	reg,(KUSEG_END>>32);	\
	sll	reg,32,reg
#endif

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
#	define	FRAMESIZE (4*8+16)
NESTED(copyin, 3, FRAMESIZE, ra, 0, 0)
	ldgp	gp,0(pv)
	lda	sp,-FRAMESIZE(sp)
	stq	ra,FRAMESIZE-8(sp)
	stq	s0,FRAMESIZE-16(sp)
#if	(NCPUS>1)
	call_pal op_mfpr_whami
#else
	mov	zero,v0
#endif
	lda	s0,active_threads
	s8addq	v0,s0,s0
	ldq	s0,0(s0)

	load_KUSEG_END(t0)
	cmpult	a0,t0,t1
	beq	t1,copy_error		/* sneaker */
	addq	a0,a2,v0
	subq	v0,1,v0
	cmpult	v0,t0,t1
	beq	t1,copy_error		/* sneaker */

	lda	t1,copy_error
	stq	t1,THREAD_RECOVER(s0)
	CALL(bcopy)

	addq	zero,KERN_SUCCESS,v0
	br	zero,copy_ok

copy_error:
	addq	zero,1,v0
copy_ok:
	stq	zero,THREAD_RECOVER(s0)	
	ldq	ra,FRAMESIZE-8(sp)
	ldq	s0,FRAMESIZE-16(sp)
	lda	sp,FRAMESIZE(sp)
	RET
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
NESTED(copyout, 3, FRAMESIZE, ra, 0, 0)
	ldgp	gp,0(pv)
	lda	sp,-FRAMESIZE(sp)
	stq	ra,FRAMESIZE-8(sp)
	stq	s0,FRAMESIZE-16(sp)
#if	(NCPUS>1)
	call_pal op_mfpr_whami
#else
	mov	zero,v0
#endif
	lda	s0,active_threads
	s8addq	v0,s0,s0
	ldq	s0,0(s0)

	load_KUSEG_END(t0)
	cmpult	a1,t0,t1
	beq	t1,copy_error		/* sneaker */
	addq	a1,a2,v0
	subq	v0,1,v0
	cmpult	v0,t0,t1
	beq	t1,copy_error		/* sneaker */

	lda	t1,copy_error
	stq	t1,THREAD_RECOVER(s0)
	CALL(bcopy)

	addq	zero,KERN_SUCCESS,v0
	br	zero,copy_ok
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
LEAF(copyinmsg,3)
#if 0
	blez	a0,copymsg_error	/* sneaker */
	addq	v0,a0,a2
	blez	v0,copymsg_error
#endif
	/*
	 *	The write buffer on a pmax handles one store/six cycles.
	 *	On a 3max, this loop might be worth unrolling.
	 *	ON ALPHA I NEED TO MAKE A SECOND PASS OVER ALL THINGS
	 */

1:	ldl	t0,0(a0)
	addq	a0,4,a0
	stl	t0,0(a1)
	subq	a2,4,a2
	addq	a1,4,a1
	bne	a2,1b

	mov	zero,v0
	RET
	END(copyinmsg)

LEAF(copyoutmsg,3)
#if 0
	blez	a1,copymsg_error	/* sneaker */
	addq	v0,a1,a2
	blez	v0,copymsg_error
#endif
	/*
	 *	The write buffer on a pmax handles one store/six cycles.
	 *	On a 3max, this loop might be worth unrolling.
	 */

1:	ldl	t0,0(a0)
	addq	a0,4,a0
	stl	t0,0(a1)
	subq	a2,4,a2
	addq	a1,4,a1
	bne	a2,1b

	mov	zero,v0
	RET
	END(copyoutmsg)
EXPORT(copymsg_end)

LEAF(copymsg_error,0)
	addq	zero,1,v0
	RET
	END(copymsg_error)

/*
 *	Object:
 *		aligned_block_copy		EXPORTED function
 *
 *		Copy bytes from word-aligned location
 *		to word-aligned location.
 *
 *	Arguments:
 *		from				long *
 *		to				long *
 *		size				long
 *
 *	Unrolled, hyperoptimized page-copy function.
 *	Addresses must be identically aligned, preferably
 *	on a cache line boundary.
 *	Count is a multiple of CHUNK_SIZE or else we overcopy.
 *
 *	Performance issues:
 *	I wrote this to see how fast a page can be copied.
 *	Copying 8k from cache to cache runs at 293.6 Mb/sec.
 */
#define	CACHE_LINE_SIZE	32
#define CHUNK_SIZE	(CACHE_LINE_SIZE*4)
	.align	4

	.set	noreorder

LEAF(aligned_block_copy,3)

	/* fetch 4 cache lines */
	ldq_u	t2,0(a0)
	ldq_u	t3,(CACHE_LINE_SIZE)(a0)
	ldq_u	t4,(CACHE_LINE_SIZE*2)(a0)
	ldq_u	t5,(CACHE_LINE_SIZE*3)(a0)

	/* fetch the rest of the first cache line */
	ldq_u	t6,8(a0)
	ldq_u	t7,16(a0)
	ldq_u	t8,24(a0)
	/* add more inst if CACHE_LINE_SIZE changes */

	/* adjust counter */
	subq	a2,CHUNK_SIZE,a2

	/* fetch the rest of the second cache line */
	ldq_u	t9,(CACHE_LINE_SIZE+8)(a0)
	ldq_u	t10,(CACHE_LINE_SIZE+16)(a0)
	ldq_u	t11,(CACHE_LINE_SIZE+24)(a0)
	/* add more ... */

	/* Now for the stores, first cache line */
	stq_u	t2,0(a1)
	stq_u	t6,8(a1)
	stq_u	t7,16(a1)
	stq_u	t8,24(a1)

	/* fetch third cache line */
	ldq_u	t6,((CACHE_LINE_SIZE*2)+8)(a0)
	ldq_u	t7,((CACHE_LINE_SIZE*2)+16)(a0)
	ldq_u	t8,((CACHE_LINE_SIZE*2)+24)(a0)

	/* stores, second cache line */
	stq_u	t3,(CACHE_LINE_SIZE)(a1)
	stq_u	t9,(CACHE_LINE_SIZE+8)(a1)
	stq_u	t10,(CACHE_LINE_SIZE+16)(a1)
	stq_u	t11,(CACHE_LINE_SIZE+24)(a1)

	/* fetch fourth cache line */
	ldq_u	t9,((CACHE_LINE_SIZE*3)+8)(a0)
	ldq_u	t10,((CACHE_LINE_SIZE*3)+16)(a0)
	ldq_u	t11,((CACHE_LINE_SIZE*3)+24)(a0)

	/* stores, third cache line */
	stq_u	t4,(CACHE_LINE_SIZE*2)(a1)
	stq_u	t6,((CACHE_LINE_SIZE*2)+8)(a1)
	stq_u	t7,((CACHE_LINE_SIZE*2)+16)(a1)
	stq_u	t8,((CACHE_LINE_SIZE*2)+24)(a1)

	/* last time round ? */
	ble	a2,finish_up
	nop		/* keep double issue */

	/* Nope, do the last line, adjust pointers and repeast */
	stq_u	t5,(CACHE_LINE_SIZE*3)(a1)
	stq_u	t9,((CACHE_LINE_SIZE*3)+8)(a1)
	stq_u	t10,((CACHE_LINE_SIZE*3)+16)(a1)
	stq_u	t11,((CACHE_LINE_SIZE*3)+24)(a1)

	addq	a0,CHUNK_SIZE,a0
	addq	a1,CHUNK_SIZE,a1
	br	zero,bcopy
	nop			/* align */

finish_up:
	/* We must still do the stores of the fourth cache line */
	stq_u	t5,(CACHE_LINE_SIZE*3)(a1)
	stq_u	t9,((CACHE_LINE_SIZE*3)+8)(a1)
	stq_u	t10,((CACHE_LINE_SIZE*3)+16)(a1)
	stq_u	t11,((CACHE_LINE_SIZE*3)+24)(a1)

	RET
	END(aligned_block_copy)


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
LEAF(kdb_getiomem,1)
#if 1
call_pal 0x80
#else
	ldgp	gp,0(pv)
	lda	v0,kdb_iomem_recover
	stq	v0,fast_thread_recover
	andi	t0,a0,3
	bne	t0,zero,g_by_shorts
	andi	t0,1
	ldq	v0,0(a0)
	b	kdb_iomem_ok
	nop
g_by_shorts:
	bne	t0,zero,g_by_bytes
	nop
	lhu	v0,0(a0)
	lhu	t1,2(a0)
	nop
	sll	t1,16
	b	kdb_iomem_ok
	or	v0,t1

g_by_bytes:
	lbu	v0,0(a0)
	lbu	t1,1(a0)
	lbu	t2,2(a0)
	lbu	t3,3(a0)
	sll	t1,8
	sll	t2,16
	sll	t3,24
	or	v0,t1
	or	v0,t2
	b	kdb_iomem_ok
	or	v0,t3

kdb_iomem_recover:
	li	v0,-1
kdb_iomem_ok:
	stq	zero,fast_thread_recover
	j	ra
	nop
#endif
	RET
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
LEAF(kdb_putiomem,2)
#if	1
call_pal 0x80
#else
	la	v0,kdb_iomem_recover
	stq	v0,fast_thread_recover
	andi	t0,a0,3
	bne	t0,zero,byshorts
	andi	t0,1
	stq	a1,0(a0)
	b	kdb_iomem_ok
	nop
byshorts:
	bne	t0,zero,bybytes
	srl	t0,a1,16
	sh	a1,0(a0)
	sh	t0,2(a0)
	b	kdb_iomem_ok
	nop

bybytes:
	sb	a1,0(a0)
	srl	a1,8
	sb	a1,1(a0)
	sb	t0,2(a0)
	srl	t0,8
	sb	t0,3(a0)
	b	kdb_iomem_ok
	nop
#endif	
	RET
	END(kdb_putiomem)
#endif	MACH_KDB
