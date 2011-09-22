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
 * $Log:	alpha_scb.c,v $
 * Revision 2.2  93/01/14  17:11:32  danner
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:11:48  af]
 * 
 * 	Created.
 * 	[92/06/03            af]
 * 
 */

/*
 *	File: alpha_scb.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Alpha System Control Block (exception and interrupt dispatch)
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 */

#include <mach/std_types.h>
#include <alpha/alpha_scb.h>

/*
 * We only play with interrupt vectors
 */
struct scb_entry {
	void	(*dispatcher_routine)( );
	void	(*service_routine)( );
};

extern void TRAP_interrupt( );
extern void stray_interrupt( );

extern struct scb_entry alpha_scb[N_SCB_ENTRIES];

boolean_t
alpha_set_scb_entry( unsigned int entry_no, void (*routine)())
{
	register struct scb_entry	*e;

	e = &alpha_scb[entry_no];

	/* sanity checks */
	if ((e < &alpha_scb[N_SCB_ENTRIES]) &&
	    (e->dispatcher_routine == TRAP_interrupt)) {
		alpha_scb[entry_no].service_routine = routine;
		return TRUE;
	}
	panic("set_scb_entry");
	return FALSE;
}

boolean_t
alpha_clear_scb_entry( unsigned int entry_no)
{
	return alpha_set_scb_entry( entry_no, stray_interrupt);
}
