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
 * $Log:	cross_32_to_64.h,v $
 * Revision 2.2  93/01/14  17:57:23  danner
 * 	Created.
 * 	[92/12/10            af]
 * 
 */

#ifdef	CROSS_COMPILE_32_TO_64_BITS

#define int32	int
typedef struct {
	unsigned int32	low;
	unsigned int32	high;
} int64;
#define	low32(x)	(x).low
#define high32(x)	(x).high

#define	_MACHINE_VM_TYPES_H_ 1
typedef int64 vm_offset_t;
typedef int64 vm_size_t;
typedef int64 integer_t;

extern int64	zero;
#define	neq(a,b)	(((a).low != (b).low) || (a).high != (b).high)
extern  int64 plus( int64, int64);
extern  int64 minus( int64, int64);
extern	int64 plus_a_32( int64, int32);

#else	/* CROSS_COMPILE_32_TO_64_BITS */

#include <mach/machine/vm_types.h>

#define int32 int

#define	low32(x)	x
#define high32(x)	0
#define zero 		0
#define	neq(a,b)	((a) != (b))
#define plus(a,b)	(a) + (b)
#define	minus(a,b)	(a) - (b)
#define plus_a_32(a,b)	(a) + (b)

#endif	/* CROSS_COMPILE_32_TO_64_BITS */

