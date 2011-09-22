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
 * $Log:	vm_pageout.h,v $
 * Revision 2.4  91/05/14  17:51:11  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  18:00:18  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:34:35  mrt]
 * 
 * Revision 2.2  90/06/02  15:12:02  rpd
 * 	Added declaration of vm_pageout_setup.
 * 	[90/05/31            rpd]
 * 
 * 	Changes for MACH_KERNEL:
 * 	. Remove non-XP declarations.
 * 	[89/04/28            dbg]
 * 
 * Revision 2.1  89/08/03  16:45:59  rwd
 * Created.
 * 
 * Revision 2.9  89/04/18  21:28:02  mwyoung
 * 	No relevant history.
 * 
 */
/*
 *	File:	vm/vm_pageout.h
 *	Author:	Avadis Tevanian, Jr.
 *	Date:	1986
 *
 *	Declarations for the pageout daemon interface.
 */

#ifndef	_VM_VM_PAGEOUT_H_
#define _VM_VM_PAGEOUT_H_

#include <vm/vm_page.h>

/*
 *	Exported routines.
 */

extern vm_page_t vm_pageout_setup();
extern void vm_pageout_page();

#endif	_VM_VM_PAGEOUT_H_
