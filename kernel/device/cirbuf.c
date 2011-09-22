/* 
 * Mach Operating System
 * Copyright (c) 1992,1991,1990 Carnegie Mellon University
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
 * $Log:	cirbuf.c,v $
 * Revision 2.10  93/05/17  10:31:09  rvb
 * 	Fixed nasty bug in getc(): it was returning -1 when
 * 	the character was 0xff. This makes SLIP work on DS boxes.
 * 	[93/05/13  20:28:59  af]
 * 
 * 
 * Revision 2.9  93/01/14  17:26:33  danner
 * 	Added function prototypes.  Added cb_clear.
 * 	[92/11/17            dbg]
 * 	Added function prototypes.  Added cb_clear.
 * 	[92/11/17            dbg]
 * 
 * Revision 2.8  92/08/03  17:32:56  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.7  92/05/21  17:08:44  jfriedl
 * 	tried prototypes.
 * 	[92/05/20            jfriedl]
 * 
 * Revision 2.6  92/05/05  10:46:15  danner
 * 	Initialize c_hog.
 * 	[92/05/04            af]
 * 
 * Revision 2.5  91/07/31  17:32:09  dbg
 * 	Put CB_CHECK under debugging switch.
 * 	[91/07/30  16:45:34  dbg]
 * 
 * Revision 2.4  91/05/14  15:39:25  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:08:01  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:26:33  mrt]
 * 
 * Revision 2.2  90/08/27  21:54:30  dbg
 * 	q_to_b was always emptying the circular buffer; fix it.
 * 	[90/08/06            dbg]
 * 	Created.
 * 	[90/07/09            dbg]
 * 
 */
/*
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date: 	7/90
 *
 * 	Circular buffers for TTY
 */

#include <device/cirbuf.h>
#include <kern/kalloc.h>



/* read at c_cf, write at c_cl */
/* if c_cf == c_cl, buffer is empty */
/* if c_cl == c_cf - 1, buffer is full */

#if	DEBUG
int cb_check_enable = 0;
#define	CB_CHECK(cb) if (cb_check_enable) cb_check(cb)

void
cb_check(register struct cirbuf *cb)
{
	if (!(cb->c_cf >= cb->c_start && cb->c_cf < cb->c_end))
	    panic("cf %x out of range [%x..%x)",
		cb->c_cf, cb->c_start, cb->c_end);
	if (!(cb->c_cl >= cb->c_start && cb->c_cl < cb->c_end))
	    panic("cl %x out of range [%x..%x)",
		cb->c_cl, cb->c_start, cb->c_end);
	if (cb->c_cf <= cb->c_cl) {
	    if (!(cb->c_cc == cb->c_cl - cb->c_cf))
		panic("cc %x should be %x",
			cb->c_cc,
			cb->c_cl - cb->c_cf);
	}
	else {
	    if (!(cb->c_cc == cb->c_end - cb->c_cf
			    + cb->c_cl - cb->c_start))
		panic("cc %x should be %x",
			cb->c_cc,
			cb->c_end - cb->c_cf +
			cb->c_cl - cb->c_start);
	}
}
#else	/* DEBUG */
#define	CB_CHECK(cb)
#endif	/* DEBUG */

/*
 * Put one character in circular buffer.
 */
int putc(
	int	c,
	register struct cirbuf *cb)
{
	register char *ow, *nw;

	ow = cb->c_cl;
	nw = ow+1;
	if (nw == cb->c_end)
	    nw = cb->c_start;
	if (nw == cb->c_cf)
	    return 1;		/* not entered */
	*ow = c;
	cb->c_cl = nw;

	cb->c_cc++;

	CB_CHECK(cb);

	return 0;
}

/*
 * Get one character from circular buffer.
 */
int getc(register struct cirbuf *cb)
{
	register unsigned char *nr;
	register int	c;

	nr = (unsigned char *)cb->c_cf;
	if (nr == (unsigned char *)cb->c_cl) {
	    CB_CHECK(cb);
	    return -1;		/* empty */
	}
	c = *nr;
	nr++;
	if (nr == (unsigned char *)cb->c_end)
	    nr = (unsigned char *)cb->c_start;
	cb->c_cf = (char *)nr;

	cb->c_cc--;

	CB_CHECK(cb);

	return c;
}

/*
 * Get lots of characters.
 * Return number moved.
 */
int
q_to_b( register struct cirbuf *cb,
	register char	*cp,
	register int	count)
{
	char *		ocp = cp;
	register int	i;

	while (count != 0) {
	    if (cb->c_cl == cb->c_cf)
		break;		/* empty */
	    if (cb->c_cl < cb->c_cf)
		i = cb->c_end - cb->c_cf;
	    else
		i = cb->c_cl - cb->c_cf;
	    if (i > count)
		i = count;
	    bcopy(cb->c_cf, cp, i);
	    cp += i;
	    count -= i;
	    cb->c_cf += i;
	    cb->c_cc -= i;
	    if (cb->c_cf == cb->c_end)
		cb->c_cf = cb->c_start;

	    CB_CHECK(cb);
	}
	CB_CHECK(cb);

	return cp - ocp;
}

/*
 * Add character array to buffer and return number of characters
 * NOT entered.
 */
int
b_to_q( register char *	cp,
	int	count,
	register struct cirbuf *cb)
{
	register int	i;
	register char	*lim;

	while (count != 0) {
	    lim = cb->c_cf - 1;
	    if (lim < cb->c_start)
		lim = cb->c_end - 1;

	    if (cb->c_cl == lim)
		break;
	    if (cb->c_cl < lim)
		i = lim - cb->c_cl;
	    else
		i = cb->c_end - cb->c_cl;

	    if (i > count)
		i = count;
	    bcopy(cp, cb->c_cl, i);
	    cp += i;
	    count -= i;
	    cb->c_cc += i;
	    cb->c_cl += i;
	    if (cb->c_cl == cb->c_end)
		cb->c_cl = cb->c_start;

	    CB_CHECK(cb);
	}
	CB_CHECK(cb);
	return count;
}

/*
 * Return number of contiguous characters up to a character
 * that matches the mask.
 */
int
ndqb(	register struct cirbuf *cb,
	register int	mask)
{
	register char *cp, *lim;

	if (cb->c_cl < cb->c_cf)
	    lim = cb->c_end;
	else
	    lim = cb->c_cl;
	if (mask == 0)
	    return (lim - cb->c_cf);
	cp = cb->c_cf;
	while (cp < lim) {
	    if (*cp & mask)
		break;
	    cp++;
	}
	return (cp - cb->c_cf);
}

/*
 * Flush characters from circular buffer.
 */
void
ndflush(register struct cirbuf *cb,
	register int	count)
{
	register int	i;

	while (count != 0) {
	    if (cb->c_cl == cb->c_cf)
		break;		/* empty */
	    if (cb->c_cl < cb->c_cf)
		i = cb->c_end - cb->c_cf;
	    else
		i = cb->c_cl - cb->c_cf;
	    if (i > count)
		i = count;
	    count -= i;
	    cb->c_cf += i;
	    cb->c_cc -= i;
	    if (cb->c_cf == cb->c_end)
		cb->c_cf = cb->c_start;
	    CB_CHECK(cb);
	}

	CB_CHECK(cb);
}

/*
 * Empty a circular buffer.
 */
void cb_clear(struct cirbuf *cb)
{
	cb->c_cf = cb->c_start;
	cb->c_cl = cb->c_start;
	cb->c_cc = 0;
}

/*
 * Allocate character space for a circular buffer.
 */
void
cb_alloc(
	register struct cirbuf *cb,
	int		buf_size)
{
	register char *buf;

	buf = (char *)kalloc(buf_size);

	cb->c_start = buf;
	cb->c_end = buf + buf_size;
	cb->c_cf = buf;
	cb->c_cl = buf;
	cb->c_cc = 0;
	cb->c_hog = buf_size - 1;

	CB_CHECK(cb);
}

/*
 * Free character space for a circular buffer.
 */
void
cb_free(register struct cirbuf *cb)
{
	int		size;

	size = cb->c_end - cb->c_start;
	kfree((vm_offset_t)cb->c_start, size);
}

