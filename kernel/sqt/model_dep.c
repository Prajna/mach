/* 
 * Mach Operating System
 * Copyright (c) 1992,1991 Carnegie Mellon University
 * Copyright (c) 1992,1991 Sequent Computer Systems
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND SEQUENT COMPUTER SYSTEMS ALLOW FREE USE OF
 * THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * SEQUENT COMPUTER SYSTEMS DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * Revision 2.7  93/01/14  17:56:03  danner
 * 	Added cpu_interrupt_to_db and nmi_intr.
 * 	[92/10/25            dbg]
 * 	Added cpu_interrupt_to_db and nmi_intr.
 * 	[92/10/25            dbg]
 * 
 * Revision 2.6  92/01/03  20:28:07  dbg
 * 	Rename kdb_init to ddb_init.  Remove esym.
 * 	Allocate interrupt stacks before initial debugger breakpoint.
 * 	[91/09/11            dbg]
 * 
 * Revision 2.5  91/07/31  18:03:05  dbg
 * 	Changed copyright.
 * 
 * 	Removed call to pcb_module_init (now called from
 * 	machine-independent code).
 * 	[91/07/26            dbg]
 * 
 * 	Call interrupt_stack_alloc.
 * 	[91/06/27            dbg]
 * 
 * Revision 2.4  91/05/20  22:23:05  rpd
 * 	Fixed avail_remaining, avail_next initialization.
 * 
 * Revision 2.3  91/05/18  14:37:37  rpd
 * 	Changed pmap_bootstrap arguments.
 * 	Moved pmap_free_pages and pmap_next_page here.
 * 	[91/05/15            rpd]
 * 
 * Revision 2.2  91/05/08  12:58:00  dbg
 * 	Each CPU calls mp_desc_init on its own.
 * 	[91/02/13            dbg]
 * 
 * 	Add pmap_valid_page.
 * 	[91/01/17            dbg]
 * 
 * 	Altered for pure kernel.
 * 	Created from pieces of sqt/machdep.c and sqt/startup.c.
 * 	[90/10/03            dbg]
 *
 */

/*
 * model_dep.c
 *	Various startup/initializations.  i386 version.
 *	Also has machine-dependent shutdown code and other
 *	similar stuff.
 */
#include <mach_kdb.h>

#include <mach/kern_return.h>
#include <mach/processor_info.h>

#include <mach/boolean.h>
#include <mach/machine.h>
#include <mach/vm_param.h>

#include <vm/pmap.h>
#include <vm/vm_kern.h>

#include <kern/lock.h>
#include <kern/time_out.h>

#include <sys/reboot.h>

#include <i386/proc_reg.h>

#include <sqt/vm_defs.h>
#include <sqt/ioconf.h>
#include <sqt/intctl.h>
#include <sqt/trap.h>
#include <sqt/mutex.h>

#include <sqt/SGSproc.h>
#include <sqt/slic.h>
#include <sqt/slicreg.h>
#include <sqt/engine.h>
#include <sqt/cfg.h>
#include <sqt/clkarb.h>

#include <sqtsec/sec.h>

int		mono_P_eng;		/* engine for mono_p drivers */

/*
 * Bootstrap memory allocator
 */
char		*calloc();
#define	csalloc(n,type)	(type *) calloc((int)(n)*(sizeof(type)))

boolean_t	calloc_ok = TRUE;		/* flag for legal calloc's */

extern	boolean_t   light_show;

extern	int	mono_P_slic;
extern	int	resphysmem;		/* reserved physical memory */

extern	vm_offset_t topmem;		/* top of memory */
extern	vm_size_t   totalmem;		/* total memory (topmem-holes) */

extern	int	master_cpu;

extern boolean_t upyet;

vm_offset_t	loadpt = 0;
vm_offset_t	avail_start;
vm_offset_t	avail_end;
vm_offset_t	virtual_avail;
vm_offset_t	virtual_end;
vm_size_t	mem_size;
vm_offset_t	avail_next;
unsigned int	avail_remaining;

/*
 * Parameters passed from bootstrap loader/start.s
 */
vm_offset_t	first_avail = 0;

extern	struct	ctlr_desc *slic_to_config[];

extern u_char		cons_scsi;	/* slic address of console scsi */
struct sec_cib *	cbdcib;		/* address of console board device */
struct sec_gmode	cbdgsm;		/* get/set modes command */

struct sec_cib *	wdtcib;		/* address of watchdog timer cib */
struct sec_smode	wdtsm;		/* for setmodes and startio commands */

int	wdtreset();	/* forward */
int	wdt_timeout;

struct reboot		bootdata;

int *	va_led = (int *)VA_LED;		/* processor LED */

extern char		version[];	/* system version string */

/*
 * machine_startup()
 *	Do basic system initializations.
 *
 * Called by first processor to start, very early after system is alive.
 * Runs with paging enabled.
 *
 * first_avail = first available physical address.
 */

void
machine_startup()
{
	register int i;
	unsigned procid;
	extern char edata, end;

	/*
	 * Zero BSS.
	 */
	blkclr((char *)&edata, (char *)&end - (char *)&edata);

	/*
	 * Set up the virtual address of the configuration table.
	 */
	va_CD_LOC = PHYSTOKV(CD_LOC, struct config_desc *);

	/*
	 * Map the rest of physical memory, and set up the
	 * kernel virtual address space.
	 */
	avail_start = first_avail;
	avail_end   = (vm_offset_t) (va_CD_LOC->c_maxmem);
	mem_size    = avail_end - loadpt;

	vm_set_page_size();

	pmap_bootstrap(loadpt);

	/*
	 * Allocate and set up mapping for SLIC and LEDs.
	 */

	map_slic_and_LEDs(&avail_start);

	/*
	 * Enable NMIs on the processor.
	 */

        wrslave(va_slic->sl_procid, PROC_CTL,
                PROC_CTL_NO_SSTEP | PROC_CTL_NO_HOLD | PROC_CTL_NO_RESET);

	/*
	 * Configure the HW and initialize interrupt table (int_bin_table).
	 *
	 * Configure allocates and fills out:
	 *	engine[] array		; one per processor
	 *	Nengine			; # processors
	 *	mono_P_slic		; Slic addr for mono_P drivers
	 * plus sets up device drivers/etc.
	 */

	va_slic->sl_lmask = 0;			/* insure interrupts OFF */
	configure();

	upyet = TRUE;				/* governs less than in DYNIX */

	/*
	 * Fill out engine structures; these were allocated by
	 * configure() who also filled out the e_slicaddr fields,
	 * and turned on E_FPU387 and/or E_FPA in e_flags if appropriate.
	 *
	 * Figure out my procid from engine structures.
	 *
	 * If there were any mono-P drivers for existing HW, then
	 * they were bound to 'me' (eg, booting processor); thus
	 * set flag to avoid ever taking `me' offline.
	 */

	procid = Nengine;
	for (i = 0; i < Nengine; i++) {
		engine[i].e_flags |= E_OFFLINE;	/* not up yet */
		if (va_slic->sl_procid == engine[i].e_slicaddr) {
			procid = i;
			master_cpu = i;
 printf("Master cpu is %d\n", i);
			if (mono_P_slic >= 0) {
				mono_P_eng = i;
				engine[i].e_flags |= E_DRIVER;
			}
		}
	}

	/*
	 * There should be NO allocations after this!
	 * (at least not from calloc)
	 */

	calloc_end();

	/*
	 * Allocate the interrupt stacks, from 1-1 physical memory.
	 * Uses avail_start.
	 */
	interrupt_stack_alloc();

	/*
	 * Allocate per-processor descriptor tables.
	 */
	(void) mp_desc_init(master_cpu);

	/*
	 *	Initialize for pmap_free_pages and pmap_next_page.
	 *	This must happen after calls to calloc.
	 */

	avail_remaining = atop(avail_end - avail_start);
	avail_next = avail_start;

#if	MACH_KDB
	/*
	 * Initialize the kernel debugger.
	 */
	ddb_init();

	/*
	 * Take a debug trap if user asked for it.
	 */
	if (boothowto & RB_KDB)
	    Debugger();

#endif	MACH_KDB

	/*
	 * Print out system version number.
	 */
	printf(version);

	/*
	 * Start Mach.
	 */
	setup_main();
	/*NOTREACHED*/
}

/*
 * machine_init()
 * Called with Mach running, but no threads.
 */
machine_init()
{
	register struct cntlrs * b8k;

	/*
	 * System is mapped enough to do self-init's.
	 */

	self_init();

	/*
	 * Enable Ecc error reporting in memory controllers.
	 *
	 * Note: errors will merely be latched.  No interrupts
	 * are generated other than NMI for uncorrectable errors.
	 */

	memenable();

	/*
	 * Initialize IO controller mapping.
	 */

	for (b8k = b8k_cntlrs; b8k->conf_b8k != NULL; b8k++)
		(*b8k->b8k_map)();

	/*
	 * Get the time
	 */
	inittodr();
}

void start_other_cpus()
{
	decl_simple_lock_data(extern, start_lock)
	register int	i;
	register struct machine_slot *ms;

	/*
	 * Allow other CPUs to run if started
	 */
	simple_unlock(&start_lock);	/* see locore.s */

#if 0
 /* do later */
	if (boothowto & RB_UNIPROC) {
	    printf("Uni-processor boot; slaves will not be started.\n");
	    return;
	}

	/*
	 * Start up other CPUs
	 */
	for (i = 0, ms = &machine_slot[0];
	     i < NCPUS;
	     i++, ms++)
	{
	    if (ms->is_cpu && i != master_cpu && !ms->running) {
		cpu_start(i);
	    }
	}
#endif 0
}

/*
 * Slave comes up here.
 */
slave_machine_init()
{
	self_init();
}


/*
 * self_init()
 *	Do self init's.  Done by each processor as it comes alive.
 */

self_init()
{
	struct	engine	*eng;			/* my engine structure */
	unsigned procid;			/* logical processor # */

	procid = cpu_number();
	eng = &engine[procid];

	/*
	 * Fill out relevant fields in "engine" structure.
	 */

	eng->e_fpuon = (CR0_PG|CR0_PE);		/* how to turn FPU on */
	if (eng->e_flags & E_FPU387) {		/* if 387... */
	    eng->e_fpuon |= CR0_ET|CR0_MP;	    /*	... set for 387 */
	    eng->e_fpuoff = eng->e_fpuon | CR0_EM;  /* off ==> emulate math */
	    init_fpu();				    /* 387 needs fninit */
	} else {
	    eng->e_fpuon |= CR0_EM;		    /*	... set for 387 */
	    eng->e_fpuoff = eng->e_fpuon;	    /* off ==> emulate math */
	}

	/*
	 * Do other processor-local inits.
	 */

	localinit();

	/*
	 * Fill out the engine structure.
	 */

	eng->e_flags &= ~E_OFFLINE;			/* on-line, now! */

	/*
	 * Say hello.
	 *
	 * Processor up -- turn processor LED on.
	 */

	if (light_show) {
		DISABLE();
		if (fp_lights)
			FP_LIGHTON(cpu_number());
		*va_led = 1;
		ENABLE();
	}

	enable_nmi();
}

/*
 * map_slic_and_LEDs(phys_addr_p)
 *
 *	Map in the SLIC and the per-processor LEDs, allocating
 *	page tables for them.
 *	Alters *phys_addr_p to allocate physical pages for page tables.
 *
 * These are shared by all processors.
 *
 * When/if need to map more such things, should make this table-driven.
 */

/*
 * Return a pointer to the page-table entry that maps a given virtual
 * address.  May allocate a physical page for the page table.
 */
extern pt_entry_t *	kpde;		/* kernel page directory VA */

pt_entry_t *
io_map_pte(va, phys_addr_p)
	vm_offset_t	va;		/* VA to map */
	vm_offset_t	*phys_addr_p;	/* PA to allocate from */
{
	register pt_entry_t *pdp;
	register pt_entry_t *ptp;

	pdp = kpde;
	pdp += pdenum(va);

	if ((*pdp & INTEL_PTE_VALID) == 0) {
	    /*
	     * Must allocate a page table page
	     */
	    vm_offset_t	pa;

	    pa = *phys_addr_p;		/* physical address */
	    *phys_addr_p += I386_PGBYTES;
	    bzero(PHYSTOKV(pa, char *), I386_PGBYTES);
	    *pdp = pa_to_pte(pa) | INTEL_PTE_VALID | INTEL_PTE_WRITE;
	}
	ptp = (pt_entry_t *)ptetokv(*pdp);	/* virtual address */
	return &ptp[ptenum(va)];
}

map_slic_and_LEDs(phys_addr_p)
	vm_offset_t	*phys_addr_p;
{
	register pt_entry_t	*ptp;

#if 0
	/*
	 * Allocate mapping for FPA.
	 * Not mapped in maplocalIO(); rather let page fault turn it on
	 * per process.
	 */

	alloc_fpa();
#endif 0

	/* Map SLIC - one page */
	ptp = io_map_pte(VA_SLIC, phys_addr_p);
	*ptp = pa_to_pte(PHYS_SLIC) | INTEL_PTE_VALID | INTEL_PTE_WRITE;

	/*
	 * Map processor LED.
	 */
	ptp = io_map_pte(VA_LED, phys_addr_p);
	*ptp = pa_to_pte(PHYS_LED) | INTEL_PTE_VALID | INTEL_PTE_WRITE;

	/*
	 * Map elapsed-time counter..
	 */
	ptp = io_map_pte(VA_ETC, phys_addr_p);
	*ptp = pa_to_pte(PHYS_ETC) | INTEL_PTE_VALID | INTEL_PTE_WRITE;

}

/*
 * localinit()
 *	Init local processor resources.
 *
 * This involves:
 *	turning on the cache,
 *	setting up SLIC interrupt control,
 */

localinit()
{
	register struct cpuslic *sl = va_slic;
	register struct engine *eng = &engine[cpu_number()];

	/*
	 * If processor has an FPA, initialize it.
	 */

	if (eng->e_flags & E_FPA) {
	}

	/*
	 * Set up SLIC interrupt control and start local clock.
	 */

	(void) splhi();				/* all intrs masked */
	ENABLE();				/* but ON at processor */

#ifdef	DEBUG
	if ((sl->sl_ictl & (SL_HARDINT|SL_SOFTINT)) != 0) {
		printf("localinit: pending interrupts 0x%x\n", 
				sl->sl_ictl & (SL_HARDINT|SL_SOFTINT));
		panic("localinit");
	}
#endif	DEBUG
	assert((sl->sl_ictl & (SL_HARDINT|SL_SOFTINT)) == 0);
	sl->sl_ictl = 0x00;			/* not using `m' bit */

	sl->sl_procgrp = TMPOS_GROUP;		/* set group ID */
	setgm(sl->sl_procid, SL_GM_ALLON);	/* set self group-mask all ON */
#ifdef	CHECKSLIC
	assert(sl->sl_gmask == SL_GM_ALLON);
#endif	CHECKSLIC

}

/*
 *	Fix up the virtual address space when all io_map calls are done.
 */
void
io_map_done()
{
	virtual_avail = round_page(virtual_avail);
}

/*
 * calloc()
 *	Allocate zeroed memory at boot time.
 *
 * Done via bumping "curmem" value.
 *
 * Skips holes in physical memory after memory is configured (topmem != 0).
 * Assumes allocations to that point are in memory contiguous from physical 0.
 *
 * callocrnd() is used to round up so that next allocation occurs
 * on a given boundary.
 *
 * XXX These should be rewritten using pmap_steal_memory.
 */

boolean_t	cmem_exists();	/* forward */

caddr_t
calloc(size)
	int	size;
{
	char *	val;

	assert(calloc_ok);

	size = (size + (sizeof(int) - 1)) & ~(sizeof(int)-1);

	/*
	 * If ok to check, insure memory exists and skip hole if necessary.
	 * Skipping hole puts curmem on hole boundary, thus arbitrary alignment.
	 */

	while (!cmem_exists(avail_start, size)) {
	    avail_start = i386_round_page(avail_start);
	    assert(avail_start < avail_end);
	}

	/*
	 * Allocate and clear the memory.
	 */

	val = PHYSTOKV(avail_start, char *);
	avail_start += size;

	bzero(val, (unsigned)size);
	return(val);
}

callocrnd(bound)
	int	bound;
{
	avail_start = ((avail_start + bound - 1) / bound) * bound;
}

calloc_end()
{
	callocrnd(PAGE_SIZE);
	calloc_ok = 0;
}

/*
 * cmem_exists()
 *	Check for existence of memory from a given address for a given size.
 */

boolean_t
cmem_exists(paddr, size)
	caddr_t	paddr;
	register int	size;
{
	register int	pg;

	size += (int) paddr & (I386_PGBYTES-1);
	for (pg = i386_btop(paddr); size > 0; size -= I386_PGBYTES, pg++)
		if (!page_exists(pg))
			return(0);
	return(1);
}

/*
 * Return a number to use in spin loops that takes into account
 * both the cpu rate and the mip rating.
 */

calc_delay(x)
	unsigned int	x;
{
	extern int	cpurate;
	extern	 int	lcpuspeed;

	if (!upyet)
		return (x*cpurate);
	else
		return (x*engine[cpu_number()].e_cpu_speed*lcpuspeed)/100;
}

/*
 * halt_all_cpus()
 *	Reboot the machine.
 *
 * Boot routine returns to Firmware.  If called by panic it tries to sync
 * up disks and returns specifying that the alternate boot name is to be
 * booted.  This is normally the Memory dumper.
 *
 * Only ONE engine is alive at this point.
 */

halt_all_cpus(do_reboot)
	boolean_t	do_reboot;
{
	register struct sec_gmode *cbdmptr = &cbdgsm;
	register struct sec_smode *wdtsmptr = &wdtsm;
	register spl_t s_ipl;
	extern etext;
	extern boolean_t dblpanic;

	int	howto = 0;		/* XXX */

	return_fw(do_reboot);		/* XXX */

	if (!upyet)
		return_fw(FALSE);

	/*
	 * Get powerup reboot structure.
	 */

	cbdmptr->gm_status = 0;
	bootdata.re_powerup = 1;	/* 0 booted data, 1 powerup values */
	cbdmptr->gm_un.gm_board.sec_reboot =
				KVTOPHYS(&bootdata, struct reboot *);
	cbdcib->cib_inst = SINST_GETMODE;
	cbdcib->cib_status = KVTOPHYS(&cbdgsm, int *);
	s_ipl = splhi();
	mIntr(cons_scsi, 7, SDEV_SCSIBOARD);
	splx(s_ipl);

	while ((cbdmptr->gm_status & SINST_INSDONE) == 0)
		continue;

	if (cbdmptr->gm_status != SINST_INSDONE) {
		printf("Cannot get Console Board modes\n");
		return_fw(FALSE);
	}

	/*
	 * Now tell FW how to reboot
	 */

	bootdata.re_powerup = 0;	/* 0 booted data, 1 powerup values */
	cbdmptr->gm_un.gm_board.sec_dopoll = 0;		/* no change */
	cbdmptr->gm_un.gm_board.sec_powerup = 0;	/* no change */
	cbdmptr->gm_un.gm_board.sec_errlight = SERR_LIGHT_SAME;

	bootdata.re_boot_flag = howto;

	cbdmptr->gm_status = 0;
	cbdcib->cib_inst = SINST_SETMODE;
	cbdcib->cib_status = KVTOPHYS(&cbdgsm, int *);
	s_ipl = splhi();
	mIntr(cons_scsi, 7, SDEV_SCSIBOARD);
	splx(s_ipl);

	while ((cbdmptr->gm_status & SINST_INSDONE) == 0)
		continue;

	if (cbdmptr->gm_status != SINST_INSDONE) {
		printf("Cannot set Console Board modes\n");
		return_fw(FALSE);
	}

	if (do_reboot) {
		/*
		 * Set watchdog for 1 minute.
		 * Prevent ERROR light from going on...
		 */

		untimeout(wdtreset, (caddr_t)0);	/* Stop wdt reset */

		wdtsmptr->sm_status = 0;
		wdtsmptr->sm_un.sm_wdt_mode = 60;	/* Set for minute! */

		wdtcib->cib_inst = SINST_SETMODE;
		wdtcib->cib_status = KVTOPHYS(&wdtsm, int *);
		s_ipl = splhi();
		mIntr(cons_scsi, 7, SDEV_WATCHDOG);
		splx(s_ipl);

		while ((wdtsmptr->sm_status & SINST_INSDONE) == 0)
			continue;

		if (wdtsmptr->sm_status != SINST_INSDONE) {
			printf("Cannot Setmode Watchdog\n");
			return_fw(FALSE);
		}

		wdtsmptr->sm_status = 0;
		wdtcib->cib_inst = SINST_STARTIO;
		wdtcib->cib_status = KVTOPHYS(&wdtsm, int *);
		s_ipl = splhi();
		mIntr(cons_scsi, 7, SDEV_WATCHDOG);
		splx(s_ipl);

		while ((wdtsmptr->sm_status & SINST_INSDONE) == 0)
			continue;

		if (wdtsmptr->sm_status != SINST_INSDONE) {
			printf("Cannot Restart Watchdog\n");
			return_fw(FALSE);
		}
	}

	(void) spl1();

	return_fw(do_reboot);
}

/*
 * return_fw()
 *	Return to Firmware.
 */

return_fw(do_reboot)
	boolean_t	do_reboot;
{
	register struct ctlr_toc *toc;
	register int	i;
	extern	boolean_t	conscsi_yet;
	extern	int	light_show;
	extern	char	*panicstr;
	extern	int	(*cust_panics[])();

	if (upyet)
		(void) splhi();

#if 0
	/*
	 * If a panic, call custom panic handlers.
	 */

	if (panicstr != NULL)
		for (i = 0; cust_panics[i] != NULL; i++)
			(*cust_panics[i])();
#endif 0
	/*
	 * Get table of contents pointer for processor board.
	 */

	toc = PHYSTOKV(&va_CD_LOC->c_toc[SLB_SGSPROCBOARD],
			struct ctlr_toc *);		/* SGS processors */

	/*
	 * Turn off light show - if enabled.
	 * Since panic may be called before initialization is complete,
	 * all front panel processor lights are turned off.
	 */

	if (light_show) {
		if (fp_lights) {
			for (i = 0; i < toc->ct_count; i++)
				FP_LIGHTOFF(i);
		}
		*va_led = 0;
	}

	/*
	 * If the console scsi has not yet received its INIT command
	 * then use the powerup cib.
	 */

	if (!conscsi_yet) {
		struct sec_powerup *scp;
		scp = PHYSTOKV(CD_LOC->c_cons->cd_sc_init_queue,
				struct sec_powerup *);
		scp->pu_cib.cib_inst = SINST_RETTODIAG;
		scp->pu_cib.cib_status = SRD_BREAK;
	} else {
		cbdcib->cib_inst = SINST_RETTODIAG;
		cbdcib->cib_status = (!do_reboot) ? SRD_BREAK : SRD_REBOOT;
	}
#if	defined(DEBUG) && defined(i386) && !defined(KXX)	/*XXX*/
	flush_cache();						/*XXX*/
#endif	DEBUG&&i386&&!KXX					/*XXX*/
	mIntr(cons_scsi, 7, SDEV_SCSIBOARD);

	/*
	 * SCED will take control.
	 */

	for (;;);
	/*NOTREACHED*/
}

/*
 * Watchdog timer routines.
 *
 * Hit watchdog timer every half second.
 */

/*
 * wdtinit()
 *	Initialize watchdog timeout interval.
 */

wdtinit()
{
	register struct sec_smode *wdtsmptr = &wdtsm;
	spl_t s_ipl;

	if (wdt_timeout <= 0)
		return;
	wdtsmptr->sm_status = 0;
	wdtsmptr->sm_un.sm_wdt_mode = wdt_timeout;

	wdtcib->cib_inst = SINST_SETMODE;
	wdtcib->cib_status = KVTOPHYS(&wdtsm, int *);
	s_ipl = splhi();
	mIntr(cons_scsi, 7, SDEV_WATCHDOG);
	splx(s_ipl);

	while ((wdtsmptr->sm_status & SINST_INSDONE) == 0)
		continue;

	if (wdtsmptr->sm_status != SINST_INSDONE)
		panic("Initializing Watchdog");
	timeout(wdtreset, (caddr_t)0, hz/2);
}

wdtreset()
{
	register struct	sec_smode *wdtsmptr = &wdtsm;
	spl_t s_ipl;

	wdtsmptr->sm_status = 0;
	wdtcib->cib_inst = SINST_STARTIO;
	wdtcib->cib_status = KVTOPHYS(&wdtsm, int *);

	/*
	 * Tell SCED about the command.  Bin 3 is sufficient, helps avoid
	 * SLIC-bus saturation/lockup (since SCED interrupts Dynix mostly on
	 * bins 4-7, using bin 3 to interrupt SCED gives SCED -> Dynix priority
	 * over Dynix -> SCED, thus SCED won't deadlock against Dynix).
	 */

	s_ipl = splhi();
	mIntr(cons_scsi, 3, SDEV_WATCHDOG);
	splx(s_ipl);

	while ((wdtsmptr->sm_status & SINST_INSDONE) == 0)
		continue;

	if (wdtsmptr->sm_status != SINST_INSDONE)
		panic("Resetting Watchdog");
	timeout(wdtreset, (caddr_t)0, hz/2);
}

light_off(cpu_num)
{
	if (light_show) {
		DISABLE();
		if (fp_lights)
			FP_LIGHTOFF(cpu_num);
		*va_led = 0;
		ENABLE();
	}
}

light_on(cpu_num)
{
	if (light_show) {
		if (fp_lights)
			FP_LIGHTON(cpu_num);
		*va_led = 1;
	}
}


/*
 * access_error()
 *	Access Error Reporting:
 *		Bus timeouts
 *		ECC Uncorrectable
 *		Processor fatal error (SGS only)
 *
 * Called from NMI handler, SEC_error and MBAd_error with
 * copy of Access error register.
 *
 * Called at SPLHI.
 */

access_error(errval)
	u_char errval;
{
	register int io_access;
	register char *s;
	u_char acctype;
	extern	memerr();

	printf("Access Error Register = 0x%x\n", errval);
	errval = ~errval;
	acctype = errval & SLB_ATMSK;
	io_access = errval & SLB_AEIO;

	switch (acctype) {
	case SLB_AEFATAL:
			s = "Fatal";
		break;
	case SLB_AENONFAT:
		if (io_access)
			s = "Non-Fatal";
		else
			s = "Ecc Correctable";
		break;
	case SLB_AETIMOUT:
		s = "Timeout";
		break;
	default:
		s = "Unknown";
		break;
	}
	printf("%s error on %s %s.\n", s,
		(errval & SLB_AEIO) ? "I/O" : "memory",
		(errval & SLB_AERD) ? "read" : "write");

	/*
	 * If memory error get more data...
	 */

	if ((acctype == SLB_AEFATAL) && (io_access != SLB_AEIO)) {
		if (upyet) {
			/*
			 * Avoid races with memory polling.
			 */
			untimeout(memerr, (caddr_t) 0);
#ifdef	notdef
			/*
			 * If concurrent access errors, the loser of the
			 * race commits suicide.
			 *
			 * Since we are about to die, do not bother releasing
			 * lock.
			 */
			if (cp_lock(&uncmem_lock, SPLHI) == CPLOCKFAIL) {
				printf("Concurrent ECC Uncorrectable Error\n");
				pause_self();
				/*NOTREACHED*/
			}
#endif	notdef
		}
		memlog();
	}
}

/*
 * cpu_start()
 *	Start another processor by "unpausing" it.
 *
 * Called by tmp_ctl TMP_ONLINE command.
 *
 * The semaphore tmp_onoff is assumed to be held by the caller.
 * This semaphore guarantees that only one on/off line transaction
 * occurs at a time.  No real need to single-thread these on SGS,
 * but doesn't hurt and provides some basic sanity (who knows, maybe
 * there is some hidden reason! ;-)
 */
kern_return_t
cpu_start(engno)
{
	register struct engine *eng = &engine[engno];
	spl_t	s = splhi();

#ifdef	notyet				/* SGS VLSI doesn't support flush yet */
	u_char	bic_slic;
	u_char	chan_ctl;

	/*
	 * Re-enable the appropriate BIC channel (this is left disabled
	 * by an offline).  This is a NOP on early rev BIC's.
	 *
	 * Note that only one SLIC on the processor board talks to the BIC.
	 */

	bic_slic = BIC_SLIC(eng->e_slicaddr,
			slic_to_config[eng->e_slicaddr]->cd_flags);
	chan_ctl = (eng->e_slicaddr & 0x01) ? BIC_CDIAGCTL1 : BIC_CDIAGCTL0;

	wrSubslave(bic_slic, PROC_BIC, chan_ctl, 
		(u_char) (rdSubslave(bic_slic, PROC_BIC, chan_ctl) & ~BICCDC_DISABLE));
#endif	notyet				/* SGS VLSI doesn't support flush yet */

	/*
	 * Un-hold the processor, turn on the LED, and *don't* reset.
	 * Also enable NMI's: it's ok for 1st online (don't expect any NMI
	 * sources) and subsequent online's need NMI's enabled here since they
	 * don't execute localinit() to enable NMI's.  This gives small risk
	 * of strange crash if NMI is asserted on 1st online (since processor
	 * is an 8086 at this time); if a problem, need to keep state in
	 * e_flags whether the processor has ever been online'd before, and
	 * initialize PROC_CTL differently here 1st time vs subsequent times.
	 */

	if (light_show && fp_lights) {
		FP_LIGHTON(engno);
	}

	wrslave(eng->e_slicaddr, PROC_CTL,
		PROC_CTL_NO_SSTEP | PROC_CTL_NO_HOLD | PROC_CTL_NO_RESET);

	splx(s);

	return KERN_SUCCESS;
}

/*
 * halt_engine()
 *	Halt processor via pause and reset.
 *
 * Turn off processor light.
 * Done implicitly via reset on B8K.
 * If fp_lights then done explicitly.
 *
 * Called by tmp_ctl with TMP_OFFLINE command to shutdown a processor.
 */

#ifdef	notyet				/* SGS VLSI doesn't support flush yet */
static	struct	proc_cmcs {
	u_char	pc_subaddr;		/* sub-slave address of CMC */
	u_int	pc_diag_flag;		/* flags -- all zero if CMC in use */
}	proc_cmcs[] = {
	{ PROC_CMC_0, CFG_SP_CMC0|CFG_SP_DRAM_0|CFG_SP_TRAM_0|CFG_SP_SRAM_0 },
	{ PROC_CMC_1, CFG_SP_CMC1|CFG_SP_DRAM_1|CFG_SP_TRAM_1|CFG_SP_SRAM_1 },
};
#endif	notyet				/* SGS VLSI doesn't support flush yet */

halt_engine(engno)
{
	register struct engine *eng = &engine[engno];
	spl_t	s = splhi();

#ifdef	notyet				/* SGS VLSI doesn't support flush yet */
	register struct ctlr_desc *cd = slic_to_config[eng->e_slicaddr];
	register struct proc_cmcs *pc;
	register int	i;
	u_char		bic_slic;
	u_char		chan_ctl;
	u_char		cmc_mode;
#endif	notyet				/* SGS VLSI doesn't support flush yet */
	u_char		slicid = eng->e_slicaddr;

	/*
	 * HOLD the processor, but don't reset it (also turn OFF led).
	 * Wait for processor to be HELD.
	 */

	wrslave(slicid, PROC_CTL,
			PROC_CTL_LED_OFF | PROC_CTL_NO_NMI |
			PROC_CTL_NO_SSTEP | PROC_CTL_NO_RESET);

	while (rdslave(slicid, PROC_STAT) & PROC_STAT_NO_HOLDA)
		continue;

#ifdef	notyet				/* SGS VLSI doesn't support flush yet */
	/*
	 * NOTE: the flush algorithm is probably WRONG!  Verify/fix
	 * when VLSI does support the flush function.
	 */
	/*
	 * Flush the processor's cache.
	 *
	 * For each cache set, if it passed all power-up diagnostics then
	 * tell the CMC it's a "master", flush it, make it a "slave" again.
	 */

	for (pc = proc_cmcs, i = 0; i < cd->cd_p_nsets; i++, pc++) {

		/*
		 * If any diagnostic flag is on, this cache set wasn't in use.
		 */

		if (eng->e_diag_flag & pc->pc_diag_flag)
			continue;

		/*
		 * Make the CMC the "master" and start the flush.
		 */

		cmc_mode = rdSubslave(slicid, pc->pc_subaddr, CMC_MODE);
		wrSubslave(slicid, pc->pc_subaddr, CMC_MODE,
				cmc_mode & ~(CMCM_SLAVE | CMCM_DISA_FLUSH));

		/*
		 * Wait for flush to finish.
		 */

		while (rdSubslave(slicid, pc->pc_subaddr, CMC_STATUS) & CMCS_FLUSH)
			continue;

		/*
		 * Make the CMC a "slave" again.
		 */

		wrSubslave(slicid, pc->pc_subaddr, CMC_MODE, cmc_mode);
	}

	/*
	 * Isolate the processor from the bus by turning off the appropriate
	 * BIC channel.  This is a NOP on early rev BIC's.
	 *
	 * Note that only one SLIC on the processor board talks to the BIC.
	 */

	bic_slic = BIC_SLIC(slicid, cd->cd_flags);
	chan_ctl = (slicid & 0x01) ? BIC_CDIAGCTL1 : BIC_CDIAGCTL0;

	wrSubslave(bic_slic, PROC_BIC, chan_ctl, 
		(u_char) (rdSubslave(bic_slic, PROC_BIC, chan_ctl) | BICCDC_DISABLE));
#endif	notyet				/* SGS VLSI doesn't support flush yet */

	if (light_show && fp_lights)
		FP_LIGHTOFF(engno);

	splx(s);
}


/*
 * halt_cpu()
 *	Arrange that processor get turned off.
 *
 * Called from MACH processor shutdown thread.
 * Processor is already running on its private stack.
 */

halt_cpu()
{
	int	engno = cpu_number();
	int	slicid = engine[engno].e_slicaddr;

	/*
	 * Interrupts have already been flushed.
	 * If we drop IPL to let interrupts in, the clock interrupt
	 * handler will die because we have no thread.
	 */
#if 0
	/*
	 * Insure no interrupts are pending on this processor.
	 * This is essentially a NOP until MACH is more symmetric
	 * and distributes interrupts to any processor.
	 */

	(void) splhi();
	flush_intr();
#endif

	if (light_show && fp_lights)
		FP_LIGHTOFF(cpu_number());

	/*
	 * HOLD the processor, but don't reset it (also turn OFF led).
	 * Wait for processor to be HELD.
	 */

	wrslave(slicid, PROC_CTL,
			PROC_CTL_LED_OFF | PROC_CTL_NO_NMI |
			PROC_CTL_NO_SSTEP | PROC_CTL_NO_RESET);

	while (rdslave(slicid, PROC_STAT) & PROC_STAT_NO_HOLDA)
		continue;

	/*
	 * There is no escape!
	 */

	for(;;);
	/*NOTREACHED*/
}

/*
 * flush_intr()
 *	Flush pending interrupts.
 *
 * Used when shutting down processor to insure pending interrupts
 * are cleared (and handled).
 */

flush_intr()
{
	int	counter;

	/*
	 * While there is a pending (HW or SW) interrupt, open
	 * a window to let it in.
	 *
	 * Need to loop (to get branch) until HW chip-workaround is
	 * removed (only admit interrupts on non-sequential fetch).
	 */

	SLICPRI(0);				/* try not to win arbitration */

	for (;;) {
		for (counter=0; counter < 100; counter++)
			continue;
		if ((va_slic->sl_ictl & (SL_HARDINT|SL_SOFTINT)) == 0)
			break;
		(void) spl0();
		for (counter=0; counter < 10; counter++)
			continue;		/* window to take int */
		(void) splhi();
	}
}

/*
 * Send cross cpu interrupt for pmap update
 */
interrupt_processor(cpu)
	int	cpu;
{
	spl_t s;

	s = splhi();
	sendsoft(engine[cpu].e_slicaddr, PMAPUPDATE);
	splx(s);
}

/*
 * Send soft-clock interrupt to master
 */
setsoftclock()
{
	spl_t	s;

	s = splhi();
	sendsoft(mono_P_slic, SOFTCLOCK);
	splx(s);
}

/*
 * Send cross cpu interrupt for debugger entry.
 * Use NMI since software interrupt is lowest priority.
 */
cpu_interrupt_to_db(cpu)
	int	cpu;
{
	spl_t	s;

	s = splhi();
	nmIntr(engine[cpu].e_slicaddr, PAUSESELF);
	splx(s);
}

panic_others()
{
	int i, me = cpu_number();

	for (i = 0; i < NCPUS; i++) {
		if (i == me)
			continue;
		if (machine_slot[i].is_cpu && machine_slot[i].running) {
			spl_t s;

			s = splhi();
			nmIntr(engine[i].e_slicaddr, PAUSESELF);
			splx(s);
		}
	}
}

/*
 * NMI interrupt.
 * If for remote debugger entry, trap to debugger.
 * Otherwise, panic (cause unknown so far).
 */
extern void	allow_nmi();
extern void	remote_db_enter();

int	sqt_proc_stat[NCPUS];
void
nmi_intr()
{
	int	proc_flt;

	/*
	 * Read processor fault register to get NMI reason
	 */
	proc_flt = rdslave(va_slic->sl_procid, PROC_FLT);
	if ((proc_flt & PROC_FLT_SLIC_NMI) == 0) {
	    /*
	     * NMI triggered from SLIC.
	     * Another CPU has entered the kernel debugger.
	     * Allow further NMIs, then stop.
	     */

	    /*
	     * Reset processor fault register (any write will do)
	     */
	    wrslave(va_slic->sl_procid, PROC_FLT, 0xbb);

	    /*
	     * XXX must re-enable NMIs at SLIC register
	     */
	    sqt_proc_stat[cpu_number()] = 
		rdslave(va_slic->sl_procid, PROC_STAT);

	    /*
	     * Toggle nmi accept in the processor control register
	     * so an NMI that arrived concurrently will be seen
	     * when NMIs are reenabled.
	     */
	    wrslave(va_slic->sl_procid, PROC_CTL,
			PROC_CTL_NO_NMI | PROC_CTL_NO_SSTEP | 
			PROC_CTL_NO_HOLD | PROC_CTL_NO_RESET);
	    wrslave(va_slic->sl_procid, PROC_CTL,
			PROC_CTL_NO_SSTEP | PROC_CTL_NO_HOLD |
			PROC_CTL_NO_RESET);
	    allow_nmi();
	    remote_db_enter();
	    return;
	}

	/*
	 * Unknown NMI.  Panic.
	 */
	panic("NMI");
}

/*ARGSUSED*/
kern_return_t
cpu_control(slot_num, info, count)
	int			slot_num;
	processor_info_t	info;
	long			*count;
{
	return (KERN_FAILURE);
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

/*
 *	XXX These functions assume physical memory is contiguous.
 *	What about page_exists?
 */

unsigned int pmap_free_pages()
{
	return avail_remaining;
}

boolean_t pmap_next_page(addrp)
	vm_offset_t *addrp;
{
	if (avail_next == avail_end)
		return FALSE;

	*addrp = avail_next;
	avail_next += PAGE_SIZE;
	avail_remaining--;
	return TRUE;
}

boolean_t pmap_valid_page(x)
	vm_offset_t x;
{
	return (avail_start <= x) && (x < avail_end);
}
