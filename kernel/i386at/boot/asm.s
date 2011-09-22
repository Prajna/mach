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
 * $Log:	asm.s,v $
 * Revision 2.3  93/05/10  17:46:52  rvb
 * 	Use C Comment
 * 	[93/05/04  17:28:59  rvb]
 * 
 * Revision 2.2  92/04/04  11:34:13  rpd
 * 	Fix Intel Copyright as per B. Davies authorization.
 * 	[92/04/03            rvb]
 * 	From 2.5 boot: pruned inb(), outb(), and pzero().
 * 	[92/03/30            rvb]
 * 
 * Revision 2.2  91/04/02  14:35:10  mbj
 * 	Added _sp() => where is the stack at. [kupfer]
 * 	Add Intel copyright
 * 	[90/02/09            rvb]
 * 
 */

/*
  Copyright 1988, 1989, 1990, 1991, 1992 
   by Intel Corporation, Santa Clara, California.

                All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Intel
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

	.file "asm.s"

#include "machine/asm.h"

BOOTSEG		=	0x100

CR0_PE_ON	=	0x1
CR0_PE_OFF	=	0xfffffffe

	.text

/*
 * real_to_prot()
 * 	transfer from real mode to protected mode.
 */

ENTRY(real_to_prot)
	/* guarantee that interrupt is disabled when in prot mode */
	cli

	/* load the gdtr */
	addr16
	data16
	lgdt	EXT(Gdtr)

	/* set the PE bit of CR0 */
	mov	%cr0, %eax

	data16
	or	$CR0_PE_ON, %eax
	mov	%eax, %cr0 

	/* make intrasegment jump to flush the processor pipeline and */
	/* reload CS register */
	data16
	ljmp	$0x08, $xprot

xprot:
	/* we are in USE32 mode now */
	/* set up the protective mode segment registers : DS, SS, ES */
	mov	$0x10, %eax
	movw	%ax, %ds
	movw	%ax, %ss
	movw	%ax, %es

	ret

/*
 * prot_to_real()
 * 	transfer from protected mode to real mode
 */

ENTRY(prot_to_real)

	/* change to USE16 mode */
	ljmp	$0x18, $x16

x16:
	/* clear the PE bit of CR0 */
	mov	%cr0, %eax
	data16
	and 	$CR0_PE_OFF, %eax
	mov	%eax, %cr0


	/* make intersegment jmp to flush the processor pipeline */
	/* and reload CS register */
	data16
	ljmp $BOOTSEG, $xreal


xreal:
	/* we are in real mode now */
	/* set up the real mode segment registers : DS, SS, ES */
	movw	%cs, %ax
	movw	%ax, %ds
	movw	%ax, %ss
	movw	%ax, %es

	data16
	ret

/*
 * startprog(phyaddr)
 *	start the program on protected mode where phyaddr is the entry point
 */

ENTRY(startprog)
	push	%ebp
	mov	%esp, %ebp

	/* push some number of args onto the stack */
	movl	0x0c(%ebp), %eax	/* &argv */
	push	0x10(%eax)		/* argv[4] = esym */
	push	0x4(%eax)		/* argv[1] = howto */
	push	0x1c(%eax)		/* argv[7] = cnvmem */
	push	0x20(%eax)		/* argv[8] = extmem */
	push	0x8(%eax)		/* argv[2] = bootdev */
	
	mov	0x8(%ebp), %ecx		/* entry offset  */
	mov	$0x28, %ebx		/* segment */
	push	%ebx
	push	%ecx

	/* set up %ds and %es */
	mov	$0x20, %ebx
	movw	%bx, %ds
	movw	%bx, %es

	lret

/*
 * pcpy(src, dst, cnt)
 *	where src is a virtual address and dst is a physical address
*/

ENTRY(pcpy)
	push	%ebp
	mov	%esp, %ebp
	push	%es
	push	%esi
	push	%edi
	push	%ecx

	cld

	/* set %es to point at the flat segment */
	mov	$0x20, %eax
	movw	%ax, %es

	mov	0x8(%ebp), %esi		/* source */
	mov	0xc(%ebp), %edi		/* destination */
	mov	0x10(%ebp), %ecx	/* count */

	rep
	movsb

	pop	%ecx
	pop	%edi
	pop	%esi
	pop	%es
	pop	%ebp

	ret
