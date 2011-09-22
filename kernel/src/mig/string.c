/* 
 * Mach Operating System
 * Copyright (c) 1993,1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	string.c,v $
 * Revision 2.6  93/11/17  18:49:17  dbg
 * 	Added strconcat3().
 * 	[93/09/14  12:22:21  af]
 * 
 * Revision 2.5  93/05/10  17:49:57  rvb
 * 	Fix include to use < vs " for new ode shadowing
 * 	[93/05/10  10:29:20  rvb]
 * 
 * 	386BSD string declarations
 * 	[93/05/05  09:25:39  rvb]
 * 
 * Revision 2.4  93/01/14  17:59:00  danner
 * 	Converted file to ANSI C.
 * 	[92/12/08            pds]
 * 
 * Revision 2.3  91/02/05  17:55:52  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:55:57  mrt]
 * 
 * Revision 2.2  90/06/02  15:05:46  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:13:46  rpd]
 * 
 * 07-Apr-89  Richard Draves (rpd) at Carnegie-Mellon University
 *	Extensive revamping.  Added polymorphic arguments.
 *	Allow multiple variable-sized inline arguments in messages.
 *
 * 15-Jun-87  David Black (dlb) at Carnegie-Mellon University
 *	Declare and initialize charNULL here for strNull def in string.h
 *
 * 27-May-87  Richard Draves (rpd) at Carnegie-Mellon University
 *	Created.
 */

#define	EXPORT_BOOLEAN
#include <mach/boolean.h>
#include <sys/types.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#ifdef	__386BSD__
 /* for str<foo> */
#include </usr/include/string.h>
#endif

string_t
strmake(const char *string)
{
    register string_t saved;

    saved = malloc(strlen(string) + 1);
    if (saved == strNULL)
	fatal("strmake('%s'): %s", string, unix_error_string(errno));
    return strcpy(saved, string);
}

string_t
strconcat(const_string_t left, const_string_t right)
{
    register string_t saved;

    saved = malloc(strlen(left) + strlen(right) + 1);
    if (saved == strNULL)
	fatal("strconcat('%s', '%s'): %s",
	      left, right, unix_error_string(errno));
    return strcat(strcpy(saved, left), right);
}

string_t
strconcat3(const_string_t left, const_string_t middle, const_string_t right)
{
    register string_t saved;

    saved = malloc(strlen(left) + strlen(middle) + strlen(right) + 1);
    if (saved == strNULL)
	fatal("strconcat('%s', '%s', '%s'): %s",
	      left, middle, right, unix_error_string(errno));
    return strcat(strcat(strcpy(saved, left), middle), right);
}

void
strfree(string_t string)
{
    free(string);
}

const char *
strbool(boolean_t bool)
{
    if (bool)
	return "TRUE";
    else
	return "FALSE";
}

const char *
strstring(const_string_t string)
{
    if (string == strNULL)
	return "NULL";
    else
	return string;
}
