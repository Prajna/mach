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
 * $Log:	cirbuf.h,v $
 * Revision 2.6  93/01/14  17:26:37  danner
 * 	Added function prototypes.  Added cb_clear.
 * 	[92/11/17            dbg]
 * 	Added function prototypes.  Added cb_clear.
 * 	[92/11/17            dbg]
 * 
 * Revision 2.5  92/05/05  10:46:18  danner
 * 	Added c_hog.
 * 	[92/05/04  11:31:37  af]
 * 
 * Revision 2.4  91/05/14  15:39:43  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:08:06  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:26:40  mrt]
 * 
 * Revision 2.2  90/08/27  21:54:39  dbg
 * 	Created.
 * 	[90/07/09            dbg]
 * 
 */
/*
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date: 	7/90
 */

#ifndef	_DEVICE_CIRBUF_H_
#define	_DEVICE_CIRBUF_H_

/*
 * Circular buffers for TTY
 */

struct cirbuf {
	char *	c_start;	/* start of buffer */
	char *	c_end;		/* end of buffer + 1*/
	char *	c_cf;		/* read pointer */
	char *	c_cl;		/* write pointer */
	short	c_cc;		/* current number of characters
				   (compatibility) */
	short	c_hog;		/* max ever */
};

/*
 * Exported routines
 */
extern int	putc(int, struct cirbuf *);
extern int	getc(struct cirbuf *);
extern int	q_to_b(struct cirbuf *, char *, int);
extern int	b_to_q(char *, int, struct cirbuf *);
extern int	nqdb(struct cirbuf *, int);
extern void	ndflush(struct cirbuf *, int);
extern void	cb_clear(struct cirbuf *);

extern void	cb_alloc(struct cirbuf *, int);
extern void	cb_free(struct cirbuf *);

#endif	/* _DEVICE_CIRBUF_H_ */
