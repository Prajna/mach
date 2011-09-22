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
 * $Log:	abios.h,v $
 * Revision 2.3  93/03/11  14:08:44  danner
 * 	u_long -> u_int
 * 	[93/03/09            danner]
 * 
 * 	[93/03/09            danner]
 * 
 * Revision 2.2  93/02/04  07:58:36  danner
 * 	Integrate PS2 code from IBm.
 * 	[93/01/18            prithvi]
 * 
 */

#ifndef _H_ABIOS
#define _H_ABIOS
/*
 * COMPONENT_NAME: INC/SYS abios.h ABIOS Structures and Define's
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
 * Advanced Bios data structures and defines.
 *
 * The structure looks something like this:
 *
 *       +---------------+
 *     	 | Request Block |
 *     	 +---------------+
 *     	 |       *       |
 *     +-+  Logical ID   |
 *     | |       *       |
 *     | +---------------+
 *     |                       +----------------+
 *     |   +-------------------+ Anchor Pointer |
 *     |   |                   +----------------+
 *     |   |  +---------------------------------+
 *     |   +->|      Common Data Area           |
 *     |      +---------------------------------+
 *     |   +--+      Data pointer 0 Offset	|            +-----------------+
 *     |   |  |      Number Logical ID's        |     +----->|  Device Block n |
 *     |   |  |                *                |     |      +-----------------+
 *     +---+->|      Device Block Pointer n     |-----+      |        *        |
 *     +---+--+ Function Transfer Table Pointer |            |   Device Data   |
 *     |   |  |       for logial device n       |            |        *        |
 *     |   |  |                *                |            +-----------------+
 *     |   |  |                *                |            +-----------------+
 *     |   |  |         Data Pointer 'p'        |----------->|  Device Memory  |
 *     |   |  |                *                |            +-----------------+
 *     |   +->|         Data Pointer 0          |
 *     |      +---------------------------------+
 *     |      +---------------------------------+            +-----------------+
 *     +----->|     Function Transfer Table     |            | ABIOS Functions |
 *            +---------------------------------+            +-----------------+
 *            |                *                |            |        *        |
 *            |        Function 1 Pointer       |----------->|   Function 1    |
 *            |        Function 2 Pointer       |----------->|   Function 2    |
 *            |        Function 3 Pointer       |----------->|   Function 3    |
 *            |                *                |            |        *        |
 *            +---------------------------------+            +-----------------+
 *
 * This diagram is from Chapter 2 of the "Advanced BIOS Supplement for the IBM 
 * Personal System/2 and Personal Computer BIOS Interface Technical Reference"
 * P/N 68X2288.
 */

/*
 * unix uses 32 bit addressing model.
 * NOTE: this does not change the size of the data structures.
 * Also note that all pointers MUST be far pointers. "Near pointers"
 * must be declared as u_short.
 */
#define  far
#define BOOTSEG_OFF	0x0
#define	FP_SEG(address)	((0xf8000 & ((u_int)(address)+BOOTSEG_OFF)) >> 4)
#define FP_OFF(address) (0x7fff & ((u_int)(address)+BOOTSEG_OFF))
#define TOFAR(address)	((FP_SEG(address)<<16)|(FP_OFF(address)))

#ifndef MACH
/* MACH will have defined these types in sys/types.h */
typedef	unsigned long	u_int;
typedef	unsigned short	u_short;
typedef	unsigned char	u_char;
#endif

#define BIOS2BYTE(address) (((u_int)(address)*1024)-BOOTSEG_OFF)

/*
 * the function transfer table contains
 * calls to the ABIOS start, interrupt, and timeout
 * routines. Each logical device also has a number of
 * function call routines. The number of this are specified
 * by the function_count field.
 */
struct Function_call	{
	void	(far *x)();
};

struct	Function_transfer_table {
	void	(far *start)();
	void	(far *interrupt)();
	void	(far *timeout)();
	u_short	function_count;
	u_short	Reserved1;
	struct Function_call	fun_call[1]; /* unknown size... */
};
#define	function_call(n)	fun_call[n].x

/*
 * the device block contains internal state information
 * about the device running. All fields fall under 2 catagories:
 * public (read-only areas which the OS may read), and private 
 * (data areas which the OS may never read data format veries between
 * different ABOIS implementations). Only the pulic areas of the structure
 * is defined in this .h file.
 */
struct Port_pairs {
	u_short	start;
	u_short end;
};

struct Device_block {
	u_short	device_block_length;
	u_char	revision;
	u_char	secondary_device_id;	/* additional revision info */
	u_short	logical_id;
	u_short	device_id;
	u_short	count_of_exclusive_ids;
	u_short count_of_common_ids;
	struct	Port_pairs	excl_ports[1]; /* unknown size */
};
#define exclusive_ports(n)	excl_ports[(n)]
#define common_ports(n)		excl_ports[(n)+count_of_exclusive_ids]
/*
 * The common block consists of a 1) header with the pointer to data '0',
 * and the number of logical ids, 2) an array of logical devices, 3) an
 * inverted array of data pointers.
 */
#define MAKE_POINTER(seg,data)	((char far *)((char far *)seg + \
							(u_short)data))
#define ALIGN_SEGMENT(pointer) (((unsigned long)(pointer) +0xf) & ~0xf)
#define ANCHOR_SEG(pointer)	(((unsigned long)(pointer)+BOOTSEG_OFF) >> 4)

struct	Logical_device {
	struct Device_block far	*device_block;
	struct Function_transfer_table far *function_transfer_table;
};

struct Data_pointer {
	u_short		dp_length;
	u_short		dp_offset;
	u_short		dp_segment;
};

struct Common_data_area_head {
	u_short		data_ptr_0;
	u_short		number_logical_ids;
	u_short		Reserved1[2];
	struct	Logical_device	log_device[1]; /* unknown size */
} far *anchor_pointer;

u_short	anchor_seg;

#define ABIOS_HEAD	10


#define logical_device(n)	(anchor_pointer->log_device[(n)-1])
#define datapointer ((struct Data_pointer far *)(&anchor_pointer->log_device \
	[anchor_pointer->number_logical_ids]))
#define data_pointer0 ((struct Data_pointer far *)MAKE_POINTER( \
			         anchor_pointer,anchor_pointer->data_ptr_0))
#define data_pointer(n) (datapointer[(data_pointer0 - &datapointer[n])])

/*
 * Initialization structures.
 */

/*
 * system parameter table. Contains the entry points into ABIOS.
 */
struct System_parameter_table {
	void	(far *common_start)();
	void	(far *common_interrupt)();
	void	(far *common_timeout)();
	u_short	min_stack;
	u_short	Reserved[8];
#ifdef notdef
	void	(far *common_32_start)();
	void	(far *common_32_interrupt)();
	void	(far *common_32_timeout)();
	u_int	Reserved;
#endif
	u_short	number_of_entries;
};

/*
 * system init table. Temparary table used in ABIOS init and thrown away
 */
struct System_init_table {
	u_short		device_id;
	u_short		number_logical_ids;
	u_short		device_block_length;
	u_short		log_device_init[2];
	u_short		request_block_length;
	u_short		function_transfer_table_length;
	u_short		data_pointer_length;
	u_char		secondary_device_id;
	u_char		revision;
	u_short		Reserved[3];
};

/*
 * Request block defines.
 */
/* Function Parametes */
struct Request_header {
	u_short	Current_req_blck_len;	/* IN */
	u_short	Logical_id;		/* IN */
	u_short	Unit;			/* IN */
	u_short Function;		/* IN */
	u_short Request_Block_Flags;	/* IN ABIOS32 */
	u_short ELA_Offset;		/* IN ABIOS32 */
	u_short Return_code;		/* IN/OUT */
	u_short Time_out;		/* OUT */
};
#define r_current_req_blck_len	request_header.Current_req_blck_len
#define r_logical_id		request_header.Logical_id
#define r_unit			request_header.Unit
#define r_function		request_header.Function
#define r_return_code		request_header.Return_code
#define r_time_out		request_header.Time_out

/* service specific info */
struct Logical_id_params {
	u_char	Hardware_intr;		/* OUT */
#define ABIOS_NO_INT	0xff	/* device doesn't interrupt */
#define ABIOS_NMI_INT	0xfe	/* interrupts on the NMI line */
	u_char	Arbitration_level;	/* OUT */
#define ABIOS_NO_ARB	0xff
	u_short	Device_id;		/* OUT */
	u_short	Number_units;		/* OUT */
	u_short	Logical_id_flags;	/* OUT */
#define	OVERLAP_IO	0x08
#define OFFSET_32_BITS	0x04
#define POINTER_TYPES	0x03
#define NO_POINTERS	0x00
#define LOG_POINTER	0x01
#define PHYS_POINTER	0x02
#define BOTH_POINTER	0x03
#define GET_POINTER_TYPE(x) ((x) & POINTER_TYPES)
	u_short	Request_block_length;	/* OUT */
	u_char	Secondary_id;		/* OUT */
	u_char	Revision_level;		/* OUT */
	u_short	Reserved2[2];
}; 
#define	r_hardware_intr		un.logical_id_params.Hardware_intr
#define r_arbitration_level	un.logical_id_params.Arbitration_level
#define r_device_id		un.logical_id_params.Device_id
#define r_number_units		un.logical_id_params.Number_units
#define r_logical_id_flags	un.logical_id_params.Logical_id_flags
#define r_request_block_length	un.logical_id_params.Request_block_length
#define r_secondary_id		un.logical_id_params.Secondary_id
#define r_revision_level	un.logical_id_params.Revision_level
#define r_reserve_1		un.logical_id_params.Reserved2[0]
#define r_reserve_2		un.logical_id_params.Reserved2[1]

#define ABIOS_MIN_REQ_SIZE	0x20

struct	Generic_parameters {
	u_short		reserved;
	u_short		data_pointer_offset;
	u_short		data_pointer_selector;
};

struct	generic_request {
	struct Request_header	request_header;
	union {
		struct	Logical_id_params	logical_id_params;
		struct	Generic_parameters	generic_parameters;
		u_char	_Dummy[ABIOS_MIN_REQ_SIZE];
	} un;
};

#define	g_data_offset	un.generic_parameters.data_pointer_offset
#define	g_data_selector	un.generic_parameters.data_pointer_selector

/* fake 32 bit abios common area */
u_int	abios_common[512];
struct System_parameter_table *system_param_p;



/*
 * Function codes.
 */
#define	ABIOS_DEFAULT_INTERRUPT	0
#define	ABIOS_DEFAULT_INTERUPT	ABIOS_DEFAULT_INTERRUPT
#define	ABIOS_LOGICAL_PARAMETER	1
#define ABIOS_RESERVED_2	2
#define ABIOS_READ_PARAMETER	3
#define ABIOS_WRITE_PARAMETER	4
#define ABIOS_RESET		5
#define ABIOS_ENABLE_INTR	6
#define ABIOS_DISABLE_INTR	7
#define ABIOS_READ		8
#define ABIOS_WRITE		9
#define ABIOS_ADDITIONAL_XFER	0xa

/*
 * return codes, valid only if bit 15 is 0
 */
#define ABIOS_DONE			0x0000
#define ABIOS_STAGE_ON_INT	0x0001
#define ABIOS_STAGE_ON_TIME	0x0002
#define	ABIOS_NOT_MY_INT	0x0004
#define ABIOS_ATTENTION		0x0008
#define ABIOS_INT_RESET		0x0080

/*
 * return codes, valid only if bit 15 is 1
 */
#define ABIOS_FAILED_OP		0x8000
#define ABIOS_BAD_PARAM		0x4000
#define ABIOS_TIME_OUT		0x2000
#define ABIOS_DEV_ERROR		0x1000
#define ABIOS_RETRYABLE		0x0100

/*
 * the following is the undefined return code
 */
#define ABIOS_UNDEFINED		0xffff

/*
 * macros to decipher ABIOS return codes
 */
#define ABIOS_STAGED(x)		((((x) & ABIOS_FAILED_OP)==0) && \
	((x) & (ABIOS_STAGE_ON_INT|ABIOS_STAGE_ON_TIME)))
#define ABIOS_RETRY(x)		((x) & ABIOS_RETRYABLE)

/*
 * the first valid abios lid
 */
#define ABIOS_FIRST_LID	2

/*
 * ABIOS ID's
 */
#define	ABIOS_ID	0	/* ABIOS internal calls */
#define FD_ID		1	/* floppy */
#define HD_ID		2	/* hard disk */
#define VIDEO_ID	3	/* display */
#define	KBD_ID		4	/* keyboard */
#define LP_ID		5	/* parallel port */
#define ASY_ID		6	/* async port */
#define SYSTIME_ID	7	/* system timer */
#define CLOCK_ID	8	/* real time clock */
#define SS_ID		9	/* system services */
#define NMI_ID		0xa	/* non-mask interrupt */
#define	MS_ID		0xb	/* mouse */
#define NVR_ID		0xe	/* Non-volatile ram */
#define DMA_ID		0xf	/* dma */
#define POS_ID		0x10	/* programable option select */
#define KBDS_ID		0x16	/* keyboard security */

/*
 * local abios structure to pass stuff from real mode to protected mode init
 */

struct Abios_header  {
	u_int sys_table;
	u_int common_block;
	u_int end_data;
	u_int sys_conf_param;
	u_int bios_ext_data;
	u_int bios_mem_map;
	u_short logical_ids;
	u_short	cpu_ids;
} *abios_info;

#define	REALTOPHYS(value)	((((value) >> 12) & ~0xf) + ((value) & 0xffff))

struct selector_array {
	u_short selector;
	u_short segment;
};

#endif /* _H_ABIOS */
