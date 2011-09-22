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
 * $Log:	adu_sl.c,v $
 * Revision 2.4  93/05/15  19:10:28  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.3  93/03/26  17:55:05  mrt
 * 	Fixed decls.
 * 	[93/03/23            af]
 * 
 * Revision 2.2  93/01/14  17:10:21  danner
 * 	Created, from the DEC specs:
 * 	"Alpha Demonstration Unit Specification"
 * 	V1.0, Aug 1990.
 * 	[92/06/02            af]
 * 
 */
/*
 *	File: adu_sl.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	7/92
 *
 *	ADU's I/O module serial line driver.
 */

#include <asl.h>
#if	NASL > 0

#define	NSL		(NASL*SL_NLINES)

#include <mach_kdb.h>

#include <machine/machspl.h>		/* spl definitions */

#include <device/io_req.h>
#include <device/tty.h>

#include <sys/types.h>
#include <chips/busses.h>
#include <chips/screen_defs.h>
#include <chips/serial_defs.h>

#include <alpha/alpha_cpu.h>
#include <alpha/DEC/tvbus.h>

#define	private	static
#undef cpu_number

struct sl_softc {
	sl_ringbuffer_t		*rb;
	volatile unsigned long	*sl_br;
	volatile unsigned long	*sl_icr;
	volatile unsigned long	*sl_db;
	long			cpu_no_and_channel;
	decl_simple_lock_data(,lock)
	boolean_t		polling_mode;
} sl_softc_data[NSL];

typedef struct sl_softc *sl_softc_t;

sl_softc_t sl_softc[NSL];

/*
 * Utilities
 */
#define	simple_lock_with_timeout(l)		\
	{ 					\
		register int n = 0;		\
		while (n < 1*1024*1024) {	\
			if (simple_lock_try(l))	\
				break;		\
			n++;			\
		}				\
	}


private
rb_putc(
	sl_ringbuffer_t	*rb,
	char		c)
{
	register int    t = rb->txli;

	if (++t >= SL_BUF_SIZE)
		t = 0;
	while (rb->txui == t)
		delay(10);
	rb->txbuf[t] = c;
	rb->txli = t;
	wbflush();
}

private 
char rb_getc(
	sl_ringbuffer_t	*rb,
	boolean_t	wait)
{
	register int    t = rb->rxui;
	char            c;
loop:
	if (rb->rxli == t)
		if (wait)
			goto loop;
		else
			return -1;
	if (++t >= SL_BUF_SIZE)
		t = 0;
	c = rb->rxbuf[t];
	rb->rxui = t;
	wbflush();
	return c;
}


/*
 * Definition of the driver for the auto-configuration program.
 */

private int	adu_sl_probe();
private void	adu_sl_attach();

vm_offset_t	adu_sl_std[NASL];
struct	bus_device *adu_sl_info[NASL];
struct	bus_driver adu_sl_driver = 
        { adu_sl_probe, 0, adu_sl_attach, 0, adu_sl_std, "asl", adu_sl_info,};


/*
 * Adapt/Probe/Attach functions
 */

private int	adu_sl_probe(), adu_sl_start(), adu_sl_zilch(),
		adu_sl_putc(), adu_sl_getc(),
		adu_sl_pollc(), adu_sl_param();

adu_sl_init(
	int		unit,
	int		cpu_no,
	vm_offset_t	module_base,
	boolean_t	isa_console)
{
	sl_ringbuffer_t	*rb;
	sl_softc_t	sl;
	register int	n;
	extern vm_offset_t	pmap_steal_memory();
	int		adu_sl_tintr(), adu_sl_rintr();

	if (isa_console) {
		console_probe	= adu_sl_probe;
		console_param	= adu_sl_param;
		console_start	= adu_sl_start;
		console_putc	= adu_sl_putc;
		console_getc	= adu_sl_getc;
		console_pollc	= adu_sl_pollc;
		console_mctl	= adu_sl_zilch;
		console_softCAR	= adu_sl_zilch;
	}

	n  = unit*SL_NLINES;
	sl = &sl_softc_data[n];

	/*
	 * First line
	 */

	sl_softc[n] = sl;
	simple_lock_init(&sl->lock);
	sl->sl_br  = (volatile unsigned long *)( module_base + TV_IO_SL1_BASE );
	sl->sl_icr = (volatile unsigned long *)( module_base + TV_IO_SL1_ICR );
	sl->sl_db  = (volatile unsigned long *)( module_base + TV_IO_SL1_S_BELL );

	rb = (sl_ringbuffer_t *) PHYS_TO_K0SEG(pmap_steal_memory(ALPHA_PGBYTES));
	sl->rb = rb;

	bzero(rb, sizeof(sl_ringbuffer_t));
	rb->rxsize = rb->txsize = SL_BUF_SIZE;
	rb->txmode = SL_M_NORMAL;

	/* Interrupts */
	{
		int tchan, rchan;
		tchan = tv_get_channel(cpu_no, adu_sl_tintr, sl);
		rchan = tv_get_channel(cpu_no, adu_sl_rintr, sl);
		sl->cpu_no_and_channel = ((cpu_no << 2) & SL_ICR_IRQ_NODE) |
				       ((rchan << 6) & SL_ICR_RIRQ_CHAN) |
				       ((tchan << 11) & SL_ICR_TIRQ_CHAN);
	}

	*sl->sl_icr = sl->cpu_no_and_channel;	/* enable intrs later */

	wbflush();
	*sl->sl_br = K0SEG_TO_PHYS(rb);
	wbflush();
	*sl->sl_db = 1;


	/*
	 * Second line
	 */

	sl_softc[n+1] = ++sl;
	simple_lock_init(&sl->lock);
	sl->sl_br  = (volatile unsigned long *)( module_base + TV_IO_SL2_BASE );
	sl->sl_icr = (volatile unsigned long *)( module_base + TV_IO_SL2_ICR );
	sl->sl_db  = (volatile unsigned long *)( module_base + TV_IO_SL2_S_BELL );

	rb = (sl_ringbuffer_t *) PHYS_TO_K0SEG(pmap_steal_memory(ALPHA_PGBYTES));
	sl->rb = rb;

	bzero(rb, sizeof(sl_ringbuffer_t));
	rb->rxsize = rb->txsize = SL_BUF_SIZE;
	rb->txmode = SL_M_NORMAL;

	{
		int tchan, rchan;
		tchan = tv_get_channel(cpu_no, adu_sl_tintr, sl);
		rchan = tv_get_channel(cpu_no, adu_sl_rintr, sl);
		sl->cpu_no_and_channel = ((cpu_no << 3) & SL_ICR_IRQ_NODE) |
				       ((rchan << 6) & SL_ICR_RIRQ_CHAN) |
				       ((tchan << 11) & SL_ICR_TIRQ_CHAN);
	}

	*sl->sl_icr = sl->cpu_no_and_channel;	/* enable intrs later */

	wbflush();
	*sl->sl_br = K0SEG_TO_PHYS(rb);
	wbflush();
	*sl->sl_db = 1;

}

private
adu_sl_zilch()
{
	/* nothing, unimplemented/unsupported */
}

private
adu_sl_putc(
	int	unit,
	int	line,
	int	c)
{
	sl_softc_t	sl = sl_softc[(unit*SL_NLINES) + line];

	simple_lock_with_timeout(&sl->lock);
	rb_putc(sl->rb, c);
	simple_unlock(&sl->lock);
	*sl->sl_db = 1;
}

private
adu_sl_getc(
	int		unit,
	int		line,
	boolean_t	wait,
	boolean_t	raw)
{
	sl_softc_t	sl = sl_softc[(unit*SL_NLINES) + line];
	char		c;

	simple_lock_with_timeout(&sl->lock);
	c = rb_getc(sl->rb, wait);
	simple_unlock(&sl->lock);
	return c;
}

private
adu_sl_probe(
	vm_offset_t	  base,
	struct bus_device *ui)
{
	int	n = ui->unit * SL_NLINES;

	/* inited already ? */
	if (sl_softc[n] == 0)
		adu_sl_init(ui->unit, cpu_number(), base, FALSE);
	/* ttys */
	for (; n < (ui->unit+1)*SL_NLINES; n++)
		console_tty[n]->t_addr = (char*)(n+1);
	return 1;
}

private void
adu_sl_attach(
	register struct bus_device *ui)
{
	int unit = ui->unit;
	extern int tty_inq_size;
	int i;

	/* We only have 2 lines, but they are quick.
	 * Give em a lot of room
	 */
	tty_inq_size = 2048;
	for (i = unit*SL_NLINES; i < (unit+1)*SL_NLINES; i++) {
		register struct tty	*tp = console_tty[i];

		ttychars(tp);
		tp->t_state |= TS_ISOPEN | TS_CARR_ON;
	}

	printf("\n sl0: (console)\n sl1:");
}

adu_sl_start(
	register struct tty	*tp)
{
	sl_softc_t	sl = sl_softc[(int)tp->t_addr - 1];

	while (tp->t_outq.c_cc > 0) {
		simple_lock_with_timeout(&sl->lock);
		rb_putc(sl->rb, getc(&tp->t_outq));
		simple_unlock(&sl->lock);
		*sl->sl_db = 1;
	}
	tp->t_state &= ~TS_BUSY;
}

adu_sl_param(
	register struct tty	*tp,
	register int		line)
{
	sl_softc_t	sl;

	line = tp->t_dev;	/* in case console, called early */
	if (line >= NASL) return;	/* sanity */
	sl = sl_softc[line];

	*sl->sl_icr = sl->cpu_no_and_channel | SL_ICR_RE;
}

adu_sl_pollc(
	int		unit,
	boolean_t	on)
{
	sl_softc[unit/SL_NLINES]->polling_mode = on;
}

/* Only one cpu gets here */
adu_sl_rintr(
	sl_softc_t	sl)
{
	register int    line = sl - sl_softc_data, c;

	if (sl->polling_mode)
		return;

	simple_lock_with_timeout(&sl->lock);
	c = rb_getc(sl->rb, FALSE);
	simple_unlock(&sl->lock);

	while (c >= 0) {
#if	MACH_KDB
		extern int	l3break;
		if (c == l3break) gimmeabreak();
#endif	/* MACH_KDB */
		cons_simple_rint(line, 0, c, 0);

		*sl->sl_db = 1;
		simple_lock_with_timeout(&sl->lock);
		c = rb_getc(sl->rb, FALSE);
		simple_unlock(&sl->lock);
	}
}

adu_sl_tintr(
	sl_softc_t	sl)
{
	if (sl->polling_mode) return;
}

#endif	/* NASL > 0 */
