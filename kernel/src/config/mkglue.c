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
 * $Log:	mkglue.c,v $
 * Revision 2.7  93/05/10  17:48:11  rvb
 * 	Fix include to use < vs " for new ode shadowing
 * 	[93/05/10  10:35:39  rvb]
 * 
 * Revision 2.6  93/02/04  13:47:42  mrt
 * 	Updated copyright.
 * 	[92/12/22            mrt]
 * 
 * Revision 2.5  91/07/08  16:59:29  danner
 * 	Luna88k support.
 * 	[91/06/26            danner]
 * 
 * Revision 2.4  91/06/19  11:58:41  rvb
 * 	cputypes.h->platforms.h
 * 	[91/06/12  13:46:04  rvb]
 * 
 * Revision 2.3  91/02/05  17:53:18  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:52:00  mrt]
 * 
 * Revision 2.2  90/08/27  22:09:40  dbg
 * 	Merged CMU changes into Tahoe release.
 * 	[90/08/16            dbg]
 * 
 * Revision 2.1  89/08/03  16:54:38  rwd
 * Created.
 * 
 * Revision 2.7  89/02/25  19:22:05  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.6  89/02/19  18:40:55  rpd
 * 	Removed printf of #include mach.h from mbglue.
 * 	[89/02/18            mrt]
 * 
 * Revision 2.5  89/02/07  22:56:04  mwyoung
 * Code cleanup cataclysm.
 * 
 * Revision 2.4  89/01/23  22:22:53  af
 * 	Changes for MIPS
 * 	[89/01/09            rvb]
 * 
 *  4-Apr-88  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Emit interrupt-counting code using a macro (that can be turned
 *	into whitespace for MACH).
 *
 * 06-Jun-86  Jonathan J. Chew (jjc) at Carnegie-Mellon University
 *	Added glue routines for Sun.
 *
 * 25-Jan-86  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Updated to 4.3.
 *
 * 30-Aug-85 David L. Black (dlb) at CMU.  Make user-time changes
 *	conditionally compiled.
 *
 * 5-Aug-85 David L. Black (dlb) at CMU.  Modified to do user-mode
 *	timing, also fixed interrupt counting problem in ubglue.
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
static char sccsid[] = "@(#)mkglue.c	5.6 (Berkeley) 6/18/88";
#endif /* not lint */

/*
 * Make the bus adaptor interrupt glue files.
 */
#include <stdio.h>
#include <config.h>
#include <gram.h>
#include <ctype.h>

/*
 * Create the UNIBUS interrupt vector glue file.
 */
ubglue()
{
	register FILE *fp, *gp;
	register struct device *dp, *mp;

	fp = fopen(path("ubglue.s"), "w");
	if (fp == 0) {
		perror(path("ubglue.s"));
		exit(1);
	}
	gp = fopen(path("ubvec.s"), "w");
	if (gp == 0) {
		perror(path("ubvec.s"));
		exit(1);
	}
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (mp != 0 && mp != (struct device *)-1 &&
		    !eq(mp->d_name, "mba")) {
			struct idlst *id, *id2;

			for (id = dp->d_vec; id; id = id->id_next) {
				for (id2 = dp->d_vec; id2; id2 = id2->id_next) {
					if (id2 == id) {
						dump_ubavec(fp, id->id,
						    dp->d_unit);
						break;
					}
					if (!strcmp(id->id, id2->id))
						break;
				}
			}
		}
	}
	dump_std(fp, gp);
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (mp != 0 && mp != (struct device *)-1 &&
		    !eq(mp->d_name, "mba")) {
			struct idlst *id, *id2;

			for (id = dp->d_vec; id; id = id->id_next) {
				for (id2 = dp->d_vec; id2; id2 = id2->id_next) {
					if (id2 == id) {
						dump_intname(fp, id->id,
							dp->d_unit);
						break;
					}
					if (!strcmp(id->id, id2->id))
						break;
				}
			}
		}
	}
	dump_ctrs(fp);
	(void) fclose(fp);
	(void) fclose(gp);
}

static int cntcnt = 0;		/* number of interrupt counters allocated */

/*
 * Print a UNIBUS interrupt vector.
 */
dump_ubavec(fp, vector, number)
	register FILE *fp;
	char *vector;
	int number;
{
	char nbuf[80];
	register char *v = nbuf;

	switch (conftype) {

	case CONFTYPE_VAX:
		(void) sprintf(v, "%s%d", vector, number);
		fprintf(fp, "\t.globl\t_X%s\n\t.align\t2\n_X%s:\n",
		    v, v);
		fprintf(fp,"\tTIM_PUSHR(0)\n");
		fprintf(fp, "\tincl\t_fltintrcnt+(4*%d)\n", cntcnt++);
		if (strncmp(vector, "dzx", 3) == 0)
			fprintf(fp, "\tmovl\t$%d,r0\n\tjmp\tdzdma\n\n", number);
		else {
			if (strncmp(vector, "uur", 3) == 0) {
				fprintf(fp, "#ifdef UUDMA\n");
				fprintf(fp, "\tmovl\t$%d,r0\n\tjsb\tuudma\n",
					    number);
				fprintf(fp, "#endif\n");
			}
			fprintf(fp, "\tpushl\t$%d\n", number);
			fprintf(fp, "\tcalls\t$1,_%s\n",vector);
			fprintf(fp, "\tCOUNT(V_INTR)\n");
			fprintf(fp, "\tTSREI_POPR\n");
		}
		break;

	case CONFTYPE_MIPSY:
	case CONFTYPE_MIPS:
		/*
		 * Actually, we should never get here!
		 * Main does not even call ubglue.
		 */
		if (strncmp(vector, "dzx", 3) == 0)
			fprintf(fp, "\tDZINTR(%s,%d)\n", vector, number);
		else
			fprintf(fp, "\tDEVINTR(%s,%d)\n", vector, number);
		break;
	}

}

static	char *vaxinames[] = {
	"clock", "cnr", "cnx", "tur", "tux",
	"mba0", "mba1", "mba2", "mba3",
	"uba0", "uba1", "uba2", "uba3"
};

#ifdef	luna88k
static	char *luna88kinames[] = {
	"clock", "uart", "xp5"
};
#endif	luna88k

static	struct stdintrs {
	char	**si_names;	/* list of standard interrupt names */
	int	si_n;		/* number of such names */
} stdintrs[] = {
	{ vaxinames, sizeof (vaxinames) / sizeof (vaxinames[0]) },
#ifdef	luna88k
	{ luna88kinames, sizeof (luna88kinames) / sizeof (luna88kinames[0]) },
#endif	luna88k
};
/*
 * Start the interrupt name table with the names
 * of the standard vectors not directly associated
 * with a bus.  Also, dump the defines needed to
 * reference the associated counters into a separate
 * file which is prepended to locore.s.
 */
dump_std(fp, gp)
	register FILE *fp, *gp;
{
	register struct stdintrs *si = &stdintrs[conftype-1];
	register char **cpp;
	register int i;

	fprintf(fp, "\n\t.globl\t_intrnames\n");
	fprintf(fp, "\n\t.globl\t_eintrnames\n");
	fprintf(fp, "\t.data\n");
	fprintf(fp, "_intrnames:\n");
	cpp = si->si_names;
	for (i = 0; i < si->si_n; i++) {
		register char *cp, *tp;
		char buf[80];

		cp = *cpp;
		if (cp[0] == 'i' && cp[1] == 'n' && cp[2] == 't') {
			cp += 3;
			if (*cp == 'r')
				cp++;
		}
		for (tp = buf; *cp; cp++)
			if (islower(*cp))
				*tp++ = toupper(*cp);
			else
				*tp++ = *cp;
		*tp = '\0';
		fprintf(gp, "#define\tI_%s\t%d\n", buf, i*sizeof (long));
		fprintf(fp, "\t.asciz\t\"%s\"\n", *cpp);
		cpp++;
	}
}

dump_intname(fp, vector, number)
	register FILE *fp;
	char *vector;
	int number;
{
	register char *cp = vector;

	fprintf(fp, "\t.asciz\t\"");
	/*
	 * Skip any "int" or "intr" in the name.
	 */
	while (*cp)
		if (cp[0] == 'i' && cp[1] == 'n' &&  cp[2] == 't') {
			cp += 3;
			if (*cp == 'r')
				cp++;
		} else {
			putc(*cp, fp);
			cp++;
		}
	fprintf(fp, "%d\"\n", number);
}

/*
 * Reserve space for the interrupt counters.
 */
dump_ctrs(fp)
	register FILE *fp;
{
	struct stdintrs *si = &stdintrs[conftype-1];

	fprintf(fp, "_eintrnames:\n");
	fprintf(fp, "\n\t.globl\t_intrcnt\n");
	fprintf(fp, "\n\t.globl\t_eintrcnt\n");
#ifdef	luna88k
	if (conftype == CONFTYPE_LUNA88K)
		fprintf(fp, "\t.align\t1\n");		/* ASSEMBLER KLUDGE */
	else
#endif	luna88k
	fprintf(fp, "\t.align 2\n");
	fprintf(fp, "_intrcnt:\n");
	fprintf(fp, "\t.space\t4 * %d\n", si->si_n);
	fprintf(fp, "_fltintrcnt:\n");
	fprintf(fp, "\t.space\t4 * %d\n", cntcnt);
	fprintf(fp, "_eintrcnt:\n\n");
	fprintf(fp, "\t.text\n");
}

/*
 * Routines for making Sun mb interrupt file mbglue.s
 */

/*
 * print an interrupt handler for mainbus
 */
dump_mb_handler(fp, vec, number)
	register FILE *fp;
	register struct idlst *vec;
	int number;
{
	fprintf(fp, "\tVECINTR(_X%s%d, _%s, _V%s%d)\n",
		vec->id, number, vec->id, vec->id, number);
}

mbglue()
{
	register FILE *fp;
	char *name = "mbglue.s";

	fp = fopen(path(name), "w");
	if (fp == 0) {
		perror(path(name));
		exit(1);
	}
	fprintf(fp, "#include <machine/asm_linkage.h>\n\n");
	glue(fp, dump_mb_handler);
	(void) fclose(fp);
}

glue(fp, dump_handler)
	register FILE *fp;
	register int (*dump_handler)();
{
	register struct device *dp, *mp;

	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (mp != 0 && mp != (struct device *)-1 &&
		    !eq(mp->d_name, "mba")) {
			struct idlst *vd, *vd2;

			for (vd = dp->d_vec; vd; vd = vd->id_next) {
				for (vd2 = dp->d_vec; vd2; vd2 = vd2->id_next) {
					if (vd2 == vd) {
						(void)(*dump_handler)
							(fp, vd, dp->d_unit);
						break;
					}
					if (!strcmp(vd->id, vd2->id))
						break;
				}
			}
		}
	}
}
