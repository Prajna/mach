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
 * $Log:	ipl.h,v $
 * Revision 2.7  93/05/15  19:30:27  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.6  93/01/14  17:29:10  danner
 * 	Proper spl typing.
 * 	[92/11/30            af]
 * 
 * Revision 2.5  91/05/14  16:10:22  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:12:35  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:35:22  mrt]
 * 
 * Revision 2.3  90/11/26  14:48:35  rvb
 * 	Change Prime copyright as per Peter J. Weyman authorization.
 * 	[90/11/19            rvb]
 * 	Add ivect, iunit, and intpri declarations.
 * 	[90/06/15            rvb]
 * 
 * Revision 2.2  89/09/25  12:32:26  rvb
 * 	This is UNPUBLISHED PROPRIETARY SOURCE CODE of Prime Computer, Inc.
 * 	[89/09/23            rvb]
 * 
 */

/*
Copyright (c) 1988,1989 Prime Computer, Inc.  Natick, MA 01760
All Rights Reserved.

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and
without fee is hereby granted, provided that the above
copyright notice appears in all copies and that both the
copyright notice and this permission notice appear in
supporting documentation, and that the name of Prime
Computer, Inc. not be used in advertising or publicity
pertaining to distribution of the software without
specific, written prior permission.

THIS SOFTWARE IS PROVIDED "AS IS", AND PRIME COMPUTER,
INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN
NO EVENT SHALL PRIME COMPUTER, INC.  BE LIABLE FOR ANY
SPECIAL, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
PROFITS, WHETHER IN ACTION OF CONTRACT, NEGLIGENCE, OR
OTHER TORTIOUS ACTION, ARISING OUR OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/


#define SPL0            0
#define SPL1            1
#define SPL2            2
#define SPL3            3
#define SPL4            4
#define SPL5            5
#define SPL6            6

#define SPLPP           5
#define SPLTTY          6
#define SPLNI           6

#define IPLHI           8
#define SPL7            IPLHI
#define SPLHI           IPLHI

#ifdef	KERNEL
#ifndef	ASSEMBLER
#include <machine/machspl.h>
extern int		(*ivect[])();
extern int		iunit[];
extern unsigned char	intpri[];
#endif	ASSEMBLER
#endif	KERNEL
