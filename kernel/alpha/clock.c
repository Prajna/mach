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
 * $Log:	clock.c,v $
 * Revision 2.2  93/01/14  17:12:08  danner
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:12:06  af]
 * 
 * 	Created.
 * 	[92/06/03            af]
 * 
 */

/*
 *	File: alpha_clock.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Alpha System Clock handler
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 */

#include <mach/std_types.h>

#include <alpha/alpha_cpu.h>
#include <alpha/alpha_scb.h>

extern void softclock();

void alpha_clock(
	struct alpha_saved_state	*ss_ptr,
	natural_t			r4,
	natural_t			r5,
	natural_t			cause)
{
	register natural_t		ps = ss_ptr->framep->saved_ps;
	extern int			tick;

	clock_interrupt( (integer_t)tick,
			 alpha_user_mode(ps),
			 (ps & PS_IPL_MASK) == 0);
}

startrtclock()
{
	alpha_set_scb_entry( SCB_CLOCK, alpha_clock);
	alpha_set_scb_entry( SCB_SOFTCLOCK, softclock);
}
stopclocks()
{
	alpha_clear_scb_entry( SCB_CLOCK );
}
resettodr()
{
}

