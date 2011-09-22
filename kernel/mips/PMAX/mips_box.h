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
 * $Log:	mips_box.h,v $
 * Revision 2.10  91/08/24  12:21:46  af
 * 	Pass along interrupt frame pointer to intr routines:
 * 	on 3min we need to munge the SR on the fly.
 * 	[91/08/02  03:37:22  af]
 * 
 * Revision 2.9  91/05/14  17:24:48  mrt
 * 	Correcting copyright
 * 
 * Revision 2.8  91/02/14  14:34:57  mrt
 * 	Fix macros so that interrupt routines can drop priority as now required.
 * 	[91/02/12  12:56:04  af]
 * 
 * Revision 2.7  91/02/05  17:42:52  mrt
 * 	Added author notices
 * 	[91/02/04  11:15:27  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:14:11  mrt]
 * 
 * Revision 2.6  91/01/09  19:50:01  rpd
 * 	Now define mipsbox_memory_check() to cope with bus lockup
 * 	caused by pmax SII chip.
 * 	[91/01/02            af]
 * 
 * Revision 2.5  90/12/05  23:32:53  af
 * 	Re-Created, from the DEC specs:
 * 	"DECstation 5000/200 KN02 System Module Functional Specification"
 * 	 Workstation Systems Engineering, Palo Alto, CA. Aug 27, 1990.
 * 	"DECstation 3100 Desktop Workstation Functional Specification"
 * 	 Workstation Systems Engineering, Palo Alto, CA. Aug 28, 1990.
 * 	[90/09/25            af]
 */
/*
 *	File: mips_box.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	The MIPS chip set gets plugged in a box in some funny way.
 *	Here we want to specify ways by which the cpu can recognize
 *	what the box looks like, during autoconf.
 *
 */

#define dec_cputype(systword)	((systword >> 24) & 0xff)
#define dec_systype(systword)	((systword >> 16) & 0xff)
#define dec_revtype(systword)	(systword & 0xffff)
#define dec_frmrev(rev)		((rev >> 8) & 0xff)
#define dec_hrdrev(rev)		(rev & 0xff)

#define	CONFIG_NOCP1		0		/* xxx */

#define	INT_LEV_CONSOLE		INT_LEV5
#define	IP_LEV_FPA		IP_LEV7

#define RDCLR_BUSERR		0xb7000000	/* pmax */
#define	WRCLR_BUSERR		0xbfd80000	/* 3max */

/*
 *	Interrupt handler definitions
 */
#define mipsbox_intr7(ss,cs,st,spl)		fpa_intr(ss)

#ifndef	ASSEMBLER
extern int (*pmax_intr2)(), (*pmax_intr3)(), (*pmax_intr4)(),
	   (*pmax_intr5)(), (*pmax_intr6)();
extern boolean_t (*pmax_memcheck)();
#endif	ASSEMBLER

#define mipsbox_intr6(ss,cs,st,spl)		(*pmax_intr6)((st),(spl),(ss))
#define mipsbox_intr5(ss,cs,st,spl)		(*pmax_intr5)((st),(spl),(ss))
#define mipsbox_intr4(ss,cs,st,spl)		(*pmax_intr4)((st),(spl),(ss))
#define mipsbox_intr3(ss,cs,st,spl)		(*pmax_intr3)((st),(spl),(ss))
#define mipsbox_intr2(ss,cs,st,spl)		(*pmax_intr2)((st),(spl),(ss))

#define mipsbox_memory_check(ss)	(*pmax_memcheck)((ss)->bad_address,(ss)->pc)
