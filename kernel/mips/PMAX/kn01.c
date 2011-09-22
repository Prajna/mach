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
 * $Log:	kn01.c,v $
 * Revision 2.10  93/05/15  19:12:50  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.9  93/01/14  17:50:06  danner
 * 	Proper spl typing.
 * 	[92/12/01            af]
 * 
 * Revision 2.8  91/05/14  17:23:02  mrt
 * 	Correcting copyright
 * 
 * Revision 2.7  91/05/13  06:03:55  af
 * 	It turns out one has to wait for all writes to complete before
 * 	reading back from the SII scsi buffer.  Odd size transfers would
 * 	sometimes clobber the next-byte-up into 0xff, now we do it right.
 * 	[91/05/12  16:06:58  af]
 * 
 * Revision 2.6.1.1  91/04/05  13:07:09  af
 * 	It turns out one has to wait for all writes to complete before
 * 	reading back from the SII scsi buffer.  Odd size transfers would
 * 	sometimes clobber the next-byte-up into 0xff, now we do it right.
 * 
 * Revision 2.6  91/02/14  14:34:21  mrt
 * 	In interrupt routines, drop priority as now required.
 * 	[91/02/12  12:54:42  af]
 * 
 * 	Added kn01_memcheck().
 * 	[91/01/03  02:09:21  af]
 * 
 * Revision 2.5  91/02/05  17:41:50  mrt
 * 	Added author notices
 * 	[91/02/04  11:14:05  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:12:38  mrt]
 * 
 * Revision 2.4  91/01/09  19:50:07  rpd
 * 	Added kn01_memcheck().
 * 	[91/01/03  02:09:21  af]
 * 
 * Revision 2.3  90/12/05  23:31:45  af
 * 	Moved errintr handler here, along with others.
 * 	Also moved here special 'gap16' memory operations.
 * 	[90/12/03  23:23:10  af]
 * 
 * Revision 2.1.1.1  90/11/01  02:59:06  af
 * 	Created, from the DEC specs:
 * 	"DECstation 3100 Desktop Workstation Functional Specification"
 * 	Workstation Systems Engineering, Palo Alto, CA. Aug 28, 1990.
 * 	[90/09/03            af]
 */
/*
 *	File: kn01.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	Routines specific to the KN01 processor (pmax)
 */

#include <mach/std_types.h>
#include <machine/machspl.h>	/* for spl */
#include <mips/mips_cpu.h>
#include <mips/PMAX/kn01.h>

#define	KN01CSR_ADDR		PHYS_TO_K1SEG(KN01_SYS_CSR)
#define	KN01ERR_ADDR		PHYS_TO_K1SEG(KN01_SYS_ERRADR)

/*
 *	Object:
 *		kn01_err_intr			EXPORTED function
 *
 *	Handle "memory errors" on a pmax:
 *		CPU write timeouts
 *	Also, vertical retrace interrupts come here too.
 *
 */
int errintr_cnt;

kn01_err_intr(st,spllevel)
	spl_t	spllevel;
{
	register short csr, adr;

	csr = *(volatile short *)KN01CSR_ADDR;
	adr = *(volatile short *)KN01ERR_ADDR;

	/* scrub error/interrupt */
	/* and keep leds off */
	*((volatile short *)KN01CSR_ADDR) =
		KN01_CSR_VINT|KN01_CSR_MERR|KN01_CSR_LEDS_MASK;

	if (csr & KN01_CSR_VINT)
		pm_intr(0,spllevel);
	if (csr & KN01_CSR_MERR) {
		errintr_cnt++;
		printf("(%d)%s%x [%x]\n", errintr_cnt,
		       "Bad memory chip at phys ",
		       adr, csr);
	}
}

/*
 *	Object:
 *		kn01_memcheck			EXPORTED function
 *
 *	Occasionally, the bus error might be due to a
 *	problem with I/O devices.
 */
boolean_t
kn01_memcheck(addr, pc)
	unsigned addr, pc;
{
	if ((PHYS_TO_K1SEG(KN01_SYS_SII_B_START) <= addr) &&
	    (addr < PHYS_TO_K1SEG(KN01_SYS_SII_B_END))) {
		sii_reset(PHYS_TO_K1SEG(KN01_SYS_SII), TRUE);
		printf("warning: did sii_reset to break bus lockup\n");
		return TRUE;
	}
	return FALSE;
}


/*
 *	Object:
 *		kn01_scsi_intr			EXPORTED function
 *		kn01_se_intr			EXPORTED function
 *		kn01_dz_intr			EXPORTED function
 *
 *	Handle interrupts from various motherboard devices
 *
 */
kn01_scsi_intr(st,spllevel)
{
	/* Pmaxen can only have one, SII scsi controller */
	return sii_intr(0,spllevel);
}

kn01_se_intr(st,spllevel)
{
	/* Pmaxen can only have one LANCE controller */
	return se_intr(0,spllevel);
}

kn01_dz_intr(st,spllevel)
{
	/* Pmaxen can only have one DZ controller */
	return dz_intr(0,spllevel);
}

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
	register volatile long	*rbuf;
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
	register volatile long	*sbuf;
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
{
	/* no big deal if we zero twice */
	bzero(addr, len << 1);
}

