/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * Copyright (c) 1991 IBM Corporation 
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation,
 * and that the name IBM not be used in advertising or publicity 
 * pertaining to distribution of the software without specific, written
 * prior permission.
 * 
 * CARNEGIE MELLON AND IBM ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON AND IBM DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	abios.c,v $
 * Revision 2.3  93/03/11  14:08:38  danner
 * 	u_long -> u_int
 * 	[93/03/09            danner]
 * 
 * 	[93/03/09            danner]
 * 
 * Revision 2.2  93/02/04  07:58:30  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */

#include <sys/types.h>

#include <mach/i386/vm_types.h>
#include <mach/i386/vm_param.h>
#include <i386/seg.h>
#include <i386ps2/abios.h>

/*
 * We must adjust the bottom of the memory hole here.
 * cnvmem doesn't count all the memory that ABIOS uses.
 */
extern vm_offset_t      hole_start;

#define MAX_SIZE	0xffff
#define BAD_DESCRIPT	0xffff
#define PTOV(x) 	(phystokv(x))

/*
 * DEBUG must be defined if ABIOS_DEBUG is (get_dev is called from test_calls)
 */
#define ABIOS_DEBUG 1

#if defined(ABIOS_DEBUG) && !defined(DEBUG)
#define DEBUG 1
#endif

#define MAXSEL 20
static struct selector_array	code_array[MAXSEL] = {0};
static struct selector_array	data_array[MAXSEL] = {0};
extern struct fake_descriptor	gdt[];
struct Abios_header *abios_info = 0;

char *make_32();
u_short allocate_gdt();

#ifdef DEBUG
#define DEBUGF(x,y)	if (x) y
char *get_dev();

int abiosdebug = 0;
#else
#define DEBUGF(x,y)
#endif

static struct real_descriptor   *gdt_hint
		= (struct real_descriptor *) &gdt[ABIOS_FIRST_AVAIL_SEL];
                                        /* a hint for allocate_gdt */

/* put the variables referenced in init_ps2 in data */

abios_init()
{
	struct Abios_header *abhp = abios_info;
	int	rc,i;
	int	data_count;

	DEBUGF(abiosdebug,printf("abhp = %x\n",abhp));
	if (abhp == 0) {
		printf("ABIOS not found. Not initialized!\n");
		return;
	}
	abhp = (struct Abios_header *) PTOV(abhp);

	/* map the abios data area into virtual memory */
	system_param_p = (struct System_parameter_table *)
			PTOV(abhp->sys_table);

	/* build addresses to get to the abios common data area */
	anchor_pointer = (struct Common_data_area_head *)
					PTOV(abhp->common_block);

	if ((anchor_seg = allocate_gdt()) == BAD_DESCRIPT) {
		printf("out of gdt for anchor_seg\n");
		return(-1);
	} 
	make_gdt_desc(anchor_seg,
		      (unsigned int) anchor_pointer,
		      anchor_pointer->data_ptr_0+7,
		      ACC_DATA_W,
		      0);


	/* copy to the fake table before fixups.. */
	bcopy(anchor_pointer,abios_common,anchor_pointer->data_ptr_0+8);

	/*
	 * now fix up the logical device pointers
	 */
	for (i=0; i < anchor_pointer->number_logical_ids; i++) {
		fixup(&logical_device(i+1),i+1);
	}

	/*
	 * fix up the data pointers
	 */
	data_count = *(u_short *)(data_pointer0 + 1);
	for (i=0 ; i < data_count; i++) {
		struct Data_pointer *dp = &data_pointer(i);

		dp->dp_segment = (dp->dp_offset >> 4) + (dp->dp_segment << 12);
		dp->dp_offset &= 0xf;
		abios_convert(data_array, &(dp->dp_offset),
			      ACC_DATA_W, dp->dp_length);
	}

	/*
	 * build the fake anchor_pointer table
	 */
	anchor_pointer = (struct Common_data_area_head *) abios_common;
	for (i=0; i < anchor_pointer->number_logical_ids; i++) {
		logical_device(i+1).device_block = (struct Device_block *)
				make_32(logical_device(i+1).device_block);
		logical_device(i+1).function_transfer_table = 
		     (struct Function_transfer_table *)
			make_32(logical_device(i+1).function_transfer_table);
	}

	/*
	 * fixup the common entry point stuff
	 */
	abios_convert(code_array,&(system_param_p->common_start),
		      ACC_CODE_R, MAX_SIZE);
	abios_convert(code_array,&(system_param_p->common_interrupt),
		      ACC_CODE_R, MAX_SIZE);
	abios_convert(code_array,&(system_param_p->common_timeout),
		      ACC_CODE_R, MAX_SIZE);
	return(0);
}

abios_common_start(request,flags)
	struct generic_request *request;
	int	flags;
{
	abios_xlate(system_param_p->common_start,request,flags);
}

abios_common_interrupt(request,flags)
	struct generic_request *request;
	int	flags;
{
	abios_xlate(system_param_p->common_interrupt,request,flags);
}

abios_common_timeout(request,flags)
	struct generic_request *request;
	int	flags;
{
	abios_xlate(system_param_p->common_timeout,request,flags);
}

abios_next_LID(dev_id,last_LID)
	int	last_LID;
{
	int	i;
	struct Device_block *db;

	if (abios_info == 0)
		return(0);	/* no abios - no LIDs */

	for (i=last_LID+1; i <= anchor_pointer->number_logical_ids; i++) {
		if (db = logical_device(i).device_block) {
			if (db->device_id == dev_id) {
				return(i);
			}
		}
	}
	/* no LID found */
	return(0);
}
	
abios_xlate(abios_routine,request,flags)
	char	*abios_routine;
	struct	generic_request *request;
	int	flags;
{
	u_short	selector;
	u_int	req_addr;
	u_int	data_addr;
	u_int	save_data = 0;
	int	i,s;

	selector = allocate_gdt();
	if (selector == 0) {
		panic("abios_xlate: out of selectors");
	}
	make_gdt_desc(selector,
		      (unsigned int) request,
		      request->r_current_req_blck_len,
		      ACC_DATA_W,
		      0);

	req_addr = selector << 16;
	if ((flags & LOG_POINTER) && ((request->r_function == ABIOS_READ) ||
		 (request->r_function == ABIOS_WRITE) ||
			 (request->r_function == ABIOS_ADDITIONAL_XFER))) {
		u_short data_select = allocate_gdt();
		if (data_select == 0) {
			panic("abios_xlate: out of selectors");
		}
		data_addr = *((u_int *)&request->g_data_offset);
		make_gdt_desc(data_select,
			      data_addr,
			      MAX_SIZE,
			      ACC_DATA_W,
			      0);
		request->g_data_offset = 0;
		request->g_data_selector = data_select;
		save_data++;
	}
		
	req_addr = selector << 16;

	abios_call(0,0,req_addr,anchor_seg,abios_routine,0,0,0,0);

	if (save_data) {
		free_gdt(request->g_data_selector);
		*(u_int *)&request->g_data_offset = data_addr;
	}
	free_gdt(selector);
}

static int	abios_last_addr = 0;
static struct Function_transfer_table *abios_ftt_array[50];

fixup(device,LID)
	struct Logical_device *device;
	int	LID;
{
	struct Function_transfer_table *ftt =
		(struct Function_transfer_table *)make_32
					(device->function_transfer_table);
	struct Device_block *db = (struct Device_block *)
					make_32(device->device_block);
	int i;
	int done;

	DEBUGF(abiosdebug,printf("LID %x",LID));
	if (ftt) {
		DEBUGF(abiosdebug,printf(", %d functions, ftt=%x",ftt->function_count,ftt));
		done = 0;
		for (i=0; i < abios_last_addr; i++) {
			if (ftt == abios_ftt_array[i]) {
				done = 1;
				DEBUGF(abiosdebug,printf(" (fixups already done)"));
				break;
			}
		}
		if (!done) {
		 abios_ftt_array[abios_last_addr++] = ftt;
		 abios_convert(code_array,&(ftt->start),
			       ACC_CODE_R, MAX_SIZE);
		 abios_convert(code_array,&(ftt->interrupt),
			       ACC_CODE_R, MAX_SIZE);
		 abios_convert(code_array,&(ftt->timeout),
			       ACC_CODE_R, MAX_SIZE);
		
		 /*
		  * first fix up the FTT
		  */
		 for (i=0; i < ftt->function_count; i++) {
			abios_convert(code_array,&(ftt->function_call(i)),
				      ACC_CODE_R, MAX_SIZE);
		 }
		}
		abios_convert(data_array,&(device->function_transfer_table),
			      ACC_DATA_W, MAX_SIZE);
	}
	if (device->device_block) {
		DEBUGF(abiosdebug,printf(", %s",get_dev(db->device_id)));
		abios_convert(data_array,&(device->device_block),
			      ACC_DATA_W, MAX_SIZE);
	}
	DEBUGF(abiosdebug,printf("\n"));
}

struct far_pointer {
	u_short	offset;
	u_short selector;
};

abios_convert(array,far_ptr,type,size)
	struct	selector_array *array;
	struct  far_pointer *far_ptr;
{
	if (*(unsigned long *)far_ptr) {
		far_ptr->selector = 
				get_selector(array,far_ptr->selector,type,size);
	}
}

get_selector(array,segment,type,size)
	struct selector_array *array;
	u_short	segment;
{
        vm_offset_t addr;
	int	count = 0;

	while (array->selector) {
		if (array->segment == segment) {
			return(array->selector);
		}
		array++;
		count++;
	}
	if (count == MAXSEL) {
		panic(" over allocated array\n");
	}
	array->selector = allocate_gdt();
	array->segment = segment;
	if (array->selector == 0) {
		panic(" out of selectors\n");
	}
	addr = (vm_offset_t) make_32(segment << 16);
	make_gdt_desc(array->selector,
		      addr,
		      size,
		      type,
		      0);

        /*
	 * Adjust hole_start to be below the lowest segment
	 * mapped here.  But don`t count segments below 0x1000 -
	 * we don`t use that for paging anyway.  See i386_init.
	 *
	 * Addr is a virtual address; hole_start is physical.
	 */
	addr = kvtophys(addr);
	if (addr >= 0x1000) {
	    if (hole_start > addr)
	        hole_start = addr;
	}
	return(array->selector);
}

/*
 * make_32 translates a dos real mode pointer (16 bit segment/16 bit offset)
 *  into a 386 flat mode paged vm pointer (32 bits).
 *  Called from several abios_init routines.
 */
char *
make_32(value)
	u_int value;
{
	u_int phys = REALTOPHYS(value);
	return(phys?(char *)PTOV(phys):(char *)0);
}

void display_request(rb_ptr)
struct generic_request *rb_ptr;
{

	printf("The address of request block = %x\n\n",rb_ptr);
	printf("The function = %d\n",rb_ptr->r_function);
	printf("Current_req_block_len = %x\n",rb_ptr->r_current_req_blck_len);
	printf("Logical_id = %x\n",rb_ptr->r_logical_id);
	printf("Device ID = %x\n\n",rb_ptr->r_device_id);
	printf("Unit = %x\n",rb_ptr->r_unit);
	printf("Return_code = %x\n",rb_ptr->r_return_code);
	printf("Time_out = %x\n",rb_ptr->r_time_out);
}



#ifdef DEBUG

/*
 * print the ascii string for the abios device type found
 */
static char *device_name[] = {
	"ABIOS internal call",	/* 0x00 */
	"diskette",		/* 0x01 */
	"disk",			/* 0x02 */
	"video",		/* 0x03 */
	"keyboard",		/* 0x04 */
	"parallel port",	/* 0x05 */
	"async port",		/* 0x06 */
	"system timer",		/* 0x07 */
	"real-time clock",	/* 0x08 */
	"system services",	/* 0x09 */
	"NMI",			/* 0x0a */
	"mouse",		/* 0x0b */
	"reserved (0x0c)",	/* 0x0c */
	"reserved (0x0d)",	/* 0x0d */
	"NVRAM",		/* 0x0e */
	"DMA",			/* 0x0f */
	"POS",			/* 0x10 */
	"reserved (0x11)",	/* 0x11 */
	"reserved (0x12)",	/* 0x12 */
	"reserved (0x13)",	/* 0x13 */
	"reserved (0x14)",	/* 0x14 */
	"reserved (0x15)",	/* 0x15 */
	"keyboard security",	/* 0x16 */
	"reserved (0x17)",	/* 0x17 */
};

static int known_device_ids=(sizeof(device_name)/sizeof(device_name[0]));

char *
get_dev(type)
	u_int	type;
{
	if (type >= known_device_ids) {
		static char buffer[] = "(unknown 0x--)";
		static char hex[] = "0123456789ABCDEF";
		buffer[11] = hex[(type>>4)&0xf];
		buffer[12] = hex[type&0xf];
		return(buffer);
	}

	return(device_name[type]);
}
#endif /* DEBUG */

#ifdef ABIOS_DEBUG

/* the following code is for testing only!!! */
static char *
map_irq (irq)
	u_char	irq;
{
	static char	buffer[30] = "irq00";
	if (irq == 0xff) {
		return("NoInt");
	}
	if (irq == 0xfe) {
		return("NMI  ");
	}
	buffer[3] = irq/10 + '0';
	buffer[4] = irq%10 + '0';
	return(buffer);
}

static char *
map_arb (arb)
	u_char	arb;
{
	static char	buffer[30] = "arb00 ";
	if (arb == 0xff) {
		return("NoArb");
	}
	buffer[3] = arb/10 + '0';
	buffer[4] = arb%10 + '0';
	return(buffer);
}

test_call()
{
	int	i;
	struct	generic_request	req;


	/* LID 1 is a dummy, LID 2 would call cause 'infinite' recursion..*/
	for (i=2; i <= anchor_pointer->number_logical_ids; i++) {
		int	dev = ABIOS_ID;

		if (logical_device(i).device_block) {
			dev = logical_device(i).device_block->device_id;
		}

		if (dev != ABIOS_ID) {
			req.r_current_req_blck_len = 
						sizeof(struct generic_request);
			req.r_logical_id = i;
			req.r_unit = 0;
			req.r_function = ABIOS_LOGICAL_PARAMETER;
			req.r_return_code = ABIOS_UNDEFINED;
			abios_common_start(&req,0);
		} else {
			req.r_hardware_intr = 0xff;
			req.r_arbitration_level = 0xff;
			req.r_number_units = 0;
			req.r_request_block_length =
					 sizeof(struct generic_request);
			req.r_logical_id_flags = 0;
			req.r_return_code = 0;
		}
		printf("device %02x LID %02x",dev,i);
		if (req.r_return_code == 0) {
			printf(" %s %s units=%d size=%3d flags=%04x",
			 map_irq(req.r_hardware_intr), 
			 map_arb(req.r_arbitration_level),req.r_number_units,
			    req.r_request_block_length,req.r_logical_id_flags);
		} else {
			printf("ERROR, rc=0x%04x",req.r_return_code);
		} 
		printf(": %s\n",get_dev(dev));
	}
}
#endif /* ABIOS_DEBUG */


int_on_ABIOS()
{
  panic("interrupt on ABIOS stack");
  /* NOTREACHED */
}

trap_on_ABIOS()
{
  panic("trap on ABIOS stack");
  /* NOTREACHED */
}

