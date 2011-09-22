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
 * $Log:	bt455.h,v $
 * Revision 2.2  91/08/24  11:50:24  af
 * 	Cleaned up a bit, described spec sources more precisely.
 * 	[91/08/22  11:03:56  af]
 * 
 * 	Created, from Brooktree specs:
 * 	"Graphics And Imaging Product Databook 1991"
 * 	"Bt454/Bt455 170 Mhz Monolithic CMOS 16 Color Palette RAMDAC"
 * 	Brooktree Corp. San Diego, CA, 3rd Ed.
 * 	[91/07/25            af]
 * 
 */
/*
 *	File: bt455.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	7/91
 *
 *	Defines for the bt455 RAMDAC
 */

typedef struct {
	volatile unsigned char	addr_cmap;
	volatile unsigned char	addr_cmap_data;
	volatile unsigned char	addr_clr;
	volatile unsigned char	addr_ovly;
} bt455_regmap_t;

/*
 * Color Map entries 00-0f are accessed indirectly
 */
