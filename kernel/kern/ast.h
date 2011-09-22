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
 * $Log:	ast.h,v $
 * Revision 2.11  92/05/22  18:37:52  jfriedl
 * 		Put calls to aston/astoff in {} for safety.
 * 
 * Revision 2.9  92/05/04  11:24:24  danner
 * 	Changed AST_PER_THREAD definition (from dbg).
 * 	[92/05/03            danner]
 * 
 * Revision 2.8  92/04/06  01:16:11  rpd
 * 	Fixed ast_context bug, with AST_PER_THREAD.  From dbg.
 * 	[92/04/05            rpd]
 * 
 * Revision 2.7  91/08/28  11:14:20  jsb
 * 	Renamed AST_CLPORT to AST_NETIPC.
 * 	[91/08/14  21:38:09  jsb]
 * 
 * Revision 2.6  91/06/06  17:06:48  jsb
 * 	Added AST_CLPORT.
 * 	[91/05/13  17:35:08  jsb]
 * 
 * Revision 2.5  91/05/14  16:39:58  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/03/16  14:49:32  rpd
 * 	Fixed dummy aston, astoff definitions.
 * 	[91/02/12            rpd]
 * 	Revised the AST interface, adding AST_NETWORK.
 * 	Added volatile attribute to need_ast.
 * 	[91/01/18            rpd]
 * 
 * Revision 2.3  91/02/05  17:25:38  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:11:14  mrt]
 * 
 * Revision 2.2  90/06/02  14:53:34  rpd
 * 	Merged with mainline.
 * 	[90/03/26  22:02:55  rpd]
 * 
 * Revision 2.1  89/08/03  15:45:04  rwd
 * Created.
 * 
 *  6-Sep-88  David Golub (dbg) at Carnegie-Mellon University
 *	Adapted to MACH_KERNEL and VAX.
 *
 * 11-Aug-88  David Black (dlb) at Carnegie-Mellon University
 *	Created.  dbg gets equal credit for the design.
 *
 */

/*
 *	kern/ast.h: Definitions for Asynchronous System Traps.
 */

#ifndef	_KERN_AST_H_
#define _KERN_AST_H_

/*
 *	A CPU takes an AST when it is about to return to user code.
 *	Instead of going back to user code, it calls ast_taken.
 *	Machine-dependent code is responsible for maintaining
 *	a set of reasons for an AST, and passing this set to ast_taken.
 */

#include <cpus.h>

#include <kern/cpu_number.h>
#include <kern/macro_help.h>
#include <machine/ast.h>

/*
 *	Bits for reasons
 */

#define	AST_ZILCH	0x0
#define AST_HALT	0x1
#define AST_TERMINATE	0x2
#define AST_BLOCK	0x4
#define AST_NETWORK	0x8
#define AST_NETIPC	0x10

#define	AST_SCHEDULING	(AST_HALT|AST_TERMINATE|AST_BLOCK)

/*
 * Per-thread ASTs are reset at context-switch time.
 * machine/ast.h can define MACHINE_AST_PER_THREAD.
 */

#ifndef	MACHINE_AST_PER_THREAD
#define	MACHINE_AST_PER_THREAD	0
#endif

#define AST_PER_THREAD  (AST_HALT | AST_TERMINATE | MACHINE_AST_PER_THREAD)

typedef unsigned int ast_t;

extern volatile ast_t need_ast[NCPUS];

#ifdef	MACHINE_AST
/*
 *	machine/ast.h is responsible for defining aston and astoff.
 */
#else	MACHINE_AST

#define aston(mycpu)
#define astoff(mycpu)

#endif	MACHINE_AST

extern void ast_taken();

/*
 *	ast_needed, ast_on, ast_off, ast_context, and ast_propagate
 *	assume splsched.  mycpu is always cpu_number().  It is an
 *	argument in case cpu_number() is expensive.
 */

#define ast_needed(mycpu)		need_ast[mycpu]

#define ast_on(mycpu, reasons)						\
MACRO_BEGIN								\
	if ((need_ast[mycpu] |= (reasons)) != AST_ZILCH)		\
		{ aston(mycpu); }					\
MACRO_END

#define ast_off(mycpu, reasons)						\
MACRO_BEGIN								\
	if ((need_ast[mycpu] &= ~(reasons)) == AST_ZILCH)		\
		{ astoff(mycpu); } 					\
MACRO_END

#define ast_propagate(thread, mycpu)	ast_on((mycpu), (thread)->ast)

#define ast_context(thread, mycpu)					\
MACRO_BEGIN								\
	if ((need_ast[mycpu] =						\
	     (need_ast[mycpu] &~ AST_PER_THREAD) | (thread)->ast)	\
					!= AST_ZILCH)			\
		{ aston(mycpu);	}					\
	else								\
		{ astoff(mycpu); }					\
MACRO_END


#define	thread_ast_set(thread, reason)		(thread)->ast |= (reason)
#define thread_ast_clear(thread, reason)	(thread)->ast &= ~(reason)
#define thread_ast_clear_all(thread)		(thread)->ast = AST_ZILCH

/*
 *	NOTE: if thread is the current thread, thread_ast_set should
 *	be followed by ast_propagate().
 */

#endif	_KERN_AST_H_
