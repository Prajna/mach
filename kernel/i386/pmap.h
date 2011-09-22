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
 * $Log:	pmap.h,v $
 * Revision 2.7  91/05/14  16:15:20  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/02/05  17:14:18  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:37:29  mrt]
 * 
 * Revision 2.5  90/12/04  14:46:31  jsb
 * 	Merged with i860 and moved to intel/pmap.h.
 * 	[90/12/04  11:19:18  jsb]
 * 
 */
/*
 * Now using shared pmap module for i386 and i860.
 */

#include <intel/pmap.h>
