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
 * $Log:	alpha_misc.c,v $
 * Revision 2.6  93/08/31  15:15:42  mrt
 * 	To reboot one has to say the operator halted it.
 * 	[93/08/25            af]
 * 
 * Revision 2.5  93/05/15  19:11:01  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.4  93/03/26  17:55:46  mrt
 * 	Lint.
 * 	[93/03/23            af]
 * 
 * Revision 2.3  93/03/09  10:49:29  danner
 * 	String protos.
 * 	[93/03/07  13:31:07  af]
 * 
 * 	Fixed reboot (almost), I had forgotten all about the HWRPB. Sigh.
 * 	[93/02/19            af]
 * 
 * Revision 2.2  93/02/05  07:57:30  danner
 * 	32bit-prom-bug support (jeffreyh).
 * 	Sketch proper reboot.
 * 	[93/02/04  00:47:40  af]
 * 
 * 	Created.
 * 	[92/06/02            af]
 * 
 */
/*
 *	File: alpha_misc.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Miscellaneous operations.
 *
 *	This is a catchall file, put things here IFF they
 *	truly do not deserve a separate module.
 */
#include <cpus.h>

#include <mach/std_types.h>
#include <kern/strings.h>
#include <machine/machspl.h>		/* spl definitions */
#include <alpha/alpha_cpu.h>
#include <alpha/prom_interface.h>
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
	prom_halt();
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
	(void) splhigh();	/* sanity */
	alpha_reset_before_reboot();
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
	if (reboot) {
		printf("rebooting.... (transferring to monitor)\n\n");
		prom_reboot();
	}
	(void) splhigh();	/* sanity */
	halt_cpu();
}

#if	(NCPUS > 1)

#include <mach/processor_info.h>

/*
 *	Object:
 *		cpu_start			EXPORTED function
 *
 *		Start a processor running
 *
 */
kern_return_t
cpu_start(processor_no)
{
	return KERN_SUCCESS;
}

/*
 *	Object:
 *		cpu_control			EXPORTED function
 *
 *		Play with a processor
 *
 */
kern_return_t
cpu_control(processor_num, info, count)
	int			processor_num;
	processor_info_t	info;
	natural_t		*count;
{
	return KERN_FAILURE;
}

/*
 *	Object:
 *		start_other_cpus		EXPORTED function
 *
 */
start_other_cpus()
{
	extern simple_lock_data_t	slave_init_lock;

	simple_unlock(&slave_init_lock);
}


/*
 *	Object:
 *		simple_lock_pause			EXPORTED function
 *
 *		Idle spinning on simple lock retry
 *
 */
/* XXX should be adjusted per CPU speed */
int simple_lock_pause_loop = 100;

unsigned int simple_lock_pause_count = 0;	/* debugging */

void
simple_lock_pause()
{
	static volatile int dummy;
	int i;

	simple_lock_pause_count++;

	/*
	 * Used in loops that are trying to acquire locks out-of-order.
	 */

	for (i = 0; i < simple_lock_pause_loop; i++)
	    dummy++;	/* keep the compiler from optimizing the loop away */
}

switch_to_shutdown_context() {gimmeabreak(); /* NYI */ }

#endif

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
 *		pre_prom_putc				EXPORTED function
 *
 *		Remap char before passing off to prom
 *
 *	Prom only takes 32 bit addresses. Copy char somewhere prom can
 *	find it. This routine will stop working after pmap_rid_of_console 
 *	is called in alpha_init. This is due to the hard coded address
 *	of the console area.
 *
 */

pre_prom_putc(c)
	unsigned char c;
{
	unsigned char *to = (unsigned char *)0x20000000;

	if (c == '\n') {
		strcpy ((char*)to,"\r\n");
		prom_puts(alpha_console,to,2);
	} else {
		*to = c;
		prom_puts(alpha_console,to,1);
	}
}

alpha_con_putc (unit, line,c)
{
	pre_prom_putc(c);
}
/*
 *	Object:
 *		pre_prom_getc				EXPORTED function
 *
 *		Wait for keystroke
 *
 *	Wait for the prom to get a real char and pass it back.
 *
 */

unsigned char
pre_prom_getc(wait,raw)
	boolean_t	wait;
	boolean_t	raw;

{
	unsigned long	val;
	int		code;

	do {
		val = prom_getc(alpha_console);
		code = val >> 61; /* the status is bits 63:61 */
	} while (((code != 0) && (code != 1)) && wait); /* 0 = success
					       * 1 = success, more ready
					       * other are failures
					       */

	return((unsigned char) val ); /* Pass back the char (lower 8 bits) */
}

int
alpha_con_getc (unit, line,wait,raw)
{
	unsigned char c = pre_prom_getc(wait,raw);
	return (c) ? c : -1;
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
	_doprnt(fmt, &listp, pre_prom_putc, 16);
	va_end(listp);
}

/*
 *	Object:
 *		prom_reboot			EXPORTED function
 *
 *		Reboots the system up.
 *
 * NOTE: it would be nice to export from the kernel a way to
 * specify which image to reboot to, but not all architectures
 * can do it. Sigh.
 */
void
prom_reboot()
{
	struct restart_blk	*r;
	struct per_cpu_slot	*p;
	int			offset;

	r = alpha_hwrpb;

	offset = r->percpu_slot_size * cpu_number();
	p = (struct per_cpu_slot *) ((char*)r +	r->percpu_slots_offset +
						offset);

	(void) splhigh();
	alpha_reset_before_reboot();

	p->state_flags = PSTATE_H_WARM_BOOT | PSTATE_PL | PSTATE_OH;
	wbflush();
	halt();
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

	return (alpha_btop(pmap_extract(pmap_kernel(), mtime)));
}

