/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 * $Log:	idt.s,v $
 * Revision 2.8  92/04/01  19:32:10  rpd
 * 	Changed $0 to $(0) for ANSI cpp.
 * 	[92/03/25            jvh]
 * 
 * Revision 2.7  92/01/03  20:06:22  dbg
 * 	Segment-not-present fault may occur during kernel exit sequence.
 * 	[91/10/29            dbg]
 * 
 * Revision 2.6  91/07/31  17:37:02  dbg
 * 	Save fewer registers on interrupt entry.
 * 	[91/07/30  16:51:33  dbg]
 * 
 * Revision 2.5  91/05/14  16:08:54  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/05/08  12:38:06  dbg
 * 	Put parentheses around substituted immediate expressions, so
 * 	that they will pass through the GNU preprocessor.
 * 	[91/04/26  14:35:28  dbg]
 * 
 * Revision 2.3  91/02/05  17:12:12  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:34:44  mrt]
 * 
 * Revision 2.2  90/08/27  21:56:52  dbg
 * 	Created.
 * 	[90/07/25            dbg]
 * 
 */
#include <machine/asm.h>
#include <assym.s>

/*
 * Interrupt descriptor table and code vectors for it.
 */
#define	IDT_ENTRY(vec,type) \
	.data	2		;\
	.long	vec		;\
	.word	KERNEL_CS	;\
	.byte	0		;\
	.byte	type		;\
	.text

/*
 * No error code.  Clear error code and push trap number.
 */
#define	EXCEPTION(n,name) \
	IDT_ENTRY(EXT(name),K_TRAP_GATE);\
ENTRY(name)				;\
	pushl	$(0)			;\
	pushl	$(n)			;\
	jmp	_alltraps

/*
 * Interrupt from user.  Clear error code and push trap number.
 */
#define	EXCEP_USR(n,name) \
	IDT_ENTRY(EXT(name),U_TRAP_GATE);\
ENTRY(name)				;\
	pushl	$(0)			;\
	pushl	$(n)			;\
	jmp	_alltraps

/*
 * Special interrupt code.
 */
#define	EXCEP_SPC(n,name)  \
	IDT_ENTRY(EXT(name),K_TRAP_GATE)

/*
 * Error code has been pushed.  Push trap number.
 */
#define	EXCEP_ERR(n,name) \
	IDT_ENTRY(EXT(name),K_TRAP_GATE);\
ENTRY(name)				;\
	pushl	$(n)			;\
	jmp	_alltraps

/*
 * Interrupt.
 */
#define	INTERRUPT(n) \
	IDT_ENTRY(0f,K_INTR_GATE)	;\
0:					;\
	pushl	%eax			;\
	movl	$(n),%eax		;\
	jmp	_all_intrs

	.data	2
ENTRY(idt)
	.text

EXCEPTION(0x00,t_zero_div)
EXCEP_SPC(0x01,t_debug)
INTERRUPT(0x02)			/* NMI */
EXCEP_USR(0x03,t_int3)
EXCEP_USR(0x04,t_into)
EXCEP_USR(0x05,t_bounds)
EXCEPTION(0x06,t_invop)
EXCEPTION(0x07,t_nofpu)
EXCEPTION(0x08,a_dbl_fault)
EXCEPTION(0x09,a_fpu_over)
EXCEPTION(0x0a,a_inv_tss)
EXCEP_SPC(0x0b,t_segnp)
EXCEP_ERR(0x0c,t_stack_fault)
EXCEP_SPC(0x0d,t_gen_prot)
EXCEP_SPC(0x0e,t_page_fault)
EXCEPTION(0x0f,t_trap_0f)
EXCEPTION(0x10,t_fpu_err)
EXCEPTION(0x11,t_trap_11)
EXCEPTION(0x12,t_trap_12)
EXCEPTION(0x13,t_trap_13)
EXCEPTION(0x14,t_trap_14)
EXCEPTION(0x15,t_trap_15)
EXCEPTION(0x16,t_trap_16)
EXCEPTION(0x17,t_trap_17)
EXCEPTION(0x18,t_trap_18)
EXCEPTION(0x19,t_trap_19)
EXCEPTION(0x1a,t_trap_1a)
EXCEPTION(0x1b,t_trap_1b)
EXCEPTION(0x1c,t_trap_1c)
EXCEPTION(0x1d,t_trap_1d)
EXCEPTION(0x1e,t_trap_1e)
EXCEPTION(0x1f,t_trap_1f)

INTERRUPT(0x20)
INTERRUPT(0x21)
INTERRUPT(0x22)
INTERRUPT(0x23)
INTERRUPT(0x24)
INTERRUPT(0x25)
INTERRUPT(0x26)
INTERRUPT(0x27)
INTERRUPT(0x28)
INTERRUPT(0x29)
INTERRUPT(0x2a)
INTERRUPT(0x2b)
INTERRUPT(0x2c)
INTERRUPT(0x2d)
INTERRUPT(0x2e)
INTERRUPT(0x2f)

INTERRUPT(0x30)
INTERRUPT(0x31)
INTERRUPT(0x32)
INTERRUPT(0x33)
INTERRUPT(0x34)
INTERRUPT(0x35)
INTERRUPT(0x36)
INTERRUPT(0x37)
INTERRUPT(0x38)
INTERRUPT(0x39)
INTERRUPT(0x3a)
INTERRUPT(0x3b)
INTERRUPT(0x3c)
INTERRUPT(0x3d)
INTERRUPT(0x3e)
INTERRUPT(0x3f)

INTERRUPT(0x40)
INTERRUPT(0x41)
INTERRUPT(0x42)
INTERRUPT(0x43)
INTERRUPT(0x44)
INTERRUPT(0x45)
INTERRUPT(0x46)
INTERRUPT(0x47)
INTERRUPT(0x48)
INTERRUPT(0x49)
INTERRUPT(0x4a)
INTERRUPT(0x4b)
INTERRUPT(0x4c)
INTERRUPT(0x4d)
INTERRUPT(0x4e)
INTERRUPT(0x4f)

INTERRUPT(0x50)
INTERRUPT(0x51)
INTERRUPT(0x52)
INTERRUPT(0x53)
INTERRUPT(0x54)
INTERRUPT(0x55)
INTERRUPT(0x56)
INTERRUPT(0x57)
INTERRUPT(0x58)
INTERRUPT(0x59)
INTERRUPT(0x5a)
INTERRUPT(0x5b)
INTERRUPT(0x5c)
INTERRUPT(0x5d)
INTERRUPT(0x5e)
INTERRUPT(0x5f)

INTERRUPT(0x60)
INTERRUPT(0x61)
INTERRUPT(0x62)
INTERRUPT(0x63)
INTERRUPT(0x64)
INTERRUPT(0x65)
INTERRUPT(0x66)
INTERRUPT(0x67)
INTERRUPT(0x68)
INTERRUPT(0x69)
INTERRUPT(0x6a)
INTERRUPT(0x6b)
INTERRUPT(0x6c)
INTERRUPT(0x6d)
INTERRUPT(0x6e)
INTERRUPT(0x6f)

INTERRUPT(0x70)
INTERRUPT(0x71)
INTERRUPT(0x72)
INTERRUPT(0x73)
INTERRUPT(0x74)
INTERRUPT(0x75)
INTERRUPT(0x76)
INTERRUPT(0x77)
INTERRUPT(0x78)
INTERRUPT(0x79)
INTERRUPT(0x7a)
INTERRUPT(0x7b)
INTERRUPT(0x7c)
INTERRUPT(0x7d)
INTERRUPT(0x7e)
INTERRUPT(0x7f)

INTERRUPT(0x80)
INTERRUPT(0x81)
INTERRUPT(0x82)
INTERRUPT(0x83)
INTERRUPT(0x84)
INTERRUPT(0x85)
INTERRUPT(0x86)
INTERRUPT(0x87)
INTERRUPT(0x88)
INTERRUPT(0x89)
INTERRUPT(0x8a)
INTERRUPT(0x8b)
INTERRUPT(0x8c)
INTERRUPT(0x8d)
INTERRUPT(0x8e)
INTERRUPT(0x8f)

INTERRUPT(0x90)
INTERRUPT(0x91)
INTERRUPT(0x92)
INTERRUPT(0x93)
INTERRUPT(0x94)
INTERRUPT(0x95)
INTERRUPT(0x96)
INTERRUPT(0x97)
INTERRUPT(0x98)
INTERRUPT(0x99)
INTERRUPT(0x9a)
INTERRUPT(0x9b)
INTERRUPT(0x9c)
INTERRUPT(0x9d)
INTERRUPT(0x9e)
INTERRUPT(0x9f)

INTERRUPT(0xa0)
INTERRUPT(0xa1)
INTERRUPT(0xa2)
INTERRUPT(0xa3)
INTERRUPT(0xa4)
INTERRUPT(0xa5)
INTERRUPT(0xa6)
INTERRUPT(0xa7)
INTERRUPT(0xa8)
INTERRUPT(0xa9)
INTERRUPT(0xaa)
INTERRUPT(0xab)
INTERRUPT(0xac)
INTERRUPT(0xad)
INTERRUPT(0xae)
INTERRUPT(0xaf)

INTERRUPT(0xb0)
INTERRUPT(0xb1)
INTERRUPT(0xb2)
INTERRUPT(0xb3)
INTERRUPT(0xb4)
INTERRUPT(0xb5)
INTERRUPT(0xb6)
INTERRUPT(0xb7)
INTERRUPT(0xb8)
INTERRUPT(0xb9)
INTERRUPT(0xba)
INTERRUPT(0xbb)
INTERRUPT(0xbc)
INTERRUPT(0xbd)
INTERRUPT(0xbe)
INTERRUPT(0xbf)

INTERRUPT(0xc0)
INTERRUPT(0xc1)
INTERRUPT(0xc2)
INTERRUPT(0xc3)
INTERRUPT(0xc4)
INTERRUPT(0xc5)
INTERRUPT(0xc6)
INTERRUPT(0xc7)
INTERRUPT(0xc8)
INTERRUPT(0xc9)
INTERRUPT(0xca)
INTERRUPT(0xcb)
INTERRUPT(0xcc)
INTERRUPT(0xcd)
INTERRUPT(0xce)
INTERRUPT(0xcf)

INTERRUPT(0xd0)
INTERRUPT(0xd1)
INTERRUPT(0xd2)
INTERRUPT(0xd3)
INTERRUPT(0xd4)
INTERRUPT(0xd5)
INTERRUPT(0xd6)
INTERRUPT(0xd7)
INTERRUPT(0xd8)
INTERRUPT(0xd9)
INTERRUPT(0xda)
INTERRUPT(0xdb)
INTERRUPT(0xdc)
INTERRUPT(0xdd)
INTERRUPT(0xde)
INTERRUPT(0xdf)

INTERRUPT(0xe0)
INTERRUPT(0xe1)
INTERRUPT(0xe2)
INTERRUPT(0xe3)
INTERRUPT(0xe4)
INTERRUPT(0xe5)
INTERRUPT(0xe6)
INTERRUPT(0xe7)
INTERRUPT(0xe8)
INTERRUPT(0xe9)
INTERRUPT(0xea)
INTERRUPT(0xeb)
INTERRUPT(0xec)
INTERRUPT(0xed)
INTERRUPT(0xee)
INTERRUPT(0xef)

INTERRUPT(0xf0)
INTERRUPT(0xf1)
INTERRUPT(0xf2)
INTERRUPT(0xf3)
INTERRUPT(0xf4)
INTERRUPT(0xf5)
INTERRUPT(0xf6)
INTERRUPT(0xf7)
INTERRUPT(0xf8)
INTERRUPT(0xf9)
INTERRUPT(0xfa)
INTERRUPT(0xfb)
INTERRUPT(0xfc)
INTERRUPT(0xfd)
INTERRUPT(0xfe)
INTERRUPT(0xff)

