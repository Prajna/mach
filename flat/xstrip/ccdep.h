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
 * $Log:	ccdep.h,v $
 * Revision 2.1.1.1  94/06/01  10:24:35  rvb
 * 	From BSDSS
 * 
 * Revision 2.2  93/05/12  13:26:50  rvb
 * 	Stolen from user/xstrip
 * 
 * Revision 2.3  93/03/21  18:12:38  mrt
 * 	Added Sandro's fix to lookfor __GNUC__=2
 * 	[93/03/20            mrt]
 * 
 * Revision 2.2  93/02/03  18:02:00  mrt
 * 	Moved from user, added pds's changes for gdb compatibility
 * 
 * Revision 2.2.1.1  92/12/23  17:46:43  pds
 * 	Converted file to ANSI C.
 * 	Removed define of file_name.
 * 	[92/12/23            pds]
 * 
 * Revision 2.2  92/01/17  14:25:27  rpd
 * 	Created for compiler dependent definitions.
 * 	[91/08/29            tak]
 * 
 */

#include <string.h>

#if	__GNUC__==2
#define COMPILER_ID		"gcc2_compiled."
#else
#define COMPILER_ID		"gcc_compiled."
#endif

#define func_local(name)	(strchr((name), '.') != 0)
