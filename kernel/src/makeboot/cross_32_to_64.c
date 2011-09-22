/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie Mellon the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	cross_32_to_64.c,v $
 * Revision 2.3  93/05/10  17:48:53  rvb
 * 	Fix include to use < vs " for new ode shadowing
 * 	[93/05/10  10:40:51  rvb]
 * 
 * Revision 2.2  93/01/14  17:57:18  danner
 * 	Created.
 * 	[92/12/10            af]
 * 
 */
#ifdef	CROSS_COMPILE_32_TO_64_BITS

#include <cross_32_to_64.h>

int64	zero = {0,0};

/* taken from my simulator */

/* add an unsigned int32 to a int64 */
#define add_l_ui(r,l,i)							\
    {									\
	register unsigned int32 lp, hp, carry;				\
	lp = (low32(l) & 0xffff) + ((i) & 0xffff);			\
	carry = lp >> 16;						\
	hp = (((unsigned)low32(l)) >> 16) + (((unsigned)i) >> 16) + carry;\
	carry = hp >> 16;						\
	low32(r) = (hp << 16) | (lp & 0xffff);				\
	high32(r) = high32(l) + carry;					\
    }

/* add to int64s */
#define add_l_l(r,a,b)							\
    {									\
	register unsigned int32 lp, hp, carry;				\
	lp = (low32(a) & 0xffff) + (low32(b) & 0xffff);			\
	carry = lp >> 16;						\
	hp = (((unsigned)low32(a))>>16) + (((unsigned)low32(b))>>16) + carry;\
	carry = hp >> 16;						\
	low32(r) = (hp << 16) | (lp & 0xffff);				\
	/* now the hi part */						\
	lp = (high32(a) & 0xffff) + (high32(b) & 0xffff) + carry;	\
	carry = lp >> 16;						\
	hp = (((unsigned)high32(a))>>16) + (((unsigned)high32(b))>>16) + carry;\
	high32(r) = (hp << 16) | (lp & 0xffff);				\
    }

/* add an int32 to a int64 */
#define add_l_i(r,l,i)							\
    {									\
	if (i == 0) {							\
	    low32(r) = low32(l); high32(r) = high32(l);			\
	} else if (i > 0) {						\
	    add_l_ui(r,l,i);						\
	} else {							\
	    int64 m;							\
	    low32(m) = i; high32(m) = -1;				\
	    add_l_l(r,l,m);						\
	}								\
    }

/* sub two int64s */
#define sub_l_l(r,a,b)							\
    {									\
	register unsigned int32 lp, hp, carry;				\
	lp = (low32(a) & 0xffff) + ((~low32(b)) & 0xffff) + 1;		\
	carry = lp >> 16;						\
	hp = (((unsigned)low32(a))>>16) + ((~(unsigned)low32(b))>>16) + carry;\
	carry = hp >> 16;						\
	low32(r) = (hp << 16) | (lp & 0xffff);				\
	/* now the hi part */						\
	lp = (high32(a) & 0xffff) + ((~high32(b)) & 0xffff) + carry;		\
	carry = lp >> 16;						\
	hp = (((unsigned)high32(a))>>16) + ((~(unsigned)high32(b))>>16) + carry;\
	high32(r) = (hp << 16) | (lp & 0xffff);				\
    }

int64 plus(a,b)
	int64 a,b;
{
	int64 c;
	add_l_l(c,a,b);
	return c;
}

int64 plus_a_32(a,b)
	int64 a;
	int32 b;
{
	int64 c;
	add_l_i(c,a,b);
	return c;
}

int64 minus(a,b)
	int64 a,b;
{
	int64 c;
	sub_l_l(c,a,b);
	return c;
}

#endif	/* CROSS_COMPILE_32_TO_64_BITS */

