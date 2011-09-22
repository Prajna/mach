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
 * $Log:	gram.y,v $
 * Revision 2.3  93/05/10  17:48:04  rvb
 * 	Fix include to use < vs " for new ode shadowing
 * 	[93/05/10  10:35:19  rvb]
 * 
 * Revision 2.2  93/02/04  13:48:09  mrt
 * 	PROFILE is now a keyword that can be set in the configuration
 * 	file and has the same effect as "-p" on the command line. (rvb).
 * 	Added PS/2 code from IBM - by Prithvi
 * 	Removed ROMP and MMAX conftyptes - by mrt
 * 		Lots of changes and additions for Sparc (sun4) port of MACH 3.0
 * 	    -by Berman
 * 	Renamed from config.y to gram.y and condensed history. -by mrt
 * 	[93/01/21            danner]
 * 
 * Revision 2.13  93/01/14  17:56:38  danner
 * 	Added alpha.
 * 	[92/12/01            af]
 *
 * Revision 2.12  92/08/03  17:18:18  jfriedl
 * 	Added pc532.
 * 	[92/05/15            jvh]
 * 
 * Revision 2.11  92/01/24  18:15:53  rpd
 * 	Removed swap/root/dump/arg stuff.
 * 	[92/01/24            rpd]
 * 
 * Condensed history
 * 	Added mac2.
 * 	Luna88k support.
 * 	cputypes.h->platforms.h
 * 	Added the Sequent SYMMETRY, which shares code
 * 	with the ATbus-based i386 configurations.
 * 	Added i860 support.
 * 	Merge CMU changes into Tahoe release.
 * 	Removed HZ, TIMEZONE, MAXUSERS, MAXDSIZ, check_tz.
 */

/*
 * Copyright (c) 1988 Regents of the University of California.
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
 *	@(#)config.y	5.8 (Berkeley) 6/18/88
 */

%union {
	char	*str;
	int	val;
	struct	file_list *file;
	struct	idlst *lst;
}

%token	ADDRMOD
%token	AND
%token	ANY
%token	AT
%token	BIN
%token	COMMA
%token	CONFIG
%token	CONTROLLER
%token	CONF
%token	CSR
%token	DEVICE
%token	DISK
%token	DRIVE
%token	DST
%token	EQUALS
%token	FLAGS
%token	IDENT
%token	MAJOR
%token	MASTER
%token	MAXUSERS
%token	MAXDSIZ
%token	MBA
%token	MBII
%token	MINOR
%token	MINUS
%token	NEXUS
%token	OPTIONS
%token	MAKEOPTIONS
%token	PLATFORM
%token	PRIORITY
%token  PROFILE
%token	PSEUDO_DEVICE
%token	SEMICOLON
%token	SLAVE
%token	TRACE
%token	UBA
%token	VECTOR
%token	VME
%token  VME16D16
%token  VME24D16
%token  VME32D16
%token  VME16D32
%token  VME24D32
%token  VME32D32


/* the following are for the I386 */
%token  IRQ
%token  MEMORY
%token  PORT
%token  SPL
%token  HIGH
%token  UPTO
%token  LENGTH

/* following 3 are unique to CMU */
%token  LUN
%token	SLOT
%token	TAPE

/* following 4 are for scsi bus machines */
%token	DEVICE_DRIVER
%token	SCSIBUS
%token	TARGET
%token	INIT

%token	<str>	ID
%token	<val>	NUMBER
%token	<val>	FPNUMBER

%type	<str>	Save_id
%type	<str>	Opt_value
%type	<str>	Dev
%type	<lst>	Id_list
%type	<val>	Value

%{

#include <config.h>
#include <ctype.h>
#include <stdio.h>

struct	device cur;
struct	device *curp = 0;
char	*temp_id;
char	*val_id;
char	*malloc();

%}
%%
Configuration:
	Many_specs
		;

Many_specs:
	Many_specs Spec
		|
	/* lambda */
		;

Spec:
	Device_spec SEMICOLON
	      = { newdev(&cur); } |
	Config_spec SEMICOLON
		|
	TRACE SEMICOLON
	      = { do_trace = !do_trace; } |
	SEMICOLON
		|
	error SEMICOLON
		;

Config_spec:
	CONF Save_id
	    = {
		if (!strcmp($2, "vax")) {
			conftype = CONFTYPE_VAX;
			conftypename = "vax";
		} else if (!strcmp($2, "sun")) {
			/* default to Sun 3 */
			conftype = CONFTYPE_SUN3;
			conftypename = "sun3";
		} else if (!strcmp($2, "sun2")) {
			conftype = CONFTYPE_SUN2;
		} else if (!strcmp($2, "sun3")) {
			conftype = CONFTYPE_SUN3;
			conftypename = "sun3";
		} else if (!strcmp($2, "sun4")) {
			conftype = CONFTYPE_SUN4;
			conftypename = "sun4";
		} else if (!strcmp($2, "sun4c")) {
			conftype = CONFTYPE_SUN4C;
			conftypename = "sun4c";
		} else if (!strcmp($2, "sqt")) {
			conftype = CONFTYPE_SQT;
			conftypename = "i386";	/* assume SYMMETRY */
		} else if (!strcmp($2, "i")) {
			conftype = CONFTYPE_I386;
			conftypename = "i386";
		} else if (!strcmp($2, "i386")) {
			conftype = CONFTYPE_I386;
			conftypename = "i386";
		} else if (!strcmp($2, "ps2")) {
			conftype = CONFTYPE_PS2;
			conftypename = "i386";
		} else if (!strcmp($2, "mipsy")) {
			conftype = CONFTYPE_MIPSY;
			conftypename = "mipsy";
		} else if (!strcmp($2, "mips")) {
			conftype = CONFTYPE_MIPS;
			conftypename = "mips";
		} else if (!strcmp($2, "i860")) {
			conftype = CONFTYPE_I860;
			conftypename = "i860";
		} else if (!strcmp($2, "m88k")) {
			conftype = CONFTYPE_LUNA88K;
			conftypename = "m88k";
		} else if (!strcmp($2, "mac2")) {
			conftype = CONFTYPE_MAC2;
			conftypename = "mac2";
		} else if (!strcmp($2, "pc532")) {
			conftype = CONFTYPE_PC532;
			conftypename = "ns532";
		} else if (!strcmp($2, "alpha")) {
			conftype = CONFTYPE_ALPHA;
			conftypename = "alpha";
		} else
			yyerror("Unknown conftype type");
	      } |
	PLATFORM Save_id
	      = {
		struct platform *pp =
		    (struct platform *)malloc(sizeof (struct platform));
		pp->name = ns($2);
		pp->next = platform;
		platform = pp;
		free(temp_id);
	      } |
	OPTIONS Opt_list
		|
	MAKEOPTIONS Mkopt_list
		|
        PROFILE
  	      = { profiling = 1 ; }
                |
	IDENT ID
	      = { ident = ns($2); } |
	System_spec
		;

System_spec:
	  System_id
	;

System_id:
	  CONFIG Save_id
		= { mkconf($2); }
	;

Opt_list:
	Opt_list COMMA Option
		|
	Option
		;

Option:
	Save_id
	      = {
		struct opt *op = (struct opt *)malloc(sizeof (struct opt));
		op->op_name = ns($1);
		op->op_next = (struct opt *) 0;
		op->op_value = 0;
		if (opt == (struct opt *) 0)
			opt = op;
		else
			opt_tail->op_next = op;
		opt_tail = op;
		free(temp_id);
	      } |
	Save_id EQUALS Opt_value
	      = {
		struct opt *op = (struct opt *)malloc(sizeof (struct opt));
		op->op_name = ns($1);
		op->op_next = (struct opt *) 0;
		op->op_value = ns($3);
		if (opt == (struct opt *) 0)
			opt = op;
		else
			opt_tail->op_next = op;
		opt_tail = op;
		free(temp_id);
		if (val_id)
			free(val_id);
	      } ;

Opt_value:
	ID
	      = { $$ = val_id = ns($1); } |
	NUMBER
	      = { char nb[16];
	          (void) sprintf(nb, "%u", $1);
	      	  $$ = val_id = ns(nb);
	      } |
	/* lambda from MIPS -- WHY */
	      = { $$ = val_id = ns(""); }
	      ;

Save_id:
	ID
	      = { $$ = temp_id = ns($1); }
	;

Mkopt_list:
	Mkopt_list COMMA Mkoption
		|
	Mkoption
		;

Mkoption:
	Save_id EQUALS Opt_value
	      = {
		struct opt *op = (struct opt *)malloc(sizeof (struct opt));
		op->op_name = ns($1);
		op->op_next =  (struct opt *) 0;
		op->op_value = ns($3);
		if (mkopt == (struct opt *) 0)
			mkopt = op;
		else
			mkopt_tail->op_next = op;
		mkopt_tail = op;
		free(temp_id);
		if (val_id)
			free(val_id);
	      } ;

Dev:
	UBA
	      = { $$ = ns("uba"); } |
	MBA
	      = { $$ = ns("mba"); } |
        VME16D16
	      = {
		if (conftype != CONFTYPE_SUN2 && conftype != CONFTYPE_SUN3
		    && conftype != CONFTYPE_SUN4)
			yyerror("wrong conftype type for vme16d16");
		$$ = ns("vme16d16");
		} |
	VME24D16
	      = {
		if (conftype != CONFTYPE_SUN2 && conftype != CONFTYPE_SUN3
		    && conftype != CONFTYPE_SUN4)
			yyerror("wrong conftype type for vme24d16");
			$$ = ns("vme24d16");
		} |
	VME32D16
	      = {
		if (conftype != CONFTYPE_SUN3 && conftype != CONFTYPE_SUN4)

                        yyerror("wrong conftype type for vme32d16");
                $$ = ns("vme32d16");
                } |
        VME16D32
              = {
                if (conftype != CONFTYPE_SUN3  && conftype != CONFTYPE_SUN4)
                        yyerror("wrong conftype type for vme16d32");
                $$ = ns("vme16d32");
                } |
        VME24D32
              = {
		if (conftype != CONFTYPE_SUN3 && conftype != CONFTYPE_SUN4)
			yyerror("wrong conftype type for vme24d32");
		$$ = ns("vme24d32");
		} |
        VME32D32
	      = {
		if (conftype != CONFTYPE_SUN3 && conftype != CONFTYPE_SUN4)
			yyerror("wrong conftype type for vme32d32");
		$$ = ns("vme32d32");
		} |
	VME
	      = {
		if (conftype != CONFTYPE_MIPSY && conftype != CONFTYPE_MIPS)
			yyerror("wrong conftype type for vme");
			$$ = ns("vme");
		} |
	MBII
	      = {
		if (conftype != CONFTYPE_MIPSY && conftype != CONFTYPE_MIPS)
			yyerror("wrong conftype type for mbii");
			$$ = ns("mbii");
		} |
	ID
	      = { $$ = ns($1); }
	;

Device_spec:
	DEVICE Dev_name Dev_info Int_spec
	      = { cur.d_type = DEVICE; } |
	MASTER Dev_name Dev_info Int_spec
	      = { cur.d_type = MASTER; } |
	DISK Dev_name Dev_info Int_spec
	      = { cur.d_dk = 1; cur.d_type = DEVICE; } |
/* TAPE rule is unique to CMU */
	TAPE Dev_name Dev_info Int_spec
	      = { cur.d_type = DEVICE; } |
	CONTROLLER Dev_name Dev_info Int_spec
	      = { cur.d_type = CONTROLLER; } |
	DEVICE_DRIVER Init_dev Dev
	      = {
		cur.d_name = $3;
		cur.d_type = DEVICE_DRIVER; /* WAS PSEUDO_DEVICE */
		cur.d_unit = 0;	/* DID NOT EXIST */
		} |
	PSEUDO_DEVICE Init_dev Dev Pseudo_init
	      = {
		cur.d_name = $3;
		cur.d_type = PSEUDO_DEVICE;
		} |
	PSEUDO_DEVICE Init_dev Dev NUMBER Pseudo_init
	      = {
		cur.d_name = $3;
		cur.d_type = PSEUDO_DEVICE;
		cur.d_slave = $4;
		} |
	SCSIBUS Init_dev NUMBER AT Dev NUMBER
	      = {
			cur.d_name = "scsibus";
			cur.d_type = SCSIBUS;
			cur.d_unit = $3;
			cur.d_conn = connect($5, $6);
		} |
	SCSIBUS Init_dev NUMBER AT Dev
	      = {
			cur.d_name = "scsibus";
			cur.d_type = SCSIBUS;
			cur.d_unit = $3;
			cur.d_conn = connect($5, 0);
		};

Pseudo_init:
	INIT Save_id = {
		cur.d_init = $2;
		}
	| /* empty */
	;

Dev_name:
	Init_dev Dev NUMBER
	      = {
		cur.d_name = $2;
		if (eq($2, "mba"))
			seen_mba = 1;
		else if (eq($2, "uba"))
			seen_uba = 1;
		else if (eq($2, "mbii"))
			seen_mbii = 1;
		else if (eq($2, "vme"))
			seen_vme = 1;
		cur.d_unit = $3;
		};

Init_dev:
	/* lambda */
	      = { init_dev(&cur); };

Dev_info:
	Con_info Info_list
		|
	/* lambda */
		;

Con_info:
	AT Dev NUMBER
	      = {
		if (eq(cur.d_name, "mba") || eq(cur.d_name, "uba")
		    || eq(cur.d_name, "mbii") || eq(cur.d_name, "vme")) {
			(void) sprintf(errbuf,
			    "%s must be connected to a nexus", cur.d_name);
			yyerror(errbuf);
		}
		cur.d_conn = connect($2, $3);
		if (conftype == CONFTYPE_SQT)
			dev_param(&cur, "index", cur.d_unit);
		} |
/* AT SLOT NUMBER rule is unique to CMU */
	AT SLOT NUMBER
	      = { 
		check_slot(&cur, $3);
		cur.d_addr = $3;
		cur.d_conn = TO_SLOT; 
		 } |
	AT NEXUS NUMBER
	      = { check_nexus(&cur, $3); cur.d_conn = TO_NEXUS; } |
	AT SCSIBUS NUMBER
	      = {
		cur.d_conn = find_scsibus($3);
	      };

Info_list:
	Info_list Info
		|
	/* lambda */
		;

Info:
	CSR NUMBER
	      = {
		cur.d_addr = $2;
                if (conftype == CONFTYPE_SUN2 || conftype == CONFTYPE_SUN3
		    || conftype == CONFTYPE_SUN4)
			bus_encode($2, &cur);
		if (conftype == CONFTYPE_SQT) {
			dev_param(&cur, "csr", $2);
		}
		} |
	DRIVE NUMBER
	      = {
			cur.d_drive = $2;
			if (conftype == CONFTYPE_SQT) {
				dev_param(&cur, "drive", $2);
			}
		} |
	SLAVE NUMBER
	      = {
		if (cur.d_conn != 0 && cur.d_conn != TO_NEXUS &&
		    cur.d_conn->d_type == MASTER)
			cur.d_slave = $2;
		else
			yyerror("can't specify slave--not to master");
		} |
/* I386 */
        PORT NUMBER
              = {
                cur.d_addr = $2;
                } |
        DRIVE NUMBER
              = {
		cur.d_drive = $2;
                } |
        SLAVE NUMBER
              = {
                if (cur.d_conn != 0 && cur.d_conn != TO_NEXUS &&
                    cur.d_conn->d_type == MASTER)
                        cur.d_slave = $2;
                else
                        yyerror("can't specify slave--not to master");
                } |
/* MIPS */
	ADDRMOD NUMBER
	      = { cur.d_addrmod = $2; } |
/* LUN NUMBER rule is unique to CMU */
	LUN NUMBER
	      = {
		if ((cur.d_conn != 0) && (cur.d_conn != TO_SLOT) &&
			(cur.d_conn->d_type == CONTROLLER)) {
			cur.d_addr = $2; 
		}
		else {
			yyerror("device requires controller card");
		    }
		} |
	FLAGS NUMBER
	      = {
		cur.d_flags = $2;
		if (conftype == CONFTYPE_SQT) {
			dev_param(&cur, "flags", $2);
		}
	      } |
	TARGET NUMBER LUN NUMBER
	      = {
		cur.d_type = DEVICE;
		cur.d_slave = $2<<3 | $4;
	      } |

	BIN NUMBER
	      = { 
		 if (conftype != CONFTYPE_SQT)
			yyerror("bin specification only valid on Sequent Balance");
		 if ($2 < 1 || $2 > 7)  
			yyerror("bogus bin number");
		 else {
			cur.d_bin = $2;
			dev_param(&cur, "bin", $2);
		}
	       } |
/* I386 rules */
        MEMORY NUMBER UPTO NUMBER
              = { cur.d_mem_addr = $2;
                  cur.d_mem_length = $4 - $2 + 1;
                                        } |
        MEMORY NUMBER
              = { cur.d_mem_addr = $2; } |
        LENGTH NUMBER
              = { cur.d_mem_length = $2; } |
        SPL NUMBER
              = { cur.d_spl = $2; } |
        SPL HIGH
              = { cur.d_spl = 8; } |
        IRQ NUMBER
              = { cur.d_pri = $2; } |
/* I386 rules end */
	INIT Save_id
	       = { cur.d_init = $2; } |
	Dev Value
	      = {
		if (conftype != CONFTYPE_SQT)
			yyerror("bad device spec");
		dev_param(&cur, $1, $2);
		};

Value:
	NUMBER
	      |
	MINUS NUMBER
	      = { $$ = -($2); }
	;

Int_spec:
        Vec_spec
	      = { cur.d_pri = 0; } |
	PRIORITY NUMBER
	      = { cur.d_pri = $2; } |
        PRIORITY NUMBER Vec_spec
	      = { cur.d_pri = $2; } |
        Vec_spec PRIORITY NUMBER
	      = { cur.d_pri = $3; } |
	/* lambda */
		;

Vec_spec:
        VECTOR Id_list
	      = { cur.d_vec = $2; };


Id_list:
	Save_id
	      = {
		struct idlst *a = (struct idlst *)malloc(sizeof(struct idlst));
		a->id = $1; a->id_next = 0; $$ = a;
		a->id_vec = 0;
		} |
	Save_id Id_list =
		{
		struct idlst *a = (struct idlst *)malloc(sizeof(struct idlst));
	        a->id = $1; a->id_next = $2; $$ = a;
		a->id_vec = 0;
		} |
        Save_id NUMBER
	      = {
		struct idlst *a = (struct idlst *)malloc(sizeof(struct idlst));
		a->id_next = 0; a->id = $1; $$ = a;
		a->id_vec = $2;
		} |
        Save_id NUMBER Id_list
	      = {
		struct idlst *a = (struct idlst *)malloc(sizeof(struct idlst));
		a->id_next = $3; a->id = $1; $$ = a;
		a->id_vec = $2;
		};

%%

yyerror(s)
	char *s;
{
	fprintf(stderr, "config: line %d: %s\n", yyline, s);
}

/*
 * return the passed string in a new space
 */
char *
ns(str)
	register char *str;
{
	register char *cp;

	cp = malloc((unsigned)(strlen(str)+1));
	(void) strcpy(cp, str);
	return (cp);
}

/*
 * add a device to the list of devices
 */
newdev(dp)
	register struct device *dp;
{
	register struct device *np;

	np = (struct device *) malloc(sizeof *np);
	*np = *dp;
	if (curp == 0)
		dtab = np;
	else
		curp->d_next = np;
	curp = np;
	curp->d_next = 0;
}

/*
 * note that a configuration should be made
 */
mkconf(sysname)
	char *sysname;
{
	register struct file_list *fl, **flp;

	fl = (struct file_list *) malloc(sizeof *fl);
	fl->f_type = SYSTEMSPEC;
	fl->f_needs = sysname;
	fl->f_fn = 0;
	fl->f_next = 0;
	for (flp = confp; *flp; flp = &(*flp)->f_next)
		;
	*flp = fl;
	confp = flp;
}

/*
 * find the pointer to connect to the given device and number.
 * returns 0 if no such device and prints an error message
 */
struct device *
connect(dev, num)
	register char *dev;
	register int num;
{
	register struct device *dp;
	struct device *huhcon();

	if (num == QUES)
		return (huhcon(dev));
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if ((num != dp->d_unit) || !eq(dev, dp->d_name))
			continue;
		if (dp->d_type != CONTROLLER && dp->d_type != MASTER &&
		    (conftype == CONFTYPE_SUN4C &&
				dp->d_type != DEVICE_DRIVER)) {
			(void) sprintf(errbuf,
			    "%s connected to non-controller", dev);
			yyerror(errbuf);
			return (0);
		}
		return (dp);
	}
	(void) sprintf(errbuf, "%s %d not defined", dev, num);
	yyerror(errbuf);
	return (0);
}

/*
 * connect to an unspecific thing
 */
struct device *
huhcon(dev)
	register char *dev;
{
	register struct device *dp, *dcp;
	struct device rdev;
	int oldtype;

	/*
	 * First make certain that there are some of these to wildcard on
	 */
	for (dp = dtab; dp != 0; dp = dp->d_next)
		if (eq(dp->d_name, dev))
			break;
	if (dp == 0) {
		(void) sprintf(errbuf, "no %s's to wildcard", dev);
		yyerror(errbuf);
		return (0);
	}
	oldtype = dp->d_type;
	dcp = dp->d_conn;
	/*
	 * Now see if there is already a wildcard entry for this device
	 * (e.g. Search for a "uba ?")
	 */
	for (; dp != 0; dp = dp->d_next)
		if (eq(dev, dp->d_name) && dp->d_unit == -1)
			break;
	/*
	 * If there isn't, make one because everything needs to be connected
	 * to something.
	 */
	if (dp == 0) {
		dp = &rdev;
		init_dev(dp);
		dp->d_unit = QUES;
		dp->d_name = ns(dev);
		dp->d_type = oldtype;
		newdev(dp);
		dp = curp;
		/*
		 * Connect it to the same thing that other similar things are
		 * connected to, but make sure it is a wildcard unit
		 * (e.g. up connected to sc ?, here we make connect sc? to a
		 * uba?).  If other things like this are on the NEXUS or
		 * if they aren't connected to anything, then make the same
		 * connection, else call ourself to connect to another
		 * unspecific device.
		 */
		if (dcp == TO_NEXUS || dcp == 0)
			dp->d_conn = dcp;
		else
			dp->d_conn = connect(dcp->d_name, QUES);
	}
	return (dp);
}

init_dev(dp)
	register struct device *dp;
{

	dp->d_name = "OHNO!!!";
	dp->d_type = DEVICE;
	dp->d_conn = 0;
	dp->d_vec = 0;
	dp->d_addr = dp->d_pri = dp->d_flags = dp->d_dk = 0;
	dp->d_slave = dp->d_drive = dp->d_unit = UNKNOWN;
	if (conftype == CONFTYPE_SUN2 ||
	    conftype == CONFTYPE_SUN3 ||
	    conftype == CONFTYPE_SUN4 ||
	    conftype == CONFTYPE_SUN4C){
		dp->d_addr = UNKNOWN;
		dp->d_mach = dp->d_bus = 0;
	}
	if (conftype == CONFTYPE_MIPSY || conftype == CONFTYPE_MIPS){
		dp->d_addrmod = 0;
	}
/* I386 */
        if (conftype == CONFTYPE_I386) {
                dp->d_spl = 0;
                dp->d_mem_addr = 0;
                dp->d_mem_length = 0;
        }
}

/*
 * make certain that this is a reasonable type of thing to connect to a nexus
 */
check_nexus(dev, num)
	register struct device *dev;
	int num;
{

	switch (conftype) {
 
	case CONFTYPE_VAX:
		if (!eq(dev->d_name, "uba") && !eq(dev->d_name, "mba"))
			yyerror("only uba's and mba's should be connected to the nexus");
		if (num != QUES)
			yyerror("can't give specific nexus numbers");
		break;

	case CONFTYPE_SUN:
		if (!eq(dev->d_name, "mb"))
			yyerror("only mb's should be connected to the nexus");
		break;

        case CONFTYPE_SUN2:
		if (!eq(dev->d_name, "virtual") &&
		    !eq(dev->d_name, "obmem") &&
		    !eq(dev->d_name, "obio") &&
		    !eq(dev->d_name, "mbmem") &&
		    !eq(dev->d_name, "mbio") &&
		    !eq(dev->d_name, "vme16d16") &&
		    !eq(dev->d_name, "vme24d16")) {
			(void)sprintf(errbuf,
			    "unknown bus type `%s' for nexus connection on %s",
			    dev->d_name, conftypename);
			yyerror(errbuf);
		}
        case CONFTYPE_SUN3:
        case CONFTYPE_SUN4:
		if (!eq(dev->d_name, "virtual") &&
		    !eq(dev->d_name, "obmem") &&
		    !eq(dev->d_name, "obio") &&
		    !eq(dev->d_name, "mbmem") &&
		    !eq(dev->d_name, "mbio") &&
		    !eq(dev->d_name, "vme16d16") &&
		    !eq(dev->d_name, "vme24d16") &&
                    !eq(dev->d_name, "vme32d16") &&
		    !eq(dev->d_name, "vme16d32") &&
		    !eq(dev->d_name, "vme24d32") &&
		    !eq(dev->d_name, "vme32d32")) {
			(void)sprintf(errbuf,
			    "unknown bus type `%s' for nexus connection on %s",
			    dev->d_name, conftypename);
			yyerror(errbuf);
		}
		break;
	case CONFTYPE_SUN4C:
		if (!eq(dev->d_name, "somewhere")) {
			(void)sprintf(errbuf,
			    "unknown bus type `%s' for nexus connection on %s",
			    dev->d_name, conftypename);
			yyerror(errbuf);
		}
		break;
	case CONFTYPE_MIPSY:
	case CONFTYPE_MIPS:
		if (!eq(dev->d_name, "vme") && !eq(dev->d_name, "mbii"))
			yyerror("only vme's and mbii's should be connected to the nexus");
		if (num != QUES)
			yyerror("can't give specific nexus numbers");
		break;
	case CONFTYPE_LUNA88K:
		if (!eq(dev->d_name, "obio") && !eq(dev->d_name, "obmem") &&
				!eq(dev->d_name, "xp"))
			yyerror("only obio's, obmem's and xp's should be connected to the nexus");
		break;
        case CONFTYPE_I386:
                if (!i386_bus(dev->d_name))
                        yyerror("bus, atbus, mc, microchannel, isa or eisa should be connected to the nexus");
                break;
	}
}

/*
 * make certain that this is a reasonable type of thing to connect to a slot
 */

check_slot(dev, num)
	register struct device *dev;
	int num;
{

	switch (conftype) {

	case CONFTYPE_SQT:
		if (!eq(dev->d_name, "mbad") &&
		    !eq(dev->d_name, "zdc") &&
		    !eq(dev->d_name, "sec")) {
			(void)sprintf(errbuf,
			    "unknown bus type `%s' for slot on %s",
			    dev->d_name, conftypename);
			yyerror(errbuf);
		}
		break;

        case CONFTYPE_LUNA88K:
		if (!eq(dev->d_name, "obio") && !eq(dev->d_name, "obmem") &&
				!eq(dev->d_name, "xp"))
			yyerror("only obio's, obmem's and xp's should be connected to the nexus");
		break;

	default:
		yyerror("don't grok 'slot' for this conftype -- try 'nexus'.");
		break;
	}
}

/*
 * bi_info gives the magic number used to construct the token for
 * the autoconf code.  bi_max is the maximum value (across all
 * machine types for a given architecture) that a given "bus 
 * type" can legally have.
 */
struct bus_info {
	char    *bi_name;
	u_short bi_info;
	u_int   bi_max;
};

struct bus_info sun2_info[] = {
	{ "virtual",    0x0001, (1<<24)-1 },
	{ "obmem",      0x0002, (1<<23)-1 },
	{ "obio",       0x0004, (1<<23)-1 },
	{ "mbmem",      0x0010, (1<<20)-1 },
	{ "mbio",       0x0020, (1<<16)-1 },
	{ "vme16d16",   0x0100, (1<<16)-1 },
	{ "vme24d16",   0x0200, (1<<24)-(1<<16)-1 },
	{ (char *)0,    0,      0 }
};

struct bus_info sun3_info[] = {
	{ "virtual",    0x0001, 0xffffffff },
	{ "obmem",      0x0002, 0xffffffff },
	{ "obio",       0x0004, (1<<21)-1 },
	{ "vme16d16",   0x0100, (1<<16)-1 },
	{ "vme24d16",   0x0200, (1<<24)-(1<<16)-1 },
	{ "vme32d16",   0x0400, 0xffffffff-(1<<24)-1 },
	{ "vme16d32",   0x1000, (1<<16) },
	{ "vme24d32",   0x2000, (1<<24)-(1<<16)-1 },
	{ "vme32d32",   0x4000, 0xffffffff-(1<<24)-1 },
	{ (char *)0,    0,      0 }
};

struct bus_info sun4_info[] = {
	{ "virtual",    0x0001, 0xffffffff },
	{ "obmem",      0x0002, 0xffffffff },
	{ "obio",       0x0004, 0xffffffff },
	{ "vme16d16",   0x0100, (1<<16)-1 },
	{ "vme24d16",   0x0200, (1<<24)-(1<<16)-1 },
	{ "vme32d16",   0x0400, 0xfeffffff },
	{ "vme16d32",   0x1000, (1<<16) },
	{ "vme24d32",   0x2000, (1<<24)-(1<<16)-1 },
	{ "vme32d32",   0x4000, 0xfeffffff },
	{ (char *)0,    0,      0 }
};


struct bus_info sun4c_info[] = {
	{ (char *)0,	0,	0 }
};

bus_encode(addr, dp)
        u_int addr;
	register struct device *dp;
{
	register char *busname;
	register struct bus_info *bip;
	register int num;

	if (conftype == CONFTYPE_SUN2)
		bip = sun2_info;
	else if (conftype == CONFTYPE_SUN3)
		bip = sun3_info;
	else if (conftype == CONFTYPE_SUN4)
		bip = sun4_info;
	else if (conftype == CONFTYPE_SUN4C)
		bip = sun4c_info;
	else {
		yyerror("bad conftype type for bus_encode");
		exit(1);
	}

        if (dp->d_conn == TO_NEXUS || dp->d_conn == 0) {
		yyerror("bad connection");
		exit(1);
	}

        busname = dp->d_conn->d_name;
        num = dp->d_conn->d_unit;

        for (; bip->bi_name != 0; bip++)
                if (eq(busname, bip->bi_name))
                        break;

        if (bip->bi_name == 0) {
                (void)sprintf(errbuf, "bad bus type '%s' for conftype %s",
                        busname, conftypename);
                yyerror(errbuf);
        } else if (addr > bip->bi_max) {
                (void)sprintf(errbuf,
                        "0x%x exceeds maximum address 0x%x allowed for %s",
                        addr, bip->bi_max, busname);
                yyerror(errbuf);
        } else {
                dp->d_bus = bip->bi_info;       /* set up bus type info */
                if (num != QUES)
                        /*
                         * Set up cpu type since the connecting
                         * bus type is not wildcarded.
                         */
                        dp->d_mach = num;
        }
}

/*
 * find the pointer to a scsibus to connect this scsi device to.
 * returns 0 if no such device and prints an error message
 */
struct device *
find_scsibus(num)
	register int num;
{
	register struct device *dp;

	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if (dp->d_type != SCSIBUS || num != dp->d_unit)
			continue;
		return (dp);
	}
	(void)sprintf(errbuf, "scsibus %d not defined", num);
	yyerror(errbuf);
	return (0);
}
