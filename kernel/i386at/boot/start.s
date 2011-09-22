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
 * $Log:	start.s,v $
 * Revision 2.4.1.3  94/05/21  13:27:21  rvb
 * 	Allow area for Default boot string at loc 0x100.
 * 
 * Revision 2.4.1.2  94/03/18  13:42:34  rvb
 * 	bigger image
 * 
 * Revision 2.4.1.1  94/02/03  13:29:00  rvb
 * 	Never initialized fd head before.
 * 	Assume %dl passed in with boot drive id.  The int 21
 * 	we did before was a noop.
 * 	[94/02/03            rvb]
 * 
 * Revision 2.4  93/08/10  15:56:59  mrt
 * 	Leave a seperator between $ and EXT to make gcc cpp happy.
 * 	[93/07/09            rvb]
 * 
 * Revision 2.3  93/05/10  17:47:07  rvb
 * 	Leave sector #1 empty so 386bsd can store its label there.
 * 	[93/04/01            rvb]
 * 
 * Revision 2.2  92/04/04  11:36:29  rpd
 * 	Fix Intel Copyright as per B. Davies authorization.
 * 	[92/04/03            rvb]
 * 	Need to zero dh on hd path; at least for an adaptec card.
 * 	[92/01/14            rvb]
 * 
 * 	From 2.5 boot:
 * 	Flush digit printing.
 * 	Fuse floppy and hd boot by using Int 21 to tell
 * 	boot type (slightly dubious since Int 21 is DOS
 * 	not BIOS)
 * 	[92/03/30            mg32]
 * 
 * Revision 2.2  91/04/02  14:42:04  mbj
 * 	Fix the BIG boot bug.  We had missed a necessary data
 * 	before a xor that was clearing a register used later
 * 	as an index register.
 * 	[91/03/01            rvb]
 * 	Remember floppy type for swapgeneric
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

#include	"machine/asm.h"

	.file	"start.s"

BOOTSEG		=	0x100	/* boot will be loaded at 4k */
BOOTSTACK	=	0xf000	/* boot stack */
SIGNATURE	=	0xaa55
LOADSZ		=	15	/* size of unix boot */
PARTSTART	=	0x1be	/* starting address of partition table */
NUMPART		=	4	/* number of partitions in partition table */
PARTSZ		=	16	/* each partition table entry is 16 bytes */
BOOTABLE	=	0x80	/* value of boot_ind, means bootable partition */

	.text	

ENTRY(boot1)

	/* boot1 is loaded at 0x0:0x7c00 */
	/* ljmp to the next instruction to set up %cs */
	data16
	ljmp $0x7c0, $start

start:
	/* set up %ds */
	mov	%cs, %ax
	mov	%ax, %ds

	/* set up %ss and %esp */
	data16
	mov	$BOOTSEG, %eax
	mov	%ax, %ss
	data16
	mov	$BOOTSTACK, %esp

	/*set up %es */
	mov	%ax, %es

	cmpb	$0x80, %dl
	data16
	jae	hd

fd:
/*	reset the disk system */
	movb	$0x0, %ah
	int	$0x13
	data16
	mov	$0x0001, %ecx	/* cyl 0, sector 1 */
	xorb	%dh, %dh
	data16
	jmp load

hd:	data16
	mov	$0x0201, %eax
	xor	%ebx, %ebx	/* %bx = 0 */
	data16
	mov	$0x0001, %ecx
	data16
	mov	$0x0080, %edx
	int	$0x13
	data16
	jb	read_error

	/* find the bootable partition */
	data16
	mov	$PARTSTART, %ebx
	data16
	mov	$NUMPART, %ecx
again:
	addr16
	movb    %es:0(%ebx), %al
	cmpb	$BOOTABLE, %al
	data16
	je	found
	data16
	add	$PARTSZ, %ebx
	data16
	loop	again
	data16
	mov	$enoboot, %esi
	data16
	jmp	message

found:
	addr16
	movb	%es:1(%ebx), %dh
	addr16
	movl	%es:2(%ebx), %ecx

load:	movb	$0x2, %ah
	movb	$LOADSZ, %al
	xor	%ebx, %ebx	/* %bx = 0 */
	int	$0x13
	data16
	jb	read_error

	/*
	 * ljmp to the second stage boot loader (boot2).
	 * After ljmp, %cs is BOOTSEG and boot1 (512 bytes) will be used
	 * as an internal buffer "intbuf".
	 */

	data16
	ljmp	$BOOTSEG, $(EXT(boot2))

/*
 *	read_error
 */

read_error:

	data16
	mov	$eread, %esi

/*
 *	message: write the error message in %ds:%esi to console
 */

message:
	/*
	 * Use BIOS "int 10H Function 0Eh" to write character in teletype mode
	 *	%ah = 0xe	%al = character
	 *	%bh = page	%bl = foreground color (graphics modes)
	 */
	data16
	mov	$0x0001, %ebx
	cld

nextb:
	lodsb			/* load a byte into %al */
	cmpb	$0x0, %al
	data16
	je	done
	movb	$0xe, %ah
	int	$0x10		/* display a byte */
	data16
	jmp	nextb
done:	hlt

/*
 *	error messages
 */

eread:	String		"Read error\r\n\0"
enoboot: String		"No bootable partition\r\n\0"

/* the last 2 bytes in the sector 0 contain the signature */
	. = EXT(boot1) + 0x100
ENTRY(Default)
	 String		"\0"
	. = EXT(Default) + 0x0fe
	.value	SIGNATURE
	. = . + 0x200
