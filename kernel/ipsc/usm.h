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
 *  Copyright 1988, 1989 by Olivetti Advanced Technology Center, Inc.,
 *  Cupertino, California.
 * 
 * 		All Rights Reserved
 * 
 *   Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appears in all
 * copies and that both the copyright notice and this permission notice
 * appear in supporting documentation, and that the name of Olivetti
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 * 
 *   OLIVETTI DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL OLIVETTI BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUR OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * HISTORY
 * $Log:	usm.h,v $
 * Revision 2.4  91/08/28  11:14:11  jsb
 * 	From Intel SSD: added some i860 UART register definitions.
 * 	[91/08/27  17:21:00  jsb]
 * 
 * Revision 2.3  91/06/18  20:52:57  jsb
 * 	New copyright from Intel.
 * 	[91/06/18  19:06:07  jsb]
 * 
 * Revision 2.2  90/12/04  14:50:57  jsb
 * 	First checkin.
 * 	[90/12/03  21:58:20  jsb]
 * 
 */

#define MAX_COM_PORTS 	1
#define COM_PORTS	1
#define MAXTIME         2               /* 2 sec */

/* line control register */
#define		iWLS0	0x01		/*word length select bit 0 */	
#define		iWLS1	0x02		/*word length select bit 2 */	
#define		iSTB	0x04		/* number of stop bits */
#define		iPEN	0x08		/* parity enable */
#define		iEPS	0x10		/* even parity select */
#define		iSP	0x20		/* stick parity */
#define		iSETBREAK 0x40		/* break key */
#define		iDLAB	0x80		/* divisor latch access bit */
#define		i5BITS	0x00		/* 5 bits per char */
#define		i6BITS	0x01		/* 6 bits per char */
#define		i7BITS	0x02		/* 7 bits per char */
#define		i8BITS	0x03		/* 8 bits per char */

/* line status register */
#define		iDR	0x01		/* data ready */
#define		iOR	0x02		/* overrun error */
#define		iPE	0x04		/* parity error */
#define		iFE	0x08		/* framing error */
#define		iBRKINTR 0x10		/* a break has arrived */
#define		iTHRE	0x20		/* tx hold reg is now empty */
#define		iTSRE	0x40		/* tx shift reg is now empty */

/* interrupt id register */
#define		iMODEM_INTR	0x01
#define		iTX_INTR	0x02
#define		iRX_INTR	0x04
#define		iERROR_INTR	0x08

/* bank select register */
#define		iBANK0		0x00
#define		iBANK1		0x20
#define		iBANK2		0x40
#define		iBANK3		0x60

/* interrupt enable register */
#define		iRX_ENAB	0x01
#define		iTX_ENAB	0x02
#define		iERROR_ENAB	0x04
#define		iMODEM_ENAB	0x08

/* modem control register */
#define		iDTR		0x01	/* data terminal ready */
#define		iRTS		0x02	/* request to send */
#define		iOUT1		0x04	/* COM aux line -not used */
#define		iOUT2		0x08	/* turns intr to 386 on/off */	
#define		iLOOP		0x10	/* loopback for diagnostics */

/* modem status register */
#define		iDCTS		0x01	/* delta clear to send */
#define		iDDSR		0x02	/* delta data set ready */
#define		iTERI		0x04	/* trail edge ring indicator */
#define		iDRLSD		0x08	/* delta rx line sig detect */
#define		iCTS		0x10	/* clear to send */
#define		iDSR		0x20	/* data set ready */
#define		iRI		0x40	/* ring indicator */
#define		iRLSD		0x80	/* rx line sig detect */


/*
 * UART Registers
 */

#if	i860
/* Values from i860ipsc/nodehw.h */
#define iUSM_REG0	UART_REG0
#define iUSM_REG1	UART_REG1
#define iUSM_REG2	UART_REG2
#define iUSM_REG3	UART_REG3
#define iUSM_REG4	UART_REG4
#define iUSM_REG5	UART_REG5
#define iUSM_REG6	UART_REG6
#define iUSM_REG7	UART_REG7
#else	i860
#define iUSM_REG0	0xa0
#define iUSM_REG1	0xa2
#define iUSM_REG2	0xa4
#define iUSM_REG3	0xa6
#define iUSM_REG4	0xa8
#define iUSM_REG5	0xaa
#define iUSM_REG6	0xac
#define iUSM_REG7	0xae
#endif	i860

/* Bank 0 */
#define iUSM_RXD	iUSM_REG0
#define iUSM_TXD	iUSM_REG0
#define iUSM_BAL	iUSM_REG0
#define iUSM_BAH	iUSM_REG1
#define iUSM_GER	iUSM_REG1
#define iUSM_BANK	iUSM_REG2
#define iUSM_GIR	iUSM_REG2
#define iUSM_LCR	iUSM_REG3
#define iUSM_MCR	iUSM_REG4
#define iUSM_LSR	iUSM_REG5
#define iUSM_MSR	iUSM_REG6
#define iUSM_ACR0	iUSM_REG7

/* Bank 1 */
#define iUSM_RXF	iUSM_REG1
#define iUSM_TXF	iUSM_REG1
#define iUSM_TMST	iUSM_REG3
#define iUSM_TMCR	iUSM_REG3
#define iUSM_FLR	iUSM_REG4
#define iUSM_RST	iUSM_REG5
#define iUSM_RCM	iUSM_REG5
#define iUSM_TCM	iUSM_REG6
#define iUSM_GSR	iUSM_REG7
#define iUSM_ICM	iUSM_REG7

/* Bank 2 */
#define iUSM_FMD	iUSM_REG1
#define iUSM_TMD	iUSM_REG3
#define iUSM_IMD	iUSM_REG4
#define iUSM_ACR1	iUSM_REG5
#define iUSM_RIE	iUSM_REG6
#define iUSM_RMD	iUSM_REG7

/* Bank 3 */
#define iUSM_CLCF	iUSM_REG0
#define iUSM_BACF	iUSM_REG1
#define iUSM_BBL	iUSM_REG0
#define iUSM_BBH	iUSM_REG1
#define iUSM_BBCF	iUSM_REG3
#define iUSM_PMD	iUSM_REG4
#define iUSM_MIE	iUSM_REG5
#define iUSM_TMIE	iUSM_REG6
