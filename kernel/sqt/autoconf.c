/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * Copyright (c) 1991 Sequent Computer Systems
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
 * $Log:	autoconf.c,v $
 * Revision 2.3  91/07/31  17:59:27  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  12:52:16  dbg
 * 	Adapted for pure Mach kernel.  No conditionals.  Removed ns32000
 * 	and KXX support.
 * 	[91/04/26  14:47:41  dbg]
 * 
 */

#ifndef	lint
static	char	rcsid[] = "$Header: autoconf.c,v 2.3 91/07/31 17:59:27 dbg Exp $";
#endif

/*
 * autoconf.c
 *	Auto-configuration.
 */

/*
 * Revision 1.2  89/07/20  18:05:40  kak
 * moved balance includes
 * 
 * Revision 1.1  89/07/05  13:15:26  kak
 * Initial revision
 * 
 * Revision 2.26  88/11/10  08:25:49  djg
 * bak242 
 * 
 * Revision 2.25  88/03/18  11:54:24  dilip
 * moved init of dmmin, dmmax, zdmap to vm_drum.c. Also, drop maxdmap
 * init, call init_dmap() instead.
 * 
 */

#include <sys/reboot.h>

#include <mach/machine.h>

#include <kern/assert.h>
#include <kern/cpu_number.h>

#include <sqt/vm_defs.h>

#include <sqt/intctl.h>
#include <sqt/ioconf.h>
#include <sqt/mutex.h>

#include <sqt/SGSproc.h>
#include <sqt/cfg.h>
#include <sqt/slic.h>
#include <sqt/slicreg.h>
#include <sqt/clkarb.h>
#include <sqt/clock.h>
#include <sqt/engine.h>

/*
 * Address of configuration table set by PROM loader.
 */
struct config_desc *va_CD_LOC;

unsigned	Nengine;			/* # processors */
struct	engine	*engine;			/* base of engine array */
struct	engine	*engine_Nengine;		/* end of engine array */
int		NFPA = 0;			/* # processors with FPA's */
int		mono_P_slic = -1;		/* no mono_P drivers == -1 */
short		fp_lights;			/* Front panel lights */
u_char		cons_scsi = 0; /* 26 */		/* console SCSI's SLIC id */
extern int	cpurate;			/* used for delays */

int		boothowto;			/* boot flags */
unsigned	sys_clock_rate;			/* # Mega Hz system runs at */

struct	bin_header int_bin_table[SLICBINS];	/* Interrupt Bin Table */
int	bin_alloc[SLICBINS];			/* for allocating vectors */

/*
 * slic_to_config[] maps SLIC number to configuration information about
 * that SLIC.
 */

struct	ctlr_desc *slic_to_config[MAX_NUM_SLIC];

/*
 * slic_to_cpu maps SLIC number to logical processor id
 */

char slic_to_cpu[MAX_NUM_SLIC];

/*
 * sec0eaddr encodes a system unique 24-bit number.  Name is historic.
 * This value is the low-order 24-bits of the ether address on SCED[0] for
 * a B8k; on a B21k it's the system-ID value from the backplane (readable thru
 * the CADM).  After system is up and /etc/init has the magic system ID number,
 * sec0eaddr holds the number of users the system can legally support.
 */

unsigned	sec0eaddr = -1;		/* system ID number */

extern char *	calloc();
int		strayint();


/*
 * configure()
 *	Scan the HW configuration information, do probes, etc,
 *	all in the name of determining what's out there.
 */

configure()
{
	register struct ctlr_desc *cd;
	register struct cntlrs	*b8k;

	/*
	 * Force io to this slic
	 */
	mono_P_slic = va_slic->sl_procid;

	/*
	 * Determine boot flags and system clock rate.
	 */

	boothowto = va_CD_LOC->c_boot_flag;
	sys_clock_rate = va_CD_LOC->c_clock_rate;

	/*
	 * Build slic to config information map.
	 */

	for (cd = PHYSTOKV(va_CD_LOC->c_ctlrs, struct ctlr_desc *);
	     cd < PHYSTOKV(va_CD_LOC->c_end_ctlrs, struct ctlr_desc *);
	     cd++)
		slic_to_config[cd->cd_slic] = cd;

	/*
	 * Do configuration/allocation of basic system components.
	 */

	conf_console();
	conf_clkarb();
	conf_proc();
	conf_mem();

	for (b8k = b8k_cntlrs; b8k->conf_b8k != NULL; b8k++)
		(*b8k->conf_b8k)();

	/*
	 * Allocate interrupt table, set up pseudo devices, and
	 * probe IO controllers (includes MBAd's, SCED's, ZDC's, etc).
	 */

	conf_intr();
	conf_pseudo();

	for (b8k = b8k_cntlrs; b8k->conf_b8k != NULL; b8k++)
		(*b8k->probe_b8k_devs)();
	setconf();
}

/*
 * conf_console()
 *	Determine Console SCSI card for putchar()'s and "boot" console device.
 */

conf_console()
{
	register struct ctlr_desc *cd;

	cd = PHYSTOKV(va_CD_LOC->c_cons, struct ctlr_desc *);
	cons_scsi = cd->cd_slic;
}

/*
 * conf_clkarb()
 *	Determine if clock arbiter is present. If present,
 *	set flag for front panel lights. And determine bus priority
 *	for slots 16-20. If all are processors then set priority to low
 *	otherwise set high (default).
 */

conf_clkarb()
{
	register struct ctlr_toc *toc =
	    PHYSTOKV(&va_CD_LOC->c_toc[SLB_CLKARBBOARD], struct ctlr_toc *);
	register struct ctlr_desc *cd;
	extern	int	light_show;

	if (toc->ct_count == 0) {
		return;
	}

	/*
	 * We're a B21k ==> set up to use front-panel LED's and get system ID
	 * from clock-arbiter board.
	 */

	cd = PHYSTOKV(&va_CD_LOC->c_ctlrs[toc->ct_start],
			struct ctlr_desc *);	/* clkarb ctlr_desc */

	fp_lights = 1;			/* use front panel LEDs */
	if (light_show > 1)
		fp_lights = -1;		/* use front-panel and processor LEDs */
	FP_IO_ONLINE;			/* ASSUME all drives online */

	sec0eaddr = cd->cd_ca_sysid & 0x00FFFFFF;

}

/*
 * conf_proc()
 *	Configure processors.
 *
 * Allocate engine table for all possible processors, but only remember
 * alive and configured processors.
 *
 * We only fill out the slic addresses in the engine structures;
 * sysinit() fills out the rest.
 *
 * We also set `Nengine' here to the # of desired processors.
 */

conf_proc()
{
	extern	int	lcpuspeed;		/* configured value */
	register struct ctlr_toc *toc;
	register struct	ctlr_desc *cd;
	register struct engine *eng;
	register int i;

	/*
	 * Get table of contents pointer for processor board.
	 */

	toc = PHYSTOKV(&va_CD_LOC->c_toc[SLB_SGSPROCBOARD],
			struct ctlr_toc *);		/* SGS processors */

	engine = (struct engine *)
		calloc((int)toc->ct_count * sizeof(struct engine));

	printf("%d processors; slic", toc->ct_count);

	cd = PHYSTOKV(&va_CD_LOC->c_ctlrs[toc->ct_start], struct ctlr_desc *);
	for (i = 0; i < toc->ct_count; i++, cd++) {
		printf(" %d", cd->cd_slic);
		if (cd->cd_diag_flag & (CFG_FAIL|CFG_DECONF))
			continue;
		eng = &engine[Nengine];
		eng->e_diag_flag = cd->cd_diag_flag;
		eng->e_slicaddr = cd->cd_slic;
		eng->e_cpu_speed = cd->cd_p_speed;

		slic_to_cpu[cd->cd_slic] = Nengine;

		machine_slot[Nengine].is_cpu = TRUE;
		machine_slot[Nengine].cpu_type = CPU_TYPE_I386;
		machine_slot[Nengine].cpu_subtype = CPU_SUBTYPE_SYMMETRY;
		machine_slot[Nengine].running = FALSE;

		/*
		 * Set the engine rate and find the slowest proccesor
		 * if not set assume minimum.
		 * Note: cpurate is recalulated to be in MIPS at the
		 * end of this routine.
		 */
		if (eng->e_cpu_speed == 0 )
			eng->e_cpu_speed = cpurate;
		if (eng->e_cpu_speed < cpurate)
			cpurate = eng->e_cpu_speed;
#ifdef	E_FPU387
		/*
		 * Set E_FPU387 and E_FPA in e_flags as appropriate.
		 * Bump NFPA for those available processors that have FPA's.
		 * Only set E_FPA if there is an FPA and it passed diagnostics.
		 */
		if (cd->cd_p_fp & SLP_387)
			eng->e_flags |= E_FPU387;
		if (cd->cd_p_fp & SLP_FPA) {
			if ((cd->cd_diag_flag & CFG_SP_FPA) == 0) {
				eng->e_flags |= E_FPA;
				++NFPA;
			}
		}
#endif	E_FPU387
		Nengine++;

	}
	engine_Nengine = &engine[Nengine];
	printf(".\n");

	if (Nengine < toc->ct_count) {
		printf("Not using processors: slic");
		cd = PHYSTOKV(&va_CD_LOC->c_ctlrs[toc->ct_start],
				struct ctlr_desc *);
		for (i = 0; i < toc->ct_count; i++, cd++) {
			if (cd->cd_diag_flag & (CFG_FAIL|CFG_DECONF))
				printf(" %d", cd->cd_slic);
		}
		printf(".\n");
	}
	/*
	 * compute cpurate for delay loops. The value should 
	 * correspond to the "mips" rating for the processor 
	 * at its running rate. 
	 * cd_p_speed: is an actual boards running rate
	 * e_cpu_speed: is the number of mips if 
	 * sys_clock_rate = 100
	 * cpurate is only used for probe routines running on the
	 * "boot" processor.
	 */
	cpurate = (lcpuspeed * cpurate) / 100;
}

/*
 * conf_intr()
 *	Allocate and initialize interrupt vector table.
 *
 * Sysinit() already set up master HW vector table.
 * Don't allocate anything for bin[0] (SW -- doesn't use this table).
 */

conf_intr()
{
	register int i;
	register int vec;
	extern	todclock();		/* tod clock handler */
	extern	hardclock();

	/*
	 * Add in local clock, tod clock to appropriate bins.
	 */

	ivecres(LCLKBIN, 1);
	ivecres(TODCLKBIN, 1);

	/*
	 * Allocate int_bin_table, init all entries to point at strayint().
	 */

	for (i = 1; i < SLICBINS; i++) {
		if (bin_intr[i] == 0)
			continue;
		assert(bin_intr[i] <= MSGSPERBIN);
		int_bin_table[i].bh_size = bin_intr[i];
		int_bin_table[i].bh_hdlrtab =
			(int(**)())calloc(bin_intr[i]*sizeof(int (*)()));
		for (vec = 0; vec < int_bin_table[i].bh_size; vec++)
			ivecinit(i, vec, strayint);
	}

	/*
	 * Set up vectors for local clock and tod clock.
	 * Must do LCLKBIN first, to insure it gets vector 0.
	 */

	ivecinit(LCLKBIN, ivecall(LCLKBIN), hardclock);
	ivecinit(TODCLKBIN, ivecall(TODCLKBIN), todclock);
}

/*
 * conf_pseudo()
 *	Call the boot procedures of pseudo-devices.
 */

conf_pseudo()
{
	register struct pseudo_dev *pd;

	printf("Pseudo devices:");
	for (pd = pseudo_dev; pd->pd_name; pd++) {
		printf(" %d %s", pd->pd_flags, pd->pd_name);
		(*pd->pd_boot)(pd->pd_flags);
	}
	printf(".\n");
}

/*
 * ivecall()
 *	Allocate a vector from a given bin.
 *
 * Insures sequential values returned per bin.
 */

u_char
ivecall(bin)
	u_char	bin;
{
	if (bin_alloc[bin] >= int_bin_table[bin].bh_size) {
		printf("Too many vectors in bin %d.\n", bin);
		panic("ivecall");
		/*NOTREACHED*/
	}
	return(bin_alloc[bin]++);
}

/*
 * strayint()
 *	Stray interrupt catcher.
 *
 * Doesn't report bin #; instead reports current value of SLIC local
 * mask which allows inference of interrupting bin.
 */

strayint(vec)
	int	vec;			/* vector number within bin */
{
	printf("Stray intr, vector %d ipl 0x%x.\n", vec, va_slic->sl_lmask);
}

/*
 * bogusint()
 *	Called from locore.s when bad vector number presented
 *	from SLIC.
 */

bogusint(bin, vec)
	unsigned bin;
	unsigned vec;
{
	printf("Bogus interrupt vector %d on bin %d.\n", vec, bin);
}
