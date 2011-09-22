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
 * $Log:	kalloc.h,v $
 * Revision 2.7  91/05/18  14:31:59  rpd
 * 	Added kalloc_init.
 * 	[91/03/22            rpd]
 * 
 * Revision 2.6  91/05/14  16:43:32  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:27:26  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:14:21  mrt]
 * 
 * Revision 2.4  90/06/02  14:54:51  rpd
 * 	Changed types to vm_offset_t.
 * 	[90/03/26  22:07:07  rpd]
 * 
 * Revision 2.3  89/09/08  11:25:56  dbg
 * 	MACH_KERNEL: remove non-MACH data types.
 * 	[89/07/11            dbg]
 * 
 * Revision 2.2  89/08/31  16:19:04  rwd
 * 	First Checkin
 * 	[89/08/23  15:41:50  rwd]
 * 
 * Revision 2.10  89/03/09  20:13:03  rpd
 * 	More cleanup.
 * 
 * Revision 2.9  89/02/25  18:04:45  gm0w
 * 	Kernel code cleanup.	
 * 	Put entire file under #ifdef KERNEL
 * 	[89/02/15            mrt]
 * 
 * Revision 2.8  89/02/07  01:01:53  mwyoung
 * Relocated from sys/kalloc.h
 * 
 * Revision 2.7  89/01/18  02:10:51  jsb
 * 	Fixed log.
 * 	[88/01/18            rpd]
 * 
 * Revision 2.2  89/01/18  01:16:25  jsb
 * 	Use MINSIZE of 16 instead of 64 (mostly for afs);
 * 	eliminate NQUEUES (see kalloc.c).
 * 	[89/01/13            jsb]
 *
 * 26-Oct-87 Peter King (king) at NeXT, Inc.
 *	Created.
 */ 

#ifndef	_KERN_KALLOC_H_
#define _KERN_KALLOC_H_

#include <mach/machine/vm_types.h>

#define MINSIZE 16

extern vm_offset_t kalloc();
extern vm_offset_t kget();
extern void kfree();

extern void kalloc_init();

#endif	_KERN_KALLOC_H_
