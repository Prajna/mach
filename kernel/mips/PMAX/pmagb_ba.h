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
 * $Log:	pmagb_ba.h,v $
 * Revision 2.2  93/02/05  08:04:09  danner
 * 	Turns out Flamingo has same offset as Ds.
 * 	Define a neutral "reset" address cuz I cannot find
 * 	a proper specific one.
 * 	[93/02/04  02:05:12  af]
 * 
 * 	Created, with imagination from the DEC specs:
 * 	"Flamingo Macrocoder's Manual" Chap 4.6 "CXTURBO"
 * 	Workstation and Servers, Maynard, MA. Oct 21, 1991.
 * 	[Yes, there is certainly a more proper document]
 * 
 * 	[90/09/03            af]
 * 
 */
/*
 *	File: pmagb_ba.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	11/92
 *
 *	Defines for the PMAGB-BA TC option board (Smart Frame Buffer)
 */

#define SFB_OFFSET_ROM		0x0		/* Diagnostic ROM */
#define SFB_OFFSET_REGS		0x100000	/* SFB registers */
#define SFB_OFFSET_BT459	0x1c0000	/* Bt459 registers */
#define	SFB_OFFSET_VRAM		0x201000	/* from module's base */

#if	mips
#define	SFB_OFFSET_RESET	0x180000	/* from module's base */
#endif

#if	alpha
#define	SFB_OFFSET_RESET	0x200000	/* ???? where is it ????? */
#endif

/* Registers, plus defines if needed */

typedef struct {
	volatile unsigned int	buf0;
	volatile unsigned int	buf1;
	volatile unsigned int	buf2;
	volatile unsigned int	buf3;
	volatile unsigned int	buf4;
	volatile unsigned int	buf5;
	volatile unsigned int	buf6;
	volatile unsigned int	buf7;
	volatile unsigned int	fg;
	volatile unsigned int	bg;
	volatile unsigned int	plane_mask;
	volatile unsigned int	pixel_mask;
	volatile unsigned int	mode;
	volatile unsigned int	bool_op;
	volatile unsigned int	pixel_shift;
	volatile unsigned int	address;
	volatile unsigned int	bresenham_0;
	volatile unsigned int	bresenham_1;
	volatile unsigned int	bresenham_2;
	volatile unsigned int	bcont;
	volatile unsigned int	deep;
	volatile unsigned int	start;
	volatile unsigned int	clear_intr;
	volatile unsigned int	pad0;
	volatile unsigned int	vrefresh_count;
	volatile unsigned int	vhor_setup;
	volatile unsigned int	vvert_setup;
	volatile unsigned int	vbase;
	volatile unsigned int	vvalid;
	volatile unsigned int	intr_enable;
	volatile unsigned int	tcclk;
	volatile unsigned int	vidclk;
} sfb_regs;

#define	SFB_OFFSET_ICLR		(SFB_OFFSET_REGS+0x58)
