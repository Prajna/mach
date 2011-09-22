/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 * $Log:	gdt.c,v $
 * Revision 2.8  93/02/04  07:56:04  danner
 * 	Merge in PS2 support.
 * 	[92/02/22            dbg@ibm]
 * 
 * Revision 2.7  92/01/03  20:05:58  dbg
 * 	Add entries for user LDT, floating-point register
 * 	access from emulator, and floating-point emulator code.
 * 	[91/10/18            dbg]
 * 
 * Revision 2.6  91/05/14  16:08:04  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/05/08  12:31:41  dbg
 * 	Collapsed GDT again.
 * 	[91/04/26  14:34:34  dbg]
 * 
 * Revision 2.4  91/02/05  17:11:52  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:34:15  mrt]
 * 
 * Revision 2.3  90/08/27  21:56:36  dbg
 * 	Collapsed GDT.  Use new segmentation definitions.
 * 	[90/07/25            dbg]
 * 
 * Revision 2.2  90/05/03  15:27:29  dbg
 * 	Created.
 * 	[90/02/15            dbg]
 * 
 */

/*
 * Global descriptor table.
 */
#include <platforms.h>

#include <i386/seg.h>
#include <i386/tss.h>
#include <mach/i386/vm_types.h>
#include <mach/i386/vm_param.h>

extern struct fake_descriptor	ldt[];
extern struct i386_tss		ktss;

#if     PS2
extern unsigned long abios_int_return;
extern unsigned long abios_th_return;
extern char intstack[];
#endif  /* PS2 */

struct fake_descriptor gdt[GDTSZ] = {
/* 0x000 */	{ 0, 0, 0, 0 },		/* always NULL */
/* 0x008 */	{ VM_MIN_ADDRESS,
		  (VM_MAX_KERNEL_ADDRESS-1-VM_MIN_ADDRESS)>>12,
		  SZ_32|SZ_G,
		  ACC_P|ACC_PL_K|ACC_CODE_R
		},			/* kernel code */
/* 0x010 */	{ VM_MIN_ADDRESS,
		  (VM_MAX_KERNEL_ADDRESS-1-VM_MIN_ADDRESS)>>12,
		  SZ_32|SZ_G,
		  ACC_P|ACC_PL_K|ACC_DATA_W
		},			/* kernel data */
/* 0x018 */	{ (unsigned int)ldt,
		  LDTSZ*sizeof(struct fake_descriptor)-1,
		  0,
		  ACC_P|ACC_PL_K|ACC_LDT
		},			/* local descriptor table */
/* 0x020 */	{ (unsigned int)&ktss,
		  sizeof(struct i386_tss),
		  0,
		  ACC_P|ACC_PL_K|ACC_TSS
		},			/* TSS for this processor */
/* 0x028 */	{ 0, 0, 0, 0 },		/* per-thread LDT */
/* 0x030 */	{ 0, 0, 0, 0 },		/* per-thread TSS for IO bitmap */
/* 0x038 */	{ 0, 0, 0, 0 },         /* FP emulator code segment */
/* 0x040 */	{ 0, 0, 0, 0 },         /* FP emulator data (FP registers) */

#ifdef	PS2
/* 0x048 */     { (unsigned int)&abios_int_return,
                  15,
		  SZ_32|SZ_G,
		  ACC_P|ACC_PL_K|ACC_CODE_R
		},			/* return from ABIOS - thread stack */ 
/* 0x050 */     { (unsigned int)&abios_th_return,
                  15, 
                  SZ_32|SZ_G,
                  ACC_P|ACC_PL_K|ACC_CODE_R
                },                      /* return from ABIOS - int stack */
/* 0x058 */     { (unsigned int) &intstack,
		  INTSTACK_SIZE-1,
		  0,			/* 16 bit, byte granularity */
		  ACC_P|ACC_PL_K|ACC_DATA_W
		},			/* ABIOS interrupt stack */
/* 0x060 */	{ 0,			/* will be filled in later */
                  KERNEL_STACK_SIZE-1,
                  0,                    /* 16 bit, byte granularity */
                  ACC_P|ACC_PL_K|ACC_DATA_W
                },                      /* ABIOS kernel stack */
/* 0x068 */     { 0, 0, 0, 0 },         /* first avail for allocate_gdt */
#endif  /* PS2 */
};


