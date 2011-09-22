/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
 * $Log:	macro_help.h,v $
 * Revision 2.4  93/01/14  17:35:26  danner
 * 	Too many replicas of this file.
 * 	[92/12/10  18:07:36  af]
 * 
 * Revision 2.3  91/05/14  16:44:49  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:28:09  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:15:31  mrt]
 * 
 * Revision 2.1  89/08/03  15:53:45  rwd
 * Created.
 * 
 * Revision 2.2  88/10/18  03:36:20  mwyoung
 * 	Added a form of return that can be used within macros that
 * 	does not result in "statement not reached" noise.
 * 	[88/10/17            mwyoung]
 * 	
 * 	Add MACRO_BEGIN, MACRO_END.
 * 	[88/10/11            mwyoung]
 * 	
 * 	Created.
 * 	[88/10/08            mwyoung]
 * 
 */
/*
 *	File:	kern/macro_help.h
 *
 *	Provide help in making lint-free macro routines
 *
 */  

#ifndef	_KERN_MACRO_HELP_H_
#define	_KERN_MACRO_HELP_H_

#if	!defined(MACRO_BEGIN)

#include <mach/boolean.h>

#ifdef	lint
boolean_t	NEVER;
boolean_t	ALWAYS;
#else	/* lint */
#define		NEVER		FALSE
#define		ALWAYS		TRUE
#endif	/* lint */

#define		MACRO_BEGIN	do {
#define		MACRO_END	} while (NEVER)

#define		MACRO_RETURN	if (ALWAYS) return

#endif	/* !MACRO_BEGIN */

#endif	/* _KERN_MACRO_HELP_H_ */
