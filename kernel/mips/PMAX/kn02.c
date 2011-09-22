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
 * $Log:	kn02.c,v $
 * Revision 2.9  92/03/02  18:34:15  rpd
 * 	The interface of the interrupt enable function has changed.
 * 	[92/03/02  02:21:38  af]
 * 
 * Revision 2.8  92/02/23  19:39:18  elf
 * 	Updated copyright, added include that af forgot.
 * 	[92/02/23            danner]
 * 
 * Revision 2.7  91/05/14  17:23:21  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/02/14  14:34:27  mrt
 * 	Pass along the spllevel to interrupt routines.
 * 	[91/02/12  12:40:09  af]
 * 
 * Revision 2.5  91/02/05  17:41:58  mrt
 * 	Added author notices
 * 	[91/02/04  11:14:18  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:12:51  mrt]
 * 
 * Revision 2.4  90/12/05  23:31:52  af
 * 	Cleaned up.
 * 	[90/12/03  23:25:08  af]
 * 
 * Revision 2.2.1.2  90/11/01  02:45:03  af
 * 	Created, from the DEC specs:
 * 	"DECstation 5000/200 KN02 System Module Functional Specification"
 * 	Workstation Systems Engineering, Palo Alto, CA. Aug 27, 1990.
 * 	[90/09/03            af]
 * 
 */
/*
 *	File: kn02.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	Routines specific to the KN02 processor (3max)
 */

#include <mach/std_types.h>
#include <mips/mips_cpu.h>
#include <mips/PMAX/kn02.h>
#include <mips/PMAX/tc.h>

#define	KN02CSR_ADDR		PHYS_TO_K1SEG(KN02_SYS_CSR)
#define	KN02ERR_ADDR		PHYS_TO_K1SEG(KN02_SYS_ERRADR)
#define	KN02CHKSYN_ADDR		PHYS_TO_K1SEG(KN02_SYS_CHKSYN)


/*
 *	Object:
 *		kn02_errintr			EXPORTED function
 *
 *	Handle "memory errors":
 *		ECC errors, DMA errors and overrun, CPU write timeouts
 *
 */
static unsigned
kn02_recover_erradr_on_read( curval)
	register unsigned	curval;
{
	register int column = curval & 0xfff;
	/* pipeline has advanced five stages */
	column -= 5;
	return (curval & 0x7fff000) | (column & 0xfff);
}

int errintr_cnt;

kn02_errintr(st,spllevel)
{
	register int csr, adr, syn;

	syn = *(volatile int *)KN02CHKSYN_ADDR;
	csr = *(volatile int *)KN02CSR_ADDR;
	adr = *(volatile int *)KN02ERR_ADDR;

	/* scrub error */
	*((volatile int *)KN02ERR_ADDR) = 0;

	errintr_cnt++;
	printf("(%d)%s%x [%x %x %x]\n", errintr_cnt,
	       "Bad memory chip at phys ",
	       kn02_recover_erradr_on_read(adr),
	       csr, syn, adr);
}

/*
 *	Object:
 *		kn02_tcintr			EXPORTED function
 *
 *	Handle interrupts from the TURBOchannel.
 *
 */

kn02_tcintr(st,spllevel)
	unsigned st;
{
	register int	csr;

	/*
	 * Get CSR
	 */
	csr = *(volatile int *)KN02CSR_ADDR;
#ifdef	FIRE_ALARM
	{
		static int user_warned = 0;

		if (user_warned && ((csr & KN02_CSR_S_PSWARN) == 0)) {
			printf("%s\n", "Power supply ok now.");
			user_warned = 0;
		}
		if ((csr & KN02_CSR_S_PSWARN) && (user_warned < 3)) {
			user_warned++;
			printf("%s\n", "Power supply overheating");
		}
	}
#endif
	/*
	 * Mask off disabled interrupts
	 */
	csr = csr & (csr >> KN02_CSR_IOINTEN_SHIFT) & KN02_CSR_IOINT;

	/*
	 * Prioritize by slot number: higher slot number first.
	 */
	if (csr & 0x80)
		(*(tc_slot_info[7].intr))(tc_slot_info[7].unit,spllevel);
	if (csr & 0x40)
		(*(tc_slot_info[6].intr))(tc_slot_info[6].unit,spllevel);
	if (csr & 0x20)
		(*(tc_slot_info[5].intr))(tc_slot_info[5].unit,spllevel);
#if 0	/* reserved slots on 3max */
	if (csr & 0x10)
		(*(tc_slot_info[4].intr))(tc_slot_info[4].unit,spllevel);
	if (csr & 0x08)
		(*(tc_slot_info[3].intr))(tc_slot_info[3].unit,spllevel);
#endif
	if (csr & 0x04)
		(*(tc_slot_info[2].intr))(tc_slot_info[2].unit,spllevel);
	if (csr & 0x02)
		(*(tc_slot_info[1].intr))(tc_slot_info[1].unit,spllevel);
	if (csr & 0x01)
		(*(tc_slot_info[0].intr))(tc_slot_info[0].unit,spllevel);
}


/*
 *	Object:
 *		kn02_enable_interrupt		EXPORTED function
 *
 *	Enable/Disable interrupts from a TURBOchannel slot.
 *
 */

kn02_enable_interrupt(slotno, on, modifier)
	register unsigned int	slotno;
	register boolean_t	on;
{
	register volatile int	*p_csr = (volatile int *)KN02CSR_ADDR;

	if (slotno >= KN02_TC_NSLOTS)
		return;

	slotno = 1 << (slotno + KN02_CSR_IOINTEN_SHIFT);
	if (on)
		*p_csr |= slotno;
	else
		*p_csr &= ~slotno;
}

/*
 *	Object:
 *		kn02_slot_hand_fill		EXPORTED function
 *
 *	Fill in by hand the info for TC slots that are non-standard.
 *	This is basically just the system slot on a 3max, it does not
 *	look to me like it follows the TC rules although some of the
 *	required info is indeed there.
 *
 */

void kn02_slot_hand_fill(slot)
	tc_option_t	*slot;
{
	slot[7].present = 1;
	slot[7].slot_size = 1;
	slot[7].rom_width = 1;
	slot[7].isa_ctlr = 0;
#if unsafe
	bcopy(0xbffc0410, slot[7].module_name, TC_ROM_LLEN+1);
#endif
	bcopy("KN02    ", slot[7].module_name, TC_ROM_LLEN+1);
	bcopy("DEC xxxx", slot[7].module_id, TC_ROM_LLEN+1);
	slot[7].k1seg_address = PHYS_TO_K1SEG(KN02_SYS_DZ);
}


