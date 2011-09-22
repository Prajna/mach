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
 * $Log:	mkheaders.c,v $
 * Revision 2.9  93/05/10  17:48:15  rvb
 * 	Fix include to use < vs " for new ode shadowing
 * 	[93/05/10  10:35:49  rvb]
 * 
 * Revision 2.8  93/02/04  13:47:48  mrt
 * 	Updated copyright.
 * 		Additions for Sparc (sun4) port. - berman
 * 	[93/01/24            mrt]
 * 
 * Revision 2.7  92/05/04  11:28:26  danner
 * 	Increase length of string buffers.
 * 	[92/05/03            danner]
 * 
 * Revision 2.6  92/04/09  09:37:20  rpd
 * 	Removed gratuitous luna88k differences.
 * 	[92/04/09            rpd]
 * 
 * Revision 2.5  91/07/08  16:59:33  danner
 * 	Luna88k support.
 * 	[91/06/26            danner]
 * 
 * Revision 2.4  91/06/19  11:58:45  rvb
 * 	cputypes.h->platforms.h
 * 	[91/06/12  13:46:08  rvb]
 * 
 * Revision 2.3  91/02/05  17:53:24  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:52:14  mrt]
 * 
 * Revision 2.2  90/08/27  22:09:46  dbg
 * 	Merged old CMU changes into Tahoe release.
 * 	[90/07/10            dbg]
 * 
 * Revision 2.1  89/08/03  16:54:43  rwd
 * Created.
 * 
 * Revision 2.5  89/02/25  19:22:14  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.4  89/02/07  22:55:58  mwyoung
 * Code cleanup cataclysm.
 * 
 * Revision 2.3  89/01/23  22:23:10  af
 * 	Improvement from mips
 * 	[89/01/09            rvb]
 * 
 * 03-Mar-88  Jonathan J. Chew (jjc) at Carnegie-Mellon University
 *	Made changes for Sun 4.
 *
 * 05-Jun-86  Jonathan J. Chew (jjc) at Carnegie-Mellon University
 *	Merged in changes for Sun 2 and 3.
 *
 *  4-Oct-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Changed all references of CMUCS to CMU.
 *
 * 25-Jan-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Upgraded to 4.3.
 *
 * 25-Jul-85  Mike Accetta (mja) at Carnegie-Mellon University
 *	Fixed bug in tomacro() which neglected to verify a lower case
 *	character before converting to upper case.
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
static char sccsid[] = "@(#)mkheaders.c	5.5 (Berkeley) 6/18/88";
#endif /* not lint */

/*
 * Make all the .h files for the optional entries
 */

#include <stdio.h>
#include <ctype.h>
#include <config.h>
#include <gram.h>

char	*toheader();	/* forward */

headers()
{
	register struct file_list *fl;

	for (fl = ftab; fl != 0; fl = fl->f_next)
		if (fl->f_needs != 0)
			do_count(fl->f_needs, fl->f_needs, 1);
}

/*
 * count all the devices of a certain type and recurse to count
 * whatever the device is connected to
 */
do_count(dev, hname, search)
	register char *dev, *hname;
	int search;
{
	register struct device *dp, *mp;
	register int count;

	for (count = 0,dp = dtab; dp != 0; dp = dp->d_next)
		if (dp->d_unit != -1 && eq(dp->d_name, dev)) {
			/*
			 * Avoid making .h files for bus types on sun machines
			 */
			if ((conftype == CONFTYPE_SUN2 ||
			     conftype == CONFTYPE_SUN3 ||
			     conftype == CONFTYPE_SUN4 ||
			     conftype == CONFTYPE_SUN4C)
			    && dp->d_conn == TO_NEXUS){
				return;
			}
			if (dp->d_type == PSEUDO_DEVICE) {
				count =
				    dp->d_slave != UNKNOWN ? dp->d_slave : 1;
				if (dp->d_flags)
					dev = NULL;
				break;
			}
                        if (conftype != CONFTYPE_SUN2 &&
			    conftype != CONFTYPE_SUN3 &&
			    conftype != CONFTYPE_SUN4 &&
			    conftype != CONFTYPE_SUN4C)
				/* avoid ie0,ie0,ie1 setting NIE to 3 */
			count++;
			/*
			 * Allow holes in unit numbering,
			 * assumption is unit numbering starts
			 * at zero.
			 */
			if (dp->d_unit + 1 > count)
				count = dp->d_unit + 1;
			if (search) {
				mp = dp->d_conn;
                                if (mp != 0 && mp != TO_NEXUS &&
				    mp->d_conn != TO_NEXUS) {
                                        /*
					 * Check for the case of the
					 * controller that the device
					 * is attached to is in a separate
					 * file (e.g. "sd" and "sc").
					 * In this case, do NOT define
					 * the number of controllers
					 * in the hname .h file.
					 */
					if (!file_needed(mp->d_name))
					    do_count(mp->d_name, hname, 0);
					search = 0;
				}
			}
		}
	do_header(dev, hname, count);
}

/*
 * Scan the file list to see if name is needed to bring in a file.
 */
file_needed(name)
	char *name;
{
	register struct file_list *fl;

	for (fl = ftab; fl != 0; fl = fl->f_next) {
		if (fl->f_needs && strcmp(fl->f_needs, name) == 0)
			return (1);
	}
	return (0);
}

do_header(dev, hname, count)
	char *dev, *hname;
	int count;
{
	char *file, *name, *inw, *toheader(), *tomacro();
	struct file_list *fl, *fl_head;
	FILE *inf, *outf;
	int inc, oldcount;

	file = toheader(hname);
	name = tomacro(dev?dev:hname) + (dev == NULL);
	inf = fopen(file, "r");
	oldcount = -1;
	if (inf == 0) {
		outf = fopen(file, "w");
		if (outf == 0) {
			perror(file);
			exit(1);
		}
		fprintf(outf, "#define %s %d\n", name, count);
		(void) fclose(outf);
		return;
	}
	fl_head = 0;
	for (;;) {
		char *cp;
		if ((inw = get_word(inf)) == 0 || inw == (char *)EOF)
			break;
		if ((inw = get_word(inf)) == 0 || inw == (char *)EOF)
			break;
		inw = ns(inw);
		cp = get_word(inf);
		if (cp == 0 || cp == (char *)EOF)
			break;
		inc = atoi(cp);
		if (eq(inw, name)) {
			oldcount = inc;
			inc = count;
		}
		cp = get_word(inf);
		if (cp == (char *)EOF)
			break;
		fl = (struct file_list *) malloc(sizeof *fl);
		fl->f_fn = inw;
		fl->f_type = inc;
		fl->f_next = fl_head;
		fl_head = fl;
	}
	(void) fclose(inf);
	if (count == oldcount) {
		for (fl = fl_head; fl != 0; fl = fl->f_next)
			free((char *)fl);
		return;
	}
	if (oldcount == -1) {
		fl = (struct file_list *) malloc(sizeof *fl);
		fl->f_fn = name;
		fl->f_type = count;
		fl->f_next = fl_head;
		fl_head = fl;
	}
	unlink(file);
	outf = fopen(file, "w");
	if (outf == 0) {
		perror(file);
		exit(1);
	}
	for (fl = fl_head; fl != 0; fl = fl->f_next) {
		fprintf(outf, "#define %s %d\n",
		    fl->f_fn, count ? fl->f_type : 0);
		free((char *)fl);
	}
	(void) fclose(outf);
}

/*
 * convert a dev name to a .h file name
 */
char *
toheader(dev)
	char *dev;
{
	static char hbuf[300];

	(void) strcpy(hbuf, path(dev));
	(void) strcat(hbuf, ".h");
	return (hbuf);
}

/*
 * convert a dev name to a macro name
 */
char *tomacro(dev)
	register char *dev;
{
	static char mbuf[300];
	register char *cp;

	cp = mbuf;
	*cp++ = 'N';
	while (*dev)
		if (!islower(*dev))
			*cp++ = *dev++;
		else
			*cp++ = toupper(*dev++);
	*cp++ = 0;
	return (mbuf);
}
