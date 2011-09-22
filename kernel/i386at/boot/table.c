/*
 * Mach Operating System
 * Copyright (c) 1992, 1991 Carnegie Mellon University
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
 * $Log:	table.c,v $
 * Revision 2.3  93/05/10  17:47:14  rvb
 * 	Leave sector #1 empty so 386bsd can store its label there.
 * 	[93/04/01            rvb]
 * 
 * Revision 2.2  92/04/04  11:36:43  rpd
 * 	Fix Intel Copyright as per B. Davies authorization.
 * 	[92/04/03            rvb]
 * 	Taken from 2.5 bootstrap.
 * 	[92/03/30            rvb]
 * 
 * Revision 2.2  91/04/02  14:42:22  mbj
 * 	Add Intel copyright
 * 	[90/02/09            rvb]
 * 
 */

/*
  Copyright 1988, 1989, 1990, 1991, 1992 
   by Intel Corporation, Santa Clara, California.

                All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Intel
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#define NGDTENT		6
#define GDTLIMIT	48	/* NGDTENT * 8 */

/*  Segment Descriptor
 *
 * 31          24         19   16                 7           0
 * ------------------------------------------------------------
 * |             | |B| |A|       | |   |1|0|E|W|A|            |
 * | BASE 31..24 |G|/|0|V| LIMIT |P|DPL|  TYPE   | BASE 23:16 |
 * |             | |D| |L| 19..16| |   |1|1|C|R|A|            |
 * ------------------------------------------------------------
 * |                             |                            |
 * |        BASE 15..0           |       LIMIT 15..0          |
 * |                             |                            |
 * ------------------------------------------------------------
 */

struct seg_desc {
	unsigned short	limit_15_0;
	unsigned short	base_15_0;
	unsigned char	base_23_16;
	unsigned char	bit_15_8;
	unsigned char	bit_23_16;
	unsigned char	base_31_24;
	};


struct seg_desc	Gdt[NGDTENT] = {
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0},		/* 0x0 : null */
	{0xFFFF, 0x1000, 0x0, 0x9E, 0x40, 0x0},	/* 0x8 : boot code */
	{0xFFFF, 0x1000, 0x0, 0x92, 0x40, 0x0},	/* 0x10 : boot data */
	{0xFFFF, 0x1000, 0x0, 0x9E, 0x0, 0x0},	/* 0x18 : boot code, 16 bits */
	{0xFFFF, 0x0, 0x0, 0x92, 0xCF, 0x0},	/* 0x20 : init data */
	{0xFFFF, 0x0, 0x0, 0x9E, 0xCF, 0x0}	/* 0x28 : init code */
	};


struct pseudo_desc {
	unsigned short	limit;
	unsigned short	base_low;
	unsigned short	base_high;
	};

struct pseudo_desc Gdtr = { GDTLIMIT, 0x1400, 0 };
			/* boot is loaded at 4k, Gdt is located at 4k+512+512 */
