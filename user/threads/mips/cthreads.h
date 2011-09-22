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
 * $Log:	cthreads.h,v $
 * Revision 2.6  93/11/17  19:00:59  dbg
 * 	Since we compile with GCC now, at least inline cthread_sp().
 * 	[93/09/21            af]
 * 
 * Revision 2.5  92/07/20  13:33:41  cmaeda
 * 	Added mips-dependent init function.
 * 	[92/05/11  14:41:31  cmaeda]
 * 
 * Revision 2.4  91/05/14  17:58:05  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/14  14:20:50  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:38:50  mrt]
 * 
 * Revision 2.2  90/11/05  14:37:42  rpd
 * 	Created.
 * 	[90/11/01            rwd]
 * 
 */

#ifndef _MACHINE_CTHREADS_H_
#define _MACHINE_CTHREADS_H_

typedef int spin_lock_t;
#define SPIN_LOCK_INITIALIZER 0
#define spin_lock_init(s) *(s)=0
#define spin_lock_locked(s) (*(s) != 0)

#define cthread_md_init() mips_cthread_md_init()

#if     defined(__GNUC__)

#define cthread_sp() \
        ({  register vm_offset_t _sp__; \
            __asm__("move %0, $29" \
              : "=r" (_sp__) ); \
            _sp__; })

#endif  /* __GNUC__ */

#endif /* _MACHINE_CTHREADS_H_ */
