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
 * $Log:	asm_misc.h,v $
 * Revision 2.6  91/08/24  12:18:07  af
 * 	Rid of longjmp crap.  Make callbacks vectorable, as needed
 * 	by new interface on 3min.  Described 3min interface.
 * 	[91/08/22  11:20:22  af]
 * 
 * Revision 2.5  91/05/14  17:17:04  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:38:27  mrt
 * 	Added author notices
 * 	[91/02/04  11:09:12  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:03:56  mrt]
 * 
 * Revision 2.3  90/12/05  23:29:20  af
 * 	Created.
 * 	[90/12/02            af]
 */
/*
 *	File: asm_misc.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	12/90
 *
 *	Miscellaneous definitions used in the assembly code.
 */

/*
 * PROM callback interface
 */
#ifndef	ASSEMBLER
extern struct _prom_call {
	void	(*halt)(/**/);
	int	(*open)(/* char*, int */);
	int	(*lseek)(/* int, long, int */);
	int	(*read)(/* int, caddr_t, int */);
	int	(*close)(/* int */);
	int	(*gets)(/* char* */);
	int	(*printf)(/* char*, ... */);
	char	*(*getenv)(/* char* */);
} prom_call;

#define	prom_halt	(*prom_call.halt)
#define	prom_open	(*prom_call.open)
#define	prom_lseek	(*prom_call.lseek)
#define	prom_read	(*prom_call.read)
#define	prom_close	(*prom_call.close)
#define	prom_gets	(*prom_call.gets)
#define	prom_printf	(*prom_call.printf)
#define	prom_getenv	(*prom_call.getenv)

#endif	ASSEMBLER

/*
 * PMAX/3MAX implementation
 */
#define	PMAX_PROM_ENTRY(x)	(VEC_RESET+((x)*8))	/* Prom jump table */

#define	PMAX_PROM_HALT		PMAX_PROM_ENTRY(2)
#define	PMAX_PROM_OPEN		PMAX_PROM_ENTRY(6)
#define	PMAX_PROM_READ		PMAX_PROM_ENTRY(7)
#define	PMAX_PROM_CLOSE		PMAX_PROM_ENTRY(10)
#define	PMAX_PROM_LSEEK		PMAX_PROM_ENTRY(11)
#define	PMAX_PROM_GETS		PMAX_PROM_ENTRY(15)
#define	PMAX_PROM_PRINTF	PMAX_PROM_ENTRY(17)
#define	PMAX_PROM_GETENV	PMAX_PROM_ENTRY(33)

/*
 * 3MIN implementation
 */

#define	KMIN_PROM_MAGIC		0x30464354

#define	KMIN_PROM_REX		0xac
#define	KMIN_PROM_HALT		0x9c
#define	KMIN_PROM_OPEN		0x54
#define	KMIN_PROM_READ		0x58
/*#define	KMIN_PROM_CLOSE		notimpl */
/*#define	KMIN_PROM_LSEEK		notimpl */
#define	KMIN_PROM_GETS		0x28
#define	KMIN_PROM_PRINTF	0x30
#define	KMIN_PROM_GETENV	0x64

