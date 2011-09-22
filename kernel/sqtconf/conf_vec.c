/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * Copyright (c) 1991 Sequent Computer Systems
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND SEQUENT COMPUTER SYSTEMS ALLOW FREE USE OF
 * THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * SEQUENT COMPUTER SYSTEMS DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	conf_vec.c,v $
 * Revision 2.3  91/07/31  18:05:40  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:03:05  dbg
 * 	Added, from Sequent SYMMETRY sources.
 * 	[91/02/26            dbg]
 * 
 */

#ifndef lint
static char rcsid[] = "$Header: conf_vec.c,v 2.3 91/07/31 18:05:40 dbg Exp $";
#endif lint

/*
 * vec_conf.c
 *
 * This file contains the definition of software
 * interrupt and trap vectors.
 */

/*
 * Revision 1.1  89/07/05  13:17:42  kak
 * Initial revision
 * 
 * Revision 2.2  88/03/24  10:47:02  dilip
 * Dropped FPU_PGXBUG define. Now always on.
 * 
 */

/*
 * Software Interrupt Vectors.
 */

extern	int	undef();
extern	int	softclock();
extern	int	pmap_update_interrupt();

int	(*softvec[8])() = {
	undef,    	/* 0 */
	undef, 		/* 1 */
	undef,   	/* 2 */
	undef,		/* 3 */
	softclock, 	/* 4 */
	undef, 		/* 5 */
	undef, 		/* 6 */
	pmap_update_interrupt,		/* 7 */
};
