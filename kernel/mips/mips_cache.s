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
 * $Log:	mips_cache.s,v $
 * Revision 2.7  93/05/10  21:21:09  rvb
 * 	How very stupid of me: while flushing the Icache we
 * 	can use the Dcache as Icache to speedup the loop !!
 * 	Thanks to Brad Chen for pointing this out.
 * 	[93/05/06  09:34:18  af]
 * 
 * Revision 2.6  92/02/19  15:09:18  elf
 * 	Changed #-style comments, for ANSI cpp.
 * 	[92/02/19  13:11:12  rpd]
 * 
 * Revision 2.5  92/01/03  20:25:03  dbg
 * 	Reload saved s0 in mipscache_tests.
 * 	[91/09/16            dbg]
 * 
 * Revision 2.4  91/05/14  17:35:20  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:49:13  mrt
 * 	Added author notices
 * 	[91/02/04  11:23:16  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:26:54  mrt]
 * 
 * Revision 2.2  89/11/29  14:14:26  af
 * 	Created.
 * 	[89/10/12            af]
 */
/*
 *	File: mips_cache.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	8/90
 *
 *	MIPS cache control operations.
 */

/*
 *	Various boxes seem to be using the MIPS chipset
 *	including the I&D caches.  Sometimes the caches are
 *	used as first-level caches in a two-level cache system.
 *	The operations provided here assume this first level
 *	cache is the MIPSco direct-mapped one, presumably
 *	they'd work on other direct-mapped caches that respond
 *	to the R3000 cache control functions.
 *
 *	Care has been taken to make these functions work on a
 *	multiprocessor with asymmetric caches (or with some
 *	broken caches).
 */

#include <mach/mips/asm.h>
#include <mach/mips/mips_instruction.h>

#include <mips/mips_cpu.h>
#include <mips/mips_box.h>

	.set	noreorder

/*
 *	Object:
 *		run_cached			LOCAL macro
 *		run_uncached			LOCAL macro
 *
 *		Execute following code out of K0SEG/K1SEG
 *
 *	Arguments:
 *		none
 *
 *	Assume kernel is linked for the K0SEG.
 *	Reserve use of the local label '9' and scratch
 *	register t9.
 */
#define run_cached()						\
	la	t9,9f;						\
	j	t9;						\
	nop;							\
9:	nop

#define run_uncached()						\
	la	t9,9f;						\
	or	t9,K1SEG_BASE;		/* k0->k1 */		\
	j	t9;						\
	nop;							\
9:	nop


/*
 *	Object:
 *		mipscache_size			EXPORTED function
 *
 *		Size the cache (initialization)
 *
 *	Arguments:
 *		d_size_ptr			int *
 *		i_size_ptr			int *
 *
 *	Returns the sizes of the I&D caches.
 *	Since the cache is direct mapped, it 'wraps-around'
 *	addresses modulo its size.  To discover the size
 *	one could fill the cache with a distinct value (say 1),
 *	then write a different value (say 0) at the start location
 *	and read the following locations until the second
 *	value (0) is found.  The difference between the two
 *	adresses is the cache size.
 *	Since cache sizes come in power-of-two we can speed
 *	things up quite a bit.
 *	Note that a zero-sized cache will confuse this code.
 *
 *	Side effects: Destroy the current cache content.
 */
LEAF(mipscache_size)
	run_uncached()
	nop
	nop
	mfc0	t0,c0_status
	/*
	 *	Do it first for the Data cache
	 */
	li	t1,SR_IsC
	mtc0	t1,c0_status		/* no interrupts, isolate cache */
	nop
	nop
	nop
					/* fill cache with -1 */
	li	t3,-1
	li	t1,MIN_CACHE_SIZE
	li	t2,MAX_CACHE_SIZE
1:	sw	t3,K0SEG_BASE(t1)
	sll	t1,1
	ble	t1,t2,1b
	nop
					/* store 0 at start */
	sw	zero,K0SEG_BASE(zero)
					/* look for it higher up */
	li	t1,MIN_CACHE_SIZE
2:	lw	t3,K0SEG_BASE(t1)
	bgt	t1,t2,3f		/* dont take forever */
	move	v0,zero
	bne	t3,zero,2b
	sll	t1,1
	srl	v0,t1,1			/* got it */
3:
	/*
	 *	Do it again for the Instruction cache
	 */
	li	t1,SR_IsC|SR_SwC
	mtc0	t1,c0_status		/* no interrupts, isolate cache, swap */
	nop
	nop
	nop
					/* fill cache with -1 */
	li	t3,-1
	li	t1,MIN_CACHE_SIZE
	li	t2,MAX_CACHE_SIZE
4:	sw	t3,K0SEG_BASE(t1)
	sll	t1,1
	ble	t1,t2,4b
	nop
					/* store 0 at start */
	sw	zero,K0SEG_BASE(zero)
					/* look for it higher up */
	li	t1,MIN_CACHE_SIZE
5:	lw	t3,K0SEG_BASE(t1)
	bgt	t1,t2,6f		/* dont take forever */
	move	v1,zero
	bne	t3,zero,5b
	sll	t1,1
	srl	v1,t1,1			/* got it */
6:
	/*
	 *	All done, return.
	 */
	mtc0	t0,c0_status		/* restore status, including cache */
	nop
	nop
	nop
	nop
					/* return results */
	sw	v0,0(a0)
	sw	v1,0(a1)
	j	ra
	nop
	END(mipscache_size)


/*
 *	Object:
 *		mipscache_flush			EXPORTED function
 *
 *		Flush both caches (initialization)
 *
 *	Arguments:
 *		d_size				unsigned
 *		i_size				unsigned
 *
 */
#		define	FRAMESIZE 4*4+4
NESTED(mipscache_flush, FRAMESIZE, zero)
	subu	sp,FRAMESIZE
	sw	ra,FRAMESIZE-4(sp)
	sw	a1,4(sp)

	move	a1,a0			/* Dcache first */
	jal	mipscache_Dflush
	li	a0,K0SEG_BASE

	lw	a1,4(sp)		/* Icache second */
	jal	mipscache_Iflush
	li	a0,K0SEG_BASE

	lw	ra,FRAMESIZE-4(sp)
	addu	sp,FRAMESIZE
	j	ra
	nop
#		undef	FRAMESIZE
	END(mipscache_flush)

/*
 *	Object:
 *		mipscache_Iflush		EXPORTED function
 *
 *		Flush a range of addresses in the Instruction cache
 *
 *	Arguments:
 *		addr				vm_offset_t
 *		bcount				unsigned
 *
 *	Since it is typically called to flush an entire
 *	page, the granularity is artificially larger than
 *	one entry and set to 4 entries (16 bytes).
 */
LEAF(mipscache_Iflush)
#if 0	/* assume a1==0 is an accident and save 1 cycle */
	beq	a1,zero,2f
#endif 0
	mfc0	t0,c0_status
	mtc0	zero,c0_status		/* no interrupts */
	run_uncached()
	li	t1,SR_IsC|SR_SwC	/* ditto, isolate and swap */
	mtc0	t1,c0_status
	nop				/* what's the right delay here ?!? */
	nop
	nop
	nop
        run_cached()
	addu	a1,a0,a1
1:	sb	zero,0(a0)
	sb	zero,4(a0)
	sb	zero,8(a0)
	addu	a0,4*4
	blt	a0,a1,1b
	sb	zero,-4(a0)
					/* done, restore and return */
        run_uncached()
	mtc0	t0,c0_status
	nop
	nop
	nop
	nop
2:	j	ra			/* implicit run_cached() */
	nop
	END(mipscache_Iflush)

/*
 *	Object:
 *		mipscache_Dflush		EXPORTED function
 *
 *		Flush a range of addresses in the Data cache
 *
 *
 *	Arguments:
 *		addr				vm_offset_t
 *		bcount				unsigned
 *
 */
LEAF(mipscache_Dflush)
#if 0	/* assume a1==0 is an accident and save 1 cycle */
	beq	a1,zero,2f
#endif 0
	li	t1,SR_IsC
	mfc0	t0,c0_status
	mtc0	t1,c0_status		/* no interrupts, isolate cache */
	nop				/* what's the right delay here ?!? */
	nop
	nop
	nop
	addu	a1,a0,a1
1:	sb	zero,0(a0)
	sb	zero,4(a0)
	sb	zero,8(a0)
	addu	a0,4*4
	blt	a0,a1,1b
	sb	zero,-4(a0)
					/* done, restore and return */
	mtc0	t0,c0_status
	nop
	nop
	nop
	nop
2:	j	ra
	nop
	END(mipscache_Dflush)

/*
 *	Object:
 *		mipscache_tests			EXPORTED function
 *
 *		Tests for the above functions
 *
 *	Arguments:
 *		addr				vm_offset_t
 *
 *	Uses the k0seg location addr points to as scratch area
 */
#		define	FRAMESIZE 4*4+4
NESTED(mipscache_tests,	FRAMESIZE, zero)
	subu	sp,FRAMESIZE
	sw	ra,FRAMESIZE-4(sp)
	sw	a0,0(sp)

	/* minimum consistency */
	or	a1,a0,K1SEG_BASE
	sw	a1,4(sp)
	sw	zero,0(a0)
	lw	a2,0(a1)
	nop
	bne	a2,zero,f1
	nop

	/* Dcache flush */
	li	t0,0xbaba
	sw	t0,8(sp)
	sw	t0,0(a1)
	lw	a2,0(a0)
	nop
	beq	t0,a2,f2
	nop
	jal	mipscache_Dflush
	li	a1,4
	lw	a0,0(sp)
	nop
	lw	a2,0(a0)
	lw	t0,8(sp)
	nop
	bne	a2,t0,f3
	nop

	/* Icache flush */
	sw	s0,8(sp)
	la	a0,gives_zero
	la	a1,gives_one
	lw	t0,4(a0)
	lw	t1,4(a1)
	sw	t0,12(sp)

	jal	gives_zero
	nop
	move	s0,v0
	sw	t1,4(a0)
	jal	mipscache_Iflush
	li	a1,8
	jal	gives_zero	/* should give one now */
	nop
	la	a0,gives_zero
	lw	t0,12(sp)
	beq	v0,s0,f4
	sw	t0,4(a0)

	lw	s0,8(sp)	/* dont forget to restore s0! */
	nop


	.set	reorder
/*	PRINTF("cache ok\n")*/
	sw	zero,mipscache_state	/* all OK */
	b	fin
	nop
f1:	PANIC("ct1")
f2:	PANIC("ct2")
f3:	PANIC("ct3")
f4:	PANIC("ct4")
	.set	noreorder
fin:	lw	ra,FRAMESIZE-4(sp)
	addu	sp,FRAMESIZE
	j	ra
	nop
#		undef	FRAMESIZE
	END(mipscache_tests)

STATIC_LEAF(gives_one)
	j	ra
	li	v0,1
	END(gives_one)
STATIC_LEAF(gives_zero)
	j	ra
	move	v0,zero
	END(gives_zero)

