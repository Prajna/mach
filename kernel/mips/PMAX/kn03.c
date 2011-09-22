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
 * $Log:	kn03.c,v $
 * Revision 2.3  93/08/03  12:33:20  mrt
 * 	Fixed clock interrupt handling.
 * 	[93/07/29  23:28:58  af]
 * 
 * Revision 2.2  93/05/30  21:08:32  rvb
 * 	RCS-ed.
 * 	[93/05/29            af]
 * 
 */
/*
 *	File: kn03.c
 * 	Author: John Wroclawski, MIT, from mips/PMAX/kn02.c
 * 		and mips/PMAX/kn02ba.c by Alessandro Forin, CMU.
 *	Date:	?/93
 *
 *	Routines specific to the KN03 processor (3max+).
 */

#include <mach/std_types.h>
#include <mips/machspl.h>
#include <mips/mips_box.h>
#include <mips/mips_cpu.h>
#include <mips/PMAX/kn03.h>
#include <mips/PMAX/kn03_cpu.h>
#include <mips/PMAX/tc.h>

/*
 * Software-maintained mask of device interrupts currently
 * enabled - this word is anded into the hardware imask
 * register whenever that register is changed by one of the
 * spl??? functions.
 */
unsigned int kn03_imask;


/*
 *	Object:
 *		kn03_errintr			EXPORTED function
 *
 *	Handle "memory errors":
 *		ECC errors, DMA errors and overrun, CPU write timeouts
 *
 */

/*
 * Compute actual erring location.
 *  If processor or DMA read from memory, pipeline has advanced 5 stages.
 *  For IO read/write or DMA write, address is as given??
 */
static unsigned int
kn03_recover_erradr(eareg_val)
	register unsigned int eareg_val;
{
        register unsigned int address;
	register int column;

	address = eareg_val & KN03_ERR_ADDRESS;

	if ((address <= KN03_PHYS_MAX) && ((eareg_val & KN03_ERR_WRITE) == 0)){
	    /* correct for controller pipeline */
	    column = address & 0xfff;
	    column -= 5;
	    return ( (address & 0xfffff000) | (column & 0xfff) );
	}
	return (address);
}

static int errintr_cnt;
static int corrected_ecc_error_count;
static int multibit_ecc_error_count;

kn03_errintr(st,spllevel)
{
	register int adr, syn;
	int actual_addr;

	syn = *(volatile int *)PHYS_TO_K1SEG(KN03_SYS_CHKSYN);
	adr = *(volatile int *)PHYS_TO_K1SEG(KN03_SYS_ERRADR);

	errintr_cnt++;

	/*
	 * The hardware has corrected single bit errors.
	 * The hardware is deeply confused about multi-bit errors.
	 * This code should detect multibit errors and kill the task
	 * (if error is in user space) or panic (if kernel). However,
	 * it doesn't.
	 */
	if (adr & KN03_ERR_VALID) {
	    int temp;

	    actual_addr = kn03_recover_erradr(adr);
	    if (adr & KN03_ERR_ECCERR) {
		boolean_t single_bit_error;

		single_bit_error = (actual_addr & 0x1) ?
		    (syn & KN03_ECC_SNGHI) : (syn & KN03_ECC_SNGLO);
		
		if (single_bit_error) {
		    /* 
		     * update failing location with corrected data.
		     * write fixed data to the cache too; is this necessary?
		     */
		    temp = *(volatile int *)PHYS_TO_K1SEG(actual_addr);
		    *(volatile int *)PHYS_TO_K0SEG(actual_addr) = temp;

		    corrected_ecc_error_count++;
		    printf("ECC corrected error, physaddr %x (count %d)\n",
			   actual_addr, corrected_ecc_error_count);
		} else {
		    multibit_ecc_error_count++;
		    printf("UNCORRECTED MEM ERROR, DATA LOST!! physaddr %x (count %d\n",
			   actual_addr, multibit_ecc_error_count);
		}
	    } else {
		printf("NXM during %s %s transaction, physaddr %x\n",
		       adr & KN03_ERR_CPU ? "CPU" : "DMA",
		       adr & KN03_ERR_WRITE ? "write" : "read",
		       actual_addr);
	    }
	} else {
	    printf("errintr with invalid data?\n");
	}

	/* Clear status reg to reenable error reporting */
	*(volatile int *)PHYS_TO_K1SEG(KN03_SYS_ERRADR) = 0;
}

/*
 *	Object:
 *		kn03_haltintr		EXPORTED function
 *
 *	Handle interrupt because user pushed the Halt button
 *
 */
unsigned kn03_haltintr_count = 0;	/* patch for production to 2 */

kn03_haltintr(st,spllevel)
{
	if (++kn03_haltintr_count == 3)
		halt_all_cpus(FALSE);
	gimmeabreak();
}

/*
 *	Object:
 *		kn03_tcintr			EXPORTED function
 *
 *	Handle interrupts from the TURBOchannel, system slot
 *
 */
extern boolean_t ignore_power_supply_overheating;

kn03_tcintr(st,spllevel)
	unsigned st;
	spl_t	 spllevel;
{
	register unsigned int	intr;
	volatile unsigned int	*imaskp;
	unsigned int		old_mask;

	intr = *(volatile int *)PHYS_TO_K1SEG(KN03_REG_INTR);
	imaskp = (volatile unsigned int *)PHYS_TO_K1SEG(KN03_REG_IMSK);

	/* masked interrupts are still observable; must ignore them */
	old_mask = *imaskp;
	intr &= old_mask;

	/* Interrupts from same slot are ok if from another device */
	spllevel |= (spllevel >> 1) & SR_INT_MASK;

	if (intr & KN03_INTR_SCC_0)
		(*(tc_slot_info[KN03_SCC0_SLOT].intr))
			(tc_slot_info[KN03_SCC0_SLOT].unit,
			 spllevel | (KN03_SR_IM3 << KN03_SR_SHIFT));

	else if (intr & KN03_INTR_SCC_1)
		(*(tc_slot_info[KN03_SCC1_SLOT].intr))
			(tc_slot_info[KN03_SCC1_SLOT].unit,
			 spllevel | (KN03_SR_IM3 << KN03_SR_SHIFT));

	else if (intr & KN03_INTR_SCSI)
		(*(tc_slot_info[KN03_SCSI_SLOT].intr))
			(tc_slot_info[KN03_SCSI_SLOT].unit,
			 spllevel | (KN03_SR_IM2 << KN03_SR_SHIFT));

	/* TC2 has highest DMA prio, give it first crack at interrupts too */
	else if (intr & KN03_INTR_TC_2)
		(*(tc_slot_info[2].intr))
			(tc_slot_info[1].unit,
			 spllevel | (KN03_SR_IM2 << KN03_SR_SHIFT));

	else if (intr & KN03_INTR_TC_1)
		(*(tc_slot_info[1].intr))
			(tc_slot_info[1].unit,
			 spllevel | (KN03_SR_IM2 << KN03_SR_SHIFT));

	else if (intr & KN03_INTR_TC_0)
		(*(tc_slot_info[0].intr))
			(tc_slot_info[0].unit,
			 spllevel | (KN03_SR_IM2 << KN03_SR_SHIFT));

	else if (intr & KN03_INTR_LANCE)
		(*(tc_slot_info[KN03_LANCE_SLOT].intr))
			(tc_slot_info[KN03_LANCE_SLOT].unit,
			 spllevel | (KN03_SR_IM1 << KN03_SR_SHIFT));

	else if (intr & KN03_INTR_ASIC)
		(*(tc_slot_info[KN03_ASIC_SLOT].intr))
			(tc_slot_info[KN03_ASIC_SLOT].unit,
			 spllevel /* UNMODIFIED! */,
			 intr);
#define	FIRE_ALARM
#ifdef	FIRE_ALARM
	else {
		static int user_warned = 0;

		if (user_warned && ((intr & KN03_INTR_PSWARN) == 0)) {
			*imaskp = 0;
			if (!ignore_power_supply_overheating)
				printf("%s\n", "Power supply ok now.");
			user_warned = 0;
		}
		if ((intr & KN03_INTR_PSWARN) && (user_warned < 3)) {
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

kn03_mc_intr(st,spllevel)
{
	spl_t	s;
	mc_intr(spllevel);
	s = splclock();
	clock_interrupt(tick, st&SR_KUo, (st&SR_INT_MASK)==INT_LEV0);
	splx(s);
}

/*
 *	Object:
 *		kn03_enable_interrupt		EXPORTED function
 *
 *	Enable/Disable interrupts from a TURBOchannel slot or
 *	internal device.
 *
 *	We pretend we actually have 8 slots even if we really have
 *	only 4: slot numbers 0-2 maps to real TC slots 0-2, others
 *	map to internal system devices.
 *
 *	This function does not actually have any effect until one of the
 *	spl??? functions is called.
 */
kn03_enable_interrupt(slotno, on, modifier)
	register unsigned int	slotno;
	boolean_t		on;
{
	register unsigned	mask;

	switch (slotno) {
	case 0:	/* a real slot */
		mask = KN03_INTR_TC_0; break;
	case 1:	/* a real slot */
		mask = KN03_INTR_TC_1; break;
	case 2:	/* a real slot */
		mask = KN03_INTR_TC_2; break;
	case KN03_SCSI_SLOT: /* system, used for SCSI */
		mask = KN03_INTR_SCSI; break;
	case KN03_LANCE_SLOT: /* pseudo: LANCE */
		mask = KN03_INTR_LANCE; break;
	case KN03_SCC0_SLOT: /* pseudo: SCC 0 */
		mask = KN03_INTR_SCC_0; break;
	case KN03_SCC1_SLOT: /* pseudo: SCC 1 */
		mask = KN03_INTR_SCC_1; break;
	case KN03_ASIC_SLOT: /* pseudo: ASIC */
		mask = KN03_INTR_ASIC & ~KN03_INTR_SCSI_FIFO; break;
	default:
		return;/* ignore */
	}
	if (on)
		kn03_imask |= mask;
	else
		kn03_imask &= ~mask;
}

/*
 *	Object:
 *		kn03_slot_hand_fill		EXPORTED function
 *
 *	Fill in by hand the info for TC slots that are non-standard.
 *	We have a bunch of these on the kn03 for internal devices.
 */
void kn03_slot_hand_fill(slot)
	tc_option_t	*slot;
{
	register int	i;

	for (i = KN03_FIRST_PSEUDOSLOT; i <= KN03_LAST_PSEUDOSLOT; i++) {
		slot[i].present = 1;
		slot[i].slot_size = 1;
		slot[i].rom_width = 1;
		slot[i].isa_ctlr = 0;
		slot[i].unit = 0;
		bcopy("DEC KN03", slot[i].module_id, TC_ROM_LLEN+1);
	}

	/* scsi */
	slot[KN03_SCSI_SLOT].isa_ctlr = 1;
	bcopy("PMAZ-AA ", slot[KN03_SCSI_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KN03_SCSI_SLOT].k1seg_address = PHYS_TO_K1SEG(KN03_SYS_SCSI);

	/* lance */
	bcopy("PMAD-AA ", slot[KN03_LANCE_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KN03_LANCE_SLOT].k1seg_address = 0;
#if 0
	do this later, when we actually allocated the buffer:
	setse_switch(2, PHYS_TO_K1SEG(KN03_SYS_LANCE), buffer, 0,
			PHYS_TO_K1SEG(KN03_SYS_ETHER_ADDRESS));
#endif

	/* scc */
	bcopy("Z8530   ", slot[KN03_SCC0_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KN03_SCC0_SLOT].k1seg_address = PHYS_TO_K1SEG(KN03_SYS_SCC_0);
	set_scc_address(0, PHYS_TO_K1SEG(KN03_SYS_SCC_0), FALSE, TRUE);

	slot[KN03_SCC1_SLOT].unit = 1;
	bcopy("Z8530   ", slot[KN03_SCC1_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KN03_SCC1_SLOT].k1seg_address = PHYS_TO_K1SEG(KN03_SYS_SCC_1);
	set_scc_address(1, PHYS_TO_K1SEG(KN03_SYS_SCC_1), FALSE, TRUE);
	
	/* asic */
	bcopy("ASIC    ", slot[KN03_ASIC_SLOT].module_name, TC_ROM_LLEN+1);
	slot[KN03_ASIC_SLOT].k1seg_address = PHYS_TO_K1SEG(KN03_SYS_ASIC);
	asic_init(BRDTYPE_DEC5000_240);
}

/*
 *	Object:
 *		kn03_steal_memory		EXPORTED function
 *
 *	Take some contiguous physical memory away from the system
 *	for our use: for the screen mapped control info and for
 *	the Lance buffer.
 *	The waste is big because: (a) the Lance buffer must be aligned
 *	on a 128k boundary and (b) we must take 128k but will
 *	only use 64k.  Double sigh.
 *	[it is actually possible to use the other 64k, ugly but possible]
 *	XXXXXXXXXX   fix me XXXXXXXXXX fix me XXXXXXXXXXXX fix me XXXXXXX
 */
vm_size_t
kn03_steal_memory(start)
	vm_offset_t	start;
{
	vm_size_t	needed;
	vm_offset_t	sebuf;

	needed = screen_memory_alloc(start);
	needed = mips_round_page(needed);

	/* make phys, round up to 128k */
	start = K0SEG_TO_PHYS(start);
	sebuf = start + needed;
	sebuf = (sebuf + 0x1ffff) & ~0x1ffff;

	needed  = sebuf - start;	/* due to rounding */
	needed += 128 * 1024;		/* 64k needed, but.. */

	setse_switch(2, PHYS_TO_K1SEG(KN03_SYS_LANCE), 
			PHYS_TO_K1SEG(sebuf), 0,
			PHYS_TO_K1SEG(KN03_SYS_ETHER_ADDR));
	asic_enable_lance(sebuf);

	return needed;
}
