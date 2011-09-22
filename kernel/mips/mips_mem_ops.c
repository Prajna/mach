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
 * $Log:	mips_mem_ops.c,v $
 * Revision 2.8  93/05/30  21:09:11  rvb
 * 	Adedd memcpy for when not optimizing.
 * 	Fix from jtw@lcs.mit.edu.
 * 	[93/05/08            af]
 * 
 * Revision 2.7  93/01/14  17:52:07  danner
 * 	Declaration cleanups.
 * 	[93/01/14            danner]
 * 
 * Revision 2.6  92/03/03  00:45:40  rpd
 * 	Removed strlen.  It duplicated one in device/subrs.c. (!!!)
 * 	[92/03/02            rpd]
 * 
 * Revision 2.5  91/05/14  17:36:15  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:49:55  mrt
 * 	Added author notices
 * 	[91/02/04  11:23:56  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:27:44  mrt]
 * 
 * Revision 2.3  90/08/07  22:29:47  rpd
 * 	Commented out unused unaligned access functions, but left them
 * 	in in case we change our mind someday about the fixade business.
 * 	[90/08/07  15:23:08  af]
 * 
 * Revision 2.2.1.1  90/06/11  11:24:46  af
 * 	Commented out unused unaligned access functions, but left them
 * 	in in case we change our mind someday about the fixade business.
 * 	[90/06/03            af]
 * 
 * Revision 2.2  89/11/29  14:14:48  af
 * 	Ooops, rounding in the wrong direction in many places.
 * 	[89/11/03  16:38:28  af]
 * 
 * 	Created.
 * 	[89/10/12            af]
 */
/*
 *	File: mips_mem_ops.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Memory copy, clear and compare operations, including string
 *	operations and byte-swaps.
 *
 *	Also includes some peculiar memory access functions
 *	for MIPS, such as unaligned accesses.
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
bzero(addr, bcount)
	register vm_offset_t addr;
	register unsigned bcount;
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
blkclr(addr,cnt)
{
	bzero(addr,cnt);
}

/*
 *	Object:
 *		bcopy				EXPORTED function
 *
 *		Memory copy
 *
 */
void
bcopy(from, to, bcount)
	register vm_offset_t from;
	register vm_offset_t to;
	register unsigned bcount;
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
 *		memcpy				EXPORTED function
 *
 *		ANSI Memory copy
 *
 *	Note that gcc 2.x inlines memcpy when optimizing.
 */
void*
memcpy(to, from, bcount)
	register vm_offset_t from;
	register vm_offset_t to;
	register unsigned bcount;
{
        bcopy(from, to, bcount);
	return (void *)to;
}

/*
 *	Object:
 *		ovbcopy				EXPORTED function
 *
 *		Overlapped byte copy
 *
 */
unsigned int ovbcopy_cnt = 0;

ovbcopy(from,to,len)
	register char *from,*to;
	register len;
{
ovbcopy_cnt++;
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
char *index(str, c)
	register char	*str;
	register char	 c;
{
	register char   cc;

	while (((cc = *str++) != c) && cc);

	return (cc == c) ? str - 1 : (char *) 0;
}


/*
 *	Object:
 *		htonl				EXPORTED function
 *
 *		Host to network byte order conversion, long
 *
 *	This one does not need recoding, actually this
 *	version in C might be better than the assembly
 *	version MIPS uses.
 */
unsigned
htonl(n)
	register unsigned n;
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
 *		hwcpin				EXPORTED function
 *		hwcpout				EXPORTED function
 *
 *		Copy from/to special memory
 *
 *	Some devices, such as the ethernet board on the MSERIES
 *	or on the PMAX, must be accessed in 16bit mode.
 *	This could be tricky, as we have a smart write-buffer
 *	in between..
 *	Dont give me a zero bcount, ok ?
 */
hwcpin(from,to,bcount)
	register vm_offset_t from;
	register vm_offset_t to;
	register unsigned bcount;
{
	/* From board to main memory */

	if ((from & 1) != (to & 1)) {
		/* tough call */
		while (bcount--)
			*((char *) to++) = *((char *) from++);
		return;
	}
	if (from & 1)
		*((char *) to++) = *((char *) from++), bcount--;

	for (; bcount > 1; bcount -= 2, to += 2, from += 2)
		*((unsigned short *) to) = *((unsigned short *) from);

	if (bcount)
		*((char *) to++) = *((char *) from++);
}

hwcpout(from,to,bcount)
	register vm_offset_t from;
	register vm_offset_t to;
	register unsigned bcount;
{
	/* From main memory to board */

	int temp;
	register volatile int *p = (volatile int *)&temp;

	if ((from & 1) != (to & 1)) {
		/* tough call */
		while (bcount--) {
			*((char *) to++) = *((char *) from++);
			*p = 0;
		}
		return;
	}
	switch (from & 3) {
	    case 1:
		*((char *) to++) = *((char *) from++);
		*p = 0;
		if (--bcount == 0)
			return;
	    case 2:
		*p = 0;
		*((char *) to++) = *((char *) from++);
		if (--bcount == 0)
			return;
	    case 3:
		*p = 0;
		*((char *) to++) = *((char *) from++);
		if (--bcount == 0)
			return;
	    default:
		break;
	}

	for (; bcount > 3; bcount -= 4, to += 4, from += 4) {
		register unsigned temp;
		temp = *((unsigned short *) from);
/*XXX byteorder?*/
		*((unsigned short *) to) = temp & 0xffff;
		*p = 0;
		*((unsigned short *) to + 2) = temp >> 16;
		*p = 0;
	}

	switch (bcount) {
	    case 3:
		*((char *) to++) = *((char *) from++);
		*p = 0;
	    case 2:
		*((char *) to++) = *((char *) from++);
	    case 1:
		*((char *) to++) = *((char *) from++);
	    default:
		break;
	}
}

#ifdef	notdef
	/* these are only needed if we decide to implement in
	   the kernel support for automatic fixing of unaligned
	   memory accesses by user program */

/*
 *	Object:
 *		ulw				EXPORTED function
 *		ulh				EXPORTED function
 *		ulhu				EXPORTED function
 *
 *		Unaligned load operations
 *
 */
ulw(addr,pval)
	vm_offset_t addr;
	unsigned *pval;
{
	unsigned char   buf[4];

	if (copyin(addr, buf, 4 /* no kidding */))
		return 1;
	*pval = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

ulh(addr,pval)
	vm_offset_t addr;
	int *pval;
{
	unsigned char            buf[2];
	register short    temp;

	if (copyin(addr, buf, 2))
		return 1;
	temp = buf[0] | (buf[1] << 8);
	*pval = temp;	/* sign extends */
}

ulhu(addr,pval)
	vm_offset_t addr;
	unsigned *pval;
{
	char            buf[2];

	if (copyin(addr, buf, 2))
		return 1;
	*pval = buf[0] | (buf[1] << 8);
}


/*
 *	Object:
 *		usw				EXPORTED function
 *		ush				EXPORTED function
 *
 *		Unaligned store operations
 */
usw(addr,val)
	vm_offset_t addr;
	unsigned val;
{
	return copyout(&val, addr, 4 );
}

ush(addr, val)
	vm_offset_t addr;
	unsigned val;
{
	return copyout( &val, addr, 2);
}

#endif	notdef

