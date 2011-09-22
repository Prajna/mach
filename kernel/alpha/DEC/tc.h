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
 * $Log:	tc.h,v $
 * Revision 2.4  93/05/10  22:19:12  rvb
 * 	Fixed TC probing.
 * 	[93/05/07  14:46:41  af]
 * 
 * 	Created, from the DEC specs:
 * 	"TURBOchannel Hardware Specification"
 * 	EK-369AA-OD-005, Version 005, July 1990
 * 	[90/09/03            af]
 * 
 * Revision 2.3  93/05/10  20:07:11  rvb
 * 	Fixed TC probing.
 * 	[93/05/07  14:46:41  af]
 * 
 * 	Created, from the DEC specs:
 * 	"TURBOchannel Hardware Specification"
 * 	EK-369AA-OD-005, Version 005, July 1990
 * 	[90/09/03            af]
 * 
 * Revision 2.2  93/03/09  10:48:54  danner
 * 	Jeffrey Heller created this from my mips code.
 * 	[93/03/06  14:28:19  af]
 * 
 * Revision 2.8  92/02/19  16:46:42  elf
 * 	Separated notion of physical and logical TC slots.
 * 	Added ANSI C function prototypes.
 * 	[92/01/17            af]
 * 
 * Revision 2.7  91/08/24  12:22:11  af
 * 	Defined max number of TC slots, arbitrary TC interrupt
 * 	enable function.
 * 	[91/08/02  03:34:38  af]
 * 
 * Revision 2.6  91/05/14  17:31:56  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:46:48  mrt
 * 	Added author notices
 * 	[91/02/04  11:20:40  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:19:35  mrt]
 * 
 * Revision 2.4  90/12/05  23:36:07  af
 * 
 * 
 */
/*
 *	File: tc.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	Definitions for the TURBOchannel BUS.
 */

/*
 * Max conceivable number of slots on the TC
 */
#define	TC_MAX_SLOTS		8

#define	TC_MAX_LOGICAL_SLOTS	15

/*
 * Address map specifications for any TC option
 * These are offsets from the option's base address.
 * Assumes dense space addressing, where applicable.
 */

#define TC_OFF_ROM		0x000003e0	/* required ROM info */
#define TC_OFF_PROTO_ROM	0x003c03e0	/* 'obsolete' alternate */

#define TC_ROM_TEST_DATA_SIZE	16
#define TC_ROM_SLEN		4
#define TC_ROM_LLEN		8

typedef struct {
	unsigned char	value;
	char		pad[3];
} tc_padded_char_t;


typedef struct {
	tc_padded_char_t	rom_width;	/* legal: 1 2 4 */
	tc_padded_char_t	rom_stride;	/* legal: 4 */
	tc_padded_char_t	rom_size;	/* legal: 0-255, unit: 8kb */
	tc_padded_char_t	slot_size;	/* legal: 1-128, unit: 4Mb */
	unsigned char		test_data[TC_ROM_TEST_DATA_SIZE];
						/* must always contain:
						/* x55 x00 xaa xff
						/* (each byte is repeated
						/*  rom_stride times) */
	tc_padded_char_t	firmware_rev[TC_ROM_LLEN];
	tc_padded_char_t	vendor_name[TC_ROM_LLEN];
	tc_padded_char_t	module_name[TC_ROM_LLEN];
	tc_padded_char_t	host_firmware_type[TC_ROM_SLEN];
} tc_rommap_t;

typedef struct {
	unsigned char	present;			/* and do we handle it */
	unsigned char	slot_size;			/* how many TC slots */
	unsigned char	rom_width;			/* bytewide or.. */
	unsigned char	isa_ctlr;			/* ..such as the scsi */
	unsigned char	unit;				/* instance id */
	char		module_name[TC_ROM_LLEN+1];	/* ROM name */
	char		module_id[TC_ROM_LLEN * 2+1];	/* vendor and rev */
	vm_offset_t	k1seg_address;			/* TC starting address */
	char		*driver_name;			/* software name */
	int		(*intr)();			/* interrupt routine */
	struct bus_driver	*driver;		/* config info ?? */
} tc_option_t;


#ifdef	KERNEL
extern	tc_option_t	tc_slot_info [TC_MAX_LOGICAL_SLOTS];
extern	vm_offset_t	tc_slot_phys_base [TC_MAX_SLOTS];
extern	short		tc_max_slot, tc_min_slot;

extern	void (*tc_slot_hand_fill) ( tc_option_t *all_slots );
extern	vm_size_t  (*tc_enable_interrupt) ( int slotno, boolean_t turn_on, int xx );

extern	void		tc_find_all_options ();
extern	vm_offset_t	tc_probe ( char *driver_name );
extern  int		tc_intr();
extern	void		tc_autoconf();
#endif	KERNEL
