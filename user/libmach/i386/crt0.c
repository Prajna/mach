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
 * $Log:	crt0.c,v $
 * Revision 2.11  93/08/03  12:35:33  mrt
 * 	Unified all the *crt into one that is conditional at runtime, not compile
 * 	time.  
 * 	[93/07/30  10:29:06  bershad]
 * 
 * Revision 2.10  93/05/10  17:50:53  rvb
 * 		MACH is true, and Wheeze is false.
 * 	[93/05/05            rvb]
 * 
 * Revision 2.9  93/03/31  12:19:05  mrt
 * 	Added a definition for _mcount in
 * 	addtion to mcount.
 * 
 * 
 * Revision 2.8  93/01/24  13:23:38  danner
 * 	Need -DCRT0 and -DMCRT0 versions.  MCRT0 will be done
 * 	later.
 * 	[92/11/29            rvb]
 * 	Export _eprol
 * 	[92/11/21  22:33:42  rvb]
 * 
 * Revision 2.7  91/07/31  18:27:51  dbg
 * 	Fix for ANSI C.
 * 	[91/07/30  17:29:44  dbg]
 * 
 * Revision 2.6  91/05/14  17:52:39  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/14  14:17:23  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  14:16:29  mrt]
 * 
 * Revision 2.4  90/11/05  14:35:42  rpd
 * 	Removed the definition of exit.
 * 	[90/10/30            rpd]
 * 
 * Revision 2.3  90/06/02  14:36:49  rpd
 * 	Converted to new IPC.
 * 	[90/06/02            rpd]
 * 
 * Revision 2.2  90/05/03  15:54:13  dbg
 * 	Fix include of i386/asm.h
 * 	[90/04/30            dbg]
 * 	Created.
 * 	[90/04/30  15:32:17  dbg]
 * 
 */
/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char sccsid[] = "@(#)crt0.c	5.2 (Berkeley) 5/14/90";
#endif /* not lint */

/*
 *	C start up routine.
 *	Robert Henry, UCB, 20 Oct 81
 *
 *	We make the following (true) assumptions:
 *	1) when the kernel calls start, it does a jump to location 2,
 *	and thus avoids the register save mask.  We are NOT called
 *	with a calls!  see sys1.c:setregs().
 *	2) The only register variable that we can trust is sp,
 *	which points to the base of the kernel calling frame.
 *	Do NOT believe the documentation in exec(2) regarding the
 *	values of fp and ap.
 *	3) We can allocate as many register variables as we want,
 *	and don't have to save them for anybody.
 *	4) Because of the ways that asm's work, we can't have
 *	any automatic variables allocated on the stack, because
 *	we must catch the value of sp before any automatics are
 *	allocated.
 */

#include <machine/asm.h>

char **environ = (char **)0;
#ifdef paranoid
static int fd;
#endif paranoid

int	(*mach_init_routine)();
int	(*_cthread_init_routine)();
int	(*_cthread_exit_routine)();
int	(*_monstartup_routine)();
int	(*_StrongBox_init_routine)();
int	errno = 0;
int	exit();

extern	unsigned char	etext;
extern	unsigned char	_eprol;
_start()
{
	struct kframe {
		int	kargc;
		char	*kargv[1];	/* size depends on kargc */
		char	kargstr[1];	/* size varies */
		char	kenvstr[1];	/* size varies */
	};
	/*
	 *	ALL REGISTER VARIABLES!!!
	 */
	register int r11;		/* needed for init */
	register struct kframe *kfp;	/* r10 */
	register char **targv;
	register char **argv;

#ifdef lint
	kfp = 0;
	initcode = initcode = 0;
#else not lint
#define Entry_sp() \
({ int _spl__, _tmp1__; \
	asm volatile("leal 4(%%ebp), %0" : "=r" (_spl__) : "r" (_tmp1__)); \
	_spl__; })

	kfp = (struct kframe *)Entry_sp();
#endif not lint
	for (argv = targv = &kfp->kargv[0]; *targv++; /* void */)
		/* void */ ;
	if (targv >= (char **)(*argv))
		--targv;
	environ = targv;
	if (mach_init_routine)
		(void) mach_init_routine();

asm(".globl __eprol");
asm("__eprol:");

#ifdef paranoid
	/*
	 * The standard I/O library assumes that file descriptors 0, 1, and 2
	 * are open. If one of these descriptors is closed prior to the start 
	 * of the process, I/O gets very confused. To avoid this problem, we
	 * insure that the first three file descriptors are open before calling
	 * main(). Normally this is undefined, as it adds two unnecessary
	 * system calls.
	 */
	do	{
		fd = open("/dev/null", 2);
	} while (fd >= 0 && fd < 3);
	close(fd);
#endif paranoid


	if (_cthread_init_routine) {
	    int new_sp;
	    new_sp = (*_cthread_init_routine)();
	    if (new_sp) {
		asm volatile("movl %0, %%esp" : : "g" (new_sp) );
	    }
	}
	if (_StrongBox_init_routine) (*_StrongBox_init_routine)();

	if (_monstartup_routine)  {
	    _monstartup_routine(&_eprol, &etext);
	}

	(* (_cthread_exit_routine ? _cthread_exit_routine : exit))
		(main(kfp->kargc, argv, targv));
}

    

