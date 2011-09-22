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
 * $Log:	bcopy.s,v $
 * Revision 2.3  93/05/10  17:50:47  rvb
 * 	Use C Comment
 * 	[93/05/04  17:57:24  rvb]
 * 
 * Revision 2.2  93/01/24  13:23:29  danner
 * 	Make the flow a little simpler to not confuse gprof profiling,
 * 	so we can see who bcopy's
 * 	[93/01/13            rvb]
 * 	Created!
 * 	[92/10/22            rvb]
 * 
 *
 * File: 	libmach_sa/bcopy.s
 * Author:	Robert V. Baron at Carnegie Mellon
 * Date:	Oct 13, 1992
 * Abstract:
 *	bcopy(from,to,count) - copies count bytes from "from"
 *		to "to". Overlapping regions are handled correctly.
 */

#include <i386/asm.h>


/* l2rbcopy(from, to, bcount) */

ENTRY(l2rbcopy)
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%edi
	pushl	%esi
	movl	B_ARG2,%edx
	movl	B_ARG1,%edi
	movl	B_ARG0,%esi
/* move longs */
0:	cld
	movl	%edx,%ecx
	sarl	$2,%ecx
	js	1f
	rep
	movsl
/* move bytes */
	movl	%edx,%ecx
	andl	$3,%ecx
	rep
	movsb
1:
	popl	%esi
	popl	%edi
	leave
	ret	

/* 
 * bcopy - like l2rbcopy, but recognizes overlapping ranges and handles 
 *           them correctly.
 *	  bcopy(from, to, bytes)
 *		char *from, *to;
 *		int bytes;
 */
ENTRY(bcopy)
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%edi
	pushl	%esi
	cld
	movl	B_ARG2,%edx
	movl	B_ARG1,%edi
	movl	B_ARG0,%esi
	leal	(%esi,%edx), %eax	/* from + bytes */
	cmpl	%eax, %edi		/* <= to 	*/
	jae	0f
	leal	(%edi,%edx), %eax	/* to + bytes	*/
	cmpl	%eax, %esi		/* <= from	*/
	jae	0f
	cmpl	%esi, %edi		/* from > to	*/
	jb	0f
	je 	1f

	addl	%edx, %esi; decl %esi
	addl	%edx, %edi; decl %edi
	std
	movl	%edx,%ecx
/* move bytes backwards */
	rep
	cld
	movsb
1:
	popl	%esi
	popl	%edi
	leave
	ret	
/* move words forwards */
0:	cld
	movl	%edx,%ecx
	sarl	$2,%ecx
	js	2f
	rep
	movsl
/* move bytes forwards */
	movl	%edx,%ecx
	andl	$3,%ecx
	rep
	movsb
2:
	popl	%esi
	popl	%edi
	leave
	ret	
