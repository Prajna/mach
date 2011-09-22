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
 * $Log:	conf_co.c,v $
 * Revision 2.3  91/07/31  18:05:08  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:02:05  dbg
 * 	Adapted for pure Mach kernel.
 * 	[91/04/26  15:02:33  dbg]
 * 
 */

#ifndef lint
static char rcsid[]= "$Header: conf_co.c,v 2.3 91/07/31 18:05:08 dbg Exp $";
#endif lint

/*
 * co_conf.c
 *
 * This file contains the binary configuration data for the
 * SCSI/Ether console driver
 */

/*
 * Revision 1.1  89/07/05  13:17:36  kak
 * Initial revision
 * 
 */

#include <device/tty.h>

#include <sqtsec/co.h>


struct co_bin_config co_bin_config[] = {
	/*gate	*/
	{ 54,	},
	{ 54,	},
	{ 54,	},
	{ 54,	},
	{ 54,	},
	{ 54,	},
	{ 54,	},
	{ 54,	},
};

int co_bin_config_count = (sizeof co_bin_config)/(sizeof co_bin_config[0]);

#ifdef	DEBUG
int co_debug = 0;
#endif	DEBUG

int coflags = EVENP|ECHO|XTABS|CRMOD;
char cospeed = B9600;
