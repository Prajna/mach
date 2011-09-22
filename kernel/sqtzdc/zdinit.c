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
 * $Log:	zdinit.c,v $
 * Revision 2.4  93/03/11  14:06:00  danner
 * 	u_long -> u_int
 * 	[93/03/10            danner]
 * 
 * Revision 2.3  91/07/31  18:09:11  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:08:34  dbg
 * 	Added volatile declarations.
 * 	Allowed interrupts on any CPU.
 * 	[91/03/25            dbg]
 * 
 * 	Adapted for pure Mach kernel.
 * 	[90/10/04            dbg]
 * 
 */

#ifndef	lint
static	char	rcsid[] = "$Header: zdinit.c,v 2.4 93/03/11 14:06:00 danner Exp $";
#endif

/*
 * zdinit.c
 *
 * Initialization portion ZDC SMD Disk Driver.
 *
 * Auto-configure ZDC controllers and drives. Intialize ZDC controllers.
 * Uses polled interface to communicate with ZDC.
 */

/*
 * Revision 1.2  89/08/16  15:20:57  kak
 * balance -> sqt
 * 
 * Revision 1.1  89/07/05  13:21:06  kak
 * Initial revision
 * 
 * Revision 1.21  89/01/12  14:22:00  djg
 * zdcmdtime is now of type unsigned.
 * 
 * Revision 1.20  88/12/21  10:38:09  djg
 * fixed lint warnings
 * 
 * Revision 1.19  88/11/10  08:24:26  djg
 * bak242 now uses l.cpu_speed
 * 
 */

#ifdef	MACH_KERNEL
#include <sys/types.h>

#include <device/buf.h>
#include <device/param.h>

#include <sqt/macros.h>
#include <sqt/vm_defs.h>
#include <sqt/cfg.h>
#include <sqt/slic.h>
#include <sqt/slicreg.h>
#include <sqt/clkarb.h>
#include <sqt/mutex.h>
#include <sqt/intctl.h>

#include <sqtzdc/zdc.h>
#include <sqtzdc/ioconf.h>

#define max_RAW_IO	(128*1024)	/* used for config */

#else	MACH_KERNEL
#include "sys/param.h"
#include "sys/buf.h"
#include "sys/systm.h"
#include "sys/time.h"
#include "sys/dk.h"
#include "sys/vmmeter.h"

#include "sqt/cfg.h"
#include "sqt/slic.h"
#include "sqt/slicreg.h"
#include "sqt/clkarb.h"

#include "sqt/mutex.h"
#include "sqt/intctl.h"
#include "sqt/vmparam.h"
#include "sqt/pte.h"
#include "sqt/plocal.h"

#include "sqtzdc/zdc.h"
#include "sqtzdc/ioconf.h"

#ifdef	MACH
#include "../machine/cpu.h"
#include "../sqt/engine.h"

#define max_RAW_IO	(128*1024)	/* used for config */
#endif
#endif	MACH_KERNEL

/*
 * Kernel Hack to turn off 16 byte writes 
 */
#define	ENABLE_SIXTEEN 0		/* do not allow 16byte writes */
int	enable_sixteen = ENABLE_SIXTEEN;

#define	PLURAL(x) ((x) == 1 ? "" : "s")
#define	CHAN_A	0			/* Channel A */
#define	CHAN_B	1			/* Channel B */
#define	ERRLIGHTON { if (fp_lights) FP_IO_ERROR; }

extern	int	zdntypes;		/* no of known drive types */
extern	int	zdc_iovpercb;		/* no of iovecs per cb */
extern	int	zdc_err_bin;		/* bin for error interrupts */
extern	int	zdc_cb_bin;		/* bin for cb interrupts */
extern	int	zdccmdtime;		/* polled command timeout value */
extern	int	zdcinitime;		/* init ctrlr timeout value */
extern	int	zdc_C_throttle;		/* Throttle count for DMA channel C */
extern	short	zdcretry;		/* retry count on errors */
extern	gate_t	zdcgate;		/* gate no. for zdc locks/semas */
extern	u_char	zdctrl;			/* additional icb_ctrl bits */
extern	caddr_t	zd_compcodes[];		/* completion codes in english */
extern	int	zdncompcodes;		/* no. of entries in zd_compcodes */

struct	zdc_ctlr *zdctrlr;		/* zdctrlr array - calloc'd */
struct	zd_unit	 *zdunit;		/* zdunit array - calloc'd */
u_char	base_cb_intr;			/* base interrupt for zdc driver */
u_char	base_err_intr;			/* base controller interrupt */
simple_lock_data_t
	zdcprlock;			/* lock for error message sync */

static	struct	cb	*basecb;	/* base cb */
static	struct	zdcdd	*chancfg;	/* ZDC_GET_CHANCFG */

struct	zdc_driver zd_driver = { "zd" };	/* STUB */

extern char *	calloc();

/*
 * conf_zdc()
 *
 * Find all ZDCs and fill-in slic addresses in controller structure.
 */
conf_zdc()
{
	register struct ctlr_toc *toc =
	    PHYSTOKV(&va_CD_LOC->c_toc[SLB_ZDCBOARD], struct ctlr_toc *);
	register struct	ctlr_desc *zdcfg =
	    PHYSTOKV(&va_CD_LOC->c_ctlrs[toc->ct_start], struct ctlr_desc *);
	register int numzdc = toc->ct_count;
	register struct zdc_ctlr *ctlrp;
	register int alivezdc;
	register int diagflag;

	alivezdc = 0;
	if (numzdc == 0) {
		printf("No ZDCs.\n");
		return;
	}
	printf("%d ZDC%s; slic", numzdc, PLURAL(numzdc));
	callocrnd(sizeof(struct zdc_ctlr));
	zdctrlr = (struct zdc_ctlr *)calloc(numzdc * sizeof(struct zdc_ctlr));
	for (ctlrp = zdctrlr; ctlrp < &zdctrlr[numzdc]; ctlrp++, zdcfg++) {
		ctlrp->zdc_slicaddr = zdcfg->cd_slic;
		diagflag = zdcfg->cd_diag_flag;
		if (diagflag & (CFG_FAIL|CFG_DECONF)) {
			ctlrp->zdc_state = ZDC_DEAD;
			if (diagflag & CFG_FAIL) {
				ERRLIGHTON;
			}
		} else {
			ctlrp->zdc_state = ZDC_ALIVE;
			alivezdc++;
		}
		printf(" %d", ctlrp->zdc_slicaddr);
		ctlrp->zdc_diagflag = diagflag;
	}
	printf(".\n");

	if (alivezdc < numzdc) {
		printf("Not using ZDC: slic");
		for (ctlrp = zdctrlr; ctlrp < &zdctrlr[numzdc]; ctlrp++) {
			if (ctlrp->zdc_state == ZDC_DEAD)
				printf(" %d", ctlrp->zdc_slicaddr);
		}
		printf(".\n");
	}

	/*
	 * Allocate ZDC error interrupt vectors.
	 * Must be allocated contiguously across ZDCs.
	 *
	 * Allocate interrupt vectors for CBs.
	 */
	ivecres(zdc_err_bin, numzdc);
	ivecres(zdc_cb_bin, numzdc * NCBPERZDC);
}

/*
 * Find and bind drives to units.
 * Initialize each controller.
 */
probe_zdc_devices()
{
	register int	i;
	register struct	cb	 *cbp;
	register struct	zdc_ctlr *ctlrp;
	register struct	zdc_dev  *zdv;
	register struct	zd_unit	 *up;
	int	val;
	int	ctrlr, drive;
	struct	zdcdd	*dd;
	int	found[ZDC_MAXCTRLR];	/* bit map - found drives */
	struct	cb lcb;
	int	numzdc =
	    PHYSTOKV(&va_CD_LOC->c_toc[SLB_ZDCBOARD], struct ctlr_toc *)
		->ct_count;
#ifndef	MACH
	extern	long	max_RAW_IO;	/* Max size RAW IO request (bytes) */
#endif
	extern	int	zdintr();	/* interrupt handler */
	extern	int	zdc_error();	/* Error interrupt handler */

	for (i = 0; i < ZDC_MAXCTRLR; i++)
		found[i] = 0;		/* no drives yet */
	/*
	 * Check to see if controller FW is initialized.
	 * Wait until ZDC is initialized. If error, mark ZDC as ZDC_DEAD.
	 */
	for (ctlrp = zdctrlr; ctlrp < &zdctrlr[numzdc]; ctlrp++) {
		if (ctlrp->zdc_state != ZDC_ALIVE)
			continue;
		i = calc_delay(zdcinitime);
		for (;;) {
			val = rdslave(ctlrp->zdc_slicaddr, SL_Z_STATUS);
			if ((val & SLB_ZPARERR) != SLB_ZPARERR) {
				if ((val & ZDC_READY) == ZDC_READY)
					break;
				if ((val & ZDC_ERRMASK) == ZDC_SINIT)  {
					/* Still initializing */
					if (--i)
						continue;
				}
			}
			/*
			 * Error found
			 */
			printf("zdc%d: controller failure - SL_Z_STATUS == 0x%x.\n",
				ctlrp - zdctrlr, val);
			printf("zdc%d at slic %d Deconfigured.\n",
				ctlrp - zdctrlr, ctlrp->zdc_slicaddr);
			ctlrp->zdc_state = ZDC_DEAD;
			ERRLIGHTON;
			break;
		}
	}

	/*
	 * Set up interrupt vectors
	 */
	base_err_intr = ivecpeek(zdc_err_bin);
	for (i = 0; i < numzdc; i++)
		ivecinit(zdc_err_bin, ivecall((u_char)zdc_err_bin), zdc_error);

	base_cb_intr = ivecpeek(zdc_cb_bin);
	for (i = 0; i < NCBPERZDC * numzdc; i++)
		ivecinit(zdc_cb_bin, ivecall((u_char)zdc_cb_bin), zdintr);

	/*
	 * Allocate CBs
	 */
	callocrnd(sizeof(struct cb));
	basecb = (struct cb *)calloc(NCBPERZDC * sizeof(struct cb) * numzdc);
	for (cbp = basecb; cbp < &basecb[NCBPERZDC * numzdc]; cbp++) {
		cbp->cb_unit = -1;
	}

	/*
	 * If zdc_iovpercb not set or too big for max_RAW_IO then
	 * set for max_RAW_IO.
	 */
	if (zdc_iovpercb <= 0 || ((zdc_iovpercb * I386_PGBYTES) > max_RAW_IO))
		zdc_iovpercb = (max_RAW_IO + (I386_PGBYTES-1)) / I386_PGBYTES;

	/*
	 * Adjust zdc_iovpercb size to safe size.
	 */
	zdc_iovpercb = roundup(zdc_iovpercb, IOVALIGN / sizeof(u_int *));

	/*
	 * init_lock to prevent error message garble.
	 */
	init_lock(&zdcprlock, zdcgate);

	/*
	 * Tell each ZDC where its CB array is located.
	 * Done via Bin1 - Bin4 SLIC interrupts, where Bin 1 is the
	 * least significant byte of the CB address and Bin 4 is
	 * the most significant byte.
	 */
#define	NBPW	4
#define	NBBY	8

	cbp = basecb;
	ctlrp = zdctrlr;
	for (; ctlrp < &zdctrlr[numzdc]; ctlrp++, cbp += NCBPERZDC) {
		ctlrp->zdc_cbp = cbp;
		if (ctlrp->zdc_state == ZDC_ALIVE) {
			unsigned int cbp_pa;
			cbp_pa = KVTOPHYS(cbp, unsigned int);
			for (i = 0; i < NBPW; i++)
				mIntr(ctlrp->zdc_slicaddr, (u_char)i + 1,
					(u_char)(cbp_pa >> (i * NBBY)));
			init_lock(&ctlrp->zdc_ctlrlock, zdcgate);
		}
	}

#undef	NBBY
#undef	NBPW

	/*
	 * Probe devices on ZDC via ZDC_PROBE and ZDC_GET_CHANCFG.
	 * Store results in controller structure.
	 */
	callocrnd(sizeof(struct zdcdd));
	chancfg = (struct zdcdd *)calloc(sizeof(struct zdcdd));
	cbp = &lcb;
	for (ctlrp = zdctrlr; ctlrp < &zdctrlr[numzdc]; ctlrp++) {
		if (ctlrp->zdc_state != ZDC_ALIVE)
			continue;
		cbp->cb_cmd = ZDC_PROBEDRIVE;
		if (pollzdcmd(ctlrp, cbp, CHAN_A) < 0) {
			printf("zdc%d: Cannot probe zdc.\n", ctlrp - zdctrlr);
			printf("zdc%d at slic %d Deconfigured.\n",
					ctlrp - zdctrlr, ctlrp->zdc_slicaddr);
			ctlrp->zdc_state = ZDC_DEAD;
			ERRLIGHTON;
			continue;
		}
		for (i = 0; i < ZDC_MAXDRIVES; i++)
			ctlrp->zdc_drivecfg[i] =
				  ((struct probe_cb *)cbp)->pcb_drivecfg[i];

		if (get_chancfg(ctlrp, CHAN_A) < 0)
			continue;
		if (get_chancfg(ctlrp, CHAN_B) < 0)
			continue;
	}

	/*
	 * Now set up units resolving any wildcarding.
	 * Report wildcard binding.
	 */
	callocrnd(sizeof(struct zd_unit));
	zdunit = (struct zd_unit *)calloc(zdc_conf->zc_nent * sizeof(struct zd_unit));
	up = zdunit;
	for (zdv=zdc_conf->zc_dev; zdv < &zdc_zd[zdc_conf->zc_nent]; zdv++, up++) {
		up->zu_state = ZU_NOTFOUND;
		/*
		 * First bind to controller.
		 */
		for (ctrlr = 0; ctrlr < numzdc; ctrlr++) {
			if (ctrlr != zdv->zdv_idx && zdv->zdv_idx != ANY)
				continue;
			ctlrp = &zdctrlr[ctrlr];
			if (ctlrp->zdc_state != ZDC_ALIVE && zdv->zdv_idx == ANY)
				continue;
			/*
			 * Controller matched.
			 */
			up->zu_ctrlr = ctrlr;
			if (ctlrp->zdc_state != ZDC_ALIVE) {
				printf("zd%d on dead controller zdc%d.\n",
							up - zdunit, ctrlr);
				up->zu_state = ZU_BAD;
				disk_offline();
				break;	/* next unit */
			}
			/*
			 * Now bind drive.
			 */
			for (drive = 0; drive < ZDC_MAXDRIVES; drive++) {
				if (drive != zdv->zdv_drive &&
							zdv->zdv_drive != ANY)
					continue;
				if (found[ctrlr] & (1 << drive)) {
					/*
					 * Already bound to another unit
					 */
					continue;
				}
				if (ctlrp->zdc_drivecfg[drive] == ZD_NOTFOUND) {
					/*
					 * If drive not found and wildcarding
					 * drive, then continue search on this
					 * ZDC. If drive not found and bound
					 * drive but wildcarding ZDC, continue
					 * search on another controller.
					 */
					if (zdv->zdv_drive == ANY)
						continue;
					if (zdv->zdv_idx == ANY)
						break;
					/* No - use this drive */
				}
				dd = (drive & 1) ? &ctlrp->zdc_chanB
						 : &ctlrp->zdc_chanA;
				if (zdv->zdv_drive_type != ANY
				&&  (dd->zdd_sectors == 0 ||
				     zdv->zdv_drive_type != dd->zdd_drive_type)) {
					/*
					 * If type doesn't match configured and
					 * drive wildcarded, then continue
					 * search on this ZDC.
					 * If type doesn't match configured but
					 * bound drive and ZDC wildcarded,
					 * continue search on another ZDC.
					 */
					if (zdv->zdv_drive == ANY)
						continue;
					if (zdv->zdv_idx == ANY)
						break;
					/* Else, bound to this drive */
				}

				/*
				 * Drive Matched.
				 */
				found[ctrlr] |= 1 << drive;
				printf("zd%d bound to zdc%d drive %d.\n",
						up - zdunit, ctrlr, drive);
				up->zu_drive = drive;
				if (dd->zdd_sectors == 0
				||  (zdv->zdv_drive_type != ANY &&
				     zdv->zdv_drive_type != dd->zdd_drive_type)
				||  (dd->zdd_drive_type >= zdntypes)) {
					/*
					 * If no channel cfg or drive doesn't
					 * match configuration, set state to
					 * ZU_NO_RW. Since zdsize() will be
					 * called before open, the size routine
					 * will not allow swap I/O to drives
					 * that mismatch the configuration.
					 */
					up->zu_drive_type = zdv->zdv_drive_type;
					up->zu_state = ZU_NO_RW;
				} else {
					/*
					 *  OK match set drive type.
					 */
					up->zu_drive_type = dd->zdd_drive_type;
					up->zu_state = ZU_GOOD;
				}
				up->zu_cfg = ctlrp->zdc_drivecfg[drive];
				up->zu_zdbad = NULL;
#ifndef	MACH
				bufinit(&up->zu_ioctl, zdcgate);
#endif	MACH
				init_lock(&up->zu_lock, zdcgate);
				init_sema(&up->zu_ocsema, 1, 0, zdcgate);
#ifndef	MACH
				if (dk_nxdrive < dk_ndrives) {
					/*
					 * Set up dk style statistics.
					 * Note: that dk_bps is NOT kept.
					 * It is set non-zero so that iostat
					 * will still report other useful stats
					 * for the drive.
					 */
					up->zu_dkstats = &dk[dk_nxdrive++];
					up->zu_dkstats->dk_bps = 1; /* FAKE */
					bcopy("zdXX", up->zu_dkstats->dk_name, 5);
					i = up - zdunit;
					if (i > 9) {
					    up->zu_dkstats->dk_name[2] = i/10 + '0';
					    i -= (i/10) * 10;
					    up->zu_dkstats->dk_name[3] = i + '0';
					} else {
					    up->zu_dkstats->dk_name[2] = i + '0';
					    up->zu_dkstats->dk_name[3] = '\0';
					}
				} else {
					up->zu_dkstats = (struct dk *)NULL;
				}
#endif	MACH
				/*
				 * Set up software cb fields for this unit.
				 */
				cbp = &ctlrp->zdc_cbp[drive * NCBPERDRIVE];
				up->zu_cbptr = cbp;
				cbp->cb_unit = up - zdunit;
				callocrnd(IOVALIGN);
				cbp->cb_iovstart =
					(u_int *)calloc(zdc_iovpercb
							* sizeof(u_int *));
				++cbp;
				cbp->cb_unit = up - zdunit;
				cbp->cb_iovstart =
					(u_int *)calloc(zdc_iovpercb
							* sizeof(u_int *));

				if (up->zu_cfg == ZD_NOTFOUND ||
				    (up->zu_cfg & ZD_ONLINE) != ZD_ONLINE) {
					/*
					 * Even though drive not present, the
					 * unit is bound to the drive. It
					 * could be that the drive is not
					 * spun up and will be later...
					 */
					printf("zd%d: drive not present.\n",
						up - zdunit);
					disk_offline();
				}
				goto nextunit;
			} /* drives */
		} /* ctrlrs */

		/*
		 * If unit not found fill out unit for post-mortem/debug.
		 */
		if (up->zu_state == ZU_NOTFOUND) {
			up->zu_ctrlr = zdv->zdv_idx;
			up->zu_drive = zdv->zdv_drive;
			up->zu_drive_type = zdv->zdv_drive_type;
		}
nextunit:;	/* the ";" necessary to workaround compiler bug */
	} /* units */

	/*
	 * Send ZDC_INIT command to all alive ZDCs.
	 */
	init_zdcs(numzdc);
}

/*
 * a null function.  
 * zdc configuration doesn't need it, but config(1) requires it.
 */
zdc_map() {}

/*
 * get_chancfg
 *	get channel configuration.
 * Return:
 *	0  - If command completes successfully. The controller structure
 *	     is filled with the channel configuration data.
 *	-1 - If error processing ZDC_GET_CHANCFG.
 *
 */
int
get_chancfg(ctlrp, channel)
	register struct zdc_ctlr *ctlrp;
	int	channel;
{
	register struct	cb *cbp;
	struct	cb lcb;

	cbp = &lcb;
	bzero((caddr_t)cbp, sizeof(struct cb));
	cbp->cb_cmd = ZDC_GET_CHANCFG;		/* command */
	cbp->cb_addr = KVTOPHYS(chancfg, u_int);	/* where to put it */
	cbp->cb_count = sizeof(struct zdcdd);
	if (pollzdcmd(ctlrp, cbp, channel) < 0) {
		printf("zdc%d: Cannot get channel cfg.\n", ctlrp - zdctrlr);
		printf("zdc%d at slic %d Deconfigured.\n",
				ctlrp - zdctrlr, ctlrp->zdc_slicaddr);
		ctlrp->zdc_state = ZDC_DEAD;
		ERRLIGHTON;
		return (-1);
	}
	if (cbp->cb_compcode == ZDC_NOCFG)
		return (0);

	if (channel == CHAN_A)
		ctlrp->zdc_chanA = *chancfg;
	else
		ctlrp->zdc_chanB = *chancfg;
	return (0);
}

/*
 * bad_zdc
 *	- mark all units on dead zdc as unusable.
 * Called only during configuration/initialization.
 */
static
bad_zdc(ctlr)
	register int ctlr;
{
	register struct zd_unit *up;
	register int any;

	printf("zdc%d at slic %d Deconfigured.\n", ctlr,
					zdctrlr[ctlr].zdc_slicaddr);
	any = 0;
	for (up = zdunit; up < &zdunit[zdc_conf->zc_nent]; up++) {
		if (up->zu_ctrlr == ctlr && up->zu_state == ZU_GOOD)  {
			++any;
			up->zu_state = ZU_BAD;		/* on dead ctrlr */
			disk_offline();
			printf("zd%d, ", up - zdunit);
		}
	}
	if (any)
		printf("%s unusable.\n", (any == 1) ? "is" : "are");
}

/*
 * init_zdc
 *	init all zdc controllers via ZDC_INIT command
 * Initialize interrupts. On successive commands, the ZDC will
 * generate interrupts to signal command completion.
 */
init_zdcs(numzdc)
	int	numzdc;
{
	register struct init_cb	 *icbp;
	register struct zdc_ctlr *ctlrp;
	u_char	cbvec, errvec;
	struct	init_cb lcb;

	icbp = &lcb;
	bzero((caddr_t)icbp, sizeof(struct init_cb));
	if (zdc_C_throttle > SLB_TVAL)
		zdc_C_throttle = SLB_TVAL;	/* Set to MAX */

	ctlrp = zdctrlr;
	errvec = base_err_intr;
	cbvec = base_cb_intr;
	for (; ctlrp < &zdctrlr[numzdc]; ctlrp++, errvec++, cbvec += NCBPERZDC){
		extern int mono_P_slic;

		if (ctlrp->zdc_state != ZDC_ALIVE)
			continue;

		icbp->icb_cmd = ZDC_INIT;
		icbp->icb_pagesize = I386_PGBYTES;
		icbp->icb_dest = SL_GROUP | TMPOS_GROUP;
		icbp->icb_bin = zdc_cb_bin;
		icbp->icb_vecbase = cbvec;
		icbp->icb_errdest = SL_GROUP | TMPOS_GROUP;
		icbp->icb_errbin = zdc_err_bin;
		icbp->icb_errvector = errvec;
		icbp->icb_ctrl = ZDC_ENABLE_INTR;
		icbp->icb_ctrl |= zdctrl;
		/*
		 * Enable 16 byte transfers if the bus mode is one of
		 * the BUS_EXTENDED bus modes.  Do this after above
		 * "or" of configurable so it can't accidently turn on
		 * 16 byte mode when illegal
		 */
		if (va_CD_LOC->c_sys_mode.sm_bus_mode == CD_BUS_EXTENDED_NARR ||
		    va_CD_LOC->c_sys_mode.sm_bus_mode == CD_BUS_EXTENDED_WIDE &&
		    enable_sixteen)
			icbp->icb_ctrl |= ZDC_SIXTEEN;
		else
			icbp->icb_ctrl &= ~ZDC_SIXTEEN;

		if (zdctrl & ZDC_DUMPONPANIC) {
			/*
			 * Give FW a place to dump its LRAM.
			 */
			callocrnd(I386_PGBYTES);    /* align at 1K boundary */
			ctlrp->zdc_dumpaddr = calloc(ZDC_LRAMSZ);
			icbp->icb_dumpaddr =
			    KVTOPHYS(ctlrp->zdc_dumpaddr, caddr_t);
		}
		if (pollzdcmd(ctlrp, (struct cb *)icbp, CHAN_A) < 0) {
			printf("zdc%d: Cannot init zdc.\n", ctlrp - zdctrlr);
			ctlrp->zdc_state = ZDC_DEAD;
			ERRLIGHTON;
			/* Mark all units on this controller as BAD */
			bad_zdc(ctlrp - zdctrlr);
		}
		/*
		 * Set throttle value for DMA channel C.
		 */
		wrslave(ctlrp->zdc_slicaddr, SL_G_CHAN2,
				(u_char)(SLB_TH_ENB | zdc_C_throttle));
	}
}

/*
 * pollzdcmd
 *
 * Issue a command to the ZDC. 
 * Poll for completion.
 * Retry, when necessary.
 *
 * Return: transfer count
 *	 0 - Success
 * 	-1 - error.
 * NOTE: If controller error ctlrp->zdc_state is set to ZDC_DEAD.
 *	 The caller may choose to set ctlrp->zdc_state to ZDC_DEAD
 *	 upon error whether or not the error was a controller error.
 */
int
pollzdcmd(ctlrp, acbp, drive)
	register struct zdc_ctlr *ctlrp;
	struct cb	*acbp;
	register int	drive;
{
	register struct cb *cbp;
	register int	i;
	register int	val;
	struct cb lcb;		/* ZDC_RESET during retries */

	cbp = &ctlrp->zdc_cbp[drive * NCBPERDRIVE];
	acbp->cb_errcnt = 0;

retry:
	/*
	 * Fill in appropriate CB
	 */
	bcopy((caddr_t)acbp, (caddr_t)cbp, FWCBSIZE);

	/*
	 * Signal ZDC to do command. Poll until completion or timeout.
	 */
	cbp->cb_compcode = ZDC_BUSY;
	mIntr(ctlrp->zdc_slicaddr, CBBIN, (u_char)(drive * NCBPERDRIVE));

	i = calc_delay(zdccmdtime);
	while (*(volatile u_char *)&cbp->cb_compcode == ZDC_BUSY) {
		if (--i == 0) {
			/*
			 * Timed out - check for controller error
			 */
			printf("zdc%d: Cmd %x timeout.\n", ctlrp - zdctrlr,
								cbp->cb_cmd);
			val = rdslave(ctlrp->zdc_slicaddr, SL_Z_STATUS);
			if ((val & SLB_ZPARERR) ||
					((val & ZDC_READY) != ZDC_READY)) {
				/*
				 * Found controller error.
				 */
				printf("zdc%d: Ctrlr error status 0x%x.\n",
					ctlrp - zdctrlr, val);
				/*
				 * controller bad!
				 */
				ctlrp->zdc_state = ZDC_DEAD;
			}
			return (-1);
		}
	}

	switch (cbp->cb_compcode) {

	case ZDC_SOFTECC:
	case ZDC_CORRECC:
		/*
		 * Corrected or Soft ECC error.
		 */
		printf("zd%d: %s at (%d, %d, %d).\n",
			cbp->cb_unit,
			zd_compcodes[cbp->cb_compcode],
			cbp->cb_cyl, cbp->cb_head, cbp->cb_sect);
		zddumpstatus(cbp);
		/* Fall into... */
	case ZDC_DONE:
		/*
		 * Normal completion
		 */
		*acbp = *cbp;
		return (0);

	case ZDC_DRVPROT:
	case ZDC_ECC:
	case ZDC_BADDRV:
	case ZDC_DDC_STAT:
		goto hard;

	case ZDC_DMA_TO:
	case ZDC_REVECT:
	case ZDC_ILLCMD:
	case ZDC_ILLMOD:
	case ZDC_ILLALIGN:
	case ZDC_ILLCNT:
	case ZDC_ILLIOV:
	case ZDC_ILLVECIO:
	case ZDC_ILLPGSZ:
	case ZDC_ILLDUMPADR:
	case ZDC_ILLCHS:
	case ZDC_CBREUSE:
		/*
		 * Hard error. This set indicates controller error.
		 */
		ctlrp->zdc_state = ZDC_DEAD;
		goto hard;

	case ZDC_CH_RESET:
		/*
		 * Drive has failed in the middle of the command!
		 */
		printf("zd%d: Drive failed during cmd 0x%x.\n",
					cbp->cb_unit, cbp->cb_cmd);
		goto hard;

	case ZDC_ACCERR:
		/*
		 * Most likely controller problem.
		 * Clear access error and notify firmware.
		 */
		val = rdslave(ctlrp->zdc_slicaddr,
				(u_char)((drive & 1) ? SL_G_ACCERR1
						     : SL_G_ACCERR0));
		printf("zd%d: Access error 0x%x on transfer at 0x%x.\n",
					drive, val, cbp->cb_addr);
		wrslave(ctlrp->zdc_slicaddr,
			(u_char)((drive & 1) ? SL_G_ACCERR1 : SL_G_ACCERR0),
			(u_char)0xbb);
		ctlrp->zdc_state = ZDC_DEAD;
		goto hard;

	case ZDC_NOCFG:
		*acbp = *cbp;
		if (cbp->cb_cmd == ZDC_GET_CHANCFG)
			return (0);
		return (-1);

	case ZDC_HDR_ECC:
	case ZDC_SNF:
	case ZDC_SO:
	case ZDC_NDS:
	case ZDC_DRVFLT:
	case ZDC_SEEKERR:
	case ZDC_SEEK_TO:
	case ZDC_CH_TO:
	case ZDC_FDL:
		/*
		 * Retry these errors.
		 */
		break;

	default:
		/*
		 * Unknown completion code - controller bad?
		 */
		printf("zdc%d: Bad compcode 0x%x.\n", ctlrp - zdctrlr,
			cbp->cb_compcode);
		cbp->cb_compcode = zdncompcodes - 1;	/* Get nice message */
		ctlrp->zdc_state = ZDC_DEAD;
		goto hard;
	}

	if (cbp->cb_cmd == ZDC_RESET)
		return (-1);

	printf("zd%d: Error (%s); cmd = 0x%x at (%d, %d, %d).\n",
		cbp->cb_unit, zd_compcodes[cbp->cb_compcode], cbp->cb_cmd,
		cbp->cb_cyl, cbp->cb_head, cbp->cb_sect);
	zddumpstatus(cbp);
	/*
	 * Retry. Reset the drive each time.
	 */
	if (acbp->cb_errcnt++ < zdcretry) {
		lcb.cb_cmd = ZDC_RESET;
		if (pollzdcmd(ctlrp, &lcb, drive) < 0) {
			printf("zd%d: RESET failed.\n", cbp->cb_unit);
			goto hard;
		}
		goto retry;
	} else {
		/*
		 * save cb contents to return.
		 */
		*acbp = *cbp;
		lcb.cb_cmd = ZDC_RESET;
		if (pollzdcmd(ctlrp, &lcb, drive) < 0)
			printf("zd%d: RESET failed.\n", cbp->cb_unit);
		bcopy((caddr_t)acbp, (caddr_t)cbp, FWCBSIZE);
	}

hard:
	/*
	 * Update argument cb and return
	 */
	printf("zd%d: Hard error (%s); cmd = 0x%x at (%d, %d, %d).\n",
		cbp->cb_unit, zd_compcodes[cbp->cb_compcode], cbp->cb_cmd,
		cbp->cb_cyl, cbp->cb_head, cbp->cb_sect);
	zddumpstatus(cbp);
	*acbp = *cbp;
	return (-1);
}
