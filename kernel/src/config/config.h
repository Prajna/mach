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
 * $Log:	config.h,v $
 * Revision 2.14  93/05/10  17:47:59  rvb
 * 	Avoid type conflicts #ifdef __386BSD__.
 * 	[93/05/04  16:38:44  rvb]
 * 
 * Revision 2.13  93/03/11  13:47:50  danner
 * 	u_long -> u_int
 * 	[93/03/10            danner]
 * 
 * Revision 2.12  93/02/04  13:47:30  mrt
 * 	16-Jan-93 Mary Thompson (mrt) at Carnegie-Mellon University
 * 	Updated for switch from single source_directory to search_path.
 * 		Integrated PS2 code from IBM - from Prithvi
 * 	Removed CONFTYPE_ROMP,MMAX
 * 		Added d_init and d_intr to struct device for Sparc (sun4) - from Berman
 * 		Condensed history.
 * 
 * Revision 2.11  93/01/14  17:56:32  danner
 * 	Added alpha.
 * 	[92/12/01            af]
 * 
 * Revision 2.10  92/08/03  17:18:11  jfriedl
 * 	Added pc532.
 * 	[92/05/15            jvh]
 * 
 * Revision 2.9  92/01/24  18:15:45  rpd
 * 	Removed swap/dump/arg/root stuff.
 * 	[92/01/24            rpd]
 * 
 * Condensed history
 * 	Luna88k support.
 * 	cputypes.h->platforms.h
 * 	Added i860 support.
 * 	Merged CMU changes into Tahoe release.
 * 	Removed timezone, hadtz, dst, maxusers, maxdsiz, CONFIGDEP.
 * 	Added fopenp, get_VPATH, VPATH declarations.
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
 *	@(#)config.h	5.8 (Berkeley) 6/18/88
 */

/*
 * Config.
 */

#include <stdio.h>
#include <sys/types.h>

struct file_list {
	struct	file_list *f_next;	
	char	*f_fn;			/* the name */
	u_char	f_type;			/* see below */
	u_char	f_flags;		/* see below */
	short	f_special;		/* requires special make rule */
	char	*f_needs;
	char	*f_extra;		/* stuff to add to make line */
};

/*
 * Types.
 */
#define DRIVER		1
#define NORMAL		2
#define	INVISIBLE	3
#define	PROFILING	4
#define	SYSTEMSPEC	5

/*
 * Attributes (flags).
 */
#define	CONFIGDEP	0x01	/* obsolete? */
#define	OPTIONSDEF	0x02	/* options definition entry */
#define ORDERED		0x04	/* don't list in OBJ's, keep "files" order */
#define SEDIT		0x08	/* run sed filter (SQT) */

/*
 * Maximum number of fields for variable device fields (SQT).
 */
#define	NFIELDS		10

struct	idlst {
	char	*id;
	struct	idlst *id_next;
	int	id_vec;		/* Sun interrupt vector number */
#ifdef	luna88k
	short	id_span;		/* ISI: distance to next vector */
#endif	luna88k
};

struct device {
	int	d_type;			/* CONTROLLER, DEVICE, bus adaptor */
	struct	device *d_conn;		/* what it is connected to */
	char	*d_name;		/* name of device (e.g. rk11) */
	struct	idlst *d_vec;		/* interrupt vectors */
	int	d_pri;			/* interrupt priority */
	int	d_addr;			/* address of csr */
	int	d_unit;			/* unit number */
	int	d_drive;		/* drive number */
	int	d_slave;		/* slave number */
#define QUES	-1	/* -1 means '?' */
#define	UNKNOWN -2	/* -2 means not set yet */
	int	d_dk;			/* if init 1 set to number for iostat */
	int	d_flags;		/* nlags for device init */
	struct	device *d_next;		/* Next one in list */
        u_short d_mach;                 /* Sun - machine type (0 = all)*/
        u_short d_bus;                  /* Sun - bus type (0 = unknown) */
	u_int	d_fields[NFIELDS];	/* fields values (SQT) */
	int	d_bin;			/* interrupt bin (SQT) */
	int	d_addrmod;		/* address modifier (MIPS) */
        int     d_spl;                  /* spl number (I386) */
        int     d_mem_addr;             /* memory address (I386) */
        int     d_mem_length;           /* memory address length (I386) */
        int     d_ctlr;                 /* controller number (I386) */
	char	*d_init;		/* name of initialization routine (sun4) */
	char	*d_intr;		/* name of interrupt routine (sun4) */
};
#define TO_NEXUS	(struct device *)-1
#define TO_SLOT		(struct device *)-1

struct config {
	char	*c_dev;
	char	*s_sysname;
};

/*
 * Config has a global notion of which machine type it is
 * configuring for.  It uses the name of the target machine in choosing
 * files and directories.  Thus if the name of the machine is ``vax'',
 * it will build from ``vax/template.mk'' and use ``../vax/inline''
 * in the makerules, etc.
 */
int	conftype;
char	*conftypename;
#define	CONFTYPE_VAX	1
#define	CONFTYPE_SUN	2
#define	CONFTYPE_SUN2	4
#define	CONFTYPE_SUN3	5
#define	CONFTYPE_SQT	7
#define CONFTYPE_SUN4	8
#define	CONFTYPE_I386	9
#define	CONFTYPE_PS2	10
#define CONFTYPE_MIPSY	11
#define	CONFTYPE_MIPS	12
#define	CONFTYPE_I860	13
#define	CONFTYPE_MAC2	14
#define	CONFTYPE_SUN4C	15
#define	CONFTYPE_LUNA88K	16
#define CONFTYPE_PC532	17
#define CONFTYPE_ALPHA	18

/*
 * For each machine, a set of CPU's may be specified as supported.
 * These and the options (below) are put in the C flags in the makefile.
 */
struct platform {
	char		 *name;
	struct	platform *next;
} *platform;

/*
 * In order to configure and build outside the kernel source tree,
 * we may wish to specify where the source tree lives.
 */

char *config_directory;
char *object_directory;

FILE *VPATHopen();

/*
 * A set of options may also be specified which are like CPU types,
 * but which may also specify values for the options.
 * A separate set of options may be defined for make-style options.
 */
struct opt {
	char	*op_name;
	char	*op_value;
	struct	opt *op_next;
}  *opt,
   *mkopt,
   *opt_tail,
   *mkopt_tail;

char	*ident;
char	*ns();
char	*tc();
char	*qu();
char	*get_word();
char	*path();
char	*raise();

int	do_trace;

#ifndef	__386BSD__
char	*index();
char	*rindex();
char	*malloc();
char	*realloc();
char	*strcpy();
char	*strcat();
#endif

#if	CONFTYPE_VAX
int	seen_mba, seen_uba;
#endif

int	seen_vme, seen_mbii;

struct	device *connect();
struct	device *dtab;
dev_t	nametodev();
char	*devtoname();

char	errbuf[80];
int	yyline;

struct	file_list *ftab, *conf_list, **confp;
char	*PREFIX;

int	profiling;

#define eq(a,b)	(!strcmp(a,b))

#ifdef	mips
#define DEV_MASK 0xf
#define	DEV_SHIFT  4
#else	mips
#define DEV_MASK 0x7
#define	DEV_SHIFT  3
#endif	mips
