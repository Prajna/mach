/* 
 * Mach Operating System
 * Copyright (c) 1992,1991,1990,1989 Carnegie Mellon University
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
 * $Log:	asm.h,v $
 * Revision 2.7  93/01/14  17:45:09  danner
 * 	Protected against multiple inclusion.
 * 	[92/06/10            pds]
 * 	Fixes for ANSI CPP.
 * 	[92/10/06            jvh]
 * 
 * Revision 2.6  91/05/14  16:56:42  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/05/13  06:03:06  af
 * 	Added author note.
 * 	[91/05/12  15:55:29  af]
 * 
 * Revision 2.4.1.1  91/02/21  18:35:36  af
 * 	Added author note.
 * 
 * 
 * Revision 2.4  91/02/05  17:34:25  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:12:05  mrt]
 * 
 * Revision 2.3  91/01/08  15:18:45  rpd
 * 	Changed EF prefix to MSS.
 * 	[90/12/30            rpd]
 * 
 * Revision 2.2  89/11/29  14:09:35  af
 * 	Added floating point control register defs.
 * 	[89/10/28  10:15:44  af]
 * 
 * 	Created for pure kernel, compatible with MIPSCo's defs.
 * 	[89/10/28  09:54:28  af]
 * 
 */
/*
 *	File: asm.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/90
 *
 *	Assembly coding style
 *
 *	This file contains macros and register defines to
 *	aid in writing more readable assembly code.
 *	Some rules to make assembly code understandable by
 *	a debugger are also noted.
 */

#ifndef	_MACH_MIPS_ASM_H_
#define	_MACH_MIPS_ASM_H_

/*
 *	Symbolic register names
 */
#define zero	$0	/* wired zero */
#define AT	$1	/* assembler temp (uprcase, because ".set at") */
#define v0	$2	/* return value */
#define v1	$3
#define a0	$4	/* argument registers */
#define a1	$5
#define a2	$6
#define a3	$7
#define t0	$8	/* caller saved */
#define t1	$9
#define t2	$10
#define t3	$11
#define t4	$12
#define t5	$13
#define t6	$14
#define t7	$15
#define s0	$16	/* callee saved */
#define s1	$17
#define s2	$18
#define s3	$19
#define s4	$20
#define s5	$21
#define s6	$22
#define s7	$23
#define t8	$24	/* caller saved */
#define t9	$25
#define k0	$26	/* kernel scratch */
#define k1	$27
#define gp	$28	/* global pointer */
#define sp	$29	/* stack pointer */
#define fp	$30	/* frame pointer */
#define ra	$31	/* return address */

#ifndef fpa_csr
#define	fpa_irr			$0		/* FPA Impl&Rev Register */
#define	fpa_eir			$30		/* FPA Exc Inst Register */
#define fpa_csr			$31		/* FPA Status Register */
#endif


/*
 *	Byte-order independent definition for
 *	unaligned memory accesses
 */
#if	BYTE_MSF
#define	lw_hi	lwl
#define	lw_lo	lwr
#define	sw_hi	swl
#define	sw_lo	swr
#else	/* BYTE_MSF */
#define	lw_hi	lwr
#define	lw_lo	lwl
#define	sw_hi	swr
#define	sw_lo	swl
#endif	/* BYTE_MSF */

/*
 *
 * Debuggers need symbol table information to be able to properly
 * decode a stack trace.  The minimum that should be provided is
 *
 *		.ent	name,lex-level
 * 	name:
 *		.frame	fp,framesize,saved_pc
 *
 * where "name" 	is the function's name,
 *	 "lex-level"	is 0 for C,
 *	 "fp" 		is the pointer (register) to the base of the
 * 			previous frame. If this is the sp register
 *			(and it usually is) the debugger will use
 *			"sp + framesize" instead.
 *	 "framesize"	is the size of the frame for this function.
 *			That is:
 *				new_sp + framesize == old_sp
 *	 "saved_pc"	is either a register which preserves the caller's PC
 *			or 'zero', if zero the saved PC should be stored at
 *				(fp)+framesize-4
 *
 * NESTED functions should also declare which registers they save
 * in a ".mask" statement:
 *
 *	.mask	regsave,displ
 *
 * where "regsave"	is a bitmask that indicates which of the registers
 *			are saved. See the M_xx defines at the end.
 *	 "displ"	is the displacement from the top of the current
 *			frame where registers are stored, in descending
 *			order. Usually RA is the one at the top.
 *
 * If you need to alias a leaf function, or to provide multiple entry points
 * use the LEAF() macro for the main entry point and XLEAF() for the other
 * additional/alternate entry points.
 * "XLEAF"s must be nested within a "LEAF" and a ".end".
 * Similar rules for nested routines, e.g. use NESTED/XNESTED
 * Symbols that should not be exported can be declared with the STATIC_xxx
 * macros.
 *
 * All functions must be terminated by the END macro
 *
 * All symbols are internal unless EXPORTed.  Symbols that are IMPORTed
 * must be appropriately described to the debugger.
 *
 * The kernel expects arguments of a SYSCALL to be passed with the normal
 * C calling sequence.  v0 should contain the system call number.
 * On return from kernel mode, a3 will be 0 to indicate no error and non-zero
 * to indicate an error; if an error occurred v0 will contain the errno.
 *
 */

/*
 * LEAF
 *	Declare a global leaf function.
 *	A leaf function does not call other functions AND does not
 *	use any register that is callee-saved AND does not modify
 *	the stack pointer.
 */
#define	LEAF(_name_)						\
	.globl	_name_;						\
	.ent	_name_,0;					\
_name_:;							\
	.frame	sp,0,ra

/*
 * STATIC_LEAF
 *	Declare a local leaf function.
 */
#define STATIC_LEAF(_name_)					\
	.ent	_name_,0;					\
_name_:;							\
	.frame	sp,0,ra

/*
 * XLEAF
 *	Global alias for a leaf function, or alternate entry point
 */
#define	XLEAF(_name_)						\
	.globl	_name_;						\
	.aent	_name_,0;					\
_name_:

/*
 * STATIC_XLEAF
 *	Local alias for a leaf function, or alternate entry point
 */
#define	STATIC_XLEAF(_name_)					\
	.aent	_name_,0;					\
_name_:

/*
 * NESTED
 *	Declare a (global) nested function
 *	A nested function calls other functions and needs
 *	therefore stack space to save/restore registers.
 */
#define	NESTED(_name_, _framesize_, _saved_pc_)			\
	.globl	_name_;						\
	.ent	_name_,0;					\
_name_:;							\
	.frame	sp,_framesize_, _saved_pc_

/*
 * STATIC_NESTED
 *	Declare a local nested function.
 */
#define	STATIC_NESTED(_name_, _framesize_, _saved_pc_)		\
	.ent	_name_,0;					\
_name_:;							\
	.frame	sp,_framesize_, _saved_pc_

/*
 * XNESTED
 *	Same as XLEAF, for a nested function.
 */
#define	XNESTED(_name_)						\
	.globl	_name_;						\
	.aent	_name_,0;					\
_name_:


/*
 * STATIC_XNESTED
 *	Same as STATIC_XLEAF, for a nested function.
 */
#define	STATIC_XNESTED(_name_)					\
	.aent	_name_,0;					\
_name_:


/*
 * END
 *	Function delimiter
 */
#define	END(_name_)						\
	.end	_name_


/*
 * CALL
 *	Function invocation
 */
#define	CALL(_name_)						\
	jal	_name_


/*
 * RET
 *	Return from function
 */
#define	RET							\
	j	ra


/*
 * EXPORT
 *	Export a symbol
 */
#define	EXPORT(_name_)						\
	.globl	_name_;						\
_name_:


/*
 * IMPORT
 *	Make an external name visible, typecheck the size
 */
#define	IMPORT(_name_, _size_)					\
	.extern	_name_,_size_


/*
 * ABS
 *	Define an absolute symbol
 */
#define	ABS(_name_, _value_)					\
	.globl	_name_;						\
_name_	=	_value_


/*
 * BSS
 *	Allocate un-initialized space for a global symbol
 */
#define	BSS(_name_,_numbytes_)					\
	.comm	_name_,_numbytes_


/*
 * LBSS
 *	Allocate un-initialized space for a local symbol
 */
#define	LBSS(_name_,_numbytes_)					\
	.lcomm	_name_,_numbytes_


/*
 * SYSCALL
 *	Simple system call sequence (needs /usr/include/syscall.h)
 */
#ifdef	__STDC__
#define	SYSCALL(_name_)						\
LEAF(_name_);							\
	li	v0,SYS_ ## _name_;				\
	syscall;						\
	beq	a3,zero,9f;					\
	j	_cerror;					\
9:
#else	/* __STDC__ */
#define	SYSCALL(_name_)						\
LEAF(_name_);							\
	li	v0,SYS_/**/_name_;				\
	syscall;						\
	beq	a3,zero,9f;					\
	j	_cerror;					\
9:
#endif	/* __STDC__ */

/*
 * PSEUDO
 *	System call sequence for syscalls that are variations
 *	of other system calls
 */
#define	PSEUDO(_name_,_alias_)					\
LEAF(_name_);							\
	li	v0,SYS_/**/_alias_;				\
	syscall


/*
 * VECTOR
 *	Make an exception entry point look like a called function,
 *	to make it digestible to the debugger (KERNEL only)
 */
#define	VECTOR(_name_, _mask_)					\
	.globl	_name_;						\
	.ent	_name_,0;					\
_name_:;							\
	.frame	sp,MSS_SIZE,$0;					\
	.mask	_mask_,-(MSS_SIZE-(MSS_RA*4))

/*
 * MSG
 *	Allocate space for a message (an ascii string)
 */
#define	MSG(msg)						\
	.rdata;							\
9:	.asciiz	msg;						\
	.text

/*
 * PRINTF
 *	Print a message
 */
#define	PRINTF(msg)						\
	la	a0,9f;						\
	jal	printf;						\
	MSG(msg)

/*
 * PANIC
 *	Fatal error (KERNEL)
 */
#define	PANIC(msg)						\
	la	a0,9f;						\
	jal	panic;						\
	MSG(msg)


/*
 * 	Bit definitions for composing register masks
 */
#define	M_EXCFRM	0x00000001
#define	M_AT		0x00000002
#define	M_V0		0x00000004
#define	M_V1		0x00000008
#define	M_A0		0x00000010
#define	M_A1		0x00000020
#define	M_A2		0x00000040
#define	M_A3		0x00000080
#define	M_T0		0x00000100
#define	M_T1		0x00000200
#define	M_T2		0x00000400
#define	M_T3		0x00000800
#define	M_T4		0x00001000
#define	M_T5		0x00002000
#define	M_T6		0x00004000
#define	M_T7		0x00008000
#define	M_S0		0x00010000
#define	M_S1		0x00020000
#define	M_S2		0x00040000
#define	M_S3		0x00080000
#define	M_S4		0x00100000
#define	M_S5		0x00200000
#define	M_S6		0x00400000
#define	M_S7		0x00800000
#define	M_T8		0x01000000
#define	M_T9		0x02000000
#define	M_K0		0x04000000
#define	M_K1		0x08000000
#define	M_GP		0x10000000
#define	M_SP		0x20000000
#define	M_FP		0x40000000
#define	M_RA		0x80000000

#endif	/* _MACH_MIPS_ASM_H_ */
