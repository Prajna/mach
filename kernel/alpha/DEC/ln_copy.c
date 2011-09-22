/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
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
 * $Log:	ln_copy.c,v $
 * Revision 2.2  93/03/09  10:48:25  danner
 * 	Jeffrey Heller created this expounding from my mips code.
 * 	[93/03/06  14:26:06  af]
 * 
 */
/*
 *	File: ln_copy.c 
 *	Torn from: mips/PMAX/kn01.c and mips/PMAX/kn02ba.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	Routines specific to the KN01 processor (pmax)
 */

#include <mach/std_types.h>

/*
 *	Object:
 *		copyin_gap16			EXPORTED function
 *		copyout_gap16			EXPORTED function
 *		bzero_gap16			EXPORTED function
 *
 *	Specialized memory copy/zero for pmax-like I/O memory
 *	Copyin moves data from lance to host memory,
 *	Copyout the other way around and Bzero you know.
 *
 */
copyin_gap16(rbuf, dp, nbytes)
	register volatile int	*rbuf;
	register unsigned short	*dp;
	register unsigned	 nbytes;
{
	register int		 nshorts;

	/*
	 * Cannot use the normal bcopy since we are moving
	 * from Pmax space into LANCE space.
	 * Use the "Duff device" with a 16bits width.
	 */
	while (nbytes) {
	    switch (nshorts = (nbytes >> 1)) {
	 	default:
		case 32:
			 nshorts = 32;
			 dp[31] = rbuf[31];
	 	case 31: dp[30] = rbuf[30];
	 	case 30: dp[29] = rbuf[29];
	 	case 29: dp[28] = rbuf[28];
	 	case 28: dp[27] = rbuf[27];
	 	case 27: dp[26] = rbuf[26];
	 	case 26: dp[25] = rbuf[25];
	 	case 25: dp[24] = rbuf[24];
	 	case 24: dp[23] = rbuf[23];
	 	case 23: dp[22] = rbuf[22];
	 	case 22: dp[21] = rbuf[21];
	 	case 21: dp[20] = rbuf[20];
	 	case 20: dp[19] = rbuf[19];
	 	case 19: dp[18] = rbuf[18];
	 	case 18: dp[17] = rbuf[17];
	 	case 17: dp[16] = rbuf[16];
	 	case 16: dp[15] = rbuf[15];
	 	case 15: dp[14] = rbuf[14];
	 	case 14: dp[13] = rbuf[13];
	 	case 13: dp[12] = rbuf[12];
	 	case 12: dp[11] = rbuf[11];
	 	case 11: dp[10] = rbuf[10];
	 	case 10: dp[9] = rbuf[9];
	 	case 9: dp[8] = rbuf[8];
	 	case 8: dp[7] = rbuf[7];
	 	case 7: dp[6] = rbuf[6];
	 	case 6: dp[5] = rbuf[5];
	 	case 5: dp[4] = rbuf[4];
	 	case 4: dp[3] = rbuf[3];
	 	case 3: dp[2] = rbuf[2];
	 	case 2: dp[1] = rbuf[1];
	 	case 1: dp[0] = rbuf[0];
			break;
		case 0:
			/* Last byte.
			 * This really happens. Kinetic boxes, for example,
			 * send 0x119 + 0t14+0t4 byte packets.
			 */
		  	*(char *)dp = *(char *)rbuf++;
			return;
	    }
	    rbuf += nshorts;
	    dp += nshorts;
	    nbytes -= nshorts << 1;
	}
}


copyout_gap16(dp,sbuf,len)
	register unsigned short	*dp;
	register volatile int	*sbuf;
	register int		 len;
{
	register int		 nshorts;

	/*
	 * Cannot use the normal bcopy since we are moving
	 * from Pmax space into LANCE space.
	 * Use the "Duff device" with a 16bits width.
	 */
	while (len) {
	    switch (nshorts = (len >> 1)) {
		default:
	 	case 32: nshorts = 32;
			 sbuf[31] = dp[31];
	 	case 31: sbuf[30] = dp[30];
	 	case 30: sbuf[29] = dp[29];
	 	case 29: sbuf[28] = dp[28];
	 	case 28: sbuf[27] = dp[27];
	 	case 27: sbuf[26] = dp[26];
	 	case 26: sbuf[25] = dp[25];
	 	case 25: sbuf[24] = dp[24];
	 	case 24: sbuf[23] = dp[23];
	 	case 23: sbuf[22] = dp[22];
	 	case 22: sbuf[21] = dp[21];
	 	case 21: sbuf[20] = dp[20];
	 	case 20: sbuf[19] = dp[19];
	 	case 19: sbuf[18] = dp[18];
	 	case 18: sbuf[17] = dp[17];
	 	case 17: sbuf[16] = dp[16];
	 	case 16: sbuf[15] = dp[15];
	 	case 15: sbuf[14] = dp[14];
	 	case 14: sbuf[13] = dp[13];
	 	case 13: sbuf[12] = dp[12];
	 	case 12: sbuf[11] = dp[11];
	 	case 11: sbuf[10] = dp[10];
	 	case 10: sbuf[9] = dp[9];
	 	case 9: sbuf[8] = dp[8];
	 	case 8: sbuf[7] = dp[7];
	 	case 7: sbuf[6] = dp[6];
	 	case 6: sbuf[5] = dp[5];
	 	case 5: sbuf[4] = dp[4];
	 	case 4: sbuf[3] = dp[3];
	 	case 3: sbuf[2] = dp[2];
	 	case 2: sbuf[1] = dp[1];
	 	case 1: sbuf[0] = dp[0];
			break;
		case 0: {
			/* Last byte of this buffer */
			register unsigned short c;

			wbflush();
			c = *(unsigned short*)sbuf;
#if	BYTE_MSF
			sbuf[0] = (c & 0x00ff) | ((*((unsigned char *)dp))<<8);
#else	/*BYTE_MSF*/
			sbuf[0] = (c & 0xff00) | *((unsigned char *)dp);
#endif	/*BYTE_MSF*/
			return;
		}
	    }
	    sbuf += nshorts;
	    dp += nshorts;
	    len -= (nshorts << 1);
	}
}

bzero_gap16(addr, len)
	vm_offset_t	addr;
	vm_size_t	len;
{
	/* no big deal if we zero twice */
	bzero(addr, len << 1);
}


/*
 *	Object:
 *		copyout_gap32			EXPORTED function
 *
 *	Copy data to lance (data) buffer: dma is 4 words every
 *	other 4 words.  We know: 'to' is aligned to a 4 words
 *	boundary, 'from' is short-aligned.  Too bad if we
 *	copy a bit too much.
 */
copyout_gap32(from, to, len)
	register unsigned int	*from;
	register unsigned int	*to;
	register int		len;
{
	register unsigned int	t0,t1,t2,t3;

	if ((((vm_offset_t)from) & 0x2) == 0)
		goto good_day;

	/* a bad day, only happens on small inline (arp) pkts */
	while (len > 0) {
		*((short *)to) = *((short*)from);
		to = (unsigned int *)((char*)to + 2);
		from = (unsigned int *)((char*)from + 2);
		len -= 2;
		if (((vm_offset_t)to & 0xf) == 0)
			to += 4;/*dma twist*/
	}
	return;

good_day:
	while (len > 0) {
		t0 = from[0]; t1 = from[1]; t2 = from[2]; t3 = from[3];
		from += 4;
		to[0] = t0; to[1] = t1; to[2] = t2; to[3] = t3;
		to += 8;/*dma twist!*/
		len -= 4 * sizeof(int);
	}
}


/*
 *	Object:
 *		copyin_gap32			EXPORTED function
 *
 *	Copy data from lance (data) buffer: dma is 4 words every
 *	other 4 words.  Called in two modes: (a) for the ether header
 *	which is 14 bytes, word aligned (b) for the data, which
 *	is any size but the source is short aligned (triple sigh).
 *	Destinations are word aligned in both cases
 */
copyin_gap32(from, to, len)
	register unsigned int	*from;
	register unsigned int	*to;
	register int		len;
{
	/* no need for generalities, just do it fast */
	if (len <= 16) {
		/* ether header */
		register int	t0,t1,t2,t3;

		t0 = from[0]; t1 = from[1]; t2 = from[2]; t3 = from[3];
		to[0] = t0; to[1] = t1; to[2] = t2; to[3] = t3;

	} else {
		/* data */
		register unsigned int	t0,t1,t2,t3;
		register unsigned short s0;

		s0 = *(unsigned short *)from;
		from = (unsigned int *)(((short*)from) + 1);	/* aligned now */
		from += 4;					/* skipto */
		len -= sizeof(short);

		while (len > 0) {
			t0 = from[0]; t1 = from[1]; t2 = from[2]; t3 = from[3];
			from += 8;/*dma twist!*/
			/* byteorderdep */
			to[0] = s0 | (t0 << 16);
			to[1] = (t0 >> 16) | (t1 << 16);
			to[2] = (t1 >> 16) | (t2 << 16);
			to[3] = (t2 >> 16) | (t3 << 16);
			s0 = t3 >> 16;
			to += 4;
			len -= 4 * sizeof(int);
		}
		*(unsigned short *)to = s0;
	}
}

