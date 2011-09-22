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
 * $Log:	mips_misc.c,v $
 * Revision 2.14  93/05/15  19:13:18  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.13  93/02/05  08:04:35  danner
 * 	Clock got more generic.
 * 	[93/02/04  01:59:32  af]
 * 
 * 	In dprintf, pass the fourth argument that was sleazily added to
 * 	doprnt() without telling anyone.  Grrr.
 * 	[92/12/10            af]
 * 
 * Revision 2.12  92/04/01  19:34:29  rpd
 * 	Removed mips_gets.
 * 	[92/03/31            rpd]
 * 
 * Revision 2.11  91/10/09  16:14:52  af
 * 	correct gets problem
 * 
 * Revision 2.10  91/08/24  12:23:28  af
 * 	Spl defs, define halt() function.
 * 	[91/08/02  03:18:57  af]
 * 
 * Revision 2.9  91/05/14  17:36:23  mrt
 * 	Correcting copyright
 * 
 * Revision 2.8  91/05/13  06:06:19  af
 * 	Found why the dprintf hung on 3max&new proms: clock interrupts.
 * 	[91/05/12  16:00:12  af]
 * 
 * Revision 2.7  91/02/05  17:50:00  mrt
 * 	Added author notices
 * 	[91/02/04  11:24:03  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:27:51  mrt]
 * 
 * Revision 2.6  90/12/05  23:38:09  af
 * 
 * 
 * Revision 2.5  90/12/05  20:49:56  af
 * 	Cannot dprintf() anymore at shutdown: new screwed up
 * 	proms on 3max hang the machine.
 * 	[90/12/03            af]
 * 
 * Revision 2.4  90/08/07  22:29:51  rpd
 * 	Added ntohl(), optionally used in the ether driver.
 * 	Removed silly set_leds() thing.
 * 	[90/08/07  15:22:41  af]
 * 
 * Revision 2.2.3.2  90/06/11  11:25:43  af
 * 	Added ntohl(), optionally used in the ether driver.
 * 
 * Revision 2.2.3.1  90/05/30  16:40:25  af
 * 	Removed silly set_leds() thing.
 * 
 * Revision 2.3  90/06/02  15:02:50  rpd
 * 	Added missing include.
 * 	[90/03/26  22:52:13  rpd]
 * 
 * Revision 2.2  89/11/29  14:14:51  af
 * 	Created.
 * 	[89/10/04            af]
 */
/*
 *	File: mips_misc.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Miscellaneous operations.
 *
 *	This is a catchall file, put things here IFF they
 *	truly do not deserve a separate module.
 */
#include <cpus.h>
#include <mach/mach_types.h>
#include <machine/machspl.h>		/* spl definitions */
#include <mips/mips_cpu.h>
#include <mips/mips_box.h>
#include <mips/prom_interface.h>
#include <sys/varargs.h>
#include <vm/pmap.h>

/*
 *	Object:
 *		halt				EXPORTED function
 *
 *		Halts the machine
 *
 */
halt()
{
	prom_halt(0,0,0,0);
}

/*
 *	Object:
 *		halt_cpu			EXPORTED function
 *
 *		Halts the current cpu
 *
 */
halt_cpu()
{
	splhigh();	/* sanity */
	mc_close();
#if	!(NCPUS>1)	/* uniproc, Bob?*/
	dprintf("\nHalting cpu... (transfer to monitor)\n\n");
#endif
	halt();
}

/*
 *	Object:
 *		halt_all_cpus			EXPORTED function
 *
 *		Stop the machine, optionally reboot
 *
 *	Is this obsoleted by the shutdown thread or not ?
 */
halt_all_cpus(reboot)
	boolean_t	reboot;
{
	register int i;

	splhigh();	/* sanity */
	if (reboot) {
		mc_close();
		dprintf("rebooting.... (transferring to monitor)\n\n");
		prom_reboot();
	}
	halt_cpu();
}

/*
 *	Object:
 *		slave_config			EXPORTED function
 *
 *		Configure a "slave" processor
 *
 *	Does nothing on a uniprocessor
 */
slave_config()
{
}


/*
 *	Object:
 *		mips_gets			EXPORTED function
 *
 *		Read a string from console
 *
 *	Get characters from console, with editing, until a
 *	newline or cr is detected.
 *	Returns a zero-terminated string (e.g. kills newline)
 */
mips_gets(cp)
	char *cp;
{
	register char *lp;
	register c;

	lp = cp;
	for (;;) {
		c = getchar() & 0177;
		if (c == -1)		/* lk201 hickup */
			continue;
		switch (c) {
		case '\n':
		case '\r':
			*lp++ = '\0';
			return;
		case '\b':
		case '#':
		case '\177':
			lp--;
			if (lp < cp)
				lp = cp;
			continue;
		case '@':
		case 'u'&037:
			lp = cp;
			cnputc('\n');
			continue;
		default:
			*lp++ = c;
		}
	}
}


/*
 *	Object:
 *		getchar				EXPORTED function
 *
 *		Read a character from console
 *
 *	Does echo, maps cr->newline
 */
getchar()
{
	register c;

	c = cngetc();
	if (c == '\r')
		c = '\n';
	if (c != -1)		/* lk201 hickup */
		cnputc(c);
	return (c);
}

/*
 *	Object:
 *		dprintf				EXPORTED function
 *
 *		Debugging printouts
 *
 *	Like printf, ship characters to prom.
 *	Used before console line is properly configured,
 *	or out of despair.
 *
 */
dprintf(fmt, va_alist)
	char	*fmt;
	va_dcl
{
	va_list listp;

	va_start(listp);
	_doprnt(fmt, &listp, prom_putchar, 16);
	va_end(listp);
}

/*
 *	Object:
 *		ntohl				EXPORTED function
 *
 *		Byteswap an integer
 *
 */
unsigned ntohl(n)
	register unsigned n;
{
	register unsigned tmp0, tmp1;

	tmp0 = (n << 24) | (n >> 24);
	tmp1 = (n & 0xff00) << 8;
	tmp0 |= tmp1;
	tmp1 = (n >> 8) & 0xff00;
	return tmp0 | tmp1;
}

/*
 *	Object:
 *		timemmap			EXPORTED function
 *
 *		Map the time info to user space
 *
 */
#include <mach/vm_prot.h>

timemmap(dev,off,prot)
	vm_prot_t prot;
{
	extern int     *mtime;

	if (prot & VM_PROT_WRITE)
		return (-1);

	return (mips_btop(pmap_extract(pmap_kernel(), mtime)));
}

/*
 *	Object:
 *		startrtclock			EXPORTED function
 *
 *		Enable clock interrupts
 *
 */
startrtclock()
{
	mc_open();
}

/*
 *	Object:
 *		resettodr		EXPORTED function
 *
 *		Change time
 *
 */
resettodr()
{
	mc_write();
}
