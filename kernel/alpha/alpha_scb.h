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
 * $Log:	alpha_scb.h,v $
 * Revision 2.2  93/01/14  17:11:36  danner
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:11:38  af]
 * 
 * 	Created.
 * 	[92/06/03            af]
 * 
 */

/*
 *	File: alpha_scb.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Alpha System Control Block, dynamically settable entries
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 */

#define	SCB_SOFTCLOCK		81
#define	SCB_CLOCK		96
#define	SCB_INTERPROC		97

#define	SCB_INTERRUPT_FIRST	128
#define	SCB_MAX_INTERRUPTS	512

#define	N_SCB_ENTRIES		(128+512)

#ifndef	ASSEMBLER

extern boolean_t	alpha_set_scb_entry( unsigned int entry_no,
					     void (*routine)() );

extern boolean_t	alpha_clear_scb_entry( unsigned int entry_no);

#endif	/* ! ASSEMBLER */
