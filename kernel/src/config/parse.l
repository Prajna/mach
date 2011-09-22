%{
/* 
 * Mach Operating System
 * Copyright (c) 1993-1987 Carnegie Mellon University
 * Copyright (c) 1991 IBM Corporation 
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation,
 * and that the name IBM not be used in advertising or publicity 
 * pertaining to distribution of the software without specific, written
 * prior permission.
 * 
 * CARNEGIE MELLON AND IBM ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON AND IBM DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	parse.l,v $
 * Revision 2.3  93/05/10  17:48:34  rvb
 * 	Fix include to use < vs " for new ode shadowing
 * 	[93/05/10  10:36:34  rvb]
 * 
 * Revision 2.2  93/02/04  13:47:24  mrt
 * 	profile is now a keyword that can be set in the configuration
 * 	   file and with -p on the command line.
 * 	Integrated PS/2 code from IBM - by Prithvi
 * 		Added device-driver, target, and scsibus for Sparc (sun4)
 * 		   port. -by Berman
 * 	[93/01/21            danner]
 * 
 * 	Renamed from config.l, updated copyright and condensed
 * 	history
 * 	[93/01/16            mrt]
 * 
 * Revision 2.7  92/01/24  18:15:49  rpd
 * 	Removed swap/dump/arg/root stuff.
 * 	[92/01/24            rpd]
 * 
 * Condensed history
 * 	Luna88k support.
 * 	cputypes.h->platforms.h
 * 	Merge CMU changes into Tahoe release.
 * 	Removed dst, maxdsiz, maxusers, timezone.
 * 	Add mips keywords. Also make makevariables an alias for
 * 	makeoptions.
 * 	Changed the definition of WORD, to allow numbers
 * 	if the identifier starts with a capital letter.
 *	Added "bin" keyword.
 */

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)config.l	5.5 (Berkeley) 6/18/88
 */

#include <ctype.h>
#include <gram.h>
#include <config.h>

#define tprintf if (do_trace) printf

/*
 * Key word table
 */

struct kt {
	char *kt_name;
	int kt_val;
} key_words[] = {
	{ "and",	AND },
	{ "at",		AT },
	{ "config",	CONFIG },
	{ "controller",	CONTROLLER },
	{ "conftype",	CONF },
	{ "csr",	CSR },
	{ "device",	DEVICE },
	{ "device-driver",	DEVICE_DRIVER },	/* Sun4 only */
	{ "disk",	DISK },
	{ "drive",	DRIVE },
	{ "flags",	FLAGS },
	{ "ident",	IDENT },
	{ "major",	MAJOR },
	{ "makeoptions", MAKEOPTIONS },
	{ "makevariables", MAKEOPTIONS },
	{ "master",	MASTER },
	{ "mba",	MBA },
	{ "minor",	MINOR },
	{ "nexus",	NEXUS },
	{ "options",	OPTIONS },
	{ "platform",	PLATFORM },
	{ "priority",	PRIORITY },
	{ "profile",    PROFILE },
	{ "pseudo-device",PSEUDO_DEVICE },
	{ "scsibus",	SCSIBUS },		/* Sun4 only */
	{ "slave",	SLAVE },
	{ "tape",	DEVICE },
	{ "target",	TARGET },		/* Sun4 only */
	{ "trace",	TRACE },
	{ "uba",	UBA },
	{ "vector",	VECTOR },
	{ "lun",	LUN },
	{ "slot",	SLOT },
	{ "tape",	TAPE },
	{ "bin",	BIN },			/* SQT ONLY */
#ifdef	mips
	{ "am",		ADDRMOD },		/* MIPS */
	{ "mbii",	MBII },			/* MIPS */
 	{ "vme",	VME },			/* MIPS */
#endif	mips
        { "high",       HIGH },                 /* I386 */
        { "memory",     MEMORY },               /* I386 */
        { "irq",        IRQ },                  /* I386 */
        { "length",     LENGTH },               /* I386 */
        { "port",       PORT },                 /* I386 */
        { "spl",        SPL },                  /* I386 */
	{ 0, 0 },
};
%}
WORD	([A-Za-z_][-A-Za-z_]*|[A-Z][-A-Za-z_0-9]*)
%%
{WORD}		{
			int i;

			if ((i = kw_lookup(yytext)) == -1)
			{
				yylval.str = yytext;
				tprintf("id(%s) ", yytext);
				return ID;
			}
			tprintf("(%s) ", yytext);
			return i;
		}
\"[^"]+\"	{
			yytext[strlen(yytext)-1] = '\0';
			yylval.str = yytext + 1;
			return ID;
		}
0[0-7]*		{
			yylval.val = octal(yytext);
			tprintf("#O:%o ", yylval.val);
			return NUMBER;
		}
0x[0-9a-fA-F]+	{
			yylval.val = hex(yytext);
			tprintf("#X:%x ", yylval.val);
			return NUMBER;
		}
[1-9][0-9]*	{
			yylval.val = atoi(yytext);
			tprintf("#D:%d ", yylval.val);
			return NUMBER;
		}
[0-9]"."[0-9]*	{
			double atof();
			yylval.val = (int) (60 * atof(yytext) + 0.5);
			return FPNUMBER;
		}
"-"		{
			return MINUS;
		}
"?"		{
			yylval.val = -1;
			tprintf("? ");
			return NUMBER;
		}
\n/[ \t]	{
			yyline++;
			tprintf("\n... ");
		}
\n		{
			yyline++;
			tprintf("\n");
			return SEMICOLON;
		}
#.*		{	/* Ignored (comment) */;	}
[ \t]*		{	/* Ignored (white space) */;	}
";"		{	return SEMICOLON;		}
","		{	return COMMA;			}
"="		{	return EQUALS;			}
"@"		{	return AT;			}
.		{	return yytext[0];		}

vme16           {       return VME16D16;                }
vme24           {       return VME24D16;                }
vme32           {       return VME32D32;                }

vme16d16        {       return VME16D16;                }
vme24d16        {       return VME24D16;                }
vme32d16        {       return VME32D16;                }
vme16d32        {       return VME16D32;                }
vme24d32        {       return VME24D32;                }
vme32d32        {       return VME32D32;                }

%%
/*
 * kw_lookup
 *	Look up a string in the keyword table.  Returns a -1 if the
 *	string is not a keyword otherwise it returns the keyword number
 */

kw_lookup(word)
register char *word;
{
	register struct kt *kp;

	for (kp = key_words; kp->kt_name != 0; kp++)
		if (eq(word, kp->kt_name))
			return kp->kt_val;
	return -1;
}

/*
 * Number conversion routines
 */

octal(str)
char *str;
{
	int num;

	(void) sscanf(str, "%o", &num);
	return num;
}

hex(str)
char *str;
{
	int num;

	(void) sscanf(str+2, "%x", &num);
	return num;
}
