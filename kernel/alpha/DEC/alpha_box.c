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
 * $Log:	alpha_box.c,v $
 * Revision 2.4  93/05/15  19:10:36  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.3  93/03/09  10:48:05  danner
 * 	Dispatch scsi interrupts to TC handler.
 * 	[93/03/05            af]
 * 
 * Revision 2.2  93/02/05  07:56:51  danner
 * 	Changes for Flamingo
 * 	[93/01/12            jeffreyh]
 * 	Created.
 * 	[92/12/10  15:24:33  af]
 * 
 */
/*
 *	File: alpha_box.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Box-specific routines and data required by the MI code
 */

#include <platforms.h>

#include <kern/time_out.h>
#include <alpha/alpha_cpu.h>
#include <machine/machspl.h>		/* spl definitions */
#include <sys/types.h>
#include <chips/busses.h>

/*
 * Table of known, possible busses
 */
#if ADU
extern struct bus_driver
	tv_driver; 				/* ADU's TVBUS */
#endif /* ADU */

#if FLAMINGO
extern struct bus_driver
	tc_driver;	 			/* Flamingo's Turbo Channel */
#endif /* FLAMINGO */

/*
 * Interrupt routines and other busses
 * and descriptors
 */
#if ADU
extern int	tv_intr();

extern struct bus_driver
	asz_driver;	/* SCSI */
extern int
	asz_intr();
#endif /* ADU */

#if FLAMINGO
extern int  	asc_intr(), tc_intr(), tcds_intr();

extern struct bus_driver
	asc_driver;	/* SCSI */
#endif /* FLAMINGO */

struct bus_ctlr bus_master_init[] = {
/*driver,	name,	unit, intr, address+am, phys, adpt, alive, flags, */
/*
 * Machine busses first
 */
#if ADU
{ &tv_driver,   "tvbus",0,    tv_intr,  0x0,0,	0,    '?',    0,    BUS_CTLR, },
#endif /* ADU */
#if FLAMINGO
{ &tc_driver,   "tcbus",0,    tc_intr,   0x0,0,	0,    '?',    0,    BUS_CTLR, },
#endif FLAMINGO
/*
 * Other busses and disk controllers and..
 */
#ifdef	ADU
{ &asz_driver,  "asz",0,      asz_intr,  0x0,0,	0,    '?',    0,     0, },
#endif	ADU
#ifdef	FLAMINGO
{ &asc_driver,	"asc",	0,   tcds_intr,  0x0,0,	0,    '?',    0,    0, },
{ &asc_driver,	"asc",	1,   tcds_intr,  0x0,0,	0,    '?',    0,    0, },
{ &asc_driver,	"asc",	2,    asc_intr,  0x0,0,	0,      0,    0,    0, },
{ &asc_driver,	"asc",	3,    asc_intr,  0x0,0,	0,      1,    0,    0, },
{ &asc_driver,	"asc",	4,    asc_intr,  0x0,0,	0,      2,    0,    0, },
{ &asc_driver,	"asc",	5,    asc_intr,  0x0,0,	0,      3,    0,    0, },
{ &asc_driver,	"asc",	6,    asc_intr,  0x0,0,	0,      4,    0,    0, },
{ &asc_driver,	"asc",	7,    asc_intr,  0x0,0,	0,      5,    0,    0, },
#endif /* FLAMINGO */
	0
};

/*
 * Table of known, possible devices
 */
extern struct bus_driver
	adu_sl_driver, adu_se_driver,
	asic_driver, se_driver, scc_driver, isdn_driver,
	ga_driver, gq_driver, fb_driver, cfb_driver, sfb_driver,
	mc_driver;

extern int  	asic_intr(), se_intr(), scc_intr(), isdn_intr(),
		ga_intr(), gq_intr(), fb_intr(), cfb_intr(), sfb_intr(),
		mc_intr();

struct bus_device bus_device_init[] = {
/* driver,      name, unit,intr,addr+am,phys, adaptor,alive,ctlr,slave,flags,*/
/* Disk and tapes and.. SCSIs */
#if ADU
{ &asz_driver,	"rz",   0,  0,  0x0,0,	0,    '?',     0,   0,   0,    0, },
{ &asz_driver,	"rz",   1,  0,  0x0,0,	0,    '?',     0,   0,   1,    0, },
{ &asz_driver,	"rz",   2,  0,  0x0,0,	0,    '?',     0,   0,   2,    0, },
{ &asz_driver,	"rz",   3,  0,  0x0,0,	0,    '?',     0,   0,   3,    0, },
{ &asz_driver,	"rz",   4,  0,  0x0,0,	0,    '?',     0,   0,   4,    0, },
{ &asz_driver,	"rz",   5,  0,  0x0,0,	0,    '?',     0,   0,   5,    0, },
{ &asz_driver,	"rz",   6,  0,  0x0,0,	0,    '?',     0,   0,   6,    0, },
{ &asz_driver,	"rz",   7,  0,  0x0,0,	0,    '?',     0,   0,   7,    0, },
{ &asz_driver,	"rz",   8,  0,  0x0,0,	0,    '?',     0,   1,   0,    0, },
{ &asz_driver,	"rz",   9,  0,  0x0,0,	0,    '?',     0,   1,   1,    0, },
{ &asz_driver,	"rz",  10,  0,  0x0,0,	0,    '?',     0,   1,   2,    0, },
{ &asz_driver,	"rz",  11,  0,  0x0,0,	0,    '?',     0,   1,   3,    0, },
{ &asz_driver,	"rz",  12,  0,  0x0,0,	0,    '?',     0,   1,   4,    0, },
{ &asz_driver,	"rz",  13,  0,  0x0,0,	0,    '?',     0,   1,   5,    0, },
{ &asz_driver,	"rz",  14,  0,  0x0,0,	0,    '?',     0,   1,   6,    0, },
{ &asz_driver,	"rz",  15,  0,  0x0,0,	0,    '?',     0,   1,   7,    0, },
{ &asz_driver,	"rz",  16,  0,  0x0,0,	0,    '?',     0,   2,   0,    0, },
{ &asz_driver,	"rz",  17,  0,  0x0,0,	0,    '?',     0,   2,   1,    0, },
{ &asz_driver,	"rz",  18,  0,  0x0,0,	0,    '?',     0,   2,   2,    0, },
{ &asz_driver,	"rz",  19,  0,  0x0,0,	0,    '?',     0,   2,   3,    0, },
{ &asz_driver,	"rz",  20,  0,  0x0,0,	0,    '?',     0,   2,   4,    0, },
{ &asz_driver,	"rz",  21,  0,  0x0,0,	0,    '?',     0,   2,   5,    0, },
{ &asz_driver,	"rz",  22,  0,  0x0,0,	0,    '?',     0,   2,   6,    0, },
{ &asz_driver,	"rz",  23,  0,  0x0,0,	0,    '?',     0,   2,   7,    0, },
{ &asz_driver,	"rz",  24,  0,  0x0,0,	0,    '?',     0,   3,   0,    0, },
{ &asz_driver,	"rz",  25,  0,  0x0,0,	0,    '?',     0,   3,   1,    0, },
{ &asz_driver,	"rz",  26,  0,  0x0,0,	0,    '?',     0,   3,   2,    0, },
{ &asz_driver,	"rz",  27,  0,  0x0,0,	0,    '?',     0,   3,   3,    0, },
{ &asz_driver,	"rz",  28,  0,  0x0,0,	0,    '?',     0,   3,   4,    0, },
{ &asz_driver,	"rz",  29,  0,  0x0,0,	0,    '?',     0,   3,   5,    0, },
{ &asz_driver,	"rz",  30,  0,  0x0,0,	0,    '?',     0,   3,   6,    0, },
{ &asz_driver,	"rz",  31,  0,  0x0,0,	0,    '?',     0,   3,   7,    0, },

{ &asz_driver,	"tz",   0,  0,  0x0,0,	0,    '?',     0,   0,   0,    0, },
{ &asz_driver,	"tz",   1,  0,  0x0,0,	0,    '?',     0,   0,   1,    0, },
{ &asz_driver,	"tz",   2,  0,  0x0,0,	0,    '?',     0,   0,   2,    0, },
{ &asz_driver,	"tz",   3,  0,  0x0,0,	0,    '?',     0,   0,   3,    0, },
{ &asz_driver,	"tz",   4,  0,  0x0,0,	0,    '?',     0,   0,   4,    0, },
{ &asz_driver,	"tz",   5,  0,  0x0,0,	0,    '?',     0,   0,   5,    0, },
{ &asz_driver,	"tz",   6,  0,  0x0,0,	0,    '?',     0,   0,   6,    0, },
{ &asz_driver,	"tz",   7,  0,  0x0,0,	0,    '?',     0,   0,   7,    0, },
{ &asz_driver,	"tz",   8,  0,  0x0,0,	0,    '?',     0,   1,   0,    0, },
{ &asz_driver,	"tz",   9,  0,  0x0,0,	0,    '?',     0,   1,   1,    0, },
{ &asz_driver,	"tz",  10,  0,  0x0,0,	0,    '?',     0,   1,   2,    0, },
{ &asz_driver,	"tz",  11,  0,  0x0,0,	0,    '?',     0,   1,   3,    0, },
{ &asz_driver,	"tz",  12,  0,  0x0,0,	0,    '?',     0,   1,   4,    0, },
{ &asz_driver,	"tz",  13,  0,  0x0,0,	0,    '?',     0,   1,   5,    0, },
{ &asz_driver,	"tz",  14,  0,  0x0,0,	0,    '?',     0,   1,   6,    0, },
{ &asz_driver,	"tz",  15,  0,  0x0,0,	0,    '?',     0,   1,   7,    0, },
{ &asz_driver,	"tz",  16,  0,  0x0,0,	0,    '?',     0,   2,   0,    0, },
{ &asz_driver,	"tz",  17,  0,  0x0,0,	0,    '?',     0,   2,   1,    0, },
{ &asz_driver,	"tz",  18,  0,  0x0,0,	0,    '?',     0,   2,   2,    0, },
{ &asz_driver,	"tz",  19,  0,  0x0,0,	0,    '?',     0,   2,   3,    0, },
{ &asz_driver,	"tz",  20,  0,  0x0,0,	0,    '?',     0,   2,   4,    0, },
{ &asz_driver,	"tz",  21,  0,  0x0,0,	0,    '?',     0,   2,   5,    0, },
{ &asz_driver,	"tz",  22,  0,  0x0,0,	0,    '?',     0,   2,   6,    0, },
{ &asz_driver,	"tz",  23,  0,  0x0,0,	0,    '?',     0,   2,   7,    0, },
{ &asz_driver,	"tz",  24,  0,  0x0,0,	0,    '?',     0,   3,   0,    0, },
{ &asz_driver,	"tz",  25,  0,  0x0,0,	0,    '?',     0,   3,   1,    0, },
{ &asz_driver,	"tz",  26,  0,  0x0,0,	0,    '?',     0,   3,   2,    0, },
{ &asz_driver,	"tz",  27,  0,  0x0,0,	0,    '?',     0,   3,   3,    0, },
{ &asz_driver,	"tz",  28,  0,  0x0,0,	0,    '?',     0,   3,   4,    0, },
{ &asz_driver,	"tz",  29,  0,  0x0,0,	0,    '?',     0,   3,   5,    0, },
{ &asz_driver,	"tz",  30,  0,  0x0,0,	0,    '?',     0,   3,   6,    0, },
{ &asz_driver,	"tz",  31,  0,  0x0,0,	0,    '?',     0,   3,   7,    0, },

/* scsi cpu links */
{ &asz_driver,	"sc",   0,  0,  0x0,0,	0,    '?',     0,   0,   0,    0, },
{ &asz_driver,	"sc",   1,  0,  0x0,0,	0,    '?',     0,   0,   1,    0, },
{ &asz_driver,	"sc",   2,  0,  0x0,0,	0,    '?',     0,   0,   2,    0, },
{ &asz_driver,	"sc",   3,  0,  0x0,0,	0,    '?',     0,   0,   3,    0, },
{ &asz_driver,	"sc",   4,  0,  0x0,0,	0,    '?',     0,   0,   4,    0, },
{ &asz_driver,	"sc",   5,  0,  0x0,0,	0,    '?',     0,   0,   5,    0, },
{ &asz_driver,	"sc",   6,  0,  0x0,0,	0,    '?',     0,   0,   6,    0, },
{ &asz_driver,	"sc",   7,  0,  0x0,0,	0,    '?',     0,   0,   7,    0, },
{ &asz_driver,	"sc",   8,  0,  0x0,0,	0,    '?',     0,   1,   0,    0, },
{ &asz_driver,	"sc",   9,  0,  0x0,0,	0,    '?',     0,   1,   1,    0, },
{ &asz_driver,	"sc",  10,  0,  0x0,0,	0,    '?',     0,   1,   2,    0, },
{ &asz_driver,	"sc",  11,  0,  0x0,0,	0,    '?',     0,   1,   3,    0, },
{ &asz_driver,	"sc",  12,  0,  0x0,0,	0,    '?',     0,   1,   4,    0, },
{ &asz_driver,	"sc",  13,  0,  0x0,0,	0,    '?',     0,   1,   5,    0, },
{ &asz_driver,	"sc",  14,  0,  0x0,0,	0,    '?',     0,   1,   6,    0, },
{ &asz_driver,	"sc",  15,  0,  0x0,0,	0,    '?',     0,   1,   7,    0, },

/* Communications */
{ &adu_sl_driver,"asl",   0,0,	0x0,0, 0,     '?',     0,  -1,  -1,    0, },

{ &adu_se_driver,"ase",   0,0,	0x0,0, 0,     '?',     0,  -1,  -1,    0, },
/* Others */
{ &asz_driver,   "asz",0,    asz_intr,  0x0,0,	0,    '?',    0,     0, },

#endif /* ADU */

#if FLAMINGO
/* Flamingo stuff */

/* Other busses and disk controllers and.. */
{ &asc_driver,	"rz",   0,  0,  0x0,0,	0,    '?',     0,   0,   0,    0, },
{ &asc_driver,	"rz",   1,  0,  0x0,0,	0,    '?',     0,   0,   1,    0, },
{ &asc_driver,	"rz",   2,  0,  0x0,0,	0,    '?',     0,   0,   2,    0, },
{ &asc_driver,	"rz",   3,  0,  0x0,0,	0,    '?',     0,   0,   3,    0, },
{ &asc_driver,	"rz",   4,  0,  0x0,0,	0,    '?',     0,   0,   4,    0, },
{ &asc_driver,	"rz",   5,  0,  0x0,0,	0,    '?',     0,   0,   5,    0, },
{ &asc_driver,	"rz",   6,  0,  0x0,0,	0,    '?',     0,   0,   6,    0, },
{ &asc_driver,	"rz",   7,  0,  0x0,0,	0,    '?',     0,   0,   7,    0, },
{ &asc_driver,	"rz",   8,  0,  0x0,0,	0,    '?',     0,   1,   0,    0, },
{ &asc_driver,	"rz",   9,  0,  0x0,0,	0,    '?',     0,   1,   1,    0, },
{ &asc_driver,	"rz",  10,  0,  0x0,0,	0,    '?',     0,   1,   2,    0, },
{ &asc_driver,	"rz",  11,  0,  0x0,0,	0,    '?',     0,   1,   3,    0, },
{ &asc_driver,	"rz",  12,  0,  0x0,0,	0,    '?',     0,   1,   4,    0, },
{ &asc_driver,	"rz",  13,  0,  0x0,0,	0,    '?',     0,   1,   5,    0, },
{ &asc_driver,	"rz",  14,  0,  0x0,0,	0,    '?',     0,   1,   6,    0, },
{ &asc_driver,	"rz",  15,  0,  0x0,0,	0,    '?',     0,   1,   7,    0, },
{ &asc_driver,	"rz",  16,  0,  0x0,0,	0,    '?',     0,   2,   0,    0, },
{ &asc_driver,	"rz",  17,  0,  0x0,0,	0,    '?',     0,   2,   1,    0, },
{ &asc_driver,	"rz",  18,  0,  0x0,0,	0,    '?',     0,   2,   2,    0, },
{ &asc_driver,	"rz",  19,  0,  0x0,0,	0,    '?',     0,   2,   3,    0, },
{ &asc_driver,	"rz",  20,  0,  0x0,0,	0,    '?',     0,   2,   4,    0, },
{ &asc_driver,	"rz",  21,  0,  0x0,0,	0,    '?',     0,   2,   5,    0, },
{ &asc_driver,	"rz",  22,  0,  0x0,0,	0,    '?',     0,   2,   6,    0, },
{ &asc_driver,	"rz",  23,  0,  0x0,0,	0,    '?',     0,   2,   7,    0, },
{ &asc_driver,	"rz",  24,  0,  0x0,0,	0,    '?',     0,   3,   0,    0, },
{ &asc_driver,	"rz",  25,  0,  0x0,0,	0,    '?',     0,   3,   1,    0, },
{ &asc_driver,	"rz",  26,  0,  0x0,0,	0,    '?',     0,   3,   2,    0, },
{ &asc_driver,	"rz",  27,  0,  0x0,0,	0,    '?',     0,   3,   3,    0, },
{ &asc_driver,	"rz",  28,  0,  0x0,0,	0,    '?',     0,   3,   4,    0, },
{ &asc_driver,	"rz",  29,  0,  0x0,0,	0,    '?',     0,   3,   5,    0, },
{ &asc_driver,	"rz",  30,  0,  0x0,0,	0,    '?',     0,   3,   6,    0, },
{ &asc_driver,	"rz",  31,  0,  0x0,0,	0,    '?',     0,   3,   7,    0, },

{ &asc_driver,	"tz",   0,  0,  0x0,0,	0,    '?',     0,   0,   0,    0, },
{ &asc_driver,	"tz",   1,  0,  0x0,0,	0,    '?',     0,   0,   1,    0, },
{ &asc_driver,	"tz",   2,  0,  0x0,0,	0,    '?',     0,   0,   2,    0, },
{ &asc_driver,	"tz",   3,  0,  0x0,0,	0,    '?',     0,   0,   3,    0, },
{ &asc_driver,	"tz",   4,  0,  0x0,0,	0,    '?',     0,   0,   4,    0, },
{ &asc_driver,	"tz",   5,  0,  0x0,0,	0,    '?',     0,   0,   5,    0, },
{ &asc_driver,	"tz",   6,  0,  0x0,0,	0,    '?',     0,   0,   6,    0, },
{ &asc_driver,	"tz",   7,  0,  0x0,0,	0,    '?',     0,   0,   7,    0, },
{ &asc_driver,	"tz",   8,  0,  0x0,0,	0,    '?',     0,   1,   0,    0, },
{ &asc_driver,	"tz",   9,  0,  0x0,0,	0,    '?',     0,   1,   1,    0, },
{ &asc_driver,	"tz",  10,  0,  0x0,0,	0,    '?',     0,   1,   2,    0, },
{ &asc_driver,	"tz",  11,  0,  0x0,0,	0,    '?',     0,   1,   3,    0, },
{ &asc_driver,	"tz",  12,  0,  0x0,0,	0,    '?',     0,   1,   4,    0, },
{ &asc_driver,	"tz",  13,  0,  0x0,0,	0,    '?',     0,   1,   5,    0, },
{ &asc_driver,	"tz",  14,  0,  0x0,0,	0,    '?',     0,   1,   6,    0, },
{ &asc_driver,	"tz",  15,  0,  0x0,0,	0,    '?',     0,   1,   7,    0, },
{ &asc_driver,	"tz",  16,  0,  0x0,0,	0,    '?',     0,   2,   0,    0, },
{ &asc_driver,	"tz",  17,  0,  0x0,0,	0,    '?',     0,   2,   1,    0, },
{ &asc_driver,	"tz",  18,  0,  0x0,0,	0,    '?',     0,   2,   2,    0, },
{ &asc_driver,	"tz",  19,  0,  0x0,0,	0,    '?',     0,   2,   3,    0, },
{ &asc_driver,	"tz",  20,  0,  0x0,0,	0,    '?',     0,   2,   4,    0, },
{ &asc_driver,	"tz",  21,  0,  0x0,0,	0,    '?',     0,   2,   5,    0, },
{ &asc_driver,	"tz",  22,  0,  0x0,0,	0,    '?',     0,   2,   6,    0, },
{ &asc_driver,	"tz",  23,  0,  0x0,0,	0,    '?',     0,   2,   7,    0, },
{ &asc_driver,	"tz",  24,  0,  0x0,0,	0,    '?',     0,   3,   0,    0, },
{ &asc_driver,	"tz",  25,  0,  0x0,0,	0,    '?',     0,   3,   1,    0, },
{ &asc_driver,	"tz",  26,  0,  0x0,0,	0,    '?',     0,   3,   2,    0, },
{ &asc_driver,	"tz",  27,  0,  0x0,0,	0,    '?',     0,   3,   3,    0, },
{ &asc_driver,	"tz",  28,  0,  0x0,0,	0,    '?',     0,   3,   4,    0, },
{ &asc_driver,	"tz",  29,  0,  0x0,0,	0,    '?',     0,   3,   5,    0, },
{ &asc_driver,	"tz",  30,  0,  0x0,0,	0,    '?',     0,   3,   6,    0, },
{ &asc_driver,	"tz",  31,  0,  0x0,0,	0,    '?',     0,   3,   7,    0, },

/* Communications */
{ &se_driver,	"se",   0,se_intr,0x0,0, 0,   '?',     0,  -1,  -1,    0, },
{ &se_driver,	"se",   1,se_intr,0x0,0, 0,   '?',     0,  -1,  -1,    0, },

/* on 3min unit 1 should be found first */
/* on maxine we call the only one we have unit 1 */
{ &scc_driver, "scc",   1,scc_intr,0x0,0,0,    '?',	0,  -1,  -1,    0, },
{ &scc_driver, "scc",   0,scc_intr,0x0,0,0,    '?',     0,  -1,  -1,    0, },

{ &isdn_driver,"isdn",  0,isdn_intr,0x0,0,0,   '?',     0,  -1,  -1,    0, },

/* DMA engines */
{ &asic_driver,"asic",  0,asic_intr,0x0,0,0,   '?',     0,  -1,  -1,    0, },

/* Graphic drivers */
#if	NGX>0
{ &gq_driver,	"gq",   0,gq_intr,0x0,0, 0,    '?',     0,  -1,  -1,    0, },
{ &ga_driver,	"ga",   0,ga_intr,0x0,0, 0,    '?',     0,  -1,  -1,    0, },
#endif	NGX>0
{ &sfb_driver,	"sfb",  0,sfb_intr,0x0,0,0,    '?',     0,  -1,  -1,    0, },
{ &cfb_driver,	"cfb",  0,cfb_intr,0x0,0,0,    '?',     0,  -1,  -1,    0, },
{ &fb_driver,	"fb",   0,fb_intr,0x0,0,0,     '?',     0,  -1,  -1,    0, },

/* Others */
{ &mc_driver,	"mc",   0,mc_intr,0x0,0,0,     '?',     0,  -1,  -1,    0, },
#endif /* FLAMINGO */
	0
};
