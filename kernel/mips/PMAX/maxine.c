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
 * $Log:	maxine.c,v $
 * Revision 2.11  93/05/30  21:08:50  rvb
 * 	Mods for asic_init's interface change.
 * 	[93/05/29  09:55:38  af]
 * 
 * Revision 2.10  93/05/15  19:11:50  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.9  93/05/10  21:20:41  rvb
 * 	No minors or sys/types.h
 * 	[93/05/06  09:39:57  af]
 * 
 * Revision 2.8  93/03/26  17:57:44  mrt
 * 	Correct return value
 * 
 * 
 * Revision 2.7  93/02/05  08:03:57  danner
 * 	Clock got more generic.
 * 	[93/02/04  02:01:08  af]
 * 
 * 	Proper spl typing.
 * 	[92/12/01            af]
 * 
 * Revision 2.6  92/05/22  15:48:49  jfriedl
 * 	Use Ultrix' name for the builtin screen.
 * 	[92/05/13  20:55:48  af]
 * 
 * Revision 2.5  92/05/05  10:46:34  danner
 * 	Make dtop interruptible from serial lines.
 * 	[92/05/04  11:35:40  af]
 * 
 * 	Changes to make the serial line work for real.
 * 	[92/04/14  12:11:08  af]
 * 
 * 	Added mappable timer device.
 * 	[92/03/11  02:36:54  af]
 * 
 * Revision 2.4  92/04/01  15:14:49  rpd
 * 	Added mappable timer device.
 * 	[92/03/11  02:36:54  af]
 * 
 * Revision 2.3  92/03/05  11:37:11  rpd
 * 	Clear vertical retrace interrupt bit here.
 * 	[92/03/04            af]
 * 
 * Revision 2.2  92/03/02  18:34:24  rpd
 * 	Created, from the DEC specs:
 * 	"MAXine System Module Functional Specification"  Revision 1.2
 * 	Workstation Systems Engineering, Palo Alto, CA. July 15, 1991.
 * 	"IO Controller ASIC Functional Specifications"
 * 	Workstation Systems Engineering, Palo Alto, CA. Feb 1, 1991.
 * 	[92/01/17            af]
 * 
 */
/*
 *	File: maxine.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	1/92
 *
 *	Routines specific to the MAXine system module (54-21325-01)
 */

#include <mach/std_types.h>
#include <mach/vm_param.h>
#include <machine/machspl.h>
#include <chips/serial_defs.h>

#include <mips/thread.h>
#include <mips/mips_cpu.h>
#include <mips/mips_box.h>
#include <mips/prom_interface.h>
#include <mips/PMAX/maxine.h>
#include <mips/PMAX/maxine_cpu.h>
#include <mips/PMAX/tc.h>

#include <chips/busses.h>
#include <chips/serial_defs.h>

#define	XINE_INTR		PHYS_TO_K1SEG(XINE_REG_INTR)
#define	XINE_IMSK		PHYS_TO_K1SEG(XINE_REG_IMSK)
#define	XINE_FCTR		PHYS_TO_K1SEG(XINE_REG_FCTR)

/*
 *	Object:
 *		xine_console_probe		LOCAL function
 *		xine_console_pollc		LOCAL function
 *
 *	This is only really needed for rconsole functionality.
 *	Since the serial line and keyboard are on totally
 *	different wavelengths we have to set up some crocks
 *	so that both are up and about asap (for debugging
 *	purposes).
 *
 */
#define	SCC_UNIT	1
#include <device/tty.h>

static
xine_console_probe( xxx, ui)
	struct bus_device	*ui;
{
	dtop_probe( 0xbc2a0000, ui);
	if (rcline) {
		ui->unit = SCC_UNIT;
		scc_probe( xxx, ui);
	}
	return 1;

}

static
xine_console_pollc(unit, on)
{
	dtop_pollc(unit, on);
	if (rcline)
		scc_pollc(0/*aurgh*/, on);
}

static
xine_console_softCAR(unit, line, on)
{
	if ((line == 0) || (line == 1))
		dtop_softCAR(unit, line, on);
	else
		scc_softCAR(0/*aurgh*/, line, on);
}

static
xine_console_param(tp, line)
	unsigned line;
{
	if (line < 2)
		dtop_param(tp, line);
	else
		scc_param(tp, line);
}

static boolean_t
xine_console_start(tp)
	struct tty	*tp;
{
	extern struct tty *console_tty[];

	if ((tp - console_tty[0]) < 2)
		return dtop_start(tp);
	else
		return scc_start(tp);

}

xine_console_mctl(dev, bits, how)
{
	if (dev < 2)
		return dtop_mctl(dev, bits, how);
	else
		return scc_mctl(dev, bits, how);
}

/*
 *	Object:
 *		xine_tc3_imask			EXPORTED unsigned int
 *
 *	Pretend some devices were not present.  The spl system
 *	for the maxine might logically enable a device, but esp
 *	during autoconf we might want to ignore it.
 *	See xine_splx/xine_spl0 for where and how this is used.
 *
 */
unsigned xine_tc3_imask = 0;

/*
 *	Object:
 *		xine_mc_intr			EXPORTED function
 *
 *	Handle interrupts from the clock RTC chip
 *
 */

xine_mc_intr(st,spllevel)
	unsigned st;
	spl_t spllevel;
{
	unsigned int		save_imask = *(unsigned int *) XINE_IMSK;
	register unsigned int	new_fctr;
	register int		fctr;
	static unsigned int	previous_fctr;

	new_fctr = *(unsigned int *) XINE_FCTR;
	fctr = new_fctr - previous_fctr;
	if (fctr < 0)
		fctr = -fctr;
	previous_fctr = new_fctr;

	spllevel = (XINE_SR_IM4 << XINE_SR_SHIFT) | XINE_INT_LEV7 | SR_IEc;
	mc_intr(spllevel);
	splx(spllevel);

	clock_interrupt( fctr, st&SR_KUo, (st&SR_INT_MASK)==INT_LEV0);

	*(unsigned int *) XINE_IMSK = save_imask;
}

/*
 *	Object:
 *		xine_tc3intr			EXPORTED function
 *
 *	Handle interrupts from the TURBOchannel, system slot
 *
 */
xine_tc3intr(st,spllevel)
	unsigned st;
	spl_t spllevel;
{
	register unsigned int  intr   = *(volatile int *) XINE_INTR;
	volatile unsigned int *imaskp =  (volatile unsigned int *) XINE_IMSK;
	unsigned int	       old_mask;

	/* masked interrupts are still observable */
	old_mask = *imaskp & xine_tc3_imask;
	intr &= old_mask;

	/* Interrupts from same slot are ok if from another device */
	spllevel |= (spllevel >> 1) & SR_INT_MASK;

	if (intr & XINE_INTR_ASIC)
		(*(tc_slot_info[XINE_ASIC_SLOT].intr))
			(tc_slot_info[XINE_ASIC_SLOT].unit,
			 spllevel /* UNMODIFIED! */ ,
			 intr);

	else if (intr & XINE_INTR_FLOPPY)
		(*(tc_slot_info[XINE_FLOPPY_SLOT].intr))
			(tc_slot_info[XINE_FLOPPY_SLOT].unit,
			 spllevel | (XINE_SR_IM2 << XINE_SR_SHIFT));

	else if (intr & XINE_INTR_TC_0)
		(*(tc_slot_info[0].intr))
			(tc_slot_info[0].unit,
			 spllevel | (XINE_SR_IM2 << XINE_SR_SHIFT));

	else if (intr & XINE_INTR_ISDN)
		(*(tc_slot_info[XINE_ISDN_SLOT].intr))
			(tc_slot_info[XINE_ISDN_SLOT].unit,
			 spllevel | (XINE_SR_IM3 << XINE_SR_SHIFT));

	else if (intr & XINE_INTR_SCSI)
		(*(tc_slot_info[XINE_SCSI_SLOT].intr))
			(tc_slot_info[XINE_SCSI_SLOT].unit,
			 spllevel | (XINE_SR_IM2 << XINE_SR_SHIFT));

	else if (intr & XINE_INTR_LANCE)
		(*(tc_slot_info[XINE_LANCE_SLOT].intr))
			(tc_slot_info[XINE_LANCE_SLOT].unit,
			 spllevel | (XINE_SR_IM1 << XINE_SR_SHIFT));

	else if (intr & XINE_INTR_SCC_0)
		(*(tc_slot_info[XINE_SCC0_SLOT].intr))
			(tc_slot_info[XINE_SCC0_SLOT].unit,
			 spllevel | (XINE_SR_IM3 << XINE_SR_SHIFT));

	else if (intr & XINE_INTR_TC_1)
		(*(tc_slot_info[1].intr))
			(tc_slot_info[1].unit,
			 spllevel | (XINE_SR_IM2 << XINE_SR_SHIFT));

	else if (intr & XINE_INTR_VINT) {
		*(volatile int *) XINE_INTR = ~XINE_INTR_VINT;
		(*(tc_slot_info[XINE_CFB_SLOT].intr))
			(tc_slot_info[XINE_CFB_SLOT].unit,
			 spllevel | (XINE_SR_IM3<<XINE_SR_SHIFT));
	}
	else if (intr & (XINE_INTR_DTOP))
		(*(tc_slot_info[XINE_DTOP_SLOT].intr))
			(tc_slot_info[XINE_DTOP_SLOT].unit,
			 spllevel | (XINE_SR_IM2a<<XINE_SR_SHIFT),
			 intr & XINE_INTR_DTOP_RX);

	(void) simple_splhigh();
	*imaskp = old_mask;
}

/*
 *	Object:
 *		xine_enable_interrupt		EXPORTED function
 *
 *	Enable/Disable interrupts from a TURBOchannel slot.
 *
 *	We pretend we actually have 11 slots even if we really have
 *	only 3: TCslots 0-1 maps to slots 0-1, TCslot 2 is used for
 *	the system (TCslot3), TCslot3 maps to slots 3-10
 *	(see xine_slot_hand_fill).
 *	Note that all these interrupts come in via the IMR.
 */

xine_enable_interrupt(slotno, on, modifier)
	register unsigned int	slotno;
	boolean_t		on;
{
	register unsigned	mask;

	switch (slotno) {

	case 0:			/* a real slot, but  */
		mask = XINE_INTR_TC_0;
		break;

	case 1:			/* a real slot, but */
		mask = XINE_INTR_TC_1;
		break;

	case XINE_FLOPPY_SLOT:

		mask = XINE_INTR_FLOPPY;
		break;

	case XINE_SCSI_SLOT:

		mask = XINE_INTR_SCSI;
		break;

	case XINE_LANCE_SLOT:

		mask = XINE_INTR_LANCE;
		break;

	case XINE_SCC0_SLOT:

		mask = XINE_INTR_SCC_0;
		break;

	case XINE_DTOP_SLOT:

		/* modifier sez if we only want tx off */
		mask = (modifier) ? XINE_INTR_DTOP_TX : XINE_INTR_DTOP;
		break;

	case XINE_ISDN_SLOT:

		mask = XINE_INTR_ISDN;
		break;

	case XINE_ASIC_SLOT:

		mask = XINE_INTR_ASIC;
		break;

	case XINE_CFB_SLOT:

		mask = XINE_INTR_VINT;
		break;

	default:
		return;/* ignore */
	}

	if (on)
		xine_tc3_imask |= mask;
	else
		xine_tc3_imask &= ~mask;
}

/*
 *	Object:
 *		xine_slot_hand_fill		EXPORTED function
 *
 *	Fill in by hand the info for TC slots that are non-standard.
 *	This is the system slot on a 3min, which we think of as a
 *	set of non-regular size TC slots.
 *
 */
void xine_slot_hand_fill(slot)
	tc_option_t	*slot;
{
	register int	i;

	for (i = XINE_FLOPPY_SLOT; i < XINE_FRC_SLOT+1; i++) {
		slot[i].present = 1;
		slot[i].slot_size = 1;
		slot[i].rom_width = 1;
		slot[i].isa_ctlr = 0;
		slot[i].unit = 0;
		bcopy("DEC XINE", slot[i].module_id, TC_ROM_LLEN+1);
	}

	/* floppy */
	slot[XINE_FLOPPY_SLOT].isa_ctlr = 1;
	bcopy("XINE-FDC", slot[XINE_FLOPPY_SLOT].module_name, TC_ROM_LLEN+1);
	slot[XINE_FLOPPY_SLOT].k1seg_address = PHYS_TO_K1SEG(XINE_SYS_FLOPPY);

	/* scsi */
	slot[XINE_SCSI_SLOT].isa_ctlr = 1;
	bcopy("PMAZ-AA ", slot[XINE_SCSI_SLOT].module_name, TC_ROM_LLEN+1);
	slot[XINE_SCSI_SLOT].k1seg_address = PHYS_TO_K1SEG(XINE_SYS_SCSI);

	/* lance */
	bcopy("PMAD-AA ", slot[XINE_LANCE_SLOT].module_name, TC_ROM_LLEN+1);
	slot[XINE_LANCE_SLOT].k1seg_address = 0;
#if 0
	do this later, when we actually allocated the buffer:
	setse_switch(2, PHYS_TO_K1SEG(XINE_SYS_LANCE), buffer, 0,
			PHYS_TO_K1SEG(XINE_SYS_ETHER_ADDRESS));
#endif

	/* scc */
	bcopy("Z8530   ", slot[XINE_SCC0_SLOT].module_name, TC_ROM_LLEN+1);
	slot[XINE_SCC0_SLOT].k1seg_address = PHYS_TO_K1SEG(XINE_SYS_SCC_0);
	set_scc_address(SCC_UNIT, PHYS_TO_K1SEG(XINE_SYS_SCC_0), FALSE, FALSE);

	/* Desktop */
	bcopy("DTOP    ", slot[XINE_DTOP_SLOT].module_name, TC_ROM_LLEN+1);
	slot[XINE_DTOP_SLOT].k1seg_address =
		PHYS_TO_K1SEG(XINE_SYS_DTOP+0x20000); /* why? */
	set_dtop_address( 0, PHYS_TO_K1SEG(XINE_REG_INTR));

	console_probe = xine_console_probe;
	console_pollc = xine_console_pollc;
	console_softCAR = xine_console_softCAR;
	console_param = xine_console_param;
	console_start = xine_console_start;
	console_mctl = xine_console_mctl;

	/* ISDN */
	bcopy("AMD79c30", slot[XINE_ISDN_SLOT].module_name, TC_ROM_LLEN+1);
	slot[XINE_ISDN_SLOT].k1seg_address = PHYS_TO_K1SEG(XINE_SYS_ISDN);

	/* Video */
	bcopy("PMAG-DV ", slot[XINE_CFB_SLOT].module_name, TC_ROM_LLEN+1);
	slot[XINE_CFB_SLOT].k1seg_address = PHYS_TO_K0SEG(XINE_PHYS_CFB_START);

	/* asic */
	bcopy("ASIC    ", slot[XINE_ASIC_SLOT].module_name, TC_ROM_LLEN+1);
	slot[XINE_ASIC_SLOT].k1seg_address = PHYS_TO_K1SEG(XINE_SYS_ASIC);
	asic_init(BRDTYPE_DEC5000_20);

	/* free-running counter (high resolution mapped time) */
	bcopy("XINE-FRC", slot[XINE_FRC_SLOT].module_name, TC_ROM_LLEN+1);
	slot[XINE_FRC_SLOT].k1seg_address = PHYS_TO_K1SEG(XINE_REG_FCTR);

}


/* debugging tools */
#if 0
imask_status()
{
	db_printf("tc3mask x%x imask x%x intr x%x\n",
		xine_tc3_imask, *(int*)0xbc040120, *(int*)0xbc040110);
}
#endif
