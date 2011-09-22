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
 * $Log:	db_alpha_aout.c,v $
 * Revision 2.4  93/08/31  15:15:53  mrt
 * 	Turn debugging off.
 * 	[93/08/31            af]
 * 
 * Revision 2.3  93/05/28  22:35:07  rvb
 * 	Reflect change in VECTOR macro for fr->isvector.
 * 	[93/05/21            af]
 * 
 * Revision 2.2  93/03/09  10:49:55  danner
 * 	Created, brutally taken from my ADB code without cleaning.
 * 	[93/02/20            af]
 * 
 */
/*
 *	File: db_alpha_aout.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	2/93
 *
 *	Extensions to A.OUT symtables for Alpha.
 */

#include <mach/std_types.h>
#include <machine/db_machdep.h>
#include <ddb/db_sym.h>
#include <ddb/nlist.h>			/* a.out symbol table */
#include <ddb/stab.h>

#include <mach/alpha/asm.h>
#include <machine/db_trace.h>

#define	DEBUG 0
#define private static

/*
 * Fillin the proc descriptor, if we can
 */
private boolean_t
aout_findproc(
	db_symtab_t	*symtab,
	db_sym_t	sym,
	frame_info_t	fr,
	db_expr_t	pc)
{
	register struct nlist *Sp, *pr, *esymtab, *fil, *lin;
	db_addr_t	val;

	if (sym == DB_SYM_NULL)
		return (FALSE);

	pr = (struct nlist *) sym;
	val = pr->n_value;

#if DEBUG
db_printf("[find %s]", pr->n_un.n_name);
#endif

	lin = fil = 0;
	esymtab = (struct nlist *)symtab->end;
	for (Sp = (struct nlist *)symtab->start; Sp < esymtab; Sp++) {
		register int typ = Sp->n_type;

		/* source file name */
		if (typ == N_SO) {
			fil = Sp;
			continue;
		} else if (typ == N_FN) {
			/* a new object file */
			lin = 0;
#if 1
			/* some files show up as /tmp/ccNNNNN.o */
			fil = Sp;
#else
			/* all non -g files show up as no source */
			fil = 0;
#endif
		/* source line */
		} else if (typ == N_SLINE)
			lin = Sp;

		/* frameinfo */
		if (typ != N_FRAME)
			continue;

		if (strcmp(pr->n_un.n_name, Sp->n_un.n_name))
			continue;

#define	n_pcreg		n_other
#define	n_framereg	n_desc
#define	n_framesize	n_pad
#define	n_regmasks	n_value

#if DEBUG
db_printf("[%X %X %X %X]\n", Sp->n_framesize, Sp->n_regmasks,
			Sp->n_pcreg, Sp->n_framereg);
#endif

		/* got it */

		fr->nloc = 0;	/* donno */

		fr->framesize = Sp->n_framesize;
		fr->regmask = Sp->n_regmasks >> 32; /* Iregs only */
		fr->isleaf = (fr->regmask & IM_RA) == 0;
		fr->isvector = (Sp->n_pcreg == 26) && (fr->regmask & IM_EXC);
		fr->mod_sp = (fr->framesize != 0);

		 /* offset of return pc from top of frame */
		fr->saved_pc_off = -fr->framesize/*???*/;

#undef	n_pcreg
#undef	n_framereg
#undef	n_framesize
#undef	n_regmasks

		/* Arguments, filename and linenum */
		fr->narg = 0;
		if (fil) {
			int linediff = 0x7fffffff;
			boolean_t doargs = TRUE;

			fr->filename = fil->n_un.n_name;

			/* keep looking now for closest line and args */
			for (Sp = lin; Sp && (Sp < esymtab); Sp++) {

				/* dont go too far */
				if (Sp->n_type == N_FN || Sp->n_type == N_SO)
					break;

				if (Sp->n_type == N_SLINE) {

				    /* funargs can be after many lines */
				    if ((pc > Sp->n_value) && !doargs)
				        break;

				    if ((pc - Sp->n_value) < linediff) {
					lin = Sp;
					linediff = pc - Sp->n_value;
				    }
				}

				else if (doargs && (Sp->n_type == N_FUN)) {
					while (Sp[1].n_type == N_PSYM) {
						fr->narg++;
						Sp++;
					}
					doargs = FALSE;
				}
			}
			fr->linenum = (lin) ? lin->n_desc : -1;
		}

		return (TRUE);
	}
	return (FALSE);
}

extern boolean_t
coff_findproc(
	db_symtab_t	*symtab,
	db_sym_t	sym,
	frame_info_t	fr,
	db_expr_t	pc);

/*
 * Interface
 */
boolean_t
findproc(
	db_symtab_t	*symtab,
	db_sym_t	sym,
	frame_info_t	fr,
	db_expr_t	pc)
{
	if (symtab->type == SYMTAB_AOUT)
		return aout_findproc(symtab, sym, fr, pc);
	else if (symtab->type == SYMTAB_COFF)
		return coff_findproc(symtab, sym, fr, pc);
	else
		return FALSE;
}

