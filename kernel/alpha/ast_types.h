/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
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
 * $Log:	ast_types.h,v $
 * Revision 2.2  93/01/14  17:11:52  danner
 * 	Created.
 * 	[92/12/10            af]
 * 
 */
/*
 *	File: ast_types.h
 *	Author: Alessandro Forin, Carnegie Mellon University
 *	Date: 11/92
 *
 *	Machine-dependent definitions for the AST mechanisms
 */

#ifndef	_ALPHA_AST_TYPES_H_
#define	_ALPHA_AST_TYPES_H_

/*
 *	Data type for remote ast_check() invocation support.  Currently
 *	not implemented.  Do this first to avoid include problems.
 */
typedef	integer_t	ast_check_t;

#endif	/* _ALPHA_AST_TYPES_H_ */
