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
 *  Software Distribution Coordinator   or   Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they made and grant Carnegie Mellon the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	fork.s,v $
 * Revision 2.4  92/03/01  00:40:16  rpd
 * 	Removed sccsid.
 * 	[92/02/29            rpd]
 * 
 * Revision 2.3  92/01/23  15:22:32  rpd
 * 	Removed dependency on syscall.h by adding
 * 	an explicit definition of SYS_fork.
 * 	[92/01/16            rpd]
 * 
 * Revision 2.2  92/01/16  00:02:01  rpd
 * 	Moved from user collection to mk collection.
 * 
 * Revision 2.2  91/04/11  11:50:50  mrt
 * 	First checkin
 * 
 */

#include <mach/mips/asm.h>

#define SYS_fork	2

SYSCALL(fork)
	beq	v1,zero,parent
	subu	sp,20		# need some temp stack space
	sw	ra,20(sp)
	jal	mach_init
	lw	ra,20(sp)
	addu	sp,20		# restore sp..
	move	v0,zero
parent:
	RET		# pid = fork()
.end fork
