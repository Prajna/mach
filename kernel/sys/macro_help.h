/* 
 * Mach Operating System
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 *	File:	kern/macro_help.h
 *
 *	Provide help in making lint-free macro routines
 *
 * HISTORY
 * $Log:	macro_help.h,v $
 * Revision 2.3  93/03/09  11:25:27  danner
 * 	Simplified.
 * 	[93/03/09            danner]
 * 
 * Revision 2.2  93/02/04  07:51:01  danner
 * 	Cast down from UX.
 * 
 * Revision 2.1.2.1  92/12/18  16:46:34  pds
 * 	Check if MACRO_BEGIN has already been defined (by cthreads.h).
 * 	[92/06/13            pds]
 * 
 * Revision 2.1  89/08/04  14:46:36  rwd
 * Created.
 * 
 * Revision 2.2  88/10/18  03:36:20  mwyoung
 * 	Added a form of return that can be used within macros that
 * 	does not result in "statement not reached" noise.
 * 	[88/10/17            mwyoung]
 * 	
 * 	Add MACRO_BEGIN, MACRO_END.
 * 	[88/10/11            mwyoung]
 * 	
 * 	Created.
 * 	[88/10/08            mwyoung]
 * 
 */

#ifndef	_MACRO_HELP_H_
#define	_MACRO_HELP_H_	1

#define		MACRO_BEGIN	do {
#define		MACRO_END	} while (0)

#define		MACRO_RETURN	if (1) return

#endif	/* _MACRO_HELP_H_ */


