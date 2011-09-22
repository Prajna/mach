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
 * $Log:	cswitch.s,v $
 * Revision 2.13  93/02/04  07:55:51  danner
 * 	Convert asm comment "/" over to "/ *" "* /"
 * 	[93/01/27            rvb]
 * 
 * Revision 2.12  92/01/03  20:04:43  dbg
 * 	Floating-point state is now saved by caller.
 * 	[91/10/20            dbg]
 * 
 * Revision 2.11  91/07/31  17:34:56  dbg
 * 	Push user state directly in PCB and switch to kernel stack on
 * 	trap entry.
 * 	[91/07/30  16:48:18  dbg]
 * 
 * Revision 2.10  91/06/19  11:54:57  rvb
 * 	cputypes.h->platforms.h
 * 	[91/06/12  13:44:37  rvb]
 * 
 * Revision 2.9  91/05/14  16:04:42  mrt
 * 	Correcting copyright
 * 
 * Revision 2.8  91/05/08  12:31:19  dbg
 * 	Put parentheses around substituted immediate expressions, so
 * 	that they will pass through the GNU preprocessor.
 * 
 * 	Handle multiple CPUs.  Save floating-point state when saving
 * 	context or switching stacks.  Add switch_to_shutdown_context to
 * 	handle CPU shutdown.
 * 	[91/03/21            dbg]
 * 
 * Revision 2.7  91/03/16  14:43:51  rpd
 * 	Renamed Switch_task_context to Switch_context.
 * 	[91/02/17            rpd]
 * 	Added active_stacks.
 * 	[91/01/29            rpd]
 * 
 * Revision 2.6  91/02/05  17:10:57  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:30:50  mrt]
 * 
 * Revision 2.5  91/01/09  22:41:09  rpd
 * 	Renamed to Load_context and Switch_task_context.
 * 	Removed ktss munging.
 * 	[91/01/09            rpd]
 * 
 * Revision 2.4  91/01/08  15:10:09  rpd
 * 	Minor cleanup.
 * 	[90/12/31            rpd]
 * 	Added switch_task_context, switch_thread_context.
 * 	[90/12/12            rpd]
 * 
 * 	Reorganized the pcb.
 * 	[90/12/11            rpd]
 * 
 * Revision 2.3  90/12/20  16:35:48  jeffreyh
 * 	Changes for __STDC__
 * 
 * 	Changes for __STDC__
 * 	[90/12/07  15:45:37  jeffreyh]
 * 
 *
 * Revision 2.2  90/05/03  15:25:00  dbg
 * 	Created.
 * 	[90/02/08            dbg]
 * 
 */

#include <cpus.h>
#include <platforms.h>

#include <i386/asm.h>
#include <i386/proc_reg.h>
#include <assym.s>

#if	NCPUS > 1

#ifdef	SYMMETRY
#include <sqt/asm_macros.h>
#endif

#define	CX(addr, reg)	addr(,reg,4)

#else	/* NCPUS == 1 */

#define	CPU_NUMBER(reg)
#define	CX(addr,reg)	addr

#endif	/* NCPUS == 1 */

/*
 * Context switch routines for i386.
 */

ENTRY(Load_context)
	movl	S_ARG0,%ecx			/* get thread */
	movl	TH_KERNEL_STACK(%ecx),%ecx	/* get kernel stack */
	lea	KERNEL_STACK_SIZE-IKS_SIZE-IEL_SIZE(%ecx),%edx
						/* point to stack top */
	CPU_NUMBER(%eax)
	movl	%ecx,CX(_active_stacks,%eax)	/* store stack address */
	movl	%edx,CX(_kernel_stack,%eax)	/* store stack top */

	movl	KSS_ESP(%ecx),%esp		/* switch stacks */
	movl	KSS_ESI(%ecx),%esi		/* restore registers */
	movl	KSS_EDI(%ecx),%edi
	movl	KSS_EBP(%ecx),%ebp
	movl	KSS_EBX(%ecx),%ebx
	xorl	%eax,%eax			/* return zero (no old thread) */
	jmp	*KSS_EIP(%ecx)			/* resume thread */

/*
 *	This really only has to save registers
 *	when there is no explicit continuation.
 */

ENTRY(Switch_context)
	CPU_NUMBER(%edx)
	movl	CX(_active_stacks,%edx),%ecx	/* get old kernel stack */

	movl	%ebx,KSS_EBX(%ecx)		/* save registers */
	movl	%ebp,KSS_EBP(%ecx)
	movl	%edi,KSS_EDI(%ecx)
	movl	%esi,KSS_ESI(%ecx)
	popl	KSS_EIP(%ecx)			/* save return PC */
	movl	%esp,KSS_ESP(%ecx)		/* save SP */

	movl	0(%esp),%eax			/* get old thread */
	movl	%ecx,TH_KERNEL_STACK(%eax)	/* save old stack */
	movl	4(%esp),%ebx			/* get continuation */
	movl	%ebx,TH_SWAP_FUNC(%eax)		/* save continuation */

	movl	8(%esp),%esi			/* get new thread */

	movl	TH_KERNEL_STACK(%esi),%ecx	/* get its kernel stack */
	lea	KERNEL_STACK_SIZE-IKS_SIZE-IEL_SIZE(%ecx),%ebx
						/* point to stack top */

	movl	%esi,CX(_active_threads,%edx)	/* new thread is active */
	movl	%ecx,CX(_active_stacks,%edx)	/* set current stack */
	movl	%ebx,CX(_kernel_stack,%edx)	/* set stack top */

	movl	KSS_ESP(%ecx),%esp		/* switch stacks */
	movl	KSS_ESI(%ecx),%esi		/* restore registers */
	movl	KSS_EDI(%ecx),%edi
	movl	KSS_EBP(%ecx),%ebp
	movl	KSS_EBX(%ecx),%ebx
	jmp	*KSS_EIP(%ecx)			/* return old thread */

ENTRY(Thread_continue)
	pushl	%eax				/* push the thread argument */
	xorl	%ebp,%ebp			/* zero frame pointer */
	call	*%ebx				/* call real continuation */

#if	NCPUS > 1
/*
 * void switch_to_shutdown_context(thread_t thread,
 *				   void (*routine)(processor_t),
 *				   processor_t processor)
 *
 * saves the kernel context of the thread,
 * switches to the interrupt stack,
 * continues the thread (with thread_continue),
 * then runs routine on the interrupt stack.
 *
 * Assumes that the thread is a kernel thread (thus
 * has no FPU state)
 */
ENTRY(switch_to_shutdown_context)
	CPU_NUMBER(%edx)
	movl	_active_stacks(,%edx,4),%ecx	/* get old kernel stack */
	movl	%ebx,KSS_EBX(%ecx)		/* save registers */
	movl	%ebp,KSS_EBP(%ecx)
	movl	%edi,KSS_EDI(%ecx)
	movl	%esi,KSS_ESI(%ecx)
	popl	KSS_EIP(%ecx)			/* save return PC */
	movl	%esp,KSS_ESP(%ecx)		/* save SP */

	movl	0(%esp),%eax			/* get old thread */
	movl	%ecx,TH_KERNEL_STACK(%eax)	/* save old stack */
	movl	$0,TH_SWAP_FUNC(%eax)		/* clear continuation */
	movl	4(%esp),%ebx			/* get routine to run next */
	movl	8(%esp),%esi			/* get its argument */

	movl	_interrupt_stack(,%edx,4),%ecx	/* point to its interrupt stack */
	lea	INTSTACK_SIZE(%ecx),%esp	/* switch to it (top) */
	
	pushl	%eax				/* push thread */
	call	EXT(thread_dispatch)		/* reschedule thread */
	addl	$4,%esp				/* clean stack */

	pushl	%esi				/* push argument */
	call	*%ebx				/* call routine to run */
	hlt					/* (should never return) */

#endif	NCPUS > 1

