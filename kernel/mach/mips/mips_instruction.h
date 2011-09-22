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
 * $Log:	mips_instruction.h,v $
 * Revision 2.6  93/01/14  17:45:36  danner
 * 	Protected against multiple inclusion.
 * 	[92/06/10            pds]
 * 
 * Revision 2.5  91/05/14  16:57:18  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/05/13  06:03:22  af
 * 	Added author note.
 * 	[91/05/12  15:54:42  af]
 * 
 * Revision 2.3.1.1  91/02/21  18:46:24  af
 * 	Added author note.
 * 
 * 
 * Revision 2.3  91/02/05  17:34:44  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:12:42  mrt]
 * 
 * Revision 2.2  89/11/29  14:09:47  af
 * 	Created.
 * 	[89/10/05            af]
 * 
 */

/*
 *	File: mips_instruction.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	MIPS Instruction set definition
 *
 *	Reference: G. Kane "MIPS RISC Architecture", Prentice Hall
 *
 */

#ifndef	_MACH_MIPS_MIPS_INSTRUCTION_H_
#define	_MACH_MIPS_MIPS_INSTRUCTION_H_

#if	!defined(ASSEMBLER)

/*
 *	All instructions are in one of three formats:
 *		Immediate, Jump, Register
 *
 *	Floating point instructions are described here
 *	as implemented in the MIPS R2010 FPA.
 *		
 *	Now if we only could avoid littering it all with
 *	stupid byteorder issues..
 *
 */


typedef union {
	/*
	 *	All instructions are 32 bits wide
	 */
	unsigned int	bits;

	/*
	 *	Immediate instructions contain a 16 bit
	 *	immediate value, which could either be
	 *	used as a signed or unsigned integer.
	 */
	struct {
#if	BYTE_MSF
		unsigned	opcode : 6,
				rs : 5,
				rt : 5;
		union {
			signed short	s_val;
			unsigned short	u_val;
		}	immediate;
#else	/* BYTE_MSF */
		union {
			signed short	s_val;
			unsigned short	u_val;
		}	immediate;
		unsigned	rt : 5,
				rs : 5,
				opcode : 6;
#endif	/* BYTE_MSF */
	} i_format;
#	define	simmediate	immediate.s_val
#	define	uimmediate	immediate.u_val

	/*
	 *	Jump instruction contain a 26 bit target,
	 *	which is shifted and combined with the PC
	 *	to form a 32 bit destination address (in
	 *	jump instructions)
	 */
	struct {
#if	BYTE_MSF
		unsigned	opcode : 6,
				target : 26;
#else	/* BYTE_NSF */
		unsigned	target : 26,
				opcode : 6;
#endif	/* BYTE_MSF */
	} j_format;

	/*
	 *	A variety of instructions are of the Register type
	 */
	struct {
#if	BYTE_MSF
		unsigned	opcode : 6,
				rs : 5,
				rt : 5,
				rd : 5,
				shamt : 5,
				func : 6;
#else	/* BYTE_MSF */
		unsigned	func : 6,
				shamt : 5,
				rd : 5,
				rt : 5,
				rs : 5,
				opcode : 6;
#endif	/* BYTE_MSF */
	} r_format;

	/*
	 *	Many floating point instruction are of the r_format
	 *	type but with a wired bit in the 'rs' field.
	 */
	struct {
#if	BYTE_MSF
		unsigned	opcode : 6,
				: 1,		/* wired 1 */
				fmt : 4,
				ft : 5,
				fs : 5,
				fd : 5,
				func : 6;
#else	/* BYTE_MSF */
		unsigned	func : 6,
				fd : 5,
				fs : 5,
				ft : 5,
				fmt : 4,
				: 1,		/* wired 1 */
				opcode : 6;
#endif	/* BYTE_MSF */
	} f_format;
} mips_instruction;

#endif	/* !defined(ASSEMBLER) */

/*
 *
 *	Encoding of regular instructions  (pag. A-87 op cit)
 *
 */

		/* OPCODE, bits 26..31 */


#define	op_special	0x0		/* see SPECIAL sub-table */
#define op_bcond	0x1
#define op_j		0x2
#define op_jal		0x3
#define op_beq		0x4
#define op_bne		0x5
#define op_blez		0x6
#define op_bgtz		0x7
#define op_addi		0x8
#define op_addiu	0x9
#define op_slti		0xa
#define op_sltiu	0xb
#define op_andi		0xc
#define op_ori		0xd
#define op_xori		0xe
#define op_lui		0xf
#define op_cop0		0x10
#define op_cop1		0x11
#define op_cop2		0x12
#define op_cop3		0x13
			/* 0x14..0x1f reserved */
#define op_lb		0x20
#define op_lh		0x21
#define op_lwl		0x22
#define op_lw		0x23
#define op_lbu		0x24
#define op_lhu		0x25
#define op_lwr		0x26
			/* 0x27 reserved */
#define op_sb		0x28
#define op_sh		0x29
#define op_swl		0x2a
#define op_sw		0x2b
			/* 0x2c..0x2d reserved */
#define op_swr		0x2e
			/* 0x2f reserved */
#define op_lwc0		0x30
#define op_lwc1		0x31
#define op_lwc2		0x32
#define op_lwc3		0x33
			/* 0x34..0x37 reserved */
#define op_swc0		0x38
#define op_swc1		0x39
#define op_swc2		0x3a
#define op_swc3		0x3b
			/* 0x3e..0x3f reserved */

		/* SPECIAL, "func" opcodes (bits 0..5)  */


#define op_sll		0x0
			/* 0x1 reserved */
#define op_srl		0x2
#define op_sra		0x3
#define op_sllv		0x4
			/* 0x5 reserved */
#define op_srlv		0x6
#define op_srav		0x7
#define op_jr		0x8
#define op_jalr		0x9
			/* 0xa..0xb reserved */
#define op_syscall	0xc
#define op_break	0xd
#	define	BREAK_USER	0	/* User level breakpoint */
#	define	BREAK_KERNEL	1	/* kernel breakpoint */
#	define	BREAK_SSTEP	5	/* single-step breakpoint */
#	define	BREAK_OVERFLOW	6	/* arithmetic overflow check */
#	define	BREAK_DIVZERO	7	/* divide by zero check */
#	define	BREAK_RANGE	8	/* range error check */
			/* 0xe..0xf reserved, but Mach steals one: */
#define op_tas		0xf
#define op_mfhi		0x10
#define op_mthi		0x11
#define op_mflo		0x12
#define op_mtlo		0x13
			/* 0x14..0x17 reserved */
#define op_mult		0x18
#define op_multu	0x19
#define op_div		0x1a
#define op_divu		0x1b
			/* 0x1c..0x1f reserved */
#define op_add		0x20
#define op_addu		0x21
#define op_sub		0x22
#define op_subu		0x23
#define op_and		0x24
#define op_or		0x25
#define op_xor		0x26
#define op_nor		0x27
			/* 0x28..0x29 reserved */
#define op_slt		0x2a
#define op_sltu		0x2b
			/* 0x2c..0x3f reserved */


		/* BCOND, "rt" opcodes (bits 16..20) */

#define op_bltz		0x0
#define op_bgez		0x1
#define op_bltzal	0x10
#define op_bgezal	0x11


		/* COP0, "func" opcodes (bits 0..4) */

#define op_tlbr		0x1
#define op_tlbwi	0x2
#define op_tlbwr	0x6
#define op_tlbp		0x8
#define op_rfe		0x10


		/* COPz, "rs" opcodes (bits 21..25) */

#define op_mfc		0x0
#define op_cfc		0x2
#define op_mtc		0x4
#define op_ctc		0x6
#define op_bc		0x8	/* same as 0xc
				   Bit 16 selects 1 -> bct, 0 -> bcf */
#define	op_BC		0xc


/*
 *
 *	Encoding of floating point instructions (pag. B-28 op cit)
 *
 *	Load and store operations use opcodes op_lwc1/op_swc1
 */

		/* FMT, precision qualifier (bits 21..24) */

#define fmt_single	0x0
#define	fmt_double	0x1
#define fmt_word	0x4

		/* COP1, "func" opcodes (bits 0..5) */

#define	op_fadd		0x0
#define	op_fsub		0x1
#define	op_fmul		0x2
#define	op_fdiv		0x3
#define	op_fabs		0x5
#define	op_fmov		0x6
#define	op_fneg		0x7
#define	op_cvts		0x20
#define	op_cvtd		0x21
#define	op_cvtw		0x24

#define	op_fcomp_f	0x30
#define	op_fcomp_un	0x31
#define	op_fcomp_eq	0x32
#define	op_fcomp_ueq	0x33
#define	op_fcomp_olt	0x34
#define	op_fcomp_ult	0x35
#define	op_fcomp_ole	0x36
#define	op_fcomp_ule	0x37
#define	op_fcomp_sf	0x38
#define	op_fcomp_ngle	0x39
#define	op_fcomp_seq	0x3a
#define	op_fcomp_ngl	0x3b
#define	op_fcomp_lt	0x3c
#define	op_fcomp_nge	0x3d
#define	op_fcomp_le	0x3e
#define	op_fcomp_ngt	0x3f

#endif	/* _MACH_MIPS_MIPS_INSTRUCTION_H_ */
