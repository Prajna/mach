/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * Copyright 1988, 1989, 1990, 1991 by Intel Corporation,
 * Santa Clara, California.
 * 
 *                          All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and that
 * both the copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Intel not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 * 
 * INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
 * SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN ACTION OF CONTRACT, NEGLIGENCE, OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */
/*
 * HISTORY
 * $Log:	bia.h,v $
 * Revision 2.4  91/12/10  16:29:49  jsb
 * 	Fixes from Intel
 * 	[91/12/10  15:32:07  jsb]
 * 
 * Revision 2.3  91/06/18  20:50:02  jsb
 * 	New copyright from Intel.
 * 	[91/06/18  20:05:41  jsb]
 * 
 * Revision 2.2  90/12/04  14:47:00  jsb
 * 	First checkin.
 * 	[90/12/04  10:55:26  jsb]
 * 
 */
/*
 *	File:	bia.h
 *	Author:	Joseph S. Barrera III
 *
 *	Copyright (c) 1990 Joseph S. Barrera III
 *
 *	Declarations for VME BIA (Bus Interface Adaptor).
 *	Derived from NX's bia.h.
 */

typedef unsigned char	byte;
typedef unsigned short	word16;
typedef unsigned long	word32;

#define	MAX_BIA		3	/* Maximum number of BIA boards in system */
#define	DEF_BIA		0	/* Default BIA board number */
#define	NUMPATS		7	/* Number of static patterns */

#define BIA_VME		0x00800000	/* Base address of BIA VME space */
#define BIA_REG		0x00c00000	/* Base address of BIA registers */
#define BIA_OFFSET	0x01000000	/* Address offset for each BIA board */
#define BIA_VMESIZE	BIA_REG-BIA_VME		/* VME address space per BIA */
#define BIA_REGSIZE	BIA_OFFSET-BIA_REG	/* Reg address space per BIA */

struct bia {
	byte	bia_vme[BIA_VMESIZE];
	union {
		struct {
			word32	r_isr;
#define				r_clr_int	r_isr
			word32	r_swap_reg;
#define				r_control	r_swap_reg
			word32	r_imr;
			word32	r_pag_mod;
		} r;
		byte	b_reg[BIA_REGSIZE];
	} bia_u;
};

#define	bia_isr		bia_u.r.r_isr
#define	bia_clr_int	bia_u.r.r_clr_int
#define	bia_swap_reg	bia_u.r.r_swap_reg
#define	bia_control	bia_u.r.r_control
#define	bia_imr		bia_u.r.r_imr
#define	bia_pag_mod	bia_u.r.r_pag_mod
#define	bia_reg		bia_u.b_reg

#define PAG_MASK	0xffc00000		/* Mask for page bits */
#define MOD_MASK	0x0000003f		/* Mask for modifier bits */
#define P_M_MASK	(PAG_MASK|MOD_MASK)	/* Mask for PAG_MOD register */


/* These defines are offsets from a BIA base address for each register */
#define ISR		0x0000		/* Read interrupt status reg */
#define CLR_INT		0x0000		/* Clear interrupt status reg */
#define SWAP_REG	0x0004		/* Read byte swapping test reg */
#define CONTROL		0x0004		/* Write BIA control reg */
#define IMR		0x0008		/* Read/write interrupt mask reg */
#define PAG_MOD		0x000c		/* Read/write VME page address reg */


/* These defines are for the interrupt status register bits */
#define ISR_IRQ1_	0x0001		/* Low true, VME IRQ1 */
#define ISR_IRQ2_	0x0002		/* Low true, VME IRQ2 */
#define ISR_IRQ3_	0x0004		/* Low true, VME IRQ3 */
#define ISR_IRQ4_	0x0008		/* Low true, VME IRQ4 */
#define ISR_IRQ5_	0x0010		/* Low true, VME IRQ5 */
#define ISR_IRQ6_	0x0020		/* Low true, VME IRQ6 */
#define ISR_IRQ7_	0x0040		/* Low true, VME IRQ7 */
#define ISR_U1		0x0080		/* Unused bit 1 */
#define ISR_DTACK_	0x0100		/* Low true, slave read/wrote VME */
#define ISR_SYSFAIL_	0x0200		/* Low true, VME SYSFAIL line */
#define ISR_BIA_TO_	0x0400		/* Low true, BIA bus time out error */
#define ISR_BERR_	0x0800		/* Low true, VME bus error */
#define ISR_S1		0x1000		/* Spare bit 1 */
#define ISR_U2		0x2000		/* Unused bit 2 */
#define ISR_U3		0x4000		/* Unused bit 3 */
#define ISR_U4		0x8000		/* Unused bit 4 */

#define ISR_NONE	0xf7ff		/* Value indicating no ints pending */
#define ISR_ALL_IRQ	0x007f		/* Mask value for IRQ levels only */


/* These defines are for the BIA CONTROL register bits */
#define LED_GRN		0x0001		/* Green LED */
#define LED_RED		0x0002		/* Red LED */
#define LED_YEL		0x0004		/* Yellow LED */
#define SHFL0		0x0008		/* LSB of memory mode */
#define CTRL_U1		0x0010		/* Unused bit 1 */
#define CTRL_U2		0x0020		/* Unused bit 2 */
#define CTRL_U3		0x0040		/* Unused bit 3 */
#define CTRL_U4		0x0080		/* Unused bit 4 */
#define SYSFAIL_	0x0100		/* Low true, asserts VME SYSFAIL line */
#define RESET_		0x0200		/* Low true, asserts ?? */
#define TRISTATE_	0x0400		/* Low true, VME address, data, ctrl high */
#define SHFL1		0x0800		/* MSB of memory mode */
#define TEST_NET	0x1000		/* Enables reading of test network latches */
#define BLK_MODE	0x2000		/* Enables VME block mode transfers */
#define VME_IACK	0x4000		/* Enables VME interrupt acknowledge cycle */
#define INT_ENBL	0x8000		/* Enables interrupts through BIA to PBX node */

/* Values for the Memory Access Modes */

#define MODE_ARRAY	0		/* Array Compatibility */
#define MODE_16_BIT	SHFL1		/* 16 bit access Mode */
#define MODE_8_BIT	SHFL0		/* 8 bit access Mode */
#define MODE_32_BIT	(SHFL0 | SHFL1)	/* 32 bit access MOde */

/* Value to reset entire BIA control register to */
#define RESET_CTRL	(SYSFAIL_ | RESET_ | TRISTATE_)
#define LED_ON_DELAY	1500L	/* LED on for 1.5 seconds */
#define LED_OFF_DELAY	500L	/* LED off for .5 seconds */


/* These defines are for the interrupt mask register bits */
#define IMR_SYSFAIL	0x0001		/* Mask off VME SYSFAIL line */
#define IMR_BIA_TO	0x0002		/* Mask off BIA bus time out error */
#define IMR_BERR	0x0004		/* Mask off VME bus error */
#define IMR_IRQ1	0x0008		/* Mask off VME IRQ1 */
#define IMR_IRQ2	0x0010		/* Mask off VME IRQ2 */
#define IMR_IRQ3	0x0020		/* Mask off VME IRQ3 */
#define IMR_IRQ4	0x0040		/* Mask off VME IRQ4 */
#define IMR_IRQ5	0x0080		/* Mask off VME IRQ5 */
#define IMR_IRQ6	0x0100		/* Mask off VME IRQ6 */
#define IMR_IRQ7	0x0200		/* Mask off VME IRQ7 */
#define IMR_U1		0x0400		/* Unused bit 1 */
#define IMR_U2		0x0800		/* Unused bit 2 */
#define IMR_U3		0x1000		/* Unused bit 3 */
#define IMR_U4		0x2000		/* Unused bit 4 */
#define IMR_U5		0x4000		/* Unused bit 5 */
#define IMR_U6		0x8000		/* Unused bit 6 */

#define IMR_ALL_IRQ	0x03f8		/* All the IRQs */
#define IMR_MASKALL	0xffff		/* Mask off all interrupts */
#define IMR_NO_MASK	0		/* No masking of any interrupts */


/* These defines are for the PAG_MOD register */
#define STD_SUP_BLK	0x003f		/* A24 supervisory block transfer */
#define STD_SUP_DATA	0x003d		/* A24 supervisory data access */
#define STD_NO_P_BLK	0x003b		/* A24 non-privileged block transfer */
#define STD_NO_P_DATA	0x0039		/* A24 non-privileged data access */
#define SHORT_SUP	0x002d		/* A16 supervisory access */
#define SHORT_NO_P	0x0029		/* A16 non-privileged access */
#define EXT_SUP_BLK	0x000f		/* A32 supervisory block transfer */
#define EXT_SUP_DATA	0x000d		/* A32 supervisory data access */
#define EXT_NO_P_BLK	0x000b		/* A32 non-privileged block transfer */
#define EXT_NO_P_DATA	0x0009		/* A32 non-privileged data access */

/*\
***   PIC ports and values
\*/

#define SPIC_PORTB	0xC6
#define NBCR_PORT	0x80
#define NUSMINT		0x20
#define LBXENBL		(NUSMINT | 0x8)
#define LBXENBINT	0x10
