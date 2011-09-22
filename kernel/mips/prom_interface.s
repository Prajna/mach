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
 * $Log:	prom_interface.s,v $
 * Revision 2.6  92/02/19  15:09:29  elf
 * 	Changed #-style comments, for ANSI cpp.
 * 	[92/02/19  13:11:25  rpd]
 * 
 * Revision 2.5  91/08/24  12:24:16  af
 * 	Reduced the number of entry points to the ones we actually use.
 * 	Added yet-another-prom-interface-from-DEC, for 3min.
 * 	[91/06/25            af]
 * 
 * Revision 2.4  91/05/14  17:37:40  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:50:54  mrt
 * 	Added author notices
 * 	[91/02/04  11:24:37  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:28:57  mrt]
 * 
 * Revision 2.2  89/11/29  14:15:18  af
 * 	Created.
 * 	[89/10/06            af]
 */
/*
 *	File: prom_interface.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	PROM entrypoints of interest to kernel
 */

#include <platforms.h>

#include <mach/mips/asm.h>
#include <mips/mips_cpu.h>
#include <mips/prom_interface.h>

	.text
	.align	2

	/******************************************************/
	/* Functions assumed by the MI code as always defined */
	/*          and their standard implementation         */
	/******************************************************/

/*
 * Return to prom
 */
d_prom_halt:
	li	v0,PROM_HALT
	j	v0

/*
 * Reboot the system
 */
d_prom_reboot:
	li	v0,PROM_AUTOBOOT
	j	v0

/*
 * Print a character on the console
 */
d_prom_putchar:
	li	v0,PROM_PUTCHAR
	j	v0

/*
 * Get the value of an environment variable
 */
d_prom_getenv:
	li	v0,PROM_GETENV
	j	v0

	.sdata
	/* This is a C structure.  Making it hard to manage is intentional */
EXPORT(prom_call)
	.word	d_prom_halt
	.word	d_prom_reboot
	.word	d_prom_putchar
	.word	d_prom_getenv

	.text


	/*******************************************************************/
	/* Machine-dependent other functions, or alternate implementations */
	/*******************************************************************/

#ifdef	MSERIES
#define	PROM_ORW_RMW	PROM_ENTRY(22)	/* r-m-w version of or word */
#define	PROM_ORH_RMW	PROM_ENTRY(23)	/* r-m-w version of or halfword */
#define	PROM_ORB_RMW	PROM_ENTRY(24)	/* r-m-w version of or byte */
#define	PROM_ANDW_RMW	PROM_ENTRY(25)	/* r-m-w version of and word */
#define	PROM_ANDH_RMW	PROM_ENTRY(26)	/* r-m-w version of and halfword */
#define	PROM_ANDB_RMW	PROM_ENTRY(27)	/* r-m-w version of and byte */
/*
 * prom read-modify-write routines
 */
EXPORT(andb_rmw)
	li	v0,PROM_ANDB_RMW
	j	v0

EXPORT(andh_rmw)
	li	v0,PROM_ANDH_RMW
	j	v0

EXPORT(andw_rmw)
	li	v0,PROM_ANDW_RMW
	j	v0

EXPORT(orb_rmw)
	li	v0,PROM_ORB_RMW
	j	v0

EXPORT(orh_rmw)
	li	v0,PROM_ORH_RMW
	j	v0

EXPORT(orw_rmw)
	li	v0,PROM_ORW_RMW
	j	v0
#endif	MSERIES


#ifdef	KMIN
/*
 * Sad as it is, DEC changed the prom interface in
 * the TC machines, forcing this to happen
 */
	IMPORT(prom_arg2,4)
	IMPORT(prom_arg3,4)


#define	KMIN_PROM_MAGIC		0x30464354

#define	KMIN_PROM_REX		0xac
#define	KMIN_PROM_HALT		0x9c
#define	KMIN_PROM_PRINTF	0x30
#define	KMIN_PROM_GETENV	0x64
#define	KMIN_MEMORY_BITMAP	0x84
#define	KMIN_FIND_SYSTYPE	0x80

	.set	noreorder

STATIC_LEAF(kmin_prom_call)
	lw	t0,prom_arg3
	nop
	addu	v0,t0
	lw	v0,0(v0)
	nop
	j	v0
	nop
	END(kmin_prom_call)

LEAF(kmin_halt)
#if 0
	/* fast, but does not reset interrupts */
	j	kmin_prom_call
	li	v0,KMIN_PROM_HALT
#else
	li	a0,0x68		/* 'h' */
	j	kmin_prom_call
	li	v0,KMIN_PROM_REX
#endif
	END(kmin_halt)

LEAF(kmin_reboot)
#if 1
	li	a0,0x62		/* 'b' */
	j	kmin_prom_call
	li	v0,KMIN_PROM_REX
#else
	/* I donno this one */
	j	d_prom_reboot
	nop
#endif
	END(kmin_reboot)

cfmt:	.asciiz	"%c"
LEAF(kmin_putchar)
	move	a1,a0
	la	a0,cfmt
	j	kmin_prom_call
	li	v0,KMIN_PROM_PRINTF
	END(kmin_putchar)

LEAF(kmin_getenv)
	b	kmin_prom_call
	li	v0,KMIN_PROM_GETENV
	END(kmin_getenv)

LEAF(kmin_memory_bitmap)
	b	kmin_prom_call
	li	v0,KMIN_MEMORY_BITMAP
	END(kmin_memory_bitmap)

LEAF(kmin_find_systype)
	b	kmin_prom_call
	li	v0,KMIN_FIND_SYSTYPE
	END(kmin_find_systype)

#endif	KMIN
