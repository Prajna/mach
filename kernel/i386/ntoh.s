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
 * $Log:	ntoh.s,v $
 * Revision 2.4  91/05/14  16:12:50  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/14  15:04:55  mrt
 * 	Changed to new Mach copyright
 * 
 * 
 * Revision 2.2  90/05/03  15:34:56  dbg
 * 	First checkin.
 * 
 * 	New a.out and coff compatible .s files.
 * 	[89/10/16            rvb]
 *
 * Revision 1.3  89/02/26  12:35:37  gm0w
 * 	Changes for cleanup.
 * 
 * 16-Feb-89  Robert Baron (rvb) at Carnegie-Mellon University
 *	Created.
 *
 */

#include <i386/asm.h>

Entry(ntohl)
ENTRY(htonl)
	movl	4(%esp), %eax
	rorw	$8, %ax
	ror	$16,%eax
	rorw	$8, %ax
	ret


Entry(ntohs)
ENTRY(htons)
	movzwl	4(%esp), %eax
	rorw	$8, %ax
	ret
