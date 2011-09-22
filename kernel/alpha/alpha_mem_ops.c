/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
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
 * $Log:	alpha_mem_ops.c,v $
 * Revision 2.3  93/03/09  10:49:13  danner
 * 	GCC lint.
 * 	[93/03/05            af]
 * 
 * Revision 2.2  93/02/05  07:57:24  danner
 * 	Fixed broken blkclr.  Added protos.  Added rindex.
 * 	[93/02/04  00:54:25  af]
 * 
 * 	Created.
 * 	[92/12/10  14:50:41  af]
 * 
 */
/*
 *	File: alpha_mem_ops.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Memory copy, clear and compare operations, including string
 *	operations and byte-swaps.
 *
 */

#include <mach/mach_types.h>

/*
 *	Object:
 *		bzero				EXPORTED function
 *
 *		Clear memory locations
 *
 *	Optimize for aligned memory ops, if possible and simple.
 *	Might need later recoding in assembly for better efficiency,
 *	just like many other functions in here.
 */

void
bzero(
	register vm_offset_t	addr,
	register unsigned	bcount)
{
	register int    i;

	if (bcount == 0)	/* sanity */
		return;
	switch (addr & 3) {
	    case 1:
		*((char *) addr++) = 0;
		if (--bcount == 0)
			return;
	    case 2:
		*((char *) addr++) = 0;
		if (--bcount == 0)
			return;
	    case 3:
		*((char *) addr++) = 0;
		if (--bcount == 0)
			return;
	    default:
		break;
	}

#define	LOG_UNROLL	5
#define	PER_PASS	(1 << LOG_UNROLL)
	if (bcount >= PER_PASS) {
		for (i = bcount >> LOG_UNROLL; i; i--, addr += PER_PASS) {
	                ((int *)addr)[ 0] = 0; ((int *)addr)[ 1] = 0;
			((int *)addr)[ 2] = 0; ((int *)addr)[ 3] = 0;
	                ((int *)addr)[ 4] = 0; ((int *)addr)[ 5] = 0;
			((int *)addr)[ 6] = 0; ((int *)addr)[ 7] = 0;
		}
		bcount &= (PER_PASS - 1);	/* fast modulus */
	}
#undef	PER_PASS
#undef	LOG_UNROLL

	for (i = bcount >> 2; i; i--, addr += 4)
		*((int *) addr) = 0;

	switch (bcount & 3) {
	    case 3: *((char*)addr++) = 0;
	    case 2: *((char*)addr++) = 0;
	    case 1: *((char*)addr++) = 0;
	    default:break;
	}
}

/*
 *	Object:
 *		blkclr				EXPORTED function
 *
 *		Same as above
 *
 */
void
blkclr(
	register vm_offset_t	addr,
	register unsigned	bcount)
{
	bzero(addr,bcount);
}

/*
 *	Object:
 *		bcopy				EXPORTED function
 *
 *		Memory copy
 *
 */
void
bcopy(
	register vm_offset_t from,
	register vm_offset_t to,
	register unsigned bcount)
{
	register int    i;

	if ((from & 3) != (to & 3)) {
		/* wont align easily */
		while (bcount--)
			*((char *) to++) = *((char *) from++);
		return;
	}
	switch (to & 3) {
	    case 1:
		*((char *) to++) = *((char *) from++);
		if (--bcount == 0)
			return;
	    case 2:
		*((char *) to++) = *((char *) from++);
		if (--bcount == 0)
			return;
	    case 3:
		*((char *) to++) = *((char *) from++);
		if (--bcount == 0)
			return;
	    default:
		break;
	}

	/* XXX unroll */
	for (i = bcount >> 2; i; i--, to += 4, from += 4)
		*((int *) to) = *((int *) from);

	switch (bcount & 3) {
	    case 3:
		*((char *) to++) = *((char *) from++);
	    case 2:
		*((char *) to++) = *((char *) from++);
	    case 1:
		*((char *) to++) = *((char *) from++);
	    default:
		break;
	}
}


/*
 *	Object:
 *		ovbcopy				EXPORTED function
 *
 *		Overlapped byte copy
 *
 */
ovbcopy(
	register char *from,
	register char *to,
	register len)
{
	if (from < to) {
		from += len;
		to += len;
		while (len--)
			*--to = *--from;
	} else {
		while (len--)
			*to++ = *from++;
	}
}


/*
 *	Object:
 *		index				EXPORTED function
 *
 *		Find a character in a string
 *
 */
const char *index(
	register const char	*str,
	register char		 c)
{
	register char   cc;

	while (((cc = *str++) != c) && cc);

	return (cc == c) ? str - 1 : (const char *) 0L;
}


/*
 *	Object:
 *		rindex				EXPORTED function
 *
 *		Find a character in a string, backwards
 *
 */
char *rindex(str, c)
	register char	*str;
	register char	 c;
{
	register char   cc, *ccp = 0;

	while (cc = *str++)
		if (cc == c) ccp = str - 1;

	return ccp;
}

/*
 *	Object:
 *		htonl				EXPORTED function
 *
 *		Host to network byte order conversion, long
 *
 */
unsigned
htonl(
	register unsigned n)
{
	register unsigned tmp0, tmp1;

	tmp0 = (n << 24) | (n >> 24);
	tmp1 = (n & 0xff00) << 8;
	tmp0 |= tmp1;
	tmp1 = (n >> 8) & 0xff00;
	return tmp0 | tmp1;
}

/*
 *	Object:
 *		ntohl				EXPORTED function
 *
 *		Byteswap an integer
 *
 */
unsigned ntohl(
	register unsigned n)
{
	register unsigned tmp0, tmp1;

	tmp0 = (n << 24) | (n >> 24);
	tmp1 = (n & 0xff00) << 8;
	tmp0 |= tmp1;
	tmp1 = (n >> 8) & 0xff00;
	return tmp0 | tmp1;
}

