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
 * $Log:	context.h,v $
 * Revision 2.2  93/01/14  17:12:20  danner
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:12:17  af]
 * 
 * 	Created.
 * 	[92/05/31            af]
 * 
 *	Created.
 *
 */

/*
 *	File: context.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	5/92
 *
 *	Register save definitions for non-local goto's
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 */

#ifndef	_ALPHA_CONTEXT_H_
#define	_ALPHA_CONTEXT_H_	1

#ifndef	ASSEMBLER
typedef struct {
	long	s0;
	long	s1;
	long	s2;
	long	s3;
	long	s4;
	long	s5;
	long	s6;
	long	sp;
	long	pc;
	long	ps;
} jmp_buf;

typedef struct hw_pcb {
	vm_offset_t	ksp;
	vm_offset_t	esp;
	vm_offset_t	ssp;
	vm_offset_t	usp;
	vm_offset_t	ptbr;
	long		asn;
	long		ast_status;
	long		fpa_enabled;
	long		cycle_counter;
	long		process_unique;
	long		pal_scratch[6];
} *hw_pcb_t;

#endif	ASSEMBLER
#endif	_ALPHA_CONTEXT_H_
