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
 * $Log:	cross_endian.h,v $
 * Revision 2.2  93/01/14  17:57:27  danner
 * 	Created.
 * 	[92/06/06            idall]
 * 
 */
/*
 *	File: cross_endian.h
 * 	Author: Ian Dall, University of Adelaide, Australia
 *	Date:	6/92
 *
 *	Defines for handling executables of the opposite endian-ness.
 */


#ifdef	CROSS_ENDIAN

/*
 * These must be chosen/provided by machdep code
 */
extern int get_num( char*, int );
extern int put_num( char*, vm_size_t, int );

#define INTERN(field)		(get_num((char*)&(field), sizeof(field)))
#define PUT_EXTERN(buf, val)	(put_num((char*)&(buf), (val), sizeof(buf)))
#define EXTERNALIZE(x)		(put_num((char*)&(x), (x), sizeof(x)))
#define INTERNALIZE(x)		((x) = get_num((char*)&(x), sizeof(x)), sizeof(x))

#else	/* CROSS_ENDIAN */

#define INTERN(field)		(field)
#define PUT_EXTERN(buf, val)	((buf) = (val))
#define EXTERNALIZE(x)		(x)
#define INTERNALIZE(x)		(x)

#endif
