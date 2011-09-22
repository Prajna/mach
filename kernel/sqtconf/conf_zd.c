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
 * $Log:	conf_zd.c,v $
 * Revision 2.3  91/07/31  18:05:48  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:03:34  dbg
 * 	Adapted for pure Mach kernel.
 * 	[90/10/05            dbg]
 * 
 */

#ifndef	lint
static	char	rcsid[] = "$Header: conf_zd.c,v 2.3 91/07/31 18:05:48 dbg Exp $";
#endif

/*
 * configuration of disks on the ZDC
 *
 * WARNING: If the tables are partially filled in unpredictable results
 *	    *WILL* occur.
 */

/*
 * Revision 1.1  89/07/05  13:17:43  kak
 * Initial revision
 * 
 * Revision 1.8  89/01/12  14:22:51  djg
 * zdccmdtime is now of type unsigned.
 * 
 * Revision 1.7  88/03/23  18:27:49  neal
 * Added m2382k (swallow 5) support to ZDC.
 * 
 */
#include <device/buf.h>

#include <sqt/mutex.h>
#include <sqtzdc/zdc.h>

/*
 *	Part A.	Partition tables.
 *
 * On all drives, the first cylinder is reserved for disk description data
 * and the last two cylinders are reserved for diagnostics. No partition
 * which will contain a filesystem should include any of these cylinders.
 *
 * N.B.: The stand-alone driver knows these offsets.
 *
 * The zdparts table is indexed by drive type.
 * Note that this list is order dependent. New entries *must*
 * correspond with the drive type.
 *
 * NOTE: The newfs utility ASSUMES that the 'c' partition starts at the
 *	 beginning of the disk when writing the bootstrap program.
 *	 The bootstrap program is written when a root filesystem is created.
 *	 The newfs utility ASSUMES that the 'a' partition is the root
 *	 filesystem. However, by writing the bootstrap to partition 'c' the
 *	 'a' partition may be moved to the middle of the disk to reduce
 *	 seek latency.
 *	 If the 'c' partition is changed so that it does not include the
 *	 start of the disk, then be sure to use the "-n" option to newfs
 *	 and use /stand/installboot to write the bootstrap program (at least
 *	 1 partition must start at the beginning of the disk).
 */

struct zdsize m2333k[NUMPARTS] = {	/* Fujitsu M2333K (swallow) */
	 25*66*10,	335,	/* A=cyl 335 thru 359 */
	102*66*10,	360,	/* B=cyl 360 thru 461 */
	820*66*10,	  1,	/* C=cyl   1 thru 820 */
	410*66*10,	  1,	/* D=cyl   1 thru 410 */
	410*66*10,	411,	/* E=cyl 411 thru 820 */
	359*66*10,	  1,	/* F=cyl   1 thru 359 */
	359*66*10,	462,	/* G=cyl 462 thru 820 */
	334*66*10,	  1,	/* H=cyl   1 thru 334 */
};

struct zdsize m2351a[NUMPARTS] = {	/* Fujitsu M2351A (Eagle) */
	 18*46*20,	366,	/* A=cyl 366 thru 383 */
	 73*46*20,	384,	/* B=cyl 384 thru 456 */
	839*46*20,	  1,	/* C=cyl   1 thru 839 */
	419*46*20,	  1,	/* D=cyl   1 thru 419 */
	419*46*20,	420,	/* E=cyl 420 thru 838 */
	383*46*20,	  1,	/* F=cyl   1 thru 383 */
	383*46*20,	457,	/* G=cyl 457 thru 839 */
	365*46*20,	  1,	/* H=cyl   1 thru 365 */
};

struct zdsize m2344k[NUMPARTS] = {	/* Fujitsu M2344K (swallow 4) */
	 10*66*27,	282,	/* A=cyl 282 thru 291 */
	 38*66*27,	292,	/* B=cyl 292 thru 329 */
	621*66*27,	  1,	/* C=cyl   1 thru 621 */
	310*66*27,	  1,	/* D=cyl   1 thru 310 */
	311*66*27,	311,	/* E=cyl 311 thru 621 */
	291*66*27,	  1,	/* F=cyl   1 thru 291 */
	292*66*27,	330,	/* G=cyl 330 thru 621 */
	281*66*27,	  1,	/* H=cyl   1 thru 281 */
};

struct zdsize m2382k[NUMPARTS] = {	/* Fujitsu M2382K (swallow 5) */
	 8*81*27,	348,	/* A=cyl 348 thru 355 */
	 32*81*27,	356,	/* B=cyl 356 thru 387 */
	742*81*27,	  1,	/* C=cyl   1 thru 742 */
	371*81*27,	  1,	/* D=cyl   1 thru 371 */
	371*81*27,	372,	/* E=cyl 372 thru 742 */
	355*81*27,	  1,	/* F=cyl   1 thru 355 */
	355*81*27,	388,	/* G=cyl 388 thru 742 */
	347*81*27,	  1,	/* H=cyl   1 thru 347 */
};

struct zdsize cdc9715_340[NUMPARTS] = {	/* CDC 9715-340 (FSD) */
	 21*34*24,	292,	/* A=cyl 292 thru 312 */
	 83*34*24,	313,	/* B=cyl 313 thru 395 */
	708*34*24,	  1,	/* C=cyl   1 thru 708 */
	354*34*24,	  1,	/* D=cyl   1 thru 354 */
	354*34*24,	355,	/* E=cyl 355 thru 708 */
	312*34*24,	  1,	/* F=cyl   1 thru 312 */
	313*34*24,	396,	/* G=cyl 396 thru 708 */
	291*34*24,	  1,	/* H=cyl   1 thru 291 */
};

struct zdsize cdc9771_800[NUMPARTS] = {	/* CDC 9771-800 (XMD) */
	 12*85*16,	474,	/* A=cyl 474 thru 485 */
	 50*85*16,	486,	/* B=cyl 486 thru 535 */
	1021*85*16,	  1,	/* C=cyl   1 thru 1021 */
	511*85*16,	  1,	/* D=cyl   1 thru 511 */
	510*85*16,	512,	/* E=cyl 512 thru 1021 */
	485*85*16,	  1,	/* F=cyl   1 thru 485 */
	486*85*16,	536,	/* G=cyl 536 thru 1021 */
	473*85*16,	  1,	/* H=cyl   1 thru 473 */
};

/*
 * A null zdsize array for Reserved drive types
 * Done this way since driver already checks for a partition
 * length of zero.  Otherwise additonal code to check
 * for NULL pointer in zdparts[] would be required.
 */
struct zdsize zd_nulltype[NUMPARTS] = {	/* Used for reserved types */
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
};

struct zdsize *zdparts[] = {
	m2333k,			/* 0 - Fujitsu M2333K (swallow) */
	m2351a,			/* 1 - Fujitsu M2351A (Eagle) */
	m2344k,			/* 2 - Fujitsu M2344K (swallow 4) */
	m2382k,			/* 3 - Fujitsu M2382K (swallow 5) */
	zd_nulltype,		/* 4 - Reserved */
	zd_nulltype,		/* 5 - Reserved */
	zd_nulltype,		/* 6 - Reserved */
	zd_nulltype,		/* 7 - Reserved */
	zd_nulltype,		/* 8 - Reserved */
	zd_nulltype,		/* 9 - Reserved */
	zd_nulltype,		/* 10 - Reserved */
	zd_nulltype,		/* 11 - Reserved */
	zd_nulltype,		/* 12 - Reserved */
	zd_nulltype,		/* 13 - Reserved */
	zd_nulltype,		/* 14 - Reserved */
	zd_nulltype,		/* 15 - Reserved */
	cdc9715_340,		/* 16 - CDC 9715-340 MB (FSD) */
	cdc9771_800,		/* 17 - CDC 9771-800 MB (XMD) */
};
int	zdntypes	= sizeof (zdparts) / sizeof(struct zdsize *);

/*
 *	Part B. Global Information.
 */

/*
 * If the number of zdc_iovpercb is 0 then the driver will automatically
 * allocate enough iovecs to handle max_RAW_IO (see physio()).
 * If the number of zdc_iovpercb is non-zero, then the value specified
 * is allocated if it is less than max_RAW_IO. If the specified value is
 * greater than max_RAW_IO the number is reduced to handle max_RAW_IO.
 */
int	zdc_iovpercb	= 0;			/* no of iovecs per cb */

int	zdc_err_bin	= 7;			/* Error interrupt bin */
int	zdc_cb_bin	= 5;			/* CB interrupt bin */
gate_t	zdcgate		= 62;			/* gate for zdc locks/semas */
/*
 * polled command timeout, should be approx. 30 secs.
 */
unsigned int	zdccmdtime	= 30000000;
/*
 * controller ready timeout, should be >= 60 secs.
 */
unsigned int	zdcinitime	= 2400000;
short	zdcretry	= 10;			/* retry count */
int	zdc_AB_throttle	= 2;			/* Channel A&B DMA throttle */
int	zdc_C_throttle	= 2;			/* Channel C DMA throttle */
u_char	zdctrl = ZDC_DUMPONPANIC;		/* ZDC_INIT control bits */
