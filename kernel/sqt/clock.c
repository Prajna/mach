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
 * $Log:	clock.c,v $
 * Revision 2.3  91/07/31  18:00:21  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  12:55:04  dbg
 * 	Adapted from Sequent Symmetry sources.
 * 	[91/04/26  14:50:24  dbg]
 * 
 */

#ifndef	lint
static	char	rcsid[] = "$Header: clock.c,v 2.3 91/07/31 18:00:21 dbg Exp $";
#endif

/*
 * Machine-dependent clock routines.
 *
 * Included are the time-of-day clock initialization and
 * the per processor real-time clock initialization.
 */

/*
 * Revision 1.2  89/07/20  18:05:38  kak
 * moved balance includes
 * 
 * Revision 1.1  89/07/05  13:15:27  kak
 * Initial revision
 * 
 */

#include <mach/boolean.h>

#include <kern/time_out.h>

#include <sys/time.h>

#include <sqt/cfg.h>
#include <sqt/clock.h>
#include <sqt/slic.h>

#include <sqt/ioconf.h>
#include <sqt/vm_defs.h>
#include <sqt/hwparam.h>
#include <sqt/intctl.h>

#include <sqtsec/sec.h>

extern u_char	cons_scsi;		/* console scsi slic address */

/* For time-of-day handling */
struct	sec_cib *todcib;
struct	sec_gmode todgm;	/* getmodes command */
struct	sec_smode todsm;	/* setmodes command */

/*
 * startrtclock()
 *	Start the real-time clock.
 *
 * Startrtclock restarts the real-time clock, which provides
 * hardclock interrupts to kern_clock.c.  On Sequent HW, this
 * is one-time only per processor (eg, no restart, clock reprimes
 * itself).
 *
 * Called by localinit() during selfinit().
 * This turns on the processor-local SLIC timer.
 *
 * For testing/performance measurement convenience, enable_local_clock
 * allows the per-processor clock to be left OFF.  Need to patch the
 * kernel binary or system memory to effect this.
 */

static	boolean_t	enable_local_clock = TRUE;	/* default ON */

startrtclock()
{
	register struct cpuslic *sl = va_slic;

	if (!enable_local_clock)
		return;

	sl->sl_trv = ((sys_clock_rate * 1000000) / (SL_TIMERDIV * hz)) - 1;
	/* clear prescaler, load reload value */
	sl->sl_tcont = 0;
	sl->sl_tctl = SL_TIMERINT | LCLKBIN;	/* timer on in given bin */
}

/*
 * Routines to manipulate the SCED based time-of-day register.
 * TOD clock interrupt handling done by todclock in kern_clock.c
 *
 *
 * Inittodr initializes the time-of-day hardware which provides
 * date functions. This starts the time-of-day clock.
 *
 */
void
inittodr()
{
	time_value_t	new_time;

	register u_int todr;
	register struct	sec_gmode *todgmptr = &todgm;
	register int i;
	long deltat;
	spl_t s_ipl;

	new_time.seconds = 0;
	new_time.microseconds = 0;

	if (todcib == 0) {
		printf("todcib null - inittodr returning!\n");
		return;
	}

	/*
	 * Find console SCED and check if the TOD clock has
	 * failed powerup diagnostics.
	 */
	for (i = 0; i < NSEC; i++) {
		/* is SEC there? */
		if ((SECvec & (1 << i)) == 0)
			continue;

		if (SEC_desc[i].sec_is_cons)
			break;
	}
	if (SEC_desc[i].sec_diag_flags & CFG_S_TOD) {
		/*
		 * Clear todr if TOD failed powerup diagnostics.
		 */
		printf("WARNING: TOD failed powerup diagnostics\n");
		todr = 0;
	} else {
		/*
		 * get the current time-of-day from the SCED tod clock.
		 */
		todgmptr->gm_status = 0;
		todcib->cib_inst = SINST_GETMODE;
		todcib->cib_status = KVTOPHYS(&todgm, int *);
		s_ipl = splhi();
		mIntr(cons_scsi, TODCLKBIN, SDEV_TOD);
		splx(s_ipl);

		while ((todgmptr->gm_status & SINST_INSDONE) == 0)
			continue;

		if (todgmptr->gm_status != SINST_INSDONE)
			panic("Cannot get TOD value");

		todr = todgmptr->gm_un.gm_tod.tod_newtime;
	}

	if (todr < 13*SECYR) {
		printf("WARNING: TOD value bad -- ");
		printf("Setting TOD to default time\n");
		todr = 13*SECYR + 19*SECDAY + (2*SECDAY)/3;
	}

	new_time.seconds = todr;
	new_time.microseconds = 0;

	/*
	 * Set the time.
	 */
	{
	    int s = splhigh();
	    time = new_time;
	    splx(s);
	}

	resettodr();	/* restart the clock */
}

/*
 * Resettodr restores the time-of-day hardware after a time change.
 * Also, also called via inittodr to start todclock interrupts.
 *
 * Reset the TOD based on the time value; used when the TOD
 * has a preposterous value and also when the time is reset
 * by the stime system call.
 */
resettodr()
{
	register struct sec_smode *todsmptr = &todsm;
	spl_t s_ipl;

	todsmptr->sm_status = 0;
	todsmptr->sm_un.sm_tod.tod_freq = TODFREQ;
	todcib->cib_inst = SINST_SETMODE;
	todcib->cib_status = KVTOPHYS(&todsm, int *);
	s_ipl = splhi();
	todsmptr->sm_un.sm_tod.tod_newtime = time.tv_sec;
	/*
	 * Bin 3 is sufficient, helps avoid SLIC-bus lockup.
	 */
	mIntr(cons_scsi, 3, SDEV_TOD);
	splx(s_ipl);

	while ((todsmptr->sm_status & SINST_INSDONE) == 0)
		continue;

	if (todsmptr->sm_status != SINST_INSDONE)
		panic("Cannot set TOD value");
}
