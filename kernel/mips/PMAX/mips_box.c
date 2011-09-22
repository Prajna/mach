/* 
 * Mach Operating System
 * Copyright (c) 1993-1989 Carnegie Mellon University
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
 * $Log:	mips_box.c,v $
 * Revision 2.20  93/05/15  19:12:02  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.19  93/05/10  21:20:46  rvb
 * 	No more sys/types.h
 * 	[93/05/06  09:40:51  af]
 * 
 * Revision 2.18  93/02/05  08:04:03  danner
 * 	Clock got more generic.
 * 	[93/02/04  02:03:31  af]
 * 
 * Revision 2.17  93/01/21  12:23:32  danner
 * 	ecc --> atm
 * 	[93/01/19  16:35:47  bershad]
 * 
 * Revision 2.16  93/01/14  17:50:31  danner
 * 	Added sfb.
 * 	[92/11/29            af]
 * 
 * Revision 2.15  92/05/22  15:48:59  jfriedl
 * 	Typos on slaves 27.  Now Mark is happy.
 * 	[92/05/13            af]
 * 
 * Revision 2.14  92/05/05  10:46:47  danner
 * 	Added scsi-cpu links to other processors.
 * 	[92/04/14  12:10:07  af]
 * 
 * 	Added maxine's mappable timer.
 * 	[92/03/11  02:36:18  af]
 * 
 * Revision 2.13  92/04/03  12:09:42  rpd
 * 	To support the FORE ATM TurboChannel board.
 * 	[92/03/23            rvb]
 * 
 * Revision 2.12  92/04/01  15:14:57  rpd
 * 	Added maxine's mappable timer.
 * 	[92/03/11  02:36:18  af]
 * 
 * Revision 2.11  92/03/02  18:34:44  rpd
 * 	Added MAXine's drivers.
 * 	[92/03/02  02:20:00  af]
 * 
 * Revision 2.10  91/08/24  12:21:39  af
 * 	Spl defines, SCC and ASIC drivers, a bunch more disks and tapes
 * 	so that we can play any number of additional SCSI interface boards.
 * 	New fb screen driver.
 * 	[91/08/02  03:36:23  af]
 * 
 * Revision 2.9  91/06/19  11:56:09  rvb
 * 	The busses.h and other header files have moved to the "chips"
 * 	directory.
 * 	[91/06/07            rvb]
 * 
 * Revision 2.8  91/05/14  17:24:39  mrt
 * 	Correcting copyright
 * 
 * Revision 2.7  91/02/14  14:34:52  mrt
 * 	In clock interrupt routine, drop priority as now required.
 * 	[91/02/12  12:59:26  af]
 * 
 * 	Defined pmax_memcheck() and related default implementation.
 * 	[91/01/03  02:10:32  af]
 * 
 * Revision 2.6  91/02/05  17:42:47  mrt
 * 	Added author notices
 * 	[91/02/04  11:15:22  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:14:01  mrt]
 * 
 * Revision 2.5  91/01/09  19:49:54  rpd
 * 	Defined pmax_memcheck() and related default implementation.
 * 	[91/01/03  02:10:32  af]
 * 
 * Revision 2.4  90/12/05  23:32:43  af
 * 	Moved a number of pmax-specific handlers in kn01.c
 * 	[90/12/03  23:28:42  af]
 * 
 * Revision 2.2.1.2  90/11/01  03:47:10  af
 * 	Mostly renames for new, copyright free drivers.
 * 	Added "pm" driver.
 * 
 * Revision 2.2.1.1  90/10/03  11:54:25  af
 * 	Reflected changes in new autoconf TC code.  Also, moved wildcarded
 * 	"se" driver entry after non-wildcarded ones.
 * 
 * Revision 2.2  90/08/07  22:25:14  rpd
 * 	Created.
 * 	[90/08/07  15:49:22  af]
 * 
 * Revision 2.1.1.1  90/05/30  15:49:07  af
 * 	Created.
 * 	[90/05/28            af]
 * 
 */
/*
 *	File: mips_box.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	5/90
 *
 *	Box-specific routines and data required by the MI code
 */

#include <gx.h>
#include <atm.h>

#include <kern/time_out.h>
#include <mips/mips_cpu.h>
#include <machine/machspl.h>		/* spl definitions */
#include <mips/PMAX/tc.h>
#include <mips/PMAX/mips_box.h>
#include <chips/busses.h>

#include <mips/PMAX/kmin.h>

extern	kn01_scsi_intr(), kn01_se_intr(),
	kn01_dz_intr(), kn01_err_intr();

/* Common clock interrupt routine for pmax/3max */
kn01_mc_intr(st,spllevel)
{
	mc_intr(spllevel);
	splx(spllevel);
	clock_interrupt(tick, st&SR_KUo, (st&SR_INT_MASK)==INT_LEV0);
}

int (*pmax_intr2)() = kn01_scsi_intr;		/* SCSI */
int (*pmax_intr3)() = kn01_se_intr;		/* Lance */
int (*pmax_intr4)() = kn01_dz_intr;		/* DZ */
int (*pmax_intr5)() = kn01_mc_intr;		/* RTC */
int (*pmax_intr6)() = kn01_err_intr;		/* Write timeouts */

static
boolean_t null_memcheck(addr,pc) { return FALSE; }
boolean_t (*pmax_memcheck)() = null_memcheck;

extern struct bus_driver
	asc_driver, sii_driver, fd_driver,
	se_driver, dz_driver, scc_driver, isdn_driver, dtop_driver,
	asic_driver,
	pm_driver, ga_driver, gq_driver, fb_driver, cfb_driver, xcfb_driver, sfb_driver,
	frc_driver;

#if	NATM > 0
extern struct bus_driver atm_driver;
#endif	NATM > 0

extern int
	asc_intr(), sii_intr(), fd_intr(),
	se_intr(), dz_intr(), scc_intr(), isdn_intr(), dtop_intr(),
	asic_intr(),
	pm_intr(), ga_intr(), gq_intr(), fb_intr(), cfb_intr(), xcfb_intr(), sfb_intr(),
	frc_intr();

#if	NATM > 0
extern int atm_intr();
#endif	NATM > 0


struct bus_ctlr bus_master_init[] = {
/*driver,	name,	unit, intr, address+am, phys, adpt, alive, flags, */
{ &asc_driver,	"asc",	0,    asc_intr,  0x0,0,	0,    '?',    0,     0, },
{ &asc_driver,	"asc",	1,    asc_intr,  0x0,0,	0,      0,    0,     0, },
{ &asc_driver,	"asc",	2,    asc_intr,  0x0,0,	0,      1,    0,     0, },
{ &asc_driver,	"asc",	3,    asc_intr,  0x0,0,	0,      2,    0,     0, },
{ &sii_driver,	"sii",	0,    sii_intr,  0x0,0,	0,    '?',    0,     0, },
{ &fd_driver,	"fdc",	0,    fd_intr,   0x0,0,	0,    '?',    0,     0, },
	0
};

struct bus_device bus_device_init[] = {
/* driver,      name, unit,intr,addr+am,phys, adaptor,alive,ctlr,slave,flags,*/

/* Disk and tapes and.. SCSIs */
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

/* scsi cpu links */
{ &asc_driver,	"sc",   0,  0,  0x0,0,	0,    '?',     0,   0,   0,    0, },
{ &asc_driver,	"sc",   1,  0,  0x0,0,	0,    '?',     0,   0,   1,    0, },
{ &asc_driver,	"sc",   2,  0,  0x0,0,	0,    '?',     0,   0,   2,    0, },
{ &asc_driver,	"sc",   3,  0,  0x0,0,	0,    '?',     0,   0,   3,    0, },
{ &asc_driver,	"sc",   4,  0,  0x0,0,	0,    '?',     0,   0,   4,    0, },
{ &asc_driver,	"sc",   5,  0,  0x0,0,	0,    '?',     0,   0,   5,    0, },
{ &asc_driver,	"sc",   6,  0,  0x0,0,	0,    '?',     0,   0,   6,    0, },
{ &asc_driver,	"sc",   7,  0,  0x0,0,	0,    '?',     0,   0,   7,    0, },
{ &asc_driver,	"sc",   8,  0,  0x0,0,	0,    '?',     0,   1,   0,    0, },
{ &asc_driver,	"sc",   9,  0,  0x0,0,	0,    '?',     0,   1,   1,    0, },
{ &asc_driver,	"sc",  10,  0,  0x0,0,	0,    '?',     0,   1,   2,    0, },
{ &asc_driver,	"sc",  11,  0,  0x0,0,	0,    '?',     0,   1,   3,    0, },
{ &asc_driver,	"sc",  12,  0,  0x0,0,	0,    '?',     0,   1,   4,    0, },
{ &asc_driver,	"sc",  13,  0,  0x0,0,	0,    '?',     0,   1,   5,    0, },
{ &asc_driver,	"sc",  14,  0,  0x0,0,	0,    '?',     0,   1,   6,    0, },
{ &asc_driver,	"sc",  15,  0,  0x0,0,	0,    '?',     0,   1,   7,    0, },

{ &sii_driver,	"rz",   0,  0,  0x0,0,	0,    '?',     0,   0,   0,    0, },
{ &sii_driver,	"rz",   1,  0,  0x0,0,	0,    '?',     0,   0,   1,    0, },
{ &sii_driver,	"rz",   2,  0,  0x0,0,	0,    '?',     0,   0,   2,    0, },
{ &sii_driver,	"rz",   3,  0,  0x0,0,	0,    '?',     0,   0,   3,    0, },
{ &sii_driver,	"rz",   4,  0,  0x0,0,	0,    '?',     0,   0,   4,    0, },
{ &sii_driver,	"rz",   5,  0,  0x0,0,	0,    '?',     0,   0,   5,    0, },
{ &sii_driver,	"rz",   6,  0,  0x0,0,	0,    '?',     0,   0,   6,    0, },
{ &sii_driver,	"rz",   7,  0,  0x0,0,	0,    '?',     0,   0,   7,    0, },

{ &sii_driver,	"tz",   0,  0,  0x0,0,	0,    '?',     0,   0,   0,    0, },
{ &sii_driver,	"tz",   1,  0,  0x0,0,	0,    '?',     0,   0,   1,    0, },
{ &sii_driver,	"tz",   2,  0,  0x0,0,	0,    '?',     0,   0,   2,    0, },
{ &sii_driver,	"tz",   3,  0,  0x0,0,	0,    '?',     0,   0,   3,    0, },
{ &sii_driver,	"tz",   4,  0,  0x0,0,	0,    '?',     0,   0,   4,    0, },
{ &sii_driver,	"tz",   5,  0,  0x0,0,	0,    '?',     0,   0,   5,    0, },
{ &sii_driver,	"tz",   6,  0,  0x0,0,	0,    '?',     0,   0,   6,    0, },
{ &sii_driver,	"tz",   7,  0,  0x0,0,	0,    '?',     0,   0,   7,    0, },

{ &fd_driver,   "fd",   0,  0,  0x0,0,  0,    '?',     0,   0,   0,    0, },
{ &fd_driver,   "fd",   1,  0,  0x0,0,  0,    '?',     0,   0,   1,    0, },
{ &fd_driver,   "fd",   2,  0,  0x0,0,  0,    '?',     0,   0,   2,    0, },
{ &fd_driver,   "fd",   3,  0,  0x0,0,  0,    '?',     0,   0,   3,    0, },

/* Communications */
{ &se_driver,	"se",   0,se_intr,0x0,0, 0,   '?',     0,  -1,  -1,    0, },
{ &se_driver,	"se",   1,se_intr,0x0,0, 0,   '?',     0,  -1,  -1,    0, },

{ &dz_driver,	"dz",   0,dz_intr,0x0,0, 0,    '?',     0,  -1,  -1,    0, },

/* on 3min unit 1 should be found first */
/* on maxine we call the only one we have unit 1 */
{ &scc_driver, "scc",   1,scc_intr,0x0,0,0,    '?',	0,  -1,  -1,    0, },
{ &scc_driver, "scc",   0,scc_intr,0x0,0,0,    '?',     0,  -1,  -1,    0, },

{ &dtop_driver,"dtop",  0,dtop_intr,0x0,0,0,   '?',     0,  -1,  -1,    0, },

{ &isdn_driver,"isdn",  0,isdn_intr,0x0,0,0,   '?',     0,  -1,  -1,    0, },

/* DMA engines */
{ &asic_driver,"asic",  0,asic_intr,0x0,0,0,   '?',     0,  -1,  -1,    0, },


/* Graphic drivers */
#if	NGX>0
{ &gq_driver,	"gq",   0,gq_intr,0x0,0, 0,    '?',     0,  -1,  -1,    0, },
{ &ga_driver,	"ga",   0,ga_intr,0x0,0, 0,    '?',     0,  -1,  -1,    0, },
#endif	NGX>0
{ &sfb_driver,	"sfb",  0,sfb_intr,0x0,0,0,    '?',     0,  -1,  -1,    0, },
{ &xcfb_driver, "xcfb", 0,xcfb_intr,0x0,0,0,   '?',     0,  -1,  -1,    0, },
{ &cfb_driver,	"cfb",  0,cfb_intr,0x0,0,0,    '?',     0,  -1,  -1,    0, },
{ &fb_driver,	"fb",   0,fb_intr,0x0,0,0,     '?',     0,  -1,  -1,    0, },
{ &pm_driver,   "pm",   0,pm_intr,0x0,0, 0,    '?',	0,  -1,  -1,	0, },
/* Others */
#if	NATM > 0
{ &atm_driver, "atm",	0,atm_intr,0x0,0,0,	'?',	0,  -1,  -1,    0, },
#endif	NATM > 0
	/* free running counter */
{ &frc_driver,   "frc", 0,frc_intr,0x0,0, 0,	       '?',	0,  -1,  -1,	0, },
	0
};
