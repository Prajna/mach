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
 * $Log:	boolean.h,v $
 * Revision 2.4  93/01/14  17:41:23  danner
 * 	Standardized include symbol name.
 * 	[92/06/10            pds]
 * 
 * Revision 2.3  91/05/14  16:51:06  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:31:38  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:16:36  mrt]
 * 
 * Revision 2.1  89/08/03  15:59:35  rwd
 * Created.
 * 
 * Revision 2.4  89/02/25  18:12:08  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.3  89/02/07  00:51:34  mwyoung
 * Relocated from sys/boolean.h
 * 
 * Revision 2.2  88/08/24  02:23:06  mwyoung
 * 	Adjusted include file references.
 * 	[88/08/17  02:09:46  mwyoung]
 * 
 *
 * 18-Nov-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Header file fixup, purge history.
 *
 */
/*
 *	File:	mach/boolean.h
 *
 *	Boolean data type.
 *
 */

#ifndef	_MACH_BOOLEAN_H_
#define	_MACH_BOOLEAN_H_

/*
 *	Pick up "boolean_t" type definition
 */

#ifndef	ASSEMBLER
#include <mach/machine/boolean.h>
#endif	/* ASSEMBLER */

#endif	/* _MACH_BOOLEAN_H_ */

/*
 *	Define TRUE and FALSE, only if they haven't been before,
 *	and not if they're explicitly refused.  Note that we're
 *	outside the BOOLEAN_H_ conditional, to avoid ordering
 *	problems.
 */

#if	(defined(KERNEL) || defined(EXPORT_BOOLEAN)) && !defined(NOBOOL)

#ifndef	TRUE
#define TRUE	((boolean_t) 1)
#endif	/* TRUE */

#ifndef	FALSE
#define FALSE	((boolean_t) 0)
#endif	/* FALSE */

#endif	/* (defined(KERNEL) || defined(EXPORT_BOOLEAN)) && !defined(NOBOOL) */
