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
 * $Log:	mips_init.c,v $
 * Revision 2.23  93/05/17  10:26:11  rvb
 * 	Type casts, etc to quiet gcc 2.3.3 warnings
 * 
 * Revision 2.22  93/05/15  19:13:10  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.21  93/02/05  08:04:22  danner
 * 	Clock got more generic.
 * 	[93/02/04  01:58:47  af]
 * 
 * Revision 2.19  92/01/03  20:25:14  dbg
 * 	Move symbol table and bootstrap image out of BSS at startup.
 * 	[91/09/04            dbg]
 * 
 * Revision 2.18  91/08/29  13:07:48  jsb
 * 	New proms on 3max problem: getenv copies strings on the
 * 	stack sometimes.  Fixed default_get_memory_bitmap() accordingly.
 * 	[91/08/29            af]
 * 
 * Revision 2.17  91/08/24  12:23:15  af
 * 	Some calls that lived in assembly live here now.
 * 	Identify proms real quick now, getenv depends on it.
 * 	More flexibility in finding the bitmap of valid pages.
 * 	Made contiguous page stealing a MI feature.
 * 	[91/08/02  03:23:48  af]
 * 
 * Revision 2.16  91/06/19  11:56:24  rvb
 * 	#ifdef PMAX -> #ifdef DECSTATION and we include <platforms.h>
 * 	[91/06/12  14:09:50  rvb]
 * 
 * 	cons_enable() now takes an argument -- which should be 1.
 * 	[91/06/07            rvb]
 * 
 * Revision 2.15  91/05/18  14:36:07  rpd
 * 	Removed zalloc_physical.
 * 	Removed arguments to pmap_bootstrap.
 * 	[91/03/22            rpd]
 * 
 * Revision 2.14  91/05/14  17:35:58  mrt
 * 	Correcting copyright
 * 
 * Revision 2.13  91/02/05  17:49:43  mrt
 * 	Added author notices
 * 	[91/02/04  11:23:44  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:27:28  mrt]
 * 
 * Revision 2.12  91/01/08  15:50:46  rpd
 * 	Changed for pcb reorganization.
 * 	[90/11/12            rpd]
 * 
 * Revision 2.11  90/12/05  23:37:58  af
 * 
 * 
 * Revision 2.10  90/12/05  20:49:54  af
 * 	Zero out memory on 3maxen too: this cures the "dropping
 * 	paging request because of paging errors" problem.
 * 	Renames for new copyright free PMAX drivers.
 * 
 * 	Moved cold adaptation later, my (new) TC code now looks at
 * 	all TC slots and does take buserrors.  Removed MSERIES-specific
 * 	code, which would live in the model_dep() code for such
 * 	machines [if I had one to test against].
 * 	[90/12/03  23:03:43  af]
 * 
 * Revision 2.9  90/09/09  23:21:02  rpd
 * 	Removed environ: use prom_getenv() if you need to query the prom.
 * 	[90/09/05            af]
 * 
 * Revision 2.8  90/08/27  22:08:40  dbg
 * 	KDB does not have a pcb anymore, use a static local one in 
 * 	the boot phase (but I think we do not need it).
 * 	Factor out from the MI code the end-of-symtab info, which
 * 	is now the esym variable that most other machines use.
 * 	[90/08/20  10:19:56  af]
 * 
 * 	Renames for new debugger.
 * 	[90/08/17  23:43:59  af]
 * 
 * Revision 2.7  90/08/07  22:29:42  rpd
 * 	3max support.
 * 	[90/08/07  15:14:01  af]
 * 
 * Revision 2.5.1.1  90/06/11  11:29:41  af
 * 	3max support.  New memory sizing routine that just uses the prom
 * 	info, apply cold model-dependent adaptations, officialize graphic
 * 	driver hack (all drivers need to allocate page-aligned memory to
 * 	be mapped to user space).
 * 
 * Revision 2.5  90/05/29  18:38:12  rwd
 * 	Set boothowto to RB_KDB.
 * 	[90/05/25            rwd]
 * 
 * Revision 2.4  90/01/22  23:07:29  af
 * 	Since all is well, default boothowto to nothing.
 * 	[89/12/22            af]
 * 
 * Revision 2.3  89/12/08  19:49:07  rwd
 * 	Change #ifdef MACH_KDB to #if MACH_KDB
 * 	[89/12/06            rwd]
 * 
 * Revision 2.2  89/11/29  14:14:39  af
 * 	Reflected ethernet name changes.
 * 	[89/11/26  10:32:30  af]
 * 
 * 	Fake a thread on the stack so that KDB will work if we got
 * 	started with boothowto->stop.
 * 	[89/11/03  16:26:19  af]
 * 
 * 	Massive changes for pure kernel.
 * 	[89/10/29            af]
 * 
 * Revision 2.6  89/09/22  13:57:47  af
 * 	If kdb is available, do not clobber the symtab that the (Mach)
 * 	loader put after the bss.
 * 	[89/09/14            af]
 * 
 * Revision 2.5  89/08/28  22:39:09  af
 * 	Reflected tlb naming changes.
 * 	[89/08/28  17:21:04  af]
 * 
 * Revision 2.4  89/08/08  21:49:13  jsb
 * 	zalloc_physical -> 400k
 * 	[89/08/03            rvb]
 * 
 * Revision 2.3  89/07/14  15:28:11  rvb
 * 	zalloc_physical set to 300k.
 * 	[89/07/14            rvb]
 * 
 * Revision 2.2  89/05/31  12:31:00  rvb
 * 	Clean up. [af]
 * 
 * Revision 2.1  89/05/30  12:55:53  rvb
 * Rcs'ed
 *
 *  2-Feb-89  Alessandro Forin (af) at Carnegie-Mellon University
 *	Created.
 */
/*
 *	File: mips_init.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	2/89
 *
 *	Basic initialization for Mips.
 *
 */

#include <platforms.h>
#include <mach_kdb.h>

#include <machine/machspl.h>		/* spl definitions */
#include <kern/thread.h>
#include <mips/mips_cpu.h>
#include <sys/reboot.h>
#include <mach/vm_param.h>
#include <mips/prom_interface.h>
#include <mips/thread.h>

/*
 * 	Cache
 */
unsigned icache_size;
unsigned dcache_size;
unsigned mipscache_state;

/*
 *	Memory
 */
vm_offset_t	avail_start, avail_end;
vm_offset_t	virtual_avail, virtual_end;
vm_size_t	mem_size;
int		memlimit = 0;		/* patchable to run in less memory */

extern char	end[], edata[];
char		*esym = (char *)0;

#if	MACH_KDB
struct pcb db_fake_pcb = {0,};
int		boothowto = RB_KDB;
#else	MACH_KDB
int		boothowto = 0;
#endif	MACH_KDB

extern vm_offset_t	boot_start;
extern vm_size_t	boot_size;

static init_memory();

mips_init(argc, argv)
	int argc;
	char *argv[];
{
	unsigned first_page;	/* first free physical page */
	int i;
	struct thread fake_th;	/* while sizing memory, and for KDB */
	struct task fake_tk;
	extern char TRAP_tlb_umiss[], TRAP_tlb_umiss_end[];
	extern char TRAP_exception[], TRAP_exception_end[];
#if	MACH_KDB
	extern pcb_t current_pcb;
#endif	MACH_KDB

	splhigh();			/* sanity precautions */
	for (i = 0; i < TLB_SIZE; i++)
		tlb_zero(i);

#if	DECSTATION
	which_prom();
#endif	/*DECSTATION*/

	/*
	 *	Move symbol table and bootstrap code
	 *	out of BSS.
	 */
	first_page = move_bootstrap();

#if	MACH_KDB
	kdb_init();
#endif

	bzero(edata, end - edata);	/* zero bss */

	/*
	 *	Size and test the caches
	 */
	mipscache_size(&dcache_size, &icache_size);
	mipscache_flush(dcache_size, icache_size);
	mipscache_tests(&avail_start);

	/*
	 *	Fake some variables so that we can take traps right away.
	 *	This is important while sizing memory, and in case KDB
	 *	is invoked before the first thread is set up.
	 */
	bzero(&fake_th, sizeof fake_th);
	bzero(&fake_tk, sizeof fake_tk);
	fake_th.task = &fake_tk;
#if	MACH_KDB
	current_pcb = &db_fake_pcb;
	fake_th.pcb = current_pcb;
#endif	MACH_KDB
	current_thread() = &fake_th;

	/*
	 *	The exception handlers must live where they need to,
	 *	but they get compiled 'wherever'.  Copy them down to
	 *	their workplace.
	 */
	bcopy(TRAP_tlb_umiss,VEC_TLB_UMISS,TRAP_tlb_umiss_end-TRAP_tlb_umiss);
	bcopy(TRAP_exception,VEC_EXCEPTION,TRAP_exception_end-TRAP_exception);

	mips_box_model_dep();		/* cold adaptation */

	/*
	 *	Stop the clock for now.
	 */
	mc_close();

	set_root_name(argv[0]);

	/*
	 * First available page of phys mem
	 */
	first_page = mips_btop(K0SEG_TO_PHYS(mips_round_page(first_page)));

	/*
	 * Parse command line, setting appropriate switches and
	 * configuration values.
	 * NOTE: Before cnprobe() is called you must do dprintf;
	 * printf won't work.
	 */

	/* First set some defaults */

	parse_args(argc, argv);

	init_memory(first_page);

	cons_find(1);		/* initialize console device: MUST be there */

#if	MACH_KDB
	if ((boothowto&RB_HALT) && (boothowto&RB_KDB))
		gimmeabreak();
#endif	MACH_KDB

	printf("Mips boot: memory from 0x%x to 0x%x\n",
	       avail_start, avail_end);

	/*
	 * Initialize the machine-dependent portion of the VM system
	 */
	pmap_bootstrap();

	machine_startup();
}

/*
 * Find memory size sweeping it up
 */
size_memory_sweeping(from, to)
	vm_offset_t from, to;
{
	register vm_offset_t start, end;

	for (start = mips_ptob(from), end = mips_ptob(to);
	     start < end;
	     start += MIPS_PGBYTES, from++)
		if (check_memory(PHYS_TO_K1SEG(start),1))
			break;
		else
			bzero(PHYS_TO_K0SEG(start), MIPS_PGBYTES);
	return from;
}

/*
 * Sometimes the best thing to do is just to
 * use what the prom knows
 */
boolean_t
default_get_memory_bitmap(bmp, blp, psp)
	int	**bmp;
	int	*blp, *psp;
{
	register char	*val, i;
	char		buff[32];

	if ((val = prom_getenv("bitmap")) == 0)
		return TRUE;
	/* chicaneries to cope with bogus prom: it copies
	   the string on the stack */
	for (i = 0; i < 32; i++) buff[i] = val[i];
	*bmp = (int*)string_to_int(buff);
	val = prom_getenv("bitmaplen");
	for (i = 0; i < 32; i++) buff[i] = val[i];
	*blp = string_to_int(buff);
	*psp = MIPS_PGBYTES;
	return FALSE;
}

boolean_t (*get_memory_bitmap_from_prom)() = default_get_memory_bitmap;

size_memory_trusting_prom(from, to)
	vm_offset_t from, to;
{
	/*
	 *	Something wrong on the 3max: must use
	 *	the prom's bitmap cuz the above gives
	 *	bogus results.
	 */
	register  int i;
        int	 *bitmap;
	int	  len, size, psize;

	if ((*get_memory_bitmap_from_prom)(&bitmap, &len, &psize)) {
		dprintf("mem bitmap?\n");
		halt();
	}

	size = 0;
	/*
	 *	Apparently, the first two words cannot be trusted
	 *	Memory MUST be contiguous, but this is old news..
	 */
	bitmap += 2;
	size += 64;

	for (i = 2; i < len; i++, bitmap++) {
		if (*bitmap == 0xffffffff)
			size += 32;	/* pages */
		else
			/*
			 *	The last word seems screwed too. Aurgh.
			 */
				break;
	}

	/*
	 *	Convert into our size pages, assume clean multiples
	 */
	if (psize < MIPS_PGBYTES)
		size = size / (MIPS_PGBYTES / psize);
	else
		size = size * (psize / MIPS_PGBYTES);

	/*
	 *	Zero out memory, appears to be still needed.
	 */
	if (size < to) to = size;

	bzero(PHYS_TO_K0SEG(mips_ptob(from)), mips_ptob(to-from));

	return size;
}

int (*memory_sizing_routine)() = size_memory_sweeping;

vm_size_t	(*mipsbox_steal_memory)() = 0;

static
init_memory(first_page)
	vm_offset_t first_page;
{
	register i, max;

	/*
	 *	Size memory
	 */
	max = memlimit ? mips_btop(memlimit) : mips_btop(K0SEG_SIZE);
	i = (*memory_sizing_routine)(first_page, max);

	/*
	 *	If we should run in less memory then we should.
	 */
	if (memlimit && (i > max))
		i = max;
	mem_size = mips_ptob(i);

	/*
	 *	Notify the VM system of what memory looks like
	 */
	vm_set_page_size();
	avail_start = (vm_offset_t)mips_ptob(first_page);
	avail_end   = (vm_offset_t)mem_size;

	if (mipsbox_steal_memory) {
		/*
		 * Boot time page stealing. Good reasons so far:
		 * - console data structures (shared screen info)
		 * - contiguous phys memory for lance buffers
		 */
		vm_size_t		needed;

		needed = (*mipsbox_steal_memory)(PHYS_TO_K0SEG(avail_start));
		avail_start += round_page(needed);
	}
}
