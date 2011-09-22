/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 * $Log:	db_output.c,v $
 * Revision 2.11  93/05/13  14:39:34  rvb
 * 	db_putc is used by the luna; rename it here.
 * 	[93/05/13            rvb]
 * 
 * Revision 2.10  93/05/10  17:45:56  rvb
 * 	_doprint assumes 1st arg of putc type function is char not int.
 * 	Make db_id_putc conform.
 * 	[93/04/27            Ian Dall <DALL@hfrd.dsto.gov.au>]
 * 
 * 	Call _doprnt with the right number of arguments. Define db_id_putc
 * 	which has an extra dummy argument cf db_putchar. Use db_id_putc
 * 	instead of db_putchar in call to _doprnt.
 * 	[93/04/25            Ian Dall <DALL@hfrd.dsto.gov.au>]
 * 
 * Revision 2.9  93/01/14  17:25:24  danner
 * 	64bit cleanup.
 * 	[92/11/30            af]
 * 
 * Revision 2.8  92/08/03  17:31:38  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.7  92/05/21  17:07:20  jfriedl
 * 	Added void type to functions that needed it.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.6  91/10/09  16:01:26  af
 * 	 Revision 2.5.2.1  91/10/05  13:06:46  jeffreyh
 * 	 	Added "more" like function.
 * 	 	[91/08/29            tak]
 * 
 * Revision 2.5.2.1  91/10/05  13:06:46  jeffreyh
 * 	Added "more" like function.
 * 	[91/08/29            tak]
 * 
 * Revision 2.5  91/07/09  23:15:54  danner
 * 	Include machine/db_machdep.c.
 * 	When db_printf is invoked, call db_printing on luna. This is used
 * 	 to trigger a screen clear. Under luna88k conditional.
 * 	[91/07/08            danner]
 *
 * Revision 2.4  91/05/14  15:34:51  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:06:45  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  16:18:41  mrt]
 * 
 * Revision 2.2  90/08/27  21:51:25  dbg
 * 	Put extra features of db_doprnt in _doprnt.
 * 	[90/08/20            dbg]
 * 	Reduce lint.
 * 	[90/08/07            dbg]
 * 	Created.
 * 	[90/07/25            dbg]
 * 
 */
/*
 * 	Author: David B. Golub, Carnegie Mellon University
 *	Date:	7/90
 */

/*
 * Printf and character output for debugger.
 */

#include <mach/boolean.h>
#include <sys/varargs.h>
#include <machine/db_machdep.h>
#include <ddb/db_lex.h>
#include <ddb/db_output.h>

/*
 *	Character output - tracks position in line.
 *	To do this correctly, we should know how wide
 *	the output device is - then we could zero
 *	the line position when the output device wraps
 *	around to the start of the next line.
 *
 *	Instead, we count the number of spaces printed
 *	since the last printing character so that we
 *	don't print trailing spaces.  This avoids most
 *	of the wraparounds.
 */

#ifndef	DB_MAX_LINE
#define	DB_MAX_LINE		24	/* maximum line */
#define DB_MAX_WIDTH		80	/* maximum width */
#endif	DB_MAX_LINE

#define DB_MIN_MAX_WIDTH	20	/* minimum max width */
#define DB_MIN_MAX_LINE		3	/* minimum max line */
#define CTRL(c)			((c) & 0xff)

int	db_output_position = 0;		/* output column */
int	db_output_line = 0;		/* output line number */
int	db_last_non_space = 0;		/* last non-space character */
int	db_tab_stop_width = 8;		/* how wide are tab stops? */
#define	NEXT_TAB(i) \
	((((i) + db_tab_stop_width) / db_tab_stop_width) * db_tab_stop_width)
int	db_max_line = DB_MAX_LINE;	/* output max lines */
int	db_max_width = DB_MAX_WIDTH;	/* output line width */

extern void	db_check_interrupt();

/*
 * Force pending whitespace.
 */
void
db_force_whitespace()
{
	register int last_print, next_tab;

	last_print = db_last_non_space;
	while (last_print < db_output_position) {
	    next_tab = NEXT_TAB(last_print);
	    if (next_tab <= db_output_position) {
		cnputc('\t');
		last_print = next_tab;
	    }
	    else {
		cnputc(' ');
		last_print++;
	    }
	}
	db_last_non_space = db_output_position;
}

static void
db_more()
{
	register  char *p;
	boolean_t quit_output = FALSE;

	for (p = "--db_more--"; *p; p++)
	    cnputc(*p);
	switch(cngetc()) {
	case ' ':
	    db_output_line = 0;
	    break;
	case 'q':
	case CTRL('c'):
	    db_output_line = 0;
	    quit_output = TRUE;
	    break;
	default:
	    db_output_line--;
	    break;
	}
	p = "\b\b\b\b\b\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b";
	while (*p)
	    cnputc(*p++);
	if (quit_output) {
	    db_error(0);
	    /* NOTREACHED */
	}
}

/*
 * Output character.  Buffer whitespace.
 */
void
db_putchar(c)
	int	c;		/* character to output */
{
	if (db_max_line >= DB_MIN_MAX_LINE && db_output_line >= db_max_line-1)
	    db_more();
	if (c > ' ' && c <= '~') {
	    /*
	     * Printing character.
	     * If we have spaces to print, print them first.
	     * Use tabs if possible.
	     */
	    db_force_whitespace();
	    cnputc(c);
	    db_output_position++;
	    if (db_max_width >= DB_MIN_MAX_WIDTH
		&& db_output_position >= db_max_width-1) {
		/* auto new line */
		cnputc('\n');
		db_output_position = 0;
		db_last_non_space = 0;
		db_output_line++;
	    }
	    db_last_non_space = db_output_position;
	}
	else if (c == '\n') {
	    /* Return */
	    cnputc(c);
	    db_output_position = 0;
	    db_last_non_space = 0;
	    db_output_line++;
	    db_check_interrupt();
	}
	else if (c == '\t') {
	    /* assume tabs every 8 positions */
	    db_output_position = NEXT_TAB(db_output_position);
	}
	else if (c == ' ') {
	    /* space */
	    db_output_position++;
	}
	else if (c == '\007') {
	    /* bell */
	    cnputc(c);
	}
	/* other characters are assumed non-printing */
}

void
db_id_putc(char c, vm_offset_t dummy)
{
  db_putchar(c);
}

/*
 * Return output position
 */
int
db_print_position()
{
	return (db_output_position);
}

/*
 * End line if too long.
 */
void db_end_line()
{
	if (db_output_position >= db_max_width-1)
	    db_printf("\n");
}

/*
 * Printing
 */
extern void	_doprnt();

/*VARARGS1*/
void
db_printf( fmt, va_alist)
	char *	fmt;
	va_dcl
{
	va_list	listp;

#ifdef	db_printf_enter
	db_printf_enter();	/* optional multiP serialization */
#endif
	va_start(listp);
	_doprnt(fmt, &listp, db_id_putc, db_radix, 0);
	va_end(listp);
}

/* alternate name */

/*VARARGS1*/
void
kdbprintf(fmt, va_alist)
	char *	fmt;
	va_dcl
{
	va_list	listp;
	va_start(listp);
	_doprnt(fmt, &listp, db_id_putc, db_radix, 0);
	va_end(listp);
}
