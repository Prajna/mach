/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
 * Copyright (c) 1991 IBM Corporation 
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation,
 * and that the name IBM not be used in advertising or publicity 
 * pertaining to distribution of the software without specific, written
 * prior permission.
 * 
 * CARNEGIE MELLON AND IBM ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON AND IBM DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * COMPONENT_NAME: (I386) Mach port to i386 platform
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 6, 27
 */
/*
 * HISTORY
 * $Log:	model_dep.c,v $
 * Revision 2.2  93/02/04  08:01:21  danner
 * 	Initialize NMI here.
 * 	[92/03/30            dbg@ibm]
 * 
 * 	Add io_space_offset_to_addr - first step in IO space object
 * 	[92/03/22            dbg@ibm]
 * 
 * 	PS2 version
 * 	[92/02/24            dbg@ibm]
 * 
 * Revision 2.7  92/01/03  20:11:49  dbg
 * 	Fixed so that mem_size can be patched to limit physical memory
 * 	use.
 * 	[91/09/29            dbg]
 * 
 * 	Rename kdb_init to ddb_init.  Remove esym.
 * 	[91/09/11            dbg]
 * 
 * Revision 2.6  91/07/31  17:42:41  dbg
 * 	Remove call to pcb_module_init (in machine-independent code).
 * 	[91/07/26            dbg]
 * 
 * Revision 2.5  91/06/19  11:55:48  rvb
 * 	cputypes.h->platforms.h
 * 	[91/06/12  13:45:37  rvb]
 * 
 * Revision 2.4  91/05/18  14:30:38  rpd
 * 	Changed pmap_bootstrap arguments.
 * 	Moved pmap_free_pages and pmap_next_page here.
 * 	[91/05/15            rpd]
 * 
 * Revision 2.3  91/05/14  16:29:13  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/05/08  12:44:52  dbg
 * 	Initialization for i386 AT bus machines only.
 * 	Combine code that was in i386/init.c and i386/i386_init.c.
 * 	[91/04/26  14:40:43  dbg]
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
 *	File:	model_dep.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Copyright (C) 1986, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Basic initialization for I386 - PS2 (MCA bus) machines.
 */

#include <platforms.h>
#include <mach_kdb.h>

#include <mach/i386/vm_param.h>

#include <mach/vm_param.h>
#include <mach/vm_prot.h>
#include <mach/machine.h>
#include <kern/time_out.h>
#include <sys/time.h>
#include <vm/vm_page.h>

#if	MACH_KDB
#include <sys/reboot.h>
#endif	MACH_KDB

int		loadpt;

vm_size_t	mem_size = 0;
vm_size_t	rawmem_size;
vm_offset_t	first_addr = 0;	/* set by start.s - keep out of bss */
vm_offset_t	first_avail = 0;/* first after page tables */
vm_offset_t	last_addr;

vm_offset_t	avail_start, avail_end;
vm_offset_t	virtual_avail, virtual_end;
vm_offset_t	hole_start, hole_end;
vm_offset_t	avail_next;
unsigned int	avail_remaining;

/* parameters passed from bootstrap loader */
int		cnvmem = 0;		/* must be in .data section */
int		extmem = 0;
int		boottype = 0;

extern char	edata, end;

extern char	version[];

extern void	setup_main();

void		inittodr();	/* forward */

int		rebootflag = 0;	/* exported to kdintr */

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
	 * any further if the proper option bit was specified in
	 * the boot flags.
	 *
	 * XXX use -a switch to invoke kdb, since there's no
	 *     boot-program switch to turn on RB_HALT!
	 */
	if (boothowto & RB_ASKNAME)
	    Debugger();
#endif	MACH_KDB

	pos_init();		/* get POS registers */
	abios_init();           /* set up ABIOS */

#if     MACH_RDB
	printf("entering db_kdb\n");
	db_kdb(-1,0,0);
#endif  /* MACH_RDB */

	printf(version);

	machine_slot[0].is_cpu = TRUE;
	machine_slot[0].running = TRUE;
	machine_slot[0].cpu_type = CPU_TYPE_I386;
	machine_slot[0].cpu_subtype = CPU_SUBTYPE_PS2;

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

	/*
	 * Enable NMI interrupts.
	 */
	nmi_enable();
}

/*
 * Halt a cpu.
 */
void
halt_cpu()
{
  printf("\n*** HALTED ***\n");
  (void) spl0();
  for (;;)
    continue;
}

/*
 * Halt the system or reboot.
 */
halt_all_cpus(reboot)
	boolean_t	reboot;
{
	if (reboot) {
	    /*
	     * Tell the BIOS not to clear and test memory.
	     */
	    *(unsigned short *)phystokv(0x472) = 0x1234;

	    kdreboot();
	}
	else {
	    rebootflag = 1;
	    printf("In tight loop: hit ctl-alt-del to reboot\n");
	    (void) spl0();
	}
	for (;;)
	    continue;
}

/*
 * Basic VM initialization.
 */
i386_init()
{
	/*
	 * Zero the BSS.
	 */
	bzero((char *)&edata,(unsigned)(&end - &edata));

	/*
	 * Initialize the pic prior to any possible call to an spl.
	 */
	picinit();

	vm_set_page_size();

	/*
	 * Compute the memory size.
	 */
	first_addr = 0x1000;
		/* BIOS leaves data in low memory */
	last_addr = 1024*1024 + extmem*1024;
		/* extended memory starts at 1MB */
	if (mem_size != 0) {
	    if (mem_size < last_addr - loadpt)
		last_addr = loadpt + mem_size;
	}
	mem_size = last_addr - loadpt;

	first_addr = round_page(first_addr);
	last_addr = trunc_page(last_addr);

	/*
	 *	Initialize kernel physical map, mapping the
	 *	region from loadpt to avail_start.
	 *	Kernel virtual address starts at VM_KERNEL_MIN_ADDRESS.
	 */

	avail_start = first_addr;
	avail_end = last_addr;
	printf("PS2 boot: memory from 0x%x to 0x%x\n", avail_start,
			avail_end);

	pmap_bootstrap(loadpt);

	/*
	 *	Initialize for pmap_free_pages and pmap_next_page.
	 *	These guys should be page-aligned.
	 */

	hole_start = trunc_page((vm_offset_t)(1024 * cnvmem));
	hole_end = round_page((vm_offset_t)first_avail);

	avail_remaining = atop((avail_end - avail_start) -
			       (hole_end - hole_start));
	avail_next = avail_start;

	printf("PS2_init: virtual_avail = %x, virtual_end = %x\n",
		virtual_avail, virtual_end);

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
	    int	s = splhigh();
	    time = new_time;
	    splx(s);
	}
}

void
resettodr()
{
	writetodc();
}

unsigned int pmap_free_pages()
{
	return avail_remaining;
}

boolean_t pmap_next_page(addrp)
	vm_offset_t *addrp;
{
	if (avail_next == avail_end)
		return FALSE;

	/* skip the hole */

	if (avail_next == hole_start)
		avail_next = hole_end;

	*addrp = avail_next;
	avail_next += PAGE_SIZE;
	avail_remaining--;
	return TRUE;
}

boolean_t pmap_valid_page(x)
	vm_offset_t x;
{
	return (((avail_start <= x) && (x < avail_end)) &&
		!((hole_start <= x) && (x < hole_end)));
}

/*
 * Given an offset into the IO space object:
 * return (TRUE, phys_addr) if the offset is valid,
 * return (FALSE, *)	    if the offset is invalid.
 */
boolean_t
io_space_offset_to_addr(offset, phys_addr)
	vm_offset_t	offset;
	vm_offset_t	*phys_addr;	/* OUT */
{
	/*
	 * Include:
	 * BIOS vectors:	0..3ff
	 * BIOS data:	      400..5ff
	 * ABIOS data: hole_start..9ffff
	 * IO space:	    a0000..fffff
	 *
	 * and eventually the rest of the 24-bit and
	 * 32-bit IO spaces...
	 */
        if (offset <= 0x1000 ||
	    (offset >= hole_start && offset < 0x100000))
	{
	    *phys_addr = offset;
	    return TRUE;
	}
	return FALSE;
}

