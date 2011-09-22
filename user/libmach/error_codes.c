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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	error_codes.c,v $
 * Revision 2.4  92/04/01  19:38:09  rpd
 * 	Added err_bootstrap.
 * 	[92/03/09            rpd]
 * 
 * Revision 2.3  92/01/23  15:21:55  rpd
 * 	Changed <servers/errorlib.h> to <errorlib.h>.
 * 	[92/01/16            rpd]
 * 
 * Revision 2.2  92/01/16  00:08:27  rpd
 * 	Moved from user collection to mk collection.
 * 
 * Revision 2.2  91/03/27  16:06:18  mrt
 * 	Changed include of "errorlib.h" to <servers/errorlib.h>
 * 	Added new copyright
 * 	[91/03/20            mrt]
 * 
 */
/*
 *	File:	error_codes.c
 *	Author:	Douglas Orr, Carnegie Mellon University
 *	Date:	Mar, 1988
 *
 *      Generic error code interface
 */

#include <mach/error.h>
#include <errorlib.h>
#include "err_kern.sub"
#include "err_us.sub"
#include "err_server.sub"
#include "err_ipc.sub"
#include "err_mach_ipc.sub"
#include "err_bootstrap.sub"

struct error_system errors[err_max_system+1] = {
	/* 0; err_kern */
	{
		errlib_count(err_os_sub),
		"(operating system/?) unknown subsystem error",
		err_os_sub,
	},
	/* 1; err_us */
	{
		errlib_count(err_us_sub),
		"(user space/?) unknown subsystem error",
		err_us_sub,
	},
	/* 2; err_server */
	{
		errlib_count(err_server_sub),
		"(server/?) unknown subsystem error",
		err_server_sub,
	},
	/* 3 (& 3f); err_ipc */
	{
		errlib_count(err_ipc_sub),
		"(ipc/?) unknown subsystem error",
		err_ipc_sub,
	},
	/* 4; err_mach_ipc */
	{
		errlib_count(err_mach_ipc_sub),
		"(ipc/?) unknown subsystem error",
		err_mach_ipc_sub,
	},
	/* 5; err_bootstrap */
	{
		errlib_count(err_bootstrap_sub),
		"(ipc/?) unknown subsystem error",
		err_bootstrap_sub,
	},
};


int error_system_count = errlib_count(errors);
