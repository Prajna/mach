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
 *
 *
 * HISTORY
 * $Log:	crt0.s,v $
 * Revision 2.5  93/08/03  12:35:45  mrt
 * 	Unified profiling and non-profiling crt0 into one file.
 * 	[93/07/30  10:29:38  bershad]
 * 
 * Revision 2.4  93/05/12  15:43:15  rvb
 * 	.aent belongs in .ent
 * 
 * Revision 2.3  93/05/10  17:51:03  rvb
 * 	Fix mcount(), to pop 8 bytes and restore AT -> RA
 * 	This will/should make gcc 2.3 happy.
 * 	[93/04/19  22:53:26  rvb]
 * 
 * Revision 2.2  93/03/26  16:17:13  mrt
 * 	Added eprol and mcount entries.
 * 	Fix by Bob Baron
 * 	Changed comments to c style, renamed to crt0.s
 * 	[93/03/22            mrt]
 * 
 * Revision 2.7  92/03/01  00:40:13  rpd
 * 		Removed .verstamp, .file, .loc directives.
 * 	[92/02/29            rpd]
 * 
 * Revision 2.6  91/05/14  17:54:31  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/14  14:18:29  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:50:02  mrt]
 * 
 * Revision 2.4  90/11/05  14:35:56  rpd
 * 	Removed the definition of exit.
 * 	[90/10/30            rpd]
 * 
 * Revision 2.3  90/06/02  15:12:57  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  23:28:04  rpd]
 * 
 * Revision 2.2  90/01/19  14:36:34  rwd
 * 	Version from sandro.
 * 	[90/01/18            rwd]
 * 
 */

	.comm	environ 4
	.comm	mach_init_routine 4
	.comm	_cthread_init_routine 4
	.comm	_cthread_exit_routine 4
	.comm	_monstartup_routine 4	
	.text	
	.align	2
	.globl	__start
	.ent	__start 2
__start:
	la	$28,_gp
	move	$4,$29
	subu	$sp, 24
	sw	$31, 20($sp)
	sw	$16, 16($sp)
	.mask	0x80010000, -4
	.frame	$sp, 24, $31
	move	$5, $4
/*
 *  		register unsigned argc;
 *  		register va_list argv;
 *  	
 *  		argc = va_arg(sp, unsigned);
 */
	addu	$5, $5, 7
	and	$5, $5, -4
	lw	$16, -4($5)
/*
 * 		argv = sp;
 * 		environ = argv + (argc + 1) * sizeof(char *);
 */
	mul	$14, $16, 4
	addu	$15, $5, $14
	addu	$24, $15, 4
	sw	$24, environ
/*
 *  		if (mach_init_routine)
 */
	lw	$25, mach_init_routine
	beq	$25, 0, $32
/*
 *			(*mach_init_routine)();
 */
	sw	$5, 24($sp)
	jal	$25
	lw	$5, 24($sp)
$32:
	.globl _eprol
_eprol:
	lw	$2, _cthread_init_routine
/*
 * 		if (_cthread_init_routine)
 */
	beq	$2, 0, $33
/*
 * 			(*_cthread_init_routine)();
 */
	sw	$5, 24($sp)
	jal	$2
	lw	$5, 24($sp)

	beq	$2, $0, $33
	subu	$2, 0x14
	move	$29,$2

$33:

/*
 *  		if (_monstartup_routine)
 */
	lw	$2, _monstartup_routine
	beq	$2, 0, $34
/*
 *			(*_monstartup_routine)();
 */
	sw	$5, 0($sp)
	jal	$2
	lw	$5, 0($sp) 
	
$34:	

/*
 * 		argc = main(argc, argv, environ);
 */
	move	$4, $16
	lw	$6, environ
	jal	main
	lw	$3, _cthread_exit_routine
	move	$16, $2
/*
 * 		if (_cthread_exit_routine)
 */
	beq	$3, 0, $35
/*
 *			(*_cthread_exit_routine)(argc);
 */
	move	$4, $16
	jal	$3
$35:
/*
 *  		exit(argc);
 */
	move	$4, $16
	jal	exit
	lw	$16, 16($sp)
	lw	$31, 20($sp)
	addu	$sp, 24
	j	$31
	.end	__start
