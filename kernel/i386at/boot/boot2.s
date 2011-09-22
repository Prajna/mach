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
 * $Log:	boot2.s,v $
 * Revision 2.2.3.1  94/03/18  13:15:48  rvb
 * 	Allow partition (say dos) to be specified
 * 
 * Revision 2.2  92/04/04  11:35:26  rpd
 * 	From 2.5
 * 	[92/03/30            rvb]
 * 
 * Revision 2.2  91/04/02  14:39:21  mbj
 * 	Put into rcs tree
 * 	[90/02/09            rvb]
 * 
 */

#include	"machine/asm.h"

/*
 * boot2() -- second stage boot
 */

ENTRY(boot2)
	movl	%cs, %ax
	movl	%ax, %ds
	movl	%ax, %es
	/* change to protected mode */
	data16
	call	_real_to_prot

	pushl	%edx
	call	_boot
	ret

ENTRY(retry)
	call	_prot_to_real
	movb	$0x80, %dl
	data16
	ljmp	$0x7c0, $0

