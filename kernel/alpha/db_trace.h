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
 * $Log:	db_trace.h,v $
 * Revision 2.2  93/01/14  17:12:55  danner
 * 	Created, from my mips version.
 * 	[92/06/01            af]
 * 
 */
/*
 *	File: db_trace.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	8/89
 *
 *	Symtab interface to stack traceback routines for MIPS
 */

typedef struct frame_info {
	int narg;		/* number of arguments, if known */
	int nloc;		/* number of local variables */
	int framesize;		/* size of the stack frame */
	unsigned regmask;	/* saved registers mask */
	int saved_pc_off;	/* offset of return pc from top of frame */
	unsigned int isleaf:1,	/* leaf procedure */
		     isvector:1,/* exception vector */
		     mod_sp:1,	/* affects the sp */
		     at_entry:1,/* sp has not been modified yet */
		     xxx:28;	/* unused */
	char *filename;		/* procedure's source file */
	int linenum;		/* linenumber in file */
} *frame_info_t;
