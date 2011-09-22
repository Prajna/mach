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
 * $Log:	machspl.h,v $
 * Revision 2.2  93/05/15  21:36:20  mrt
 * 	Used to be machspl.h
 * 	[93/05/15            mrt]
 * 
 * Revision 2.6  93/02/05  08:04:29  danner
 * 	Corrected splx proto now that we have protos and a compiler that
 * 	groks void better.
 * 	[93/01/19            af]
 * 	Proper spl typing.
 * 	[92/12/01            af]
 * 
 * Revision 2.5  91/08/24  12:22:57  af
 * 	Made spls indirect jumps, to allow more flexibility in redefining
 * 	them.  This is dictated by the ridicoulous interrupt dispatching
 * 	found on the DEC 3min, which mixes both different levels and an
 * 	interrupt mask on one level.
 * 	[91/06/24            af]
 * 
 * Revision 2.4  91/05/14  17:35:02  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:49:02  mrt
 * 	Added author notices
 * 	[91/02/04  11:23:02  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:26:43  mrt]
 * 
 * Revision 2.2  89/11/29  14:14:21  af
 * 	Created.
 * 	[89/10/12            af]
 */
/*
 *	File: machparam.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Machine-dependent SPL definitions.
 */

#ifndef	_MACHINE_MACHSPL_H_
#define	_MACHINE_MACHSPL_H_

/*
 *	SPLs are true functions on PMAX, defined elsewhere.
 *	But they can be box-dependent in many interesting ways
 */

typedef unsigned int	spl_t;

extern struct _spl_vector {
	void	(*splx_function)( spl_t old_status );
	spl_t	(*spl0_function)();
	spl_t	(*splsoftclock_function)();
	spl_t	(*splnet_function)();
	spl_t	(*splimp_function)();
	spl_t	(*splbio_function)();
	spl_t	(*spltty_function)();
	spl_t	(*splclock_function)();
	spl_t	(*splvm_function)();
	spl_t	(*splhigh_function)();
} spl_vector;

#define	splx(s)			(*spl_vector.splx_function)(s)
#define	spl0()			(*spl_vector.spl0_function)()
#define	splsoftclock()		(*spl_vector.splsoftclock_function)()
#define	splnet()		(*spl_vector.splnet_function)()
#define	splimp()		(*spl_vector.splimp_function)()
#define	splbio()		(*spl_vector.splbio_function)()
#define	spltty()		(*spl_vector.spltty_function)()
#define	splclock()		(*spl_vector.splclock_function)()
#define	splvm()			(*spl_vector.splvm_function)()
#define	splhigh()		(*spl_vector.splhigh_function)()
#define	splsched()		(*spl_vector.splhigh_function)()

#endif	_MACHINE_MACHSPL_H_
