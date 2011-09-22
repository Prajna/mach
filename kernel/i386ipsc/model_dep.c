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
 * $Log:	model_dep.c,v $
 * Revision 2.13  93/01/14  17:32:12  danner
 * 	Proper spl typing.
 * 	[92/12/10  17:54:33  af]
 * 
 * Revision 2.12  92/01/03  20:12:48  dbg
 * 	Rename kdb_init to ddb_init.
 * 	Use '-d' switch (RB_KDB) for initial debugger breakpoint.
 * 	[91/12/18            dbg]
 * 
 * Revision 2.11  91/12/10  16:30:02  jsb
 * 	Fixes from Intel
 * 	[91/12/10  15:32:17  jsb]
 * 
 * Revision 2.10  91/08/03  18:18:07  jsb
 * 	Force RB_ASKNAME into boothowto, since we cannot do so at boot time.
 * 	[91/07/25  18:40:17  jsb]
 * 
 * 	Added node_self(), _node_self.
 * 	[91/07/24  23:15:52  jsb]
 * 
 * Revision 2.9  91/07/31  17:42:57  dbg
 * 	Remove call to pcb_module_init (in machine-independent code).
 * 	[91/07/26            dbg]
 * 
 * Revision 2.8  91/07/01  08:24:16  jsb
 * 	Temporary hack to prevent kernel text and data from being used
 * 	as free memory.
 * 	[91/06/29  17:48:16  jsb]
 * 
 * Revision 2.7  91/06/19  11:55:52  rvb
 * 	cputypes.h->platforms.h
 * 	[91/06/12  13:45:41  rvb]
 * 
 * Revision 2.6  91/06/18  20:50:23  jsb
 * 	New copyright from Intel.
 * 	[91/06/18  20:07:00  jsb]
 * 
 * Revision 2.5  91/06/17  15:44:12  jsb
 * 	Changed intialization printf.
 * 	[91/06/17  10:08:20  jsb]
 * 
 * Revision 2.4  91/06/06  17:04:49  jsb
 * 	Changes for new bootstrapper; see asm_startup.h.
 * 	[91/05/13  17:08:15  jsb]
 * 
 * Revision 2.3  91/05/18  14:30:45  rpd
 * 	Changed pmap_bootstrap arguments.
 * 	Moved pmap_free_pages and pmap_next_page here.
 * 	[91/05/15            rpd]
 * 
 * Revision 2.2  91/05/08  12:46:18  dbg
 * 	Initialization for iPSC only.
 * 	Add code that was in i386/init.c.
 * 	Add pmap_valid_page.
 * 	[91/04/26  14:41:16  dbg]
 * 
 * Revision 2.3  90/09/23  17:45:10  jsb
 * 	Added support for iPSC2.
 * 	[90/09/21  16:39:41  jsb]
 * 
 * Revision 2.2  90/05/03  15:27:39  dbg
 * 	Alter for pure kernel.
 * 	[90/02/15            dbg]
 * 
 * Revision 1.5.1.4  90/02/01  13:36:37  rvb
 * 	esym must always be defined.  This is as good a place as any.
 * 	[90/01/31            rvb]
 * 
 * Revision 1.5.1.3  89/12/28  12:43:10  rvb
 * 	Fix av_start & esym initialization, esp for MACH_KDB.
 * 	[89/12/26            rvb]
 * 
 * Revision 1.5.1.2  89/12/21  17:59:49  rvb
 * 	enable esym processing.
 * 
 * 
 * Revision 1.5.1.1  89/10/22  11:30:41  rvb
 * 	Setup of rootdevice should not be here.  And it was wrong.
 * 	[89/10/17            rvb]
 * 
 * 	Scary!  We've changed sbss to edata.  AND the coff loader
 * 	following the vuifile spec was actually aligning the bss 
 * 	on 4k boundaries.
 * 	[89/10/16            rvb]
 * 
 * Revision 1.5  89/04/05  12:57:39  rvb
 * 	Move extern out of function scope for gcc.
 * 	[89/03/04            rvb]
 * 
 * Revision 1.4  89/02/26  12:31:25  gm0w
 * 	Changes for cleanup.
 * 
 * 31-Dec-88  Robert Baron (rvb) at Carnegie-Mellon University
 *	Derived from MACH2.0 vax release.
 *
 */

/*
 *	File:	i386_init.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Copyright (C) 1986, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Basic initialization for I386
 */

#include <platforms.h>
#include <mach_kdb.h>

#include <mach/i386/vm_param.h>

#include <mach/machine.h>
#include <mach/vm_param.h>
#include <mach/vm_prot.h>
#include <vm/vm_page.h>
#include <kern/time_out.h>
#include <sys/time.h>


#if	MACH_KDB
#include <sys/reboot.h>
#endif	MACH_KDB

#define	HOLE	1

int		loadpt;

vm_size_t	mem_size;
vm_size_t	rawmem_size;
vm_offset_t	first_addr = 0;	/* set by start.s - keep out of bss */
vm_offset_t	first_avail = 0;/* first address after page tables */
vm_offset_t	last_addr;
#if	HOLE
vm_offset_t	hole_start, hole_end;
#endif	HOLE

vm_offset_t	avail_start, avail_end;
vm_offset_t	virtual_avail, virtual_end;
vm_offset_t	avail_next;
unsigned int	avail_remaining;

int		boottype = 0;
char		*esym = (char *)0;

/* parameters passed from NX bootstrap loader */
int		ipsc_basemem = 0;	/* must be in .data section */
int		ipsc_physnode = 0;
int		ipsc_slot = 0;
char		*tmp_bootenv = 0;	/* must be in .data section */
extern char	**bootenv, **envcopy();
extern int	boothowto;

extern char	edata, end;

extern char	version[];

extern void	setup_main();

void		inittodr();	/* forward */

int		rebootflag = 0;	/* exported to kdintr */

int		_node_self = -1;

extern unsigned char *md_address;
extern unsigned long md_size;

/*
 *	Functional form, used by most machine independent code.
 */
node_self()
{
	return _node_self;
}

/*
 *	Cpu initialization.  Running virtual, but without MACH VM
 *	set up.  First C routine called.
 */
void machine_startup()
{
	/*
	 * Do basic VM initialization
	 */
	i386_init();

	/*
	 * Initialize the console so we can print.
	 */
	cninit();
#if	MACH_KDB

	/*
	 * Initialize the kernel debugger.
	 */
	ddb_init();

	/*
	 * Cause a breakpoint trap to the debugger before proceeding
	 * any further if RB_KDB (the '-d' switch to the boot program)
	 * was specified in the boot flags.
	 */
	if (boothowto & RB_KDB) {
		Debugger();
	}

#if 1 /* stan set to 0 [10-14-91] */
	/*
	 * XXX Until bootcube or something else allows us to dynamically
	 * XXX provide boot options, always ask root and bootstrap names.
	 */
	boothowto |= RB_ASKNAME;
	boothowto |= (RB_ASKNAME << RB_SHIFT);
#endif
#endif	MACH_KDB

printf("EXPERIMENTAL ");
	printf(version);
#if 0
{
        int     i;

        db_printf("boot magic:\n");
        for (i = 0; bootenv[i]; i++) {
                db_printf("%d %x %s\n", i, bootenv[i], bootenv[i]);
        }
}
#endif

	machine_slot[0].is_cpu = TRUE;
	machine_slot[0].running = TRUE;
	machine_slot[0].cpu_type = CPU_TYPE_I386;
	machine_slot[0].cpu_subtype = CPU_SUBTYPE_iPSC386;

	/*
	 * Start the system.
	 */
	setup_main();
}

/*
 * Find devices.  The system is alive.
 */
void machine_init()
{
	/*
	 * Set up to use floating point.
	 */
	init_fpu();

	/*
	 * Find the devices
	 */
	probeio();

	/*
	 * Find the root device
	 */
	get_root_device();

	/*
	 * Get the time
	 */
	inittodr();
}

/*
 * Halt a cpu.
 */
halt_cpu()
{
}

/*
 * Halt the system or reboot.
 */
halt_all_cpus(reboot)
	boolean_t	reboot;
{
	rebootflag = 1;
	printf("In tight loop: hit ctl-alt-del to reboot\n");
	(void) spl0();
	for (;;)
	    continue;

}

startrtclock()
{
	clkstart();
}

void
inittodr()
{
	time_value_t	new_time;

	new_time.seconds = 0;
	new_time.microseconds = 0;

	(void) readtodc(&new_time.seconds);

	{
	    spl_t s = splhigh();
	    time = new_time;
	    splx(s);
	}
}

void
resettodr()
{
	writetodc();
}

/*
 * Basic VM initialization.
 */
i386_init()
{
	/*
	 * Clear the BSS.
	 */
	bzero((char *)&edata,(unsigned)(&end - &edata));

	/*
	 * Set _node_self
	 */
	_node_self = ipsc_physnode;

	/*
	 * Set ram-disk parameters and fix esym
	 */
XXX	md_address = (unsigned char *)esym - md_size;
XXX	esym = (char *)md_address;
#if 1
	bootenv = envcopy(tmp_bootenv); /* bootmagic! */
#endif

	/*
	 * Initialize the pic prior to any possible call to an spl.
	 */
	picinit();

	vm_set_page_size();

	/*
	 *	Find the size of memory.
	 */
	last_addr = ipsc_basemem;
	mem_size = last_addr - loadpt;

	first_addr = 0;
	last_addr = trunc_page(last_addr);

	/*
	 *	Initialize kernel physical map, mapping the
	 *	region from loadpt to avail_start.
	 *	Kernel virtual address starts at VM_KERNEL_MIN_ADDRESS.
	 */

	avail_start = first_addr;
	avail_end = last_addr;
	printf("i386 iPSC boot: ");
	printf("node %d, slot %d, memory from 0x%x to 0x%x (%d KB)\n",
	       ipsc_physnode, ipsc_slot, avail_start, avail_end,
	       (avail_end - avail_start) / 1024);
	printf("edata %08X  end %08X  esym %08X\n",
		&edata, &end, esym);

	pmap_bootstrap(loadpt);

	/*
	 *	Initialize for pmap_free_pages and pmap_next_page.
	 */

#if	HOLE

	hole_start =	0x100000; /* loadpt */
	hole_end =	(vm_offset_t)(((long)md_address + md_size - 0xC0000000 +
					PAGE_SIZE-1) & ~(PAGE_SIZE-1));

	avail_remaining = atop((avail_end - avail_start) -
			       (hole_end - hole_start));
#else	HOLE

	avail_remaining = atop(avail_end - avail_start);

#endif	HOLE

	avail_next = avail_start;

	printf("i386_init: virtual_avail = %x, virtual_end = %x\n",
		virtual_avail, virtual_end);

	/*
	 * XXX I'll fix it when I understand it...
	 */
	vm_page_free_count -= ((round_page(&esym) & 0x0fffffff) / PAGE_SIZE);
}


#include <mach/vm_prot.h>
#include <vm/pmap.h>
#include <mach/time_value.h>

timemmap(dev,off,prot)
	vm_prot_t prot;
{
	extern time_value_t *mtime;

#ifdef	lint
	dev++; off++;
#endif	lint

	if (prot & VM_PROT_WRITE) return (-1);

	return (i386_btop(pmap_extract(pmap_kernel(), (vm_offset_t) mtime)));
}

unsigned int pmap_free_pages()
{

	printf("Call pmap_free_pages, now %d range %d-%d\n",
		avail_remaining, avail_next, avail_end);
	return avail_remaining;
}

boolean_t pmap_next_page(addrp)
	vm_offset_t *addrp;
{
	if (avail_next == avail_end)
		return FALSE;

	*addrp = avail_next;

#if	HOLE
	/* skip the hole */

	if (avail_next == hole_start)
		avail_next = hole_end;
#endif	HOLE

	avail_next += PAGE_SIZE;
	avail_remaining--;
pchk(*addrp, pmap_next_page, esym);
	return TRUE;
}

boolean_t pmap_valid_page(x)
	vm_offset_t x;
{
	return (avail_start <= x) && (x < avail_end);
}

pchk(a, s, e)
	unsigned long a, s, e;
{
#define NSTART 1
	static n = NSTART;

	s &= ~(PAGE_SIZE-1);
	if (s >= 0xC0000000)
		s -= 0xC0000000;
	e &= ~(PAGE_SIZE-1);
	if (e >= 0xC0000000)
		e -= 0xC0000000;
	if (n == NSTART) {
		printf("Check at 0x%08X against 0x%08X-0x%08X\n", a, s, e);
		n--;
	} else if (a >= s && a < e && n) {
		printf("Giving away 0x%08X\n", a);
		n--;
	} else if (a == s - PAGE_SIZE || a == s ||
		   a == e - PAGE_SIZE || a == e) {
		printf("Giving away 0x%08X\n", a);
	}
}
