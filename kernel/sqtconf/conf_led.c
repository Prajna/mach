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
 * $Log:	conf_led.c,v $
 * Revision 2.3  91/07/31  18:05:25  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:02:33  dbg
 * 	Adapted for pure Mach kernel.
 * 	[90/10/03            dbg]
 * 
 */

#ifndef	lint
static	char	rcsid[] = "$Header: conf_led.c,v 2.3 91/07/31 18:05:25 dbg Exp $";
#endif

/*
 * Configuration of front panel led light show
 */

/*
 * Revision 1.2  89/08/16  15:17:38  root
 * balance -> sqt
 * 
 * Revision 1.1  89/07/05  13:17:38  kak
 * Initial revision
 * 
 */

#include <sys/types.h>

#include <sqt/slicreg.h>
#include <sqt/clkarb.h>

#define	FP_LED(i)	(SL_C_FP_LIGHT + ((i) * 2))

/*
 * The front panel has 48 programmable leds. These are arranged in 12 columns
 * with 4 leds in each row. The front panel led's are addressed from
 * left to right, top to bottom.
 */

/*
 * Currently assumes only processors will turn on lights.
 * Table is indexed by processor number. The first MAXNUMPROC entries
 * are reserved for processor use.
 */
u_char	fp_lightmap[FP_NLIGHTS] = {
	FP_LED(0),
	FP_LED(1),
	FP_LED(2),
	FP_LED(3),
	FP_LED(4),
	FP_LED(5),
	FP_LED(6),
	FP_LED(7),
	FP_LED(8),
	FP_LED(9),
	FP_LED(10),
	FP_LED(11),
	FP_LED(12),
	FP_LED(13),
	FP_LED(14),
	FP_LED(15),
	FP_LED(16),
	FP_LED(17),
	FP_LED(18),
	FP_LED(19),
	FP_LED(20),
	FP_LED(21),
	FP_LED(22),
	FP_LED(23),
	FP_LED(24),
	FP_LED(25),
	FP_LED(26),
	FP_LED(27),
	FP_LED(28),
	FP_LED(29),
	FP_LED(30),
	FP_LED(31),
	FP_LED(32),
	FP_LED(33),
	FP_LED(34),
	FP_LED(35),
	FP_LED(36),
	FP_LED(37),
	FP_LED(38),
	FP_LED(39),
	FP_LED(40),
	FP_LED(41),
	FP_LED(42),
	FP_LED(43),
	FP_LED(44),
	FP_LED(45),
	FP_LED(46),
	FP_LED(47)
};
