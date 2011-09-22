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
 * $Log:	sqt_io_conf.c,v $
 * Revision 2.3  91/07/31  18:05:55  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:04:10  dbg
 * 	Adapted for pure Mach kernel.
 * 	[90/10/03            dbg]
 * 
 */

/*
 * Configuration file for Sequent SYMMETRY.
 * Sequent generates this file with bin/config.
 */

#include	<sqt/ioconf.h>

/*
 * Pseudo-device configuration
 */

struct	pseudo_dev pseudo_dev[] = {
	{ 0 },
};

/*
 * Interrupt table
 */

int	bin_intr[8] = {
		0,				/* bin 0, always zero */
		0,				/* bin 1 */
		0,				/* bin 2 */
		0,				/* bin 3 */
		26,				/* bin 4 */
		17,				/* bin 5 */
		8,				/* bin 6 */
		0,				/* bin 7 */
};

/*
 * b8k_cntlrs array collects all controller entries
 */
extern int  conf_sec(),	probe_sec_devices(),	sec_map();
#if 0
extern int  conf_zdc(),	probe_zdc_devices(),	zdc_map();
extern int  conf_mbad(),probe_mbad_devices(),	mbad_map();
#endif

struct	cntlrs b8k_cntlrs[] = {
/*	conf		probe_devs		map	*/
#if 0
{	conf_mbad,	probe_mbad_devices,	mbad_map	}, 
#endif
{	conf_sec,	probe_sec_devices,	sec_map	}, 
#if 0
{	conf_zdc,	probe_zdc_devices,	zdc_map	}, 
#endif
{	0,	},
};

u_long	MBAd_IOwindow =		3*256*1024;	/* top 1/4 Meg */

/*
 * mbad device configuration.
 */


#include	<sqtmbad/ioconf.h>

#if 0
extern	struct	mbad_driver	st_driver;
extern	struct	mbad_driver	zt_driver;
extern	struct	mbad_driver	lp_driver;

struct	mbad_dev mbad_st[] = {
/*	index	csr	flags	maps	bin	intr */
{	-1,	512,	0,	0,	4,	3,	},	/* st0 */
{	-1,	528,	0,	0,	4,	4,	},	/* st1 */
{	-1,	544,	0,	0,	4,	5,	},	/* st2 */
{	-1,	560,	0,	0,	4,	6,	},	/* st3 */
{	-1,	576,	0,	0,	4,	3,	},	/* st4 */
{	-1,	592,	0,	0,	4,	4,	},	/* st5 */
};

struct	mbad_dev mbad_zt[] = {
/*	index	csr	flags	maps	bin	intr */
{	-1,	768,	0,	34,	5,	7,	},	/* zt0 */
};

struct	mbad_dev mbad_lp[] = {
/*	index	csr	flags	maps	bin	intr */
{	-1,	1024,	0,	8,	5,	6,	},	/* lp0 */
};

#endif 0
/*
 * mbad_conf array collects all mbad devices
 */

struct	mbad_conf mbad_conf[] = {
/*	Driver		#Entries	Devices		*/
#if 0
{	&st_driver,	6,		mbad_st,	},	/* st */
{	&zt_driver,	1,		mbad_zt,	},	/* zt */
{	&lp_driver,	1,		mbad_lp,	},	/* lp */
#endif 0
	{ 0 },
};


/*
 * sec device configuration.
 */


#include	<sqtsec/ioconf.h>

extern	struct	sec_driver	co_driver;
extern	struct	sec_driver	se_driver;
extern	struct	sec_driver	sd_driver;
#if 0
extern	struct	sec_driver	ts_driver;
extern	struct	sec_driver	sm_driver;
#endif 0


struct	sec_dev sec_co[] = {
/*	flags	bin	req	doneq	index	target	unit */
{	0,	4,	4,	4,	0,	-1,	0,	},	/* co0 */
{	0,	4,	4,	4,	0,	-1,	1,	},	/* co1 */
{	0,	4,	4,	4,	0,	-1,	2,	},	/* co2 */
{	0,	4,	4,	4,	0,	-1,	3,	},	/* co3 */
{	0,	4,	4,	4,	1,	-1,	0,	},	/* co4 */
{	0,	4,	4,	4,	1,	-1,	1,	},	/* co5 */
{	0,	4,	4,	4,	1,	-1,	2,	},	/* co6 */
{	0,	4,	4,	4,	1,	-1,	3,	},	/* co7 */
{	0,	4,	4,	4,	2,	-1,	0,	},	/* co8 */
{	0,	4,	4,	4,	2,	-1,	1,	},	/* co9 */
{	0,	4,	4,	4,	2,	-1,	2,	},	/* co10 */
{	0,	4,	4,	4,	2,	-1,	3,	},	/* co11 */
{	0,	4,	4,	4,	3,	-1,	0,	},	/* co12 */
{	0,	4,	4,	4,	3,	-1,	1,	},	/* co13 */
{	0,	4,	4,	4,	3,	-1,	2,	},	/* co14 */
{	0,	4,	4,	4,	3,	-1,	3,	},	/* co15 */
};

struct	sec_dev sec_se[] = {
/*	flags	bin	req	doneq	index	target	unit */
{	0,	6,	25,	25,	0,	-1,	0,	},	/* se0 */
{	0,	6,	10,	10,	0,	-1,	1,	},	/* se1 */
{	0,	6,	25,	25,	1,	-1,	0,	},	/* se2 */
{	0,	6,	10,	10,	1,	-1,	1,	},	/* se3 */
};

struct	sec_dev sec_sd[] = {
/*	flags	bin	req	doneq	index	target	unit */
{	0,	5,	4,	4,	-1,	-1,	-1,	},	/* sd0 */
{	0,	5,	4,	4,	-1,	-1,	-1,	},	/* sd1 */
{	0,	5,	4,	4,	-1,	-1,	-1,	},	/* sd2 */
};

struct	sec_dev sec_ts[] = {
/*	flags	bin	req	doneq	index	target	unit */
{	0,	5,	4,	4,	-1,	4,	-1,	},	/* ts0 */
};

struct	sec_dev sec_sm[] = {
/*	flags	bin	req	doneq	index	target	unit */
{	0,	4,	3,	3,	0,	-1,	0,	},	/* sm0 */
{	0,	4,	3,	3,	1,	-1,	0,	},	/* sm1 */
};

/*
 * sec_conf array collects all sec devices
 */

struct	sec_conf sec_conf[] = {
/*	Driver		#Entries	Devices		*/
{	&co_driver,	16,		sec_co,	},	/* co */
{	&se_driver,	4,		sec_se,	},	/* se */
{	&sd_driver,	3,		sec_sd,	},	/* sd */
#if 0
{	&ts_driver,	1,		sec_ts,	},	/* ts */
{	&sm_driver,	2,		sec_sm,	},	/* sm */
#endif 0
	{ 0 },
};


/*
 * zdc device configuration.
 */


#include	<sqtzdc/ioconf.h>

#if 0
extern	struct	zdc_driver	zd_driver;

struct	zdc_dev zdc_zd[] = {
/*	index	drive	drive_type */
{	0,	-1,	-1,	},	/* zd0 */
{	0,	-1,	-1,	},	/* zd1 */
{	0,	-1,	-1,	},	/* zd2 */
{	0,	-1,	-1,	},	/* zd3 */
{	0,	-1,	-1,	},	/* zd4 */
{	0,	-1,	-1,	},	/* zd5 */
{	0,	-1,	-1,	},	/* zd6 */
{	0,	-1,	-1,	},	/* zd7 */
{	1,	-1,	-1,	},	/* zd8 */
{	1,	-1,	-1,	},	/* zd9 */
{	1,	-1,	-1,	},	/* zd10 */
{	1,	-1,	-1,	},	/* zd11 */
{	1,	-1,	-1,	},	/* zd12 */
{	1,	-1,	-1,	},	/* zd13 */
{	1,	-1,	-1,	},	/* zd14 */
{	1,	-1,	-1,	},	/* zd15 */
{	2,	-1,	-1,	},	/* zd16 */
{	2,	-1,	-1,	},	/* zd17 */
{	2,	-1,	-1,	},	/* zd18 */
{	2,	-1,	-1,	},	/* zd19 */
{	2,	-1,	-1,	},	/* zd20 */
{	2,	-1,	-1,	},	/* zd21 */
{	2,	-1,	-1,	},	/* zd22 */
{	2,	-1,	-1,	},	/* zd23 */
{	3,	-1,	-1,	},	/* zd24 */
{	3,	-1,	-1,	},	/* zd25 */
{	3,	-1,	-1,	},	/* zd26 */
{	3,	-1,	-1,	},	/* zd27 */
{	3,	-1,	-1,	},	/* zd28 */
{	3,	-1,	-1,	},	/* zd29 */
{	3,	-1,	-1,	},	/* zd30 */
{	3,	-1,	-1,	},	/* zd31 */
};

#endif 0
/*
 * zdc_conf array collects all zdc devices
 */

struct	zdc_conf zdc_conf[] = {
/*	Driver		#Entries	Devices		*/
#if 0
{	&zd_driver,	32,		zdc_zd,	},	/* zd */
#endif 0
	{ 0 },
};
