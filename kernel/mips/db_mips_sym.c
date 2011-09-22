/* 
 * Mach Operating System
 * Copyright (c) 1993-1987 Carnegie Mellon University
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
 * $Log:	db_mips_sym.c,v $
 * Revision 2.13  93/01/14  17:51:24  danner
 * 	Added casts.
 * 	[93/01/14            danner]
 * 
 * 	Typos.
 * 	[92/12/01            af]
 * 
 * Revision 2.12  92/05/05  15:47:26  danner
 * 	X_db_symbol_names now uses db_last_symtab. 
 * 	[92/05/05            danner]
 * 
 * Revision 2.11  92/01/03  20:23:35  dbg
 * 	Symbol table is now always moved to 'end', NOT preceded by
 * 	filehdr.
 * 	[91/10/30            dbg]
 * 
 * Revision 2.10  91/10/09  16:14:08  af
 * 	Supported address lookup with line number.
 * 	[91/09/05            tak]
 * 
 * Revision 2.9  91/08/28  11:15:50  jsb
 * 	Made message about preserving symtab uniform with other machines.
 * 	Added missing one in X_db_sym_init().
 * 	[91/08/27  16:42:03  af]
 * 
 * Revision 2.8  91/07/31  17:56:42  dbg
 * 	Added X_db_sym_init.
 * 	[91/07/29            dbg]
 * 
 * Revision 2.7  91/05/14  17:33:47  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/03/16  14:55:43  rpd
 * 	Updated for new kmem_alloc interface.
 * 	[91/03/03            rpd]
 * 
 * Revision 2.5  91/02/05  17:48:07  mrt
 * 	Added author notices
 * 	[91/02/04  11:22:10  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:25:43  mrt]
 * 
 * Revision 2.4  90/12/05  23:37:31  af
 * 	Executable header is now declared in coff.h.
 * 	Dropped bogus extra param from fixup_symtab.
 * 	[90/12/02            af]
 * 
 * Revision 2.2.1.1  90/10/03  11:36:14  af
 * 	Changes to allow the kernel to preserve the symtab even when
 * 	booted off DEC prom code (as in netowrk download via tftp()).
 * 	Needs a companion little program that alters the coff header
 * 	in the image to pretend the symtab is part of the data section.
 * 
 * Revision 2.2  90/08/27  22:07:41  dbg
 * 	Cleanups.  Got rid of nlist emulation: now send back a pointer to the
 * 	real symbol, worry later about deciding of what type the symbol is.
 * 	The MI code now treats symbols as opaque entities.
 * 	Split findproc into the two suggested functions: the MI code only
 * 	asks for X_db_line_at_pc for filename and linenumber info.
 * 	Changed search procedures to understand the MI strategies defined
 * 	in ddb/db_syms.h.  This still needs some more factoring out into
 * 	smaller pieces.
 * 	Reduced the crap in localsym, who has a MI interface now.
 * 	Got rid of the external_symbols thing, useless.
 * 	[90/08/20  10:16:52  af]
 * 
 * 	Created from my old KDB code. History summary:
 * 		From jsb: multiple symbol table support.  Line number support.
 * 		Preliminary local variable support.
 * 		[90/04/23            rpd]
 * 		Took a first shot at getting locals to work.
 * 		Reworked fixup() to preserve static procedures
 * 		and removed some useless code.
 * 		Switched over to MI version of KDB.
 * 		[90/01/20  16:38:27  af]
 * 		Created.
 * 		[89/08/08            af]
 * 
 */
/*
 *	File: db_mips_sym.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	8/89
 *
 *	Symtab module for COFF files (MIPS compiler)
 */

#include <mach/std_types.h>
#include <machine/db_machdep.h>
#include <ddb/db_sym.h>

#include <vm/vm_kern.h>
#include <mips/thread.h>
#include <mips/db_trace.h>
#include <mips/syms.h>

#define IS_DATA(c,t)	(((c)==scData)||((c)==scSData)||\
			 ((c)==scCommon)||((c)==scSCommon))
#define IS_BSS(c,t)	(((c)==scBss)||((c)==scSBss))
#define IS_TEXT(c,t)	(((c)==scText)||((c)==scRData))
#define IS_LOCAL(c,t)	((t)==stLocal)
#define IS_PARAM(c,t)	((t)==stParam)
#define IS_ABS(c,t)	((c)==scAbs)
#define IS_REG(c,t)	((c)==scRegister)

char *fh_noname = "noname";
static unsigned long localval;

static EXTR *last_external_symbol;
static PDR  *last_procedure_symbol;
static SYMR *last_local_symbol;
static unsigned last_proc_line_addr;


/* lookup a symbol by name */
db_sym_t
coff_db_lookup(stab, symstr)
	db_symtab_t	*stab;
	char *symstr;
{
	db_sym_t db_sym_parse_and_lookup();
	static db_sym_t coff_db_qualified_search();

	return(db_sym_parse_and_lookup(coff_db_qualified_search, stab, symstr));
}

static db_sym_t
coff_db_lookup_proc_line(symtab, pd, line_number)
	HDRR 	*symtab;
	PDR	*pd;
	int	line_number;
{
	register char *lp;
	register unsigned addr;
	register line, line_diff;
	register PDR *npd;
	char *elp;
	PDR  *epd;
	
	last_proc_line_addr = 0;
	if (symtab->cbLine == 0
		|| line_number < pd->lnLow || line_number > pd->lnHigh
		|| pd->cbLineOffset == 0)
		return DB_SYM_NULL;
	elp = (char *)(symtab->cbLineOffset) + symtab->cbLine;
	epd = ((PDR *)symtab->cbPdOffset) + symtab->ipdMax;
	for (npd = pd + 1; npd < epd; npd++) {
		if (npd->cbLineOffset) {
			elp = (char *)npd->cbLineOffset;
			break;
		}
	}
	addr = pd->adr;
	line = pd->lnLow;
	line_diff = 0x7fffffff;
	for (lp = (char *)pd->cbLineOffset; lp < elp; lp++) {
		register inst_count = (*lp & 0xf) + 1;
		if ((*lp & 0xf0) == 0x80) {
			line += (lp[1] << 8) + (lp[2] & 0xff);
			lp += 2;
		} else {
			line += (*lp >> 4);
		}
		if (line >= line_number && line - line_number < line_diff) {
			line_diff = line - line_number;
			last_proc_line_addr = addr;
			if (line_diff == 0)
				break;
		}
		addr += (inst_count*4);
	}
	if (last_proc_line_addr == 0)
		last_proc_line_addr = addr;
	last_procedure_symbol = pd;
	return (db_sym_t) pd;
}

/* lookup an external symbol, by name */
static EXTR *
coff_lookup_external(symtab, sym_name)
	HDRR	*symtab;
	char	*sym_name;
{
	register i;
	register EXTR *es = (EXTR *)symtab->cbExtOffset;

	for (i = 0; i < symtab->iextMax; i++, es++) {
		if (strcmp((char *)es->asym.iss, sym_name) == 0) {
			last_external_symbol = es;
			return(es);
		}
	}
	return(0);
}

/* lookup a procedure symbol, by name */
static PDR *
coff_lookup_proc(symtab, sym_name)
	HDRR	*symtab;
	char	*sym_name;
{
	register i;
	register PDR *pd = (PDR *)symtab->cbPdOffset;

	for (i = 0; i < symtab->ipdMax; i++, pd++) {
		if (strcmp((char *)((SYMR *)pd->isym)->iss, sym_name) == 0) {
			last_proc_line_addr = 0;
			last_procedure_symbol = pd;
			return(pd);
		}
	}
	return(0);
}

/* compare for equality, modulo traling dot */
static boolean_t
coff_streq(s1, s2)
register char *s1, *s2;
{
	for ( ; *s2; s1++, s2++) {
		if (*s1 != *s2)
			return(FALSE);
	}
	return(*s2 == 0 && (*s1 == 0 || *s1 == '.'));
}
		
/* lookup a symbol by filename, name, and possibly line number */
static db_sym_t
coff_db_qualified_search(stab, file_name, sym_name, line_number)
	db_symtab_t	*stab;
	char		*file_name, *sym_name;
{
	HDRR	*symtab;
	register i;
	register SYMR	*sp;
	register FDR	*fh;
	register PDR	*pd;
	register EXTR	*es;

	if (sym_name == 0 || stab == 0 || (symtab = (HDRR*)stab->private) == 0)
		return(DB_SYM_NULL);

	if (file_name) {
		/* search file descriptor entry */
		fh = ((FDR *)symtab->cbFdOffset);
		for (i = 0; i < symtab->ifdMax; i++, fh++) {
			register char *p, *cp;
			if (fh->rss == 0)
				continue;
			for (cp = p = (char *)fh->rss; *p; p++) {
				if (*p == '/')
					cp = p + 1;
			}
			if (strcmp(cp, file_name) == 0)
				break;
		}
		if (i >= symtab->ifdMax)
			return(DB_SYM_NULL);

		/* search symbol in the file */
		sp = (SYMR *)(fh->isymBase);
		for (i = 0; i < fh->csym; i++, sp++) {
			if (sp->st != stStatic && sp->st != stGlobal &&
			    sp->st != stProc && sp->st != stStaticProc)
				continue;
			if (coff_streq(sp->iss, sym_name))
				break;
		}
		if (i >= fh->csym)
			return((db_sym_t)coff_lookup_external(symtab, sym_name));

		if (sp->st == stStatic || sp->st == stGlobal) {
			if (line_number != 0)
				return DB_SYM_NULL;
			last_local_symbol = sp;
			return((db_sym_t) sp);
		}
		/*
		 * get function descriptor entry
		 */
		pd = ((PDR *)symtab->cbPdOffset) + fh->ipdFirst;
		for (i = 0; i < fh->cpd; i++, pd++) {
			if ((SYMR *)pd->isym == sp)
				break;
		}
		if (i >= fh->cpd) {
			db_error("bad symbol table information\n");
			/* NOTREACHED */
		}
		if (line_number == 0) {
			last_proc_line_addr = 0;
			last_procedure_symbol = pd;
			return (db_sym_t) pd;
		}
	} else if (line_number == 0) {

		/* search external symbol */
		if (es = coff_lookup_external(symtab, sym_name))
			return((db_sym_t)es);

		/* search function symbol */
		if (pd = coff_lookup_proc(symtab, sym_name))
			return((db_sym_t)pd);

		/* search static variable symbol */
		sp = (SYMR *)symtab->cbSymOffset;
		for (i = 0; i < symtab->isymMax; i++, sp++) {
			if (sp->st != stStatic)
				continue;
			if (coff_streq(sp->iss, sym_name)) {
				last_local_symbol = sp;
				return((db_sym_t)sp);
			}
		}
		return(DB_SYM_NULL);
	} else {
		if ((pd = coff_lookup_proc(symtab, sym_name)) == 0)
			return(DB_SYM_NULL);
	}
	return(coff_db_lookup_proc_line(symtab, pd, line_number));
}

/*
 * Find the closest symbol to val, return it and
 * the difference between val and the symbol found.
 *
 * Matching symbol should be closer than *diffp, which
 * gets updated on succesful return
 */
db_sym_t
coff_db_search_symbol(stab, value, strategy, diffp)
	db_symtab_t	*stab;
	db_expr_t	value;
	db_strategy_t	strategy;
	db_expr_t	*diffp;
{
	HDRR 		*symtab;
	register SYMR *sp, *best_sp = 0;
	register EXTR *es, *best_es = 0;
	register PDR  *pr, *best_pr = 0;
	int i;
	unsigned newdiff, diff = *diffp;
	char *last_thing_found = 0;

	if (stab == 0 || (symtab = (HDRR*)stab->private) == 0)
		return DB_SYM_NULL;

	/*
	 * Strategy says how to search:
	 *	DB_STGY_ANY	as in coff_db_lookup()
	 *	DB_STGY_XTRN	externals first, then procedures
	 *	DB_STGY_PROC	procedures only
	 */

	if (strategy == DB_STGY_PROC)
		goto search_procedures;

	for (es = (EXTR*)symtab->cbExtOffset, i = 0;
	     i < symtab->iextMax; i++, es++) {
		newdiff = value - (unsigned)es->asym.value;
		if (newdiff < diff) {
			diff = newdiff;
			best_es = es;
			if (diff == 0)
				break;
		}
	}

	if (best_es) {
		last_external_symbol = best_es;
		last_thing_found = (char*)best_es;
		if (diff == 0)
			goto out;
	}

search_procedures:

	for (pr = (PDR*)symtab->cbPdOffset, i = 0;
	     i < symtab->ipdMax; i++, pr++) {
		newdiff = value - (unsigned)pr->adr;
		if (newdiff < diff) {
			diff = newdiff;
			best_pr = pr;
			if (diff == 0)
				break;
		}
	}

	if (best_pr) {
		last_proc_line_addr = 0;
		last_procedure_symbol = best_pr;
		last_thing_found = (char*)best_pr;
		if (diff == 0)
			goto out;
	}

	if (strategy != DB_STGY_ANY)
		/*
		 * Return what the last space we searched gave,
		 * note that best_pr is set only if better than
		 * best_es (which could be null, yes).
		 */
		goto out;

	for (sp = (SYMR*)symtab->cbSymOffset, i = 0;
	     i < symtab->isymMax; i++, sp++) {
		newdiff = value - (unsigned)sp->value;
		if (newdiff < diff) {
			diff = newdiff;
			best_sp = sp;
			if (diff == 0)
				break;
		}
	}
	if (best_sp) {
		last_local_symbol = best_sp;
		last_thing_found = (char*)best_sp;
	}
out:
	*diffp = diff;
	return (db_sym_t)last_thing_found;
}

/*
 * Advance to the next local variable of procedure sym.
 * Leave its value in localval as a side effect.
 * Return 0 at end of file.
 */
db_sym_t
localsym(sym, isReg, Nesting)
	db_sym_t	sym;
	boolean_t	*isReg;
	int		*Nesting;
{
	HDRR	*symtab_hdr;
	register SYMR *sp;
	register SYMR *maxsym;
	static int nesting = 0;

	if (sym == DB_SYM_NULL)
		return sym;

	if (db_last_symtab == 0) {
		return DB_SYM_NULL;	/* sanity? */
	}
	symtab_hdr = (HDRR*)db_last_symtab->private;

	maxsym = ((SYMR*)symtab_hdr->cbSymOffset) + symtab_hdr->isymMax;
	/*
	 * "sym" is a token which we'll pass back.  We are basically
	 * scanning all local symbols starting from the procedure's
	 * itself (e.g. sym is a PDR on first call, a SYMR afterwards)
	 */
	if ((PDR*) sym == last_procedure_symbol) {
		sp = (SYMR*) last_procedure_symbol->isym;
		nesting = 1;
	} else
		sp = (SYMR*) sym;

	/* Note: a procedure's symbols really start _after_ pr->isym */
	while (++sp < maxsym && nesting > 0) {

		register int   sc, st;

		last_local_symbol = sp;

		sc = sp->sc;
		st = sp->st;

		if (IS_TEXT(sc,st)) {
			if (st==stBlock)
				nesting++;
			if (st==stEnd)
				nesting--;
			continue;
		}

		if (IS_LOCAL(sc,st) || IS_PARAM(sc,st)) {
			if (isReg)
				*isReg = IS_REG(sc,st);
			if (Nesting)
				*Nesting = nesting;
			return (db_sym_t)sp;
		}
	}
	return DB_SYM_NULL;
}

/* find the fdr to which a procedure belongs */
FDR *
findfdr(symtab, pr)
	HDRR *symtab;
	PDR  *pr;
{
	int i, j;
	register FDR *fh;
	register PDR *pd_base;

	for (i = 0; i < symtab->ifdMax; i++) {
		fh = ((FDR *)symtab->cbFdOffset) + i;
		pd_base = ((PDR*)symtab->cbPdOffset)+fh->ipdFirst;
		for (j = 0; j < fh->cpd; j++) {
			if (pr == &pd_base[j]) {
				return fh;
			}
		}
	}
	return 0;
}

/*
 *  If fh has no valid filename, return 0.
 *  Otherwise, remove all but the last directory from the pathname.
 */
char *
fdr_filename(fh)
	FDR *fh;
{
	char *s, *s0, *s1;

	if (! fh || ! fh->rss || (char *)fh->rss == fh_noname) {
		return 0;
	}
	for (s0 = s1 = s = (char *)fh->rss; *s; s++) {
		if (s[0] == '/' && s[1]) {
			s0 = s1;
			s1 = &s[1];
		}
	}
	return s0;
}

/* given a PC and some more info return line number in file */
int
fdr_linenum(symtab, fh, pr, pc)
	HDRR *symtab;
	FDR *fh;
	PDR *pr;
	int pc;
{
	char *base;
	PDR *pr0, *pr_end;
	int halt, lineno, best_offset, best_lineno, delta, linepc, count;

	/*
	 *  If we don't have enough info, return linenum of 0.
	 */
	if (! fh || ! fh->cbLineOffset || ! pr) {
		return 0;
	}
	if (pr->iline == ilineNil || pr->lnLow == -1 || pr->lnHigh == -1) {
		return 0;
	}

	/*
	 *  Find line entry to stop at for this procedure,
	 *  which is either first line entry of next procedure,
	 *  or last line entry for file if there are no more
	 *  procedures with line numbers.
	 *
	 *  XXX why can't we just use fh->LnHigh?
	 */
	halt = fh->cline;
	pr_end = ((PDR*)symtab->cbPdOffset) + fh->ipdFirst + fh->cpd;
	for (pr0 = pr+1; pr0 < pr_end; pr0++) {
		if (pr0->iline != -1) {
			halt = pr0->iline;
			break;
		}
	}

	/*
	 *  When procedures are moved around the linenumbers
	 *  are attributed to the next procedure up.
	 *  This only happens in -O4 images, dear jsb.
	 */
	if (pr->iline >= halt) {
		return pr->lnLow; /* best effort */
	}

	base = (char *) pr->cbLineOffset;
	linepc = (int)(pr->adr >> 2);	/* in words */
	pc = (int)((unsigned)pc >> 2);
	halt += (pr->adr >> 2) - pr->iline;
	lineno = best_lineno = pr->lnLow;
	best_offset = 9999999;

	while (linepc < halt) {
		count = *base & 0x0f;
		delta = *base++ >> 4;
		if (delta == -8) {
			delta = (base[0] << 8) | (base[1] & 0xff);
			base += 2;
		}
		lineno += delta;/* first delta is 0 */
		if (pc >= linepc && (pc - linepc) < best_offset) {
			best_offset = pc - linepc;
			best_lineno = lineno;
		}
		linepc += count + 1;
	}

	return best_lineno;
}

/*
 * Find filename and lineno within, given the
 * procedure's symbol and the current pc.
 */
boolean_t
coff_db_line_at_pc( stab, sym, filenamep, linep, pc)
	db_symtab_t	*stab;
	db_sym_t	sym;
	char		**filenamep;
	int		*linep;
	db_expr_t	pc;
{
	register FDR *fh;

	if (sym == DB_SYM_NULL)
		return FALSE;
	fh = findfdr(stab->private, sym);
	*filenamep = fdr_filename(fh);
	*linep = fdr_linenum(stab->private, fh, sym, pc);
	return TRUE;
}


/*
 * Find the descriptor for the function described by sym (a PDR)
 * return all we know about it.
 */
boolean_t
findproc( sym, fr, pc)
	db_sym_t	sym;
	frame_info_t 	fr;
	long 		pc;
{
	HDRR		*symtab_hdr;
	register PDR  *pr;
	register SYMR *sp;
	register FDR *fh;

	if (sym == DB_SYM_NULL)
		return FALSE;

	pr = (PDR*) sym;	/* opaque revealed */

	(void) coff_db_line_at_pc( db_last_symtab, sym,
				&fr->filename, &fr->linenum,pc);

	if (fr->filename == 0) {	/* XXX ie, stripped */
		fr->narg = 5;
	} else {
		fr->narg = 0;
		for (sp = 1 + (SYMR*)pr->isym; sp->st == stParam; sp++)
			fr->narg++;
	}

	/* Leaf procedures do not save the ra */
	fr->isleaf = (pr->regmask & 0x80000000) == 0;

	fr->framesize = pr->frameoffset;
	fr->saved_pc_off = pr->regoffset;
	if (pr->frameoffset) {
		fr->nloc = (pr->frameoffset / sizeof(int)) - fr->narg;
	} else {
		fr->nloc = 0;
	}
	fr->mod_sp = (pr->frameoffset != 0);

	fr->isvector = ((pr->pcreg == 0)
		&& (fr->framesize == sizeof(struct mips_saved_state)));
	return TRUE;
}

/*
 * Return name and value of a symbol.
 * Since we have symbols of various types this
 * is messier than it should be. Oh well.
 */
coff_db_symbol_values(sym, namep, valuep)
	register db_sym_t	*sym;
	char		**namep;
	db_expr_t	*valuep;
{
	if ((EXTR*)sym  == last_external_symbol) {
ext:
		*namep = (char*) ((EXTR*)sym)->asym.iss;
		*valuep = ((EXTR*)sym)->asym.value;
/*		ret.n_type  = es->asym.st; ret.n_other = es->asym.sc; */
	} else
	if ((PDR*)sym  == last_procedure_symbol) {
proc:
		*namep = (char*) ((SYMR*)((PDR*)sym)->isym)->iss;
		if (last_proc_line_addr)
			*valuep = last_proc_line_addr;
		else
			*valuep = ((PDR*)sym)->adr;
/*		ret.n_type  = sp->st; ret.n_other = sp->sc; */
	} else
	if ((SYMR*)sym  == last_local_symbol) {
loc:
		*namep = (char*)((SYMR*)sym)->iss;
		*valuep = ((SYMR*)sym)->value;
/*		ret.n_type  = sp->st; ret.n_other = sp->sc; */
	} else {
		/* Wasn't cached, go look for it. */
		HDRR *stab = (HDRR *)db_last_symtab->private;

		if (stab == 0) { /* won't happen, don't panic */
			*namep = "???";
			*valuep = 0;
		}
		if ((EXTR*)sym >= (EXTR*)stab->cbExtOffset &&
		    (EXTR*)sym < (((EXTR*)stab->cbExtOffset) + stab->iextMax))
			goto ext;		
		if ((PDR*)sym >= (PDR*)stab->cbPdOffset &&
		    (PDR*)sym < (((PDR*)stab->cbPdOffset) + stab->ipdMax))
			goto proc;
		if ((SYMR*)sym >= (SYMR*)stab->cbSymOffset &&
		    (SYMR*)sym < (((SYMR*)stab->cbSymOffset) + stab->isymMax))
			goto loc;
		panic("db_symbol_values");
	}
}

/*
 *	Initialization functions
 */
#include <mips/coff.h>

read_mips_symtab()
{
	char *esym;
	int stsize, st_hdrsize;
	unsigned st_filptr;
	HDRR *symtab;

	extern vm_offset_t	kern_sym_start;

	symtab = (HDRR*)kern_sym_start;
	if (symtab->magic != magicSym) {
		dprintf("[ symbol table: bad magic]\n");
		return;
	}

	st_filptr = symtab->cbLineOffset - sizeof(HDRR);
	st_hdrsize = sizeof(HDRR);

	/* find out how much stuff is there */
	stsize = (symtab->cbExtOffset + symtab->iextMax * cbEXTR)
		 - (st_filptr + st_hdrsize);
	dprintf("[ preserving %d bytes of mach symbol table ]\n", stsize);

	esym = (char*)symtab + stsize + st_hdrsize;

	/* Change all file pointers into memory pointers */
	fixup_symtab(symtab, (char*)symtab + st_hdrsize,
		     st_filptr + st_hdrsize);
	db_add_symbol_table(SYMTAB_COFF, (char*)symtab, esym, "mach", (char *)symtab,
			    (char *)0);
}

/*
 *	Load an external symbol table.
 */
boolean_t
coff_db_sym_init(symtab_start, symtab_end, name, map_pointer)
	char *	symtab_start;
	char *	symtab_end;
	char *	name;
	char *	map_pointer;	/* opaque pointer */
{
	int	st_fil_start;
	int	st_fil_end;
	int	st_size;
	HDRR	*symtab;

	/*
	 * All the pointers in the symbol table are offsets
	 * relative to the start of the file.  We must relocate
	 * them to be actual pointers.
	 *
	 * We do not know where in the file the header was.
	 * However:
	 *   We know the size of the symbol table.
	 *   We also know that (assuming external symbol entries
	 *   are last in the symbol table)
	 *	cbExtOffset + iextMax*cbEXTR
	 *   is a file_relative pointer to the end of the
	 *   symbol table.
	 * So we can calculate where in the file the header was.
	 */

	st_size = symtab_end - symtab_start;

	symtab = (HDRR *) symtab_start;
	st_fil_end = symtab->cbExtOffset + symtab->iextMax * cbEXTR;

	st_fil_start = st_fil_end - st_size;

	/*
	 * Fix up the symbol table to hold in-memory pointers.
	 */
	fixup_symtab(symtab,
		     (char *) symtab,
		     st_fil_start);

	db_printf("[ preserving %#x bytes of %s symbol table ]\n",
		symtab_end - symtab_start, name);

	return db_add_symbol_table(SYMTAB_COFF,
				   symtab_start,
				   symtab_end,
				   name,
				   (char *) symtab,
				   map_pointer);
}

/*
 * Fixup the symbol table to hold in-memory pointers, as opposed
 * to file displacements
 */
fixup_symtab(hdr, data, f_ptr)
	HDRR		*hdr;
	char		*data;
	int		f_ptr;
{
	int             f_idx, s_idx;
	FDR            *fh;
	SYMR	       *sh;
	OPTR	       *op;
	PDR	       *pr;
	EXTR	       *esh;
	static SYMR	static_procedure =
			{ (long)"<static procedure>", 0, stStatic, scText, 0, 0};

#define FIX(off) \
	if (hdr->off) hdr->off = (unsigned int)data + (hdr->off - f_ptr);

	FIX(cbLineOffset);
	FIX(cbDnOffset);
	FIX(cbPdOffset);
	FIX(cbSymOffset);
	FIX(cbOptOffset);
	FIX(cbAuxOffset);
	FIX(cbSsOffset);
	FIX(cbSsExtOffset);
	FIX(cbFdOffset);
	FIX(cbRfdOffset);
	FIX(cbExtOffset);
#undef FIX
	/*
	 * Now go on and fixup all the indexes within the symtab itself.
	 * We must work on a file-basis, hence the mess. 
	 */
	for (f_idx = 0; f_idx < hdr->ifdMax; f_idx++) {
		register int stripped;
		register unsigned code_off;

		fh = (FDR *) (hdr->cbFdOffset + f_idx * sizeof(FDR));
		/* fix file descriptor itself */
		fh->issBase += hdr->cbSsOffset;
		fh->isymBase = hdr->cbSymOffset + fh->isymBase * sizeof(SYMR);
		fh->ioptBase = hdr->cbOptOffset + fh->ioptBase * sizeof(OPTR);
		/* cannot fix fh->ipdFirst since it is a short */
#define IPDFIRST	((long)hdr->cbPdOffset + fh->ipdFirst * sizeof(PDR))
		fh->iauxBase = hdr->cbAuxOffset + fh->iauxBase * sizeof(AUXU);
		fh->rfdBase = hdr->cbRfdOffset + fh->rfdBase * sizeof(FDR);
		fh->cbLineOffset += hdr->cbLineOffset;
		code_off = fh->adr;
		stripped = (fh->rss == -1);	/* lang==4 ? */
		if (stripped)
			fh->rss = (long)fh_noname;
		else
			fh->rss = (long)fh->rss + fh->issBase;

		/* fixup local symbols */
		for (s_idx = 0; s_idx < fh->csym; s_idx++) {
			sh = (SYMR*)(fh->isymBase + s_idx * sizeof(SYMR));
			sh->iss = (long) sh->iss + fh->issBase;
		}
		/* fixup procedure symbols */
		for (s_idx = 0; s_idx < fh->cpd; s_idx++) {
			pr = (PDR*)(IPDFIRST + s_idx * sizeof(PDR));
			if (stripped) {
				if (pr->isym == -1) {
					pr->isym = (long)&static_procedure;
				} else {
					esh = (EXTR*)(hdr->cbExtOffset + pr->isym * sizeof(EXTR));
					pr->isym = (long)&(esh->asym);
				}
			} else {
				pr->isym = fh->isymBase + pr->isym * sizeof(SYMR);
				if (s_idx == 0 && pr->adr != 0) {
					code_off -= pr->adr;
				}
				pr->adr += code_off;
			}

			/* Fix line numbers */
			pr->cbLineOffset += fh->cbLineOffset;

		}
		/* forget about everything else */
	}

	/* fixup external symbols */
	for (s_idx = 0; s_idx < hdr->iextMax; s_idx++) {
		esh = (EXTR*)(hdr->cbExtOffset + s_idx * sizeof(EXTR));
		esh->asym.iss = esh->asym.iss + hdr->cbSsExtOffset;
	}
}

