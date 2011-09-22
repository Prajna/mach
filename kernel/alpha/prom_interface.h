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
 * $Log:	prom_interface.h,v $
 * Revision 2.4  93/08/10  15:15:37  mrt
 * 	"..all the systems implemented the fields backwards, so that
 * 	" MAJOR and MINOR were switched.  The SRM was changed to match.."
 * 	Consequently, we switched our definition in the struct per_cpu_slot.
 * 	[93/08/06            af]
 * 
 * Revision 2.3  93/03/09  10:50:46  danner
 * 	Changed prom dispatching to be more link-safe.
 * 	[93/02/20            af]
 * 
 * Revision 2.2  93/02/05  07:59:57  danner
 * 	Working version, taken from boot directory.
 * 	[93/02/04  00:57:09  af]
 * 
 * 	Added reference to doc for the HWRPB &co.
 * 	[92/12/22            af]
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:17:51  af]
 * 
 * 	Created.
 * 	[92/06/02            af]
 * 
 */
/*
 *	File: prom_interface.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Functions and data structures that link the kernel
 *	to the prom environment.
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

/*
 * In the MI code we only need halt, reboot, putchar, getenv
 */
#ifndef	ASSEMBLER

/*
 * This is set up in init_prom_interface
 */
extern int alpha_console;

extern struct {
    int	(*routine)();
    struct console_routine_descriptor *routine_arg;
} prom_dispatch_v;


void	prom_halt();
void	prom_reboot();
integer_t	prom_putchar( char );
char	*prom_getenv( char*);

/*
 * The (complicated) return value from a prom call
 */
typedef union {
    struct {
	unsigned long
		retval	: 32,
		unit	: 8,
		mbz	: 8,
		error	: 13,
		status	: 3;
    } u;
    long bits;
} prom_return_t;


#endif	ASSEMBLER

/*
 * Callback codes
 */
#define	PROM_R_GETC		0x1
#define	PROM_R_PUTS		0x2	/* puts to console */
#define	PROM_R_SETENV		0x20	/* for reboot */
#define	PROM_R_GETENV		0x22


/*
 * What you can do with it
 */

	/* gets a character from console device no. X
	   ok status is 0 or 1 */
#define	prom_getc(x)		prom_dispatch( PROM_R_GETC, x)

	/* Print string Y of length Z on console no X
	   ok status is 0 or 1 */
#define	prom_puts(x,y,z)	prom_dispatch( PROM_R_PUTS, x, y, z)

	/* Copy environment variable X in buffer Y of length Z
	   ok status is 0 or 1 */
#define	prom_getenv(x,y,z)	prom_dispatch( PROM_R_GETENV, x, y, z)

	/* Change value of environment variable X to value Y of length Z
	   ok status is 0 */
#define	prom_setenv(x,y,z)	prom_dispatch( PROM_R_SETENV, x, y, z)

/*
 * Which of the mandatory environment variables we need
 */

#define	PROM_E_AUTO_ACTION	0x1	/* for reboot ? */
#define	PROM_E_BOOTED_DEV	0x4
#define	PROM_E_BOOTED_OSFLAGS	0x8
#define	PROM_E_TTY_DEV		0xf


/*
 * Restart block -- monitor support for "warm" starts
 */

#define	RESTART_ADDR	0x10000000	/* prom restart block (virtual, at boot) */
#define	RESTART_CSUMCNT	0xc8		/* chksum this many bytes, as longs */

#ifndef	ASSEMBLER

struct restart_blk {
	vm_offset_t	my_phys_address;
	char		my_name[8];		/* "HWRPB" (magic number) */
	natural_t	my_version;
	vm_size_t	my_size;
	natural_t	primary_cpu_id;
	vm_size_t	page_size;
	natural_t	valid_phys_bits;
	natural_t	maximum_asn;
	char		system_serial_number[16];
	natural_t	system_type;
	natural_t	system_variation;
	char		system_revision[8];	/* first 4 valid */
	vm_size_t	clock_interrupt_frequency;
	vm_size_t	cycle_counter_resolution;
	vm_offset_t	virtual_pte_base;
	integer_t	reserved[1];
	vm_offset_t	tb_hint_block_offset;
	natural_t	num_processors;
	vm_size_t	percpu_slot_size;
	vm_offset_t	percpu_slots_offset;
	vm_size_t	ctb_count;	/* 'console terminal block' */
	vm_size_t	ctb_size;
	vm_offset_t	ctb_offset;
	vm_offset_t	console_routine_block_offset;
	vm_offset_t	memory_data_descriptor_table_offset;
	vm_offset_t	config_data_block_offset;
	vm_offset_t	FRU_table_offset;
	integer_t	(*save_term_routine)();
	integer_t	save_term_routine_pv;	/* procedure value */
	integer_t	(*restore_term_routine)();
	integer_t	restore_term_routine_pv;
	integer_t	(*restart_routine)();
	integer_t	restart_routine_pv;
	integer_t	reserved_for_os;
	integer_t	reserved_for_hw;
	integer_t	checksum;
	integer_t	ready_bitmasks[2];	/* VARSIZE */
};

#ifdef	KERNEL
extern struct restart_blk	*alpha_hwrpb;
#endif	/* KERNEL */

/*
 * Defined system types
 */
#define	SYSTEM_TYPE_ADU		1
#define	SYSTEM_TYPE_COBRA	2
#define	SYSTEM_TYPE_RUBY	3
#define	SYSTEM_TYPE_FLAMINGO	4
#define	SYSTEM_TYPE_MANNEQUIN	5
#define	SYSTEM_TYPE_JENSEN	6

/*
 * System variation bitfields
 */

#define	SYSTEM_VAR_MPCAP	0x1	/* isa multiprocessor */

#define	SYSTEM_VAR_CONSOLE	0x1e	/* what sort of console hw */
#	define	SYSTEM_VAR_CNSL_DETACHED	0x2
#	define	SYSTEM_VAR_CNSL_EMBEDDED	0x4

#define	SYSTEM_VAR_POWERFAIL	0xe0	/* powerfail provisions */
#	define	SYSTEM_VAR_PF_UNITED		0x20
#	define	SYSTEM_VAR_PF_SEPARATE		0x40
#	define	SYSTEM_VAR_PF_BBACKUP		0x60

#define	SYSTEM_VAR_PF_ACTION	0x100	/* 1 -> restart all processors
					   on powerfail, 0 -> only primary */
#define	SYSTEM_VAR_GRAPHICS	0x200	/* do we have a graphic engine */
#define	SYSTEM_VAR_mbz		0xfffffffffffffc00

struct console_routine_descriptor {
	integer_t	descriptor;
	int		(*code)();
};

struct console_routine_blk {

	struct console_routine_descriptor
			*dispatch_func_desc;
	vm_offset_t	dispatch_func_phys;

	integer_t	other_stuff[1];		/* which we do not care */
};

struct memory_data_descriptor_table {
	integer_t	checksum;
	vm_offset_t	implementation_specific_table_address;	/* phys */
	vm_size_t	num_clusters;
	struct  mem_cluster {
		vm_offset_t	first_pfn;
		vm_size_t	num_pfn;
		vm_size_t	num_tested;
		vm_offset_t	bitmap_v_address;
		vm_offset_t	bitmap_p_address;
		integer_t	checksum;
		integer_t	usage;
	} mem_clusters[1];
};

struct per_cpu_slot {
	char		hwpcb[128];	/* pal-dep */
	natural_t	state_flags;
	vm_size_t	palcode_memsize;
	vm_size_t	palcode_scratchsize;
	vm_offset_t	palcode_memory;
	vm_offset_t	palcode_scratch;
	natural_t	palcode_revision_info;
	unsigned int	processor_major_type;
	unsigned int	processor_minor_type;
	natural_t	processor_variation;
	char		processor_revision[8];	/* first 4 valid */
	char		processor_serial_number[16]; /* first 10 valid */
	vm_offset_t	logout_area;
	vm_size_t	logout_area_length;
	vm_offset_t	halt_pcbb;		/* phys of PCB at halt */
	vm_offset_t	halt_pc;
	natural_t	halt_ps;
	natural_t	halt_r25;
	natural_t	halt_r26;
	natural_t	halt_r27;
	natural_t	halt_reason;
	natural_t	reserved_sw;
	char		mp_console_area[168];		/* +296d */
	char		architecture_specific[48];	/* +464d */
	/* Total size 512 bytes minimum, trust hwrpb->slot_size */
};

/* State flags */

#define	PSTATE_BIP		0x1	/* boot in progress */
#define	PSTATE_RC		0x2	/* restart capable */
#define	PSTATE_PA		0x4	/* processor available */
#define	PSTATE_PP		0x8	/* processor present */
#define	PSTATE_OH		0x10	/* operator halted */
#define	PSTATE_CV		0x20	/* context valid */
#define	PSTATE_PV		0x40	/* palcode valid */
#define	PSTATE_PMV		0x80	/* palcode memory valid */
#define	PSTATE_PL		0x100	/* palcode loaded */
#define	PSTATE_HALT_REQ		0xff0000
#	define PSTATE_H_DEFAULT		0x000000
#	define PSTATE_H_SAVE_EXIT	0x010000
#	define PSTATE_H_COLD_BOOT	0x020000
#	define PSTATE_H_WARM_BOOT	0x030000
#	define PSTATE_H_STAY_HALTED	0x040000
#define	PSTATE_mbz		0xffffffffff00fe00

/* Halt reasons */

#define	PHALT_START		0
#define	PHALT_SYSCRASH		1
#define	PHALT_KSTACK_INVALID	2
#define	PHALT_SCBB_INVALID	3
#define	PHALT_PTBR_INVALID	4
#define	PHALT_FROM_KERNEL	5
#define	PHALT_DOUBLE_ABORT	6

#endif ASSEMBLER




