/* 
 * Mach Operating System
 * Copyright (c) 1993,1991,1990,1989 Carnegie Mellon University
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
 * $Log:	printf.c,v $
 * Revision 2.6  93/01/14  18:03:58  danner
 * 	Added protections and warnings for calls to printf before printf init.
 * 	[93/01/10            danner]
 * 	64bit cleanup.
 * 	[92/12/21  14:38:43  af]
 * 
 * 	Retry until truly flushed output, or something broken.
 * 	[92/12/01            af]
 * 
 * Revision 2.5  92/02/19  15:10:55  elf
 * 	Created.
 * 	[92/02/11            rpd]
 * 
 */

#include <mach.h>
#include <device/device.h>
#include <varargs.h>

static mach_port_t console_port = MACH_PORT_NULL;
static boolean_t dropped_print = 0;

void
printf_init(device_server_port)
	mach_port_t device_server_port;
{
	(void) device_open(device_server_port,
			   (dev_mode_t)0,
			   "console",
			   &console_port);
	if (dropped_print)
	  printf("WARNING: printfs attempted before printf_init\n");
	return;
}

#define	PRINTF_BUFMAX	128

struct printf_state {
	char buf[PRINTF_BUFMAX + 1]; /* extra for '\r\n' */
	unsigned int index;
};

static void
flush(state)
	struct printf_state *state;
{
	int             amt, cnt = 0;

	if (console_port == MACH_PORT_NULL)
	  dropped_print = TRUE;
	else
	  {
	    while (cnt < state->index) {
	      if (device_write_inband(console_port, (dev_mode_t)0, (recnum_t)0,
				      &state->buf[cnt],
				      (mach_msg_type_number_t)(state->index - cnt),
				      &amt) !=
		  D_SUCCESS) 
		break;
	      cnt += amt;
	    }
	  }
	state->index = 0;
}

static void
putchar(arg, c)
	char *arg;
	int c;
{
	struct printf_state *state = (struct printf_state *) arg;

	if (c == '\n') {
	    state->buf[state->index] = '\r';
	    state->index++;
	}
	state->buf[state->index] = c;
	state->index++;

	if (state->index >= PRINTF_BUFMAX)
	    flush(state);
}

/*
 * Printing (to console)
 */
vprintf(fmt, args)
	char *fmt;
	va_list args;
{
	struct printf_state state;

	state.index = 0;
	_doprnt(fmt, args, 0, (void (*)()) putchar, (char *) &state);

	if (state.index != 0)
	    flush(&state);
}

/*VARARGS1*/
printf(fmt, va_alist)
	char *fmt;
	va_dcl
{
	va_list	args;

	va_start(args);
	vprintf(fmt, args);
	va_end(args);
}

safe_gets(str, maxlen)
	char *str;
	int  maxlen;
{
	register char *lp;
	register int c;

	char	inbuf[IO_INBAND_MAX];
	mach_msg_type_number_t count;
	register char *ip;
	char *strmax = str + maxlen - 1; /* allow space for trailing 0 */

	lp = str;
	for (;;) {
	    count = IO_INBAND_MAX;
	    (void) device_read_inband(console_port, (dev_mode_t)0, (recnum_t)0,
				      sizeof(inbuf), inbuf, &count);
	    for (ip = inbuf; ip < &inbuf[count]; ip++) {
		c = *ip;
		switch (c) {
		    case '\n':
		    case '\r':
			printf("\n");
			*lp++ = 0;
			return;

		    case '\b':
		    case '#':
		    case '\177':
			if (lp > str) {
			    printf("\b \b");
			    lp--;
			}
			continue;
		    case '@':
		    case 'u'&037:
			lp = str;
			printf("\n\r");
			continue;
		    default:
			if (c >= ' ' && c < '\177') {
			    if (lp < strmax) {
				*lp++ = c;
				printf("%c", c);
			    }
			    else {
				printf("%c", '\007'); /* beep */
			    }
			}
		}
	    }
	}
}
