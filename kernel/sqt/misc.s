/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * Copyright (c) 1991 Sequent Computer Systems
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND SEQUENT COMPUTER SYSTEMS ALLOW FREE USE OF
 * THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * SEQUENT COMPUTER SYSTEMS DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	misc.s,v $
 * Revision 2.3  91/07/31  18:02:35  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  12:57:48  dbg
 * 	Converted for pure kernel.
 * 	[90/05/02            dbg]
 * 
 */

/* $Copyright:	$
 * Copyright (c) 1984, 1985, 1986, 1987 Sequent Computer Systems, Inc.
 * All rights reserved
 *  
 * This software is furnished under a license and may be used
 * only in accordance with the terms of that license and with the
 * inclusion of the above copyright notice.   This software may not
 * be provided or otherwise made available to, or used by, any
 * other person.  No title to or ownership of the software is
 * hereby transferred.
 */

/*
 * misc.s
 *	Miscellaneous Assembly routines.
 */

#include <assym.s>
#include <machine/asm.h>
#include <sqt/asm_macros.h>

/*
 * bit = ffs(mask)
 *	unsigned int mask;
 *
 * Return first found set bit position (1-32) or zero if none set.
 */

ENTRY(ffs)
	movl	$-1, %eax	# in case no bits set
	bsfl	S_ARG0, %eax	# check all 32 bits of arg
	incl	%eax		# incr by one for 0-32 (0 if S_ARG==0)
	ret

/*
 * enable_nmi: NMI's are enabled at the processor by
 * an iret. This routine enables interrupts and does
 * an iret back to the caller, thus enabling interrupts
 * and NMI's. Only called by trap().
 *	On entry, stack looks like
 *		<return_eip>
 *	Change to (stack grows down)
 *		<flags>
 *		<kernel_cs>
 *		<return_eip>
 *	so the iret works
 */

ENTRY(enable_nmi)
	popl	%eax		# return addr
	sti
	pushfl
	pushl	$(KERNEL_CS)
	pushl	%eax
	iret

/*
 * Return logical processor ID
 */

ENTRY(cpu_number)
	CPU_NUMBER(%eax)
	ret
