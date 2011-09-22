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
 * $Log:	crt0.s,v $
 * Revision 2.3  93/05/17  18:00:05  mrt
 * 	Changed comments from # to /* so cpp would be happy.
 * 	[93/05/17            mrt]
 * 
 * Revision 2.2  93/01/14  18:02:49  danner
 * 	RCS-ed.
 * 	[92/12/10            af]
 * 
 */
	.verstamp	6 0
	.comm	environ 8
	.comm	mach_init_routine 8
	.comm	_cthread_init_routine 8
	.comm	_cthread_exit_routine 8
	.text	
	.align	4
	.file	2 "crt0.c"
	.globl	__start
	.loc	2 83
/*  83	{ */
	.ent	__start 2
__start:

	/* +++added by hand */
	mov	$sp,$16
	br	$27,1f
1:
	/* ---added by hand */

	ldgp	$gp, 0($27)
	lda	$sp, -96($sp)
	stq	$10, 24($sp)
	stq	$9, 16($sp)
	stq	$26, 8($sp)
	stq	$16, 48($sp)
	.mask	0x04000600, -88
	.frame	$sp, 96, $26
	.loc	2 83
	.loc	2 87
/*  84		register unsigned argc;
 *  85		register char	  ** argv;
 *  86	
 *  87		argc = *(int *)sp; */
	ldq	$1, 48($sp)
	ldl	$9, 0($1)
	.loc	2 88
/*  88		argv = ++sp; */
	addq	$1, 8, $2
	stq	$2, 48($sp)
	bis	$2, $2, $10
	.loc	2 89
/*  89		environ = argv + (argc + 1); */
	addl	$9, 1, $3
	zap	$3, 240, $4
	s8addq	$4, $10, $6
	stq	$6, environ
	.loc	2 91
/*  90	
 *  91		if (mach_init_routine) */
	ldq	$7, mach_init_routine
	beq	$7, $32
	.loc	2 92
/*  92			(*mach_init_routine)(); */
	.livereg	0x00010002,0x00000000
	bis	$7, $7, $27
	jsr	$26, ($7), 0
	ldgp	$gp, 0($26)
$32:
	.loc	2 93
/*  93		if (_cthread_init_routine) */
	ldq	$8, _cthread_init_routine
	beq	$8, $33
	.loc	2 94
/*  94			(*_cthread_init_routine)(); */
	.livereg	0x00010002,0x00000000
	bis	$8, $8, $27
	jsr	$26, ($8), 0
	ldgp	$gp, 0($26)

	/* +++added by hand */
	beq	$0,$33
	subq	$0,96-8,$sp
	/* ---added by hand */

$33:
	.loc	2 96
/*  96		argc = main(argc, argv, environ); */
	bis	$9, $9, $16
	bis	$10, $10, $17
	ldq	$18, environ
	.livereg	0x0001F002,0x00000000
	jsr	$26, main
	ldgp	$gp, 0($26)
	bis	$0, $0, $9
	.loc	2 98
/*  98		if (_cthread_exit_routine) */
	ldq	$22, _cthread_exit_routine
	beq	$22, $34
	.loc	2 99
/*  99			(*_cthread_exit_routine)(argc); */
	bis	$9, $9, $16
	.livereg	0x00018002,0x00000000
	bis	$22, $22, $27
	jsr	$26, ($22), 0
	ldgp	$gp, 0($26)
$34:
	.loc	2 101
/* 101		exit(argc); */
	bis	$9, $9, $16
	.livereg	0x00018002,0x00000000
	jsr	$26, exit
	ldgp	$gp, 0($26)
	.loc	2 102
/* 102	} */
	.livereg	0xFC7F0002,0x3FC00000
	ldq	$26, 8($sp)
	ldq	$9, 16($sp)
	ldq	$10, 24($sp)
	lda	$sp, 96($sp)
	ret	$31, ($26), 1
	.end	__start
