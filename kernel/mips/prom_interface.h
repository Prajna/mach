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
 * $Log:	prom_interface.h,v $
 * Revision 2.9  91/08/24  12:24:10  af
 * 	Reduced the number of entry points to the ones we actually use.
 * 	[91/06/25            af]
 * 
 * Revision 2.8  91/06/19  11:56:35  rvb
 * 	#ifdef PMAX -> #ifdef DECSTATION and we include <platforms.h>
 * 	[91/06/12  14:09:01  rvb]
 * 
 * Revision 2.7  91/05/14  17:37:30  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/02/05  17:50:47  mrt
 * 	Added author notices
 * 	[91/02/04  11:24:31  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:28:51  mrt]
 * 
 * Revision 2.5  90/12/05  23:38:40  af
 * 
 * 
 * Revision 2.4  90/12/05  20:50:11  af
 * 	Added decl for prom_getenv(), dropped some unused defs.
 * 	[90/12/03  23:04:48  af]
 * 
 * Revision 2.3  90/09/09  23:21:11  rpd
 * 	Removed depends on MIPSco compiler defines.
 * 	[90/09/05  10:30:34  af]
 * 
 * Revision 2.2  89/11/29  14:15:12  af
 * 	Created.
 * 	[89/10/06            af]
 */
/*
 *	File: prom_interface.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Functions and data structures that link the kernel
 *	to the prom environment.
 */
#include <platforms.h>

#define	PROM_ENTRY(x)	(VEC_RESET+((x)*8))	/* Prom jump table */

/*
 * In the MI code we only need halt, reboot, putchar, getenv
 */
#ifndef	ASSEMBLER
extern struct _prom_call {
	void	(*halt)(/**/);
	void	(*reboot)(/**/);
	int	(*putchar)(/* char */);
	char	*(*getenv)(/* char* */);
} prom_call;

#define	prom_halt	(*prom_call.halt)
#define	prom_reboot	(*prom_call.reboot)
#define	prom_putchar	(*prom_call.putchar)
#define	prom_getenv	(*prom_call.getenv)

#endif	ASSEMBLER


#ifdef	MSERIES
#define	PROM_HALT	PROM_ENTRY(2)	/* re-enter monitor command loop */
#define	PROM_AUTOBOOT	PROM_ENTRY(5)	/* autoboot the system */
#define	PROM_PUTCHAR	PROM_ENTRY(12)	/* putchar to console */
#define	PROM_GETENV	PROM_ENTRY(33)	/* get environment variable */
#endif	/*MSERIES*/


#ifdef	DECSTATION
#define	PROM_HALT	PROM_ENTRY(2)	/* re-enter monitor command loop */
#define	PROM_AUTOBOOT	PROM_ENTRY(5)	/* autoboot the system */
#define	PROM_PUTCHAR	PROM_ENTRY(13)	/* putchar to console */
#define	PROM_GETENV	PROM_ENTRY(33)	/* get environment variable */
#endif	/*DECSTATION*/


/*
 * Restart block -- monitor support for "warm" starts
 *
 * prom will perform "warm start" if restart_blk is properly set-up:
 *	rb_magic == RESTART_MAGIC
 *	rb_occurred == 0
 *	rb_checksum == 2's complement, 32-bit sum of first 32, 32-bit words 
 */
#define	RESTART_ADDR	0xa0000400	/* prom restart block */
#define	RESTART_MAGIC	0xfeedface
#define	RESTART_CSUMCNT	32		/* chksum 32 words of restart routine */
#define	RB_BPADDR	(RESTART_ADDR+24)/* address of rb_bpaddr */

#ifndef	ASSEMBLER
struct restart_blk {
	int	rb_magic;		/* magic pattern */
	int	(*rb_restart)();	/* restart routine */
	int	rb_occurred;		/* to avoid loops on restart failure */
	int	rb_checksum;		/* checksum of 1st 32 wrds of restrt */
	char	*rb_fbss;		/* start of prom bss and stack area */
	char	*rb_ebss;		/* end of prom bss and stack area */
	int	(*rb_bpaddr)();		/* breakpoint handler */
	int	(*rb_vtop)();		/* virtual to physical conversion rtn */
};

#endif ASSEMBLER
