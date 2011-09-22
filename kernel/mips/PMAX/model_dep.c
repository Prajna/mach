/* 
 * Mach Operating System
 * Copyright (c) 1993-1990 Carnegie Mellon University
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
 * Revision 2.20  93/08/10  15:15:42  mrt
 * 	Fixed clock interrupt handling for 240s.
 * 	[93/07/29  23:30:38  af]
 * 
 * 	Added support for 5000/240, from John Wroclawski (jtw@lcs.mit.edu).
 * 	[93/05/29  09:56:54  af]
 * 
 * 	Initialize the interrupt mask register on maxine and kmin.
 * 	GCC quiet.
 * 	[93/05/06  09:39:18  af]
 * 
 * Revision 2.16  93/02/05  08:04:16  danner
 * 	Clock got more generic.
 * 	[93/02/04  02:02:17  af]
 * 
 * Revision 2.15  93/01/14  17:50:37  danner
 * 	Proper spl typing.
 * 	[92/12/01            af]
 * 	Fixes for ANSI C.
 * 	[92/10/06            jvh]
 * 	Added some missing extern declarations.
 * 	[92/11/06            cmaeda]
 * 
 * Revision 2.14  92/05/05  10:46:51  danner
 * 	From jcb.
 * 	[92/05/04  11:38:27  af]
 * 
 * Revision 2.13.1.1  92/04/17  14:27:42  jcb
 * 	Set the offset for FRC counters on all machines that
 * 	can see an NSC's one.
 * 	[92/04/13            jcb]
 * 
 * Revision 2.13  92/04/01  15:15:05  rpd
 * 	On maxine and kmin, set speed of any possible scsi option board
 * 	down to 12.5 Mhz. Quadruple sigh.
 * 	[92/04/01  14:58:54  af]
 * 
 * Revision 2.12  92/03/05  11:37:15  rpd
 * 	On maxine, do not make I/O space uncached; speeds up the screen
 * 	about 30% under X11.
 * 	[92/03/04            af]
 * 
 * Revision 2.11  92/03/03  09:33:38  rpd
 * 	Support for MAXine.
 * 	[92/03/02  02:17:50  af]
 * 
 * Revision 2.10  92/02/23  19:42:21  elf
 * 	Copyright update. Added dec_check_rcline
 * 	[92/02/23            danner]
 * 
 * Revision 2.9  91/10/09  16:13:03  af
 * 	Sigh, ds 5000 with new proms AND new boot need the REX
 * 	interface, all of it.
 * 	Moved REX's memory bitmap routine here from kmin file.
 * 
 * Revision 2.8  91/08/24  12:21:51  af
 * 	Added 3min support, spl dispatcher lives here too.
 * 	[91/08/02  03:41:00  af]
 * 
 * Revision 2.7  91/05/14  17:24:58  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/02/05  17:42:56  mrt
 * 	Added author notices
 * 	[91/02/04  11:15:34  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:14:20  mrt]
 * 
 * Revision 2.5  91/01/09  19:50:15  rpd
 * 	Set pmax_memcheck() for pmaxen.
 * 	[91/01/03  02:12:58  af]
 * 
 * Revision 2.4  90/12/05  23:33:02  af
 * 
 * 
 * Revision 2.2.1.2  90/11/01  03:54:28  af
 * 	Moved hand filling routine where it belonged,
 * 	added isa_pmax() needed by the pm driver: it
 * 	doesn't have any way to distinguish between
 * 	real memory and the framebuffer on a 3max
 * 	where it MUST fail to probe.
 * 
 * Revision 2.2.1.1  90/10/03  11:57:38  af
 * 	Reflected new TC autoconf code.
 * 	[90/10/03            af]
 * 
 * Revision 2.2  90/08/07  22:25:24  rpd
 * 	New 3max-specific memory error routine.
 * 	[90/08/07  15:38:47  af]
 * 
 * Revision 2.1.1.1  90/05/30  15:49:37  af
 * 	Moved over to pure kernel.
 * 
 * 
 * Revision 2.1.1.1  90/05/20  14:25:23  af
 * 	Created.
 * 	[90/04/17            af]
 */
/*
 *	File: model_dep.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	4/90
 *
 *	Cold-configuration functions for specific DEC mips boxes.
 *
 *	This file defines model-dependent changes that must be
 *	applied to various modules before autoconf time.
 *	This includes handling of bus errors, strange SPL setups,
 *	minor variations over standard components (clock), etc.
 */

#include <mach/std_types.h>
#include <machine/machspl.h>		/* spl definitions */
#include <mips/mips_cpu.h>
#include <mips/mips_box.h>
#include <mips/prom_interface.h>

#include <mips/PMAX/tc.h>
#include <mips/PMAX/kn01.h>
#include <mips/PMAX/kn02.h>
#include <mips/PMAX/kmin.h>
#include <mips/PMAX/kmin_cpu.h>
#include <mips/PMAX/maxine.h>
#include <mips/PMAX/maxine_cpu.h>
#include <mips/PMAX/kn03.h>
#include <mips/PMAX/kn03_cpu.h>

/* IPL dispatcher */

extern spl_t
	simple_spl0(), simple_splsoftclock(), simple_splnet(), 
	simple_splimp(), simple_splbio(), simple_spltty(), simple_splclock(), 
	simple_splclock(), simple_splvm(), simple_splhigh();
extern void
	simple_splx( spl_t );

struct _spl_vector	spl_vector = {
	simple_splx,
	simple_spl0,
	simple_splsoftclock,
	simple_splnet,
	simple_splimp,
	simple_splbio,
	simple_spltty,
	simple_splclock,
	simple_splvm,
	simple_splhigh
};

/*
 * Proms
 */
extern int
	prom_arg2;
#define	REX_PROMS	0x30464354

d_find_systype()
{
	return string_to_int(prom_getenv("systype"));
}

int	(*find_systype)() = d_find_systype;

void
which_prom()
{
	extern void	kmin_halt(), kmin_reboot();
	extern int	kmin_putchar();
	extern char	*kmin_getenv();
	extern int	kmin_find_systype();

	if (prom_arg2 == REX_PROMS) {
		prom_call.halt		= kmin_halt;
		prom_call.reboot	= kmin_reboot;
		prom_call.putchar	= kmin_putchar;
		prom_call.getenv	= kmin_getenv;

		find_systype = kmin_find_systype;
	}
}

boolean_t
rex_get_memory_bitmap(bmp, blp, psp)
	int	**bmp;
	int	*blp, *psp;
{
	register int	val;
	struct kmin_bitmap {
		int	pagesize;
		int	bitmap[64*1024*1024 - 4];
	} *bm;

	/* some free 64k */
	bm = (struct kmin_bitmap *)0x80020000;
	*blp = kmin_memory_bitmap(bm);
	*bmp = bm->bitmap;
	*psp = bm->pagesize;

	return FALSE;
}


dec_check_rcline(rclinep)
	int	*rclinep;
{
	char *cp = prom_getenv("osconsole");

	if (cp) {
		/* pmax way */
		if (isa_pmax() && (string_to_int(cp) & 0x8))
			*rclinep = 3;
#if 0
		/* TC way, but is this right */
		if (!isa_pmax())
			*rclinep = string_to_int(cp);
#endif
	}
}

/* Model-dependent customizations of the default code */

extern vm_size_t
	(*mipsbox_steal_memory)(),
	screen_memory_alloc(), kn02ba_steal_memory();
extern boolean_t (*get_memory_bitmap_from_prom)();
extern          kn02_tcintr(), kn01_mc_intr(), stray_intr(), kn02_errintr();
extern int	asc_clock_speed_in_mhz[];
extern int	kn02_dma_ops[];


boolean_t	ignore_power_supply_overheating = FALSE;

static char pmax_box = 0;
isa_pmax() {return pmax_box;}

kn01_model_dep()
{
	/*
	 *	The pmax is our "generic" DEC box.
	 */
	extern boolean_t kn01_memcheck();

	set_dz_address(0, PHYS_TO_K1SEG(KN01_SYS_DZ), 0);

	pmax_box = 1;
	pmax_memcheck = kn01_memcheck;

	mipsbox_steal_memory = screen_memory_alloc;
}

kn02_model_dep()
{
	/*
	 * 3max 
	 */
	extern int      ((*mips_box_buserror[]) ()), wrclr_buserror();
	extern int      (*memory_sizing_routine) (), size_memory_trusting_prom();
	extern          (*identify_devices) (), config_3max_routine();
	extern void	kn02_slot_hand_fill();
	extern int	kn02_dma_ops[];

	mips_box_buserror[0] = wrclr_buserror;

	patch_simple_spl(simple_splimp, 3);
	patch_simple_spl(simple_spltty, 3);
	patch_simple_spl(simple_splclock, 4);

	memory_sizing_routine = size_memory_trusting_prom;
	if (prom_arg2 == REX_PROMS)
		get_memory_bitmap_from_prom = rex_get_memory_bitmap;

	pmax_intr2 = kn02_tcintr;
	pmax_intr3 = kn01_mc_intr;
	pmax_intr4 = stray_intr;
	pmax_intr5 = kn02_errintr;
	pmax_intr6 = stray_intr;

	identify_devices = config_3max_routine;

	mc_probe(PHYS_TO_K1SEG(KN02_SYS_CLOCK));
	set_dz_address(0, PHYS_TO_K1SEG(KN02_SYS_DZ), 1);
	set_se_switch(1);
	asc_set_dmaops(0, kn02_dma_ops);

	/* If we have a free running counter from Network Systems Corp. */
	frc_set_address(0, 0x1000);
	frc_set_address(1, 0x1000);
	frc_set_address(2, 0x1000);

	mipsbox_steal_memory = screen_memory_alloc;

	/*
	 * Printing on console might require use of
	 * a TURBOchannel (graphic) option.
	 */
	tc_slot_hand_fill = kn02_slot_hand_fill;
	tc_find_all_options();

}

extern int	kmin_dma_ops[];

kmin_model_dep()
{
	/*
	 * 3min
	 */
	extern spl_t
		kmin_spl0(), kmin_splsoftclock(), kmin_splnet(), 
		kmin_splimp(), kmin_splbio(), kmin_spltty(), kmin_splclock(), 
		kmin_splclock(), kmin_splvm(), kmin_splhigh();
	extern void
		kmin_splx( spl_t );
	extern boolean_t (*get_memory_bitmap_from_prom)();
	extern int      ((*mips_box_buserror[]) ()), wrclr_buserror();
	extern int	*wrclr_buserror_address;
	extern int      (*memory_sizing_routine)(),size_memory_trusting_prom();
	extern		kmin_tc0intr(), kmin_tc1intr(), kmin_tc2intr(),
			kmin_tc3intr(), kn02ba_haltintr();
	extern          (*identify_devices) (), config_3max_routine();
	extern void	kmin_slot_hand_fill();
	extern int	kmin_enable_interrupt();
	extern int	kn02_dma_ops[];
	extern int	asc_clock_speed_in_mhz[];

	mips_box_buserror[0] = wrclr_buserror;
	wrclr_buserror_address = (int*)PHYS_TO_K1SEG(KMIN_REG_TIMEOUT);

	spl_vector.splx_function		= kmin_splx;
	spl_vector.spl0_function		= kmin_spl0;
	spl_vector.splsoftclock_function	= kmin_splsoftclock;
	spl_vector.splnet_function		= kmin_splnet;
	spl_vector.splimp_function		= kmin_splimp;
	spl_vector.splbio_function		= kmin_splbio;
	spl_vector.spltty_function		= kmin_spltty;
	spl_vector.splclock_function		= kmin_splclock; 
	spl_vector.splvm_function		= kmin_splvm;
	spl_vector.splhigh_function		= kmin_splhigh;

	memory_sizing_routine = size_memory_trusting_prom;
	get_memory_bitmap_from_prom = rex_get_memory_bitmap;

	pmax_intr2 = kmin_tc0intr;
	pmax_intr3 = kmin_tc1intr;
	pmax_intr4 = kmin_tc2intr;
	pmax_intr5 = kmin_tc3intr;
	pmax_intr6 = kn02ba_haltintr;

	identify_devices = config_3max_routine;

	mc_probe(PHYS_TO_K1SEG(KMIN_SYS_CLOCK));

	asc_set_dmaops(0, kmin_dma_ops);

	asc_set_dmaops(1, kn02_dma_ops);	/* in option slot, if any */
	asc_clock_speed_in_mhz[1] = 12;			/* 12.5 actually */

	asc_set_dmaops(2, kn02_dma_ops);	/* in option slot, if any */
	asc_clock_speed_in_mhz[2] = 12;			/* 12.5 actually */

	asc_set_dmaops(3, kn02_dma_ops);	/* in option slot, if any */
	asc_clock_speed_in_mhz[3] = 12;			/* 12.5 actually */

	/* If we have a free running counter from Network Systems Corp. */
	frc_set_address(0, 0x1000);
	frc_set_address(1, 0x1000);
	frc_set_address(2, 0x1000);

	tc_enable_interrupt = kmin_enable_interrupt;

	/* clear leftover interrupts from asic chip,
	   enable all 'devices', but intrs are still off */
	{
		extern unsigned	  kmin_tc3_imask;
		*(volatile unsigned*)(PHYS_TO_K1SEG(KMIN_REG_INTR)) = 0;
		*(volatile unsigned*)(PHYS_TO_K1SEG(KMIN_REG_IMSK)) = KMIN_IM5;
		kmin_tc3_imask = KMIN_IM0;
	}

	mipsbox_steal_memory = kn02ba_steal_memory;

	/*
	 * Printing on console might require use of
	 * a TURBOchannel (graphic) option.
	 */
	tc_max_slot = KMIN_TC_MAX;
	tc_min_slot = KMIN_TC_MIN;
	tc_slot_phys_base[0] = KMIN_PHYS_TC_0_START;
	tc_slot_phys_base[1] = KMIN_PHYS_TC_1_START;
	tc_slot_phys_base[2] = KMIN_PHYS_TC_2_START;
	tc_slot_phys_base[3] = KMIN_PHYS_TC_3_START;
	tc_slot_phys_base[4] = -1;
	tc_slot_phys_base[5] = -1;
	tc_slot_phys_base[6] = -1;
	tc_slot_phys_base[7] = -1;

	tc_slot_hand_fill = kmin_slot_hand_fill;

	tc_find_all_options();

}

xine_model_dep()
{
	/*
	 * MAXine
	 */
	extern boolean_t pmap_uncache_io_space;

	extern spl_t
		xine_spl0(), xine_splsoftclock(), xine_splnet(), 
		xine_splimp(), xine_splbio(), xine_spltty(), xine_splclock(), 
		xine_splclock(), xine_splvm(), xine_splhigh();
	extern void
		xine_splx( spl_t );

	extern boolean_t (*get_memory_bitmap_from_prom)();
	extern int      ((*mips_box_buserror[]) ()), wrclr_buserror();
	extern int	*wrclr_buserror_address;
	extern int      (*memory_sizing_routine)(),size_memory_trusting_prom();
	extern		xine_tc3intr(), kn02ba_haltintr(), kn02ba_errintr(),
			xine_mc_intr();
	extern          (*identify_devices) (), config_3max_routine();
	extern		stray_intr();
	extern void	xine_slot_hand_fill();
	extern int	xine_enable_interrupt();
	extern int	kn02_dma_ops[];
	extern int	asc_clock_speed_in_mhz[];

	pmap_uncache_io_space = FALSE;

	mips_box_buserror[0] = wrclr_buserror;
	wrclr_buserror_address = (int*)PHYS_TO_K1SEG(KMIN_REG_TIMEOUT);

	spl_vector.splx_function		= xine_splx;
	spl_vector.spl0_function		= xine_spl0;
	spl_vector.splsoftclock_function	= xine_splsoftclock;
	spl_vector.splnet_function		= xine_splnet;
	spl_vector.splimp_function		= xine_splimp;
	spl_vector.splbio_function		= xine_splbio;
	spl_vector.spltty_function		= xine_spltty;
	spl_vector.splclock_function		= xine_splclock; 
	spl_vector.splvm_function		= xine_splvm;
	spl_vector.splhigh_function		= xine_splhigh;

	memory_sizing_routine = size_memory_trusting_prom;
	get_memory_bitmap_from_prom = rex_get_memory_bitmap;

	pmax_intr2 = stray_intr;	/* periodic intr, unused */
	pmax_intr3 = xine_mc_intr;
	pmax_intr4 = kn02ba_errintr;
	pmax_intr5 = xine_tc3intr;
	pmax_intr6 = kn02ba_haltintr;

	identify_devices = config_3max_routine;

	mc_probe(PHYS_TO_K1SEG(XINE_SYS_CLOCK));

	asc_set_dmaops(0, kmin_dma_ops);

	asc_set_dmaops(1, kn02_dma_ops);	/* in option slot, if any */
	asc_clock_speed_in_mhz[1] = 12;			/* 12.5 actually */

	asc_set_dmaops(2, kn02_dma_ops);	/* in option slot, if any */

	asc_clock_speed_in_mhz[2] = 12;			/* 12.5 actually */

	/* If we have a free running counter from Network Systems Corp. */
	frc_set_address(1, 0x1000);
	frc_set_address(2, 0x1000);

	tc_enable_interrupt = xine_enable_interrupt;

	/* clear leftover interrupts from asic chip,
	   enable all 'devices', but intrs are still off */
	{
		extern unsigned	  xine_tc3_imask;
		*(volatile unsigned*)(PHYS_TO_K1SEG(XINE_REG_INTR)) = 0;
		*(volatile unsigned*)(PHYS_TO_K1SEG(XINE_REG_IMSK)) = XINE_IM4;

		xine_tc3_imask = XINE_IM0;
	}

	mipsbox_steal_memory = kn02ba_steal_memory;

	/*
	 * Printing on console might require use of
	 * a TURBOchannel (graphic) option.
	 */
	tc_max_slot = XINE_TC_MAX;
	tc_min_slot = XINE_TC_MIN;
	tc_slot_phys_base[0] = XINE_PHYS_TC_0_START;
	tc_slot_phys_base[1] = XINE_PHYS_TC_1_START;
	tc_slot_phys_base[2] = XINE_PHYS_TC_3_START;
	tc_slot_phys_base[3] = -1;
	tc_slot_phys_base[4] = -1;
	tc_slot_phys_base[5] = -1;
	tc_slot_phys_base[6] = -1;
	tc_slot_phys_base[7] = -1;

	tc_slot_hand_fill = xine_slot_hand_fill;

	tc_find_all_options();

}

kn03_model_dep()
{
	/*
	 * 3max+
	 */
	extern spl_t
		kn03_spl0(), kn03_splsoftclock(), kn03_splnet(), 
		kn03_splimp(), kn03_splbio(), kn03_spltty(), kn03_splclock(), 
		kn03_splclock(), kn03_splvm(), kn03_splhigh();
	extern void	kn03_splx( spl_t );
	extern boolean_t (*get_memory_bitmap_from_prom)();
	extern int      ((*mips_box_buserror[]) ()), wrclr_buserror();
	extern int	*wrclr_buserror_address;
	extern int      (*memory_sizing_routine)(),size_memory_trusting_prom();
	extern int	kn03_tcintr();
	extern int	kn03_errintr();
	extern int	kn03_haltintr();
	extern int	kn03_mc_intr();
	extern int	(*identify_devices) (), config_3max_routine();
	extern void	kn03_slot_hand_fill();
	extern int	kn03_enable_interrupt();
	extern int	kmin_dma_ops[];
	extern int	kn02_dma_ops[];
	extern int	asc_clock_speed_in_mhz[];
	extern unsigned kn03_imask;
	extern vm_size_t kn03_steal_memory();

	mips_box_buserror[0] = wrclr_buserror;
	wrclr_buserror_address = (int*)PHYS_TO_K1SEG(KN03_SYS_ERRADR);

	spl_vector.splx_function		= kn03_splx;
	spl_vector.spl0_function		= kn03_spl0;
	spl_vector.splsoftclock_function	= kn03_splsoftclock;
	spl_vector.splnet_function		= kn03_splnet;
	spl_vector.splimp_function		= kn03_splimp;
	spl_vector.splbio_function		= kn03_splbio;
	spl_vector.spltty_function		= kn03_spltty;
	spl_vector.splclock_function		= kn03_splclock; 
	spl_vector.splvm_function		= kn03_splvm;
	spl_vector.splhigh_function		= kn03_splhigh;

	/* 
	 * Clear stray interrupts from hardware. Enable all devices in
	 * software interrupt mask, but leave hardware off.
	 */
	*((volatile unsigned *)(PHYS_TO_K1SEG(KN03_REG_INTR))) = 0;
	*((volatile unsigned *)(PHYS_TO_K1SEG(KN03_REG_IMSK))) = KN03_IM4;
	kn03_imask = KN03_IM3;

	memory_sizing_routine = size_memory_trusting_prom;
	get_memory_bitmap_from_prom = rex_get_memory_bitmap;

	pmax_intr2 = kn03_tcintr;
	pmax_intr3 = kn03_mc_intr;
	pmax_intr4 = stray_intr;
	pmax_intr5 = kn03_errintr;
	pmax_intr6 = kn03_haltintr;

	identify_devices = config_3max_routine;

	mc_probe(PHYS_TO_K1SEG(KN03_SYS_CLOCK));

	asc_set_dmaops(0, kmin_dma_ops);
	asc_clock_speed_in_mhz[1] = 25;

	asc_set_dmaops(1, kn02_dma_ops);	/* in option slot, if any */

	asc_set_dmaops(2, kn02_dma_ops);	/* in option slot, if any */

	asc_set_dmaops(3, kn02_dma_ops);	/* in option slot, if any */

	/* If we have a free running counter from Network Systems Corp. */
	frc_set_address(0, 0x1000);
	frc_set_address(1, 0x1000);
	frc_set_address(2, 0x1000);
	
	tc_enable_interrupt = kn03_enable_interrupt;

	mipsbox_steal_memory = kn03_steal_memory;

	/*
	 * Printing on console might require use of
	 * a TURBOchannel (graphic) option.
	 */
	tc_max_slot = KN03_TC_MAX;
	tc_min_slot = KN03_TC_MIN;
	tc_slot_phys_base[0] = KN03_PHYS_TC_0_START;
	tc_slot_phys_base[1] = KN03_PHYS_TC_1_START;
	tc_slot_phys_base[2] = KN03_PHYS_TC_2_START;
	tc_slot_phys_base[3] = KN03_PHYS_TC_3_START;
	tc_slot_phys_base[4] = -1;
	tc_slot_phys_base[5] = -1;
	tc_slot_phys_base[6] = -1;
	tc_slot_phys_base[7] = -1;

	tc_slot_hand_fill = kn03_slot_hand_fill;

	tc_find_all_options();
}

/* Recognize which machine we are on */

mips_box_model_dep()
{
	int temp;

	temp = (*find_systype)();

	if (dec_cputype(temp) != 130) {
		dprintf("Unknown System type: 0x%x.  Assuming pmax-like\n",
			temp);
		return kn01_model_dep();
	}

	switch(dec_systype(temp)) {
	case 1:	/* DS3100 Pmax */
		return kn01_model_dep();
	case 2:	/* DS5000 3max */
		return kn02_model_dep();
	case 3:	/* DS5000/100 3min */
		return kmin_model_dep();
	case 4:	/* DS5000/240 3max+ */
		return kn03_model_dep();
	case 7: /* Personal 5000/2x */
		return xine_model_dep();
/* unsupported known types */
	case 5:	/* DS5800 Isis */
	case 6:	/* DS5400 MIPSfair */
	case 11:/* DS5500 MIPSfair-2 */
	case 12:/* DS5100 MIPSMATE */
/* unknown type */
	default:
		dprintf("Unknown Box 0x%x.  Assuming pmax-like\n",
			dec_systype(temp));
		return kn01_model_dep();
	}
}
