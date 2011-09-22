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
 * $Log:	db_variables.h,v $
 * Revision 2.6  93/01/14  17:26:12  danner
 * 	64bit cleanup.
 * 	[92/11/30            af]
 * 
 * Revision 2.5  91/10/09  16:04:17  af
 * 	Added suffix related field to db_variable structure.
 * 	Added macro definitions of db_{read,write}_variable.
 * 	[91/08/29            tak]
 * 
 * Revision 2.4  91/05/14  15:37:12  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:07:23  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  16:19:54  mrt]
 * 
 * Revision 2.2  90/08/27  21:53:40  dbg
 * 	Modularized typedef name.  Documented the calling sequence of
 * 	the (optional) access function of a variable.  Now the valuep
 * 	field can be made opaque, eg be an offset that fcn() resolves.
 * 	[90/08/20            af]
 * 
 * 	Created.
 * 	[90/07/25            dbg]
 * 
 */
/*
 * 	Author: David B. Golub, Carnegie Mellon University
 *	Date:	7/90
 */

#ifndef	_DB_VARIABLES_H_
#define	_DB_VARIABLES_H_

#include <kern/thread.h>

/*
 * Debugger variables.
 */
struct db_variable {
	char	*name;		/* Name of variable */
	db_expr_t *valuep;	/* pointer to value of variable */
				/* function to call when reading/writing */
	int	(*fcn)(/* db_variable, db_expr_t, int, db_var_aux_param_t */);
	short	min_level;	/* number of minimum suffix levels */
	short	max_level;	/* number of maximum suffix levels */
	short	low;		/* low value of level 1 suffix */
	short	high;		/* high value of level 1 suffix */
#define DB_VAR_GET	0
#define DB_VAR_SET	1
};
#define	FCN_NULL	((int (*)())0)

#define DB_VAR_LEVEL	3	/* maximum number of suffix level */

#define db_read_variable(vp, valuep)	\
	db_read_write_variable(vp, valuep, DB_VAR_GET, 0)
#define db_write_variable(vp, valuep)	\
	db_read_write_variable(vp, valuep, DB_VAR_SET, 0)

/*
 * auxiliary parameters passed to a variable handler
 */
struct db_var_aux_param {
	char		*modif;			/* option strings */
	short		level;			/* number of levels */
	short		suffix[DB_VAR_LEVEL];	/* suffix */
	thread_t	thread;			/* target task */
};

typedef struct db_var_aux_param	*db_var_aux_param_t;
	

extern struct db_variable	db_vars[];	/* debugger variables */
extern struct db_variable	*db_evars;
extern struct db_variable	db_regs[];	/* machine registers */
extern struct db_variable	*db_eregs;

#endif	/* _DB_VARIABLES_H_ */
