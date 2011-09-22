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
 * $Log:	alpha_lock.s,v $
 * Revision 2.4  93/03/09  10:49:07  danner
 * 	Added placeholder for non-MP case to satisfy GLD.
 * 	[93/02/16            af]
 * 
 * Revision 2.3  93/01/19  08:59:08  danner
 * 	Locks are quad-words now.
 * 	[92/12/30            af]
 * 
 * Revision 2.2  93/01/14  17:11:20  danner
 * 	Try_lock should try harder, until either wins it or lose it.
 * 	[92/12/24            af]
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:11:28  af]
 * 
 * 	Created.
 * 	[92/06/02            af]
 * 
 */
/*
 *	File: alpha_lock.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	11/92
 *
 *	Simple inter-locked operations
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 */

#include <cpus.h>
#include <platforms.h>

#if	(NCPUS>1)

#include <mach/alpha/asm.h>

	.set	noreorder
	.set	noat

/*
 *	Object:
 *		simple_lock_init			EXPORTED function
 *
 *		Initialize a simple lock
 *
 *	Arguments:
 *		lock					struct slock *
 */
LEAF(simple_lock_init,1)
	stq	zero,0(a0)		/* its a long */
	mb
	RET
	END(simple_lock_init)

#if	ADU
/* We donno what this is yet.  Possibly because of bus overload,
   the ADU misses out on cache invalidates, sometimes.  A fix
   seems to be to flush the dcache in between attempts to get
   a lock.  That is done in C code. */

#define	simple_lock	Simple_lock

#endif	/* ADU */

/*
 *	Object:
 *		simple_lock				EXPORTED function
 *
 *		Acquire a simple lock
 *
 *	Arguments:
 *		lock					struct slock *
 */
LEAF(simple_lock,1)
or	s0,zero,t1
or	s1,zero,t2
	mb
or	t1,zero,s0
or	t2,zero,s1
	ldq_l	t0,0(a0)		/* fetch&lock */
	or	zero,2,v0		/* build "locked" value */
	bne	t0,simple_lock_loop	/* was it taken already */
	stq_c	v0,0(a0)		/* race to grab it */
	beq	v0,simple_lock_loop
#if 1 /* debug */
	mb
#endif
	RET				/* got it alright */
simple_lock_loop:
#if 1 /* debug */
	ldah	a1,0x10(zero)  /* 1mil */
	lda	a2,0x10(zero)   /* in-between flushes */
simple_lock_loop_:
#endif
	lda	a1,-1(a1)
	lda	a2,-1(a2)
or	s0,zero,t1
or	s1,zero,t2
	ldq	t0,0(a0)		/* check again */
	nop				/* do not double-issue */
or	t1,zero,s0
or	t2,zero,s1
bgt a1,1f
call_pal 0x80/*op_bpt*/
ldah a1,10(zero)
1:
bgt a2,1f
call_pal 0x86/*op_imb*/
lda a2,0x10(zero)
1:
	bne	t0,simple_lock_loop_	/* probably still held */
	nop
	br	zero,simple_lock	/* go and try again now */
	END(simple_lock)

/*
 *	Object:
 *		simple_unlock				EXPORTED function
 *
 *		Release a simple lock
 *
 *	Arguments:
 *		lock					struct slock *
 */
LEAF(simple_unlock,1)
or	s0,zero,t1
or	s1,zero,t2
	mb				/* make sure all writes completed */
	stq	zero,0(a0)		/* its a long */
or	t1,zero,s0
or	t2,zero,s1
	mb				/* make damn sure they see it */
	RET
	END(simple_unlock)

/*
 *	Object:
 *		simple_lock_try				EXPORTED function
 *
 *		Try once to acquire a simple lock
 *
 *	Arguments:
 *		none
 */
LEAF(simple_lock_try,1)
or	s0,zero,t1
or	s1,zero,t2
	mb
or	t1,zero,s0
or	t2,zero,s1
	ldq_l	t0,0(a0)
	or	zero,2,v0		/* build "locked" value */
	bne	t0,nope			/* already set, forget it */
	stq_c	v0,0(a0)		/* see if we still had the lock */
	beq	v0,yipe
#if 1 /* debug */
	mb
#endif
	RET				/* if v0 != 0 then we got it */
nope:
	mov	zero,v0			/* failed to acquire lock */
	RET
yipe:	br	zero,simple_lock_try	/* try once more */
	END(simple_lock_try)

/*
 *	Object:
 *		i_bit_clear			EXPORTED function
 *
 *		Clear a bit in interlocked fashion
 *
 *	Arguments:
 *		bitno				unsigned int
 *		bitset				unsigned long *
 */
LEAF(i_bit_clear,2)
	or	zero,1,t0
	sll	t0,a0,t0		/* mask up the bit */
	ldq_l	v0,0(a1)		/* fetch&lock */
	andnot	v0,t0,v0		/* clear it */
	stq_c	v0,0(a1)		/* put it back */
	beq	v0,i_bit_clear_again	/* did the store succeed */
	mb				/* make sure they see it */
	RET
i_bit_clear_again:
	br	zero,i_bit_clear
	END(i_bit_clear)

/*
 *	Object:
 *		i_bit_set			EXPORTED function
 *
 *		Clear a bit in interlocked fashion
 *
 *	Arguments:
 *		bitno				unsigned int
 *		bitset				unsigned long *
 */
LEAF(i_bit_set,2)
	or	zero,1,t0
	sll	t0,a0,t0		/* mask up the bit */
	ldq_l	v0,0(a1)		/* fetch&lock */
	or	v0,t0,v0		/* set it */
	stq_c	v0,0(a1)		/* put it back */
	beq	v0,i_bit_set_again	/* did the store succeed */
	mb				/* make sure they see it */
	RET
i_bit_set_again:
	br	zero,i_bit_set
	END(i_bit_set)

/*
 *	Object:
 *		bit_lock			EXPORTED function
 *
 *		Acquire a bit lock
 *
 *	Arguments:
 *		bitno				unsigned int
 *		bitstring			unsigned char *
 *
 *	Do the argument mods and call the above
 */
LEAF(bit_lock,2)
	and	a0,0x7,t0	/* bit within byte */
	srl	a0,3,t1		/* byte no */
	addq	a1,t1,t1	/* pointer to byte */
	and	t1,0x7,t2	/* byte within word */
	andnot	t1,0x7,a1	/* aligned, arg ok now */
	sll	t2,3,t2
	or	t2,t0,a0	/* bit within word, arg ok */
	br	zero,i_bit_set

	END(bit_lock)

/*
 *	Object:
 *		bit_unlock			EXPORTED function
 *
 *		Release a bit lock
 *
 *	Arguments:
 *		bitno				unsigned int
 *		bitstring			unsigned char *
 */
LEAF(bit_unlock,2)
	and	a0,0x7,t0	/* bit within byte */
	srl	a0,3,t1		/* byte no */
	addq	a1,t1,t1	/* pointer to byte */
	and	t1,0x7,t2	/* byte within word */
	andnot	t1,0x7,a1	/* aligned, arg ok now */
	sll	t2,3,t2
	or	t2,t0,a0	/* bit within word, arg ok */
	br	zero,i_bit_clear

	END(bit_unlock)

/*
 *	Object:
 *		Some statically allocated synchronization variables
 *
 *	To reduce contention, some variables better not land on
 *	the same cache lines.  Doing the allocation here, by hand
 *	assures the compiler will not make mistakes.  Ugly, but.
 */
	.data
	.align 5	/* align on 32 byte boundary */
	.globl cpus_active
cpus_active:	.space 32
	.globl cpus_idle
cpus_idle:	.space 32

#else	/* NCPUS>1 */

	/* Linker does not like empty .o files */
	.text
	.globl uniprocessor_kernel
uniprocessor_kernel:	nop

#endif	/* NCPUS>1 */
