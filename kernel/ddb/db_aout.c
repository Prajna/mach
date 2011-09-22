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
 * $Log:	db_aout.c,v $
 * Revision 2.15  93/03/09  10:53:19  danner
 * 	Make searching slightly less confusing if the symtab has not
 * 	been xstripped.
 * 	[93/03/05            af]
 * 
 * Revision 2.14  93/02/01  09:55:09  danner
 * 	aout_db_search_by_addr returns boolean_t.
 * 	[93/01/25            jfriedl]
 * 
 * Revision 2.13  93/01/14  17:24:24  danner
 * 	Support for coexhistance of multiple symbol table types.
 * 	64bit cleanup.
 * 	[92/11/30            af]
 * 
 * Revision 2.12  92/08/03  17:30:27  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.11  92/05/21  17:06:15  jfriedl
 * 	Added init for func_diff and line_diff in X_db_search_by_addr().
 * 	Also added proper declaration for that function.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.10  92/01/03  20:02:21  dbg
 * 	Don't print 'preserving symbols' if loading symbol table fails.
 * 	[91/11/29            dbg]
 * 
 * 	Use kern_sym_start, kern_sym_size to locate kernel symbol table.
 * 	Rename routine to ddb_init.
 * 	[91/10/30            dbg]
 * 
 * Revision 2.9  91/10/09  15:57:16  af
 * 	Supported address lookup with line number.
 * 	Changed X_db_line_at_pc to get file name without compiled
 * 	  with -g option.
 * 	Included "stab.h" for symbol type definitions.
 * 	[91/08/29            tak]
 * 
 * Revision 2.8  91/08/28  11:10:58  jsb
 * 	Added line number support, via X_db_line_at_pc.
 * 	[91/08/13  18:12:37  jsb]
 * 
 * Revision 2.7  91/07/31  17:29:43  dbg
 * 	Removed read_symtab_from_file.
 * 	Added task argument to X_db_sym_init.
 * 	[91/07/30  16:42:51  dbg]
 * 
 * Revision 2.6  91/07/09  23:15:35  danner
 * 	On luna, kdb_init needs to be called ddb_init. Add ifndef
 * 	 DB_SYMBOLS_PRELOADED for machines that use a.out format but
 * 	 whose prom loaders load generously load the symbol table.
 * 	[91/07/08            danner]
 * 
 * Revision 2.5  91/05/14  15:32:00  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/03/16  14:42:23  rpd
 * 	Updated for new kmem_alloc interface.
 * 	[91/03/03            rpd]
 * 
 * Revision 2.3  91/02/05  17:05:55  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  16:16:44  mrt]
 * 
 * Revision 2.2  90/08/27  21:48:35  dbg
 * 	Created.
 * 	[90/08/17            dbg]
 * 
 */
/*
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date:	7/90
 */
/*
 * Symbol table routines for a.out format files.
 */

#include <mach/std_types.h>
#include <machine/db_machdep.h>		/* data types */
#include <ddb/db_sym.h>

#ifndef	DB_NO_AOUT

#include <ddb/nlist.h>			/* a.out symbol table */
#include <ddb/stab.h>

#define private static

/*
 * An a.out symbol table as loaded into the kernel debugger:
 *
 * symtab	-> size of symbol entries, in bytes
 * sp		-> first symbol entry
 *		   ...
 * ep		-> last symbol entry + 1
 * strtab	== start of string table
 *		   size of string table in bytes,
 *		   including this word
 *		-> strings
 */

/*
 * Find pointers to the start and end of the symbol entries,
 * given a pointer to the start of the symbol table.
 */
#define	db_get_aout_symtab(symtab, sp, ep) \
	(sp = (struct nlist *)((vm_offset_t *)(symtab) + 1), \
	 ep = (struct nlist *)((char *)sp + *((int*)symtab)))

boolean_t
aout_db_sym_init(symtab, esymtab, name, task_addr)
	char *	symtab;		/* pointer to start of symbol table */
	char *	esymtab;	/* pointer to end of string table,
				   for checking - may be rounded up to
				   integer boundary */
	char *	name;
	char *	task_addr;	/* use for this task only */
{
	register struct nlist	*sym_start, *sym_end;
	register struct nlist	*sp;
	register char *	strtab;
	register int	strlen;
	char *		estrtab;

	db_get_aout_symtab(symtab, sym_start, sym_end);

	strtab = (char *)sym_end;
	strlen = *(int *)strtab;
	estrtab = strtab + strlen;

#define	round_to_size(x) \
	(((vm_offset_t)(x) + sizeof(vm_size_t) - 1) & ~(sizeof(vm_size_t) - 1))

	if (round_to_size(estrtab) != round_to_size(esymtab)) {
	    db_printf("[ %s symbol table not valid ]\n", name);
	    return (FALSE);
	}

#undef	round_to_size

	for (sp = sym_start; sp < sym_end; sp++) {
	    register long strx;
	    strx = sp->n_un.n_strx;
	    if (strx != 0) {
		if (strx > strlen) {
		    db_printf("Bad string table index (%#x)\n", strx);
		    sp->n_un.n_name = 0;
		    continue;
		}
		sp->n_un.n_name = strtab + strx;
	    }
	}

	if (db_add_symbol_table(SYMTAB_AOUT,
				(char *)sym_start,
				(char *)sym_end,
				name,
				symtab,
				task_addr))
	{
	    /* Successfully added symbol table */
	    db_printf("[ preserving %d bytes of %s symbol table ]\n",
		esymtab - (char *)symtab, name);
	    return TRUE;
	}
	else
	    return FALSE;
}

/*
 * check file name or not (check xxxx.x pattern)
 */
private boolean_t
aout_db_is_filename(name)
	register char *name;
{
	while (*name) {
	    if (*name == '.') {
		if (name[1])
		    return(TRUE);
	    }
	    name++;
	}
	return(FALSE);
}

/*
 * special name comparison routine with a name in the symbol table entry
 */
private boolean_t
aout_db_eq_name(sp, name)
	struct nlist *sp;
	char *name;
{
	register char *s1, *s2;

	s1 = sp->n_un.n_name;
	s2 = name;
	if (*s1 == '_' && *s2 && *s2 != '_')
	    s1++;
	while (*s2) {
	    if (*s1++ != *s2++) {
		/*
		 * check .c .o file name comparison case
		 */
		if (*s2 == 0 && sp->n_un.n_name <= s1 - 2 
			&& s1[-2] == '.' && s1[-1] == 'o')
		    return(TRUE);
		return(FALSE);
	    }
	}
	/*
	 * do special check for
	 *     xxx:yyy for N_FUN
	 *     xxx.ttt for N_DATA and N_BSS
	 */
	return(*s1 == 0 || (*s1 == ':' && sp->n_type == N_FUN) || 
		(*s1 == '.' && (sp->n_type == N_DATA || sp->n_type == N_BSS)));
}

/*
 * search a symbol table with name and type
 *	fp(in,out): last found text file name symbol entry
 */
private struct nlist *
aout_db_search_name(sp, ep, name, type, fp)
	register struct nlist *sp;
	struct nlist	*ep;
	char		*name;
	int 		type;
	struct nlist	**fp;
{
	struct nlist	*file_sp = *fp;
	struct nlist	*found_sp = 0;

	for ( ; sp < ep; sp++) {
	    if (sp->n_type == N_TEXT && aout_db_is_filename(sp->n_un.n_name))
		*fp = sp;
	    if (type) {
		if (sp->n_type == type) {
		    if (aout_db_eq_name(sp, name))
	    		return(sp);
		}
		if (sp->n_type == N_SO)
		    *fp = sp;
		continue;
	    }
	    if (sp->n_type & N_STAB)
		continue;
	    if (sp->n_un.n_name && aout_db_eq_name(sp, name)) {
		/*
		 * In case of qaulified search by a file,
		 * return it immediately with some check.
		 * Otherwise, search external one
		 */
		if (file_sp) {
		    if ((file_sp == *fp) || (sp->n_type & N_EXT))
			return(sp);
		} else if (sp->n_type & N_EXT)
		    return(sp);
		else
		    found_sp = sp;
	    }
	}
	return(found_sp);
}

/*
 * search a symbol with file, func and line qualification
 */
private db_sym_t
aout_db_qualified_search(stab, file, sym, line)
	db_symtab_t	*stab;
	char		*file;
	char		*sym;
	int 		line;
{
	register struct nlist *sp = (struct nlist *)stab->start;
	struct nlist	*ep = (struct nlist *)stab->end;
	struct nlist	*fp = 0;
	struct nlist	*found_sp;
	unsigned long	func_top;
	boolean_t	in_file;

	if (file == 0 && sym == 0)
	    return(0);
	if (file) {
	    if ((sp = aout_db_search_name(sp, ep, file, N_TEXT, &fp)) == 0)
		return(0);
	}
	if (sym) {
	    sp = aout_db_search_name(sp, ep, sym, (line > 0)? N_FUN: 0, &fp);
	    if (sp == 0)
		return(0);
	}
	if (line > 0) {
	    if (file && !aout_db_eq_name(fp, file))
		return(0);
	    found_sp = 0;
	    if (sp->n_type == N_FUN) {
		/*
		 * qualified by function name
		 *     search backward because line number entries
		 *     for the function are above it in this case.
		 */
		func_top = sp->n_value;
		for (sp--; sp >= (struct nlist *)stab->start; sp--) {
		    if (sp->n_type != N_SLINE)
			continue;
		    if (sp->n_value < func_top)
			break;
		    if (sp->n_desc <= line) {
			if (found_sp == 0 || found_sp->n_desc < sp->n_desc)
			    found_sp = sp;
			if (sp->n_desc == line)
			    break;
		    }
		}
		if (sp->n_type != N_SLINE || sp->n_value < func_top)
		    return(0);
	    } else {
		/*
		 * qualified by only file name
		 *    search forward in this case
		 */
		in_file = TRUE;
		for (sp++; sp < ep; sp++) {
		    if (sp->n_type == N_TEXT 
			&& aout_db_is_filename(sp->n_un.n_name))
			break;		/* enter into another file */
		    if (sp->n_type == N_SOL) {
			in_file = aout_db_eq_name(sp, file);
			continue;
		    }
		    if (!in_file || sp->n_type != N_SLINE)
			continue;
		    if (sp->n_desc <= line) {
			if (found_sp == 0 || found_sp->n_desc < sp->n_desc)
			    found_sp = sp;
			if (sp->n_desc == line)
			    break;
		    }
		}
	    }
	    sp = found_sp;
	}
	return((db_sym_t) sp);
}

/*
 * lookup symbol by name
 */
db_sym_t
aout_db_lookup(stab, symstr)
	db_symtab_t	*stab;
	char *		symstr;
{
	db_sym_t db_sym_parse_and_lookup();

	return(db_sym_parse_and_lookup(aout_db_qualified_search, stab, symstr));
}

db_sym_t
aout_db_search_symbol(symtab, off, strategy, diffp)
	db_symtab_t *	symtab;
	register
	db_addr_t	off;
	db_strategy_t	strategy;
	db_expr_t	*diffp;		/* in/out */
{
	register unsigned long	diff = *diffp;
	register struct nlist	*symp = 0;
	register struct nlist	*sp, *ep;

	sp = (struct nlist *)symtab->start;
	ep = (struct nlist *)symtab->end;

	for (; sp < ep; sp++) {
	    if (sp->n_un.n_name == 0)
		continue;
	    if ((sp->n_type & N_STAB) != 0)
		continue;
	    if (strategy == DB_STGY_XTRN && (sp->n_type & N_EXT) == 0)
		continue;
	    if (off >= sp->n_value) {

		unsigned int type = sp->n_type;

		if (type == N_FN) continue;
		if (off - sp->n_value < diff) {
		    diff = off - sp->n_value;
		    symp = sp;
		    if (diff == 0 && (type & N_EXT))
			break;
		}
		else if (off - sp->n_value == diff) {
		    if (symp == 0)
			symp = sp;
		    else if ((symp->n_type & N_EXT) == 0 &&
				(type & N_EXT) != 0)
			symp = sp;	/* pick the external symbol */
		}
	    }
	}
	if (symp == 0) {
	    *diffp = off;
	}
	else {
	    *diffp = diff;
	}
	return ((db_sym_t)symp);
}

/*
 * Return the name and value for a symbol.
 */
void
aout_db_symbol_values(sym, namep, valuep)
	db_sym_t	sym;
	char		**namep;
	db_expr_t	*valuep;
{
	register struct nlist *sp;

	sp = (struct nlist *)sym;
	if (namep)
	    *namep = sp->n_un.n_name;
	if (valuep)
	    *valuep = sp->n_value;
}

#define X_DB_MAX_DIFF	8	/* maximum allowable diff at the end of line */

/*
 * search symbol by value
 */
private boolean_t
aout_db_search_by_addr(stab, addr, file, func, line, diff)
	db_symtab_t	*stab;
	register	vm_offset_t addr;
	char		**file;
	char		**func;
	int 	 	*line;
	unsigned long	*diff;
{
	register	struct nlist *sp;
	register	struct nlist *line_sp, *func_sp, *file_sp, *line_func;
	register vm_size_t func_diff, line_diff;
	boolean_t	found_line = FALSE;
	struct 	  	nlist *ep = (struct nlist *)stab->end;

	line_sp = func_sp = file_sp = line_func = 0;
	*file = *func = 0;
	*line = 0;
 	func_diff = line_diff = ~0;
	for (sp = (struct nlist *)stab->start; sp < ep; sp++) {
	    switch(sp->n_type) {
	    case N_SLINE:
		if (sp->n_value <= addr) {
		    if (line_sp == 0 || line_diff >= addr - sp->n_value) {
			if (line_func)
			    line_func = 0;
			line_sp = sp;
			line_diff = addr - sp->n_value;
		    }
		}
		if (sp->n_value >= addr && line_sp)
		    found_line = TRUE;
		continue;
	    case N_FUN:
		if ((found_line || (line_sp && line_diff < X_DB_MAX_DIFF))
		    && line_func == 0)
		    line_func = sp;
		continue;
	    case N_SO:
		if (sp->n_value > addr)
		    continue;
		if (file_sp == 0 || file_sp->n_value <= sp->n_value)
		    file_sp = sp;
		continue;
	    case N_TEXT:
		if (aout_db_is_filename(sp->n_un.n_name)) {
		    if (sp->n_value > addr)
			continue;
		    if (file_sp == 0 || file_sp->n_value <= sp->n_value)
			file_sp = sp;
		} else if (sp->n_value <= addr &&
			 (func_sp == 0 || func_diff > addr - sp->n_value)) {
		    func_sp = sp;
		    func_diff = addr - sp->n_value;
		}
		continue;
	    case N_TEXT|N_EXT:
		if (sp->n_value <= addr &&
			 (func_sp == 0 || func_diff >= addr - sp->n_value)) {
		    func_sp = sp;
		    func_diff = addr - sp->n_value;
		    if (func_diff == 0 && file_sp && func_sp)
		        break;
		}
	    default:
		continue;
	    }
	    break;
	}
	if (line_sp) {
	    if (line_func == 0 || func_sp == 0
		|| line_func->n_value != func_sp->n_value)
		line_sp = 0;
	}
	if (file_sp) {
	    *diff = addr - file_sp->n_value;
	    *file = file_sp->n_un.n_name;
	}
	if (func_sp) {
	    *diff = addr - func_sp->n_value;
	    *func = (func_sp->n_un.n_name[0] == '_')?
			func_sp->n_un.n_name + 1: func_sp->n_un.n_name;
	}
	if (line_sp) {
	    *diff = addr - line_sp->n_value;
	    *line = line_sp->n_desc;
	}
	return(file_sp || func_sp || line_sp);
}

/*
 * Find filename and lineno within, given the current pc.
 */
boolean_t
aout_db_line_at_pc(stab, sym, file, line, pc)
	db_symtab_t	*stab;
	db_sym_t	sym;
	char		**file;
	int		*line;
	db_expr_t	pc;
{
	char		*func;
	unsigned long	diff;
	boolean_t	found;

	found = aout_db_search_by_addr(stab,(vm_offset_t)pc,file,&func,line,&diff);
	return(found && func && *file);
}

/*
 * Initialization routine for a.out files.
 */
void
ddb_init()
{
	extern vm_offset_t	kern_sym_start;
	extern vm_size_t	kern_sym_size;

	if (kern_sym_size != 0) {
	    aout_db_sym_init((char *) kern_sym_start,
			  (char *)(kern_sym_start + kern_sym_size),
			  "mach",
			  (char *)0);
	}
}

#endif	/* DB_NO_AOUT */
