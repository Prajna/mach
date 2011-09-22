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
 * $Log:	thread_swap.h,v $
 * Revision 2.6  91/07/31  17:50:25  dbg
 * 	Merge remaining swap states with state field in thread.
 * 	[91/07/30  17:07:30  dbg]
 * 
 * Revision 2.5  91/05/14  16:49:20  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/03/16  14:53:18  rpd
 * 	Removed thread_swappable, swapout_thread, etc.
 * 	Removed unnecessary swap states.
 * 	[91/01/18            rpd]
 * 
 * Revision 2.3  91/02/05  17:30:46  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:20:24  mrt]
 * 
 * Revision 2.2  90/06/02  14:57:23  rpd
 * 	Updated for new swapping technology.
 * 	[90/03/26  22:27:27  rpd]
 * 
 * Revision 2.1  89/08/03  15:54:23  rwd
 * Created.
 * 
 * 21-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Created.
 *
 */
/*
 *	File:	kern/thread_swap.h
 *
 *	Declarations of thread swapping routines.
 */

#ifndef	_KERN_THREAD_SWAP_H_
#define _KERN_THREAD_SWAP_H_

/*
 *	exported routines
 */
extern void	swapper_init();
extern void	thread_swapin( /* thread_t thread */ );
extern void	thread_doswapin( /* thread_t thread */ );
extern void	swapin_thread();
extern void	thread_swapout( /* thread_t thread */ );

#endif	_KERN_THREAD_SWAP_H_
