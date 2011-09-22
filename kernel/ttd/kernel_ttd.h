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
 * Definitions for Kernel TTD
 *
 * HISTORY:
 * $Log:	kernel_ttd.h,v $
 * Revision 2.2  93/05/10  23:24:23  rvb
 * 	Checkin for MK80 branch.
 * 	[93/05/10  15:04:38  grm]
 * 
 * Revision 2.1.2.1  93/03/03  14:36:12  grm
 * 	Gratuitious checkin for branch reasons.
 * 	[93/03/03            grm]
 * 
 * Revision 2.1.1.3  92/10/23  21:15:25  grm
 * 	Added single stepping boolean.
 * 	[92/10/23            grm]
 * 
 * Revision 2.1.1.2  92/10/08  14:26:16  grm
 * 	Changed the kttd_breakpoint structure and removed some cruft.
 * 	[92/10/08            grm]
 * 
 * Revision 2.1.1.1  92/09/25  15:14:26  grm
 * 	Intial checkin.
 * 	[92/09/25            grm]
 * 
 * 
 */
/***********************************************************
Copyright 1992 by Digital Equipment Corporation, Maynard, Massachusetts,

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, provided 
that the above copyright notice appear in all copies and that both that 
copyright notice and this permission notice appear in supporting 
documentation, and that the name of Digital not be used in advertising 
or publicity pertaining to distribution of the software without specific, 
written prior permission.  Digital makes no representations about the 
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifndef _KERNEL_TTD_H_
#define _KERNEL_TTD_H_

#include <ttd/ttd_types.h>

/* State of the single debug target -- i.e the kernel */

#define KTTD_MAXBPT 500 /* Need room for debugger's hidden breakpoints */

typedef ttd_saved_inst *kttd_break_ptr;

typedef struct {
	ttd_address	address;
	ttd_saved_inst	saved_inst;
	ttd_flavor	flavor;
	ttd_thread	thread;
	boolean_t	free;
} kttd_breakpoint;

typedef unsigned kttd_breakno;

#endif	/* _KERNEL_TTD_H_ */
