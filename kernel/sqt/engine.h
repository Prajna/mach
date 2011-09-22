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
 * $Log:	engine.h,v $
 * Revision 2.3  91/07/31  18:00:44  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  12:55:47  dbg
 * 	Adapted from Sequent SYMMETRY sources.
 * 	[91/04/26  14:51:36  dbg]
 * 
 */

/*
 * $Header: engine.h,v 2.3 91/07/31 18:00:44 dbg Exp $
 *
 * engine.h
 *	Per-processor basic "engine" structure.  Fundamental representation
 *	of a processor for dispatching and initialization.
 *
 * Allocated per-processor at boot-time in an array.  Base stored in
 * "engine".
 */

/*
 * Revision 1.1  89/07/19  14:48:54  kak
 * Initial revision
 * 
 * Revision 2.7  89/02/20  07:59:43  djg
 * fixed comment for E_FPU387 flag. 1=387 0 = no 387
 * 
 * Revision 2.6  88/11/10  08:25:57  djg
 * bak242 
 * 
 */

#ifndef	_SQT_ENGINE_H_
#define	_SQT_ENGINE_H_

struct	engine	{
	unsigned char	e_slicaddr;	/* the processor's SLIC address	*/
	char		e_unused;
	short		e_flags;	/* processor flags - see below	*/
	unsigned int	e_diag_flag;	/* copy of power-up diagnostic flags */
	int		e_cpu_speed;	/* copy of config cpu speed     */
	unsigned int	e_fpuon;	/* bits to turn on fpu */
	unsigned int	e_fpuoff;	/* bits to turn on fpu */
};

/* currently defined flag bits */
#define	E_OFFLINE	0x01		/* processor is off-line	*/
#define E_BAD		0x02		/* processor is bad		*/
#define	E_SHUTDOWN	0x04		/* shutdown has been requested	*/ 
#define E_DRIVER	0x08		/* processor has driver bound	*/
#define E_PAUSED	0x10		/* processor paused - see panic */
#define	E_FPU387	0x20		/* 1==387 0 not (i386 only) */
#define	E_FPA		0x40		/* processor has an FPA (i386 only) */
#define E_NOWAY		(E_OFFLINE|E_BAD|E_SHUTDOWN|E_PAUSED)

/* Cannot switch process to Engine - see runme */
#define E_UNAVAIL	-1

#ifdef KERNEL
extern	struct engine	*engine;	/* Engine Array Base */
extern	struct engine	*engine_Nengine;/* just past Engine Array Base */
extern	unsigned 	Nengine;	/* # Engines to alloc at boot */
extern	unsigned	nonline;	/* count of online engines */
#endif KERNEL

#endif	/* _SQT_ENGINE_H_ */

