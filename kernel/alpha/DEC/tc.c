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
 * $Log:	tc.c,v $
 * Revision 2.3  93/05/10  20:07:08  rvb
 * 	ANSI-C. Lint.
 * 	[93/05/07  14:45:57  af]
 * 
 * 	CPU driver changed name, removed unnecessary buserror scrubbing,
 * 	made silent by default, start off with all TC interrupts off.
 * 	[90/11/01  03:49:16  af]
 * 
 * 	Created, from the DEC specs:
 * 	"TURBOchannel Hardware Specification"
 * 	EK-369AA-OD-005, Version 005, July 1990
 * 	[90/09/03            af]
 * 
 * Revision 2.2  93/03/09  10:48:49  danner
 * 	Jeffrey Heller created this from my mips code.
 * 	[93/03/06  14:27:58  af]
 * 
 * Revision 2.12.1.1  92/05/13  20:55:02  af
 * 	Use Ultrix' name for Maxine's screen.
 * 
 * Revision 2.12  92/05/05  10:46:55  danner
 * 	From jcb.
 * 	[92/05/04  11:37:51  af]
 * 
 * Revision 2.11.1.1  92/04/17  14:34:21  jcb
 * 	Added NSC's free running counter board for TC.
 * 	[92/04/17            jcb]
 * 
 * Revision 2.11  92/04/03  12:09:46  rpd
 * 	Support for FORE INC. ATM Board (ecc) board.
 * 	[92/03/30            rvb]
 * 
 * Revision 2.10  92/04/01  15:15:09  rpd
 * 	Added maxine's mappable timer.
 * 	[92/03/11  02:35:18  af]
 * 
 * Revision 2.9  92/02/19  16:46:36  elf
 * 	Additions for Maxine.  ANSI-C mods.  Define more slots.
 * 	[92/02/10  17:44:58  af]
 * 
 * Revision 2.8  91/08/24  12:22:04  af
 * 	Bugfix from Michael Pagels (pagels@cs.arizona.edu): did
 * 	not properly skip unknown devices.
 * 	[91/08/02  12:48:21  af]
 * 
 * 	Added PMAG-AA, and ASIC and Z8530 pseudos.
 * 	Made number of slots dynamic, probe them from
 * 	high to low slot number (system devices take precedence).
 * 	The 'someday' for an arbitrary interrupt enable function has come.
 * 	[91/08/02  03:33:19  af]
 * 
 * Revision 2.7  91/06/19  11:56:13  rvb
 * 	The busses.h and other header files have moved to the "chips"
 * 	directory.
 * 	[91/06/07            rvb]
 * 
 * Revision 2.6  91/05/14  17:31:45  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:46:36  mrt
 * 	Added author notices
 * 	[91/02/04  11:20:32  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:19:26  mrt]
 * 
 * Revision 2.4  90/12/05  23:35:59  af
 * 	Look for the standard offset as well as for the old one
 * 	when checking an option's ROM.
 * 	[90/12/03  23:50:37  af]
 * 
 */
/*
 *	File: tc.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	Routines for the TURBOchannel BUS.
 */

#include <sys/types.h>
#include <mach/std_types.h>
#include <alpha/alpha_cpu.h>

#include <chips/busses.h>
#include <alpha/DEC/tc.h>

static int	tc_probe_tc();
static char	my_name[] = "tc";
struct bus_driver tc_driver =
	{ tc_probe_tc, 0, tc_autoconf, 0, 0, my_name,};


#define	private	static
#define	public

/*
 *	Driver map: associates a device driver to an option type.
 *	Drivers name are (arbitrarily) defined in each driver and
 *	used in the various config tables.  Starred entries do not
 *	correspond to real TC boards, they are just a convenience.
 */
public
struct drivers_map {
	char		module_name[TC_ROM_LLEN];	/* from ROM, literally! */
	char		*driver_name;			/* in bus_??_init[] tables */
} tc_drivers_map[] = {
	{ "PMAD-AA ",	"se"},		/* Ether */
	{ "PMAD-BA ",	"se"},		/* Ether */
	{ "PMAZ-AA ",	"asc"},		/* SCSI */
	{ "PMAZ-DS ",	"asc"},		/* SCSI */
	{ "PMAG-AA ",	"fb"},		/* Mono Frame Buffer */
	{ "PMAG-BA ",	"cfb"},		/* Color Frame Buffer */
	{ "PMAGB-BA",	"sfb"},		/* Smart Frame Buffer */
#if 1	/* untested */
	{ "PMAGB-BB",	"sfb"},		/* Smart Frame Buffer */
	{ "PMAGB-BC",	"sfb"},		/* Smart Frame Buffer */
	{ "PMAGB-BD",	"sfb"},		/* Smart Frame Buffer */
	{ "PMAGB-BE",	"sfb"},		/* Smart Frame Buffer */
#endif
	{ "PMAG-CA ",	"ga"},		/* 2D graphic board */
	{ "PMAG-DA ",	"gq"},		/* 3D graphic board (LM) */
	{ "PMAG-FA ",	"gq"},		/* 3D graphic board (HE) */
	{ "PMAG-DV ",	"xcfb"},	/* (*) maxine Color Frame Buffer */
	{ "Z8530   ",	"scc"},		/* (*) 3min/maxine serial lines */
	{ "ASIC    ",	"asic"},	/* (*) 3min/maxine DMA controller */
/*	{ "XINE-FDC",	"fdc"},		 (*) maxine floppy controller */
/*	{ "DTOP    ",	"dtop"},	 (*) maxine desktop bus */
	{ "AMD79c30",	"isdn"},	/* (*) maxine ISDN chip */
	{ "FORE_ATM",	"ecc"},		/* Fore Inc. ATM */
/*	{ "XINE-FRC",	"frc"},		 (*) maxine free-running counter */
/*	{ "Counter ",	"frc"},		 NSC counter board for TC */
	{ "TOY_RTC ",	"mc"},		/* Motorola/Dallas clock chip */
	{ "", 0}			/* list end */
};

/*
 * Identify an option on the TC.  Looks at the mandatory
 * info in the option's ROM and checks it.
 */
public int tc_verbose = 0;

private boolean_t
tc_identify_option_from_rom(
	tc_rommap_t	*addr,
	tc_option_t	*slot,
	boolean_t	complain)
{
	register int	i;
	unsigned int    width;
	char            firmwr[TC_ROM_LLEN+1], vendor[TC_ROM_LLEN+1],
			module[TC_ROM_LLEN+1], host_type[TC_ROM_SLEN+1];

	/*
	 * We do not really use the 'width' info, but take advantage
	 * of the restriction that the spec impose on the portion
	 * of the ROM that maps between +0x3e0 and +0x470, which
	 * is the only piece we need to look at.
	 */
	width = addr->rom_width.value;
	switch (width) {
	    case 1:
	    case 2:
	    case 4:
		break;
	    default:
		if (complain)
			dprintf("%s (x%x) at x%x\n", "Invalid ROM width",
			       width, addr);
		return FALSE;
	}

	if (addr->rom_stride.value != 4) {
		if (complain)
			dprintf("%s (x%x) at x%x\n", "Invalid ROM stride",
			       addr->rom_stride.value, addr);
		return FALSE;
	}

	if ((addr->test_data[0] != 0x55) ||
	    (addr->test_data[4] != 0x00) ||
	    (addr->test_data[8] != 0xaa) ||
	    (addr->test_data[12] != 0xff)) {
		if (complain)
			dprintf("%s x%x\n", "Test pattern failed, option at",
			       addr);
		return FALSE;
	}

	for (i = 0; i < TC_ROM_LLEN; i++) {
		firmwr[i] = addr->firmware_rev[i].value;
		vendor[i] = addr->vendor_name[i].value;
		module[i] = addr->module_name[i].value;
		if (i >= TC_ROM_SLEN)
			continue;
		host_type[i] = addr->host_firmware_type[i].value;
	}
	firmwr[TC_ROM_LLEN] = vendor[TC_ROM_LLEN] =
	module[TC_ROM_LLEN] = host_type[TC_ROM_SLEN] = 0;

	if (tc_verbose)
	dprintf("%s %s '%s' at x%x\n %s %s %s '%s'\n %s %d %s %d %s\n",
	       "Found a", vendor, module, addr,
	       "Firmware rev.", firmwr,
	       "diagnostics for a", host_type,
	       "ROM size is", addr->rom_size.value << 3,
	       "Kbytes, uses", addr->slot_size.value, "TC slot(s)");

	bcopy(module, slot->module_name, TC_ROM_LLEN);
	bcopy(vendor,  slot->module_id, TC_ROM_LLEN);
	bcopy(firmwr, &slot->module_id[TC_ROM_LLEN], TC_ROM_LLEN);
	slot->slot_size = addr->slot_size.value;
	slot->rom_width = width;

	return TRUE;
}

/*
 * Probe a slot in the TC.  Returns TRUE if a valid option
 * is present, FALSE otherwise.  Side-effects the slot
 * descriptor with the size of the option, whether it is
 * recognized or not.
 */
private boolean_t
tc_probe_slot(
	vm_offset_t	addr,
	tc_option_t	*slot)
{
	int		i;
#	define		TC_N_OFFSETS	2
	static vm_offset_t	tc_offset_rom[TC_N_OFFSETS] = 
					{TC_OFF_PROTO_ROM, TC_OFF_ROM};

	slot->slot_size = 1;

	for (i = 0; i < TC_N_OFFSETS; i++) {
		if (check_memory(addr + tc_offset_rom[i], 0))
			continue;
		/* Complain only on last chance */
		if (tc_identify_option_from_rom(
				(tc_rommap_t *)(addr + tc_offset_rom[i]),
				slot,
				i == (TC_N_OFFSETS-1)))
			return TRUE;
	}
	return FALSE;
#undef	TC_N_OFFSETS
}

/*
 * TURBOchannel autoconf procedure.  Finds in one sweep what is
 * hanging on the bus and fills in the tc_slot_info array.
 * This is only the first part of the autoconf scheme, at this
 * time we are basically only looking for a graphics board to
 * use as system console (all workstations).
 */

#include <alpha/DEC/flamingo.h>

public
tc_option_t	tc_slot_info [TC_MAX_LOGICAL_SLOTS];

public
vm_offset_t	tc_slot_phys_base [TC_MAX_SLOTS] = {
	/* 
	 * Use flamingo for default values 
	 */
	KN15AA_PHYS_TC_0_START_S, KN15AA_PHYS_TC_1_START_S,
	KN15AA_PHYS_TC_2_START_S, KN15AA_PHYS_TC_3_START_S,
	KN15AA_PHYS_TC_4_START_S, KN15AA_PHYS_TC_5_START_S,
	KN15AA_PHYS_TC_6_START_S, KN15AA_PHYS_TC_7_START_S
};

/* Will scan from max to min, inclusive */
public short	tc_max_slot = KN15AA_TC_MAX;
public short	tc_min_slot = KN15AA_TC_MIN;

public void (*tc_slot_hand_fill)( tc_option_t* ) = 0;

public
void
tc_find_all_options()
{
	register int	i;
	vm_offset_t    addr;
	boolean_t       found;
	register tc_option_t	*sl;
	struct drivers_map	*map;
	struct bus_ctlr		*m;
	struct bus_device	*d;

	/*
	 * Take a look at the bus
	 */
	bzero(tc_slot_info, sizeof(tc_slot_info));
	for (i = tc_max_slot; i >= tc_min_slot;) {
#if 1
		/* This needs kseg1 1:1 mapping turned on */
		addr = PHYS_TO_K0SEG(tc_slot_phys_base[i]);

#else
		addr = pmap_map_io(tc_slot_phys_base[i],0x2000);
#endif
		found = tc_probe_slot(addr, &tc_slot_info[i]);

		if (found) {
			/*
			 * Found a slot, make a note of it 
			 */
			tc_slot_info[i].present = 1;
			tc_slot_info[i].k1seg_address = addr;
		}

		i -= tc_slot_info[i].slot_size;
	}

	/*
	 * Some slots (e.g. the system slot on 3max) might require
	 * hand-filling.  If so, do it now. 
	 */
	if (tc_slot_hand_fill)
		(*tc_slot_hand_fill) (tc_slot_info);

	/*
	 * Now for each alive slot see if we have a device driver that
	 * handles it.  This is done in "priority order", meaning that
	 * always present devices are at higher slot numbers on all
	 * current TC machines, and option slots are at lowest numbers.
	 */
	for (i = TC_MAX_LOGICAL_SLOTS-1; i >= 0; i--) {
		sl = &tc_slot_info[i];
		if (!sl->present)
			continue;
		found = FALSE;
		for (map = tc_drivers_map; map->driver_name; map++) {
			if (strncmp(sl->module_name, map->module_name, TC_ROM_LLEN))
				continue;
			sl->driver_name = map->driver_name;
			found = TRUE;
			break;
		}
		if (!found) {
			dprintf("%s %s %s %d%s\n",
				"Cannot associate a device driver to",
			        sl->module_name, "at TCslot", i,
				". Will (try to) ignore it.");
			sl->present = 0;
			continue;
		}
		/*
		 * We have a device driver.  Associate its interrupt routine
		 * with this slot. Also make sure we will find it later at
		 * this very same slot, we do not want to screwup the 'unit'.
		 */
		found = FALSE;
		for (m = bus_master_init; m->driver; m++) {
			if (strcmp(m->name,sl->driver_name))
				continue;
			if (m->adaptor == '?')
				m->adaptor = i;
			if (m->adaptor != i)
				continue;
			sl->intr	= m->intr;
			sl->isa_ctlr	= TRUE;
			sl->unit	= m->unit;
			sl->driver	= m->driver;
			found		= TRUE;
			break;
		}
		if (found)
			continue;
		for (d = bus_device_init; d->driver; d++) {
			if (strcmp(d->name,sl->driver_name))
				continue;
			if (d->intr == 0)
				continue;
			if (d->adaptor == '?')
				d->adaptor = i;
			if (d->adaptor != i)
				continue;
			sl->intr	= d->intr;
			sl->driver	= d->driver;
			sl->unit	= d->unit;
			found		= TRUE;
			break;
		}
		if (!found) {
			dprintf("Cannot find \"%s\" device driver.  Will (try to) ignore it.\n",
			       sl->driver_name);
			sl->driver_name = (char *) 0;
			sl->present = 0;
			continue;
		}
	}
}

/*
 * See if a particular driver has a corresponding option
 * installed.
 */
public 
vm_offset_t
tc_probe(
	char	*name)
{
	register int    i;
	register tc_option_t *sl;

	for (i = 0, sl = tc_slot_info; i < TC_MAX_LOGICAL_SLOTS; i++, sl++) {
		if (!sl->driver_name)
			continue;
		if (strcmp(sl->driver_name, name) == 0)
			return sl->k1seg_address;
	}
	return 0;
}


/* This really probes if there is a TC adaptor on this box. */
public
tc_probe_tc()
{
	return (tc_slot_info != 0);
}


/*
 * Autoconfiguration for the TURBOchannel
 * Assumes you did a tc_find_all_options() first.
 */
public void
tc_autoconf()
{
	register int    i;
	boolean_t       found;
	register tc_option_t *sl;

	/*
	 * Interrupts off
	 */
	for (i = 0; i < TC_MAX_LOGICAL_SLOTS; i++)
		(*tc_enable_interrupt)(i, FALSE, 0); /* XXX last 0 changes on
						      * sandpiper */

	/*
	 * Do the configuration as we commonly know it: for each device driver
	 * call probe and attach. 
	 */
	for (i = TC_MAX_LOGICAL_SLOTS-1; i >= 0; i--) {
		boolean_t	(*config) ();

		sl = &tc_slot_info[i];
		if (!sl->present)
			continue;

		config = (sl->isa_ctlr) ? configure_bus_master : configure_bus_device;
		/*
		 * If probing needs interrupts on, enable them 
		 */
		if (sl->driver->flags & BUS_INTR_B4_PROBE)
			(*tc_enable_interrupt)(i, TRUE, 0);/* XXX last 0 needs
							    * to change on a
							    * sandpiper 
							    */
/*XXX need to replace the K0SEG_TO_PHYS below */

		found = (*config) (sl->driver_name, sl->k1seg_address,
				   K0SEG_TO_PHYS(sl->k1seg_address),
				   i, "tc");

		if (!found) {
			printf("%s %s %s %s%d\n",
			       sl->driver_name, "did not recognize",
			       sl->module_name, "at tc", i);
			(*tc_enable_interrupt)(i, FALSE, 0);
			sl->present = 0;
			continue;
		}
		/*
		 * Enable interrupts from this slot, unless.. 
		 */
		(*tc_enable_interrupt)(i, (sl->driver->flags & BUS_INTR_DISABLED) == 0, 0);
	}
}

/*
 * Yet another machdep switch
 */
extern vm_size_t kn15aa_enable_interrupt();

public vm_size_t (*tc_enable_interrupt)() = kn15aa_enable_interrupt;

int
tc_intr()
{
}

