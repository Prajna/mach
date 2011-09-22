/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
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
 * $Log:	types.h,v $
 * Revision 2.9  93/03/11  13:46:33  danner
 * 	Removed u_long definition.
 * 	Removed definition of size_t.
 * 	This file is EXCLUSIVELY here for compat reasons, nothing
 * 	should be added to it at this point. 
 * 	[93/03/06  14:46:00  af]
 * 
 * Revision 2.8  93/02/01  09:50:33  danner
 * 	Updated copyright.
 * 	[93/01/12            berman]
 * 	Added label_t for sparc compilers.
 * 	[92/12/24            berman]
 * 
 * Revision 2.7  93/01/14  18:00:11  danner
 * 	Added definition of size_t.
 * 	Added void * cast to the definition of NULL.
 * 	[92/12/15            pds]
 * 
 * Revision 2.6  91/05/14  17:40:39  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:57:07  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:49:41  mrt]
 * 
 * Revision 2.4  90/08/27  22:13:03  dbg
 * 	Created.
 * 	[90/07/16            dbg]
 * 
 */
#ifndef	_SYS_TYPES_H_
#define	_SYS_TYPES_H_

/*
 * Common type definitions that lots of old files seem to want.
 */

typedef	unsigned char	u_char;		/* unsigned char */
typedef	unsigned short	u_short;	/* unsigned short */
typedef	unsigned int	u_int;		/* unsigned int */

typedef struct _quad_ {
	unsigned int	val[2];		/* 2 long values make... */
} quad;					/* an 8-byte item */

typedef	char *		caddr_t;	/* address of a (signed) char */

#if	!defined(_TIME_T_)
#define	_TIME_T_ 1
typedef	unsigned int	time_t;		/* a 32bit integer */
#endif	/* not defined(_TIME_T_) */

typedef unsigned int	daddr_t;	/* a 32bit integer */
typedef	unsigned int	off_t;		/* another 32bit integer */

typedef	unsigned short	dev_t;		/* another unsigned short */
#define	NODEV		((dev_t)-1)	/* and a null value for it */

#define	major(i)	((i) >> 8)
#define	minor(i)	((i) & 0xFF)
#define	makedev(i,j)	(((i) << 8) | (j))

#ifndef	NULL
#define	NULL		((void *) 0) 	/* the null pointer */
#endif

#ifdef sparc
typedef struct  _physadr { int r[1]; } *physadr;
typedef struct label_t {
        int     val[2];
} label_t;
#endif

#endif	/* _SYS_TYPES_H_ */
