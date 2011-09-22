/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS AS-IS
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
 * $Log:	sgmap.c,v $
 * Revision 2.3  93/05/15  19:10:43  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.2  93/03/09  10:48:36  danner
 * 	Created.
 * 	[93/02/18            jeffreyh]
 * 
 */
/*
 *	File: sgmap.c
 * 	Author: Jeffrey Heller, Kubota Pacific Computer 
 *	Date:	11/92
 *
 *	Scater/Gather DMA support.  See FMM 5.0.
 *
 */
/*
 * This code is only for the DEC Flamingo. It might be made to work on
 * some other machine that implements virtual DMA.
 * This first version just divides the sgmap into 8 pieces, each piece
 * corresponds to an active DMA transfer.
 * This allows hopefully enough pages per DMA per busy scsi device. 
 * If other drivers are written that need sgdma then the NSGMAP define 
 * below might need to be changed.
 *
 * If larger DMA's are needed then are possible by dividing up the entries
 * device by device, then code to divide the sgmap using dynamic submaps
 * must replace this code.
 */
#include <alpha/machspl.h>
#include <alpha/pmap.h>
#include <alpha/DEC/flamingo.h>
#include <alpha/DEC/sgmap.h>
#include <mach/alpha/vm_param.h>
#include <kern/assert.h>

#define NSGMAP 16		/* XXX This should be a config option */
#define NULL 0L
/* 
 * This is an array so that I can use an index to get the map.
 * Someday we might want to change this to a real list
 */
struct sgmap {
	struct sgmap	*next;
	unsigned int	index;
	unsigned int	start_entry;
	unsigned int	num_entries;
} sgmap_entry_data[NSGMAP];

typedef struct sgmap *sgmap_t;
sgmap_t	sgmap_entry[NSGMAP];
	
sgmap_t sgmap_pool;	

/*
 * Do any initalization needed 
 */
int sgmap_debug = 0;
sgmap_init()
{
	sgmap_t		map, tmpmap;
	int		x;
	/* 
	 * The system firmware need the use of the 
	 * first 24 slots 
	 */
	int 		start = SGMAP_NUM_SYS_ENTRIES;    

	for (x = 0; x < NSGMAP ; x++) {
		map = sgmap_entry[x] = &sgmap_entry_data[x];
		map->start_entry = start;
		map->num_entries = (SGMAP_NUM_USABLE_ENTRIES / NSGMAP);
		map->next = sgmap_pool;
		map->index = x + 1;
		sgmap_pool = map; /* Put this at start of list */
		if (sgmap_debug)
			printf("sgmap init: x=%x start_entry = %x num_entries = %x next = %x\n",x,map->start_entry, map->num_entries, map->next);

		start += (SGMAP_NUM_USABLE_ENTRIES / NSGMAP) +1;
			
	}
	/* We might as well use any uneven entries, last is as good as first */
	map->num_entries += SGMAP_NUM_USABLE_ENTRIES % NSGMAP;

}

unsigned int
sgmap_grab_entry()
{
	sgmap_t		map;
	spl_t 		s = splhigh();
	/*XXX Grab a lock to protect list on an mp */

	assert (sgmap_pool != NULL);
	/*XXX Assers not on right now */
	if (!sgmap_pool){
		printf("sgmap_grab_entry: No maps! \n");
		gimmeabreak();
		splx(s);
		return 1000 + NSGMAP;
	}

	map = sgmap_pool;
	sgmap_pool = map->next;
	if (sgmap_debug)
		printf("sgmap_grab_entry %x \n",map->index);
	splx(s);
	return (map->index);
}

sgmap_free_entry(index)
	unsigned int index;
{
	sgmap_t		map;
	spl_t 		s = splhigh();

	/*XXX Grab a lock to protect list on an mp */
	if (sgmap_debug )
		printf("sgmap_free_entry %x \n",index);
	map = sgmap_entry[index - 1];
	map->next = sgmap_pool;
	sgmap_pool = map;
	splx(s);
	return;
		
}

unsigned int
sgmap_load_map(addr, count, index)
	vm_offset_t	addr;
	unsigned int	count;
	unsigned int	index;
{
	sgmap_t		map;
	vm_offset_t	paddr;
	unsigned long	*sgmap;
	int		x, npages, offset;
	
	assert( index < (NSGMAP+ 1));
	/* XXX If asserts are not on double check for now */
	if (index > (NSGMAP+ 1)){
		printf("sgmap_load_map: index out of bounds ");
		gimmeabreak();
	}
		
	sgmap = (unsigned long *)PHYS_TO_K0SEG(KN15AA_REG_SGMAP);
	map = sgmap_entry[index - 1];
	sgmap += map->start_entry; /* Index from the first entry in the
				    * map to the start of our part
				    * of the map */

	offset = addr & ALPHA_OFFMASK;
	npages = (alpha_btop(count+ offset + ALPHA_OFFMASK));
	if (sgmap_debug){
		printf("sgmap_load_map: addr = %x count = %x  npages = %x\n",
		       addr, count, npages);
		printf("sgmap_load_map: index = %x sgmap = %x start_entry = %x \n", index, sgmap, map->start_entry);
	}
	/*
	 * Time to fill in the map
	 */
	if (npages > map->num_entries)
		panic("sgmap_load_map: request size bigger then map");
	for (x = 0; x < npages; x++) {
		unsigned int sgval;

		paddr = kvtophys (addr);
		if (paddr == NULL){
#if 0
printf("sgmap_load_map: about to panic, addr = %x paddr = %x kv = %x \n",addr,paddr, kvtophys (addr));
#endif
			gimmeabreak();
/*			panic ("sgmap_load_map: bad address\n");*/
		}
		sgval = (unsigned int)(((paddr & SGMAP_ENTRY_MASK)>> 9) | SGMAP_ENTRY_VALID);
		if (sgmap_debug) printf("fill_map: sgmap=%x sgval =%x \n",
					(unsigned int *) sgmap, sgval);
		*(unsigned int *)sgmap =sgval;
		wbflush();
		sgmap++;
		addr += ALPHA_PGBYTES;
	}
	
	*(unsigned int *)sgmap = 0; /* Mark the next page as invalid */
	wbflush();
	if (sgmap_debug)
		printf("sgmap_load_map: Virtual addr = %x\n",(map->start_entry << 13) + offset);
	return ((map->start_entry << 13) + offset);
}




