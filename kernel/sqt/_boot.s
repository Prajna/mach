/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * $Log:	_boot.s,v $
 * Revision 2.3  91/05/14  17:39:49  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/05/08  12:50:41  dbg
 * 	Created.
 * 	[91/04/26  14:46:24  dbg]
 * 
 * Revision 2.1.1.1  91/02/26  10:52:23  dbg
 * 	Created.
 * 	[91/02/26            dbg]
 * 
 */

/*
 * Sequent code to bootstrap from real to protected mode.
 */
/ at 0x20
	.text
				/ segment 8: code
	.long	0x0000ffff	/ length[0..15] = FFFF
				/ base[0..15] = 0
	.long	0x00cf9a00	/ base[16..23],base[24..31] = 0
				/ limit[16..19] = F
				/ G D
				/ pl = 0; code
	.long	0x0000ffff
	.long	0x00cf9200	/ data
/ at 0x30
	.word	0x0017		/ length (GDT) - 1
	.long	0x00000018	/ address of GDT
	.word	0		/ align to long boundary
	.long	0,0,0		/ filler
/ at 0x44
/ code				/ should be at address 0x44
boot:	.byte	0x67		/ use 32-bit addressing from real mode
	.byte	0x66		/ 32-bit data from real mode
	lgdt	0x30		/ load GDT
	mov	%cr0,%ebx	/ get CR0
	orb	$0x1,%bl	/ turn on protected-mode enable
	mov	%ebx,%cr0	/ turn on protected mode
	.byte	0xea		/ 16-bit long jump
	.word	0x5b		/ to location 0x5b,
	.word	0x08		/ segment 8 (32-bit code)
/ location 0x5b
	movl	$0x10,%ebx	/ get number for data segment
///	movw	%bx,%ds		/ set up DS
	.byte	0x8e,0xdb		/ (gas always generates Data16)
///	movw	%bx,%ss		/ SS
	.byte	0x8e,0xd3		/ (gas always generates Data16)
///	movw	%bx,%es		/ and ES
	.byte	0x8e,0xc3		/ (gas always generates Data16)
	jmp	*0x6c		/ jump to start of program
/ location 0x6c
	.long	_pstart-0xC0000000
				/ physical address of pstart

/ location 0x70
	.fill	0x4000-0x70,1,0x0

