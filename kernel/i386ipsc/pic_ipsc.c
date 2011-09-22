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
 * $Log:	pic_ipsc.c,v $
 * Revision 2.6  91/08/03  18:18:10  jsb
 * 	Added cnpintr.
 * 	[91/07/17  13:59:08  jsb]
 * 
 * Revision 2.5  91/07/31  17:43:17  dbg
 * 	Changed hardclock for stack_switching support.
 * 	[91/07/29            dbg]
 * 
 * Revision 2.4  91/06/18  20:50:26  jsb
 * 	New copyright from Intel.
 * 	[91/06/18  20:07:09  jsb]
 * 
 * Revision 2.3  91/06/06  17:04:53  jsb
 * 	Hang cnppoll off of hardclock (since cnp doesn't get interrupts).
 * 	[91/05/13  17:09:57  jsb]
 * 
 * Revision 2.2  90/12/04  14:47:32  jsb
 * 	First checkin.
 * 	[90/12/04  10:57:23  jsb]
 * 
 */

#include "cnp.h"

#include <sys/types.h>
#include <i386/ipl.h>
#include <i386/pic.h>

#if	NCNP > 0
#define	hardclock	fakehardclock
extern int cnpintr();
#endif	NCNP > 0

/* These interrupts are always present */
extern intnull(), hardclock();
extern dcm_send_intr(), dcm_recv_intr(), usmintr(), sdintr();

int (*ivect[NINTR])() = {
	/*00*/		intnull,        /* parity error */
	/*01*/		intnull,        /* bus timeout */
	/*02*/ 		hardclock,      /* timer 0 */
	/*03*/		dcm_send_intr,  /* EOD 0, DCM send */
	/*04*/		intnull,        /* EOD 1 not used */
	/*05*/		intnull,        /* DCM error */
	/*06*/		intnull,        /* grounded */
	/*07*/		intnull,        /* slave interrupt */
	/*08*/		dcm_recv_intr,  /* EOD 2, DCM receive */
	/*09*/		intnull,        /* EOD 3, not used */
	/*10*/		intnull,        /* timer 2 */
	/*11*/		intnull,        /* timer 1 */
	/*12*/		usmintr,        /* serial port */
	/*13*/		cnpintr,        /* cnp ethernet */
	/*14*/		sdintr,         /* SX/SCSI interrupt */
	/*15*/		intnull,	/* ??? */
};
	
u_char intpri[NINTR] = {
	/* 00 */	0, 	0,	SPLHI,	SPL5,
	/* 04 */	0,	0,	0,	0,
	/* 08 */	SPL5,	0,	0,	0,
	/* 12 */	SPLTTY,	SPLHI,	SPL5,	0,
};
	
#if	NCNP > 0
#include <i386/thread.h>

hardclock(ivect, old_ipl, ret_addr, regs)
	int ivect;
	int old_ipl;
	char *ret_addr;
	struct i386_saved_state regs;
{
#undef	hardclock
	hardclock(ivect, old_ipl, ret_addr, regs);
	cnppoll();
}
#endif	NCNP > 0
