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
 * $Log:	start.s,v $
 * Revision 2.10  92/02/19  15:09:33  elf
 * 	Changed #-style comments, for ANSI cpp.
 * 	[92/02/19  13:11:27  rpd]
 * 
 * Revision 2.9  91/08/24  12:24:22  af
 * 	Moved any call to C that can be made from C in C code.
 * 	Save all 4 arguments for later inspection: new DEC
 * 	proms are more creative.
 * 	[91/08/02  03:17:56  af]
 * 
 * Revision 2.8  91/06/20  22:09:59  rvb
 * 	Make it assemble with the 2.1 compiler
 * 
 * Revision 2.7  91/05/14  17:38:30  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/02/05  17:51:38  mrt
 * 	Added author notices
 * 	[91/02/04  11:24:52  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:29:38  mrt]
 * 
 * Revision 2.5  90/12/05  23:38:49  af
 * 
 * 
 * Revision 2.4  90/12/05  20:50:13  af
 * 	Simplified a bit more.  I love killing code.
 * 	[90/12/03  23:05:12  af]
 * 
 * Revision 2.3  90/09/09  23:21:16  rpd
 * 	Removed environ thing.  It's not there and not worth the pain.
 * 	Use prom_getenv() if you need to query the prom.
 * 	[90/09/05            af]
 * 
 * Revision 2.2  89/11/29  14:15:20  af
 * 	Zero initial status, let mips_init go where it has to.
 * 	[89/11/03  16:28:19  af]
 * 
 * 	Rewritten for pure kernel.
 * 	[89/10/04            af]
 * 
 * Revision 2.3  89/07/14  15:27:50  rvb
 * 	Make _argv external for, check_debug.c
 * 	[89/07/14            rvb]
 * 
 * Revision 2.2  89/05/31  12:29:26  rvb
 * 	Flush unused "entry points". [af]
 * 
 * Revision 2.1  89/05/30  12:55:34  rvb
 * Rcs'ed
 * 
 * 13-Jan-89  Alessandro Forin (af) at Carnegie-Mellon University
 *	Created.
 */
/*
 *	File: start.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	1/89
 *
 *	Kernel startup code.
 *	This is where we receive control from the PROM.
 */

#include <mach/mips/asm.h>
#include <mach/mips/vm_param.h>
#include <mips/mips_cpu.h>

	.set	noreorder

/*
 * We better get a stack for ourselves quickly.
 * Note: the "intstack" misnomer is because the machine-independent
 * code knows about this symbol, which therefore must be defined.
 */
	.text				/* so that it goes upfront */
	.align	2
	.globl	intstack
intstack:
	.space	INTSTACK_SIZE-4
	.globl	boot_stack
boot_stack:
	.space	4

/*
 *	This is the initial kernel entry point.
 *	We are somehow vulnerable to what the loader on the
 *	given machine does, but in general we should be called
 *	just like normal U*x programs.
 */
#define ENTRY_FRAME	(4*4)+4
					/* 4 argsaves + ra */
	.sdata
EXPORT(prom_arg0)	.word 0		/* should anyone need to look at it */
EXPORT(prom_arg1)	.word 0		/* NOT in BSS!! */
EXPORT(prom_arg2)	.word 0
EXPORT(prom_arg3)	.word 0

	.text

EXPORT(eprol)				/* profiling */
NESTED(start, ENTRY_FRAME, zero)

	move	ra,zero			/* no going back */

	la	gp,_gp			/* load up GP register */
	la	sp,boot_stack		/* switch stack */
	subu	sp,ENTRY_FRAME

	sw	a0,prom_arg0
	sw	a1,prom_arg1
	sw	a2,prom_arg2
	sw	a3,prom_arg3
	sw	ra,(ENTRY_FRAME-4)(sp)

	move	a2,zero			/* no environ, make clear */
	move	a3,zero			/* no extra args, either */

	mtc0	zero,c0_status		/* running in kernel mode */

	sw	zero,ENTRY_FRAME-4(sp)	/* zero old ra for kdb */

	mtc0	zero,c0_tlbhi		/* kernel's ptes */
	li	t0,KPTEADDR
	mtc0	t0,c0_tlbcxt

	jal	mips_init		/* cold initialization */
	nop
	/* NOTREACHED */
	END(start)
