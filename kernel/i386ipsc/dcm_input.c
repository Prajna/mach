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
 * $Log:	dcm_input.c,v $
 * Revision 2.9  93/05/15  19:33:13  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.8  93/01/14  17:31:59  danner
 * 	Proper spl typing.
 * 	[92/12/10  17:52:00  af]
 * 
 * Revision 2.7  91/08/28  11:11:58  jsb
 * 	Moved shortword count support here from norma/ipc_net.c.
 * 	[91/08/26  14:03:39  jsb]
 * 
 * Revision 2.6  91/08/03  18:17:55  jsb
 * 	Moved all device independent code to norma/ipc_net.c.
 * 	[91/07/25  18:30:13  jsb]
 * 
 * Revision 2.5  91/07/01  08:24:11  jsb
 * 	Use clport_page_grab instead of vm_page_grab.
 * 	Removed bogus null definition of clport_replenish.
 * 	[91/06/29  16:08:08  jsb]
 * 
 * Revision 2.4  91/06/18  20:50:14  jsb
 * 	New copyright from Intel.
 * 	[91/06/18  20:06:09  jsb]
 * 
 * Revision 2.3  91/06/17  15:44:07  jsb
 * 	Changed include of renamed norma file. Changed call to vm_page_init
 * 	to match its redefinition (it lost two parameters). 
 * 	[91/06/17  10:06:40  jsb]
 * 
 * Revision 2.2  91/06/06  17:04:28  jsb
 * 	First checkin.
 * 	[91/05/14  13:27:06  jsb]
 * 
 */

#include <norma/ipc_net.h>
#include <ipsc/dcmcom.h>
#include <i386ipsc/dcm.h>
#include <machine/machspl.h>

struct adma_chan	dcm_recv_chan;
struct dcm_header	dcm_recv_header;

dcm_init_recv()
{
	/*
	 * Initialize unchanging elements of dcm_recv_chan
	 */
	bzero(&dcm_recv_chan, sizeof(dcm_recv_chan));
	dcm_recv_chan.src1 = kvtophys(&dcm_recv_header) >> 1;
	dcm_recv_chan.cnt1 = sizeof(dcm_recv_header) >> 1;
	dcm_recv_chan.cmd1 = RECVCMD;
	dcm_recv_chan.cmd2 = RECVCMD;
	dcm_recv_chan.cmd3 = RECVCMD;
	dcm_recv_chan.cmd4 = STOPCMD;
}

netipc_recv(vec, count)
	register struct netvec *vec;
	int count;
{
	spl_t s;

	if (count != 2) {
		panic("netipc_recv");
	}
	dcm_recv_chan.src2 = vec[0].addr >> 1;
	dcm_recv_chan.cnt2 = vec[0].size >> 1;
	dcm_recv_chan.src3 = vec[1].addr >> 1;
	dcm_recv_chan.cnt3 = vec[1].size >> 1;
	s = sploff();
	dcm_recv();
	splon(s);
}

dcm_recv_intr()
{
	spl_t s;

	if (dcm_recv_header.type == DCM_HDR_TYPE_MACH) {
		netipc_recv_intr();
	} else {
		printf("dcm_recv_intr: bad type 0x%x\n", dcm_recv_header.type);
		s = sploff();
		dcm_recv();
		splon(s);
	}
}
