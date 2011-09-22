/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
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
 * Revision 2.4  93/11/17  16:09:50  dbg
 * 	Looks like the bus number is in a different place
 * 	than where I thought.
 * 	[93/09/14  12:09:39  af]
 * 
 * Revision 2.3  93/03/09  10:51:04  danner
 * 	Parse partition digit too.
 * 	[93/03/05            af]
 * 
 * Revision 2.2  93/02/05  07:59:34  danner
 * 	Rewritten, old code did not fly.
 * 	[93/02/02            af]
 * 	Created, from rvb's version for mips.
 * 	[92/06/07            af]
 * 
 */
/*
 *	File: swapgeneric.c
 * 	Author: Alessandro Forin and Robert V. Baron,
 *		at Carnegie Mellon University
 *	Date:	6/92
 *
 *	Define boot device for default pager.
 */

#include <mach/std_types.h>
#include <alpha/prom_interface.h>
#include <sys/reboot.h>

#define LEN 80
static char root_name_space[LEN];
char *root_name = root_name_space;	/* the name of the bootdevice */

int askme;				/* maybe set by parse_args, or by
					 * patching the kernel */

static boolean_t isdigit(const char c) { return ((c) >= '0' && (c) <= '9');}

/*
 * If the variable "askme" is set, enter a dialogue with the
 * user to get the name of the root device.  We only check
 * for a legal syntax here, too bad if she screws up.
 */
static char *examples[] = {	/* some examples of legal device names */
	"rz", 0
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

	name = &root_name[2];		/* check unit num */
	c = *name++;
	if (isdigit(c)) {
		c = *name++;
		if (isdigit(c))
			c = *name++;
					/* check part num */
		if (c && c >= 'a' && c <= 'h')
			return;		/* all is well */
		printf("bad partition number\n");
	} else
		printf("bad/missing unit number\n");

	printf("bad root specification, use something like:");
	for (gc = examples; *gc; gc++)
		printf(" %s%%d[a-h]", *gc);
	printf("\n");
	goto retry;
}


extern char *index( char *str, char c);
extern char *rindex( char *str, char c);

/*
 *	Object:
 *		set_root_name			EXPORTED function
 *
 *		Decide where we were booted from
 *
 */
set_root_name()
{
	char	booted_dev[128];

	prom_getenv( PROM_E_BOOTED_DEV, booted_dev, sizeof(booted_dev) );

/*dprintf("Booted dev is %s\n", booted_dev);*/
	switch (alpha_get_system_type()) {

	case SYSTEM_TYPE_ADU:
		if (adu_parse_root( booted_dev ))
			goto no_se;
		break;

	case SYSTEM_TYPE_FLAMINGO:
		if (flamingo_parse_root( booted_dev ))
			goto no_se;
		break;

	default:
no_se:
		mig_strncpy(root_name, "rz0a", 5);
		dprintf("Root on %s.\n", root_name);
	}
}

/*
 * utils
 */
static char *
skipnum(const char *p)
{
	while (*p != ' ') p++;
	while (*p == ' ') p++;
	return (char *)p;
}

/*
 * Parse Flamingo-style bootdev specification
 */
flamingo_parse_root( char *spec)
{
	char *p;
	int unit, tgtid, part;

	p = rindex(spec, ' ') + 1;
	if (strcmp(p, "FLAMG-IO"))
		return 1;

	p = index( spec, ' ');
	*p++ = 0;

	if (strcmp(spec, "SCSI"))
		return 1;

	/* dont know what this field is */
	p = skipnum(p);

	/* skip initiator id */
	p = skipnum(p);

	/* take bus number */
	unit = *p - '0';
	p = skipnum(p);

	/* skip one more numeric field (what is it) */
	p = skipnum(p);

	/* take target id */
	tgtid = (*p - '0') + (unit * 8);

	/* partition. I am assuming its the last digit of
	   the "dka200", but "dka201" gets the same boot
	   block with the OSF boot ?  My own bootblock
	   does it this way anyways... so there. */
	part = p[2] - '0';
	
	p = root_name;
	*p++ = 'r';
	*p++ = 'z';
	if (tgtid > 9) {
	    *p++ = '0' + (tgtid / 10);
	    tgtid %= 10;
	}
	*p++ = '0' + tgtid;

	/* ? partition ? */
	*p++ = 'a' + part;
	*p = 0;

	return 0;
}

/*
 * Parse ADU-type bootdev specification
 */
adu_parse_root( char *spec)
{
	/* ???? */
	mig_strncpy( root_name, "rz2a", 5);
	return 0;
}

