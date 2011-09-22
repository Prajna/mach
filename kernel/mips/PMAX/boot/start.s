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
 * Revision 2.6  91/08/24  12:20:50  af
 * 	Rid of jumpbuffer, shrinked stack frame, call which_prom()
 * 	asap to set up callback vector, moved exit() elsewhere.
 * 	[91/08/22  11:25:55  af]
 * 
 * Revision 2.5  91/05/14  17:18:34  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:39:16  mrt
 * 	Added author notices
 * 	[91/02/04  11:11:06  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:06:30  mrt]
 * 
 * Revision 2.3  90/12/05  23:30:13  af
 * 	Created.
 * 	[90/12/02            af]
 * 
 */
/*
 *	File: start.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	12/90
 *
 *	Entry point for Bootstrap.
 */

#include <mach/mips/asm.h>
#include <mips/PMAX/boot/asm_misc.h>

/*
 *	Object:
 *		start				EXPORTED function
 *
 *		Entry point for standalone pgms (boots)
 *
 *	Arguments:
 *		a0				int
 *		a1				char **
 *		a2				char ** [or int]
 *		[a3				caddr_t]
 *
 *	This is where the prom comes to.
 * 	Leaves all exception and interrupts to prom,
 * 	runs off prom's stack too.
 *
 * 	First two args as in Unix: number of args and vector
 *	of strings.  Third arg either as in Unix (MIPSco) a
 *	vector of environment strings, or something else (DEC).
 *
 * 	No meaningful return values.
 */
/*
 */
IMPORT(prom_restart,4)

	.text
	.set	noreorder

#define ENTRY_FRAME	512

NESTED(start, ENTRY_FRAME, zero)

	la	gp,_gp			# load up GP register

	subu	sp,ENTRY_FRAME

					# put args in save area
	sw	a0,ENTRY_FRAME(sp)
	sw	a1,ENTRY_FRAME+4(sp)
	sw	a2,ENTRY_FRAME+8(sp)
	sw	a3,ENTRY_FRAME+12(sp)
	jal	which_prom		# init prom callback vector
	sw	zero,ENTRY_FRAME-4(sp)	# backtrace stop

	lw	a0,ENTRY_FRAME(sp)
	lw	a1,ENTRY_FRAME+4(sp)
	lw	a2,ENTRY_FRAME+8(sp)
					# xfer to C
	jal	main
	lw	a3,ENTRY_FRAME+12(sp)

	/* if we ever get back */

	j	exit
	move	a0,v0

	END(start)
