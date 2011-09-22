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
 * $Log:	db_access.c,v $
 * Revision 2.9  93/03/09  10:53:11  danner
 * 	Here we go again. Pls leave db_extend[] alone, ok ?
 * 	[93/03/05            af]
 * 
 * Revision 2.8  93/01/14  17:24:15  danner
 * 	Fixed last entry of db_extend.
 * 	[93/01/14            danner]
 * 
 * 	64bit cleanup. db_extend[] was rightly signed.
 * 	[92/11/30            af]
 * 
 * Revision 2.7  92/08/03  17:30:22  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.6  92/05/21  17:06:10  jfriedl
 * 	Made db_extend unsigned and made constants unsigned as well.
 * 	This make it the same in STDC and traditional. Also shuts up lint.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.5  91/10/09  15:56:44  af
 * 	Added db_{get,put}_task_value, and changed db_{get,put}_value
 * 	  to call them.  db_{get,put}_value are left for compatibility
 * 	  reason.
 * 	Added "task" parameter to specifiy target task space.
 * 	Added db_access_level to indicate implementation dependent
 * 	  access capability.
 * 	[91/08/29            tak]
 * 
 * Revision 2.4  91/05/14  15:31:33  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:05:44  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  16:16:22  mrt]
 * 
 * Revision 2.2  90/08/27  21:48:20  dbg
 * 	Fix type declarations.
 * 	[90/08/07            dbg]
 * 	Created.
 * 	[90/07/25            dbg]
 * 
 */
/*
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date:	7/90
 */
#include <mach/boolean.h>
#include <machine/db_machdep.h>		/* type definitions */
#include <machine/setjmp.h>
#include <kern/task.h>
#include <ddb/db_access.h>



/*
 * Access unaligned data items on aligned (longword)
 * boundaries.
 */

extern void	db_read_bytes();	/* machine-dependent */
extern void	db_write_bytes();	/* machine-dependent */

int db_access_level = DB_ACCESS_LEVEL;

/*
 * This table is for sign-extending things.
 * Therefore its entries are signed, and yes
 * they are infact negative numbers.
 * So don't you put no more Us in it. Or Ls either.
 * Otherwise there is no point having it, n'est pas ?
 */
static int db_extend[sizeof(int)+1] = {	/* table for sign-extending */
	0,
	0xFFFFFF80,
	0xFFFF8000,
	0xFF800000,
	0x80000000
};

db_expr_t
db_get_task_value(addr, size, is_signed, task)
	db_addr_t	addr;
	register int	size;
	boolean_t	is_signed;
	task_t		task;
{
	char		data[sizeof(db_expr_t)];
	register db_expr_t value;
	register int	i;

	db_read_bytes((void*)addr, size, data, task);

	value = 0;
#if	BYTE_MSF
	for (i = 0; i < size; i++)
#else	/* BYTE_LSF */
	for (i = size - 1; i >= 0; i--)
#endif
	{
	    value = (value << 8) + (data[i] & 0xFF);
	}
	    
	if (size <= sizeof(int)) {
	    if (is_signed && (value & db_extend[size]) != 0)
		value |= db_extend[size];
	}
	return (value);
}

void
db_put_task_value(addr, size, value, task)
	db_addr_t	addr;
	register int	size;
	register db_expr_t value;
	task_t		task;
{
	char		data[sizeof(db_expr_t)];
	register int	i;

#if	BYTE_MSF
	for (i = size - 1; i >= 0; i--)
#else	/* BYTE_LSF */
	for (i = 0; i < size; i++)
#endif
	{
	    data[i] = value & 0xFF;
	    value >>= 8;
	}

	db_write_bytes((void*)addr, size, data, task);
}

db_expr_t
db_get_value(addr, size, is_signed)
	db_addr_t	addr;
	int		size;
	boolean_t	is_signed;
{
	return(db_get_task_value(addr, size, is_signed, TASK_NULL));
}

void
db_put_value(addr, size, value)
	db_addr_t	addr;
	int		size;
	db_expr_t	value;
{
	db_put_task_value(addr, size, value, TASK_NULL);
}
