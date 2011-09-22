/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	parse_args.c,v $
 * Revision 2.14  93/01/14  17:52:15  danner
 * 	Added monitor# command line options.
 * 	[92/11/30            jtp]
 * 
 * Revision 2.13  92/05/05  10:46:59  danner
 * 	Added setting of scsi-ids to use on the various busses.
 * 	[92/04/14  12:08:11  af]
 * 
 * Revision 2.12  92/02/19  16:46:58  elf
 * 	Changed default init switches to go multiuser.
 * 	Removed SCC.
 * 	[92/01/30            af]
 * 
 * Revision 2.11  92/01/03  20:25:24  dbg
 * 	Remove startup_name.
 * 	[91/09/13            dbg]
 * 
 * Revision 2.10  91/08/24  12:23:40  af
 * 	Added rcline, and temporarily SCC .
 * 	[91/08/02  03:11:11  af]
 * 
 * Revision 2.9  91/05/18  14:36:15  rpd
 * 	Replaced zalloc_physical with zdata_size.
 * 	[91/03/24            rpd]
 * 
 * Revision 2.8  91/05/14  17:36:39  mrt
 * 	Correcting copyright
 * 
 * Revision 2.7  91/02/05  17:50:09  mrt
 * 	Added author notices
 * 	[91/02/04  11:24:14  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:28:04  mrt]
 * 
 * Revision 2.6  90/09/09  23:21:07  rpd
 * 	Removed environ: use prom_getenv() if you need to query the prom.
 * 	[90/09/05            af]
 * 
 * Revision 2.5  90/08/27  22:08:45  dbg
 * 	Drop use_kdb.
 * 	[90/08/17  23:43:29  af]
 * 
 * Revision 2.4  90/08/07  22:29:55  rpd
 * 	Removed led_period, as set_leds() is gone.
 * 	[90/08/07  15:22:12  af]
 * 
 * Revision 2.2.3.1  90/05/30  16:55:58  af
 * 	Removed led_period, as set_leds() is gone.
 * 
 * Revision 2.3  90/06/02  15:03:07  rpd
 * 	Purged pmap_reclaim_ptepages.
 * 	[90/04/12            rpd]
 * 
 * Revision 2.2  89/11/29  14:14:56  af
 * 	Created.
 * 	[89/10/06            af]
 */
/*
 *	File: parse_args.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Command line parsing for the Mach kernel.
 *
 */

/*
 *	Switches are names of kernel variables, which can be
 *	either simply set to 1 (flags) or set to some specific
 *	value provided in the comand line.
 *
 *	Example:
 *		rz()mach boothowto=0xc memlim=0xc00000
 *
 *	Certain variables might also require some special processing
 *	before and/or after being set.  For this reason we invoke
 *	a function that is associated with the variable, passing the
 *	desired new value as argument.  Responsibility for setting
 *	the variable to the desired value remains with the function.
 *	Yes, this is intentionally very much like Lisp's setf.
 *
 */

#include <mach/boolean.h>


typedef struct {
	char *name;			/* variable's name */
	unsigned *address;		/* variable's address */
	unsigned (*setf)();		/* how to modify the variable */
} switch_table;

unsigned default_setf();		/* default setf method */
unsigned string_setf();			/* for strings */
unsigned uchar_setf();			/* for unsigned chars */


/*
 *	Object:
 *		kernel_switch			EXPORTED structure
 *
 *	Kernel arguments table.
 *	Add/remove variables from here and create the setf methods
 *	in the module that exports that variable, if necessary.
 *	For simple integers use the default_setf() method.
 *
 *	This table is exported even if it need not, so that it
 *	can be patched with ADB if necessary.
 *
 */
extern unsigned
	askme, xprflags, zdata_size, rcline,
	memlimit, boothowto, pmap_debug, trap_debug,
	tlbpid_recycle_fifo, tlbpid_flushes, page_size;
extern unsigned char
	scsi_initiator_id[];

unsigned char
	*monitor_types[4];

extern	unsigned		/* cheat on pointers: C cannot init unions */
	root_name;

switch_table kernel_switch[] = {
	{ "askme",			&askme,			default_setf },
	{ "boothowto",			&boothowto,		default_setf },
	{ "memlimit",			&memlimit,		default_setf },
	{ "monitor0",			(unsigned *)&monitor_types[0], 
	      							string_setf },
	{ "monitor1",			(unsigned *)&monitor_types[1],
	  							string_setf },
	{ "monitor2",			(unsigned *)&monitor_types[2],
	  							string_setf },
	{ "monitor3",			(unsigned *)&monitor_types[3],
	  							string_setf },
	{ "page_size",			&page_size,		default_setf },
	{ "pmap_debug",			&pmap_debug,		default_setf },
	{ "rcline",			&rcline,		default_setf },
	{ "root",			&root_name,		string_setf  },
	{ "scsi0",			(unsigned *)(& scsi_initiator_id[0]),
								uchar_setf   },
	{ "scsi1",			(unsigned *)(& scsi_initiator_id[1]),
								uchar_setf   },
	{ "tlbpid_flushes",		&tlbpid_flushes,	default_setf },
	{ "tlbpid_recycle_fifo",	&tlbpid_recycle_fifo,	default_setf },
	{ "trap_debug", 		&trap_debug,		default_setf },
	{ "xprflags",			&xprflags,		default_setf },
	{ "zdata_size", 		&zdata_size,		default_setf },
	{ 0, 0, 0 }
};


/*
 *	Object:
 *		parse_args			EXPORTED function
 *
 *	Kernel argument parsing function
 *
 *	Invoked at kernel startup, _before_ configuration
 */
static char init_args[12] = "-xx";

parse_args(argc, argv)
	int		argc;
	char	      **argv;
{
	static char    *isa_switch();
	register int    i;
	switch_table   *variable;
	char           *vptr;

	for (i = 1; i < argc; i++) {

		if (argv[i][0] == '-') {
			dprintf("Init args: %s\n", argv[i]);
			strncpy(init_args, argv[i], sizeof init_args - 1);
			continue;
		}
		if (vptr = isa_switch(argv[i], &variable)) {
			dprintf("Boot option: %s", variable->name);
			(variable->setf) (variable->address, vptr);
			dprintf("\n");
		}
	}
}


/*
 *	Object:
 *		isa_switch			LOCAL function
 *
 *	Attempt a match between a token and a switch.
 *
 *	Returns 0 if no match, a pointer to the ascii
 *	representation of the desired value for the switch
 *	in case of a perfect match.
 *	"foo" will match with "foo" "foo=" "foo=baz"
 */
static char*
isa_switch(str,swptr)
	char 		 *str;
	switch_table	**swptr;
{
	register switch_table	*sw = kernel_switch;
	register char 		*cp0, *cp1, c;

	if (!str)			/* sanity */
		return FALSE;
	while (cp0 = sw->name) {
		cp1 = str;
		/* This is faster if the table is alphabetically ordered */
		while ((c = *cp0++) == *cp1++)
			if (c == 0)
				break;	/* a true prefix */
		if (c == 0) {
			/* prefix match, but we want a full match */
			*swptr = sw;
			if ((c = *--cp1) == 0)
				return cp1;
			if (c == '=')
				return cp1 + 1;
		}
		sw++;
	}

	return (char*)0;
}


/*
 *	Object:
 *		default_setf			EXPORTED function
 *
 *	This is the "default" setf method.
 *	Understands integers, which default to "1"
 *	if no value is provided.
 */
unsigned
default_setf(address, new_val)
	unsigned       *address;
	char	       *new_val;
{
	unsigned	binval;
	int		string_to_int();

	if (*new_val == 0)
		binval = 1;
	else
		binval = string_to_int(new_val);

	dprintf(" = x%x", binval);
	return *address = binval;
}


/*
 *	Object:
 *		uchar_setf			EXPORTED function
 *
 *	Understands single-byte integers, no default.
 */
unsigned
uchar_setf(address, new_val)
	unsigned char  *address;
	char	       *new_val;
{
	unsigned char	binval;

	binval = string_to_int(new_val);

	dprintf(" = x%x", binval);
	return *address = binval;
}


/*
 *	Object:
 *		string_to_int			EXPORTED function
 *
 *	Convert a string into an integer.
 *	Understands decimal (default), octal and hex numbers
 *	like MIPS' atob(), for compat reasons.
 *	It also understands Ada-like numbers of the form
 *		#base#digits
 *	Defaults to 0 for all strange strings.
 *
 */
int
string_to_int(s)
	char	*s;
{
	static unsigned      char_to_uint();
	char            c;
	unsigned        base = 10, d;
	int             neg = 1, val = 0;

	if ((s == 0) || ((c = *s++) == 0))
		goto out;

	/* skip spaces if any */
	while ((c == ' ') || (c == '\t'))
		c = *s++;

	/* parse sign, allow more than one (compat) */
	while (c == '-') {
		neg = -neg;
		c = *s++;
	}

	/* parse base specification, if any */
	if ((c == '0') || (c == '#')) {
		if (c == '0') {
			c = *s++;
			switch (c) {
			case 'X':
			case 'x':
				base = 16;
				break;
			case 'B':
			case 'b':
				base = 2;
				break;
			default:
				base = 8;
				break;
			}
		} else {
			c = *s++;
			while ((d = char_to_uint(c)) < base) {
				val *= base;
				val += d;
				c = *s++;
			}
			base = val;
			val = 0;
		}
		c = *s++;	/* forgiving of not '#' */
	}

	/* parse number proper */
	while ((d = char_to_uint(c)) < base) {
		val *= base;
		val += d;
		c = *s++;
	}
	val *= neg;
out:
	return val;	
}


/*
 *	Object:
 *		char_to_uint			LOCAL function
 *
 *	Convert a character into an integer.
 *	Besides the numbers 0..9 understands all
 *	letters of the alphabet, starting at a=10.
 *	Case insensitive, returns -1 for bogus characters.
 *
 */
static unsigned
char_to_uint(c)
	register char c;
{
	if ((c >= '0') && (c <= '9'))
		return c - '0';

	if ((c >= 'a') && (c <= 'z'))
		return c - 'a' + 10;

	if ((c >= 'A') && (c <= 'Z'))
		return c - 'A' + 10;

	return 0xffffffff;
}


/*
 *	Object:
 *		string_setf			EXPORTED function
 *
 *	This is a setf method for strings, which are
 *	just pointer-assigned.
 *	NOTE: might have to copy string into safe place
 *
 */
unsigned
string_setf(address, new_val)
	char	**address;
	char 	 *new_val;
{
	dprintf(" = %s", new_val);
	return (unsigned)(*address = new_val);
}


/*
 *	Object:
 *		machine_get_boot_flags		EXPORTED function
 *
 *	Pass up any explicit arguments to /etc/init
 *
 */
char *
machine_get_boot_flags(str)
	char *str;
{
	strncpy(str, init_args, sizeof init_args);
	return(str + strlen(str));
}
