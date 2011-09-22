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
 * Revision 2.3  93/03/09  10:55:56  danner
 * 	Changed .ent stmts to use spaces.
 * 	[93/02/15            af]
 * 
 * Revision 2.2  93/01/14  17:41:03  danner
 * 	Created.
 * 	[91/12/29            af]
 * 
 */

/*
 *	File:	alpha/syscall_sw.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	12/91
 *
 *	Mach syscall trap argument passing convention, on Alpha
 */

#ifndef	_MACH_ALPHA_SYSCALL_SW_H_
#define	_MACH_ALPHA_SYSCALL_SW_H_	1

#include <mach/alpha/asm.h>

/*
 * Unix kernels expects arguments to be passed with the normal C calling
 * sequence (a0-a5+stack), v0 contains the system call number on entry
 * and v0-at contain the results from the call, and a3 the success/fail.
 *
 * On Mach we pass all the arguments in registers, the trap number is in v0
 * and the return value is placed in v0.  There are no awful hacks for
 * returning multiple values from a trap.
 *
 * Performance: a trap with up to 6 args takes 3 cycles in user mode,
 * with no memory accesses. Any arg after the sixth takes 1 more cycle
 * to load from the cache (which cannot possibly miss) into a register.
 */

/*
 * A simple trap is one with up to 6 args. Args are passed to us
 * in registers, and we keep them there.
 */
#define simple_kernel_trap(trap_name, trap_number, nargs)	\
	.globl	trap_name;	 			 	\
	.ent	trap_name 0;				 	\
trap_name:;						 	\
	.frame	sp,0,ra;				 	\
	lda	v0,trap_number(zero);			 	\
	call_pal 0x83;					 	\
	RET;						 	\
	.end trap_name

#define kernel_trap_0(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number,0)
#define kernel_trap_1(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number,1)
#define kernel_trap_2(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number,2)
#define kernel_trap_3(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number,3)
#define kernel_trap_4(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number,4)
#define kernel_trap_5(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number,5)
#define kernel_trap_6(trap_name,trap_number)		 \
	simple_kernel_trap(trap_name,trap_number,6)

/*
 * A trap with more than 6 args requires popping of args
 * off the stack, where they are placed by the compiler
 * or by the user.
 */
#define kernel_trap_7(trap_name, trap_number)	 	 	\
	.globl	trap_name; 				 	\
	.ent	trap_name 0;			 		\
trap_name:;						 	\
	.frame	sp,0,ra;				 	\
	ldq	t0,0(sp);				 	\
	lda	v0,trap_number(zero);			 	\
	call_pal 0x83;					 	\
	RET;						 	\
	.end trap_name

#define kernel_trap_11(trap_name, trap_number)	 	 	\
	.globl	trap_name; 				 	\
	.ent	trap_name 0;			 		\
trap_name:;						 	\
	.frame	sp,0,ra;				 	\
	ldq	t0,0(sp);				 	\
	ldq	t1,8(sp);				 	\
	ldq	t2,16(sp);				 	\
	ldq	t3,24(sp);				 	\
	ldq	t4,32(sp);				 	\
	lda	v0,trap_number(zero);			 	\
	call_pal 0x83;					 	\
	RET;						 	\
	.end trap_name

/*
 * There are no Mach traps with more than 11 args.
 * If that changes, the kernel needs to be fixed also.
 */

#ifdef	__STDC__
#define kernel_trap(trap_name,trap_number,nargs)	 \
	kernel_trap_ ## nargs(trap_name,trap_number)
#else	/* __STDC__ */
#define kernel_trap(trap_name,trap_number,nargs)	 \
	kernel_trap_/**/nargs(trap_name,trap_number)
#endif

#endif	_MACH_ALPHA_SYSCALL_SW_H_
