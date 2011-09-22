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
 * $Log:	dcm.h,v $
 * Revision 2.4  91/08/03  18:17:53  jsb
 * 	Removed PHYSADDR definition.
 * 	[91/07/24  22:59:34  jsb]
 * 
 * Revision 2.3  91/06/18  20:50:12  jsb
 * 	New copyright from Intel.
 * 	[91/06/18  20:06:00  jsb]
 * 
 * Revision 2.2  91/06/06  17:04:24  jsb
 * 	First checkin.
 * 	[91/05/14  13:24:02  jsb]
 * 
 */ 

/* Adma programming commands */
#define SENDCMD		0x800D
#define RECVCMD		0x500D
#define CANCMD		0x5009
#define STOPCMD		0x0000
#define JUMPCMD		0x200F
#define NOPCMD		0x2000
#define LISTCTL		0x0600
#define EOD		0x0800

struct adma_chan {		/* Adma program body */
	unsigned short	fill1;

	unsigned short	cmd1;		/* Adma transfer command */
	unsigned long	src1;		/* physical address of source data */
	unsigned long	dst1;
	unsigned long	cnt1;		/* data count */
	unsigned short	stat1;

	unsigned short	cmd2;
	unsigned long	src2;
	unsigned long	dst2;
	unsigned long	cnt2;
	unsigned short	stat2;

	unsigned short	cmd3;
	unsigned long	src3;
	unsigned long	dst3;
	unsigned long	cnt3;
	unsigned short	stat3;

	unsigned short	cmd4;
	unsigned long	src4;
	unsigned long	dst4;
	unsigned long	cnt4;
	unsigned short	stat4;

	unsigned short	cmd5;
	unsigned long	fill5;
};
