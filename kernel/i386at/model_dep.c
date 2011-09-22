/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989, 1988 Carnegie Mellon University
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
 * $Log:	model_dep.c,v $
 * Revision 2.11.2.1  93/11/29  15:33:50  rvb
 * 	Fix dma > 16 meg.
 * 	[93/11/29            rvb]
 * 
 * Revision 2.11  93/08/10  15:57:58  mrt
 * 	Look for the rconsole serial line FIRST.
 * 	[93/06/17  22:43:15  rvb]
 * 
 *	Added extern (patchable) use_all_mem variable. It defaults to
 *	0 which limits memory use to 16M. If set to 1 all memory will
 *	be used, but DMA from higher memory does not work.
 * 	[93/06/08            rvb]
 * 
 * Revision 2.10  93/05/15  19:33:02  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.9  93/01/14  17:31:41  danner
 * 	Proper spl typing.
 * 	[92/11/30            af]
 * 
 * Revision 2.8  92/07/09  22:54:52  rvb
 * 	GROSS temporary (?) hack to limit memory to 16Meg to not freak
 * 	out ISA machines doing dma.
 * 	[92/06/18            rvb]
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
 *	Basic initialization for I386 - ISA bus machines.
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
#include <i386/machspl.h>

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
	proberc();
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

	printf(version);

	machine_slot[0].is_cpu = TRUE;
	machine_slot[0].running = TRUE;
	machine_slot[0].cpu_type = CPU_TYPE_I386;
	machine_slot[0].cpu_subtype = CPU_SUBTYPE_AT386;

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
int use_all_mem = 0;
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
#include <scsi.h>
#if	NSCSI > 0
	/*
	 * Only scsi still has dma problems
	 */
	if ((!use_all_mem) && last_addr - loadpt > 16 * 1024*1024) {
		printf("************ i386at/model_dep.c: i386_init(); ");
		printf("Limiting useable memory to 16 Meg to avoid DMA problems.\n");
		last_addr = loadpt + 16 * 1024*1024;		/* XXX */
	}
#endif	/* NSCSI > 0 */
	mem_size = last_addr - loadpt;

	first_addr = round_page(first_addr);
	last_addr = trunc_page(last_addr);

	/*
	 * Steal physical pages for devices that will do DMA
	 */
#include <fd.h>
#if	NFD > 0
	 {
		extern vm_offset_t FdDmaThreshold;
		extern int FdDmaEISA;
		if (last_addr > FdDmaThreshold && !FdDmaEISA) {
			extern vm_offset_t FdDmaPage;
		 	FdDmaPage = first_addr;
			first_addr += PAGE_SIZE * (NFD>>1);	 
		}
	 }
#endif	/* NFD > 0 */

	/*
	 *	Initialize kernel physical map, mapping the
	 *	region from loadpt to avail_start.
	 *	Kernel virtual address starts at VM_KERNEL_MIN_ADDRESS.
	 */

	avail_start = first_addr;
	avail_end = last_addr;
	printf("AT386 boot: memory from 0x%x to 0x%x\n", avail_start,
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

	printf("i386_init: virtual_avail = %x, virtual_end = %x\n",
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
	    spl_t	s = splhigh();
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
