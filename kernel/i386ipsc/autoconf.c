/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * Copyright 1988, 1989, 1990, 1991 by Intel Corporation,
 * Santa Clara, California.
 * 
 *                          All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and that
 * both the copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Intel not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 * 
 * INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
 * SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN ACTION OF CONTRACT, NEGLIGENCE, OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */
/*
 * HISTORY
 * $Log:	autoconf.c,v $
 * Revision 2.7  92/04/01  19:32:38  rpd
 * 	Renamed gets to safe_gets.
 * 	[92/03/31            rpd]
 * 
 * Revision 2.6  91/08/03  18:17:47  jsb
 * 	Eliminated call to ask_bootstrap_server.
 * 	Only call ask_root_device if RB_ASKNAME.
 * 	Split dcm_init call into dcm_init_{send,recv}.
 * 	[91/07/25  18:38:51  jsb]
 * 
 * Revision 2.5  91/07/01  08:24:08  jsb
 * 	Call ask_bootstrap_server.
 * 	[91/06/29  16:39:11  jsb]
 * 
 * Revision 2.4  91/06/18  20:50:00  jsb
 * 	New copyright from Intel.
 * 	[91/06/18  20:05:31  jsb]
 * 
 * Revision 2.3  91/06/06  17:04:20  jsb
 * 	Added implementation and call to ask_root_device.
 * 	[91/05/13  16:59:05  jsb]
 * 
 * Revision 2.2.1.1  91/01/07  11:03:51  jsb
 * 	Added implementation and call to ask_root_device.
 * 
 * Revision 2.2  90/12/04  14:46:57  jsb
 * 	First checkin.
 * 	[90/12/04  10:55:12  jsb]
 * 
 */ 

#include <sys/types.h>
#include <sys/reboot.h>
#include <i386at/atbus.h>

#include <cnp.h>
#if NCNP > 0
extern	struct	isa_dev	cnpinfo[];
#endif NCNP

/* overrides hard-coded root_name used by get_root_device() */
ask_root_device()
{
	static char root[64];
	extern char *root_name;

	printf("root device? [%s] ", root_name);
	safe_gets(root, sizeof(root));
	if (root[0] != '\0') {
		root_name = root;
	}
}

probeio()
{
	int	i;
	struct	isa_dev	*dev_p;
	struct	isa_driver *drv_p;

	if (boothowto & RB_ASKNAME) {
		ask_root_device();
	}
#if NCNP > 0
	for (i = 0; i < NCNP; i++) {
		dev_p = &cnpinfo[i];
		drv_p = dev_p->dev_driver;

		if (drv_p->driver_probe(dev_p->dev_addr, i)) {
			printf("cnp%d at 0x%x\n", i, dev_p->dev_addr);
			dev_p->dev_alive = 1;
			drv_p->driver_attach(dev_p);
		}
	}
#endif NCNP

	/*
	 * Initialize the DCM routing hardware.
	 */
	dcm_init_send();
	dcm_init_recv();
}
