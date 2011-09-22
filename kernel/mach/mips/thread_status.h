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
 * $Log:	thread_status.h,v $
 * Revision 2.8  93/01/14  17:45:47  danner
 * 	Standardized include symbol name.
 * 	[92/06/10            pds]
 * 
 * Revision 2.7  91/05/14  16:57:37  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/05/13  06:03:34  af
 * 	Added author note.
 * 	[91/05/12  15:54:03  af]
 * 
 * Revision 2.5.1.1  91/02/21  18:47:21  af
 * 	Added author note.
 * 
 * 
 * Revision 2.5  91/02/05  17:34:54  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:13:04  mrt]
 * 
 * Revision 2.4  90/06/02  14:59:24  rpd
 * 	Added missing include.
 * 	[90/03/26  22:37:55  rpd]
 * 
 * Revision 2.3  90/01/22  12:02:14  af
 * 	Added single-stepping interface, via setstatus of the exception
 * 	flavor.  Sounded like an "exception" state is the closest thing
 * 	to a vax with a T bit set.  Added relevant defs.
 * 	[90/01/22            af]
 * 
 * Revision 2.2  89/11/29  14:09:54  af
 * 	Added exc_state flavor.  Needed by Unix emulator to properly pass
 * 	machine-level exception information to signal handlers.
 * 	Note that this is a read-only flavor, it is (currently?) impossible
 * 	to impose a machine-level exception on a thread.
 * 	[89/11/27            af]
 * 
 * 	For pure kernel: removed kernel-only structures, added vanilla
 * 	coprocessor state.
 * 	[89/10/04            af]
 * 
 * Revision 2.2  89/07/14  15:26:37  rvb
 * 	Added floating point flavor, fixed mips_thread_state to export
 * 	the pc and hi/lo registers.
 * 	[89/07/07            af]
 * 
 * Revision 2.1  89/05/30  16:55:47  rvb
 * Created.
 * 
 *  3-Jan-89  Alessandro Forin (af) at Carnegie-Mellon University
 *	Created.
 */

/*
 *	File:	mips/thread_status.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	1/89
 *
 *
 *	This file contains the structure definitions for the thread
 *	state as applicable to Mips processors.
 *
 */

#ifndef	_MACH_MIPS_THREAD_STATUS_H_
#define	_MACH_MIPS_THREAD_STATUS_H_

#include <mach/machine/vm_types.h>

/*
 *	The structures defined in here are exported to users for
 *	use in status/mutate calls.
 *
 *	mips_thread_state	basic machine state
 *
 *	mips_coproc_state	state of other (optional) coprocessors
 *
 *	mips_float_state	state of floating point coprocessor (alias)
 *
 *	mips_exc_state		exception state (fault address, etc.)
 */

#define	MIPS_THREAD_STATE	(1)
#define MIPS_FLOAT_STATE	(2)
#define	MIPS_COPROC_STATE	(3)
#define MIPS_EXC_STATE		(4)

struct mips_thread_state {
	int	r1;		/* at:  assembler temporary */
	int	r2;		/* v0:  return value 0 */
	int	r3;		/* v1:  return value 1 */
	int	r4;		/* a0:  argument 0 */
	int	r5;		/* a1:  argument 1 */
	int	r6;		/* a2:  argument 2 */
	int	r7;		/* a3:  argument 3 */
	int	r8;		/* t0:  caller saved 0 */
	int	r9;		/* t1:  caller saved 1 */
	int	r10;		/* t2: caller saved 2 */
	int	r11;		/* t3: caller saved 3 */
	int	r12;		/* t4: caller saved 4 */
	int	r13;		/* t5: caller saved 5 */
	int	r14;		/* t6: caller saved 6 */
	int	r15;		/* t7: caller saved 7 */
	int	r16;		/* s0: callee saved 0 */
	int	r17;		/* s1: callee saved 1 */
	int	r18;		/* s2: callee saved 2 */
	int	r19;		/* s3: callee saved 3 */
	int	r20;		/* s4: callee saved 4 */
	int	r21;		/* s5: callee saved 5 */
	int	r22;		/* s6: callee saved 6 */
	int	r23;		/* s7: callee saved 7 */
	int	r24;		/* t8: code generator 0 */
	int	r25;		/* t9: code generator 1 */
	int	r26;		/* k0: kernel temporary 0 */
	int	r27;		/* k1: kernel temporary 1 */
	int	r28;		/* gp: global pointer */
	int	r29;		/* sp: stack pointer */
	int	r30;		/* fp: frame pointer */
	int	r31;		/* ra: return address */
	int	mdlo;		/* low mult result */
	int	mdhi;		/* high mult result */
	int	pc;		/* user-mode PC */
};

#define	MIPS_THREAD_STATE_COUNT	(sizeof(struct mips_thread_state)/sizeof(int))


struct mips_coproc_state {
	int	r0;	/* 32 general coprocessor registers */
	int	r1;
	int	r2;
	int	r3;
	int	r4;
	int	r5;
	int	r6;
	int	r7;
	int	r8;
	int	r9;
	int	r10;
	int	r11;
	int	r12;
	int	r13;
	int	r14;
	int	r15;
	int	r16;
	int	r17;
	int	r18;
	int	r19;
	int	r20;
	int	r21;
	int	r22;
	int	r23;
	int	r24;
	int	r25;
	int	r26;
	int	r27;
	int	r28;
	int	r29;
	int	r30;
	int	r31;
	int	csr;	/* status register */
	int	esr;	/* exception status register */
};

#define	MIPS_COPROC_STATE_COUNT	(sizeof(struct mips_coproc_state)/sizeof(int))

#define mips_float_state	mips_coproc_state
#define		eir		esr	/* exception instruction reg */

#define	MIPS_FLOAT_STATE_COUNT	(sizeof(struct mips_float_state)/sizeof(int))


struct mips_exc_state {
	unsigned	cause;		/* machine-level trap code */
#define EXC_SST		0x00000044
	vm_offset_t	address;	/* last invalid virtual address */
	unsigned	coproc_state;	/* which coprocessors thread used */
#define MIPS_STATUS_USE_COP0	1	/* (by definition) */
#define MIPS_STATUS_USE_COP1	2	/* FPA */
#define MIPS_STATUS_USE_COP2	4
#define MIPS_STATUS_USE_COP3	8
};

#define	MIPS_EXC_STATE_COUNT	(sizeof(struct mips_exc_state)/sizeof(int))

#endif	/* _MACH_MIPS_THREAD_STATUS_H_ */
