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
 * $Log:	usm.c,v $
 * Revision 2.7  91/12/10  16:32:35  jsb
 * 	Fixes from Intel
 * 	[91/12/10  15:33:41  jsb]
 * 
 * Revision 2.3  91/09/04  11:28:22  jsb
 * 	This is Intel's most recent usm.c, with my JUSTHACKIT code added
 * 	and enabled since things still act funny without it.
 * 	[91/09/04  08:39:09  jsb]
 * 
 * Revision 2.2  91/06/18  20:52:42  jsb
 * 	First checkin. Just a hack until ipsc/usm.c gets merged again.
 * 	[91/06/18  19:04:14  jsb]
 * 
 * Revision 2.3  90/12/22  00:00:00  prp
 * 	Change to use slot number
 * 
 * Revision 2.2  90/12/04  14:50:53  jsb
 * 	Eliminated compiler warning.
 * 	[90/12/04  12:18:23  jsb]
 * 
 * 	Merged (and mostly replaced) with i386ipsc version.
 * 	Unmerged parts are conditionalized with JUSTHACKIT;
 * 	the i386ipsc seems to need this code.
 * 	[90/12/04  10:46:15  jsb]
 * 
 * 	First checkin.
 * 	[90/12/03  21:57:51  jsb]
 * 
 */

#include <mach_kdb.h>
#include <sys/types.h>
#include <device/conf.h>
#include <device/tty.h>
#include <device/io_req.h>
#include <device/errno.h>
#include <device/cirbuf.h>

#define	JUSTHACKIT	0

#if i860
#include <i860ipsc/nodehw.h>
#endif
#include <ipsc/usm.h>

/* These magic values from NX's kt.h */
#define KT_RDY		0x81
#define KT_HOLD		0x82
#define KT_SELECT 	0x83
#define KT_KDB	 	0x84

/* Multiplex states */
#define NOT_WAITING		0
#define WAITING_FOR_KT_RDY	1
#define WAITING_FOR_SELECT	2

extern void 	timeout(), ttrstrt();
int	usmstop(), usmgetstat(), usmsetstat();


/*
 * Switches for polling vs. interrupt.
 */
static int	polling = 0;
static int	interrupts_on = 0;

extern void 	splx();
extern int	spltty();
extern int	soft_pic_enable(), soft_pic_disable();

struct tty	usm_tty;
static int	initialized = 0;

/* variables below allow USM serial line to be shared by more than one node */
static boolean_t selected = FALSE; /* is serial line selected for this node */
static int state = NOT_WAITING;	/* are we waiting for special characters */

/* variables for polled IO */
#define UNSBUF		2048
static char	unselbuf[UNSBUF]; /* buffer to hold output when unselected */
static char	stray[64];	/* characters rcv'd while waiting for KT_RDY */
static struct cirbuf conbuf;	/* circular buffer for output */
static char	column = 0;	/* for tab expansion */
static short	stray_count = 0;

extern ipsc_slot;

/* forward declarations */
static int enable_interrupts();
static int disable_interrupts();
int usmstart();
int usmstop();

#if	i860
int	led_console = 0;
int	led_usm = 0;
#endif	i860

/*
 *	Initialize the console.
 */
cninit()
{
	if (!initialized) {
		/*
		 * I'd like to use cb_alloc(&conbuf, UNSBUF),
		 * but kalloc() hasn't been initialized yet.
		 */
		conbuf.c_start = unselbuf;
		conbuf.c_end = unselbuf + UNSBUF;
		conbuf.c_cf = unselbuf;
		conbuf.c_cl = unselbuf;
		conbuf.c_cc = 0;

		uart_hardreset();
		disable_interrupts();

		while (!inb(iUSM_GIR) & 1) {
			(void) inb(iUSM_LSR);
			(void) inb(iUSM_RXD);
			(void) inb(iUSM_MSR);
		}

		column = 0;
		polling = 1;
		initialized++;
	}
}


usmopen(dev, flag, ior)
dev_t		dev;
int	 	flag;
io_req_t	ior;
{
	int		s;
	struct 	tty *	tp;
	int		unit;
	io_return_t     ret_val;

	unit = minor(dev);
	if (unit != 0) {
		/* we only support a single usm device */
		return(ENXIO);
	}

	tp = &usm_tty;
	s = spltty();

	if ((tp->t_state & (TS_ISOPEN|TS_WOPEN)) == 0) {
		ttychars(tp);
		tp->t_addr = (caddr_t) tp;
		tp->t_dev = dev;
		tp->t_oproc = usmstart;
		tp->t_stop = usmstop;
		tp->t_getstat = usmgetstat;
		tp->t_setstat = usmsetstat;
		tp->t_ispeed = B9600;
		tp->t_ospeed = B9600;
		tp->t_flags = ODDP|EVENP|ECHO|CRMOD;
		tp->t_state &= ~TS_BUSY;
#if i860
		uart_softreset();
		uart_clkstart();
#endif
		polling = 0;
		enable_interrupts();
	}
	if ((tp->t_state & TS_ISOPEN) == 0)
		usmparam(unit);
	tp->t_state |= TS_CARR_ON;

	splx(s);

	ret_val = char_open(dev, tp, flag, ior);
	return ret_val;
}


int usmclose(dev, flag)
int	dev;
int	flag;
{
	struct	tty	*tp;
	int	s;

	tp = &usm_tty;

	s = spltty();
	ttyclose(tp);
	disable_interrupts();
	polling = 1;
	splx(s);

	return 0;

}


int usmread(dev, ior)
int	dev;
io_req_t ior;
{
	int	r;

	r = char_read(&usm_tty, ior);
	return r;
}


int usmwrite(dev, ior)
int	dev;
io_req_t ior;
{
	int	r;

	r = char_write(&usm_tty, ior);
	return r;
}


usmportdeath(dev, port)
dev_t		dev;
mach_port_t	port;
{
	return (tty_portdeath(&usm_tty, port));
}


/*ARGSUSED*/
io_return_t usmgetstat(dev, flavor, data, count)
dev_t		dev;
int		flavor;
int		*data;		/* pointer to OUT array */
unsigned int	*count;		/* OUT */
{
	return (tty_get_status(&usm_tty, flavor, data, count));
}


/*ARGSUSED*/
io_return_t usmsetstat(dev, flavor, data, count)
dev_t		dev;
int		flavor;
int		*data;
unsigned int	count;
{
	io_return_t	result = D_SUCCESS;
	int		unit = minor(dev);

	result = tty_set_status(&usm_tty, flavor, data, count);
	if (result == D_SUCCESS && flavor == TTY_STATUS)
		usmparam(unit);
	return (result);
}


usmparam(unit)
register int unit;
{
	/* we don't need no stinking params... */
	return;
}


/*
 * we're only called if there was an RX interrupt from the usm...
 * ...but that can mean more than one thing...
 */
usmintr(unit)
int	unit;
{
	register struct	tty *tp;
	register int	c;
	int	s;
#if	i860
	extern int intr_debug;
#endif	i860

	if (polling) {
		return;
	}

#if	i860
	if (intr_debug) led_char('u');
#endif	i860

	tp = &usm_tty;
	if (!(tp->t_state & TS_ISOPEN)) {
		tt_open_wakeup(tp);
		return;
	}

#if	i860
	if (intr_debug) led_char('R');
#endif	i860

	c = inb(iUSM_RXD) & 0xff;

	if (state == WAITING_FOR_SELECT) {
		state = NOT_WAITING;
		if ((c & 0x7F) == ipsc_slot || (c & 0x80)) {
			selected = TRUE;
		} else {
			selected = FALSE;
		}
	} else if (c == KT_SELECT) {
		state = WAITING_FOR_SELECT;
	} else if (! selected) {
		;
	} else if (c == KT_RDY) {
		if (state == WAITING_FOR_KT_RDY) {
			/*
			 * getting a KT_RDY is kind of like getting
			 * a TX interrupt from a *normal* uart.
			 */
			state = NOT_WAITING;
			tp->t_state &= ~TS_BUSY;
		} else {
			outb(iUSM_TXD, KT_HOLD);
		}
	} else if (c == KT_KDB) {
		kdb_kintr();
	} else {
		if (tp->t_state&TS_ISOPEN) {
			ttyinput(c, tp);
		} else {
			tt_open_wakeup(tp);
		}
	}

	if (selected) {
		usmstart(tp);
	}

#if	i860
	if (intr_debug) led_char(';');
#endif	i860
}


#if	JUSTHACKIT
int kt_rdy_deficit = 0;
#define	MAX_KT_RDY_DEFICIT	16

justhackit(c, tp)
	u_char c;
	struct tty *tp;
{
	while ((inb(iUSM_LSR) & iTHRE) == 0) {
		continue;
	}
	outb(iUSM_TXD, c);
	if (++kt_rdy_deficit < MAX_KT_RDY_DEFICIT) {
		return;
	}
	while (kt_rdy_deficit > 0) {
		while((inb(iUSM_LSR) & iDR) == 0) {
			continue;
		}
		c = inb(iUSM_RXD) & 0xff;
		if (c == KT_RDY) {
			kt_rdy_deficit--;
			break;
		}
		if (tp) {
			(*linesw[tp->t_line].l_rint)(c, tp);
		} else if (stray_count < sizeof(stray)) {
			stray[stray_count++] = c;
		}
	}
}
#endif


usmstart(tp)
struct tty *tp;
{
	char	nch;

	if (polling) {
		return;
	}
	if (!selected) {
		return;
	}
	if (state != NOT_WAITING) {
		return;
	}
	if (tp->t_state & (TS_TIMEOUT|TS_TTSTOP|TS_BUSY)) {
		return;
	}
#if	JUSTHACKIT
	while (tp->t_outq.c_cc > 0 && (nch=getc(&tp->t_outq)) != -1) {
		justhackit(nch, tp);
	}
	tp->t_state &= ~TS_BUSY;
	tt_write_wakeup(tp);
#else	JUSTHACKIT
	if (tp->t_outq.c_cc <= TTLOWAT(tp)) {
		tt_write_wakeup(tp);
	}
	if (!tp->t_outq.c_cc) {
		return;
	}
	nch = getc(&tp->t_outq);
	if ((tp->t_flags & LITOUT) == 0 && (nch & 0200)) {
		timeout(ttrstrt, (char *)tp, (nch & 0x7f) + 6);
		tp->t_state |= TS_TIMEOUT;
		return;
	}
#if	i860
	if (led_usm) led_char(nch);
#endif
	outb(iUSM_TXD, nch);
	state = WAITING_FOR_KT_RDY;
	tp->t_state |= TS_BUSY;
	return;
#endif	JUSTHACKIT
}


usmstop(tp, flags)
register struct	tty *tp;
int	flags;
{
	if ((tp->t_state & TS_BUSY) && (tp->t_state & TS_TTSTOP) == 0)
		tp->t_state |= TS_FLUSH;
}


dumpunsel()
{
	int	c;

	while ((c = getc(&conbuf)) != -1) {
		cnputc(c);
	}
}


/*
 * This procedure provides the console out routine for the
 * kernel's putchar (printf) routine.  When output for this 
 * node is selected, it polls the UART until the TX Ready bit
 * is set and then outputs the character. If IO is not selected
 * then the character is saved in a buffer so that it can be
 * written later.
 */
cnputc(c)
	char	c;
{
	unsigned char x;
	int s, i, was;

	if (!initialized) {
		cninit();
	}

	switch (c) {
	case '\n':
		cnputc('\r');
	case '\r':
		column = 0;
		break;
	case '\t':
		do {
			cnputc(' ');
		} while (column & 07);
		return(c);
	case '\b':
		if (column)
			column--;
		break;
	default:
		column++;
		break;

	}

#if	i860
	if (led_console) led_char(c);
#endif	i860

	if (!selected) {
		if (putc(c, &conbuf)) {
			int	drop = getc(&conbuf);
			(void) putc(c, &conbuf);
		}
		return (c);
	}

#if	JUSTHACKIT
	justhackit(c, 0);
#else	JUSTHACKIT

	was = usm2cn();
	outb(iUSM_TXD, c);
	for (;;) {
		while((inb(iUSM_LSR) & iDR) == 0);
		if ((x = inb(iUSM_RXD)) == KT_RDY) {
			break;
		} else if (stray_count < sizeof(stray)) {
			stray[stray_count++] = x;
		}
		outb(iUSM_TXD, c);
	}
	cn2usm(was);

#endif	JUSTHACKIT

	return(c);
}


usm2cn()
{
	int	was, s;

	was = interrupts_on;
	if (was) {
		s = spltty();
		/*
		 * we need to let the interrupt driven state machine
		 * kick around until we can jump in.
		 */
		while (state != NOT_WAITING) {
#if	i860
			/*led_printf("usm2cn: state = %d\n", state);*/
#endif	i860
			splx(s);
			s = spltty();
		}
		disable_interrupts();
		polling = 1;
		splx(s);
	}

	return was;
}


cn2usm(was)
int	was;
{
	if (was) {
		polling = 0;
		enable_interrupts();
	}
}


cnswitch(wait)
boolean_t	wait;
{
	int	c, was;

	was = usm2cn();
	c = cndogetc(wait);
	cn2usm(was);

	return c;
}


cngetc()
{
	return (cnswitch(TRUE));
}


cnmaygetc()
{
	return (cnswitch(FALSE));
}


/*
 * This procedure reads a character from the console. It returns
 * -1 if there is no character in the UART receive buffer or if
 * this node is not selected for IO.
 *
 * Note that interrupts must be disabled so that usmintr doesn't
 * get the character first.
 */
cndogetc(wait)
	boolean_t	wait;
{
	int i, c, s, was;

	assert(interrupts_on == 0);

	if (stray_count > 0) {
		c = stray[0];
		for (i = 1; i < stray_count; i++) {
			stray[i - 1] = stray[i]; /* lazyness */
		}
		stray_count--;
	} else {
		do {
			c = uart_getc();
			if (c == KT_RDY)  {
				if (selected) {
					outb(iUSM_TXD, KT_HOLD);
				}
				c = -1;
			} else if (c == KT_SELECT) {
	
				do { c = uart_getc(); } while (c == -1);
				if (((c & 0x7F) == ipsc_slot) || (c & 0x80)) {
					selected = TRUE;
					dumpunsel();
				} else {
					selected = FALSE;
				}
				c = -1;
			} else if (! selected) {
				c = -1;
			} else if (c == KT_KDB) {
#if	MACH_KDB
				kdb_kintr();
#endif	MACH_KDB
				c = -1;
			}
	
		} while (wait && (c == -1));
	}
	if (c == '\r')
		c = '\n';

	if (c != -1) {
		c &= 0177;
	}

	return(c);
}


/* Called by kdb to switch keyboard from polled to interrupt */
cnpollc(on)
	boolean_t	on;
{
	static int	usm_interrupt_state = 0;
	static int	usm_interrupt_state_valid = 0;
	int	s;

	s = sploff();
	if (on) {
		usm_interrupt_state = interrupts_on;
		usm_interrupt_state_valid = 1;
		disable_interrupts();
		polling = 1;
	}
	else {
		assert(usm_interrupt_state_valid != 0);
		if (usm_interrupt_state) {
			usm_interrupt_state = 0;
			polling = 0;
			enable_interrupts();
		}
	}
	splon(s);
}


static disable_interrupts()
{
	int	s;

	s = sploff();

	/* switch to bank 0, 8250 compatibility */
	outb(iUSM_BANK, iBANK0);

	/* disable interrupts */
	outb(iUSM_GER, inb(iUSM_GER) & ~(iRX_ENAB | iTX_ENAB));
	interrupts_on = FALSE;

	splon(s);
}

static enable_interrupts()
{
	int	s;

	s = sploff();

	/* switch to bank 0, 8250 compatibility */
	outb(iUSM_BANK, iBANK0);

	/* enable interrupts */
	outb(iUSM_GER, inb(iUSM_GER) | iRX_ENAB);

	interrupts_on = TRUE;

#if	i860
	soft_pic_enable(SERIAL_INT_MASK);
#endif	i860

	splon(s);
}


/*
 * Hard reset the uart.
 * Not really a hardware reset, but it does send a software reset command
 * to the uart (which is more "firm" than reprogramming a bunch of registers).
 */
uart_hardreset()
{
	uart_reset(1);
}


/*
 * Reprogram the uart without zapping some of the timer registers
 * that are used in uart_clkstart().
 */
uart_softreset()
{
	uart_reset(0);
}


/*
 * Program the uart to a known state.
 * If a hard reset, it fakes it with a software reset command (which is
 * slightly more firm than simply programming a few of the registers).
 */
uart_reset(hard)
	int	hard;
{
	int	b;

	if (hard) {
		outb(iUSM_BANK, iBANK1); /* move to work bank (bank 1) */
		outb(iUSM_ICM, 0x10);	/* do a SW reset usmmand */
		uart_delay();
		uart_delay();
		uart_delay();
		uart_delay();
	}
	else {
		outb(iUSM_BANK, iBANK0);
	}

	/*
	 * Set timer B count for 100 Hz
	 */
	b = 25000;

	/* we are now in bank 0 (because a SW reset usmmand puts us there) */
	outb(iUSM_LCR, 0x80);	/* turn on DLAB bit to load BAL & BAH regs */
	outb(iUSM_BAL, 0x01);	/* set duty cycle of baud rate generator A */
	outb(iUSM_BAH, 0x00);	/* same as external serial clock */
	outb(iUSM_BANK, iBANK3); /* Switch to bank 3 */
	outb(iUSM_BBL, b & 0xff);	/* set baud rate generator B */
	outb(iUSM_BBH, b >> 8);
	outb(iUSM_BANK, iBANK0); /* Switch back to bank 0 */
	outb(iUSM_LCR, 0x03);	/* 1 stop, 8 bits, non-parity, clear DLAB */
	outb(iUSM_MCR, 0x00);	/* clear modem I/O pins status register */
/*hmm*/	outb(iUSM_GER, 0x28);	/* Enable modem, timer interrupt */

	outb(iUSM_BANK, iBANK2); /* move to general config (bank 2) */
	outb(iUSM_FMD, 0x00);	/* normal */
	outb(iUSM_RMD, 0x00);	/* normal */
	outb(iUSM_TMD, 0xC0);	/* manual mode, 4/4 stop bits */
	outb(iUSM_IMD, 0x08);	/* auto interrupt ack, FIFO depth 4 bytes */
	outb(iUSM_RIE, 0x00);	/* disable receive interrupts */

	outb(iUSM_BANK, iBANK3); /* Switch to bank 3 */
	outb(iUSM_CLCF, 0x50);	/* X16 mode, BRG A is clock source */
	outb(iUSM_BACF, 0x44);	/* SCLK pin is clk src, enables BRG mode */
	outb(iUSM_BBCF, 0x00);	/* timer mode (system clock input) */
	outb(iUSM_MIE, 0x09);	/* enable CTS/, DCD/ change interrupts */
	outb(iUSM_TMIE, 0x02);	/* enable timer B interrupts */
	outb(iUSM_PMD, 0xA0);	/* make DSR/, DCD/ inputs */

	outb(iUSM_BANK, iBANK1); /* move to work bank (bank 1) */
	outb(iUSM_RCM, 0xB4);	/* enables reception of characters */
	outb(iUSM_TCM, 0x0E);	/* enables transmission of characters */
	outb(iUSM_ICM, 0x0C);	/* status clear, int acknowledge */

	outb(iUSM_BANK, iBANK0); /* move to compat bank (bank 0) */
}


/*
 * Retrigger timer B to interrupt.
 * It assumes that the timer value set in uart_reset() is
 * still intact.
 */
uart_clkstart()
{
	int	s;

	s = sploff();
	outb(iUSM_BANK, iBANK1);
	inb(iUSM_TMST);
	outb(iUSM_TMCR, 0x22);
	outb(iUSM_BANK, iBANK0);

#if	i860
	soft_pic_enable(SERIAL_INT_MASK);
#endif	i860

	splon(s);
}


#if i386
int uart_delay()
{
	volatile long time = 64000;

	while (time--);
}
#endif i386

#if i860
/*
 * uart_delay allows the serial chip to recover from read or write
 */
uart_delay()
{
        while ((inb(COUNTER_PORT) & 0x20) == 0);
        while ((inb(COUNTER_PORT) & 0x20) != 0);
}
#endif

/*
 * Pull in a character from the uart.
 * No interrupts while checking...
 */
uart_getc()
{
	int	s, c;

	s = sploff();
	if ((inb(iUSM_LSR) & iDR) == 0) {
		splon(s);
		return (-1);
	}
	c = inb(iUSM_RXD) & 0xff;
	splon(s);

	return (c);
}


#if 0
/*
 * Push a character out of the uart.
 * No interrupts while waiting for TX ready...
 */
uart_putc(c)
	char	c;
{
	int	s = sploff();

	while ((inb(iUSM_LSR) & iTHRE) == 0);
	outb(iUSM_TXD, c);

	splon(s);

	return (c);
}
#endif 0
