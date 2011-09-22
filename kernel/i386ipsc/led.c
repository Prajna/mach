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
 * $Log:	led.c,v $
 * Revision 2.6  93/01/14  17:32:04  danner
 * 	Proper spl typing.
 * 	[92/12/10  17:53:44  af]
 * 
 * Revision 2.5  91/12/10  16:29:55  jsb
 * 	Fixes from Intel
 * 	[91/12/10  15:32:14  jsb]
 * 
 * Revision 2.4  91/06/18  20:50:22  jsb
 * 	New copyright from Intel.
 * 	[91/06/18  20:06:51  jsb]
 * 
 * Revision 2.3  91/06/06  17:04:46  jsb
 * 	Added led_idle, led_active.
 * 	[91/05/13  17:07:21  jsb]
 * 
 * Revision 2.2  90/12/04  14:47:29  jsb
 * 	First checkin.
 * 	[90/12/04  10:57:09  jsb]
 * 
 */
#include <sys/varargs.h>

/* Send data out the LED */

/* Red and Green LED bits */
#define RED    		0x40
#define GREEN    	0x80

/* Port address for the trace port */
#define TRACE_PORT	0x84

#define BAUD_1200   0x1f0   /* best estimate for 1200 baud */

#define START_BIT   1
#define STOP_BIT    0

/* forward declaration */
static delay();

int led_state;

led(color)
int	color;
{
	led_state = color;
	outb(TRACE_PORT, led_state);
}


led_idle()
{
	led(RED);
}


led_active()
{
	led(GREEN);
}


static delay(time)
volatile unsigned int time;
{
	while(time--);
}


led_bit(b)
int b;
{
	if (b) {
		led(led_state | RED);
	} else {
		led(led_state & ~RED);
	}
	delay(BAUD_1200);
}


led_char(c)
char	c;
{
	int i;
	spl_t s;

	s = splhi();
	led_bit(START_BIT);
	for (i = 0; i < 8; i++) {
		led_bit(!((c >> i) & 1));
	}
	led_bit(STOP_BIT);
	delay(4 * BAUD_1200);
	splx(s);
}


led_putchar(c)
int	c;
{
	spl_t	s = splhi();

	led_char((char) c);
	if (c == '\n') {
		led_char('\r');
	}

	splx(s);
}


/*VARARGS1*/
led_printf(fmt, va_alist)
	char	*fmt;
	va_dcl
{
	va_list	listp;
	spl_t	s = splhi();

	va_start(listp);
	_doprnt(fmt, &listp, led_putchar, 0);
	va_end(listp);

	splx(s);
}
