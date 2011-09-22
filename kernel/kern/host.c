/* 
 * Mach Operating System
 * Copyright (c) 1993,1992,1991,1990,1989,1988 Carnegie Mellon University
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
 * $Log:	host.c,v $
 * 	Fixed host_info to correctly return min_timeout and min_quantum
 * 	in milliseconds.  Added ANSI function prototypes.
 * 	[93/01/27            dbg]
 * 
 * Revision 2.10  93/01/14  17:34:15  danner
 * 	64bit cleanup.
 * 	[92/12/01            af]
 * 
 * Revision 2.9  92/08/03  17:37:08  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.8  92/05/21  17:13:27  jfriedl
 * 	Removed unused variable 'size_used' from host_processor_sets.
 * 	Changes to quiet gcc warnings.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.7  91/05/14  16:41:16  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/02/05  17:26:15  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:12:16  mrt]
 * 
 * Revision 2.5  90/08/27  22:02:30  dbg
 * 	Fix bug in host_processor_sets.  Import assert.h.
 * 	[90/07/18            dbg]
 * 
 * Revision 2.4  90/08/07  17:58:28  rpd
 * 	Changed host_processor_sets to use unprivileged ports.
 * 	[90/08/07            rpd]
 * 
 * Revision 2.3  90/06/19  22:58:20  rpd
 * 	Fixed bug in host_kernel_version.
 * 	[90/06/16            rpd]
 * 
 * 	Fixed bug in host_processors.
 * 	[90/06/16            rpd]
 * 
 * Revision 2.2  90/06/02  14:53:48  rpd
 * 	Added HOST_LOAD_INFO.
 * 	[90/04/27            rpd]
 * 	Created for new host/processor technology.
 * 	[90/03/26  23:50:27  rpd]
 * 
 * 	Move includes.
 * 	[89/08/02            dlb]
 * 	Remove interrupt protection from all_psets lock.
 * 	[89/06/14            dlb]
 * 
 * 	Add host_processor_sets and host_processor_set_priv.
 * 	[89/06/09            dlb]
 * 
 * 	Add scheduler information flavor to host_info.
 * 	[89/06/08            dlb]
 * 	Declare realhost here.
 * 	[89/02/03            dlb]
 * 	[89/01/30  16:54:25  dlb]
 * 
 * Revision 2.3  89/10/15  02:03:53  rpd
 * 	Minor cleanups.
 * 
 * Revision 2.2  89/10/11  14:04:44  dlb
 * 	Optimize host_processor_sets and add simplified !MACH_HOST case.
 * 	Remove interrupt protection from all_psets lock.
 * 	Add host_processor_sets and host_processor_set_priv.
 * 	Add scheduler information flavor to host_info.
 * 
 *	Reformat includes.  mach_host.h moved to mach/ directory.
 *	[89/01/26            dlb]
 *	
 *	Move kernel version to a separate routine so it can be returned
 *	as a string of characters.
 *	[88/12/02            dlb]
 *	
 *	Bug fixes, add new flavors of host information.
 *	[88/12/01            dlb]
 *	
 *	Created.
 *	[88/10/31            dlb]
 */

/*
 *	host.c
 *
 *	Non-ipc host functions.
 */

#include <cpus.h>
#include <mach_host.h>

#include <kern/assert.h>
#include <kern/kalloc.h>
#include <kern/host.h>
#include <mach/host_info.h>
#include <mach/kern_return.h>
#include <mach/machine.h>
#include <mach/port.h>
#include <kern/processor.h>
#include <kern/ipc_host.h>

#include <mach/vm_param.h>



host_data_t	realhost;

kern_return_t host_processors(
	host_t			host,
	processor_array_t	*processor_list,
	natural_t		*countp)
{
	register int		i;
	register processor_t	*tp;
	vm_offset_t		addr;
	unsigned int		count;

	if (host == HOST_NULL)
		return KERN_INVALID_ARGUMENT;

	/*
	 *	Determine how many processors we have.
	 *	(This number shouldn't change.)
	 */

	count = 0;
	for (i = 0; i < NCPUS; i++)
		if (machine_slot[i].is_cpu)
			count++;

	if (count == 0)
		panic("host_processors");

	addr = kalloc((vm_size_t) (count * sizeof(mach_port_t)));
	if (addr == 0)
		return KERN_RESOURCE_SHORTAGE;

	tp = (processor_t *) addr;
	for (i = 0; i < NCPUS; i++)
		if (machine_slot[i].is_cpu)
			*tp++ = cpu_to_processor(i);

	*countp = count;
	*processor_list = (mach_port_t *) addr;

	/* do the conversion that Mig should handle */

	tp = (processor_t *) addr;
	for (i = 0; i < count; i++)
		((mach_port_t *) tp)[i] =
		      (mach_port_t)convert_processor_to_port(tp[i]);

	return KERN_SUCCESS;
}

kern_return_t	host_info(
	host_t		host,
	int		flavor,
	host_info_t	info,
	natural_t	*count)
{
	register integer_t	i, *slot_ptr;

	if (host == HOST_NULL)
		return KERN_INVALID_ARGUMENT;
	
	switch(flavor) {

	case HOST_BASIC_INFO:
	    {
		register host_basic_info_t	basic_info;

		/*
		 *	Basic information about this host.
		 */
		if (*count < HOST_BASIC_INFO_COUNT)
			return KERN_FAILURE;

		basic_info = (host_basic_info_t) info;

		basic_info->max_cpus = machine_info.max_cpus;
		basic_info->avail_cpus = machine_info.avail_cpus;
		basic_info->memory_size = machine_info.memory_size;
		basic_info->cpu_type =
			machine_slot[master_processor->slot_num].cpu_type;
		basic_info->cpu_subtype =
			machine_slot[master_processor->slot_num].cpu_subtype;

		*count = HOST_BASIC_INFO_COUNT;
		return KERN_SUCCESS;
	    }

	case HOST_PROCESSOR_SLOTS:
		/*
		 *	Return numbers of slots with active processors
		 *	in them.
		 */
		if (*count < NCPUS)
			return KERN_INVALID_ARGUMENT;

		slot_ptr = (integer_t *)info;
		*count = 0;
		for (i = 0; i < NCPUS; i++) {
			if (machine_slot[i].is_cpu &&
				machine_slot[i].running) {
					*slot_ptr++ = i;
					(*count)++;
				}
		}
		return KERN_SUCCESS;

	case HOST_SCHED_INFO:
	    {
		register host_sched_info_t	sched_info;
		extern int	tick;	/* microseconds per clock tick */
		extern int	min_quantum;
					/* minimum quantum, in microseconds */

		/*
		 *	Return scheduler information.
		 */
		if (*count < HOST_SCHED_INFO_COUNT)
			return(KERN_FAILURE);

		sched_info = (host_sched_info_t) info;

		sched_info->min_timeout = tick / 1000;
		sched_info->min_quantum = min_quantum / 1000;
				/* convert microseconds to milliseconds */

		*count = HOST_SCHED_INFO_COUNT;
		return KERN_SUCCESS;
	    }

	case HOST_LOAD_INFO:
	    {
		register host_load_info_t load_info;
		extern long avenrun[3], mach_factor[3];

		if (*count < HOST_LOAD_INFO_COUNT)
			return KERN_FAILURE;

		load_info = (host_load_info_t) info;

		bcopy((char *) avenrun,
		      (char *) load_info->avenrun,
		      sizeof avenrun);
		bcopy((char *) mach_factor,
		      (char *) load_info->mach_factor,
		      sizeof mach_factor);

		*count = HOST_LOAD_INFO_COUNT;
		return KERN_SUCCESS;
	    }

	default:
		return KERN_INVALID_ARGUMENT;
	}
}

/*
 *	Return kernel version string (more than you ever
 *	wanted to know about what version of the kernel this is).
 */

kern_return_t host_kernel_version(
	host_t			host,
	kernel_version_t	out_version)
{
	extern char	version[];

	if (host == HOST_NULL)
		return KERN_INVALID_ARGUMENT;

	(void) strncpy(out_version, version, sizeof(kernel_version_t));

	return KERN_SUCCESS;
}

/*
 *	host_processor_sets:
 *
 *	List all processor sets on the host.
 */
#if	MACH_HOST
kern_return_t
host_processor_sets(
	host_t				host,
	processor_set_name_array_t	*pset_list,
	natural_t			*count)
{
	unsigned int actual;	/* this many psets */
	processor_set_t pset;
	processor_set_t *psets;
	int i;

	vm_size_t size;
	vm_size_t size_needed;
	vm_offset_t addr;

	if (host == HOST_NULL)
		return KERN_INVALID_ARGUMENT;

	size = 0; addr = 0;

	for (;;) {
		simple_lock(&all_psets_lock);
		actual = all_psets_count;

		/* do we have the memory we need? */

		size_needed = actual * sizeof(mach_port_t);
		if (size_needed <= size)
			break;

		/* unlock and allocate more memory */
		simple_unlock(&all_psets_lock);

		if (size != 0)
			kfree(addr, size);

		assert(size_needed > 0);
		size = size_needed;

		addr = kalloc(size);
		if (addr == 0)
			return KERN_RESOURCE_SHORTAGE;
	}

	/* OK, have memory and the all_psets_lock */

	psets = (processor_set_t *) addr;

	for (i = 0, pset = (processor_set_t) queue_first(&all_psets);
	     i < actual;
	     i++, pset = (processor_set_t) queue_next(&pset->all_psets)) {
		/* take ref for convert_pset_name_to_port */
		pset_reference(pset);
		psets[i] = pset;
	}
	assert(queue_end(&all_psets, (queue_entry_t) pset));

	/* can unlock now that we've got the pset refs */
	simple_unlock(&all_psets_lock);

	/*
	 *	Always have default port.
	 */

	assert(actual > 0);

	/* if we allocated too much, must copy */

	if (size_needed < size) {
		vm_offset_t newaddr;

		newaddr = kalloc(size_needed);
		if (newaddr == 0) {
			for (i = 0; i < actual; i++)
				pset_deallocate(psets[i]);
			kfree(addr, size);
			return KERN_RESOURCE_SHORTAGE;
		}

		bcopy((char *) addr, (char *) newaddr, size_needed);
		kfree(addr, size);
		psets = (processor_set_t *) newaddr;
	}

	*pset_list = (mach_port_t *) psets;
	*count = actual;

	/* do the conversion that Mig should handle */

	for (i = 0; i < actual; i++)
		((mach_port_t *) psets)[i] =
			(mach_port_t)convert_pset_name_to_port(psets[i]);

	return KERN_SUCCESS;
}
#else	/* MACH_HOST */
/*
 *	Only one processor set, the default processor set, in this case.
 */
kern_return_t
host_processor_sets(
	host_t				host,
	processor_set_name_array_t	*pset_list,
	natural_t			*count)
{
	vm_offset_t addr;

	if (host == HOST_NULL)
		return KERN_INVALID_ARGUMENT;

	/*
	 *	Allocate memory.  Can be pageable because it won't be
	 *	touched while holding a lock.
	 */

	addr = kalloc((vm_size_t) sizeof(mach_port_t));
	if (addr == 0)
		return KERN_RESOURCE_SHORTAGE;

	/* take for for convert_pset_name_to_port */
	pset_reference(&default_pset);
	/* do the conversion that Mig should handle */
	*((mach_port_t *) addr) = 
			(mach_port_t) convert_pset_name_to_port(&default_pset);

	*pset_list = (mach_port_t *) addr;
	*count = 1;

	return KERN_SUCCESS;
}
#endif	/* MACH_HOST */

/*
 *	host_processor_set_priv:
 *
 *	Return control port for given processor set.
 */
kern_return_t
host_processor_set_priv(
	host_t		host,
	processor_set_t	pset_name,
	processor_set_t	*pset)
{
    if ((host == HOST_NULL) || (pset_name == PROCESSOR_SET_NULL)) {
	*pset = PROCESSOR_SET_NULL;
	return KERN_INVALID_ARGUMENT;
    }

    *pset = pset_name;
    pset_reference(*pset);
    return KERN_SUCCESS;
}
