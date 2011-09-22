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
 * $Log:	port.h,v $
 * Revision 2.3  91/06/18  20:50:28  jsb
 * 	New copyright from Intel.
 * 	[91/06/18  20:07:23  jsb]
 * 
 * Revision 2.2  90/12/04  14:47:35  jsb
 * 	First checkin.
 * 	[90/12/04  10:57:43  jsb]
 * 
 */ 
 
/* Port addresses for the DCL */
 
#define DCC_PORT 0x90
#define DCS_PORT 0x98
 
/* Port addresses for the ADMA */
 
#define GSR_PORT 0x204
#define GMR_PORT 0x208
#define GCR_PORT 0x200
#define GBR_PORT 0x20A
#define GDR_PORT 0x20C
#define CPR0L_PORT 0x220
#define CPR0H_PORT 0x222
#define SPR0L_PORT 0x224
#define SPR0H_PORT 0x226
#define DPR0L_PORT 0x228
#define DPR0H_PORT 0x22A
#define TTPR0L_PORT 0x22C
#define TTPR0H_PORT 0x22E
#define LPR0L_PORT 0x230
#define LPR0H_PORT 0x232
#define BCR0L_PORT 0x238
#define BCR0H_PORT 0x23A
#define CCR0L_PORT 0x23C
#define CCR0H_PORT 0x23E
#define MASKR0_PORT 0x214
#define CMPR0_PORT 0x216
#define DAR0_PORT 0x212
#define CSR0_PORT 0x210
#define CPR2L_PORT 0x2A0
#define CPR2H_PORT 0x2A2
#define SPR2L_PORT 0x2A4
#define SPR2H_PORT 0x2A6
#define DPR2L_PORT 0x2A8
#define DPR2H_PORT 0x2AA
#define TTPR2L_PORT 0x2AC
#define TTPR2H_PORT 0x2AE
#define LPR2L_PORT 0x2B0
#define LPR2H_PORT 0x2B2
#define BCR2L_PORT 0x2B8
#define BCR2H_PORT 0x2BA
#define CCR2L_PORT 0x2BC
#define CCR2H_PORT 0x2BE
#define MASKR2_PORT 0x294
#define CMPR2_PORT 0x296
#define DAR2_PORT 0x292
#define CSR2_PORT 0x290
 
#define ADMA_MODE 0x3CF3
#define START_CHAN0 0x12
#define CONT_CHAN2 0x41
#define START_CHAN2 0x42
#define STOP_CHAN2 0x44
 
#define NPGM_DONE 0x01
#define DATABITS 0x0F
#define RLSRCV 0x02
#define NXRESET 0x10
#define NTRESET 0x20
#define CCLOCK 0x40
#define NPROGRAM 0x80
