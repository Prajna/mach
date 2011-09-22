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
 * $Log:	asm.h,v $
 * Revision 2.9  93/05/10  17:46:19  rvb
 * 	Define LCLL(x) "Local label" == x:
 * 	Specify ALIGN == 2.
 * 	[93/05/03            rvb]
 * 
 * Revision 2.8  93/03/09  11:17:03  danner
 * 	Flushed wheeze and fixed for gcc cpp.
 * 	[93/03/03            mrt]
 * 
 * Revision 2.7  92/02/29  15:33:41  rpd
 * 	Added ENTRY2.
 * 	[92/02/28            rpd]
 * 
 * Revision 2.6  92/02/19  15:07:52  elf
 * 	Changed #if __STDC__ to #ifdef __STDC__
 * 	[92/01/16            jvh]
 * 
 * Revision 2.5  91/05/14  16:02:45  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:10:42  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:30:29  mrt]
 * 
 * Revision 2.3  90/12/20  16:35:27  jeffreyh
 * 	changes for __STDC__
 * 	[90/12/06            jeffreyh]
 * 
 * Revision 2.2  90/05/03  15:24:12  dbg
 * 	First checkin.
 * 
 *
 * 	Typo on ENTRY if gprof
 * 	[90/03/29            rvb]
 * 
 * 	fix SVC for "ifdef wheeze" [kupfer]
 * 	Fix the GPROF definitions.
 * 	ENTRY(x) gets profiled iffdef GPROF.
 * 	Entry(x) (and DATA(x)) is NEVER profiled.
 * 	MCOUNT can be used by asm that intends to build a frame,
 * 	after the frame is built.
 *	[90/02/26            rvb]
 *
 * 	Add #define addr16 .byte 0x67
 * 	[90/02/09            rvb]
 * 	Added LBi, SVC and ENTRY
 * 	[89/11/10  09:51:33  rvb]
 * 
 * 	New a.out and coff compatible .s files.
 * 	[89/10/16            rvb]
 */


#define S_ARG0	 4(%esp)
#define S_ARG1	 8(%esp)
#define S_ARG2	12(%esp)
#define S_ARG3	16(%esp)

#define FRAME	pushl %ebp; movl %esp, %ebp
#define EMARF	leave

#define B_ARG0	 8(%ebp)
#define B_ARG1	12(%ebp)
#define B_ARG2	16(%ebp)
#define B_ARG3	20(%ebp)

#define ALIGN 	2
#define	LCL(x)	x

#define LB(x,n) n
#ifdef	__STDC__
#define EXT(x) _ ## x
#define LEXT(x) _ ## x ## :
#define LCLL(x) x ## :
#define gLB(n)  n ## :
#define LBb(x,n) n ## b
#define LBf(x,n) n ## f
#else __STDC__
#define EXT(x) _/**/x
#define LEXT(x) _/**/x/**/:
#define LCLL(x) x/**/:
#define gLB(n) n/**/:
#define LBb(x,n) n/**/b
#define LBf(x,n) n/**/f
#endif __STDC__
#define SVC .byte 0x9a; .long 0; .word 0x7

#define String	.ascii
#define Value	.word
#define Times(a,b) (a*b)
#define Divide(a,b) (a/b)

#define INB	inb	%dx, %al
#define OUTB	outb	%al, %dx
#define INL	inl	%dx, %eax
#define OUTL	outl	%eax, %dx

#define data16	.byte 0x66
#define addr16	.byte 0x67



#ifdef GPROF

#define MCOUNT		.data; gLB(9) .long 0; .text; lea LBb(x, 9),%edx; call mcount
#define	ENTRY(x)	.globl EXT(x); .align ALIGN; LEXT(x) ; \
			pushl %ebp; movl %esp, %ebp; MCOUNT; popl %ebp;
#define	ENTRY2(x,y)	.globl EXT(x); .globl EXT(y); \
			.align ALIGN; LEXT(x) LEXT(y)
#define	ASENTRY(x) 	.globl x; .align ALIGN; gLB(x) ; \
  			pushl %ebp; movl %esp, %ebp; MCOUNT; popl %ebp;

#else	GPROF

#define MCOUNT
#define	ENTRY(x)	.globl EXT(x); .align ALIGN; LEXT(x)
#define	ENTRY2(x,y)	.globl EXT(x); .globl EXT(y); \
			.align ALIGN; LEXT(x) LEXT(y)
#define	ASENTRY(x)	.globl x; .align ALIGN; gLB(x)
#endif	GPROF

#define	Entry(x)	.globl EXT(x); .align ALIGN; LEXT(x)
#define	DATA(x)		.globl EXT(x); .align ALIGN; LEXT(x)
