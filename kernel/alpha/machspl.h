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
 * $Log:	machspl.h,v $
 * Revision 2.2  93/05/15  20:59:16  mrt
 * 	Used to be machparam.h
 * 	[93/05/15            mrt]
 * 
 * 
 */
/*
 *	File: machspl.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Machine-dependent SPL definitions.
 */

#ifndef	_MACHINE_MACHSPL_H_
#define	_MACHINE_MACHSPL_H_

#include <mach/machine/vm_types.h>
#include <alpha/alpha_cpu.h>

/*
 *	We trust THE BOOK that this is truly box-independent.
 */

typedef natural_t		spl_t;

extern spl_t			alpha_swap_ipl( spl_t );

#define	splx(s)			(void) alpha_swap_ipl(s)
#define	spl0()			alpha_swap_ipl(ALPHA_IPL_0)
#define	splsoftclock()		alpha_swap_ipl(ALPHA_IPL_SOFTC)
#define	splimp()		alpha_swap_ipl(ALPHA_IPL_IO)
#define	splbio()		alpha_swap_ipl(ALPHA_IPL_IO)
#define	spltty()		alpha_swap_ipl(ALPHA_IPL_IO)
#define	splclock()		alpha_swap_ipl(ALPHA_IPL_CLOCK)
#define	splhigh()		alpha_swap_ipl(ALPHA_IPL_HIGH)
#define	splvm()			splhigh()
#define	splsched()		splhigh()

#endif	_MACHINE_MACHSPL_H_
