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
 * $Log:	pio.h,v $
 * Revision 2.6  93/01/14  17:29:31  danner
 * 	altered malformed error message
 * 	[92/12/18            danner]
 * 	Took the warning inside ifndef __GNUC__ outside of comments. This
 * 	will make the compiler find the error instead of having to find
 * 	out about it manually.
 * 	Protected against multiple inclusions.
 * 	[92/11/30            jvh]
 * 
 * Revision 2.5  91/05/14  16:14:20  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:13:56  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:37:08  mrt]
 * 
 * Revision 2.3  90/12/20  16:36:37  jeffreyh
 * 	changes for __STDC__
 * 	[90/12/07            jeffreyh]
 * 
 * Revision 2.2  90/11/26  14:48:41  rvb
 * 	Pulled from 2.5
 * 	[90/11/22  10:09:38  rvb]
 * 
 * 	[90/08/14            mg32]
 * 
 * 	Now we know how types are factor in.
 * 	Cleaned up a bunch: eliminated ({ for output and flushed unused
 * 	output variables.
 * 	[90/08/14            rvb]
 * 
 * 	This is how its done in gcc:
 * 		Created.
 * 	[90/03/26            rvb]
 * 
 */

#ifndef _I386_PIO_H_
#define _I386_PIO_H_

#ifndef	__GNUC__
#error	You do not stand a chance.  This file is gcc only.
#endif	__GNUC__

#define inl(y) \
({ unsigned long _tmp__; \
	asm volatile("inl %1, %0" : "=a" (_tmp__) : "d" ((unsigned short)(y))); \
	_tmp__; })

#define inw(y) \
({ unsigned short _tmp__; \
	asm volatile(".byte 0x66; inl %1, %0" : "=a" (_tmp__) : "d" ((unsigned short)(y))); \
	_tmp__; })

#define inb(y) \
({ unsigned char _tmp__; \
	asm volatile("inb %1, %0" : "=a" (_tmp__) : "d" ((unsigned short)(y))); \
	_tmp__; })


#define outl(x, y) \
{ asm volatile("outl %0, %1" : : "a" (y) , "d" ((unsigned short)(x))); }


#define outw(x, y) \
{asm volatile(".byte 0x66; outl %0, %1" : : "a" ((unsigned short)(y)) , "d" ((unsigned short)(x))); }


#define outb(x, y) \
{ asm volatile("outb %0, %1" : : "a" ((unsigned char)(y)) , "d" ((unsigned short)(x))); }

#endif /* _I386_PIO_H_ */
