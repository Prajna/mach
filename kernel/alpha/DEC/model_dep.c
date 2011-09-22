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
 * $Log:	model_dep.c,v $
 * Revision 2.7  93/05/10  22:16:46  rvb
 * 	Fixed TC probing.
 * 	[93/05/07  14:44:51  af]
 * 
 * Revision 2.6  93/05/10  20:07:04  rvb
 * 	Fixed TC probing.
 * 	[93/05/07  14:44:51  af]
 * 
 * Revision 2.5  93/03/11  13:49:31  danner
 * 	Move platforms.h def before use.
 * 
 * Revision 2.4  93/03/09  10:48:29  danner
 * 	Reflected changes in prom dispatching.
 * 	[93/02/20            af]
 * 
 * 	Add sgmap_init()
 * 	[93/02/08            jeffreyh]
 * 
 * Revision 2.3  93/03/04  09:26:41  danner
 * 	Wrapped includes in ifdef FLAMINGO.
 * 	[93/03/01            danner]
 * 
 * Revision 2.2  93/02/05  07:56:58  danner
 * 	Flamingo changes
 * 	[93/01/12            jeffreyh]
 * 	Fixed prom dispatch init.
 * 	[93/01/16  12:19:28  af]
 * 
 * 	No more stealing, now we use grab_contiguous_pages.
 * 	[92/12/25  01:46:02  af]
 * 
 * 	Created.
 * 	[92/06/02            af]
 * 
 */
/*
 *	File: model_dep.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Cold-configuration functions for specific DEC ALPHA boxes.
 *
 *	This file defines model-dependent changes that must be
 *	applied to various modules before autoconf time.
 *	This includes machine checks and minor variations
 *	over otherwise common code.
 */

#include <platforms.h>

#include <mach/std_types.h>
#include <alpha/alpha_cpu.h>
#include <alpha/prom_interface.h>

#ifdef FLAMINGO
#include <alpha/DEC/tc.h>
#include <alpha/DEC/flamingo.h>
#endif 

#include <alpha/alpha_scb.h>


/* sanity */
#ifdef	cpu_number
#undef	cpu_number
#endif

extern int		(*console_putc)(), (*console_getc)(), (*console_pollc)();
extern int 		alpha_con_putc(), alpha_con_getc(), wbflush();

/*
 * Utils
 */
int null_interrupt_handler_counter;

static void 
null_interrupt_handler()
{
	/*
	 * You do not seriously want to keep on running with a recurring
	 * unhandled interrupt, now do you ? 
	 */
	if (++null_interrupt_handler_counter > 1000000)
		prom_halt();
}

/*
 * Proms, and all the information we want from them
 */

struct restart_blk	*alpha_hwrpb = 0;	/* not in BSS ! */
int			alpha_console = 9999;


void
init_prom_interface()
{
	struct restart_blk *r;
	struct console_routine_blk *c;
	vm_offset_t	addr;

#if	0
	we might rid of the default mappings early on so.. no RESTART_ADDR
	r = (struct restart_blk *)RESTART_ADDR;
	alpha_hwrpb = (struct restart_blk *) (r->my_phys_address);
#else
	/* In start() we get the phys address of the HWRPB into alpha_hwrpb */
#endif
	alpha_hwrpb = (struct restart_blk *) PHYS_TO_K0SEG(alpha_hwrpb);

	r = alpha_hwrpb;

	c = (struct console_routine_blk *)
		((char*)r + r->console_routine_block_offset);

#if 1
	prom_dispatch_v.routine_arg = c->dispatch_func_desc;
	prom_dispatch_v.routine = c->dispatch_func_desc->code;
#else
	/* XXXX  we really should call the fixup routine  XXXX */
	addr = c->dispatch_func_phys;
	prom_dispatch_v.routine = (int (*)()) PHYS_TO_K0SEG(addr);

	/* this would probably be changed by fixup */
	prom_dispatch_v.routine_arg = (vm_offset_t)
		c->dispatch_func_desc;

	/* problem is.. console code is 32bit only. Aurgh!! */
#endif

	/*
	 * Now look for console tty
	 */
	{
		char buf[4];
		prom_getenv( PROM_E_TTY_DEV, buf, 4);
		alpha_console = buf[0] - '0';
	}

	/*
	 * Might as well use prom until something better comes along
	 */
	{
		extern rcline;

		console_putc = alpha_con_putc;
		console_getc = alpha_con_getc;
		console_pollc = wbflush;
		rcline = 0; /* No dup chars this early on... */
	}

}

natural_t
alpha_get_system_type()
{
	return alpha_hwrpb->system_type;
}

/*
 * Rconsole
 */
find_rconsole( int * rclinep)
{
	switch (alpha_get_system_type()) {

	    case SYSTEM_TYPE_FLAMINGO:
#if later
		/* no rconsole if done in HW */
		*rclinep = (*(int *)0xfffffc01f0080220 & 0x80) ? 0 : 3;
#else
		*rclinep = 3;
#endif
		break;

	    default:
		*rclinep = 0;
	}
}


/*
 * Model-dependent customizations of the default code
 */

#if ADU

int adu_debuggo = 0;

adu_model_dep()
{
	extern vm_offset_t	tv_map_console();

	tv_find_all_options();

if (adu_debuggo) gimmeabreak();
	adu_sl_init(0,cpu_number(),tv_map_console(),TRUE);
	adu_se_cold_init(0,cpu_number(),tv_map_console());
}

#endif	/* ADU */

#if FLAMINGO

extern void	kn15aa_iointr(), config_kn15aa_routine();

kn15aa_model_dep()
{
	extern void		kn15aa_slot_hand_fill(),
				kn15aa_machine_check();
	extern vm_size_t	kn15aa_enable_interrupt();
	extern void		(*alpha_machine_check)();
	extern char		kn15aa_dma_ops[];
	extern vm_size_t	(*alphabox_steal_memory)(),
				screen_memory_alloc();

	/* Screens */

	alphabox_steal_memory = screen_memory_alloc;

	/* Machine check handler */

	alpha_machine_check = kn15aa_machine_check;

	asc_set_dmaops(0, kn15aa_dma_ops);

#if notyet
	asc_set_dmaops(1, kn02_dma_ops);	/* in option slot, if any */
	asc_clock_speed_in_mhz[1] = 25;			

	asc_set_dmaops(2, kn02_dma_ops);	/* in option slot, if any */
	asc_clock_speed_in_mhz[2] = 25;			

	asc_set_dmaops(3, kn02_dma_ops);	/* in option slot, if any */
	asc_clock_speed_in_mhz[3] = 25;			

	asc_set_dmaops(4, kn02_dma_ops);	/* in option slot, if any */
	asc_clock_speed_in_mhz[4] = 25;			

	asc_set_dmaops(5, kn02_dma_ops);	/* in option slot, if any */
	asc_clock_speed_in_mhz[5] = 25;			

	asc_set_dmaops(6, kn02_dma_ops);	/* in option slot, if any */
	asc_clock_speed_in_mhz[6] = 25;			

	asc_set_dmaops(7, kn02_dma_ops);	/* in option slot, if any */
	asc_clock_speed_in_mhz[7] = 25;			

#endif

	/* Machdep interrupt vectors */

	alpha_set_scb_entry( KN15AA_IO_INTERRUPT_SCB, kn15aa_iointr);
        alpha_set_scb_entry( SCB_CLOCK, null_interrupt_handler);


	/* Bus molding */

	tc_enable_interrupt = kn15aa_enable_interrupt;

	tc_max_slot = KN15AA_TC_MAX;
	tc_min_slot = KN15AA_TC_MIN;
	/*
	 * Dense addresses!
	 */
	tc_slot_phys_base[0] = KN15AA_PHYS_TC_0_START_D;
	tc_slot_phys_base[1] = KN15AA_PHYS_TC_1_START_D;
	tc_slot_phys_base[2] = KN15AA_PHYS_TC_2_START_D;
	tc_slot_phys_base[3] = KN15AA_PHYS_TC_3_START_D;
	tc_slot_phys_base[4] = KN15AA_PHYS_TC_4_START_D;
	tc_slot_phys_base[5] = KN15AA_PHYS_TC_5_START_D;
	tc_slot_phys_base[6] = KN15AA_PHYS_TC_6_START_D;
	tc_slot_phys_base[7] = KN15AA_PHYS_TC_7_START_D;

	tc_slot_hand_fill = kn15aa_slot_hand_fill;
	sgmap_init();
	tc_find_all_options();
}

#endif /* FLAMINGO */


/*
 * Recognize which machine we are on
 */
alpha_box_model_dep()
{
	switch (alpha_get_system_type()) {

#if ADU
	    case SYSTEM_TYPE_ADU:
		adu_model_dep();
		break;
#endif

#if FLAMINGO
	    case SYSTEM_TYPE_FLAMINGO:
		kn15aa_model_dep();
		break;
#endif

	    case SYSTEM_TYPE_COBRA:
	    case SYSTEM_TYPE_RUBY:
	    case SYSTEM_TYPE_MANNEQUIN:
	    case SYSTEM_TYPE_JENSEN:
	    default:
		/* dprintf should be ok cuz prom callbacks */
		dprintf("alpha_box_model_dep: Unsupported system type x%x",
		      alpha_get_system_type());
		prom_halt();
	}
}

/*
 * Some machines (PROMS !) need resetting on halt
 * in order to reboot properly later on.  So make
 * sure we get clean to proms for all machines.
 */
void alpha_reset_before_reboot()
{
	stopclocks();
	switch (alpha_get_system_type()) {
#ifdef	FLAMINGO
	case SYSTEM_TYPE_FLAMINGO:
		kn15aa_set_ioslot( 6, 1, FALSE);/* clear sgdma for prom */
		/* put isdn and lance in reset */
		*(unsigned int *)0xfffffc01f0080200 |= 1100;
		break;
#endif
	default:	break;
	}
}
