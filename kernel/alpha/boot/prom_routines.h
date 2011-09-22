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
 * $Log:	prom_routines.h,v $
 * Revision 2.2  93/02/05  08:01:07  danner
 * 	Created a while back.
 * 	[93/02/04            af]
 * 
 */
/*
 *	File: prom_routines.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	12/90
 *
 *	PROM callback interface
 */

#ifndef	_BOOT_PROM_ROUTINES_H_
#define	_BOOT_PROM_ROUTINES_H_	1

#include "../prom_interface.h"

/*
 * Which of the mandatory callbacks we need..
 * besides what the kernel uses
 */
#define	PROM_R_OPEN		0x10
#define	PROM_R_CLOSE		0x11
#define	PROM_R_READ		0x13

/*
 * The (extra) prom calls
 */
	/* Opens device X (a string of Y bytes)
	   ok status is 0 */
#define	prom_open(x,y)		prom_dispatch( PROM_R_OPEN, x, y)

	/* Close channel X
	   ok status is 0 */
#define	prom_close(x)		prom_dispatch( PROM_R_CLOSE, x)

	/* Read from channel X Y bytes at address W, blockno Z
	   ok status is 0 */
#define	prom_read(x,y,w,z)	prom_dispatch( PROM_R_READ, x, y, w, z)

/*
 * A more friendly approach
 */
extern int console;
#define	puts(s)		prom_puts(console, s, sizeof(s)-1)
extern void putnum( unsigned long int );

/*
 * Which other of the mandatory environment variables we need
 */

#define	PROM_E_BOOTED_FILE	0x6

#endif	/* _BOOT_PROM_ROUTINES_H_ */
