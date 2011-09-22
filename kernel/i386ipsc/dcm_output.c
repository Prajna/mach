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
 * $Log:	dcm_output.c,v $
 * Revision 2.6  91/08/28  11:12:00  jsb
 * 	Moved shortword count support here from norma/ipc_net.c.
 * 	[91/08/26  14:04:28  jsb]
 * 
 * Revision 2.5  91/08/03  18:18:01  jsb
 * 	Added setting of size field in dcm_header for i860ipsc support.
 * 	[91/07/27  19:04:34  jsb]
 * 
 * 	Moved all device independent code to norma/ipc_net.c.
 * 	[91/07/25  18:30:44  jsb]
 * 
 * Revision 2.4  91/06/18  20:50:16  jsb
 * 	New copyright from Intel.
 * 	[91/06/18  20:06:21  jsb]
 * 
 * Revision 2.3  91/06/17  15:44:09  jsb
 * 	Changed include of renamed norma file.
 * 	[91/06/17  10:07:18  jsb]
 * 
 * Revision 2.2  91/06/06  17:04:32  jsb
 * 	First checkin.
 * 	[91/05/14  13:27:30  jsb]
 * 
 */ 

#include <norma/ipc_net.h>
#include <ipsc/dcmcom.h>
#include <i386ipsc/dcm.h>

struct adma_chan	dcm_send_chan_1;
struct adma_chan	dcm_send_chan_2;
struct dcm_header	dcm_send_header;
int			ipsc_route;


/*
 * Called when the dcm is initialized.
 */
dcm_init_send()
{
	bzero(&dcm_send_header, sizeof(dcm_send_header));
	dcm_send_header.type = DCM_HDR_TYPE_MACH;

	/*
	 * Initialize dcm_send_chan_1
	 */
	dcm_send_chan_1.src1 = kvtophys(&dcm_send_header) >> 1;
	dcm_send_chan_1.cnt1 = sizeof(dcm_send_header) >> 1;
	dcm_send_chan_1.cmd1 = SENDCMD;
	dcm_send_chan_1.cmd2 = SENDCMD;
	dcm_send_chan_1.cmd3 = SENDCMD;
	dcm_send_chan_1.cmd4 = STOPCMD | EOD;

	/*
	 * Initialize dcm_send_chan_2
	 */
	dcm_send_chan_2.src1 = kvtophys(&dcm_send_header) >> 1;
	dcm_send_chan_2.cnt1 = sizeof(dcm_send_header) >> 1;
	dcm_send_chan_2.cmd1 = SENDCMD;
	dcm_send_chan_2.cmd1 = SENDCMD;
	dcm_send_chan_2.cmd2 = SENDCMD;
	dcm_send_chan_2.cmd3 = SENDCMD;
	dcm_send_chan_2.cmd4 = SENDCMD;
	dcm_send_chan_2.cmd5 = STOPCMD | EOD;

	/*
	 * Initialize routing info
	 */
	ipsc_route = node_self() & 0x7f;
}

netipc_send(remote, vec, count)
	int remote;
	register struct netvec *vec;
	int count;
{
	int s;

	dcm_send_header.route = remote ^ ipsc_route;
	if (count == 2) {
		dcm_send_chan_1.src2 = vec[0].addr >> 1;
		dcm_send_chan_1.cnt2 = vec[0].size >> 1;
		dcm_send_chan_1.src3 = vec[1].addr >> 1;
		dcm_send_chan_1.cnt3 = vec[1].size >> 1;
		dcm_send_header.size = vec[0].size + vec[1].size;
		s = sploff();
		dcm_send_1();
		splon(s);
	} else if (count == 3) {
		dcm_send_chan_2.src2 = vec[0].addr >> 1;
		dcm_send_chan_2.cnt2 = vec[0].size >> 1;
		dcm_send_chan_2.src3 = vec[1].addr >> 1;
		dcm_send_chan_2.cnt3 = vec[1].size >> 1;
		dcm_send_chan_2.src4 = vec[2].addr >> 1;
		dcm_send_chan_2.cnt4 = vec[2].size >> 1;
		dcm_send_header.size = vec[0].size + vec[1].size + vec[2].size;
		s = sploff();
		dcm_send_2();
		splon(s);
	} else {
		panic("netipc_send: bad count=%d\n", count);
	}
}

dcm_send_intr()
{
	netipc_send_intr();
}

netipc_network_init()
{
}
