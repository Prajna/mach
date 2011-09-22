/* 
 * Mach Operating System
 * Copyright (c) 1993-1989 Carnegie Mellon University
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
 * $Log:	swapgeneric.c,v $
 * Revision 2.14  93/01/14  18:12:03  danner
 * 	Fixed declarations.
 * 
 * Revision 2.13  92/04/01  19:35:33  rpd
 * 	Replaced mips_gets with machine-independent safe_gets.
 * 	[92/03/31            rpd]
 * 
 * Revision 2.12  92/02/19  16:47:01  elf
 * 	Forward 'askme' switch on to MI layer.
 * 	[92/02/10  17:20:59  af]
 * 
 * Revision 2.11  92/01/28  10:42:39  rpd
 * 	Moved prom_map_bootdev to end of set_root_name, for tftp.
 * 	[92/01/28            rpd]
 * 
 * Revision 2.10  91/10/09  16:15:05  af
 * 	correct gets problem
 * 
 * Revision 2.9  91/08/24  12:24:27  af
 * 	Better parsing of both old and new syntax, assumed
 * 	arguments are a bit more consistent.  Understands
 * 	unit numbers greater than 9 also.  Can tftp() boot,
 * 	but only if we have a local disk :-((
 * 	[91/08/02  03:27:33  af]
 * 
 * Revision 2.8  91/05/14  17:38:37  mrt
 * 	Correcting copyright
 * 
 * Revision 2.7  91/02/05  17:51:43  mrt
 * 	Added author notices
 * 	[91/02/04  11:24:57  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:29:43  mrt]
 * 
 * Revision 2.6  90/09/10  16:04:37  rpd
 * 	Apparently, with the new prom the only reliable way to get 
 * 	the boot image name is to query the prom environment vars.
 * 	[90/09/10  15:13:56  af]
 * 
 * Revision 2.5  90/09/09  23:21:20  rpd
 * 	Mods for new syntax used in new DEC proms for 3maxen.
 * 	[90/09/09  19:02:17  af]
 * 
 * Revision 2.4  90/08/07  22:30:25  rpd
 * 	Subtle bug: rz(0,2) turned out just rz2, but the U*x server really
 * 	wants rz2a.
 * 	[90/08/07  15:18:21  af]
 * 
 * Revision 2.3.2.1  90/06/11  11:26:35  af
 * 	Ok, so I did rewrite it.  We only work on a string basis, and it
 * 	does not really matter what we say anyways: the kernel will ask
 * 	for the swapfile later if not found.  What we pass on to the U*x
 * 	server is settable as a command line option, so there really is
 * 	not much we have to do here besides mapping from the prom syntax
 * 	into our own syntax for the name of the boot device.
 * 
 * Revision 2.3  89/12/08  19:48:28  rwd
 * 	I know, I should have rewritten it. I just fixed bugs for now.
 * 	We did not understand rz(0,1,0) properly.
 * 	[89/12/05            af]
 * 
 * Revision 2.2  89/11/29  14:15:23  af
 * 	Checked in temporarily, until I get around to rewrite this code.
 * 	[89/11/16  14:48:03  af]
 * 
 * Revision 2.3  89/08/08  21:49:45  jsb
 * 	A few errors in the code that guesses the root device iff
 * 	you had more than one disk.
 * 	[89/07/31            rvb]
 * 
 * Revision 2.2  89/05/31  12:32:13  rvb
 * 	Define "asmke" here. [af]
 * 
 * Revision 2.1  89/05/30  12:56:14  rvb
 * Created.
 * 
 */
/*
 *	File: swapgeneric.c
 * 	Author: Alessandro Forin and Robert V. Baron,
 *		at Carnegie Mellon University
 *	Date:	5/89
 *
 *	Define boot device for kernel, and consequently where
 *	the root filesystem and swapfile lives by default.
 */

#include <sys/varargs.h>
#include <mips/prom_interface.h>
#include <sys/reboot.h>

#define LPAR '('
#define RPAR ')'
#define SLASH '/'
#define COMMA ','

#define LEN 80
static char root_name_space[LEN];
char *root_name = root_name_space;	/* the name of the bootdevice */

/* extern decls */
extern char    *index();
/* forward declarations */
static set_rootname_oldstyle(void);
static set_rootname_newstyle(void);
static lrparse_rootname(char **);
static prom_map_bootdev(void);

int askme;				/* maybe set by parse_args, or by
					 * patching the kernel */

static isdigit(c) { return ((c) >= '0' && (c) <= '9');}

set_root_name(name)
	char *name;
{
	char           *cp;
	int		ret;

	/*
	 *	The prom gets us something like "rz(0,2,0)mach",
	 *	which we turn into "rz2a"
	 *
	 *	Syntax: dev '(' [ctlr ','] [unit ','] [part] ')' file
	 *	Becomes: dev unit part1
	 *	Where:
	 *		part1 == 'a' + part
	 */

	if (strcmp(name, "boot") == 0)
		name = prom_getenv("boot");

	mig_strncpy(root_name, name, LEN);	/* copy and terminate arg */

	/*
	 *	Weeeell, that's before DEC "fixed" the proms.  The joyful
	 *	motivation was to make them "less DEC-ish".  Sigh.
	 *	Anyways, now we get something like "5/rz2/mach" for slot5
	 *	of the TURBOchannel, driver "rz" unit 2 file "mach".
	 *	Which is just peachy, but we must cope with both old&new,
	 *	'course.
	 */

	if (isdigit(root_name[0]))
		/* New wave prom */
		ret = set_rootname_newstyle();
	else
		ret = set_rootname_oldstyle();
	if (ret)
		askme = 1;

	prom_map_bootdev();
}

static set_rootname_oldstyle(void)
{
	char 	       *cp;
	char           *c1p, part;
	int		unit, ctlr;
	static		rlparse_orootname();

	/* check */
	if ((cp = index(root_name, RPAR)) == 0)
		return 1;	/* sanity */

	/* find where the device name ends */
	if ((c1p = index(root_name, LPAR)) == 0)
		return 1;	/* sanity */

	/* parse left to right, what's missing defaults to zero */
	cp = c1p;

	ctlr = lrparse_rootname(&cp) - '0';		/* controller.. */
	unit = lrparse_rootname(&cp) - '0';		/* .. unit .. */
	part = lrparse_rootname(&cp);			/* .. partition, */
	if (isdigit(part))				/* ..(in letters).. */
		part += 'a' - '0';
	
	/* map (ctlr,unit) => (unit).  max 8 devs per ctlr, max 9 ctlrs */

	unit += ctlr * 8;
	if (unit > 9) {
		*c1p++ = '0' + (unit / 10);
		unit = unit % 10;
	}

	*c1p++ = '0' + unit;				/* set unit */

	*c1p++ = part;					/* set partition */
	*c1p = 0;
	return 0;
}

static lrparse_rootname(
	register char	**cpp)
{
	register char	*cp = *cpp;
	register	val = '0';

	while (*cp != RPAR && *cp != COMMA) val = *cp++;	
	if (*cp == COMMA)
		cp++;
	*cpp = cp;
	if ((val == LPAR) || (val == COMMA))
		return '0';
	return val;
}


static set_rootname_newstyle(void)
{
	char 	       *cp;
	char           *c1p, *c2p, part;
	int		ctlr, unit;

	ctlr = *root_name - '0';
	if ((cp = index(root_name, SLASH)) == 0)
		return 1;
	*cp++ = 0;

	if ((c1p = index(cp, SLASH)) == 0)
		return 1;	/* sanity */
	*c1p = 0, c1p[1] = 0;
	root_name = cp;

	/*
	 * Find unit and partition
	 */
	for (c2p = cp; c2p < c1p && !isdigit(*c2p); c2p++) ;

	cp = c2p;
	unit = (isdigit(*c2p) ? *c2p++ : '0');

	part = (c2p < c1p ? *c2p : 'a');
	if (isdigit(part))
		part += 'a' - '0';

	/*
	 * Hacks, trying to make things meaningful to poor users
	 */
	if (ctlr > 2) /* the on-board SCSI then */
		ctlr = 0;
	else {
		ctlr += 1;
		ctlr = (ctlr * 8) + (unit - '0');
		unit = '0' + (ctlr % 10);
		ctlr /= 10;
	}

	/*
	 * Set unit and partition in root_name
	 */
	if (ctlr)
		*cp++ = '0' + ctlr;

	*cp++ = unit;
	*cp++ = part;
	*cp = 0;
	return 0;
}


static
prom_map_bootdev(void)
{
	if (strcmp("dkip", root_name) == 0)
		root_name += 2;		/* --> "ip" */
	if (strcmp("tftp", root_name) == 0) {
		/* someday will do it right */
		root_name += 2;
		root_name[0] = 'r';	/* --> "rz" */
		root_name[1] = 'z';
	}
}

/*
 * If the variable "askme" is set, enter a dialogue with the
 * user to get the name of the root device.  We only check
 * for a legal syntax here, too bad if he screws up.
 */
static char *examples[] = {	/* some examples of legal device names */
	"rz", "ip", 0
};

get_root_name()
{
	register char **gc, *name, c;

	if (!askme)
		return;
	boothowto |= RB_ASKNAME;
retry:
	root_name = root_name_space;
	bzero(root_name,LEN);

	printf("root device? ");

	safe_gets(root_name, sizeof root_name_space);
	prom_map_bootdev();

	name = &root_name[2];		/* check unit num */
	c = *name++;
	if (isdigit(c)) {
		c = *name++;
		if (isdigit(c))
			c = *name++;
					/* check part num */
		if (c && c >= 'a' && c <= 'h')
			goto found;
		printf("bad partition number\n");
	} else
		printf("bad/missing unit number\n");

	printf("bad root specification, use something like:");
	for (gc = examples; *gc; gc++)
		printf(" %s%%d[a-h]", *gc);
	printf("\n");
	goto retry;
found:
	set_root_name(root_name);
}
