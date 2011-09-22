/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 * $Log:	cross64.h,v $
 * Revision 2.2  93/01/14  17:57:43  danner
 * 	64bit cross-compile support.
 * 	[92/12/01            af]
 * 
 */

#ifdef	CROSS_COMPILE_32_TO_64_BITS

#define	word_size			8
#define	word_size_in_bits		64
#define	word_size_name			MACH_MSG_TYPE_INTEGER_64
#define	word_size_name_string		"MACH_MSG_TYPE_INTEGER_64"
#define	sizeof_pointer			8
#define	sizeof_mach_msg_header_t	40
#define	sizeof_mach_msg_type_long_t	16
#define	sizeof_mach_msg_type_t		4

#else

#define	word_size			sizeof(integer_t)
#define	word_size_in_bits		(sizeof(integer_t)*8)
#if	alpha
#define	word_size_name			MACH_MSG_TYPE_INTEGER_64
#define	word_size_name_string		"MACH_MSG_TYPE_INTEGER_64"
#else
#define	word_size_name			MACH_MSG_TYPE_INTEGER_32	/*??*/
#define	word_size_name_string		"MACH_MSG_TYPE_INTEGER_32"	/*??*/
#endif
#define	sizeof_pointer			sizeof(char*)
#define	sizeof_mach_msg_header_t	sizeof(mach_msg_header_t)
#define	sizeof_mach_msg_type_long_t	sizeof(mach_msg_type_long_t)
#define	sizeof_mach_msg_type_t		sizeof(mach_msg_type_t)

#endif
