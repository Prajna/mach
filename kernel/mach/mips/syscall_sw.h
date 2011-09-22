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
 * $Log:	syscall_sw.h,v $
 * Revision 2.8  93/01/14  17:45:40  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 	Fixes for ANSI CPP.
 * 	[92/10/06            jvh]
 * 
 * Revision 2.7  91/05/14  16:57:28  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/05/13  06:03:27  af
 * 	Added author note.
 * 	[91/05/12  15:54:25  af]
 * 
 * Revision 2.5.1.1  91/02/21  18:46:58  af
 * 	Added author note.
 * 
 * 
 * Revision 2.5  91/02/05  17:34:49  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:12:51  mrt]
 * 
 * Revision 2.4  90/06/02  14:59:19  rpd
 * 	Added kernel_trap_7.
 * 	[90/03/26  22:37:28  rpd]
 * 
 * Revision 2.3  90/05/29  18:36:54  rwd
 * 	Added support for 11 arguments to a syscall.
 * 	[90/05/22            rwd]
 * 
 * Revision 2.2  89/11/29  14:09:51  af
 * 	Adapted for pure kernel.
 * 	[89/10/28  09:56:54  af]
 * 
 * Revision 2.1  89/05/30  16:55:46  rvb
 * Created.
 * 
 * 12-Jan-89  Alessandro Forin (af) at Carnegie-Mellon University
 *	Created for Mips.
 *
 *  1-Sep-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Created from mach_syscalls.h in the user library sources.
 */

/*
 *	File: syscall_sw.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	4/90
 *
 *	Mach syscall trap argument passing convention
 */

#ifndef	_MACH_MIPS_SYSCALL_SW_H_
#define	_MACH_MIPS_SYSCALL_SW_H_	1

#include <mach/mips/asm.h>

/*
 * Unix kernels expects arguments to be passed with the normal C calling
 * sequence (a0-a3+stack), v0 contains the system call number on entry
 * and v0-v1 contain the results from the call, and a3 the success/fail.
 *
 * On Mach we pass all the arguments in registers, the trap number is in v0
 * and the return value is placed in v0.  There are no awful hacks for
 * returning multiple values from a trap.
 *
 * Performance: a trap with up to 4 args takes 4 cycles in user mode,
 * with an unfortunate and unavoidable nop instruction and no memory
 * accesses. Any arg after the fourth takes 1 more cycle to load
 * from the cache (which cannot possibly miss) into a register.
 */

/*
 * A simple trap is one with up to 4 args. Args are passed to us
 * in registers, and we keep them there.
 */
#define simple_kernel_trap(trap_name, trap_number)	 \
	.globl	trap_name;	 			 \
	.ent	trap_name,0;				 \
trap_name:;						 \
	.frame	sp,0,ra;				 \
	li	v0,trap_number;				 \
	syscall;					 \
	j	ra;					 \
	.end trap_name

#define kernel_trap_0(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)
#define kernel_trap_1(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)
#define kernel_trap_2(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)
#define kernel_trap_3(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)
#define kernel_trap_4(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number)

/*
 * A trap with more than 4 args requires popping of args
 * off the stack, where they are placed by the compiler
 * or by the user.
 */
#define kernel_trap_5(trap_name, trap_number)	 	 \
	.globl	trap_name; 				 \
	.ent	trap_name,0;				 \
trap_name:;						 \
	.frame	sp,0,ra;				 \
	lw	t0,16(sp);				 \
	li	v0,trap_number;				 \
	syscall;					 \
	j	ra;					 \
	.end trap_name

#define kernel_trap_6(trap_name, trap_number)	 	 \
	.globl	trap_name; 				 \
	.ent	trap_name,0;				 \
trap_name:;						 \
	.frame	sp,0,ra;				 \
	lw	t0,16(sp);				 \
	lw	t1,20(sp);				 \
	li	v0,trap_number;				 \
	syscall;					 \
	j	ra;					 \
	.end trap_name

#define kernel_trap_7(trap_name, trap_number)	 	 \
	.globl	trap_name; 				 \
	.ent	trap_name,0;				 \
trap_name:;						 \
	.frame	sp,0,ra;				 \
	lw	t0,16(sp);				 \
	lw	t1,20(sp);				 \
	lw	t2,24(sp);				 \
	li	v0,trap_number;				 \
	syscall;					 \
	j	ra;					 \
	.end trap_name

#define kernel_trap_11(trap_name, trap_number)	 	 \
	.globl	trap_name; 				 \
	.ent	trap_name,0;				 \
trap_name:;						 \
	.frame	sp,0,ra;				 \
	lw	t0,16(sp);				 \
	lw	t1,20(sp);				 \
	lw	t2,24(sp);				 \
	lw	t3,28(sp);				 \
	lw	t4,32(sp);				 \
	lw	t5,36(sp);				 \
	lw	t6,40(sp);				 \
	li	v0,trap_number;				 \
	syscall;					 \
	j	ra;					 \
	.end trap_name

/*
 * There are no Mach traps with more than 11 args.
 * If that changes, the kernel needs to be fixed also.
 */

#ifdef __STDC__
#define kernel_trap(trap_name,trap_number,nargs)	 \
	kernel_trap_ ## nargs(trap_name,trap_number)

#else /* __STDC__ */

#define kernel_trap(trap_name,trap_number,nargs)	 \
	kernel_trap_/**/nargs(trap_name,trap_number)
#endif /* __STDC__ */

#endif	/* _MACH_MIPS_SYSCALL_SW_H_ */
