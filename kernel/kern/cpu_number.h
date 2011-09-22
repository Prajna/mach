/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
 * $Log:	cpu_number.h,v $
 * Revision 2.4  93/01/14  17:33:48  danner
 * 	ANSI C cleanups.
 * 	[92/12/10  18:01:47  af]
 * 
 * Revision 2.3  91/05/14  16:40:40  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:25:48  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:11:32  mrt]
 * 
 * Revision 2.1  89/08/03  15:45:22  rwd
 * Created.
 * 
 *  8-Aug-88  David Golub (dbg) at Carnegie-Mellon University
 *	Created.
 *
 */

#ifndef _KERN_CPU_NUMBER_H_
#define _KERN_CPU_NUMBER_H_

#include <cpus.h>

/*
 *	Definitions for cpu identification in multi-processors.
 */

int	master_cpu;	/* 'master' processor - keeps time */

#if	(NCPUS == 1)
	/* cpu number is always 0 on a single processor system */
#define	cpu_number()	(0)

#else	/* NCPUS == 1 */
	/* get cpu_number definition from machine-dependent code */
#include <machine/cpu_number.h>

#endif	/* NCPUS == 1 */
#endif /* _KERN_CPU_NUMBER_H_ */
