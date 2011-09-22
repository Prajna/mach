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
 * $Log:	asm_misc.s,v $
 * Revision 2.6  91/08/24  12:18:12  af
 * 	Rid of longjmp crap. Added 3min callbacks.
 * 	[91/08/22  11:21:27  af]
 * 
 * Revision 2.5  91/05/14  17:17:12  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:38:31  mrt
 * 	Added author notices
 * 	[91/02/04  11:09:18  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:04:02  mrt]
 * 
 * Revision 2.3  90/12/05  23:29:22  af
 * 	Created.
 * 	[90/12/02            af]
 */
/*
 *	File: asm_misc.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	12/90
 *
 *	Miscellaneous assembly code.
 */

#include <mach/mips/asm.h>
#include <mips/mips_cpu.h>
#include <mips/PMAX/boot/asm_misc.h>

	.text
	.set	noreorder

/*
 *	PROM interface jump table, pmax/3max version
 *
 *	Object:
 * 		pmax_halt			EXPORTED function
 * 		pmax_open			EXPORTED function
 * 		pmax_lseek			EXPORTED function
 * 		pmax_read			EXPORTED function
 * 		pmax_close			EXPORTED function
 * 		pmax_gets			EXPORTED function
 * 		pmax_printf			EXPORTED function
 * 		pmax_geten			EXPORTED function
 */
	.text

EXPORT(pmax_halt)
	li	v0,PMAX_PROM_HALT
	j	v0
	nop
EXPORT(pmax_open)
	li	v0,PMAX_PROM_OPEN
	j	v0
	nop
EXPORT(pmax_lseek)
	li	v0,PMAX_PROM_LSEEK
	j	v0
	nop
EXPORT(pmax_read)
	li	v0,PMAX_PROM_READ
	j	v0
	nop
EXPORT(pmax_close)
	li	v0,PMAX_PROM_CLOSE
	j	v0
	nop
EXPORT(pmax_gets)
	li	v0,PMAX_PROM_GETS
	j	v0
	nop
EXPORT(pmax_printf)
	li	v0,PMAX_PROM_PRINTF
	j	v0
	nop
EXPORT(pmax_getenv)
	li	v0,PMAX_PROM_GETENV
	j	v0
	nop

/*
 *	PROM interface jump table, 3min version
 *
 *	Object:
 * 		kmin_halt			EXPORTED function
 * 		kmin_open			EXPORTED function
 * 		kmin_lseek			EXPORTED function
 * 		kmin_read			EXPORTED function
 * 		kmin_close			EXPORTED function
 * 		kmin_gets			EXPORTED function
 * 		kmin_printf			EXPORTED function
 * 		kmin_getenv			EXPORTED function
 */
	.text

	BSS(kmin_prom_base,4)

kmin_call:
	lw	t0,kmin_prom_base
	nop
	addu	v0,t0
	lw	v0,0(v0)
	nop
	j	v0
	nop
	
EXPORT(kmin_halt)
	j	kmin_call
	li	v0,KMIN_PROM_HALT

EXPORT(kmin_gets)
	j	kmin_call
	li	v0,KMIN_PROM_GETS

EXPORT(kmin_printf)
	j	kmin_call
	li	v0,KMIN_PROM_PRINTF

EXPORT(kmin_getenv)
	j	kmin_call
	li	v0,KMIN_PROM_GETENV

EXPORT(kmin_open)
	j	kmin_call
	li	v0,KMIN_PROM_OPEN

	.set	reorder
	BSS(kmin_seek_ptr,4)
EXPORT(kmin_lseek)
	/* turn seek ptr back to blockno */
	srl	a1,9
	sw	a1,kmin_seek_ptr
	j	ra
	.set	noreorder

EXPORT(kmin_read)
	lw	a0,kmin_seek_ptr
	j	kmin_call
	li	v0,KMIN_PROM_READ

EXPORT(kmin_close)
	j	ra
	li	v0,-1

