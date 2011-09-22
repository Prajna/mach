/* 
 * Mach Operating System
 * Copyright (c) 1993-1987 Carnegie Mellon University
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
 * $Log:	main.c,v $
 * Revision 2.14  93/06/02  13:08:33  rvb
 * 	malloc is (void *) cast to (char *) where necessary.
 * 	[93/05/14            rvb]
 * 
 * Revision 2.13  93/05/10  17:48:08  rvb
 * 	Fix include to use < vs " for new ode shadowing
 * 	[93/05/10  10:35:30  rvb]
 * 
 * Revision 2.12  93/02/04  13:46:43  mrt
 * 	Changed input parameters from -d <source_directory> to [-I<dir>]*
 * 	   to allow a search path to be used to find the input files.
 * 	   Needed to make shadowing work.
 * 		Added CONFTYPE_SUN4C for Sparc (sun4) port. - by berman
 * 	Condensed History
 * 	[92/12/22            mrt]
 * 
 * Revision 2.11  93/01/14  17:56:43  danner
 * 	Added alpha, simplified crap.
 * 	[92/12/01            af]
 * 
 * Revision 2.10  92/08/03  17:18:29  jfriedl
 * 	Added pc532.
 * 	[92/05/15            jvh]
 * 
 * Revision 2.9  92/01/24  18:15:58  rpd
 * 	Removed swapconf.
 * 	[92/01/24            rpd]
 * 
 * Condensed history
 * 	Added mac2.
 * 	Luna88k support.
 * 	cputypes.h->platforms.h
 * 	Added i860 support.
 * 	Merged CMU changes into Tahoe release.
 *
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
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)main.c	5.9 (Berkeley) 6/18/88";
#endif /* not lint */

#include <stdio.h>
#include <ctype.h>
#include <gram.h>
#include <config.h>

char *search_path;
/*
 * Config builds a set of files for building a UNIX
 * system given a description of the desired system.
 */
main(argc, argv)
	int argc;
	char **argv;
{

	object_directory = "..";
	search_path = (char *) 0;
	config_directory = (char *) 0;
	while ((argc > 1) && (argv[1][0] == '-')) {
		char		*c;

		argv++; argc--;
		c = &argv[0][1];
		if (*c == 'I') {
			int i;

			c++;
			if (search_path == 0) {
				i = strlen(c) + 1;
				search_path = (char *)malloc(i);
				strcpy(search_path, c);
			} else {
				i = strlen(c) + strlen(search_path) + 2;
				search_path = (char *)realloc(search_path, i);
				strcat(search_path, ":");
				strcat(search_path, c);
			}
			continue;
		}
		for (; *c; c++) {
			switch (*c) {
				case 'o':
					object_directory = argv[1];
					goto check_arg;

				case 'c':
					config_directory = argv[1];

				 check_arg:
				 	if (argv[1] == (char *) 0)
						goto usage_error;
					argv++; argc--;
					break;

				case 'p':
					profiling++;
					break;
				default:
					goto usage_error;
			}
		}
	}
	if (config_directory == (char *) 0)
		config_directory = "conf";
	if (search_path == (char *) 0)
		search_path = ".";
	if (argc != 2) {
		usage_error: ;
		fprintf(stderr, "usage: config [ -co <dir> ] [-I<dir>] [ -p ] sysname\n");
	}
	PREFIX = argv[1];
	if (freopen(argv[1], "r", stdin) == NULL) {
		perror(argv[1]);
		exit(2);
	}
	dtab = NULL;
	confp = &conf_list;
	opt = 0;
	if (yyparse())
		exit(3);
	switch (conftype) {

	case CONFTYPE_VAX:
		vax_ioconf();		/* Print ioconf.c */
		ubglue();		/* Create ubglue.s */
		break;

	case CONFTYPE_SUN:
		sun_ioconf();
		break;

	case CONFTYPE_SUN2:
	case CONFTYPE_SUN3:
	case CONFTYPE_SUN4:
		sun_ioconf();           /* Print ioconf.c */
		mbglue();               /* Create mbglue.s */
		break;

	case CONFTYPE_SUN4C:
		sun4c_ioconf();         /* Print ioconf.c */
		break;

	case CONFTYPE_SQT:
		sqt_ioconf();
		break;

	case CONFTYPE_PS2:
		ps2_ioconf();
		break;

	case CONFTYPE_MIPSY:
	case CONFTYPE_MIPS:
		mips_ioconf();
		break;

	case CONFTYPE_I386:
	case CONFTYPE_I860:
	case CONFTYPE_LUNA88K:
        case CONFTYPE_MAC2:
	case CONFTYPE_PC532:
	case CONFTYPE_ALPHA:
		empty_ioconf();
		break;

	default:
		printf("Specify conftype type, e.g. ``conftype vax''\n");
		exit(1);
	}

	makefile();			/* build Makefile */
	headers();			/* make a lot of .h files */
	exit(0);
}

/*
 * get_word
 *	returns EOF on end of file
 *	NULL on end of line
 *	pointer to the word otherwise
 */
char *
get_word(fp)
	register FILE *fp;
{
	static char line[80];
	register int ch;
	register char *cp;

	while ((ch = getc(fp)) != EOF)
		if (ch != ' ' && ch != '\t')
			break;
	if (ch == EOF)
		return ((char *)EOF);
	if (ch == '\n')
		return (NULL);
	if (ch == '|')
		return( "|");
	cp = line;
	*cp++ = ch;
	while ((ch = getc(fp)) != EOF) {
		if (isspace(ch))
			break;
		*cp++ = ch;
	}
	*cp = 0;
	if (ch == EOF)
		return ((char *)EOF);
	(void) ungetc(ch, fp);
	return (line);
}

/*
 * get_rest
 *	returns EOF on end of file
 *	NULL on end of line
 *	pointer to the word otherwise
 */
char *
get_rest(fp)
	register FILE *fp;
{
	static char line[80];
	register int ch;
	register char *cp;

	cp = line;
	while ((ch = getc(fp)) != EOF) {
		if (ch == '\n')
			break;
		*cp++ = ch;
	}
	*cp = 0;
	if (ch == EOF)
		return ((char *)EOF);
	return (line);
}

/*
 * prepend the path to a filename
 */
char *
path(file)
	char *file;
{
	register char *cp;

	cp = (char *)malloc((unsigned)(strlen(PREFIX)+
			       strlen(file)+
			       strlen(object_directory)+
			       3));
	(void) sprintf(cp, "%s/%s/%s", object_directory, PREFIX, file);
	return (cp);
}
