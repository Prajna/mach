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
 * $Log:	rtc.c,v $
 * Revision 2.10  93/05/15  19:32:50  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.9  93/05/10  17:47:35  rvb
 * 	Let's not use time_t in the kernel.
 * 	[93/05/04  17:37:13  rvb]
 * 
 * Revision 2.8  93/01/14  17:31:46  danner
 * 	Proper spl typing.
 * 	[92/11/30            af]
 * 
 * Revision 2.7  92/03/04  10:50:21  rpd
 * 	Well, writetodc() is broken for after leapday on leapyears,
 * 	till Jan 1 the next year.
 * 	[92/03/03            rvb]
 * 
 * Revision 2.6  91/05/14  16:29:48  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:20:21  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:47:09  mrt]
 * 
 * Revision 2.4  90/11/26  14:51:00  rvb
 * 	jsb bet me to XMK34, sigh ...
 * 	[90/11/26            rvb]
 * 	Synched 2.5 & 3.0 at I386q (r1.5.1.3) & XMK35 (r2.4)
 * 	[90/11/15            rvb]
 * 
 * Revision 1.5.1.2  90/07/27  11:27:01  rvb
 * 	Fix Intel Copyright as per B. Davies authorization.
 * 	[90/07/27            rvb]
 * 
 * Revision 2.2  90/05/03  15:46:00  dbg
 * 	Converted for pure kernel.
 * 	[90/02/20            dbg]
 * 
 * Revision 1.5.1.1  90/01/08  13:29:51  rvb
 * 	Add Intel copyright.
 * 	[90/01/08            rvb]
 * 
 * Revision 1.5  89/09/25  12:27:28  rvb
 * 	File was provided by Intel 9/18/89.
 * 	[89/09/23            rvb]
 * 
 */

/*
  Copyright 1988, 1989 by Intel Corporation, Santa Clara, California.

		All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Intel
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <sys/types.h>
#ifdef	MACH_KERNEL
#include <kern/time_out.h>
#include <sys/time.h>
#else	MACH_KERNEL
#include <sys/param.h>
#include <sys/kernel.h>
#endif	MACH_KERNEL
#include <i386/machspl.h>
#include <i386at/rtc.h>

static unsigned char rtc[RTC_NREG];
static int first_rtcopen_ever = 1;

rtcinit()
{
	outb(RTC_ADDR, RTC_A);
	outb(RTC_DATA, RTC_DIV2 | RTC_RATE6);
	outb(RTC_ADDR, RTC_B);
	outb(RTC_DATA, RTC_HM);
}


int
rtcget(regs)
unsigned char	*regs;
{
	if (first_rtcopen_ever) {
		rtcinit();
		first_rtcopen_ever = 0;
	}
	outb(RTC_ADDR, RTC_D); 
	if (inb(RTC_DATA) & RTC_VRT == 0) return(-1);
	outb(RTC_ADDR, RTC_A);	
	while (inb(RTC_DATA) & RTC_UIP)		/* busy wait */
		outb(RTC_ADDR, RTC_A);	
	load_rtc(regs);
	return(0);
}	

rtcput(regs)
unsigned char 	*regs;
{
	register unsigned char	x;

	if (first_rtcopen_ever) {
		rtcinit();
		first_rtcopen_ever = 0;
	}
	outb(RTC_ADDR, RTC_B);
	x = inb(RTC_DATA);
	outb(RTC_ADDR, RTC_B);
	outb(RTC_DATA, x | RTC_SET); 	
	save_rtc(regs);
	outb(RTC_ADDR, RTC_B);
	outb(RTC_DATA, x & ~RTC_SET); 
}


extern struct timeval time;
extern struct timezone tz;

static int month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

yeartoday(year)
int year;
{
	return((year%4) ? 365 : 366);
}

hexdectodec(n)
char n;
{
	return(((n>>4)&0x0F)*10 + (n&0x0F));
}

char
dectohexdec(n)
int n;
{
	return((char)(((n/10)<<4)&0xF0) | ((n%10)&0x0F));
}


readtodc(tp)
	u_int	*tp;
{
	struct rtc_st rtclk;
	time_t n;
	int sec, min, hr, dom, mon, yr;
	int i, days = 0;
	spl_t	ospl;

#ifdef	MACH_KERNEL
	ospl = splclock();
#else	MACH_KERNEL
	ospl = spl5();
#endif	MACH_KERNEL
	if (rtcget(&rtclk)) {
		splx(ospl);
		return(-1);
	}
	splx (ospl);

	sec = hexdectodec(rtclk.rtc_sec);
	min = hexdectodec(rtclk.rtc_min);
	hr = hexdectodec(rtclk.rtc_hr);
	dom = hexdectodec(rtclk.rtc_dom);
	mon = hexdectodec(rtclk.rtc_mon);
	yr = hexdectodec(rtclk.rtc_yr);
	yr = (yr < 70) ? yr+100 : yr;

	n = sec + 60 * min + 3600 * hr;
	n += (dom - 1) * 3600 * 24;

	if (yeartoday(yr) == 366)
		month[1] = 29;
	for (i = mon - 2; i >= 0; i--)
		days += month[i];
	month[1] = 28;
	for (i = 70; i < yr; i++)
		days += yeartoday(i);
	n += days * 3600 * 24;

#ifdef	MACH_KERNEL
#else	MACH_KERNEL
	n += tz.tz_minuteswest * 60;
	if (tz.tz_dsttime)
		n -= 3600;
#endif	MACH_KERNEL

	*tp = n;

	return(0);
}

writetodc()
{
	struct rtc_st rtclk;
	time_t n;
	int diff, i, j;
	spl_t	ospl;

#ifdef	MACH_KERNEL
	ospl = splclock();
#else	MACH_KERNEL
	ospl = spl5();
#endif	MACH_KERNEL
	if (rtcget(&rtclk)) {
		splx(ospl);
		return(-1);
	}
	splx(ospl);

#ifdef	MACH_KERNEL
	diff = 0;
#else	MACH_KERNEL
	diff = tz.tz_minuteswest * 60;
	if (tz.tz_dsttime)
		diff -= 3600;
#endif	MACH_KERNEL
	n = (time.tv_sec - diff) % (3600 * 24);   /* hrs+mins+secs */
	rtclk.rtc_sec = dectohexdec(n%60);
	n /= 60;
	rtclk.rtc_min = dectohexdec(n%60);
	rtclk.rtc_hr = dectohexdec(n/60);

	n = (time.tv_sec - diff) / (3600 * 24);	/* days */
	rtclk.rtc_dow = (n + 4) % 7;  /* 1/1/70 is Thursday */

	for (j = 1970, i = yeartoday(j); n >= i; j++, i = yeartoday(j))
		n -= i;

	rtclk.rtc_yr = dectohexdec(j - 1900);

	if (i == 366)
		month[1] = 29;
	for (i = 0; n >= month[i]; i++)
		n -= month[i];
	month[1] = 28;
	rtclk.rtc_mon = dectohexdec(++i);

	rtclk.rtc_dom = dectohexdec(++n);

#ifdef	MACH_KERNEL
	ospl = splclock();
#else	MACH_KERNEL
	ospl = spl5();
#endif	MACH_KERNEL
	rtcput(&rtclk);
	splx(ospl);

	return(0);
}
