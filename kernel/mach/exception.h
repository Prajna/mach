/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
 * $Log:	exception.h,v $
 * Revision 2.4  93/01/14  17:41:36  danner
 * 	Standardized include symbol name.
 * 	[92/06/10            pds]
 * 
 * Revision 2.3  91/05/14  16:51:41  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:31:55  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:17:06  mrt]
 * 
 * Revision 2.1  89/08/03  16:02:18  rwd
 * Created.
 * 
 * Revision 2.4  89/02/25  18:13:29  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.3  89/02/07  00:52:07  mwyoung
 * Relocated from sys/exception.h
 * 
 * Revision 2.2  88/08/24  02:26:52  mwyoung
 * 	Adjusted include file references.
 * 	[88/08/17  02:12:09  mwyoung]
 * 
 *
 * 29-Sep-87  David Black (dlb) at Carnegie-Mellon University
 *	Created.
 *
 */

#ifndef	_MACH_EXCEPTION_H_
#define	_MACH_EXCEPTION_H_

#include <mach/machine/exception.h>

/*
 *	Machine-independent exception definitions.
 */

#define EXC_BAD_ACCESS		1	/* Could not access memory */
		/* Code contains kern_return_t describing error. */
		/* Subcode contains bad memory address. */

#define EXC_BAD_INSTRUCTION	2	/* Instruction failed */
		/* Illegal or undefined instruction or operand */

#define EXC_ARITHMETIC		3	/* Arithmetic exception */
		/* Exact nature of exception is in code field */

#define EXC_EMULATION		4	/* Emulation instruction */
		/* Emulation support instruction encountered */
		/* Details in code and subcode fields	*/

#define EXC_SOFTWARE		5	/* Software generated exception */
		/* Exact exception is in code field. */
		/* Codes 0 - 0xFFFF reserved to hardware */
		/* Codes 0x10000 - 0x1FFFF reserved for OS emulation (Unix) */

#define EXC_BREAKPOINT		6	/* Trace, breakpoint, etc. */
		/* Details in code field. */

#endif	/* _MACH_EXCEPTION_H_ */
