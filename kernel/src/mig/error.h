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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * 26-May-93  Johannes Helander (jvh) Helsinki University of Technology
 *	Removed volatile from fatal declaration.
 *
 * $Log:	error.h,v $
 * Revision 2.7  93/08/02  21:44:22  mrt
 * 	Added definitions of sys_nerr and sys_errlist since they
 * 	are missing from errno.h on some older systems.
 * 	Changed sys/errno.h to conform to ANSI errno.h.
 * 	[93/06/23            mrt]
 * 
 * Revision 2.6  93/05/10  17:49:04  rvb
 * 	Only use __attribute__ iff __GNUC__ > 1
 * 	[93/05/05  09:15:24  rvb]
 * 
 * Revision 2.5  93/01/14  17:57:52  danner
 * 	Made text on #endif lines into comments.
 * 	[92/12/14            pds]
 * 	Converted file to ANSI C.
 * 	[92/12/08            pds]
 * 
 * Revision 2.4  92/01/14  16:46:17  rpd
 * 	Removed <mach_error.h>.
 * 	[92/01/06            rpd]
 * 
 * Revision 2.3  91/02/05  17:54:14  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:54:00  mrt]
 * 
 * Revision 2.2  90/06/02  15:04:33  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:10:30  rpd]
 * 
 * 07-Apr-89  Richard Draves (rpd) at Carnegie-Mellon University
 *	Extensive revamping.  Added polymorphic arguments.
 *	Allow multiple variable-sized inline arguments in messages.
 *
 * 27-May-87  Richard Draves (rpd) at Carnegie-Mellon University
 *	Created.
 */

#ifndef	_ERROR_H
#define	_ERROR_H

#include <errno.h>

#if	defined(__GNUC__)
extern void fatal(const char *format, ...)
#if	__GNUC__ > 1
			__attribute__ ((format (printf, 1, 2)))
#endif
							;
extern void warn(const char *format, ...)
#if	__GNUC__ > 1
			__attribute__ ((format (printf, 1, 2)))
#endif
							;
extern void error(const char *format, ...)
#if	__GNUC__ > 1
			__attribute__ ((format (printf, 1, 2)))
#endif
							;
#else	/* not defined(__GNUC__) */
extern void fatal(const char *format, ...);
extern void warn(const char *format, ...);
extern void error(const char *format, ...);
#endif	/* defined(__GNUC__) */

extern const char *unix_error_string(int error_num);
extern int sys_nerr;
extern char *sys_errlist[];

extern int errors;
extern void set_program_name(const char *name);

#endif	/* _ERROR_H */
