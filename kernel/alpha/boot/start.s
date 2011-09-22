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
 * HISTORY
 * $Log:	start.s,v $
 * Revision 2.5  93/05/20  21:02:55  mrt
 * 	Changed use of zero to ra in call to NESTED.
 * 	[93/05/18            mrt]
 * 
 * Revision 2.4  93/05/17  18:11:58  mrt
 * 	Must set sp again, let it start below text so that 
 * 	endless recursion (trap inside trap) does not clobbber
 * 	data.
 * 	Fix from Michael Uhlenberg of PCS.
 * 	[93/05/17            mrt]
 * 
 * Revision 2.3  93/03/09  10:49:45  danner
 * 	Call main function _main_ so that GCC won't generate
 * 	its bogon extra call.
 * 	[93/02/20            af]
 * 
 * Revision 2.2  93/02/05  08:01:19  danner
 * 	[93/02/04  01:02:11  af]
 * 	Created.
 * 
 */
/*
 *	File: start.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	12/92
 *
 *	Entry point for Bootstrap.
 */

#include <mach/alpha/asm.h>

/*
 *	Object:
 *		start				EXPORTED function
 *
 *		Entry point for standalone pgms (boots)
 *
 *	Arguments:
 *		a0				long
 *
 *	This is where the prom comes to.
 * 	Leaves all exception and interrupts to prom,
 * 	runs off prom's stack too.
 *
 *	Argument is first free page (physical)
 *
 * 	No meaningful return values.
 */
/*
 */

	.text
	.set	noreorder

#define ENTRY_FRAME	32

NESTED(start, 1, ENTRY_FRAME, ra, 0, 0)

	br	pv,1f			/* just in case */
1:

	ldgp	gp,0(pv)		/* load up GP register */
#if	__GNU_AS__
	setgp	0(pv)
#endif
	lda     sp,start        	/* let stack grow below text */
	lda	sp,-ENTRY_FRAME(sp)

					/* put arg in save area */
	stq	a0,(ENTRY_FRAME-8)(sp)
	CALL(init_prom_calls)		/* init prom callback vector */

	ldq	a0,(ENTRY_FRAME-8)(sp)
					/* xfer to C */
	CALL(_main_)

	/* if we ever get back */

	br	zero,prom_halt

	END(start)

LEAF(cpu_number,0)
	call_pal	0x3f/*op_mfpr_whami*/
	RET
	END(cpu_number)
