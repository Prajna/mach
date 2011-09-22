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
 * $Log:	dcm_route.s,v $
 * Revision 2.5  91/08/03  18:18:04  jsb
 * 	Removed dcm_send_k, dcm_send_a. Changed send_chan_* to dcm_send_chan_*.
 * 	[91/07/24  23:18:30  jsb]
 * 
 * 	Changed for rename of recv_chan to dcm_recv_chan.
 * 	[91/07/17  23:26:00  jsb]
 * 
 * 	Added dcm_send_a; added outb_dx, outw_dx, outb_i macros.
 * 	[91/07/17  13:58:26  jsb]
 * 
 * Revision 2.4  91/06/18  20:50:18  jsb
 * 	New copyright from Intel.
 * 	[91/06/18  20:06:30  jsb]
 * 
 * Revision 2.3  91/06/06  17:04:36  jsb
 * 	Support for new dcm code.
 * 	[91/05/13  17:01:17  jsb]
 * 
 * Revision 2.2  90/12/04  14:47:11  jsb
 * 	First checkin.
 * 	[90/12/04  10:56:16  jsb]
 * 
 */ 

#include <i386/asm.h>
#include <i386ipsc/port.h>

#define	outb_dx	.byte 0xee
#define	outw_dx	.byte 0xef
#define	outb_i	.byte 0xe6,

/*
 * dcm_send_1():
 * Start ADMA send with dcm_send_chan_1
 */
ENTRY(dcm_send_1)
	mov	$CPR0L_PORT, %edx
	mov	$_dcm_send_chan_1 + 2, %eax
	outw_dx
	mov	$CPR0H_PORT, %edx
	shr	$16, %eax
	outw_dx
	mul	%eax
	mov	$GCR_PORT, %edx
	movb	$START_CHAN0, %al
	outb_dx
	ret

/*
 * dcm_send_2():
 * Start ADMA send with dcm_send_chan_2
 */
ENTRY(dcm_send_2)
	mov	$CPR0L_PORT, %edx
	mov	$_dcm_send_chan_2 + 2, %eax
	outw_dx
	mov	$CPR0H_PORT, %edx
	shr	$16, %eax
	outw_dx
	mul	%eax
	mov	$GCR_PORT, %edx
	movb	$START_CHAN0, %al
	outb_dx
	ret

/*
 * dcm_recv():
 * Start ADMA recv with dcm_recv_chan
 */
ENTRY(dcm_recv)
	mov	$CPR2L_PORT, %edx
	mov	$_dcm_recv_chan + 2, %eax
	outw_dx
 
	movb	$RLSRCV | NTRESET | NXRESET | NPROGRAM, %al
	outb_i	DCC_PORT
 
	mov	$CPR2H_PORT, %edx
	shr	$16, %eax
	outw_dx
 
	movb	$NTRESET | NXRESET | NPROGRAM, %al
	outb_i	DCC_PORT
 
	mov	$GCR_PORT, %edx
	movb	$START_CHAN2, %al
	outb_dx
	ret
