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
 * $Log:	db_output.h,v $
 * Revision 2.6  93/01/14  17:25:28  danner
 * 	Proper prototype for db_printf(), but incompat with varargs
 * 	so set it aside for now.
 * 	[92/11/30            af]
 * 
 * Revision 2.5  92/05/21  17:07:24  jfriedl
 * 	Cleanup to quiet gcc warnings.
 * 	[92/05/18            jfriedl]
 * 
 * Revision 2.4  91/05/14  15:35:07  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:06:49  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  16:18:48  mrt]
 * 
 * Revision 2.2  90/08/27  21:51:32  dbg
 * 	Created.
 * 	[90/08/07            dbg]
 * 
 */
/*
 * 	Author: David B. Golub, Carnegie Mellon University
 *	Date:	8/90
 */

/*
 * Printing routines for kernel debugger.
 */

extern void	db_force_whitespace();
extern int	db_print_position();
extern void	db_end_line();
#if 1
extern void	db_printf();
#else
extern void	db_printf( char *fmt, ...);
#endif


