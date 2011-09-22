/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	dz_defs.h,v $
 * Revision 2.8  93/05/10  20:07:37  rvb
 * 	Use Mach types.
 * 	[93/05/06  09:55:29  af]
 * 
 * Revision 2.7  93/03/26  17:57:53  mrt
 * 	Rid of dev_t.
 * 	[93/03/19            af]
 * 
 * Revision 2.6  91/06/19  11:47:29  rvb
 * 	File moved here from mips/PMAX since it tries to be generic;
 * 	it is used on the PMAX and the Vax3100.
 * 	[91/06/04            rvb]
 * 
 * Revision 2.5  91/05/14  17:20:59  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:40:31  mrt
 * 	Added author notices
 * 	[91/02/04  11:12:57  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:10:47  mrt]
 * 
 * Revision 2.3  90/12/05  23:31:00  af
 * 
 * 
 * Revision 2.1.1.1  90/11/01  03:37:34  af
 * 	Created.
 * 	[90/09/03            af]
 */
/*
 *	File: dz_defs.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	Internal definitions for the DZ Serial Line Driver
 */

#include <mach/std_types.h>
#include <chips/busses.h>

#include <kern/time_out.h>
#include <sys/syslog.h>

#include <device/io_req.h>
#include <device/conf.h>
#include <device/tty.h>
#include <device/errno.h>

#include <chips/dz_7085.h>

extern struct tty *dz_tty[];

extern struct pseudo_dma {
	dz_regmap_t	*p_addr;
	char		*p_mem;
	char		*p_end;
	int		p_arg;
	int		(*p_fcn)();
} dz_pdma[];

extern int rcline, cnline;
extern int	console;

/*
 * Modem control operations on DZ lines
 */

extern unsigned dz_mctl(/* int, int, int */);

