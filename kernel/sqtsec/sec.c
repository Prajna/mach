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
 * $Log:	sec.c,v $
 * Revision 2.4  93/03/10  11:30:51  danner
 * 	u_long -> u_int
 * 	[93/03/10            danner]
 * 
 * Revision 2.3  91/07/31  18:07:30  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:07:03  dbg
 * 	Added volatile declarations.  Distribute interrupts to any CPU.
 * 	[91/03/25            dbg]
 * 
 * 	Adapted for pure kernel.
 * 	[90/09/24            dbg]
 * 
 */

#ifndef	lint
static	char	rcsid[] = "$Header: sec.c,v 2.4 93/03/10 11:30:51 danner Exp $";
#endif

/*
 * sec.c
 *	Various procedures dealing with SEC's.
 */

/*
 * Revision 1.2  89/08/03  12:30:17  kak
 * balance -> sqt
 * 
 * Revision 1.1  89/07/05  13:20:14  kak
 * Initial revision
 * 
 * Revision 2.22  88/12/21  10:37:01  djg
 * fixed lint warnings
 * 
 * Revision 2.21  88/11/10  08:24:12  djg
 * bak242 now uses l.cpu_speed
 * 
 * Revision 2.20  88/06/02  09:43:19  dilip
 * buf_iat now flushes the tlb in both i386 and 032 cases.
 * 
 */

#ifdef	MACH_KERNEL
#include <sys/types.h>

#include <device/param.h>
#include <device/io_req.h>
#include <device/buf.h>

#include <sqt/macros.h>

#include <sqt/vm_defs.h>
#include <sqt/sqtparam.h>
#include <sqt/clock.h>
#include <sqt/slic.h>
#include <sqt/slicreg.h>
#include <sqt/cfg.h>

#include <sqt/ioconf.h>
#include <sqt/hwparam.h>
#include <sqt/mutex.h>
#include <sqt/intctl.h>
#include <sqt/trap.h>

#include <sqtsec/sec.h>

#else	/* MACH_KERNEL */
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/map.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/vm.h"
#include "sys/clist.h"
#include "sys/buf.h"

#include "sqt/clock.h"
#include "sqt/slic.h"
#include "sqt/slicreg.h"
#include "sqt/cfg.h"

#include "sqt/ioconf.h"
#include "sqt/pte.h"
#include "sqt/hwparam.h"
#include "sqt/mutex.h"
#include "sqt/intctl.h"
#include "sqt/trap.h"
#include "sqt/mftpr.h"
#include "sqt/plocal.h"
#include "sqt/pmap.h"

#include "sqtsec/sec.h"

#endif	/* MACH_KERNEL */

#define	SEC_ERROR_BIN 7

#ifdef	DEBUG
int	sec_debug = 2;		/* 0=off, 1=little, 2=all */	/* XXX */
#endif	DEBUG

struct sec_desc	*SEC_desc;	/* array alloc'd and filled by conf_scsi() */
int	 	NSEC;		/* # SEC's to map at boot */
u_int	 	SECvec;		/* bit-vector of existing SEC's */
u_char		SEC_errbase;	/* Base vector number for SEC reported errors */
u_char	SEC_accerr[MAXNUMSEC];	/* access errors reported by SEC */
unsigned	sec0eaddr;	/* lower 24 bits of ether address */
				/* later used to contain # users */
boolean_t	conscsi_yet;	/* Console SCSI init'd yet */
boolean_t	probescsi_yet;	/* probed the SCSI bus yet */
boolean_t	CCS_present;	/* are the drives embedded SCSI */

/*
 * Distinguished location for embedded SCSI drive.  If there is a CCS disk
 * at this target, we will assume that all disks on the SCSI bus are
 * embedded, and we will scan wildcarded SEC targets from low to high.
 * If the device at this target is not a CCS disk (or does not exist),
 * assume we have target-adaptor-based disks and scan SCSI target
 * wildcards from high to low.
 */

int	CCS_magic_target = 0;

/*
 * appropriate bit is set if a target is found on a SCSI bus that
 * can only have one Logical Unit Number on it, or we are convinced that
 * there is no target there.  This bitmap allows us to scan the SCSI bus
 * more rapidly by eliminating possibilities.
 */

u_char SECnoprobe[MAXNUMSEC];

#define	SEC_EXISTS(SEC_idx)	(SECvec & (1 << (SEC_idx)))
#define	WILDCARD(sd)		(((sd)->sd_sec_idx == -1) || ((sd)->sd_target == -1) || ((sd)->sd_unit == -1))

#define	PLURAL(x)		 ((x)==1?"":"s")

int		SEC_error();	/* SCED error catcher */
u_char		*SECgarbuf;	/* SCED garbage buffer pointer */

struct	sec_cib	*SEC_alloc_channels();
struct	sec_desc *SEC_probe();

extern char *	calloc();	/* bootstrap memory allocator */
extern int	mono_P_slic;	/* interrupt address for mono-processor
				   drivers */
/*
 * conf_sec()
 *	Configure SCSI boards.
 */

conf_sec()
{
	register struct ctlr_toc *toc;
	register struct	ctlr_desc *cd;
	register struct	sec_desc *sec;
	register int	i;
	register int	j;

	toc = PHYSTOKV(&va_CD_LOC->c_toc[SLB_SCSIBOARD], struct ctlr_toc *);

	SEC_desc = (struct sec_desc *)calloc((int)toc->ct_count * sizeof(*sec));
	printf("%d SCSI/Ether controller%s; slic",
					toc->ct_count, PLURAL(toc->ct_count));
	NSEC = 0;
	cd = PHYSTOKV(&va_CD_LOC->c_ctlrs[toc->ct_start], struct ctlr_desc *);
	for (i = 0; i < toc->ct_count; i++, cd++) {
		sec = &SEC_desc[i];
		sec->sec_diag_flags = cd->cd_diag_flag;
		sec->sec_slicaddr = cd->cd_slic;
		sec->sec_target_no = cd->cd_sc_host_num;
		sec->sec_is_cons = cd->cd_sc_cons;
		sec->sec_powerup =
			PHYSTOKV(cd->cd_sc_init_queue, struct sec_powerup *);
		sec->sec_version = cd->cd_sc_version;
		for (j = 0; j < 6; ++j)
			sec->sec_ether_addr[j] = cd->cd_sc_enet_addr[j];
		/*
		 * Save lower 24 bits of ether address to be passed to init,
		 * if not already initialized (eg, on a B21k -- see
		 * conf_clkarb()).
		 *
		 * Note: sec0eaddr is later used to contain number of users.
		 * BTW: typecast to int gets around a compiler bug.
		 */
		if (i == 0 && sec0eaddr == (unsigned)-1) {
			u_char *s = &cd->cd_sc_enet_addr[0];
			sec0eaddr = (int)s[5] | (int)(s[4]<<8) | (int)(s[3]<<16);
		}

		if ((cd->cd_diag_flag & (CFG_FAIL|CFG_DECONF)) == 0) {
			++NSEC;
			SECvec |= 1 << i;
		}
		printf(" %d", sec->sec_slicaddr);
#ifdef DEBUG
		if (cd->cd_diag_flag)
			printf(" flags 0x%b", cd->cd_diag_flag,
			"\20\2FAIL\3DECONF\4BDUSE\5SCSIFAIL\6ETHERFAIL\7SLICFAIL\10TODFAIL\11PORT0FAIL\12PORT1FAIL\13SEFAIL\14DIFFFAIL");
#endif DEBUG
	}
	printf(".\n");

	/*
	 * Allocate SEC error interrupt vectors.
	 * Must be allocated contiguously over all SECs.
	 */

	ivecres(SEC_ERROR_BIN, toc->ct_count);

	if (NSEC != toc->ct_count) {
		printf("Not using SCSI/Ether Controllers: slic");
		for (i = 0; i < toc->ct_count; ++i) {
			if (!SEC_EXISTS(i))
				printf(" %d", SEC_desc[i].sec_slicaddr);
		}
		printf(".\n");
	}
}

/*
 * probe_sec_devices()
 *	Probe for devices attached to SCSI controllers.
 *
 * Probing gets tricky when ambiguous specifications of devices
 * are allowed.  Too much wildcarding can require that we
 * keep track of where devices were found, to avoid looking
 * for other devices there.  The array 'found' is a 2-D table of
 * bits; a one in the (sec, chan) position of the table means we found
 * a device on that SEC at that channel number (SEC device number
 * between 0 and 95).
 *
 * For the moment, let's just "know" how to index this,
 * as I don't know where the macros that manipulate such
 * a table should live.
 */

probe_sec_devices()
{
	register struct ctlr_toc	*toc;
	register struct	sec_conf	*sec;
	register struct	sec_driver	*secd;
	register struct	sec_dev		*sd;
	register struct sec_desc	*sec_desc;
	register int i;
	struct	sec_cib	*cibs[MAXNUMSEC];
	u_char	vec;
	u_int	found[MAXNUMSEC*3];
	int secno, chan;
	extern	int	mono_P_slic;
	extern	struct	sec_cib *cbdcib, *todcib, *wdtcib;

	toc = PHYSTOKV(&va_CD_LOC->c_toc[SLB_SCSIBOARD], struct ctlr_toc *);

	for (secno = 0; secno < MAXNUMSEC; ++secno)
		for (chan = 0; chan < 3; ++chan)
			found[secno*3 + chan] = 0;

	/*
	 * Run through the sec_conf array and do the probes.
	 * We call the boot procedure even if no existing HW is found.
	 *
	 * for each configured SEC driver, for each device ...
	 */

	for (sec = sec_conf; sec->sec_driver; ++sec) {
		secd = sec->sec_driver;
		for (sd = sec->sec_dev, i = 0;  i < sec->sec_nent;  i++, sd++) {

			/*
			 * If it has an interrupt procedure, allocate a vector
			 * in the appropriate bin.  Must allocate even if
			 * probe ==> not here, to preserve order of devices.
			 */
			if (secd->sed_cflags & SED_HASINTR)
				vec = ivecall(sd->sd_bin);

			/*
			 * If it's there, fill out configure fields in `sd'.
			 */
			if ((sec_desc = SEC_probe(secd, sd, found)) == 0)
				continue;
			found[(sec_desc-SEC_desc)*3 + (sd->sd_chan >> 5)] |= 1 << (sd->sd_chan & 0x1F);
			sd->sd_desc = sec_desc;
			sd->sd_alive = 1;
			if (secd->sed_cflags & SED_MONOP) {
				mono_P_slic = va_slic->sl_procid;
			}
			if (secd->sed_cflags & SED_HASINTR) {
				sd->sd_vector = vec;
				ivecinit(sd->sd_bin, vec, secd->sed_intr);
				if (secd->sed_cflags & SED_MONOP)
					sd->sd_destslic = mono_P_slic;
				else
					sd->sd_destslic = SL_GROUP|TMPOS_GROUP;
			}

			/*
			 * Allocate the request and done queues.
			 * The program pointers themselves are filled
			 * in later by the driver (at boot time).
			 * The pointer to the cib queue is filled in
			 * by SEC_init_channels, when the contiguous
			 * cib channel array is allocated.
			 */
			SEC_allocate_progqs(sd);
			assert(sd->sd_requestq != 0);
			assert(sd->sd_doneq != 0);

			/*
			 * Tell the world it's alive.
			 */
			printf("%s%d found at SEC%d", secd->sed_name, i, sec_desc-SEC_desc);
			if (secd->sed_cflags & SED_IS_SCSI) {
				printf(" target adapter %d unit %d", sd->sd_target, sd->sd_unit);
			}
			if (secd->sed_cflags & SED_HASINTR) {
				printf(" bin %d vec %d", sd->sd_bin, sd->sd_vector);
			}
			printf(" input q %d output q %d",
				sd->sd_req_size, sd->sd_doneq_size);
#ifdef DEBUG
			if (sec_debug)
				printf(" dest slic 0x%x",
					sd->sd_destslic);
#endif
			printf(".\n");
		}
	}

	/*
	 * Allocate a garbage buffer for raw io.
	 * Only one is needed for *all* SCED controllers.
	 * Must not cross a 64kbyte physical boundry because of
	 * SEC DMA hardware limitation.
	 */

	callocrnd(DEV_BSIZE);
	SECgarbuf = (u_char *)calloc(DEV_BSIZE*sizeof(u_char));

	/*
	 * Allocate the SEC controller queues/cibs.
	 */

	for (i = 0; i < toc->ct_count; ++i) {
		if (!SEC_EXISTS(i))
			continue;
		cibs[i] = SEC_alloc_channels(i, &SEC_desc[i], sec_conf);
		/*
		 * If console SCED board, save away pointers to the cib's
		 * for the TOD clock, Watchdog timer, and console board device.
		 */
		if (SEC_desc[i].sec_is_cons) {
			cbdcib = &cibs[i][SDEV_SCSIBOARD];
			todcib = &cibs[i][SDEV_TOD];
			wdtcib = &cibs[i][SDEV_WATCHDOG];
		}
	}

	/*
	 * Probes done, "boot" the drivers.
	 * This must be done before the SEC is initialized, as the boot
	 * routine may need to ask/tell the device something, and interrupts
	 * aren't available yet.
	 */

	for (sec = sec_conf; sec->sec_driver; ++sec) {
		secd = sec->sec_driver;
		if (secd->sed_cflags & SED_HASBOOT)
			(*secd->sed_boot)(sec->sec_nent, sec->sec_dev);
	}

	/*
	 * Inform the SCSI/Ether controller of the queues/cib/etc.
	 * And initialize SEC error reporting vectors.
	 *
	 * This relies on ivecall() returning consecutive vectors.
	 */

	SEC_errbase = ivecpeek(SEC_ERROR_BIN);
	for (i = 0; i < toc->ct_count; ++i) {
		vec = ivecall(SEC_ERROR_BIN);
		ivecinit(SEC_ERROR_BIN, vec, SEC_error);
		if (!SEC_EXISTS(i))
			continue;		/* deconf'd */
		SEC_init_channels(i, &SEC_desc[i], sec_conf, cibs[i],
			SEC_ERROR_BIN, vec);
		if (SEC_desc[i].sec_is_cons)
			conscsi_yet = 1;
	}
}

/*
 * SEC_probe()
 *	Probe for a device on a scsi/ether controller.
 *
 * Handles wild-card SEC specification, finding a SEC with the device
 * if it exists.  But we won't look where a device has already been found.
 *
 * Returns pointer to SEC descriptor that device lives on or zero if
 * device not found.
 */

struct sec_desc *
SEC_probe(secd, sd, found)
	struct sec_driver *secd;
	register struct sec_dev *sd;
	u_int found[MAXNUMSEC*3];
{
	register int SEC_idx;
	register struct sec_desc *sec;
	register int target, unit;
	int secbits, targstart, targend, targincr, unitmin, unitmax;
	int proberes;
	struct sec_probe sp, sp0;

	/*
	 * SEC is either wild-carded or must exist in the configuration.
	 */

	if (sd->sd_sec_idx >= 0 && !SEC_EXISTS(sd->sd_sec_idx))
		return ((struct sec_desc *)0);

	/*
	 * If no probe procedure, then assume it exists if the
	 * desired SEC exists.  Note:  SEC_EXISTS is false for
	 * wildcard SEC index.
	 */

	if ((secd->sed_cflags & SED_HASPROBE) == 0) {
		if (WILDCARD(sd)) {
			printf("illegal config: %s device can't be probed for wildcards.\n",
				secd->sed_name);
			return ((struct sec_desc *)0);
		}
		if (SEC_EXISTS(sd->sd_sec_idx))
			return (&SEC_desc[sd->sd_sec_idx]);
		else
			return ((struct sec_desc *)0);
	}

	/*
	 * It has a probe procedure, so we have to try to find it.
	 * For SCSI/Ether controller devices, this means
	 * we may have to look at the target adapter and unit,
	 * as well as the controller.
	 * The loops themselves (though nested to a depth of 4 if
	 * you count the one that calls this) shouldn't really be
	 * a speed problem.  We set up loop ranges outside the loop
	 * to speed up the autoconf of those things already specified.
	 */

	if (sd->sd_target >= 0) {
		targstart = targend = sd->sd_target;
	} else if (!CCS_present) {
		targstart = 7; targend = 0; targincr = -1;
	} else {
		targstart = 0; targend = 7; targincr = 1;
	}
	if (sd->sd_unit >= 0) {
		unitmin = unitmax = sd->sd_unit;
	} else {
		unitmin = 0; unitmax = 7;
	}
	for (SEC_idx = 0, secbits = SECvec; secbits != 0; ++SEC_idx, secbits >>= 1) {
		if (!SEC_EXISTS(SEC_idx))
			continue;

		if (sd->sd_sec_idx >= 0 && sd->sd_sec_idx != SEC_idx)
			continue;

		sec = &SEC_desc[SEC_idx];
		sp.secp_desc = sec;
		sp.secp_flags = sd->sd_flags;
		sp.secp_puq = sec->sec_powerup;

		if ((secd->sed_cflags & SED_IS_SCSI) == 0) {
			/*
			 * The device is not a SCSI bus device, and
			 * it matches this SCSI/Ether controller.
			 * If no other device has been found here, probe it.
			 */
			if (sd->sd_unit < 0) {
				printf("SEC_probe: invalid config: non-scsi device has unit wildcard\n");
				continue;
			}
			sp.secp_chan = secd->sed_base_chan + sd->sd_unit;
			sp.secp_target = 0;
			if (found[SEC_idx*3 + (sp.secp_chan >> 5)] & (1 << (sp.secp_chan & 0x1F)))
				continue;

			if ((*secd->sed_probe)(&sp)) {
				sd->sd_sec_idx = SEC_idx;
				sd->sd_target = 0;
				sd->sd_chan = sp.secp_chan;
				sd->sd_alive = 1;
				return (sec);
			}
		} else {
			/*
			 * It's a SCSI bus device.  If this is our first time
			 * here, probe the guy at the distinguished target
			 * location to see if it is an embedded SCSI disk.
			 */
			if ((strcmp(secd->sed_name, "sd") == 0)
			  && (sd->sd_target < 0)
			  && (CCS_magic_target != sec->sec_target_no)
			  && (probescsi_yet == 0)) {
				probescsi_yet++;
				sp0 = sp;
				sp0.secp_target = CCS_magic_target;
				sp0.secp_unit = 0;
				sp0.secp_chan = secd->sed_base_chan
							+ CCS_magic_target*8;

				/*
				 * if the guy exists and is an embedded
				 * SCSI drive, prepare to do all scans
				 * of SCSI targets from low to high
				 */

				proberes = (*secd->sed_probe)(&sp0);
				if ((proberes & SECP_FOUND) == SECP_FOUND 
				    && CCS_present) {
					targstart = 0; targend = 7;
					targincr = 1;
				}
			}

			/*
			 * For wildcarding, etc. we have to look at
			 * all target adapters and unit numbers.
			 * But for non-wildcarded entries, targmin, etc
			 * are set up just to look at the right target/unit.
			 * Search target adapters in reverse numeric order
			 * if the drive at target 0 is not an embedded SCSI,
			 * in numeric order otherwise.
			 */
			for (target = targstart; target != targend + targincr;
						    target += targincr) {
				if (target == sec->sec_target_no)
					continue;

				/*
				 * loop short-circuit:  no way our goal
				 * can be resolved here;
				 * no need to scan further.
				 */
				if (SECnoprobe[SEC_idx] & (1 << target))
					continue;

				for (unit = unitmin; unit <= unitmax; ++unit) {

					/*
					 * Somewhere along the line, this
					 * (SEC_idx, target, unit) matches the
					 * one we are trying to configure.
					 * Probe for it.
					 */

					sp.secp_target = target;
					sp.secp_unit = unit;
					sp.secp_chan = secd->sed_base_chan + target*8 + unit;
					if (found[SEC_idx*3 + (sp.secp_chan >> 5)] & (1 << (sp.secp_chan & 0x1F)))
						continue;

					proberes = (*secd->sed_probe)(&sp);

					if ((proberes & SECP_ONELUN)
								== SECP_ONELUN)
						SECnoprobe[SEC_idx] |=
							1 << target;

					if ((proberes & SECP_FOUND) 
								== SECP_FOUND) {
						sd->sd_sec_idx = SEC_idx;
						sd->sd_target = target;
						sd->sd_unit = unit;
						sd->sd_chan = sp.secp_chan;
						sd->sd_alive = 1;
						return (sec);
					}

					/*
					 * no need to scan for
					 * more units or ever look at this
					 * target again.
					 */
					if (proberes & SECP_NOTARGET) {

						/*
						 * NOTARGET may come up
						 * erroneously true on valid
						 * targets with invalid units.
						 */
						if (unit == 0) {
							SECnoprobe[SEC_idx] |=
								1 << target;
						}
						break;
					}

					if (proberes == SECP_NOTFOUND)
						continue;
				}
			}
		}
	}

	/*
	 * Ran through the existing SEC's, targets, etc. and didn't find it;
	 */

	return ((struct sec_desc *)0);
}


/*
 * Allocate program queues for SEC devices.
 */

SEC_allocate_progqs(sd)
	register struct sec_dev *sd;
{
	register int nbytes;

	/*
	 * Allocate the request queue, and the programs.
	 */
	nbytes = (int)(&((struct sec_progq *)0)->pq_un.pq_progs[sd->sd_req_size]);
	if (nbytes <= 0) {
		sd->sd_requestq = (struct sec_progq *)0;
	} else {
		sd->sd_requestq = (struct sec_progq *) calloc(nbytes);
	}

	if (sd->sd_chan != SDEV_ETHERREAD) {
		/*
		 * Non-ether-read output queue.
		 * These all look alike from device to device.
		 * We just allocate the output queue, as the programs
		 * will be filled in by firmware as the programs complete.
		 */
		nbytes = (int)(&((struct sec_progq *)0)->pq_un.pq_progs[sd->sd_doneq_size]);
		if (nbytes <= 0) {
			sd->sd_doneq = (struct sec_progq *)0;
		} else {
			sd->sd_doneq = (struct sec_progq *)calloc(nbytes);
		}
	} else {
		/*
		 * Ether read output (done) queue looks different.
		 */
		nbytes = (int)(&((struct sec_eprogq *)0)->epq_status[sd->sd_doneq_size]);
		if (nbytes <= 0) {
			sd->sd_doneq = (struct sec_progq *)0;
		} else {
			sd->sd_doneq = (struct sec_progq *)calloc(nbytes);
		}
	}
}

/*
 * SEC_fill_progq - fill a program queue with device programs.
 *
 * This may be called by the device driver from its boot procedure
 * to fill in the array of drive program pointers allocated by
 * the autoconfig code.
 */

SEC_fill_progq(progq, n, width)
	register struct sec_progq *progq;
{
	register int i;
	caddr_t	base = calloc(n*width);

	for (i = 0; i < n; ++i) {
		progq->pq_un.pq_progs[i] =
		    (struct sec_dev_prog *)
			KVTOPHYS(base + i*width, struct sec_dev_prog *);
#ifdef DEBUG
		if (sec_debug > 2 || (sec_debug > 0 && i == 0))
			printf("0x%x->pq_un.pq_progs[%d] = 0x%x\n",
					progq, i, progq->pq_un.pq_progs[i]);
#endif DEBUG
	}
	progq->pq_head = progq->pq_tail = 0;
}

/*
 * SEC_alloc_channels()
 *	Initialize the channels for a single SEC controller.
 */

struct sec_cib *
SEC_alloc_channels(SEC_idx, desc, conf)
	int SEC_idx;
	register struct sec_desc *desc;
	struct sec_conf *conf;
{
	struct sec_driver *driver;
	register struct sec_dev *dev;
	struct sec_cib *cibs;
	int i, chan;

	assert(desc - SEC_desc == SEC_idx);
	assert(desc->sec_powerup != 0);
	cibs = (struct sec_cib *) calloc(SDEV_NUM_DEVICES*sizeof(struct sec_cib));
#ifdef DEBUG
	if (sec_debug > 1)
		printf("SEC_alloc_channels: cibs @ 0x%x\n", cibs);
#endif DEBUG

	for (; conf->sec_driver; ++conf) {
		driver = conf->sec_driver;
		for (i = 0; i < conf->sec_nent; ++i) {
			dev = &conf->sec_dev[i];
			if (!dev->sd_alive)
				continue;

			if (dev->sd_sec_idx != SEC_idx)
				continue;

			/*
			 * Found a device on this SEC controller.
			 * Add its entry to the data to be initialized.
			 */
			if (driver->sed_cflags & SED_IS_SCSI) {
				chan = driver->sed_base_chan + 8*dev->sd_target + dev->sd_unit;
			} else {
				chan = driver->sed_base_chan + dev->sd_unit;
			}
			dev->sd_cib = &cibs[chan];
			assert(dev->sd_cib != 0);
			assert(dev->sd_requestq != 0);
			assert(dev->sd_doneq != 0);
			assert(dev->sd_bin != 0);
		}
	}

#ifdef DEBUG
	if (sec_debug > 1)
		printf("done\n");
#endif DEBUG
	return (cibs);
}


/*
 * SEC_init_channels
 *
 * Issue the INIT command to the specified SEC controller
 * after collecting all the relevant data for each channel.
 */

#define	DELAY_TIME	50000

SEC_init_channels(SEC_idx, desc, conf, cibs, errbin, errvec)
	int SEC_idx;
	register struct sec_desc *desc;
	struct sec_conf *conf;
	struct sec_cib *cibs;
	u_char errbin;
	u_char errvec;
{
	struct sec_init_chan_data data;
	struct sec_driver *driver;
	register struct sec_chan_descr *cp;
	register struct sec_dev *dev;
	int i, chan, delay;

	assert(desc - SEC_desc == SEC_idx);
	assert(KVTOPHYS(&data, int) < 4*1024*1024);
	assert(desc->sec_powerup != 0);

	data.sic_status = 0;
	data.sic_cib = KVTOPHYS(cibs, struct sec_cib *);
	for (i = 0; i < SDEV_NUM_DEVICES; ++i) {
		data.sic_chans[i].scd_requestq = 0;
		data.sic_chans[i].scd_doneq = 0;
		data.sic_chans[i].scd_bin = 0;
		data.sic_chans[i].scd_vector = 0;
		data.sic_chans[i].scd_destslic = 0;
	}
#ifdef DEBUG
	if (sec_debug > 1)
		printf("SEC_init_channels: cibs @ 0x%x, data @ 0x%x\n", data.sic_cib, &data);
#endif DEBUG

	for (; conf->sec_driver; ++conf) {
		driver = conf->sec_driver;
		for (i = 0; i < conf->sec_nent; ++i) {
			dev = &conf->sec_dev[i];
			if (!dev->sd_alive)
				continue;

			if (dev->sd_sec_idx != SEC_idx)
				continue;

			/*
			 * Found a device on this SEC controller.
			 * Add its entry to the data to be initialized.
			 */
			if (driver->sed_cflags & SED_IS_SCSI) {
				chan = driver->sed_base_chan + 8*dev->sd_target + dev->sd_unit;
			} else {
				chan = driver->sed_base_chan + dev->sd_unit;
			}
			cp = &data.sic_chans[chan];
			if (cp ->scd_requestq != (struct sec_progq *)0) {
				printf("%s%d: ioconf.c error, already initialized: target %d unit %d\n",
					driver->sed_name, i, dev->sd_target, dev->sd_unit);
				continue;
			}
			assert(dev->sd_cib != 0);
			assert(dev->sd_requestq != 0);
			assert(dev->sd_doneq != 0);
			assert(dev->sd_bin != 0);
			cp->scd_requestq =
			    KVTOPHYS(dev->sd_requestq, struct sec_progq *);
		/*	cp->scd_requestq->pq_head = cp->scd_requestq->pq_tail = dev->sd_req_size; */
			dev->sd_requestq->pq_head = dev->sd_req_size;
			dev->sd_requestq->pq_tail = dev->sd_req_size;

			cp->scd_doneq =
			    KVTOPHYS(dev->sd_doneq, struct sec_progq *);
		/*	cp->scd_doneq->pq_head = cp->scd_doneq->pq_tail = dev->sd_doneq_size; */
			dev->sd_doneq->pq_head = dev->sd_doneq_size;
			dev->sd_doneq->pq_tail = dev->sd_doneq_size;

			cp->scd_bin = dev->sd_bin;
			cp->scd_vector = dev->sd_vector;
			cp->scd_destslic = dev->sd_destslic;
			assert(cp->scd_requestq != 0);
			assert(cp->scd_doneq != 0);
			assert(cp->scd_bin != 0);
#ifdef DEBUG
			if (sec_debug > 1)
				printf("  add 0x%x: dev %s%d: unit %d base %d chan %d cib 0x%x rq 0x%x dq 0x%x bin %d vector 0x%x destslic 0x%x\n",
					cp, driver->sed_name, i, dev->sd_unit,
					driver->sed_base_chan, chan,
					&data.sic_cib[chan], cp->scd_requestq,
					cp->scd_doneq, cp->scd_bin,
					cp->scd_vector, cp->scd_destslic);
#endif DEBUG
		}
	}

	/*
	 * Initialize TOD interrupts from console SCED.
	 */
	if (desc->sec_is_cons) {
		data.sic_chans[SDEV_TOD].scd_bin = TODCLKBIN;
		data.sic_chans[SDEV_TOD].scd_vector = TODCLKVEC;
#ifdef	MACH
		data.sic_chans[SDEV_TOD].scd_destslic = mono_P_slic;
#else	MACH
		data.sic_chans[SDEV_TOD].scd_destslic = SL_GROUP|TMPOS_GROUP;
#endif	MACH
	}

	/*
	 * Initialize this SEC's error bin, vector. Also, tell SEC
	 * where to place the copy of its access error register for
	 * the kernel.
	 */
	data.sic_chans[SDEV_SCSIBOARD].scd_bin = errbin;
	data.sic_chans[SDEV_SCSIBOARD].scd_vector = errvec;
#ifdef	MACH
	data.sic_chans[SDEV_SCSIBOARD].scd_destslic = mono_P_slic;
#else	MACH
	data.sic_chans[SDEV_SCSIBOARD].scd_destslic = SL_GROUP|TMPOS_GROUP;
#endif	MACH
	data.sic_chans[SDEV_SCSIBOARD].scd_doneq =
	    KVTOPHYS(&SEC_accerr[errvec-SEC_errbase], struct sec_progq *);

	/*
	 * Initialize the channels.
	 */
	desc->sec_powerup->pu_cib.cib_inst = SINST_INIT;
	desc->sec_powerup->pu_cib.cib_status = KVTOPHYS(&data, int *);
#ifdef DEBUG
	if (sec_debug>1) {
		printf("Before SINST_INIT: cib 0x%x, cib_status 0x%x\nchans:\n",
				&desc->sec_powerup->pu_cib,
				desc->sec_powerup->pu_cib.cib_status);
		for (i = 0; i < SDEV_NUM_DEVICES; ++i) {
			if (data.sic_chans[i].scd_requestq == (struct sec_progq *)0)
				continue;
			cp = &data.sic_chans[i];
			printf("   %d (0x%x): reqq 0x%x doneq 0x%x bin %d vector %d\n",
				i, cp, cp->scd_requestq, cp->scd_doneq,
				cp->scd_bin, cp->scd_vector);
		}
		printf("mIntr(%d, 7, 0): ", desc->sec_slicaddr);
	}
#endif DEBUG
	mIntr(desc->sec_slicaddr, 7, 0);
	delay = calc_delay((unsigned int)DELAY_TIME);
	while ((data.sic_status & SINST_INSDONE) == 0) {
		if (delay-- <= 0) {
			printf("SEC_init_channels: no response after %d peeks\n",
					calc_delay((unsigned int)DELAY_TIME));
			break;
		}
	}

	data.sic_status &= ~SINST_INSDONE;
#ifdef DEBUG
	if (sec_debug > 1)
		printf("init_chan: status %d\n", data.sic_status);
	if (data.sic_status != 0) {
		printf("can't initialize SCSI/Ether controller %d: status %d\n",
				i, data.sic_status);
	}
#endif DEBUG
	/*
	 * These are no longer valid.
	 */
	desc->sec_powerup = (struct sec_powerup *)0;
#ifdef DEBUG
	if (sec_debug>1)
		printf("done\n");
#endif DEBUG
}

/*
 * SEC_error()
 *	Error interrupt from SEC board(s).
 *
 * SEC received an access error. Report and panic.
 * This is a SPLHI interrupt routine.
 */

int
SEC_error(vector)
	u_char	vector;
{
	int offset;

	offset = vector - SEC_errbase;
	printf("SEC%d received Access Error\n", offset);
	access_error(SEC_accerr[offset]);
	panic("SEC Access Error");
}

/* 
 * The following are run-time support routines used by device drivers.
 */
#ifdef	MACH_KERNEL
/*
 * buf_iat()
 *	Takes an io_req_t ('buf') and a struct iat * to
 *	set up an indirect address table for the SED.
 *
 * Returns nothing.
 * Does *NO* locking.
 *
 * Implementation assumes that the number of iat slots pointed to
 * by the iat parameter has enough entries to handle
 * (ior->io_count + (IATBYTES-1))/IATBYTES iat slots.
 *
 * All data is in the kernel map.
 *
 * Assumes NBPG >= IATBYTES.
 */

/*ARGSUSED*/
buf_iat(bp, iat, niat)
	struct buf	*bp;		/* io_req_t */
	register struct sec_iat *iat;	/* iat descriptor */
	int		niat;		/* Max iats to fill */
{
	register int	bcount;
	int		junkbytes;
	vm_offset_t	virt_addr;
	vm_offset_t	phys_addr;
	register pt_entry_t *pte;
	int		pgoffset;
	int		iatoffset;

	bcount = bp->b_bcount;
	junkbytes = 0;

	/*
	 * Look into alignment, since we can start on an arbitrary
	 * boundary.  Once the offset into the page is found and mapped,
	 * the iats are filled in IATBYTE chunks until the count is zero.
	 */

	virt_addr = (vm_offset_t)bp->b_un.b_addr;

	junkbytes = bcount % DEV_BSIZE;
	pgoffset = virt_addr & (I386_PGBYTES - 1);
	iatoffset = pgoffset & (IATBYTES-1);

	pte = pmap_pte(kernel_pmap, virt_addr);
	phys_addr = pte_to_pa(*pte) + pgoffset;

	iat->iat_data = (u_char *) phys_addr;
	iat->iat_count = MIN(IATBYTES-iatoffset, bcount);
	bcount -= iat->iat_count;
	phys_addr += iat->iat_count;

	++pte;
	++iat;

	for (; bcount > 0; phys_addr += IATBYTES, bcount -= IATBYTES, iat++)
	{
	    if ((phys_addr & (I386_PGBYTES-1)) == 0) {
		phys_addr = pte_to_pa(*pte);
		pte++;
	    }
	    iat->iat_count = MIN(bcount, IATBYTES);
	    iat->iat_data = (u_char *)phys_addr;
	}

	/*
	 * On raw transfers, round up to a DEV_BSIZE (512) byte transfer
	 * because the interface is block (DEV_BSIZE) oriented.
	 */
	if (junkbytes) {
	    iat->iat_count = DEV_BSIZE - junkbytes;
	    iat->iat_data = SECgarbuf;
	}
}

/*
 * bufiat_sz()
 *	Takes a "bp" (struct buf *) and returns	the number of
 *	indirect address tables needed for this io operation.
 *
 * Returns count of iats required.
 *
 * Does *no* locking.
 *
 * Assumes NBPG >= IATBYTES.
 */

buf_iatsz(bp)
	register struct	buf	*bp;			/* buffer header */
{

	/*
	 * Raw can start on any byte boundary so will add an
	 * extra iat to map an additional page when it is
	 * needed.
	 *
	 * IATVARIANCE is used to tell the device drivers the 
	 * number of iats used for data alignment so that the
	 * the drivers can calculate the maximum transfer count
	 * based on an iat count.
	 *
	 * Add an additional iat for garbage collection of the
	 * last block.
	 */

	return (howmany(bp->b_bcount, IATBYTES) + IATVARIANCE + 1);
}

#else	/* MACH_KERNEL */

#ifdef	MACH

/*
 * buf_iat()
 *	Takes a "bp" (struct buf *) and iat (struct iat *) to
 *	setup an indirect address table for the SED.
 *
 * Returns returns nothing.
 * Does *no* locking.
 * Panics if bad pte found; "can't" happen.
 *
 * Implementation assumes that the number of iat slots
 * pointed to by the iat parameter has enough entries to
 * handle (bp->b_bcount+(IATBYTES-1))/IATBYTES iat slots.
 *
 * B_RAWIO, B_PTEIO, B_PTBIO cases must flush TLB to avoid stale mappings
 * thru Usrptmap[], since this is callable from interrupt procedures.
 *
 * Also assumes NBPG >= IATBYTES, and IATBYTES divides CLBYTES.
 */

/*ARGSUSED*/
buf_iat(bp, iat, niat)
	struct	buf	*bp;			/* buffer header */
	register struct	sec_iat	*iat;		/* iat descriptor */
	int		niat;			/* Max iat's to spray */
{
	register unsigned paddr = 0;
	register int	bcount = bp->b_bcount;
	struct	pte	*pte;
	int		junkbytes = 0;
	unsigned	pgoffset;
	unsigned	iatoffset;

#ifdef	DEBUG
	assert(iat != 0);
	assert(bcount > 0);
	assert(niat > 0);
#endif	DEBUG

	/*
	 * Source/target pte's are found differently based on type
	 * of IO operation.
	 */

	if (bp->b_flags & B_PHYS) {
		/*
		 * In this case, must look into alignment of physical
		 * memory, since we can start on arbitrary boundary.
		 * Once the offset into the page is found and mapped,
		 * the iat's are sprayed in IATBYTE chunks until b_bcount 
		 * is zero.
		 */

		junkbytes = bcount % DEV_BSIZE;
		pte = (struct pte *) pmap_pte(vm_map_pmap(bp->b_proc->task->map),
					(vm_offset_t)bp->b_un.b_addr | VA_USER);
		pgoffset = (int)bp->b_un.b_addr & (NBPG-1);
		iatoffset = pgoffset & (IATBYTES-1);
		paddr = PTETOPHYS(*pte) + pgoffset;
		iat->iat_data = (u_char *) paddr;
		iat->iat_count = MIN(IATBYTES-iatoffset, bcount);
		bcount -= iat->iat_count;
		paddr += iat->iat_count;
#ifdef	DEBUG
		--niat;
		if (bcount > 0)
			assert((paddr&(IATBYTES-1)) == 0);
		if (sec_debug)
			printf("iat = 0x%x %d\n", iat->iat_data,iat->iat_count);
#endif	DEBUG
		++pte;
		++iat;
	} else {

		/*
		 * Filesys/buffer-cache IO.  These are always cluster aligned
		 * both physically and virtually.  b_bcount is a multiple of 
		 * DEV_BSIZE.
		 */

		pte = &Sysmap[btop(bp->b_un.b_addr)];
#ifdef	DEBUG
		if (((int)bp->b_un.b_addr & (IATBYTES-1)) != 0) {
			printf("bp=0x%x, addr=0x%x\n", bp, bp->b_un.b_addr);
			panic("buf_iat: bad FS IO");
		}
		if (bp->b_bcount <= 0 || bp->b_bcount > MAXBSIZE) {
			printf("bp=0x%x, bcount=%d\n", bp, bp->b_bcount);
			panic("buf_iat: bad FS count");
		}
#endif	DEBUG
	}

	/*
	 * Check count, and set up the iat's.
	 */

#ifdef	DEBUG
	assert(howmany(bcount);
#endif	DEBUG

	for (; bcount > 0; paddr += IATBYTES, bcount -= IATBYTES, iat++) {
		if ((paddr & (NBPG-1)) == 0) {
#ifdef	DEBUG
			assert(PTEPF(*pte) != 0);
#endif	DEBUG
			paddr = PTETOPHYS(*pte);
			++pte;
		}
		iat->iat_count = MIN(bcount, IATBYTES);
		iat->iat_data = (u_char *) paddr;
#ifdef	DEBUG
		if (sec_debug)
			printf("iat = 0x%x %d\n", iat->iat_data,iat->iat_count);
		assert(iat->iat_count > 0);
#endif	DEBUG
	}

	/*
	 * On raw xfers, round up to a DEV_BSIZE (512) byte transfer because
	 * interface is block (DEV_BSIZE) oriented.
	 */

	if (junkbytes) {
		iat->iat_count = DEV_BSIZE - junkbytes;
		iat->iat_data = SECgarbuf;
#ifdef DEBUG
		if (sec_debug > 1)
			printf("buf_iat: %d garbage bytes from bcount = %d\n",
					iat->iat_count, bp->b_bcount); 
#endif DEBUG
	}
}

/*
 * bufiat_sz()
 *	Takes a "bp" (struct buf *) and returns	the number of
 *	indirect address tables needed for this io operation.
 *
 * Returns count of iats required.
 *
 * Does *no* locking.
 *
 * Assumes NBPG >= IATBYTES.
 */

buf_iatsz(bp)
	register struct	buf	*bp;			/* buffer header */
{

	if (bp->b_flags & B_PHYS) {
		/*
		 * Raw can start on any byte boundary so will add an
		 * extra iat to map an additional page when it is
		 * needed.
		 *
		 * IATVARIANCE is used to tell the device drivers the 
		 * number of iat's used for data alignment so that the
		 * the drivers can calculate the maximum transfer count
		 * based on an iat count.
		 *
		 * Add an additional iat for garbage collection of the
		 * last block.
		 */

		return (howmany(bp->b_bcount, IATBYTES) + IATVARIANCE +1);
	} else {

		/*
		 * Pte-based IO -- already know pte of 1st page, which
		 * is cluster aligned, and b_count is a multiple of CLBYTES.
		 *
		 * Page-table based IO is page-aligned and b_bcount is
		 * a multiple of NBPG.
		 *
		 * Filesys/buffer-cache IO.  These always start cluster aligned
		 * both physically and virtually. 
		 */
		return (howmany(bp->b_bcount, IATBYTES));
	}

}
#else	MACH

#if	NBPG >= IATBYTES			/* eg, SGS */

/*
 * buf_iat()
 *	Takes a "bp" (struct buf *) and iat (struct iat *) to
 *	setup an indirect address table for the SED.
 *
 * Returns returns nothing.
 * Does *no* locking.
 * Panics if bad pte found; "can't" happen.
 *
 * Implementation assumes that the number of iat slots
 * pointed to by the iat parameter has enough entries to
 * handle (bp->b_bcount+(IATBYTES-1))/IATBYTES iat slots.
 *
 * B_RAWIO, B_PTEIO, B_PTBIO cases must flush TLB to avoid stale mappings
 * thru Usrptmap[], since this is callable from interrupt procedures.
 *
 * Also assumes NBPG >= IATBYTES, and IATBYTES divides CLBYTES.
 */

/*ARGSUSED*/
buf_iat(bp, iat, niat)
	struct	buf	*bp;			/* buffer header */
	register struct	sec_iat	*iat;		/* iat descriptor */
	int		niat;			/* Max iat's to spray */
{
	register unsigned paddr = 0;
	register int	bcount = bp->b_bcount;
	struct	pte	*pte;
	int		junkbytes = 0;
	unsigned	pgoffset;
	unsigned	iatoffset;
	struct pte	*vtopte();

#ifdef	DEBUG
	assert(iat != 0);
	assert(bcount > 0);
	assert(niat > 0);
#endif	DEBUG

	/*
	 * Source/target pte's are found differently based on type
	 * of IO operation.
	 */

	switch(bp->b_iotype) {
	case B_RAWIO:					/* RAW IO */
		/*
		 * In this case, must look into alignment of physical
		 * memory, since we can start on arbitrary boundary.
		 * Once the offset into the page is found and mapped,
		 * the iat's are sprayed in IATBYTE chunks until b_bcount 
		 * is zero.
		 */

		flush_tlb();
		junkbytes = bcount % DEV_BSIZE;
		pte = vtopte(bp->b_proc, btop(bp->b_un.b_addr));
		pgoffset = (int)bp->b_un.b_addr & (NBPG-1);
		iatoffset = pgoffset & (IATBYTES-1);
		paddr = PTETOPHYS(*pte) + pgoffset;
		iat->iat_data = (u_char *) paddr;
		iat->iat_count = MIN(IATBYTES-iatoffset, bcount);
		bcount -= iat->iat_count;
		paddr += iat->iat_count;
#ifdef	DEBUG
		--niat;
		if (bcount > 0)
			assert((paddr&(IATBYTES-1)) == 0);
		if (sec_debug)
			printf("iat = 0x%x %d\n", iat->iat_data,iat->iat_count);
#endif	DEBUG
		++pte;
		++iat;
		break;

	case B_FILIO:					/* file-sys IO */
		/*
		 * Filesys/buffer-cache IO.  These are always cluster aligned
		 * both physically and virtually.  b_bcount is a multiple of 
		 * DEV_BSIZE.
		 */

		pte = &Sysmap[btop(bp->b_un.b_addr)];
#ifdef	DEBUG
		if (((int)bp->b_un.b_addr & (IATBYTES-1)) != 0) {
			printf("bp=0x%x, addr=0x%x\n", bp, bp->b_un.b_addr);
			panic("buf_iat: bad FS IO");
		}
		if (bp->b_bcount <= 0 || bp->b_bcount > MAXBSIZE) {
			printf("bp=0x%x, bcount=%d\n", bp, bp->b_bcount);
			panic("buf_iat: bad FS count");
		}
#endif	DEBUG
		break;

	case B_PTBIO:					/* Page-Table IO */
		/*
		 * Page-Table IO: like B_PTEIO, but can start/end with
		 * non-cluster aligned memory (but is always HW page
		 * aligned).  Count is multiple of NBPG.
		 *
		 * Since NBPG >= IATBYTES, this case is identical to
		 * B_PTEIO, so...
		 */

	case B_PTEIO:					/* swap/page IO */
		/*
		 * Pte-based IO -- already know pte of 1st page, which
		 * is cluster aligned, and b_count is a multiple of CLBYTES.
		 */

		flush_tlb();
		pte = bp->b_un.b_pte;
		break;

	default:
		panic("buf_iat: bad b_iotype");
		/*NOTREACHED*/
	}

	/*
	 * Check count, and set up the iat's.
	 */

#ifdef	DEBUG
	assert(howmany(bcount);
#endif	DEBUG

	for (; bcount > 0; paddr += IATBYTES, bcount -= IATBYTES, iat++) {
		if ((paddr & (NBPG-1)) == 0) {
#ifdef	DEBUG
			assert(PTEPF(*pte) != 0);
#endif	DEBUG
			paddr = PTETOPHYS(*pte);
			++pte;
		}
		iat->iat_count = MIN(bcount, IATBYTES);
		iat->iat_data = (u_char *) paddr;
#ifdef	DEBUG
		if (sec_debug)
			printf("iat = 0x%x %d\n", iat->iat_data,iat->iat_count);
		assert(iat->iat_count > 0);
#endif	DEBUG
	}

	/*
	 * On raw xfers, round up to a DEV_BSIZE (512) byte transfer because
	 * interface is block (DEV_BSIZE) oriented.
	 */

	if (junkbytes) {
		iat->iat_count = DEV_BSIZE - junkbytes;
		iat->iat_data = SECgarbuf;
#ifdef DEBUG
		if (sec_debug > 1)
			printf("buf_iat: %d garbage bytes from bcount = %d\n",
					iat->iat_count, bp->b_bcount); 
#endif DEBUG
	}
}

/*
 * bufiat_sz()
 *	Takes a "bp" (struct buf *) and returns	the number of
 *	indirect address tables needed for this io operation.
 *
 * Returns count of iats required.
 *
 * Does *no* locking.
 *
 * Assumes NBPG >= IATBYTES.
 */

buf_iatsz(bp)
	register struct	buf	*bp;			/* buffer header */
{

	switch(bp->b_iotype) {
	case B_RAWIO:					/* RAW IO */
		/*
		 * Raw can start on any byte boundary so will add an
		 * extra iat to map an additional page when it is
		 * needed.
		 *
		 * IATVARIANCE is used to tell the device drivers the 
		 * number of iat's used for data alignment so that the
		 * the drivers can calculate the maximum transfer count
		 * based on an iat count.
		 *
		 * Add an additional iat for garbage collection of the
		 * last block.
		 */

		return (howmany(bp->b_bcount, IATBYTES) + IATVARIANCE +1);

	case B_PTEIO:					/* swap/page IO */
	case B_PTBIO:					/* Page-Table IO */
	case B_FILIO:					/* file-sys IO */
		/*
		 * Pte-based IO -- already know pte of 1st page, which
		 * is cluster aligned, and b_count is a multiple of CLBYTES.
		 *
		 * Page-table based IO is page-aligned and b_bcount is
		 * a multiple of NBPG.
		 *
		 * Filesys/buffer-cache IO.  These always start cluster aligned
		 * both physically and virtually. 
		 */
		return (howmany(bp->b_bcount, IATBYTES));

	default:
		panic("buf_iatsz: bad b_iotype");
		/*NOTREACHED*/
	}
}

#else	NBPG < IATBYTES				/* eg, FGS */

/*
 * PTESEDOFF returns offset of memory pointed at by pte.
 */

#define	PTESEDOFF(pte) \
		((unsigned)((*(int*)(&(pte))) & ((~(NBPG-1))&(IATBYTES-1))))

/*
 * buf_iat()
 *	Takes a "bp" (struct buf *) and iat (struct iat *) to
 *	setup an indirect address table for the SED.
 *
 * Returns returns nothing.
 * Does *no* locking.
 * Panics if bad pte found; "can't" happen.
 *
 * Implementation assumes that the number of iat slots
 * pointed to by the iat parameter has enough entries to
 * handle (bp->b_bcount+(CLBYTES-1))/CLBYTES iat slots.
 * Also assumes NBPG<=IATBYTES<=CLBYTES, and IATBYTES divides CLBYTES.
 */

buf_iat(bp, iat, niat)
	register struct	buf	*bp;		/* buffer header */
	register struct	sec_iat	*iat;		/* iat descriptor */
	int		niat;			/* Max iat's to spray */
{
	register struct pte *pte;
	register int	count;
	register int	bcount;
	int		offset;
	struct pte	*vtopte();
	int		iatbytes = 0;
	extern		u_char	*SECgarbuf;

#ifdef	DEBUG
	assert(iat != 0);
	assert(bp->b_bcount > 0);
	assert(niat > 0);
#endif	DEBUG

	/*
	 * Source/target pte's are found differently based on type
	 * of IO operation.
	 */

	switch(bp->b_iotype) {
	case B_RAWIO:					/* RAW IO */
		/*
		 * In this case, must look into alignment of physical
		 * memory, since we can start on arbitrary boundary.
		 * Once the offset into the page is found and mapped,
		 * the iat's are sprayed in IATBYTE chunks until b_bcount 
		 * is zero.
		 */

		flush_tlb();
		pte = vtopte(bp->b_proc, btop(bp->b_un.b_addr));
		offset = PTESEDOFF(*pte) + ((int)bp->b_un.b_addr & (NBPG-1));
		pte -= btop(offset);
		iat->iat_data = (u_char *)PTETOPHYS(*pte) + offset;
		iat->iat_count = MIN(IATBYTES-offset, bp->b_bcount);
		bcount = bp->b_bcount - iat->iat_count;
		count = (bcount + (IATBYTES-1)) / IATBYTES;
		iat++;
		niat--;
		pte += IATSIZE;
		iatbytes = bp->b_bcount % DEV_BSIZE;
		break;

	case B_FILIO:					/* file-sys IO */
		/*
		 * Filesys/buffer-cache IO.  These are always cluster aligned
		 * both physically and virtually.  b_bcount is a multiple of 
		 * DEVBSIZE (512, currently).
		 */

		pte = &Sysmap[btop(bp->b_un.b_addr)];
		bcount = bp->b_bcount;
		count = (bcount + IATBYTES-1) / IATBYTES;
#ifdef	DEBUG
		if (((int)bp->b_un.b_addr & (IATBYTES-1)) != 0) {
			printf("bp=0x%x, addr=0x%x\n", bp, bp->b_un.b_addr);
			panic("buf_iat: bad FS IO");
		}
		if (bp->b_bcount <= 0 || bp->b_bcount > MAXBSIZE) {
			printf("bp=0x%x, bcount=%d\n", bp, bp->b_bcount);
			panic("buf_iat: bad FS count");
		}
#endif	DEBUG
		break;

	case B_PTEIO:					/* swap/page IO */
		/*
		 * Pte-based IO -- already know pte of 1st page, which
		 * is cluster aligned, and b_count is a multiple of CLBYTES.
		 * Since IATBYTES <= CLBYTES, and IATBYTES divides CLBYTES,
		 * can just divide IATBYTES.
		 *
		 * One special case here is read/write of Uarea during
		 * swap -- swap code passes in b_bcount = UPAGES*NBPG
		 * and b_iotype == B_PTEIO.  Thus, insure CLSIZE divides
		 * UPAGES.
		 */

#		if UPAGES % CLSIZE != 0
			ERROR -- code assumes UPAGES is multiple of CLSIZE
#		endif

		flush_tlb();
		pte = bp->b_un.b_pte;
		bcount = bp->b_bcount;
		count = bcount / IATBYTES;
		break;

	case B_PTBIO:					/* Page-Table IO */
		/*
		 * Page-Table IO: like B_PTEIO, but can start/end with
		 * non-cluster aligned memory (but is always HW page
		 * aligned).  Count is multiple of NBPG.  Case mostly
		 * borrowed from B_RAWIO.
		 *
		 * Separate case for greater efficiency in B_PTEIO.
		 */

		flush_tlb();
		pte = bp->b_un.b_pte;
		offset = PTESEDOFF(*pte);
		pte -= btop(offset);
		iat->iat_data = (u_char *)PTETOPHYS(*pte) + offset;
		iat->iat_count = MIN(IATBYTES-offset, bp->b_bcount);
		bcount = bp->b_bcount - iat->iat_count;
		count = (bcount + (IATBYTES-1)) / IATBYTES;
		iat++;
		niat--;
		pte += IATSIZE;
		break;

	default:
		panic("buf_iat: bad b_iotype");
		/*NOTREACHED*/
	}

	/*
	 * Check count, and set up the iat's.
	 */

	assert(count <= niat);

	for (; count--; pte += (IATBYTES/NBPG), bcount -= IATBYTES, iat++) {
		iat->iat_count = MIN(bcount, IATBYTES);
		iat->iat_data = (u_char *)PTETOPHYS(*pte);
#ifdef	DEBUG
		if (sec_debug)
			printf("iat = 0x%x %d\n", PTETOPHYS(*pte), iat->iat_count);
		assert(PTEPF(*pte) != 0);
		assert((PTETOPHYS(*pte) & (IATBYTES-1)) == 0);
		assert(iat->iat_count > 0);
#endif	DEBUG
	}

	/*
	 * On raw xfers, round up to a DEV_BSIZE (512) byte transfer because
	 * interface is block (DEV_BSIZE) oriented.
	 */
	if (iatbytes>0) {
		iat->iat_count = DEV_BSIZE - iatbytes;
		iat->iat_data = SECgarbuf;
#ifdef DEBUG
		if (sec_debug>1)
			printf("collecting garbage bytes of %d from a bcount of %d\n",
					iat->iat_count, bp->b_bcount); 
#endif DEBUG
	}
}

/*
 * bufiat_sz()
 *	Takes a "bp" (struct buf *) to
 *	determine the number of indirect address tables needed for
 *	this io operation.
 *
 * Returns count of iats required.
 * Does *no* locking.
 *
 * Guesstimate number of iat's needed for this xfer.
 * Assumes NBPG<=IATBYTES<=CLBYTES
 */

buf_iatsz(bp)
	register struct	buf	*bp;			/* buffer header */
{

	switch(bp->b_iotype) {
	case B_RAWIO:					/* RAW IO */
	case B_PTBIO:					/* Page-Table IO */
		/*
		 * Raw can start on any byte boundary so will add an
		 * extra iat to map an additional page when it is
		 * needed.  Page-Table IO similar (can start in middle
		 * of page), but more constrained.
		 *
		 * IATVARIANCE is used to tell the device drivers the 
		 * number of iat's used for data alignment so that the
		 * the drivers can calculate the maximum transfer count
		 * based on an iat count.
		 *
		 * Add an additional iat for garbage collection of the last block.
		 */

		return (howmany(bp->b_bcount, IATBYTES) + IATVARIANCE +1);

	case B_PTEIO:					/* swap/page IO */
	case B_FILIO:					/* file-sys IO */
		/*
		 * Pte-based IO -- already know pte of 1st page, which
		 * is cluster aligned, and b_count is a multiple of CLBYTES.
		 *
		 * Filesys/buffer-cache IO.  These always start cluster aligned
		 * both physically and virtually. 
		 */
		return (howmany(bp->b_bcount, IATBYTES));

	default:
		panic("buf_iatsz: bad b_iotype");
		/*NOTREACHED*/
	}
}

#endif	NBPG >= IATBYTES

#endif	MACH
#endif	/* MACH_KERNEL */

/*
 * SEC_startio - start an operation by sending a command to the
 *		sec board via slic.
 *
 * This procedure may be used until the init command is sent
 * to turn on sec interrupts, which occurs after the boot procedure
 * routines for all device drivers have been called.
 *
 * Calls mIntr() to do the actual slic fussing to send the message.
 *
 * NOTE: It's critical that the status pointer live below 0x400000
 * because the sec can't talk above that address, hence all status
 * variable must *not* live on kernel stack.
 */

SEC_startio(cmd, statptr, bin, mesg, q, slicid)
	register
	volatile int	*statptr;
	int		cmd;
	unsigned char	bin;
	unsigned char	mesg;
	unsigned char	slicid;
	struct	sec_cib	*q;
{
	register int	spin;

	*statptr = 0;
	q->cib_inst = cmd;
	q->cib_status = KVTOPHYS(statptr, volatile int *);

	mIntr(slicid, bin, mesg);

	spin = calc_delay(10 * DELAY_TIME);
	while ((*statptr & SINST_INSDONE) == 0) {
		if (spin-- <= 0) {
			printf("SEC_startio: timeout\n");
			break;
		}
	}
}

/*
 * sec_startio - start an operation by sending a command to the
 *		sec board via slic.
 *
 * This procedure is used once interrupts have been enabled
 * in the kernel.
 *
 * Calls mIntr() to do the actual slic fussing to send the message.  Use
 * bin 3, since this helps avoid SLIC-bus saturation/lockup (since SCED
 * interrupts Dynix mostly on bins 4-7, using bin 3 to interrupt SCED gives
 * SCED -> Dynix priority over Dynix -> SCED, thus SCED won't deadlock
 * against Dynix).  Initialization-time mIntr()'s can use other bins since
 * SLIC-bus is not busy at that time.
 *
 * NOTE: It's critical that the status pointer live below 0x400000
 * because the sec can't talk above that address, hence all status
 * variable must *not* live on kernel stack.
 */

sec_startio(cmd, statptr, sd)
	int		cmd;
	register
	volatile int	*statptr;
	register struct sec_dev *sd;
{
	register int	spin;

	*statptr = 0;
	sd->sd_cib->cib_inst = cmd;
	sd->sd_cib->cib_status = KVTOPHYS(statptr, int *);

	/*
	 * Must insure that the interrupt gets sent, so don't allow
	 * interrupts on this processor while sending the interrupt.
	 */

#ifdef	ns32000
	{ spl_t spl = splhi();
	mIntr(sd->sd_desc->sec_slicaddr, 3, sd->sd_chan);
	splx(spl);
	}
#endif	ns32000

#ifdef	i386
	DISABLE();
	mIntr(sd->sd_desc->sec_slicaddr, 3, sd->sd_chan);
	ENABLE();
#endif	i386

	spin = calc_delay(10 * DELAY_TIME);
	while ((*statptr & SINST_INSDONE) == 0) {
		if (spin-- <= 0) {
			printf("sec_startio: timeout\n");
			break;
		}
	}
}

/* 
 * u_char *			<= IATIFIED PTR!
 * SEC_rqinit(rqbuf, rqbufsz)
 *	u_char *rqbuf;
 *
 * SEC_rqinit - init a request sense iat chain to
 *	allow the SEC dma hardware to transfer complete
 *	request sense data buffers which are *not* 8 byte
 *	aligned. This procedure returns an SEC iat pointer
 *	that is used to point at a request sense buffer
 *	(ie. device_prog.dp_datap = rqbuf can be replaced with
 *	device_prog.dp_datap = returned pointer). Max size is 255.
 *
 *	NOTE: the larger the buffer size the more memory this uses! 
 *	Memory used =
 *		sz<8	16 bytes.
 *		sz<24	16+((sz-8)*8) 		; max 144 bytes
 *		sz>24	144+((sz-24)/2)*8+8	; max 1072 bytes (for 255 buf)
 *
 *	This hardware limititation is by-passed by
 *	creating a chain of iat's that point to the
 *	(passed in) buffer pointer in the following
 *	structure respectively:
 *	#iat's_used	length	bytes covered
 *		1	4	0-3	(standard)
 *		1	4	4-7	(extended)
 *		16	1	8-23	(additional)
 *		sz-24/2	2	24-sz	(additional)
 *	
 *	Note: this must be a generic buffer to handle
 *	target adapter interchange since the number of
 *	returned bytes from the target adapter will vary 
 *	based on vendor, error type and buffer size.
 *	Maximum buffer size allowed is 255.
 *	Assumes calloc gets buffer from an address <4Mbyte.
 *	For target adapters that transfer greater than 24 bytes
 *	it's possible to loose the last byte on an odd transfer.
 *	Returns type char * to please lint with straight replacement
 *	in existing drivers. Enforces a minimum of eight bytes to
 *	ease the iat fill out process.
 *
 *	This conforms to ansi X3T9.2/82-2 revision 14 specification.
 *
 */

u_char	*
SEC_rqinit(rqbuf, rqbufsize)
	register u_char	*rqbuf;
	register int	rqbufsize;
{
	register u_char	*ret_iatptr;
	register struct	sec_iat	*iat;		/* iat descriptor */
	register int	count;
	int	 	iat_count;
#ifdef DEBUG
	int		dcnt;
	u_char		*drqbuf = rqbuf;
#endif DEBUG

	/*
	 * Calculate the iat chain size.
	 */

	rqbufsize = MAX(rqbufsize, 8);		/* enforce minimum of eight */
	count = rqbufsize-8;
	iat_count = 2;				/* first two */
	if (count>0) {
		iat_count += (count-16 > 0) ? 16 : count;	/* bytes 8-23 */
		count  -= 16;
	}
	if (count>0) {
		iat_count += count/2 + count%2;	/* bytes 24-rqbufsize */
		count	= 0;			/* for sanity */
	}
	assert(count<=0);
#ifdef DEBUG
	if (sec_debug>1)
		printf("SEC_rqinit: iat's needed=%d\n", iat_count);
	dcnt = iat_count;			/* debug */
#endif DEBUG
	
	/*
	 * Allocate space for the iat's,
	 * and save the callers reference pointer.
	 */

	iat = (struct sec_iat *)calloc(iat_count*sizeof(struct sec_iat));
	ret_iatptr = (u_char *)SEC_IATIFY(KVTOPHYS(iat,struct sec_iat *));

	/*
	 * Fill out iat chain.
	 */

	count = 0;
	for (; count<2; count++, iat++, rqbuf = (u_char *)((int)rqbuf+4)) {
		iat->iat_count = 4;
		iat->iat_data = KVTOPHYS(rqbuf, u_char *);
	}
	iat_count -=2;

	/*
	 * Bytes 8-23.
	 */

	count = 0;
	for (; count<16 && iat_count>0; count++, iat++, iat_count--, rqbuf++) {
		iat->iat_count = 1;
		iat->iat_data = KVTOPHYS(rqbuf, u_char *);
	}

	/*
	 * Bytes 24-rqbufsize.
	 */

	for (; iat_count>0; iat++, iat_count--, rqbuf++, rqbuf++) {
		iat->iat_count = 2;
		iat->iat_data = KVTOPHYS(rqbuf, u_char *);
	}

	/*
	 * Adjust the last iat if the count is odd, the data ptr is ok.
	 */

	if (rqbufsize&1) {
		iat--;
		iat->iat_count = 1;
	}

#ifdef DEBUG
	if (sec_debug>1) {
		iat = PHYSTOKV((u_int)ret_iatptr & ~SEC_IAT_FLAG,
				struct sec_iat *);
		for (; dcnt; dcnt--, iat++)
			printf("buf 0x%x iat 0x%x count %d retiat 0x%x\n",
				drqbuf, iat->iat_data, iat->iat_count, ret_iatptr);
	}
#endif DEBUG
			
	return (ret_iatptr);
}

/*
 * sec_map()
 *	Null memory mapping function.  
 *
 * Sec configuration doesn't need to map any address space; all data
 * and interface memory is calloc()'d.
 */

sec_map() {}
