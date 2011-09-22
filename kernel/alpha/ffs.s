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
 * $Log:	ffs.s,v $
 * Revision 2.2  93/01/14  17:13:04  danner
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:14:20  af]
 * 
 * 	Created.
 * 	[91/12/29            af]
 * 
 */
/*
 *	File: ffs.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Find-first-set function
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 */
#include <mach/alpha/asm.h>

	.set noat

/*
 * Object:
 *
 *	ffs				EXPORTED function
 *
 * Arguments:
 *
 *	value				unsigned long
 *
 * Returns:
 *
 *	first_set			unsigned char/short/int/long
 *
 * Description:
 *
 *	Returns the first bit set in the 64bit argument, counting
 *	up from 1 for the lsb. It is ok to use it on smaller
 *	quantities, because they ought to be zero-extended.
 *
 * Implementation:
 *
 *	Two methods are coded. The first, optimized one uses the
 *	special cmpbge instruction, the second one a straight
 *	test-and-branch loop.  Here are some sample instruction counts:
 *
 *	value		cmpbge		loop
 *	0		3		3
 *	1		7		5
 *	2		10		8
 *	4		13		11
 *	2^8		11		29
 *	2^63		54		194
 *	2^n		6+3k+3j		5+3n
 *
 *	where k==log2(n) and j==n-2^k
 *
 *	Pending more detailed informations on actual cycle counts
 *	cmpbge clearly wins out.
 */

#define	USE_CMPBGE	1

LEAF(ffs,1)

#if	USE_CMPBGE

	.livereg	IM_V0|IM_AT|IM_A0, 0

	mov	zero,v0
	beq	a0,done

	cmpbge	zero,a0,at

	blbc	at,sbyte
	srl	at,1,at
	srl	a0,8,a0
	addq	v0,8,v0

	blbc	at,sbyte
	srl	at,1,at
	srl	a0,8,a0
	addq	v0,8,v0

	blbc	at,sbyte
	srl	at,1,at
	srl	a0,8,a0
	addq	v0,8,v0

	blbc	at,sbyte
	srl	at,1,at
	srl	a0,8,a0
	addq	v0,8,v0

	blbc	at,sbyte
	srl	at,1,at
	srl	a0,8,a0
	addq	v0,8,v0

	blbc	at,sbyte
	srl	at,1,at
	srl	a0,8,a0
	addq	v0,8,v0

	blbc	at,sbyte
	srl	at,1,at
	srl	a0,8,a0
	addq	v0,8,v0

sbyte:	addq	v0,1,v0
	blbs	a0,done
1:	srl	a0,1,a0
	addq	v0,1,v0
	blbc	a0,1b

done:
	ret

#else	/* USE_CMPBGE */

	.livereg	IM_V0|IM_A0,0

	cmovq	a0,zero,v0
	beq	a0,2f
	or	zero,1,v0
	blbs	a0,2f

1:	srl	a0,1,a0
	addq	v0,1,v0
	blbc	a0,1b

2:	ret

#endif	/* USE_CMPBGE */

	END(ffs)

