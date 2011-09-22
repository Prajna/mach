/* 
 * Mach Operating System
 * Copyright (c) 1993-1989 Carnegie Mellon University
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
 * $Log:	flamingo.c,v $
 * Revision 2.8  93/08/31  15:15:38  mrt
 * 	Made kn15aa_tc7intr() drop in the debugger if it cannot 
 * 	handle an interrupt.  Else we would loop.
 * 	[93/08/31            af]
 * 
 * Revision 2.7  93/08/10  15:45:46  mrt
 * 	Negated mask value used in setting kn15aa_tc7_imask in
 * 	kn15aa_enable_interrupt.
 * 	[93/08/10            mrt]
 * 
 * Revision 2.6  93/05/15  19:10:02  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.5  93/05/10  20:06:57  rvb
 * 	Support for 3000/400. Lint.
 * 	[93/05/07  14:43:16  af]
 * 
 * Revision 2.4  93/03/26  17:55:15  mrt
 * 	Dispatch ISDN interrupts properly.
 * 	[93/03/22            af]
 * 
 * Revision 2.3  93/03/11  13:49:35  danner
 * 	correct pmap_steal_memory args.
 * 	[93/03/11            danner]
 * 
 * Revision 2.2  93/03/09  10:48:14  danner
 * 	Disable some debug code.
 * 	[93/03/06            af]
 * 	Created.
 * 	[93/02/18            jeffreyh]
 * 
 */
/*
 *	File: kn15aa.c
 * 	Author: Jeffrey Heller, Kubota Pacific
 *	Adapted from kmin.c code by Alessandro Forin, Carnegie Mellon University
 *	Date:	12/92
 *
 *	Routines specific to the KN15AA
 */

#include <mach/std_types.h>
#include <sys/types.h>
#include <mach/vm_param.h>
#include <machine/machspl.h>
#include <chips/serial_defs.h>

#include <alpha/thread.h>
#include <alpha/alpha_cpu.h>
/*#include <alpha/alpha_box.h> */
/*#include <alpha/prom_interface.h> */
#include <alpha/DEC/flamingo.h>
/*#include <alpha/DEC/flamingo_cpu.h> */
#include <alpha/DEC/tc.h>

#define	KN15AA_INTR		PHYS_TO_K0SEG(KN15AA_REG_INTR)
#define	KN15AA_IMSK		PHYS_TO_K0SEG(KN15AA_REG_IMSK)


/*
 *	Object:
 *		kn15aa_tc3_imask			EXPORTED unsigned int
 *  XXX The below is garbage from mips. Not true to flamingo, but...
 *
 *	Pretend some devices were not present.  The spl system
 *	for the kn15aa might logically enable a device, but esp
 *	during autoconf we might want to ignore it.
 *	See kn15aa_splx/kn15aa_spl0 for where and how this is used.
 *
 */
unsigned kn15aa_tc7_imask = 0;

/*
 *	Object:
 *		kn15aa_iointr			EXPORTED function
 *
 *	Handle interupts from the all TC io interupts 
 */

kn15aa_iointr ()
{
	register unsigned int ir;

	wbflush();
	ir = (*(unsigned int *)PHYS_TO_K0SEG(KN15AA_REG_IR));
	/* XXX Need to mask ir with the currently enabled interupts */

	/*
	 * Prioritize by slot number: higher slot number first.
	 */
	if (ir & 0x100)			/* CXTurbo */
		(*(tc_slot_info[KN15AA_CFB_SLOT].intr))
			(tc_slot_info[KN15AA_CFB_SLOT].unit);
	else if (ir & 0x80)			/* System IOCTL ASIC */
		kn15aa_tc7intr(); 

	else if (ir & 0x40) {
		tcds_intr();
	} else if (ir & 0x20)
		(*(tc_slot_info[5].intr))
			(tc_slot_info[5].unit);
	else if (ir & 0x10)
		(*(tc_slot_info[4].intr))
			(tc_slot_info[4].unit);
	else if (ir & 0x8)
		(*(tc_slot_info[3].intr))
			(tc_slot_info[3].unit);
	else if (ir & 0x4)
		(*(tc_slot_info[2].intr))
			(tc_slot_info[2].unit);
	else if (ir & 0x2)
		(*(tc_slot_info[1].intr))
			(tc_slot_info[1].unit);
	else if (ir & 0x1)
		(*(tc_slot_info[0].intr))
			(tc_slot_info[0].unit);
}
/*
 *	Object:
 *		kn15aa_tc7intr			EXPORTED function
 *
 *	Handle interrupts from the TURBOchannel, system slot
 *
 */
extern boolean_t ignore_power_supply_overheating;

kn15aa_tc7intr()
{
	register unsigned int sir;

	wbflush();
	sir = (*(unsigned int *)PHYS_TO_K0SEG(KN15AA_REG_SIR));
	wbflush();
		
	sir &= (*(unsigned int *)PHYS_TO_K0SEG(KN15AA_REG_SIMR));
	
	if (sir & ASIC_INTR_SCC_0)
		(*(tc_slot_info[KN15AA_SCC_0_SLOT].intr))
			(tc_slot_info[KN15AA_SCC_0_SLOT].unit);

	else if (sir & ASIC_INTR_SCC_1)
		(*(tc_slot_info[KN15AA_SCC_1_SLOT].intr))
			(tc_slot_info[KN15AA_SCC_1_SLOT].unit);

	else if (sir & ASIC_INTR_LANCE)
		(*(tc_slot_info[KN15AA_LANCE_SLOT].intr))
			(tc_slot_info[KN15AA_LANCE_SLOT].unit);
	else if (sir & ASIC_INTR_ISDN)
		(*(tc_slot_info[KN15AA_ISDN_SLOT].intr))
			(tc_slot_info[KN15AA_ISDN_SLOT].unit);
	else
		gimmeabreak();
}


/*
 *	Object:
 *		kn15aa_slot_hand_fill		EXPORTED function
 *
 *	Fill in by hand the info for TC slots that are non-standard.
 *	This is the system slot on a 3min, which we think of as a
 *	set of non-regular size TC slots.
 *
 */
void kn15aa_slot_hand_fill(slot)
	tc_option_t	*slot;
{
	register int	i;

	for (i = KN15AA_SCC_0_SLOT; i < KN15AA_LANCE_SLOT+1; i++) {
		slot[i].present = 1;
		slot[i].slot_size = 1;
		slot[i].rom_width = 1;
		slot[i].isa_ctlr = 0;
		slot[i].unit = 0;
		bcopy("DEC KN15AA", slot[i].module_id, TC_ROM_LLEN+1);
	}

	tcds_init();

	/* scsi */
	slot[KN15AA_SCSI_SLOT].isa_ctlr = 1;
	bcopy("PMAZ-DS ", slot[KN15AA_SCSI_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KN15AA_SCSI_SLOT].k1seg_address = 0xfffffc01d0100000;
					/* KN15AA_REG_SCSI_94REG0 */

	slot[KN15AA_SCSI1_SLOT].unit = 1;
	slot[KN15AA_SCSI1_SLOT].isa_ctlr = 1;
	bcopy("PMAZ-DS ", slot[KN15AA_SCSI1_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KN15AA_SCSI1_SLOT].k1seg_address = 0xfffffc01d0100200;
					/* KN15AA_REG_SCSI_94REG1 */

	/* lance */
	bcopy("PMAD-BA ", slot[KN15AA_LANCE_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KN15AA_LANCE_SLOT].k1seg_address = 0;

	{
		vm_offset_t	sebuf, where;

		where = pmap_steal_memory( 0x20000 /* 128k */  );
		sebuf = (where + 0x1ffff) & ~0x1ffff;
		pmap_steal_memory(sebuf - where);

		setse_switch(0, PHYS_TO_K0SEG(KN15AA_SYS_LANCE), sebuf, 0,
			PHYS_TO_K0SEG(KN15AA_SYS_ETHER_ADDRESS));
		asic_enable_lance( sebuf );
	}


	/* scc */
	bcopy("Z8530   ", slot[KN15AA_SCC_0_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KN15AA_SCC_0_SLOT].k1seg_address = PHYS_TO_K0SEG(KN15AA_SYS_SCC_0);
	set_scc_address(0, PHYS_TO_K0SEG(KN15AA_SYS_SCC_0), FALSE, TRUE);

	slot[KN15AA_SCC_1_SLOT].unit = 1;
	bcopy("Z8530   ", slot[KN15AA_SCC_1_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KN15AA_SCC_1_SLOT].k1seg_address = PHYS_TO_K0SEG(KN15AA_SYS_SCC_1);
	set_scc_address(1, PHYS_TO_K0SEG(KN15AA_SYS_SCC_1), FALSE, TRUE);

	/* ISDN */
	bcopy("AMD79c30", slot[KN15AA_ISDN_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KN15AA_ISDN_SLOT].k1seg_address = PHYS_TO_K0SEG(KN15AA_SYS_ISDN);

	/* CFB, only on model 500 */
	if (check_memory(PHYS_TO_K0SEG(KN15AA_PHYS_TC_CXT_START_D), 0) == 0) {
		bcopy("PMAGB-BA", slot[KN15AA_CFB_SLOT].module_name, TC_ROM_LLEN+1);
		slot[KN15AA_CFB_SLOT].k1seg_address = PHYS_TO_K0SEG(KN15AA_PHYS_TC_CXT_START_D);
	} else
		slot[KN15AA_CFB_SLOT].present = 0;

	/* IO asic */
	bcopy("ASIC    ", slot[KN15AA_REG_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KN15AA_REG_SLOT].k1seg_address = PHYS_TO_K0SEG(KN15AA_SYS_ASIC_REGS /* ?? */);

	/* Clock chip */
	bcopy("TOY_RTC ", slot[KN15AA_TOY_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KN15AA_TOY_SLOT].k1seg_address = PHYS_TO_K0SEG(KN15AA_SYS_TOY);

}



/*
 *	Object:
 *		kn15aa_enable_interrupt		EXPORTED function
 *
 *	Enable/Disable interrupts from a TURBOchannel slot.
 *
 */

kn15aa_enable_interrupt(slotno, on, shift)
	int	slotno;
	boolean_t	on;
	int		shift; /*
				* shift will be needed to support the 
				* sandpiper. It is 0 for flamingo 3 for sandpiper
				*/
{
	unsigned int	mask = 0;
							      
	if (slotno < KN15AA_TC_MAX){
		volatile u_int *p_imr =
			(volatile u_int *)PHYS_TO_K0SEG(KN15AA_REG_IMR_WR);

/* 
 * If it is an option slot then set the imr
 * otherwise set the simr for the option slots. 
 * XXX might need to do something else for SCSI and CXturbo
 */

		slotno = 1 << (slotno + shift);
		if (on) {
			*p_imr |= slotno;
			wbflush();
		}
		else {
			*p_imr &= ~slotno;
			wbflush();
		}
	} else {
		volatile u_int *p_simr = 
			(unsigned int *)PHYS_TO_K0SEG(KN15AA_REG_SIMR);
		switch (slotno) {
		    case KN15AA_SCC_0_SLOT:
			mask = ASIC_INTR_SCC_0; break;
		    case KN15AA_SCC_1_SLOT:
			mask = ASIC_INTR_SCC_1; break;
		    case KN15AA_LANCE_SLOT: 
			mask = ASIC_INTR_LANCE; break;
		    case KN15AA_ISDN_SLOT: 
			mask = ASIC_INTR_ISDN; break;
		    case KN15AA_SCSI_SLOT:
		    case KN15AA_SCSI1_SLOT:
		    case KN15AA_REG_SLOT:
		    case KN15AA_TOY_SLOT:
		    case KN15AA_CFB_SLOT:		
		    default:
/*			printf("kn15aa_enable_interrup: No action taken for slot %d \n",slotno); */
		break;
		}
		if (mask) {
			if (on) {
				kn15aa_tc7_imask |= mask;
				*p_simr = kn15aa_tc7_imask;
				wbflush();
			} else {
				kn15aa_tc7_imask &= ~mask;
				*p_simr = kn15aa_tc7_imask;
				wbflush();
			}
		}
	}
}

boolean_t kn15aa_expect_machine_check = FALSE;
kn15aa_machine_check()
{
	/* 
	 * If this was due to testing a memory location then 
	 * set the flag and do nothing.
	 */
	if (kn15aa_expect_machine_check) {
		mtpr_mces(MCES_MCK|MCES_DPC|MCES_DSC);
		kn15aa_expect_machine_check = FALSE;
		return;
	}
	panic("kn15aa_machine_check: Either hardware problem or write more code \n");
}

/*
 * check_memory is inteded to see if one longword is valid.
 * It is only inteded for use by the turbochannel probe code.
 * use for other things may require more smarts.
 * It returns true if there is no readable memory at the location
 *
 * It requires support from the trap code to know is the address is valid.
 * See trap.c and kn15aa_machine_check() for other details
 */

boolean_t
check_memory(volatile int *addr)
{
	int test;
	boolean_t memok;

	kn15aa_expect_machine_check = TRUE;
	wbflush();
	test = *addr;
	wbflush();
	memok = kn15aa_expect_machine_check;
	kn15aa_expect_machine_check = FALSE;
/*dprintf("tested memory @ %lx .. %s\n",addr, memok ? "OK" : "nope");*/
	wbflush();
	return (!memok);
}

/*
 *	Object:
 *		kn15aa_steal_memory		EXPORTED function
 *
 *	Take some contiguous physical memory away from the system
 *	for our use: for the screen mapped control info and for
 *	the Lance buffer.
 *	The waste is big because: (a) the Lance buffer must be aligned
 *	on a 128k boundary and (b) we must take 128k but will
 *	only use 64k.  Double sigh.
 *	[it is actually possible to use the other 64k, ugly but possible]
 *	XXXXXXXXXX   fix me XXXXXXXXXX fix me XXXXXXXXXXXX fix me XXXXXXX
 *
 */
vm_size_t
kn15aa_steal_memory(start)
	vm_offset_t	start;
{
	vm_size_t	needed;
	vm_offset_t	sebuf;


	needed = screen_memory_alloc(start);
	needed = alpha_round_page(needed);

	/* make phys, round up to 128k */
	start = K0SEG_TO_PHYS(start);
	sebuf = start + needed;
	sebuf = (sebuf + 0x1ffff) & ~0x1ffff;

	needed  = sebuf - start;	/* due to rounding */
	needed += 128 * 1024;		/* 64k needed, but.. */

	setse_switch(2, PHYS_TO_K0SEG(KN15AA_SYS_LANCE), 
			PHYS_TO_K0SEG(sebuf), 0,
			PHYS_TO_K0SEG(KN15AA_SYS_ETHER_ADDRESS));
	asic_enable_lance(sebuf);

	return needed;
}



/* 
 * This routine is to set the PBS bits in the ioslot register for a
 * slot. If you do not want to lose information you need to read the bits
 * and use the old bits along with any new bits when you write set the new bits
 */

kn15aa_set_ioslot(slot, flags, set)
	unsigned int slot, flags;
	boolean_t	set;
{
	volatile unsigned int *ioslot;
	register unsigned int ioslotv, newioslotv, mask;
	spl_t s;

	assert( slot <= 8);
	ioslot = (volatile unsigned int *)PHYS_TO_K0SEG(KN15AA_REG_IOSLOT);
	slot *=3;		/* PB&S bits for each slot */
	mask = 0x7 << slot;
	flags &= 0x7;	/* Do not overwrite other slots bits, I did */
	flags <<= slot; /* Shift the new flags into place */

	s = splhigh();
	ioslotv = *ioslot;
	if (set) {
		ioslotv &= ~mask;
		ioslotv |= flags;
	} else
		ioslotv &= ~flags;
	*ioslot = ioslotv;
	wbflush();
	splx(s);
}	

/*
 * Used to read the value of the PB&S bits for a slot
 */
unsigned int
kn15aa_read_ioslot(slot)
	unsigned int slot;
{
	volatile unsigned int *ioslot, flags;

	assert( slot <= 8);
	ioslot = (volatile unsigned int *)PHYS_TO_K0SEG(KN15AA_REG_IOSLOT);
	flags = (*ioslot >> (slot * 3)) & 0x7;
	return (flags);
}

#if 0
/* hacking merrily away */

flamingo_set_leds(unsigned char val)
{
	*((int*)0xfffffc01f0080200) = 0x15d00 | val;
	delay(1000000);
}

#endif
