/* 
 * Mach Operating System
 * Copyright (c) 1992,1991,1990 Carnegie Mellon University
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
 * $Log:	db_command.h,v $
 * Revision 2.7  93/01/14  17:24:49  danner
 * 	Added db_recover.
 * 	[92/10/26            dbg]
 * 	Added db_recover.
 * 	[92/10/26            dbg]
 * 
 * Revision 2.6  91/10/09  15:58:45  af
 * 	Added db_exec_conditional_cmd(), and db_option().
 * 	Deleted db_skip_to_eol().
 * 	[91/08/29            tak]
 * 
 * Revision 2.5  91/07/09  23:15:46  danner
 * 	Grabbed up to date copyright.
 * 	[91/07/08            danner]
 * 
 * Revision 2.2  91/04/10  16:02:32  mbj
 * 	Grabbed 3.0 copyright/disclaimer since ddb comes from 3.0.
 * 	[91/04/09            rvb]
 * 
 * Revision 2.3  91/02/05  17:06:15  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  16:17:28  mrt]
 * 
 * Revision 2.2  90/08/27  21:50:19  dbg
 * 	Replace db_last_address_examined with db_prev, db_next.
 * 	[90/08/22            dbg]
 * 	Created.
 * 	[90/08/07            dbg]
 * 
 */
/*
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date:	7/90
 */
/*
 * Command loop declarations.
 */

#include <machine/db_machdep.h>
#include <machine/setjmp.h>

extern void		db_command_loop();
extern boolean_t	db_exec_conditional_cmd();
extern boolean_t	db_option(/* char *, int */);

extern void		db_error(/* char * */);	/* report error */

extern db_addr_t	db_dot;		/* current location */
extern db_addr_t	db_last_addr;	/* last explicit address typed */
extern db_addr_t	db_prev;	/* last address examined
					   or written */
extern db_addr_t	db_next;	/* next address to be examined
					   or written */
extern jmp_buf_t *	db_recover;	/* error recovery */

extern jmp_buf_t *	db_recover;	/* error recovery */

/*
 * Command table
 */
struct db_command {
	char *	name;		/* command name */
	void	(*fcn)();	/* function to call */
	int	flag;		/* extra info: */
#define	CS_OWN		0x1	    /* non-standard syntax */
#define	CS_MORE		0x2	    /* standard syntax, but may have other
				       words at end */
#define	CS_SET_DOT	0x100	    /* set dot after command */
	struct db_command *more;   /* another level of command */
};
