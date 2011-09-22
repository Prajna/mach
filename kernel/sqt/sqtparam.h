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
 * $Log:	sqtparam.h,v $
 * Revision 2.3  91/07/31  18:04:30  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:00:43  dbg
 * 	Adapted from Sequent SYMMETRY sources.
 * 	[91/04/26  15:00:13  dbg]
 * 
 */

/*
 * Various tuning parameters.
 */

#ifndef	_SQT_SQTPARAM_H_
#define	_SQT_SQTPARAM_H_

#define	MAXNUMCPU	30	/* max supported # processors.  Used in */
				/* various statistics, most of */
				/* kernel is independent of this */

#define	MAXNUMSEC	6	/* max supported # SEC controllers. */
				/* Used in autoconf, etc. especially for */
				/* handling ambiguous wildcards. */
				/* Most of kernel is independent of this. */


#define	MEMINTVL	(60*10)	/* Memory ecc error polling interval */
				/* Check for errors every 10 minutes */

#endif	/* _SQT_SQTPARAM_H_ */
