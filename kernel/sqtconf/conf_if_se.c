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
 * $Log:	conf_if_se.c,v $
 * Revision 2.3  91/07/31  18:05:19  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:02:17  dbg
 * 	Adapted for pure Mach kernel.
 * 	[90/10/05            dbg]
 * 
 */

#ifndef lint
static char rcsid[] = "$Header: conf_if_se.c,v 2.3 91/07/31 18:05:19 dbg Exp $";
#endif lint

/*
 * if_se_conf.c
 *
 * This file contains the binary configuration data for the
 * Ether interface driver for SCSI/Ether.
 */

/*
 * Revision 1.1  89/07/05  13:17:37  kak
 * Initial revision
 * 
 */

#include <device/if_ether.h>

int se_watch_interval = 10;	/* seconds between stats collection */
int se_write_iats = 5;		/* number of write IATs; 5 should be OK */
int se_bin = 5;			/* bin number for mIntr to interrupt SEC */
short se_mtu = ETHERMTU;	/* max transfer unit for se transmit */

#ifdef DEBUG
int se_ibug = 0;		/* input debug level */
int se_obug = 0;		/* output debug level */
#endif DEBUG
