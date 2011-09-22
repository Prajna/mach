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
 * $Log:	slic.h,v $
 * Revision 2.3  91/07/31  18:03:55  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  12:59:37  dbg
 * 	Added volatile declarations.
 * 	[90/11/13            dbg]
 * 
 * 	Adapted for pure Mach kernel.  I386 SGS processors only.
 * 	[90/10/02            dbg]
 * 
 */

/*
 * $Header: slic.h,v 2.3 91/07/31 18:03:55 dbg Exp $
 *
 * This defines the offsets for the slic addresses and the base
 * address for accessing it.
 */

/*
 * Revision 1.1  89/07/19  14:48:55  kak
 * Initial revision
 * 
 * Revision 2.7  88/10/10  15:07:12  rto
 * Added 532-specific definitions in support of a common inter-project
 * build environment.
 * 
 * Revision 2.8  88/08/02  12:04:25  rto
 * 532port:  Added conditional definitions for LOAD_CPUSLICADDR.
 * 
 * Revision 2.7  88/07/28  10:05:47  rto
 * 532port:  LOAD__CPUSLICADDR is now set to PHYS_SLIC to support
 * 1-1 mapping of SLIC addresses for the 532.
 * 
 */

#ifndef	_SQT_SLIC_H_
#define	_SQT_SLIC_H_

#include <sys/types.h>

#ifndef __CHIPTYPES__
#include <sqt/chiptypes.h>
#endif

#include <sqt/vm_defs.h>

#ifdef	i386
#define LOAD_CPUSLICADDR	PHYS_SLIC	/* i386 uses virt==phys */
#endif	i386

#if defined(ns32000) && (CPU_TYPE == 32532)
#define LOAD_CPUSLICADDR	PHYS_SLIC	/* 532 uses virt==phys */
#endif ns32000

#if defined(ns32000) && (CPU_TYPE != 32532)
#define LOAD_CPUSLICADDR	0x0FF0000	/* 032 boot/loader uses this */
#endif	ns32000

struct	cpuslic {
  volatile u_char   sl_cmd_stat,d0[3];	/* RW W: command, R: status */
	   u_char   sl_dest,	d1[3];	/* W */
	   u_char   sl_smessage,d2[3];	/* W   send message data */
  volatile u_char   sl_b0int,	d3[3];	/* R   bin 0 interrupt */
  volatile u_char   sl_binint,	d4[3];	/* RW  bin 1-7 interrupt */
  volatile u_char   sl_nmiint,	d5[3];	/* R   NMI interrupt */
  volatile u_char   sl_lmask,	d6[3];	/* RW  local interrupt mask */
  volatile u_char   sl_gmask,	d7[3];	/* R   group interrupt mask */
  volatile u_char   sl_ipl,	d8[3];	/* RW  interrupt priority level */
  volatile u_char   sl_ictl,	d9[3];	/* RW  interrupt control */
  volatile u_char   sl_tcont,	d10[3];	/* RW  timer contents */
  volatile u_char   sl_trv,	d11[3];	/* RW  timer reload value */
	   u_char   sl_tctl,	d12[3];	/* W   timer control */
  volatile u_char   sl_sdr,	d13[3];	/* R   slave data register */
  volatile u_char   sl_procgrp,	d14[3];	/* RW  processor group */
  volatile u_char   sl_procid,	d15[3];	/* RW  processor id */
  volatile u_char   sl_crl,	d16[3];	/* R   chip revision level */
};

#define	NUMGATES	64		/* number of slic gates */

/*
 * MAX_NUM_SLIC is the maximum number of different slic addresses possible.
 * Slic addresses are 0 thru MAX_NUM_SLIC-1.
 */

#define	MAX_NUM_SLIC	64

/* Commands: */
#define	SL_MINTR	0x10	/* transmit maskable interrupt */
#define	SL_INTRACK	0x20	/* interrupt acknowledge */
#define	SL_SETGM	0x30	/* set group interrupt mask */
#define	SL_REQG		0x40	/* request Gate */
#define	SL_RELG		0x50	/* release Gate */
#define	SL_NMINTR	0x60	/* transmit non-maskable interrupt */
#define	SL_RDDATA	0x70	/* read slave data */
#define	SL_WRDATA	0x80	/* write slave data */
#define	SL_WRADDR	0x90	/* write slave I/O address */

/* Returned command status: */
#define	SL_BUSY		0x80	/* SLIC busy */
#define	SL_GATEFREE	0x10	/* Gate[send_message_data] free */
#define	SL_WRBE		0x08	/* Processor write buffer empty */
#define	SL_PARITY	0x04	/* parity error during SLIC message */
#define	SL_EXISTS	0x02	/* destination SLIC's exist */
#define	SL_OK		0x01	/* command completed ok */

/* Destination id's */
#define	SL_GROUP	0x40
#define	SL_ALL		0x3F

/* Interrupt control */
#define	SL_HARDINT	0x80	/* hardware interrupts accepted */
#define	SL_SOFTINT	0x40	/* software interrupts accepted */
#define	SL_MODEACK	0x01	/* interrupt acknowledge mode */

#define	SL_GM_ALLON	0xFF	/* Group Mask all enabled */

/* Timer interrupts */
#define	SL_TIMERINT	0x80	/* enable timer interrupts */
#define	SL_TIM5MHZ	0x08	/* decrement timer at 5 MHz */
#define	SL_TIMERBIN	0x07	/* interrupt bin mask of timer */
#define	SL_TIMERFREQ	10000	/* counts per second */
#define	SL_TIMERDIV	1000	/* system clock divisor for one clock count */

/* Processor ID */
#define	SL_TESTM	0x80	/* enable test mode */
#define	SL_PROCID	0x3F	/* processor ID mask */

/* Chip version stuff */
#define	SL_VENDOR	0xE0	/* vendor number */
#define	SL_RELEASE	0x1C	/* release number */
#define	SL_STEPPING	0x03	/* step number */

/*
 * "va_slic" is virtual-pointer to structured SLIC.
 */
#define va_slic ((struct cpuslic *)VA_SLIC)

/* 
 * SLICPRI() macro programs current execution priority into SLIC for
 * interrupt arbitration.  Argument is runQ # (eg, 0-31); thus, we
 * shift by 3 bits to get this into the writable portion of the register.
 */

#define	SLICPRI(p)	va_slic->sl_ipl = (p) << 3

#endif	/* _SQT_SLIC_H_ */
