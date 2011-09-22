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
 * $Log:	test.c,v $
 * Revision 2.6  91/08/24  12:20:55  af
 * 	Now take 4 args, and do not try to deref the third one.
 * 	[91/08/22  11:18:37  af]
 * 
 * Revision 2.5  91/05/14  17:18:45  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:39:21  mrt
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:06:37  mrt]
 * 
 * Revision 2.3  90/12/05  23:30:15  af
 * 	Created, standalone simple program to test new boot.
 * 	[90/12/02            af]
 * 
 */

#include <asm_misc.h>

main(argc,argv,envp,arg3)
	char **argv, **envp;
{
	prom_printf("Hi mom(%x %x %x %x)!\n", argc, argv, envp, arg3);
	prom_printf("argv -> %s\n", *argv);
}
