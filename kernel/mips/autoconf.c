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
 * $Log:	autoconf.c,v $
 * Revision 2.19  93/05/30  21:09:00  rvb
 * 	Added 5000/240.
 * 	[93/05/29  09:53:24  af]
 * 
 * Revision 2.18  93/01/14  17:50:55  danner
 * 	cleaned tiny spl spot.
 * 	[92/12/01            af]
 * 
 * Revision 2.17  92/02/19  16:46:45  elf
 * 	Added Maxine.
 * 	[92/02/10  17:40:34  af]
 * 
 * Revision 2.16  91/09/04  11:41:24  jsb
 * 	Don't let ethernet interrupts blow us away.
 * 	[91/09/04  11:36:17  jsb]
 * 
 * Revision 2.15  91/08/24  12:22:17  af
 * 	We understand 3mins.  Finding systype is a problem now.
 * 	[91/08/02  03:24:57  af]
 * 
 * Revision 2.14  91/07/31  17:56:28  dbg
 * 	Moved call to pcb_module_init to machine-independent code.
 * 	[91/07/26            dbg]
 * 
 * Revision 2.13  91/06/19  11:56:16  rvb
 * 	#ifdef PMAX -> #ifdef DECSTATION and we include <platforms.h>
 * 	[91/06/12  14:08:42  rvb]
 * 
 * Revision 2.12  91/05/14  17:32:11  mrt
 * 	Correcting copyright
 * 
 * Revision 2.11  91/03/16  14:55:11  rpd
 * 	Replaced pcb_zone initialization with pcb_module_init.
 * 	[91/02/17            rpd]
 * 
 * Revision 2.10  91/02/14  14:35:37  mrt
 * 	New values for new delay() function.
 * 	[91/02/12  12:29:29  af]
 * 
 * Revision 2.9  91/02/05  17:46:58  mrt
 * 	Added author notices
 * 	[91/02/04  11:20:55  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:24:26  mrt]
 * 
 * Revision 2.8  91/01/08  15:48:46  rpd
 * 	Added pcb_zone.
 * 	[90/12/09  17:34:12  rpd]
 * 
 * Revision 2.7  90/12/05  23:36:17  af
 * 
 * 
 * Revision 2.6  90/12/05  20:49:24  af
 * 	Mods for new, copyright free PMAX drivers.
 * 	Use the configure_bus_.. functions for probing on pmaxen.
 * 	Since there really is a new R3000A chip from MIPSco now,
 * 	use the proper name for the one on 3maxen.
 * 
 * 	Reflected new copyright-free autoconf code defines and funs.
 * 	[90/12/03  22:59:59  af]
 * 
 * Revision 2.5  90/08/28  19:05:35  dbg
 * 	Initialize console TTY structures after VM system is running
 * 	(machine_init).
 * 	[90/08/28            dbg]
 * 
 * Revision 2.4  90/08/07  22:28:56  rpd
 * 	New, bus-dependent autoconf procedures. 3max support.
 * 	[90/08/07  15:26:25  af]
 * 
 * Revision 2.2  89/11/29  14:12:39  af
 * 	Reflected name change in ether device.
 * 	[89/11/26  10:31:06  af]
 * 
 * 	Changes for pure kernel.
 * 	[89/10/04            af]
 * 
 * Revision 2.2  89/07/14  15:27:34  rvb
 * 	Pass back to users the clock frequency in machine_info[].
 * 	[89/07/12            af]
 * 
 * Revision 2.1  89/05/30  12:55:24  rvb
 * Created.
 * 
 */

/*
 * Setup the system to run on the current machine.
 *
 */

#include <platforms.h>
#include <vme.h>
#include <cpus.h>

#include <mach/machine.h>
#include <kern/cpu_number.h>
#include <kern/zalloc.h>

#include <mips/thread.h>
#include <mips/mips_cpu.h>
#include <mips/mips_box.h>
#include <mips/prom_interface.h>

int	cold;		/* flag we're configuring */

/*
 * coprocessor revision identifiers
 */
unsigned fpa_type;

/*
 * Get cpu type, and then switch out to machine specific procedures
 * which will probe adaptors to see what is out there.
 */
machine_init()
{
	cpu_subtype_t subtype;
	extern int hz;
	extern int (*identify_devices)();

	cold = 1;

	/*
	 * Look out and see how many coprocessors we have
	 * and what kind of cpu chip.
	 */
	identify_cpu();

	/*
	 * Find out what sort of machine we are on.
	 */
	subtype = identify_mipsbox();

	master_cpu = 0;
#if	NCPUS > 1
	printf("Master cpu at slot %d.\n", master_cpu);
#endif	NCPUS > 1
	machine_slot[master_cpu].is_cpu = TRUE;
	machine_slot[master_cpu].cpu_type = CPU_TYPE_MIPS;
	machine_slot[master_cpu].cpu_subtype = subtype;
	machine_slot[master_cpu].running = TRUE;
	machine_slot[master_cpu].clock_freq = hz;

	/*
	 * Now for peripherals.
	 * Note that console is already up by now.
	 */
	(*identify_devices)();

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
	/* Nothing yet */
}


/*
 *	Device probing procedures.  These
 *	are largely bus-specific functions.
 */
#if	(NVME > 0)

config_vme_routine()
{
	/*
	 * Initialize the VME bus interface
	 */
	vme_init();

	/*
	 * Lower priority so that interrupts will get through
	 */
	(void) spl0();

	/*
	 * Look for devices on VME bus
	 */
	vmefind();

}

#endif	(NVME > 0)

#ifdef	MSERIES
int (*identify_devices)() = config_vme_routine;
#endif	MSERIES


#ifdef	DECSTATION

config_pmax_routine() 
{
	/*
	 * Make sure lance will not bother us in the following
	 */
	extern char	*se_unprobed_addr;

	se_unprobed_addr = (char*)0xb8000000;

	/*
	 * Configure the system clock 
	 */
	config_delay(8);

	/*
	 * Normally, we would first probe the bitmap screen here (both color
	 * and mono bitmaps use the same driver), but we wanted to use it
	 * much earlier on, so cons_attach() has done most of the work.
	 */
	configure_bus_device( "dz", 0, 0, 1, "slot");
	configure_bus_device( "pm", 0, 0, 0, "slot");

	/*
	 * Probe the sii based scsi bus. 
	 */
	configure_bus_master( "sii", 0xba000000, 0, 2, "slot");

	/*
	 * Probe the lance based ethernet network. 
	 */
	configure_bus_device( "se", 0xb8000000, 0, 3, "slot");

}

config_3max_routine() 
{
	extern int tc_autoconf();

	/*
	 * Configure the system clock
	 */
	config_delay(12);

	/*
	 * Config the TURBOchannel
	 */
	tc_autoconf();
}

int (*identify_devices)() = config_pmax_routine;

#endif	/*DECSTATION*/


/*
 *	Table to decode hardware IDs
 */
typedef	struct {
	char 		*name;		/* readable name */
	unsigned	 id;		/* code */
} hardware_id_table;


/*
 *  Implementation tables for cpu and floating point coprocessors
 */
static hardware_id_table cpu_names[] = {
	/* MipsCo byteorder and fab codes */
	{ "R2000",				0 },
	{ "R2000",				1 },
	{ "R3000",				2 },
	{ "R2300",				3 },
	/* DEC byteorder and fab codes */
	{ "R2000A",				0x81 },
	{ "R3000",				0x82 },
	{ 0,					0 }
};

static hardware_id_table fpa_names[] = {
	/* MipsCo byteorder and fab codes */
	{ "R2360 Floating Point Board",		1 },
	{ "R2010 VLSI Floating Point Chip",	2 },
	{ "R3010 VLSI Floating Point Chip",	3 },
	/* DEC byteorder and fab codes */
 	{ "R2010A VLSI Floating Point Chip",	0x82 },
	{ "R3010 VLSI Floating Point Chip",	0x83 },
	{ 0,					0 }
};

static char *
decode_id(id, tbl)
unsigned id;
hardware_id_table *tbl;
{
	for (; tbl->name; tbl++)
		if (tbl->id == id)
			return(tbl->name);
	return("Unknown");
}


/*
 * Find out what cpu and coprocessors we've got
 */
identify_cpu()
{
	union {
		unsigned int	bits;
		struct {
#if	BYTE_MSF
		unsigned int	pad : 16,
				imp : 8,		/* implementation */
				maj : 4,		/* major revision */
				min : 4;		/* minor revision */
#else	BYTE_MSF
		unsigned int	min : 4,		/* minor revision */
				maj : 4,		/* major revision */
				imp : 8,		/* implementation */
				pad : 16;
#endif	BYTE_MSF
		} id;
	} identification;
	unsigned char brd, mod = 0;

	/*
	 * First thing first: the cpu chip
	 */
	identification.bits = cpu_get_prid();
#ifdef	DECSTATION
	/* Pmaxen cheat.. */
	if (identification.id.imp == 2 &&
	    identification.id.maj == 1 &&
	    identification.id.min == 6)
		identification.id.imp = 1;
	/* .. and DEC uses a different byteorder */
	mod = 0x80;
#endif
	printf("cpu: MIPS %s Processor Chip, Version %d Revision %d.%d\n",
	    decode_id(identification.id.imp|mod, cpu_names),
	    identification.id.imp,
	    identification.id.maj,
	    identification.id.min);

	/*
	 * For floating point, some early Mseries boxes
	 * do not have an FPA coprocessor and it is not
	 * safe to call fpa_get_irr();
	 */

#ifdef	MSERIES
	brd = *(unsigned char *)PHYS_TO_K1SEG(CPU_CONFIG);
#else
	brd = CONFIG_INVAL;
#endif
	if ((brd == CONFIG_INVAL) || ((brd & CONFIG_NOCP1) == 0)) {
		identification.bits = fpa_get_irr();
		fpa_type = identification.bits & FPA_IRR_IMP;
#ifdef	DECSTATION
		/* Like I said, Pmaxen cheat.. */
		if (identification.id.imp == 3 &&
		    identification.id.maj == 1 &&
		    identification.id.min == 5)
			identification.id.imp = 2;
#endif
		printf("fpa: MIPS %s, Version %d Revision %d.%d\n",
		    decode_id(identification.id.imp|mod, fpa_names),
		    identification.id.imp, identification.id.maj,
		    identification.id.min);
		fp_init();
	} else {
		fpa_type = 0;
		printf("No floating point coprocessor\n");
	}
}

cpu_subtype_t
identify_mipsbox()
{
	int      mipsbox_type, mipsbox_rev;
	char	*mipsbox_name;
	int	 temp;
	cpu_subtype_t	ret;

#ifdef	MSERIES
	mipsbox_type = *(unsigned char *) PHYS_TO_K1SEG(IDPROM_TYPE);
	mipsbox_rev  = *(unsigned char *) PHYS_TO_K1SEG(IDPROM_REV);
#endif
#ifdef	DECSTATION
	extern int (*find_systype)();

	temp = (*find_systype)();
	mipsbox_type = dec_systype(temp) | 0x80;
	mipsbox_rev  = dec_revtype(temp);
#endif	/*DECSTATION*/

	switch (mipsbox_type) {
	    case BRDTYPE_R2300:
		mipsbox_name = "MIPS R2300";
		ret = CPU_SUBTYPE_MIPS_R2300;
		break;
	    case BRDTYPE_R2600:
		mipsbox_name = "MIPS R2600";
		ret = CPU_SUBTYPE_MIPS_R2600;
		break;
	    case BRDTYPE_R2800:
		mipsbox_name = "MIPS R2800";
		ret = CPU_SUBTYPE_MIPS_R2800;
		break;

	    case BRDTYPE_DEC3100:
		mipsbox_name = "DEC 3100";
		ret = CPU_SUBTYPE_MIPS_R2000a;
		break;
	    case BRDTYPE_DEC5000:
		mipsbox_name = "DEC 5000/200";
		ret = CPU_SUBTYPE_MIPS_R3000a;
		break;
	    case BRDTYPE_DEC5000_100:
		mipsbox_name = "DEC 5000/100";
		ret = CPU_SUBTYPE_MIPS_R3000a;
		break;
	    case BRDTYPE_DEC5000_20:
		mipsbox_name = "Personal DS 5000";
		ret = CPU_SUBTYPE_MIPS_R3000a;
		break;
	    case BRDTYPE_DEC5000_240:
		mipsbox_name = "DEC 5000/240";
		ret = CPU_SUBTYPE_MIPS_R3000a;
		break;
	    case BRDTYPE_DEC5800:
		mipsbox_name = "DEC 5800";
		ret = CPU_SUBTYPE_MIPS_R3000a;
		break;
	    case BRDTYPE_DEC5400:
		mipsbox_name = "DEC 5400";
		ret = CPU_SUBTYPE_MIPS_R3000a;
		break;
	    default:
		mipsbox_name = "Unknown";
		ret = CPU_SUBTYPE_MIPS_R2000;
		break;
	}
	printf("box: %s, Revision %x.%x, ",
		mipsbox_name,
		(mipsbox_rev & 0xf0) >> 4,
		mipsbox_rev & 0xf);
#ifdef	MSERIES
	printf("Serial Number %c%c%c%c%c\n",
	       *(char *) PHYS_TO_K1SEG(IDPROM_SN1),
	       *(char *) PHYS_TO_K1SEG(IDPROM_SN2),
	       *(char *) PHYS_TO_K1SEG(IDPROM_SN3),
	       *(char *) PHYS_TO_K1SEG(IDPROM_SN4),
	       *(char *) PHYS_TO_K1SEG(IDPROM_SN5));
#endif	MSERIES
#ifdef	DECSTATION
	printf("Firmware revision %d,  Hardware revision %d\n",
	       dec_frmrev(mipsbox_rev), dec_hrdrev(mipsbox_rev));
#endif	/*DECSTATION*/

	return ret;
}


/*
 * Initialize the floating point coprocessor
 * to a known and silent state
 */
fp_init()
{
#ifdef	MSERIES
	register volatile char *led = (char*)PHYS_TO_K1SEG(LED_REG);

	*led = -1 &~ LED_FPBD_RUN;
	wbflush();
	delay(10);
	*led = -1;
	wbflush();
	delay(10);
#endif	MSERIES

	fpa_set_csr(0);
	if (fpa_type == FPA_IMP_R2360)
		fpa_set_irr(-1);
}
