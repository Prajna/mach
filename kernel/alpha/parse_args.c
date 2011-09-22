/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
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
 * Revision 2.3  93/03/09  10:50:27  danner
 * 	String protos.
 * 	[93/03/07            af]
 * 
 * Revision 2.2  93/02/05  07:59:28  danner
 * 	Since there a (twisted) way to pass arguments to the
 * 	kernel, resurvived and adapted my old mips code.
 * 	[93/02/02            af]
 * 	Created empty.
 * 	[92/12/10            af]
 * 
 */
/*
 *	File: parse_args.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Command line parsing for the Mach kernel on Alpha.
 *
 */

/*
 *	Switches are names of kernel variables, which can be
 *	either simply set to 1 (flags) or set to some specific
 *	value provided in the comand line.
 *
 *	Example:
 *		setenv BOOT_OSFLAGS "boothowto=0xc memlim=0xc00000"
 *
 *	Certain variables might also require some special processing
 *	before and/or after being set.  For this reason we invoke
 *	a function that is associated with the variable, passing the
 *	desired new value as argument.  Responsibility for setting
 *	the variable to the desired value remains with the function.
 *
 */

#include <mach/std_types.h>
#include <kern/strings.h>
#include <alpha/prom_interface.h>


typedef struct {
	char *name;			/* variable's name */
	vm_offset_t address;		/* variable's address */
	void	 (*setf)(vm_offset_t var, char *val);
					/* how to modify the variable */
} switch_table;

void int_setf    (vm_offset_t var, char *val);	/* default setf method */
#define bool_setf int_setf
void nat_setf    (vm_offset_t var, char *val);	/* natural wordsize */
void string_setf (vm_offset_t var, char *val);	/* for strings */
void uchar_setf  (vm_offset_t var, char *val);	/* for unsigned chars */

static char* isa_switch(
			char 		 *str,
			switch_table	**swptr);

static void scan_cmdline(
			char	*cmd,
			int	*p_argc,
			char	**argv);

integer_t string_to_int( char	*s);

static unsigned int char_to_uint( char c );

/*
 *	Object:
 *		kernel_switch			EXPORTED structure
 *
 *	Kernel arguments table.
 *	Add/remove variables from here and create the setf methods
 *	in the module that exports that variable, if necessary.
 *	The most common methods are defined herein.
 *
 *	This table is exported even if it need not, so that it
 *	can be patched with ADB if necessary.
 *
 */
extern boolean_t
	askme;

extern vm_size_t
	zdata_size, memlimit, page_size;

extern int
	rcline, boothowto, pmap_debug;

extern unsigned char
	scsi_initiator_id[];

unsigned char
	*monitor_types[4];

/* The table proper */

switch_table kernel_switch[] = {
	{ "askme",		(vm_offset_t)&askme,
							bool_setf },
	{ "boothowto",		(vm_offset_t)&boothowto,
							int_setf },
	{ "memlimit",		(vm_offset_t)&memlimit,
							nat_setf },
	{ "monitor0",		(vm_offset_t)&monitor_types[0], 
	      						string_setf },
	{ "monitor1",		(vm_offset_t)&monitor_types[1],
	  						string_setf },
	{ "monitor2",		(vm_offset_t)&monitor_types[2],
	  						string_setf },
	{ "monitor3",		(vm_offset_t)&monitor_types[3],
	  						string_setf },
	{ "page_size",		(vm_offset_t)&page_size,
							nat_setf },
	{ "pmap_debug",		(vm_offset_t)&pmap_debug,
							int_setf },
	{ "rcline",		(vm_offset_t)&rcline,
							int_setf },
	{ "scsi0",		(vm_offset_t)(&scsi_initiator_id[0]),
							uchar_setf   },
	{ "scsi1",		(vm_offset_t)(&scsi_initiator_id[1]),
							uchar_setf   },
	{ "zdata_size", 	(vm_offset_t)&zdata_size,
							nat_setf },
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

void
parse_args()
{
	register int    i;
	switch_table   *variable;
	char           *vptr;
	int		argc;
#define	MAX_ARGS	24
	char		*argv[MAX_ARGS+1];
	static char	flags[128];

	flags[0] = 0;
	prom_getenv( PROM_E_BOOTED_OSFLAGS, flags, sizeof(flags) );

	scan_cmdline( flags, &argc, argv );

	for (i = 0/*1*/; i < argc; i++) {

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
isa_switch(
	char 		 *str,
	switch_table	**swptr)
{
	register switch_table	*sw = kernel_switch;
	register char 		*cp0, *cp1, c;

	if (!str)			/* sanity */
		return 0L;
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
 *		int_setf			EXPORTED function
 *
 *	This is the "default" setf method.
 *	Understands integers, which default to "1"
 *	if no value is provided.
 */
void
int_setf(
	vm_offset_t	var,
	char	       *val)
{
	unsigned int	binval;

	if (*val == 0)
		binval = 1;
	else
		binval = string_to_int(val);

	dprintf(" = x%x", binval);
	* (unsigned int *) var = binval;
}


/*
 *	Object:
 *		nat_setf			EXPORTED function
 *
 *	Understands naturally-sized, unsigned integers.
 *	No defaults.
 */
void
nat_setf(
	vm_offset_t	var,
	char	       *val)
{
	natural_t	binval;

	binval = string_to_int(val);

	dprintf(" = x%lx", binval);
	* (natural_t *) var = binval;
}


/*
 *	Object:
 *		uchar_setf			EXPORTED function
 *
 *	Understands single-byte integers, no default.
 */
void
uchar_setf(
	vm_offset_t	var,
	char	       *val)
{
	unsigned char	binval;

	binval = string_to_int(val);

	dprintf(" = x%x", binval);
	* (unsigned char *) var = binval;
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
void
string_setf(
	vm_offset_t	var,
	char	       *val)
{
	dprintf(" = %s", val);
	* (char **) var = val;
}


/*
 *	Object:
 *		string_to_int			EXPORTED function
 *
 *	Convert a string into an integer.
 *	Understands decimal (default), octal and hex numbers.
 *	It also understands Ada-like numbers of the form
 *		#base#digits
 *	Defaults to 0 for all strange strings.
 *
 */
integer_t
string_to_int(	char	*s)
{
	char            c;
	unsigned int    base = 10, d;
	int             neg = 1;
	integer_t	val = 0;

	if ((s == 0) || ((c = *s++) == 0))
		goto out;

	/* skip spaces if any */
	while ((c == ' ') || (c == '\t'))
		c = *s++;

	/* parse sign, allow more than one (frills) */
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
 *	Case insensitive, returns infinity for bogus characters.
 *
 */
static unsigned int
char_to_uint(
	register char c)
{
	if ((c >= '0') && (c <= '9'))
		return c - '0';

	if ((c >= 'a') && (c <= 'z'))
		return c - 'a' + 10;

	if ((c >= 'A') && (c <= 'Z'))
		return c - 'A' + 10;

	return ~0;
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

/*
 *	Object:
 *		scan_cmdline		EXPORTED function
 *
 *	Parse boot 'flags' into U*x-like cmd vector.
 */
static void
scan_cmdline(
	char	*cmd,
	int	*p_argc,
	char	**argv)
{
	register int c, argc;
	register char *p = cmd;

	c = *p; argc = 0;
	while (c) {
		/* One more in the bag */
		argv[argc++] = p;

		/* look for separator */
		while ((c != ' ') && (c != '\t') && c)
			c = *++p;

		/* terminate string */
		*p = 0;

		/* skip blanks */
		while ((c == ' ') || (c == '\t'))
			c = *++p;
	}
	*p_argc = argc;
}

