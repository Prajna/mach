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
 * $Log:	tvbus.c,v $
 * Revision 2.4  93/03/26  17:55:11  mrt
 * 	Fixed some decls.
 * 	[93/03/23            af]
 * 
 * Revision 2.3  93/03/09  10:48:58  danner
 * 	GCC lint.
 * 	[93/03/05            af]
 * 
 * Revision 2.2  93/01/14  17:10:43  danner
 * 	Created, from the DEC specs:
 * 	"Alpha Demonstration Unit Specification"
 * 	V1.0, Aug 1990.
 * 	[92/06/02            af]
 * 
 */
/*
 *	File: tvbus.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	ADU's TVbus routines.
 */

#include <mach/std_types.h>
#include <sys/types.h>
#include <chips/busses.h>
#include <alpha/alpha_cpu.h>
#include <alpha/alpha_scb.h>

#include <alpha/DEC/tvbus.h>
#include <alpha/prom_interface.h>

extern vm_offset_t	pmap_map_io( vm_offset_t phys, vm_size_t length);

extern int	tv_intr(), tv_probe();
extern void	tv_autoconf();

static char	my_name[] = "tv";

struct bus_driver tv_driver =
	{ tv_probe, 0, tv_autoconf, 0, 0, my_name,};

tv_any_module_t *tv_slot_info = 0;

void tv_find_all_options()
{
	struct memory_data_descriptor_table *mddt;

	mddt = (struct memory_data_descriptor_table *)
		((char*)alpha_hwrpb + alpha_hwrpb->memory_data_descriptor_table_offset);

	tv_slot_info = (tv_any_module_t *)
		PHYS_TO_K0SEG(mddt->implementation_specific_table_address);

}

long tv_console_slot()
{
	long i;
	tv_any_module_t	*t = tv_slot_info;

	for (i = 0; i < TV_MAX_SLOTS; i++, t++)
	    if (t->h.type == TV_TYPE_IO_MODULE &&
		(t->h.flags & TV_FLAG_CONSOLE))
			return i;
	return -1;
}

vm_offset_t tv_map_console()
{
	static vm_offset_t	cnsvirt = 0;

	if (cnsvirt == 0)
	    cnsvirt = pmap_map_io( TV_PHYS_START + (long)(tv_console_slot() * (long)TV_SLOT_SIZE),
			    0x800000);
	return cnsvirt;
}

int tv_probe()
{
	return (tv_slot_info != 0);
}

void tv_autoconf()
{
	int i, n_io;
	tv_memory_module_t	*t = (tv_memory_module_t *)tv_slot_info;
	static char *names[] = {
		"", "I/O", "64Mb Memory", "EV3 Processor",  "EV4 Processor"
	};

	for (n_io = i = 0; i < TV_MAX_SLOTS; i++, t++) {

		unsigned long f = t->h.flags;

		switch (t->h.type & 0xff) {

		case TV_TYPE_EMPTY:
			break;
		case TV_TYPE_IO_MODULE:
			tv_io_module(n_io++,i,t);
			break;
		case TV_TYPE_MEMORY:
		case TV_TYPE_DC227:
		case TV_TYPE_DC228:
			printf("%s%d: %s %b\n", my_name,
				i, names[t->h.type],
				f, "\20\1broken\2primary-cpu\3console");
			break;
		default:
			printf("%s%d: ?Unk module type x%x (%x)\n", my_name,
				i, t->h.type, f);
		}
	}
}


tv_io_module(
	int		unit,
	int		slotno,
	tv_io_module_t	*slotp)
{
	vm_offset_t	virt, phys;

	printf("Probing I/O module #%d, at tv%d\n",
		unit, slotno);

	phys = TV_PHYS_START + (slotno * TV_SLOT_SIZE);
	if (slotno == tv_console_slot())
		virt = tv_map_console();
	else
		virt = pmap_map_io( phys, 0x800000);

	configure_bus_device( "asl", virt, phys, slotno, my_name);
	adu_se_etheraddr(unit, slotp->ether_address);
	configure_bus_device( "ase", virt, phys, slotno, my_name);
	asz_set_scsiid(unit, slotp->scsi_id);
	configure_bus_master( "asz", virt, phys, slotno, my_name);
}

/*
 * Interrupts
 */

tv_get_channel(
	int		cpu_no,
	void		(*routine)(),
	natural_t	argument)
{
	static int	last_chan = 0;
	long		scbno, chan;

	chan = last_chan++;

	/* we dispatch interrupts on one node only */
	if (chan > TV_MAX_IPL20_CHAN)
		panic("tv_get_channel");

	scbno = SCB_INTERRUPT_FIRST + 32*(cpu_no-3) + chan;

	/* tell interrupt() to handle it */
	interrupt_dispatch(chan, routine, argument);

	/* tell locore that interrupt() knows */
	alpha_set_scb_entry( scbno, (void(*)())chan);

	return chan;
}

tv_intr()
{
}
