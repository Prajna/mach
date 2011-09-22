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
 * $Log:	setjmp.h,v $
 * Revision 2.4  91/05/14  16:16:12  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:14:34  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:37:53  mrt]
 * 
 * Revision 2.2  90/08/27  21:58:23  dbg
 * 	Created.
 * 	[90/07/16            dbg]
 * 
 */

/*
 * Setjmp/longjmp buffer for i386.
 */
#ifndef	_I386_SETJMP_H_
#define	_I386_SETJMP_H_

typedef	struct jmp_buf {
	int	jmp_buf[6];	/* ebx, esi, edi, ebp, esp, eip */
} jmp_buf_t;

#endif	/* _I386_SETJMP_H_ */
