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
 * $Log:	strings.h,v $
 * Revision 2.4  92/02/19  15:11:03  elf
 * 	Change strlen return type from int to unsigned long to keep gcc2
 * 	silent.
 * 	[92/02/19            jvh]
 * 
 * Revision 2.3  91/05/14  17:55:24  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/14  14:18:57  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:45:12  mrt]
 * 
 */

/*
 * External function definitions
 * for routines described in string(3).
 */
char	*strcat();
char	*strncat();
int	strcmp();
int	strncmp();
char	*strcpy();
char	*strncpy();
unsigned long	strlen();
char	*index();
char	*rindex();
