/* 
 * Mach Operating System
 * Copyright (c) 1993,1992 Carnegie Mellon University
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
 * $Log:	autoconf.c,v $
 * Revision 2.4  93/08/10  15:15:34  mrt
 * 	The answer to the puzzle below came in, and it reads:
 * 	"..all the systems implemented the fields backwards, so that
 * 	MAJOR and MINOR were switched.  The SRM was changed to match.."
 * 	[93/08/06            af]
 * 
 * 	An FRU on my Flamingo brought by new codes for the processor
 * 	type.
 * 	[93/08/05            af]
 * 
 * Revision 2.3  93/01/19  08:29:08  danner
 * 	Added reference to doc for the HWRPB &co.
 * 	[93/01/19            af]
 * 
 * Revision 2.2  93/01/14  17:11:58  danner
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:11:55  af]
 * 
 * 	Created.
 * 	[92/12/10  14:53:51  af]
 * 
 */

/*
 *	File: autoconf.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Setup the system to run on the current machine.
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 *
 *	"VMS for Alpha Platforms Internals and Data Structures"
 *	Digital Press 1992, Burlington, MA 01803
 *	Order number EY-L466E-P1/2, ISBN 1-55558-095-5
 *	[Especially volume 1, chapter 33 "Bootstrap processing"]
 */

#include <platforms.h>
#include <cpus.h>

#include <mach/std_types.h>
#include <sys/types.h>
#include <chips/busses.h>

#include <mach/machine.h>
#include <kern/cpu_number.h>
#include <kern/zalloc.h>

#include <alpha/thread.h>
#include <alpha/alpha_cpu.h>
#include <alpha/prom_interface.h>

boolean_t	cold;		/* flag we're configuring */

/*
 * Get cpu type, and then switch out to machine specific procedures
 * which will probe adaptors to see what is out there.
 */
machine_init()
{
#if	NCPUS > 1
	/*
	 * See how many cpus we got
	 */
	extern int processors_running[];
	int i;

	for (i = 0; i < NCPUS; i++)
		if (processors_running[i]) machine_slot[i].is_cpu = TRUE;
#endif

	cold = 1;

	/*
	 * Say what kind of cpu chip is the primary
	 */
	identify_cpu(master_cpu);

	/*
	 * Find out what sort of machine we are on.
	 */
	identify_alphabox();

	/*
	 * Now for peripherals.
	 * Note that console is already up by now.
	 */
	identify_devices();

	/*
	 * Find root device, if different from boot
	 */
	get_root_name();

	/*
	 * All done.
	 */
	cold = 0;
}


void slave_machine_init()
{
	identify_cpu(cpu_number());
}


/*
 *	Device probing procedure.
 */

identify_devices()
{
	/* each bus should have its prober (a-la tc_config())
	   which is truly searched when we look for the console.
	   Here we should just call configure_bus_master on the
	   bus descriptors of all the possible busses */

	register struct bus_ctlr	*bus = bus_master_init;

	while (bus->driver) {
		if ((bus->flags & BUS_CTLR) &&
		    ((*bus->driver->probe)(0,bus) != 0)) {
			printf("Attaching %s%d:\n", bus->name, bus->unit);
			(*bus->driver->attach)((struct bus_device *)bus);
		}
		bus++;
	}
}

/*
 * Say which machine
 */
identify_alphabox()
{
#	define	_MAX_KNOWN_ST_	6
	static char *system_types[_MAX_KNOWN_ST_+1] = {
		"Undefined", "ADU", "Cobra", "Ruby", "Flamingo",
		"Mannequin", "Jensen"
	};

	/*
	 * Sanity checks
	 */
	if (alpha_hwrpb->my_version > 3)
		printf("Warning: HWRPB version %d might be incompatible\n",
			alpha_hwrpb->my_version);
#if	NCPUS > 1
	if (alpha_hwrpb->primary_cpu_id != cpu_number())
		printf("Mumble.. primary cpu %d not master %d ?\n",
			alpha_hwrpb->primary_cpu_id, cpu_number());
#endif

	/*
	 * System data
	 */
	{
		char	*box;
		char	*ssn = alpha_hwrpb->system_serial_number;
		char	*rev = alpha_hwrpb->system_revision;

		if (alpha_hwrpb->system_type > _MAX_KNOWN_ST_)
			box = "New/Unknown System";
		else
			box = system_types[alpha_hwrpb->system_type];

		printf("box: %s, Revision %c%c%c%c, Variation %x,",
			box, rev[0], rev[1], rev[2], rev[3],
			alpha_hwrpb->system_variation);

		printf(" Serial Number %c%c%c%c%c%c%c%c%c%c\n",
			ssn[0], ssn[1], ssn[2], ssn[3], ssn[4],
			ssn[5],	ssn[6], ssn[7], ssn[8], ssn[9]);
	}

	/*
	 * Clock and timings
	 */
	{
		extern int hz, tick;

		hz = alpha_hwrpb->clock_interrupt_frequency / 4096;
		tick = 1000000 / hz;
		printf("clk: %d cycles/sec. HZ is %d intr/sec.\n",
			alpha_hwrpb->cycle_counter_resolution, hz);
	}

}

/* Here we truly want our hw slot */
#ifdef	cpu_number
#undef	cpu_number
#endif
/*
 * Say what cpu we've got
 */
identify_cpu(self)
	int self;
{
	struct per_cpu_slot	*pcpu;
	unsigned int		slotno = cpu_number();

#	define	_MAX_KNOWN_CT_	3
	static char	*cpu_names[_MAX_KNOWN_CT_+1] = {
		"Unknown", "EV3", "EV4", "ISP"
	};

/*	if (alpha_hwrpb->system_variation & SYSTEM_VAR_MPCAP) broken */
#if	(NCPUS>1)
	printf("%s cpu %d: ",
			(self == master_cpu) ? "Primary" : "Secondary",
			slotno);
#else
	printf("cpu: ");
#endif

	pcpu = (struct per_cpu_slot *)
		((vm_offset_t)alpha_hwrpb + alpha_hwrpb->percpu_slots_offset +
		 (slotno * alpha_hwrpb->percpu_slot_size));

	machine_slot[self].is_cpu	= TRUE;
	machine_slot[self].cpu_type	= CPU_TYPE_ALPHA;
	machine_slot[self].cpu_subtype	= pcpu->processor_major_type;
	machine_slot[self].running	= TRUE;
	machine_slot[self].clock_freq	= /* hz incorrect for now */
			alpha_hwrpb->clock_interrupt_frequency / 4096;

	/*
	 * Say about the processor itself
	 */
	{
		char			*cpuname, *pass, *rev, *ssn;

		if (pcpu->processor_major_type > _MAX_KNOWN_CT_)
			cpuname = "New/Unknown";
		else
			cpuname = cpu_names[pcpu->processor_major_type];

		switch (pcpu->processor_minor_type) {
		    case 0:	pass = " Pass 2"; break;
		    case 1:	pass = " Pass 3"; break;
		    default:	pass = ""; break;
		}

		rev = pcpu->processor_revision;

		ssn = pcpu->processor_serial_number;

		printf("Alpha %s%s Processor, Variation %x, ",
			cpuname, pass, /* pcpu->state_flags, */
			pcpu->processor_variation);

		printf("Revision %c%c%c%c, Serial Number %c%c%c%c%c%c%c%c%c%c\n",
			rev[0], rev[1], rev[2], rev[3],
			ssn[0], ssn[1], ssn[2], ssn[3], ssn[4],
			ssn[5], ssn[6], ssn[7], ssn[8], ssn[9]);
#if 0
EV3 has 1k caches, EV4 has 8k, 21064 has 8k
#endif
	}

	/*
	 * Say about the PAL code
	 */
	{
		union {
		    struct {
			unsigned char	minor_no;
			unsigned char	major_no;
			unsigned char	type;
			unsigned char	mbz;
			unsigned short	compatibility;
			unsigned short	max_share;
		    } info;
		    natural_t	bits;
		} palrev;

		palrev.bits = pcpu->palcode_revision_info;
		printf("pal: at slot %d Rev %d.%d, Type %d, Compat %d, MP %x\n",
			slotno, palrev.info.major_no, palrev.info.minor_no,
			palrev.info.type, palrev.info.compatibility,
			palrev.info.max_share);
	}
}

