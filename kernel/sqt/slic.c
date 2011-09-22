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
 * $Log:	slic.c,v $
 * Revision 2.3  91/07/31  18:03:48  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  12:58:59  dbg
 * 	Adapted for pure Mach kernel.
 * 	[90/10/03            dbg]
 * 
 */

#ifndef	lint
static	char	rcsid[] = "$Header: slic.c,v 2.3 91/07/31 18:03:48 dbg Exp $";
#endif

#include <sqt/SGSproc.h>
#include <sqt/slic.h>
#include <sqt/intctl.h>

/*
 * slic.c
 *	Slic functions.
 *
 * All routines assume caller arranged mutex of SLIC usage (splhi() or
 * disable processor interrupts) -- SLIC registers are not save/restored
 * across interrupts.
 *
 * Conditionals:
 *	-DCHECKSLIC	check slic results (parity, etc) for sanity
 *
 * Gates are handled in gate.c; interrupt acceptence/etc in locore.s
 */

/*
 * Revision 1.2  89/07/20  18:05:24  kak
 * moved balance includes
 * 
 * Revision 1.1  89/07/05  13:15:46  kak
 * Initial revision
 * 
 */

void	wrAddr();		/* forward */
void	wrData();		/* forward */
int	rdData();		/* forward */
spl_t	lock_subslave();	/* forward */
void	unlock_subslave();	/* forward */
/*
 * wrslave()
 *	Write to a slave port.
 *
 * Note: this does no mutex on destination; assumes caller handles mutex
 * if necessary.  If done to "self" (eg, during initialization), no mutex
 * necessary.
 *
 * Caller assures mutex of SLIC usage (splhi() or holding gate is sufficient).
 */

wrslave(destination, reg, data)
	unsigned char destination, reg, data;
{
	wrAddr(destination, reg);
	wrData(destination, data);
}

/*
 * rdslave()
 *	Read a slave port.
 *
 * Note: this does no mutex on destination; assumes caller handles mutex
 * if necessary.  If done to "self" (eg, during initialization), no mutex
 * necessary.
 *
 * Caller assures mutex of SLIC usage (splhi() or holding gate is sufficient).
 */

int
rdslave(destination, reg)
	unsigned char destination, reg;
{
	wrAddr(destination, reg);
	return(rdData(destination));
}

/*
 * wrSubslave()
 *	Write a register that responds to SLIC slave sub-register addressing.
 *
 * Some slic/slave accesses for different resources are done thru the
 * same SLIC shared (eg, SGS processor BIC); use a locking protocol
 * on relevant slic/slave accesses.
 *
 * For compatibility with diagnostic usage, if slave==0 don't do the wrAddr().
 *
 * Caller assures mutex of SLIC usage (splhi() or holding gate is sufficient).
 */

wrSubslave(slic, slave, subreg, val)
	u_char slic, slave, subreg, val;
{
	spl_t	s;

	s = lock_subslave(slic, slave);

	if (slave != 0)
		wrAddr(slic, slave);
	wrData(slic, subreg);
	wrData(slic, val);

	unlock_subslave(slic, slave, s);
}

/*
 * rdSubslave()
 *	Read a register that responds to SLIC slave sub-register addressing.
 *
 * Some slic/slave accesses for different resources are done thru the
 * same SLIC shared (eg, SGS processor BIC); use a locking protocol
 * on relevant slic/slave accesses.
 *
 * For compatibility with diagnostic usage, if slave==0 don't do the wrAddr().
 *
 * Caller assures mutex of SLIC usage (splhi() or holding gate is sufficient).
 */

int
rdSubslave(slic, slave, subreg)
	u_char slic, slave, subreg;
{
	int	val;
	spl_t	s;

	s = lock_subslave(slic, slave);

	if (slave != 0)
		wrAddr(slic, slave);
	wrData(slic, subreg);
	val = rdData(slic);

	unlock_subslave(slic, slave, s);

	return (val);
}

/*
 * wrAddr()
 *	Write address to slave.
 *
 * Caller assures mutex of SLIC usage (splhi() or holding gate is sufficient).
 */
void
wrAddr(destination, address)
	unsigned char destination, address;
{
	register struct cpuslic *sl = va_slic;

	sl->sl_dest = destination;
	sl->sl_smessage = address;
	sl->sl_cmd_stat = SL_WRADDR;
	while (sl->sl_cmd_stat & SL_BUSY)
		continue;
#ifdef	CHECKSLIC
	check_slic("wrAddr");
#endif	CHECKSLIC
}

/*
 * wrData()
 *	Write data to previously addressed slave register.
 *
 * Internal interface.
 *
 * Caller assures mutex of SLIC usage (splhi() or holding gate is sufficient).
 */
void
wrData(destination, data)
	unsigned char destination, data;
{
	register struct cpuslic *sl = va_slic;

	sl->sl_dest = destination;
	sl->sl_smessage = data;
	sl->sl_cmd_stat = SL_WRDATA;
	while (sl->sl_cmd_stat & SL_BUSY)
		continue;
#ifdef	CHECKSLIC
	check_slic("wrData");
#endif	CHECKSLIC
}

/*
 * rdData()
 *	Read data from previously addressed slave register.
 *
 * Internal interface.
 *
 * Caller assures mutex of SLIC usage (splhi() or holding gate is sufficient).
 */

int
rdData(destination)
	unsigned char destination;
{
	register struct cpuslic *sl = va_slic;

	sl->sl_dest = destination;
	sl->sl_cmd_stat = SL_RDDATA;
	while (sl->sl_cmd_stat & SL_BUSY)
		continue;
#ifdef	CHECKSLIC
	check_slic("rdData");
#endif	CHECKSLIC
	return(sl->sl_sdr & 0xff);
}

/*
 * sendsoft()
 *	Post SW interrupt to somebody.
 *
 * Used to post resched "nudge", net handler interrupts, pff calc, softclock.
 *
 * Caller assures mutex of SLIC usage (splhi() or holding gate is sufficient).
 */

sendsoft(dest, bitmask)
	unsigned char dest;
	unsigned char bitmask;
{
	register struct cpuslic *sl = va_slic;

	sl->sl_dest = dest;
	sl->sl_smessage = bitmask;

	sl->sl_cmd_stat = SL_MINTR | 0;		/* 0 ==> bin 0 */
	while (sl->sl_cmd_stat & SL_BUSY)
		continue;

#ifdef	CHECKSLIC
	check_slic("sendsoft");
#endif	CHECKSLIC
}

/*
 * nmIntr()
 *	Post NMI interrupt to somebody.
 *
 * Used to post send NMI to a processor to have it shut down.
 *
 * Caller assures mutex of SLIC usage (splhi() or holding gate is sufficient).
 */

nmIntr(dest, message)
	unsigned char dest;
	unsigned char message;
{
	register struct cpuslic *sl = va_slic;

	sl->sl_dest = dest;
	sl->sl_smessage = message;

	do {
		sl->sl_cmd_stat = SL_NMINTR;
		while (sl->sl_cmd_stat & SL_BUSY)
			continue;
	} while ((sl->sl_cmd_stat & SL_OK) == 0);
#ifdef	CHECKSLIC
	check_slic("nmIntr");
#endif	CHECKSLIC
}

/*
 * mIntr()
 *	Post HW interrupt to somebody.
 *
 * Used to send commands to MBAd's.
 *
 * Caller assures mutex of SLIC usage (splhi() or holding gate is sufficient).
 *
 * Implementation acquires gate before sending message, to avoid SLIC bus
 * saturation.
 */

#ifdef	PERFSTAT
#define	MINTR_MAXHIST	65
int	mintr_hist[MINTR_MAXHIST];
#endif	PERFSTAT

mIntr(dest, bin, data)
	unsigned char dest;
	unsigned char bin;
	unsigned char data;
{
	register struct cpuslic *sl = va_slic;
	register unsigned stat;
#ifdef	PERFSTAT
	register int mintr_cnt = 0;
#endif	PERFSTAT

	/*
	 * Send message.  Spin forever until sent.
	 */

	sl->sl_dest = dest;
	sl->sl_smessage = data;

	do {
#ifdef	PERFSTAT
		++mintr_cnt;
#endif	PERFSTAT
		sl->sl_cmd_stat = SL_MINTR | bin;
		while ((stat = sl->sl_cmd_stat) & SL_BUSY)
			continue;
	} while ((stat & SL_OK) == 0);

#ifdef	PERFSTAT
	if (mintr_cnt > MINTR_MAXHIST)
		mintr_cnt = MINTR_MAXHIST;
	++mintr_hist[mintr_cnt-1];
#endif	PERFSTAT

#ifdef	CHECKSLIC
	check_slic("mIntr");
#endif	CHECKSLIC

	/*
	 * Release mIntr gate.
	 */

}

/*
 * setgm()
 *	Set group mask in destination slic.
 *
 * Caller assures mutex of SLIC usage (splhi() or holding gate is sufficient).
 */

setgm(dest, mask)
	unsigned char dest;
	unsigned char mask;
{
	register struct cpuslic *sl = va_slic;

	sl->sl_dest = dest;			/* set this guy's... */
	sl->sl_smessage = mask;			/* group mask to "mask" */
	sl->sl_cmd_stat = SL_SETGM;		/* set the group-mask */
	while (sl->sl_cmd_stat & SL_BUSY)	/* and wait for cmd done */
		continue;

#ifdef	CHECKSLIC
	check_slic("setgm");
#endif	CHECKSLIC
}

/*
 * lock_subslave(), unlock_subslave()
 *	Mutex access to certain slic/sub-slave combinations which
 *	may be shared (eg, processor BIC -- both processors access
 *	the BIC via only one of their slic id's).
 *
 * Other uses of slic slave addressing is mutually excluded at higher levels;
 * upper level code only online/offline's one processor at a time, memory
 * error polling guaranteed single-thread, etc.
 */

spl_t
lock_subslave(slic, slave)
	unsigned char slic, slave;
{
	return 0;
}

void
unlock_subslave(slic, slave, s)
	unsigned char slic, slave;
	spl_t	s;
{
}

#ifdef	CHECKSLIC
/* 
 * check_slic()
 *	Check status from SLIC for parity, exists, and ok bits.
 */

check_slic(procname)
	char	*procname;
{
	register unsigned stat;

	stat = va_slic->sl_cmd_stat;

	if ((stat & SL_PARITY) == 0) {
		printf("%s: slic parity error\n", procname);
		panic("slic");
		/*NOTREACHED*/
	}
	if ((stat & SL_EXISTS) == 0) {
		printf("%s: slic(s) don't exist\n", procname);
		panic("slic");
		/*NOTREACHED*/
	}
	if ((stat & SL_OK) == 0) {
		printf("%s: slic not ok\n", procname);
		panic("slic");
		/*NOTREACHED*/
	}
}
#endif	CHECKSLIC
