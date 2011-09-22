/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * Copyright (c) 1991 IBM Corporation 
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation,
 * and that the name IBM not be used in advertising or publicity 
 * pertaining to distribution of the software without specific, written
 * prior permission.
 * 
 * CARNEGIE MELLON AND IBM ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON AND IBM DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	hardclock.c,v $
 * Revision 2.9  93/02/04  07:56:18  danner
 * 	Integrate PS2 and non PS2 cases into one routine.
 * 	[93/01/25            rvb]
 * 
 * 	Don't check for SPL 0 here.  Interrupt return code checks
 * 	for pending softclock interrupt.
 * 	[92/04/19            dbg@ibm]
 * 
 * 	PS2-only version.  Different interrupt stack format,
 * 	clock interrupt reset code.  Does NOT use ABIOS.
 * 	[92/03/30            dbg@ibm]
 * 
 * Revision 2.8  91/07/31  17:36:33  dbg
 * 	New interrupt save area.
 * 	[91/07/30  16:50:59  dbg]
 * 
 * Revision 2.7  91/06/19  11:55:09  rvb
 * 	cputypes.h->platforms.h
 * 	[91/06/12  13:44:49  rvb]
 * 
 * Revision 2.6  91/05/14  16:08:38  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/05/08  12:37:56  dbg
 * 	Include sqt/intctl.h if building for Sequent.
 * 	[91/03/21            dbg]
 * 
 * Revision 2.4  91/02/05  17:12:01  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:34:29  mrt]
 * 
 * Revision 2.3  91/01/08  17:32:02  rpd
 * 	EFL_VM => user_mode
 * 	[90/12/21  10:50:54  rvb]
 * 
 * Revision 2.2  90/05/03  15:27:35  dbg
 * 	Created.
 * 	[90/03/06            dbg]
 * 
 */

/*
 * Clock interrupt.
 */
#include <platforms.h>

#include <kern/time_out.h>
#include <i386/thread.h>
#include <i386/eflags.h>

#ifdef	SYMMETRY
#include <sqt/intctl.h>
#endif
#if	defined(AT386) || defined(iPSC386)
#include <i386/ipl.h>
#endif
#ifdef	PS2
#include <i386/pic.h>
#include <i386/pio.h>
#endif	PS2

extern void	clock_interrupt();
extern char	return_to_iret[];

void
#ifdef	PS2
hardclock(iunit, ivect, old_ipl, ret_addr, regs)
        int     iunit;          /* 'unit' number */
	int	ivect;		/* interrupt number */
#else	/* PS2 */
hardclock(iunit,        old_ipl, ret_addr, regs)
        int     iunit;          /* 'unit' number */
	int	old_ipl;	/* old interrupt level */
#endif	/* PS2 */
	char *	ret_addr;	/* return address in interrupt handler */
	struct i386_interrupt_state *regs;
				/* saved registers */
{
	if (ret_addr == return_to_iret)
	    /*
	     * Interrupt from user mode or from thread stack.
	     */
	    clock_interrupt(tick,			/* usec per tick */
			    (regs->efl & EFL_VM) ||	/* user mode */
			    ((regs->cs & 0x03) != 0),	/* user mode */
#ifdef	PS2
			    FALSE			/* ignore SPL0 */
#else	/* PS2 */
			    old_ipl == SPL0		/* base priority */
#endif	/* PS2 */
			    );
	else
	    /*
	     * Interrupt from interrupt stack.
	     */
	    clock_interrupt(tick,			/* usec per tick */
			    FALSE,			/* kernel mode */
			    FALSE);			/* not SPL0 */

#ifdef	PS2
	/*
	 * Reset the clock interrupt line.
	 */
	outb(0x61, inb(0x61) | 0x80);
#endif	/* PS2 */
}				
