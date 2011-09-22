/* 
 * Mach Operating System
 * Copyright (c) 1992-1989 Carnegie Mellon University
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
 * $Log:	kmin.c,v $
 * Revision 2.9  93/05/30  21:08:13  rvb
 * 	asic_init() interface changed.
 * 	[93/05/29  09:54:25  af]
 * 
 * Revision 2.8  93/05/15  19:12:11  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.7  93/05/10  21:20:16  rvb
 * 	Take the HALT button interrupt.
 * 	No more sys/types.h
 * 	[93/05/06  09:43:20  af]
 * 
 * Revision 2.6  93/02/05  08:03:51  danner
 * 	Clock got more generic.
 * 	[93/02/04  02:02:48  af]
 * 
 * 	Proper spl typing.
 * 	[92/12/01            af]
 * 
 * Revision 2.5  92/03/02  18:33:36  rpd
 * 	Made some code common with MAXine.
 * 	[92/03/02  02:32:51  af]
 * 
 * Revision 2.4  92/02/23  19:41:37  elf
 * 	Copyright update, removed af mouse debug code.
 * 	[92/02/23            danner]
 * 
 * Revision 2.3  91/10/09  16:12:35  af
 * 	Moved REX's memory bitmap routine elsewhere, it is used
 * 	also by 3maxen with the new boot.
 * 
 * Revision 2.2  91/08/24  12:21:01  af
 * 	Created, from the DEC specs:
 * 	"3MIN System Module Functional Specification"  Revision 1.7
 * 	Workstation Systems Engineering, Palo Alto, CA. Sept 14, 1990.
 * 	"KN02BA Daughter Card Functional Specification" Revision 1.0
 * 	Workstation Systems Engineering, Palo Alto, CA. Aug  14, 1990.
 * 	[91/06/21            af]
 * 
 */
/*
 *	File: kmin.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/91
 *
 *	Routines specific to the KN02BA/KN02DA processors and 3MIN
 *	system module (54-20604-01)
 */

#include <mach/std_types.h>
#include <mach/vm_param.h>
#include <machine/machspl.h>
#include <chips/serial_defs.h>

#include <mips/thread.h>
#include <mips/mips_cpu.h>
#include <mips/mips_box.h>
#include <mips/prom_interface.h>
#include <mips/PMAX/kmin.h>
#include <mips/PMAX/kmin_cpu.h>
#include <mips/PMAX/tc.h>

#define	KMIN_INTR		PHYS_TO_K1SEG(KMIN_REG_INTR)
#define	KMIN_IMSK		PHYS_TO_K1SEG(KMIN_REG_IMSK)


/*
 *	Object:
 *		kmin_tc3_imask			EXPORTED unsigned int
 *
 *	Pretend some devices were not present.  The spl system
 *	for the kmin might logically enable a device, but esp
 *	during autoconf we might want to ignore it.
 *	See kmin_splx/kmin_spl0 for where and how this is used.
 *
 */
unsigned kmin_tc3_imask = 0;

/*
 *	Object:
 *		kmin_tc0intr			EXPORTED function
 *		kmin_tc1intr			EXPORTED function
 *		kmin_tc2intr			EXPORTED function
 *
 *	Handle interrupts from the TURBOchannel, option slots
 *
 */

kmin_tc0intr(st,spllevel)
	register spl_t spllevel;
{
	/* if we took an interrupt from this slot clearly
	   the system slot is enabled, right ? */
	spllevel |= (KMIN_SR_IM0<<KMIN_SR_SHIFT);
	return
		(*(tc_slot_info[0].intr))(tc_slot_info[0].unit,spllevel);
}

kmin_tc1intr(st,spllevel)
	register spl_t	spllevel;
{
	spllevel |= (KMIN_SR_IM0<<KMIN_SR_SHIFT);
	return
		(*(tc_slot_info[1].intr))(tc_slot_info[1].unit,spllevel);
}

kmin_tc2intr(st,spllevel)
	register spl_t	spllevel;
{
	spllevel |= (KMIN_SR_IM0<<KMIN_SR_SHIFT);
	return
		(*(tc_slot_info[2].intr))(tc_slot_info[2].unit,spllevel);
}

/*
 *	Object:
 *		kmin_tc3intr			EXPORTED function
 *
 *	Handle interrupts from the TURBOchannel, system slot
 *
 */
extern boolean_t ignore_power_supply_overheating;

kmin_tc3intr(st,spllevel)
	unsigned st;
	spl_t	 spllevel;
{
	register unsigned int	intr = *(volatile int *)KMIN_INTR;
	volatile unsigned int	*imaskp = (volatile unsigned int *)KMIN_IMSK;
	unsigned int		old_mask;

	/* masked interrupts are still observable */
	old_mask = *imaskp;
	intr &= old_mask;

	/* Interrupts from same slot are ok if from another device */
	spllevel |= (spllevel >> 1) & SR_INT_MASK;

	if (intr & KMIN_INTR_TIMEOUT)
		kn02ba_errintr(st, 0);

	else if (intr & KMIN_INTR_CLOCK) {
		register spl_t	modspl;

		/* expand mc_clock() inline because basepri */
		modspl = spllevel | (KMIN_SR_IM4 << KMIN_SR_SHIFT);
		mc_intr(modspl);
		splx(modspl);
		clock_interrupt( tick, st&SR_KUo, old_mask == kmin_tc3_imask);
	}

	else if (intr & KMIN_INTR_SCC_0)
		(*(tc_slot_info[KMIN_SCC0_SLOT].intr))
			(tc_slot_info[KMIN_SCC0_SLOT].unit,
			 spllevel | (KMIN_SR_IM3<<KMIN_SR_SHIFT));

	else if (intr & KMIN_INTR_SCC_1)
		(*(tc_slot_info[KMIN_SCC1_SLOT].intr))
			(tc_slot_info[KMIN_SCC1_SLOT].unit,
			 spllevel | (KMIN_SR_IM3<<KMIN_SR_SHIFT));

	else if (intr & KMIN_INTR_SCSI)
		(*(tc_slot_info[KMIN_SCSI_SLOT].intr))
			(tc_slot_info[KMIN_SCSI_SLOT].unit,
			 spllevel|(KMIN_SR_IM2<<KMIN_SR_SHIFT));

	else if (intr & KMIN_INTR_LANCE)
		(*(tc_slot_info[KMIN_LANCE_SLOT].intr))
			(tc_slot_info[KMIN_LANCE_SLOT].unit,
			 spllevel | (KMIN_SR_IM1<<KMIN_SR_SHIFT));

	else if (intr & KMIN_INTR_ASIC)
		(*(tc_slot_info[KMIN_ASIC_SLOT].intr))
			(tc_slot_info[KMIN_ASIC_SLOT].unit,
			 spllevel /* UNMODIFIED! */,
			 intr);

	else if (intr & KMIN_INTR_PBNO)
		kn02ba_haltintr(st, spllevel);

#define	FIRE_ALARM
#ifdef	FIRE_ALARM
	else {
		static int user_warned = 0;

		if (user_warned && ((intr & KMIN_INTR_PSWARN) == 0)) {
			*imaskp = 0;
			if (!ignore_power_supply_overheating)
				printf("%s\n", "Power supply ok now.");
			user_warned = 0;
		}
		if ((intr & KMIN_INTR_PSWARN) && (user_warned < 3)) {
			*imaskp = 0;
			user_warned++;
			if (!ignore_power_supply_overheating)
				printf("%s\n", "Power supply overheating");
		}
	}
#endif

	(void) simple_splhigh();
	*imaskp = old_mask;
}

/*
 *	Object:
 *		kmin_bad_intr		EXPORTED function
 *
 *	Handle an undesireable interrupt.  This is used
 *	almost exclusively during autoconf, to be able
 *	to enable interrupts at arbitrary priorities and
 *	still ignore some devices.  The problem we have is
 *	that the system slot (3) has a mask register but
 *	the option slots 0-2 don't.  When we get here we
 *	got an interrupt from one of the slots 0-2.  We
 *	play with the status register to make sure we will
 *	ignore the interrupt even if we do not clear it.
 *
 */
kmin_bad_intr(st,spllevel,ss)
	spl_t			spllevel;
	struct mips_saved_state	*ss;
{
	register unsigned	bit = 0;

	/* but which slot are we ? oh well .. */
	if (pmax_intr2 == kmin_bad_intr)
		bit |= IP_LEV2;
	if (pmax_intr3 == kmin_bad_intr)
		bit |= IP_LEV3;
	if (pmax_intr4 == kmin_bad_intr)
		bit |= IP_LEV4;

	/* mask it off now */
	ss->sr &= ~bit;
}


/*
 *	Object:
 *		kmin_enable_interrupt		EXPORTED function
 *
 *	Enable/Disable interrupts from a TURBOchannel slot.
 *
 *	We pretend we actually have 8 slots even if we really have
 *	only 4: TCslots 0-2 maps to slots 0-2, TCslot3 maps to
 *	slots 3-7 (see kmin_slot_hand_fill).
 */

kmin_enable_interrupt(slotno, on, modifier)
	register unsigned int	slotno;
	boolean_t		on;
{
	register unsigned	mask;

	switch (slotno) {
	case 0:	/* a real slot */
		pmax_intr2 = (on) ? kmin_tc0intr : kmin_bad_intr;
		return;
	case 1:	/* a real slot */
		pmax_intr3 = (on) ? kmin_tc1intr : kmin_bad_intr;
		return;
	case 2:	/* a real slot */
		pmax_intr4 = (on) ? kmin_tc2intr : kmin_bad_intr;
		return;
	case KMIN_SCSI_SLOT: /* system, used for SCSI */
		mask = KMIN_INTR_SCSI; break;
	case KMIN_LANCE_SLOT: /* pseudo: LANCE */
		mask = KMIN_INTR_LANCE; break;
	case KMIN_SCC0_SLOT: /* pseudo: SCC 0 */
		mask = KMIN_INTR_SCC_0; break;
	case KMIN_SCC1_SLOT: /* pseudo: SCC 1 */
		mask = KMIN_INTR_SCC_1; break;
	case KMIN_ASIC_SLOT: /* pseudo: ASIC */
		mask = KMIN_INTR_ASIC; break;
	default:
		return;/* ignore */
	}
	if (on)
		kmin_tc3_imask |= mask;
	else
		kmin_tc3_imask &= ~mask;
}

/*
 *	Object:
 *		kmin_slot_hand_fill		EXPORTED function
 *
 *	Fill in by hand the info for TC slots that are non-standard.
 *	This is the system slot on a 3min, which we think of as a
 *	set of non-regular size TC slots.
 *
 */
void kmin_slot_hand_fill(slot)
	tc_option_t	*slot;
{
	register int	i;

	for (i = KMIN_SCSI_SLOT; i < KMIN_ASIC_SLOT+1; i++) {
		slot[i].present = 1;
		slot[i].slot_size = 1;
		slot[i].rom_width = 1;
		slot[i].isa_ctlr = 0;
		slot[i].unit = 0;
		bcopy("DEC KMIN", slot[i].module_id, TC_ROM_LLEN+1);
	}

	/* scsi */
	slot[KMIN_SCSI_SLOT].isa_ctlr = 1;
	bcopy("PMAZ-AA ", slot[KMIN_SCSI_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KMIN_SCSI_SLOT].k1seg_address = PHYS_TO_K1SEG(KMIN_SYS_SCSI);

	/* lance */
	bcopy("PMAD-AA ", slot[KMIN_LANCE_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KMIN_LANCE_SLOT].k1seg_address = 0;
#if 0
	do this later, when we actually allocated the buffer:
	setse_switch(2, PHYS_TO_K1SEG(KMIN_SYS_LANCE), buffer, 0,
			PHYS_TO_K1SEG(KMIN_SYS_ETHER_ADDRESS));
#endif

	/* scc */
	bcopy("Z8530   ", slot[KMIN_SCC0_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KMIN_SCC0_SLOT].k1seg_address = PHYS_TO_K1SEG(KMIN_SYS_SCC_0);
	set_scc_address(0, PHYS_TO_K1SEG(KMIN_SYS_SCC_0), FALSE, TRUE);

	slot[KMIN_SCC1_SLOT].unit = 1;
	bcopy("Z8530   ", slot[KMIN_SCC1_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KMIN_SCC1_SLOT].k1seg_address = PHYS_TO_K1SEG(KMIN_SYS_SCC_1);
	set_scc_address(1, PHYS_TO_K1SEG(KMIN_SYS_SCC_1), FALSE, TRUE);
	
	/* asic */
	bcopy("ASIC    ", slot[KMIN_ASIC_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KMIN_ASIC_SLOT].k1seg_address = PHYS_TO_K1SEG(KMIN_SYS_ASIC);
	asic_init(BRDTYPE_DEC5000_100);
}
