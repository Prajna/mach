/*
 * Mach Operating System
 * Copyright (c) 1993,1992 Carnegie Mellon University
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
 * $Log:	alpha_init.c,v $
 * Revision 2.5  93/05/15  19:10:51  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.4  93/03/11  13:57:46  danner
 * 	Corrected boot time page stealing commentary.
 * 	[93/03/11            danner]
 * 
 * Revision 2.3  93/03/09  10:49:03  danner
 * 	GCC quiets, protos, standard boothowto, lint.
 * 	[93/03/05            af]
 * 
 * Revision 2.2  93/02/05  07:57:03  danner
 * 	No more ISP hacks.  Now we parse args.
 * 	Fixed bug in memory stealing call.
 * 	[93/02/04  00:56:23  af]
 * 
 * 	MP Icache sanity call, help Jeffrey in dprintf by delaying
 * 	dropping of bootstrap VM spaces.
 * 	[93/01/15            af]
 * 	Set vm_page_big_pagenum to support vm_page_grab_contiguous_pages.
 * 	[92/12/25  01:42:54  af]
 * 
 * 	Added reference to doc for the HWRPB &co.
 * 	[92/12/22            af]
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:11:03  af]
 * 
 * 	Created.
 * 	[92/06/03            af]
 * 
 */
/*
 *	File: alpha_init.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Basic initialization for Alpha
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 *
 *	"VMS for Alpha Platforms Internals and Data Structures"
 *	Digital Press 1992, Burlington, MA 01803
 *	Order number EY-L466E-P1/2, ISBN 1-55558-095-5
 *	[Especially volume 1, chapter 33 "Bootstrap processing"]
 */

#include <mach_kdb.h>

#include <machine/machspl.h>		/* spl definitions */
#include <kern/thread.h>
#include <alpha/alpha_cpu.h>
#include <sys/reboot.h>
#include <mach/vm_param.h>
#include <alpha/prom_interface.h>
#include <alpha/thread.h>

/*
 *	Memory
 */
vm_offset_t	memory_start, avail_start, avail_end;
vm_offset_t	virtual_avail, virtual_end;
vm_size_t	mem_size;
vm_size_t	memlimit = 0;		/* patchable to run in less memory */

vm_size_t	(*alphabox_steal_memory)() = 0;

extern char	end[], edata[];
char		*esym = (char *)0;

extern struct pcb boot_pcb;

#if	MACH_KDB
int		boothowto = RB_KDB;
#else	MACH_KDB
int		boothowto = 0;
#endif	MACH_KDB

static init_memory( vm_offset_t first_page);	/* forward */

alpha_init( boolean_t gcc_compiled )
{
	vm_offset_t first_page;	/* first free physical page */
	int i;
	struct thread fake_th;	/* while sizing memory, and for KDB */
	struct task fake_tk;
	extern int pmap_max_asn;

	/*
	 *	Only the master cpu can get here.
	 */

	(void) splhigh();			/* sanity precautions */
	tbia();
	alphacache_Iflush();

	init_prom_interface();

	/*
	 *	Move symbol table and bootstrap code
	 *	out of BSS.
	 */
	pmap_max_asn = alpha_hwrpb->maximum_asn;
	PAGE_SIZE = alpha_hwrpb->page_size;
	vm_set_page_size();

	first_page = move_bootstrap();

	/*
	 * Tune the delay() function
	 */
	{
		extern natural_t	machine_cycles_per_usec;

		machine_cycles_per_usec =
			alpha_hwrpb->cycle_counter_resolution / 1000000;
	}

#if	MACH_KDB
	kdb_init(gcc_compiled);
#endif

	bzero(edata, end - edata);	/* zero bss */

	/*
	 *	Fake some variables so that we can take traps right away.
	 *	This is important while sizing memory, and in case KDB
	 *	is invoked before the first thread is set up.
	 *
	 *	We will also be allocating VM before switching to the
	 *	first thread, so we need to setup our HW process context
	 *	immediately.
	 */
	bzero(&fake_th, sizeof fake_th);
	bzero(&fake_tk, sizeof fake_tk);
	fake_th.task = &fake_tk;
	fake_th.pcb = &boot_pcb;
	boot_pcb.mss.framep = &boot_pcb.mss.saved_frame;

	{
		extern pt_entry_t	root_kpdes[];
		boot_pcb.mss.hw_pcb.ptbr =
			alpha_btop(K0SEG_TO_PHYS(root_kpdes));
	}
	set_current_thread(&fake_th);
	master_cpu = cpu_number();
	active_threads[master_cpu] = &fake_th;
	swpctxt(K0SEG_TO_PHYS(&boot_pcb), &boot_pcb.mss.hw_pcb.ksp);

	/*
	 *	Stop the clock for now.
	 */
	stopclocks();

	set_root_name();

	/*
	 * First available page of phys mem
	 */
	first_page = alpha_btop(K0SEG_TO_PHYS(alpha_round_page(first_page)));

	/*
	 * Parse command line, setting appropriate switches and
	 * configuration values.
	 * NOTE: Before cnprobe() is called you must do dprintf;
	 * printf won't work.
	 */

	/* First set some defaults */

	parse_args();

	init_memory(first_page);

	/*
	 * Initialize the machine-dependent portion of the VM system
	 */
	pmap_bootstrap();

	alpha_box_model_dep();		/* cold adaptation */

	/* if we need big contiguous chunks then we do */
	if (alphabox_steal_memory) {
		/*
		 * Boot time page stealing. Good reasons so far:
		 * - console data structures (shared screen info)
		 */
		vm_size_t		needed;

		needed = (*alphabox_steal_memory)(PHYS_TO_K0SEG(avail_start));
		avail_start += round_page(needed);
	}

	cons_find(1);		/* initialize console device: MUST be there */
#if 0
	pmap_rid_of_console();	/* here, whence debuged */
#endif

#if	MACH_KDB
	if ((boothowto&RB_HALT) && (boothowto&RB_KDB))
		gimmeabreak();
#endif	MACH_KDB

	printf("Alpha boot: memory from 0x%x to 0x%x\n",
	       memory_start, avail_end);
	printf("Kernel virtual space from %#X to %#X.\n",
		       virtual_avail, virtual_end);
	printf("Available physical space from %#X to %#X\n",
			avail_start, avail_end);

#if 1
	pmap_rid_of_console();
#endif

	machine_startup();

}

#if	(NCPUS > 1)

alpha_slave_init()
{
	struct thread fake_th;	/* while sizing memory, and for KDB */
	struct task fake_tk;

	splhigh();

	bzero(&fake_th, sizeof fake_th);
	bzero(&fake_tk, sizeof fake_tk);
	fake_th.task = &fake_tk;
	fake_th.pcb = &boot_pcb;
	boot_pcb.mss.framep = &boot_pcb.mss.saved_frame;
	active_threads[cpu_number()] = &fake_th;
	set_current_thread(&fake_th);
	swpctxt(K0SEG_TO_PHYS(&boot_pcb), &boot_pcb.mss.hw_pcb.ksp);

	tbia();
	alphacache_Iflush();

	slave_main();
}
#endif	(NCPUS > 1)

static
init_memory( vm_offset_t first_page)
{
	register vm_size_t	i, j, max;
	extern vm_size_t	vm_page_big_pagenum;
	struct memory_data_descriptor_table	*mddt;

	/*
	 *	See how much memory we have
	 */
	mddt = (struct memory_data_descriptor_table *)
		((char*)alpha_hwrpb + alpha_hwrpb->memory_data_descriptor_table_offset);

	max = memlimit ? alpha_btop(memlimit) : alpha_btop(K0SEG_SIZE);
	i = first_page;
	for (j = 0; j < mddt->num_clusters; j++) {
		vm_size_t	first, last;

		first = mddt->mem_clusters[j].first_pfn;
		last = first + mddt->mem_clusters[j].num_pfn;

		if (i >= first && i < last) {
			memory_start = alpha_ptob(first);
			i = last;
			break;
		}
	}

	/*
	 *	If we should run in less memory then we should.
	 */
	if (memlimit && (i > max))
		i = max;
	vm_page_big_pagenum = i;
	mem_size = alpha_ptob(i);

	/*
	 *	Notify the VM system of what memory looks like
	 */
	avail_start = (vm_offset_t)alpha_ptob(first_page);
	avail_end   = (vm_offset_t)mem_size;
}

