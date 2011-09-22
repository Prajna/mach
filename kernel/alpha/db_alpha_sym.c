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
 * $Log:	db_alpha_sym.c,v $
 * Revision 2.4  93/05/28  22:35:12  rvb
 * 	Reflect change in VECTOR macro.
 * 	[93/05/21            af]
 * 
 * Revision 2.3  93/03/09  10:50:00  danner
 * 	Findproc now exists for a.out too.
 * 	[93/03/05            af]
 * 
 * Revision 2.2  93/02/05  07:58:02  danner
 * 	Turned on all prints now that prom interface is ok.
 * 	[93/02/04  00:39:42  af]
 * 
 * 	Created, from mips version.
 * 	[92/05/31            af]
 * 
 */
/*
 *	File: db_alpha_sym.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	8/89
 *
 *	Symtab module for COFF files (MIPS/DEC compiler for ALPHA)
 */

#include <mach/std_types.h>
#include <machine/db_machdep.h>
#include <ddb/db_sym.h>

#include <vm/vm_kern.h>
#include <alpha/thread.h>
#include <alpha/db_trace.h>
#include <alpha/syms.h>
#include <mach/alpha/asm.h>


#define	DEBUG	0

/* Simple predicates */
#define IS_DATA(c,t)	(((c)==scData)||((c)==scSData)||\
			 ((c)==scCommon)||((c)==scSCommon))
#define IS_BSS(c,t)	(((c)==scBss)||((c)==scSBss))
#define IS_TEXT(c,t)	(((c)==scText)||((c)==scRData))
#define IS_LOCAL(c,t)	((t)==stLocal)
#define IS_PARAM(c,t)	((t)==stParam)
#define IS_ABS(c,t)	((c)==scAbs)
#define IS_REG(c,t)	((c)==scRegister)

static vm_size_t localval;

static EXTR *last_external_symbol;
static PDR  *last_procedure_symbol;
static SYMR *last_local_symbol;
static vm_offset_t last_proc_line_addr;


/*
 * To make things quick, we would like to have the in-memory
 * symbol table hold real pointers and not file offsets.
 * But many fields are too narrow, so we must preserve them
 * as offsets and just use some access funcs/macros in the rest
 * of the code.
 * fixup_symtab() is the function that makes the following work,
 * do not forget to fix it if you change anything in this section.
 */
static void fixup_symtab(HDRR *, char *, unsigned int);

#define fh_iss_base(hdr,fh)	((hdr)->cbSsOffset + (fh)->issBase)
#define fh_isym_base(hdr,fh)	((hdr)->cbSymOffset + (fh)->isymBase)
#define fh_iopt_base(hdr,fh)	((hdr)->cbOptOffset + (fh)->ioptBase)
#define fh_ipd_first(hdr,fh)	((hdr)->cbPdOffset + (fh)->ipdFirst)
#define fh_iaux_base(hdr,fh)	((hdr)->cbAuxOffset + (fh)->iauxBase)
#define fh_rfd_base(hdr,fh)	((hdr)->cbRfdOffset + (fh)->rfdBase)
#define fh_cbline_offset(hdr,fh) ((fh)->cbLineOffset)
#define fh_rss(hdr,fh)		((hdr)->cbSsOffset + (fh)->rss)

#define pr_at_index(hdr,fh,i)	(PDR*)(fh_ipd_first(hdr,fh) + (i) * sizeof(PDR))
static SYMR *pr_isym(HDRR *, PDR *);
static char *pr_iss(HDRR *, PDR *);

#define sh_at_index(hdr,fh,i)	(SYMR*)(fh_isym_base(hdr,fh) + (i) * sizeof(SYMR))
#define sh_iss(hdr,sh)		(char*)((hdr)->cbSsOffset + (sh)->iss)

#define esh_at_index(hdr,i)	(EXTR*)((hdr)->cbExtOffset + (i) * sizeof(EXTR));
#define esh_iss(hdr,esh)	(char*)((hdr)->cbSsExtOffset + (esh)->asym.iss)


static SYMR*
pr_isym(symtab,pr)
	HDRR	*symtab;
	PDR	*pr;
{
	static SYMR	static_procedure =
		{ 0, 0, stStatic, scText, 0, 0};

	if (pr->reserved) {
		if (pr->isym == -1) {
			return &static_procedure;
		} else {
			return & ((EXTR*)(symtab->cbExtOffset + pr->isym))->asym;
		}
	} else {
		return (SYMR*) ((symtab)->cbSymOffset + pr->isym);
	}
}

static char *
pr_iss(symtab, pr)
	register HDRR	*symtab;
	register PDR	*pr;
{
	register char	*name;

	/* was the file stripped ? */
	if (pr->reserved) {
		/* so did we lose static infos ? */
		if (pr->isym == -1) {
			name = "<static procedure>";
		} else {
			register EXTR *esh;

			esh = (EXTR*)(symtab->cbExtOffset + pr->isym);
			name = esh_iss(symtab, esh);
		}
	} else {
		register SYMR *sh;

		sh = (SYMR*) ((symtab)->cbSymOffset + pr->isym);
		name = sh_iss(symtab, sh);
	}
	return name;
}

/*
 * Symbol table searching
 */

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

/* lookup a symbol by line number (sortof) */
static db_sym_t
coff_db_lookup_proc_line(symtab, pd, line_number)
	HDRR 	*symtab;
	PDR	*pd;
	int	line_number;
{
	register char *lp;
	register vm_offset_t addr;
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
		if ( strcmp( esh_iss(symtab,es), sym_name) == 0) {
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
		if (strcmp( pr_iss(symtab,pd), sym_name) == 0) {
			last_proc_line_addr = 0;
			last_procedure_symbol = pd;
			return(pd);
		}
	}
	return(0);
}

/* compare for equality, modulo traling dot */
static boolean_t
coff_streq(str1, str2)
register const char *str1, *str2;
{
	for ( ; *str2; str1++, str2++) {
		if (*str1 != *str2)
			return(FALSE);
	}
	return(*str2 == 0 && (*str1 == 0 || *str1 == '.'));
}
		
/* lookup a symbol by filename, name, and possibly line number */
static db_sym_t
coff_db_qualified_search(stab, file_name, sym_name, line_number)
	db_symtab_t	*stab;
	char		*file_name, *sym_name;
{
	HDRR	*symtab;
	register i;
	register SYMR	*ssp;
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
			if (fh->rss == 0 || fh->rss == -1)
				continue;
			for (cp = p = (char*)fh_rss(symtab,fh); *p; p++) {
				if (*p == '/')
					cp = p + 1;
			}
			if (strcmp(cp, file_name) == 0)
				break;
		}
		if (i >= symtab->ifdMax)
			return(DB_SYM_NULL);

		/* search symbol in the file */
		ssp = (SYMR *)(fh_isym_base(symtab,fh));
		for (i = 0; i < fh->csym; i++, ssp++) {
			if (ssp->st != stStatic && ssp->st != stGlobal &&
			    ssp->st != stProc && ssp->st != stStaticProc)
				continue;
			if (coff_streq(sh_iss(symtab,ssp), sym_name))
				break;
		}
		if (i >= fh->csym)
			return((db_sym_t)coff_lookup_external(symtab, sym_name));

		if (ssp->st == stStatic || ssp->st == stGlobal) {
			if (line_number != 0)
				return DB_SYM_NULL;
			last_local_symbol = ssp;
			return((db_sym_t) ssp);
		}
		/*
		 * get function descriptor entry
		 */
		pd = (PDR *) (fh_ipd_first(symtab,fh));
		for (i = 0; i < fh->cpd; i++, pd++) {
			if (pr_isym(symtab,pd) == ssp)
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
		ssp = (SYMR *)symtab->cbSymOffset;
		for (i = 0; i < symtab->isymMax; i++, ssp++) {
			if (ssp->st != stStatic)
				continue;
			if (coff_streq(sh_iss(symtab,ssp), sym_name)) {
				last_local_symbol = ssp;
				return((db_sym_t)ssp);
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
	register SYMR *ssp, *best_ssp = 0;
	register EXTR *es, *best_es = 0;
	register PDR  *pr, *best_pr = 0;
	int i;
	vm_offset_t newdiff, diff = *diffp;
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
		newdiff = value - (vm_offset_t)es->asym.value;
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
		newdiff = value - (vm_offset_t)pr->adr;
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

	for (ssp = (SYMR*)symtab->cbSymOffset, i = 0;
	     i < symtab->isymMax; i++, ssp++) {
		newdiff = value - (vm_offset_t)ssp->value;
		if (newdiff < diff) {
			diff = newdiff;
			best_ssp = ssp;
			if (diff == 0)
				break;
		}
	}
	if (best_ssp) {
		last_local_symbol = best_ssp;
		last_thing_found = (char*)best_ssp;
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
	HDRR	*symtab;
	register SYMR *ssp;
	register SYMR *maxsym;
	static int nesting = 0;

	if (sym == DB_SYM_NULL)
		return sym;

	if (db_last_symtab == 0) {
		return DB_SYM_NULL;	/* sanity? */
	}
	symtab = (HDRR*)db_last_symtab->private;

	maxsym = ((SYMR*)symtab->cbSymOffset) + symtab->isymMax;
	/*
	 * "sym" is a token which we'll pass back.  We are basically
	 * scanning all local symbols starting from the procedure's
	 * itself (e.g. sym is a PDR on first call, a SYMR afterwards)
	 */
	if ((PDR*) sym == last_procedure_symbol) {
		ssp = pr_isym(symtab, last_procedure_symbol);
		nesting = 1;
	} else
		ssp = (SYMR*) sym;

	/* Note: a procedure's symbols really start _after_ pr->isym */
	while (++ssp < maxsym && nesting > 0) {

		register int   sc, st;

		last_local_symbol = ssp;

		sc = ssp->sc;
		st = ssp->st;

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
			return (db_sym_t)ssp;
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
		pd_base = (PDR *) (fh_ipd_first(symtab,fh));
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
fdr_filename(symtab,fh)
	HDRR	*symtab;
	FDR	*fh;
{
	char *s, *str0, *str1;

	if (! fh || ! fh->rss || fh->rss == -1) {
		return 0;
	}
	for (str0 = str1 = s = (char *)fh_rss(symtab,fh); *s; s++) {
		if (s[0] == '/' && s[1]) {
			str0 = str1;
			str1 = &s[1];
		}
	}
	return str0;
}

/* given a PC and some more info return line number in file */
int
fdr_linenum(symtab, fh, pr, pc)
	HDRR *symtab;
	FDR *fh;
	PDR *pr;
	db_expr_t pc;
{
	char *base;
	PDR *pr0, *pr_end;
	long halt, lineno, best_offset, best_lineno, delta, linepc, count;

	/*
	 *  If we don't have enough info, return linenum of 0.
	 */
	if (! fh || ! fh_cbline_offset(symtab,fh) || ! pr) {
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
	pr_end = (PDR *) (fh_ipd_first(symtab,fh)) + fh->cpd;
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
	linepc = (long)(pr->adr >> 2);	/* in words */
	pc = (long)((vm_offset_t)pc >> 2);
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
	register FDR	*fh;
	HDRR		*symtab;

	if (sym == DB_SYM_NULL)
		return FALSE;
	symtab = (HDRR*)db_last_symtab->private;
	fh = findfdr(symtab, sym);
	*filenamep = fdr_filename(symtab,fh);
	*linep = fdr_linenum(symtab, fh, sym, pc);
	return TRUE;
}


/*
 * Find the descriptor for the function described by sym (a PDR)
 * return all we know about it.
 */
boolean_t
coff_findproc(
	db_symtab_t	*symtab,
	db_sym_t	sym,
	frame_info_t 	fr,
	db_expr_t	pc)
{
	register PDR  *pr;
	register SYMR *ssp;
	register FDR *fh;

	if (sym == DB_SYM_NULL)
		return FALSE;

	pr = (PDR*) sym;	/* opaque revealed */

	(void) coff_db_line_at_pc( symtab, sym,
				&fr->filename, &fr->linenum,pc);

	if (fr->filename == 0) {	/* XXX ie, stripped */
		fr->narg = 5;
	} else {
		fr->narg = 0;
		ssp = pr_isym((HDRR*)symtab->private, pr) + 1;
#if	DEBUG
		db_printf("[s: %x]", ssp);
#endif	/* DEBUG */
		for ( ; ssp->st == stParam; ssp++)
			fr->narg++;
	}

	/* Leaf procedures do not save the ra */
	fr->isleaf = (pr->regmask & IM_RA) == 0;

	fr->framesize = pr->frameoffset;
	fr->regmask = pr->regmask;
	fr->saved_pc_off = pr->regoffset;
	if (pr->frameoffset) {
		fr->nloc = (pr->frameoffset / sizeof(long)) - fr->narg;
	} else {
		fr->nloc = 0;
	}
	fr->mod_sp = (pr->frameoffset != 0);

	fr->isvector = ((pr->pcreg == 26) && (pr->regmask & IM_EXC) 
		&& (fr->framesize == sizeof(struct alpha_saved_state)));
#if 0
if (pr->regmask & IM_EXC) db_printf("{VEC %x %x %x}", pr->regmask, pr->pcreg, fr->framesize);
#endif
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
	HDRR *symtab = (HDRR*)db_last_symtab->private;

	if ((EXTR*)sym  == last_external_symbol) {
ext:
		*namep = esh_iss(symtab, ((EXTR*)sym) );
		*valuep = ((EXTR*)sym)->asym.value;
	} else


	if ((PDR*)sym  == last_procedure_symbol) {
proc:
		*namep = pr_iss(symtab, ((PDR*)sym));
		if (last_proc_line_addr)
			*valuep = last_proc_line_addr;
		else
			*valuep = ((PDR*)sym)->adr;
	} else


	if ((SYMR*)sym  == last_local_symbol) {
loc:
		*namep = sh_iss(symtab, ((SYMR*)sym) );
		*valuep = ((SYMR*)sym)->value;

	} else {

		/* Wasn't cached, go look for it. */

		if (symtab == 0) { /* won't happen, don't panic */
			*namep = "???";
			*valuep = 0;
		}
		if ((EXTR*)sym >= (EXTR*)symtab->cbExtOffset &&
		    (EXTR*)sym < (((EXTR*)symtab->cbExtOffset) + symtab->iextMax))
			goto ext;		
		if ((PDR*)sym >= (PDR*)symtab->cbPdOffset &&
		    (PDR*)sym < (((PDR*)symtab->cbPdOffset) + symtab->ipdMax))
			goto proc;
		if ((SYMR*)sym >= (SYMR*)symtab->cbSymOffset &&
		    (SYMR*)sym < (((SYMR*)symtab->cbSymOffset) + symtab->isymMax))
			goto loc;
		panic("db_symbol_values");
	}
}

/*
 *	Initialization functions
 */
#include <alpha/coff.h>

read_alpha_symtab()
{
	char           *esym;
	int             stsize, st_hdrsize;
	unsigned int   st_filptr;
	HDRR           *symtab;

	extern vm_offset_t kern_sym_start;

	symtab = (HDRR *) kern_sym_start;
	if (db_alpha_symtab_type( (char *) symtab) != SYMTAB_COFF) {
		dprintf("[ symbol table: bad magic]\n");
		return;
	}

	st_hdrsize = sizeof(HDRR);
	st_filptr = symtab->cbLineOffset - st_hdrsize;

	/* find out how much stuff is there */
	stsize = (symtab->cbExtOffset + symtab->iextMax * cbEXTR)
		- symtab->cbLineOffset;
	dprintf("[ preserving %#x bytes of mach symbol table ]\n", stsize);

	esym = (char *) symtab + st_hdrsize + stsize;

	/* Change all file pointers into memory pointers */
	fixup_symtab(symtab, (char *) symtab + st_hdrsize,
		     st_filptr + st_hdrsize);
	db_add_symbol_table(SYMTAB_COFF, (char *) symtab, esym, "mach", (char *) symtab,
			    (char *) 0);
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
	unsigned int	st_fil_start;
	unsigned int	st_fil_end;
	int		st_size;
	HDRR		*symtab;

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
	if (symtab->magic != magicSym)
		return FALSE;

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


db_alpha_symtab_type( char *st )
{
	HDRR           *symtab = (HDRR *)st;

	if (symtab->magic == magicSym)
		return SYMTAB_COFF;
	return SYMTAB_AOUT;
}

/*
 * Fixup the symbol table to hold in-memory pointers, as opposed
 * to file displacements.
 */
static void
fixup_symtab(symtab, data, f_ptr)
	HDRR		*symtab;
	char		*data;
	unsigned int	f_ptr;
{
	int             f_idx, s_idx;
	FDR            *fh;
	SYMR	       *sh;
	OPTR	       *op;
	PDR	       *pr;
	EXTR	       *esh;

#define FIX(off) \
	if (symtab->off) symtab->off = (vm_offset_t)data + (symtab->off - f_ptr);

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
	 * We must work on a file-basis.
	 */
	for (f_idx = 0; f_idx < symtab->ifdMax; f_idx++) {
		register int stripped;
		register vm_offset_t code_off;

		fh = (FDR *) (symtab->cbFdOffset + f_idx * sizeof(FDR));

		/*
		 * Fix file descriptor itself. See macros atop.
		 */
		fh->isymBase *= sizeof(SYMR);
		fh->ioptBase *= sizeof(OPTR);
		fh->ipdFirst *= sizeof(PDR);
		fh->iauxBase *= sizeof(AUXU);
		fh->rfdBase *= sizeof(FDR);    
		fh->cbLineOffset += symtab->cbLineOffset;	/* this one fits */

		/* name and address of the file */
		stripped = (fh->rss == -1);	/* lang==4 ? */
		if (!stripped)
			fh->rss += fh->issBase;
		code_off = fh->adr;

		/*
		 * Fixup local symbols. See macros atop.
		 */
		for (s_idx = 0; s_idx < fh->csym; s_idx++) {
			sh = sh_at_index(symtab,fh,s_idx);
			sh->iss += fh->issBase;
		}

		/*
		 * Fixup procedure symbols. See pr_xxx functions atop.
		 */
		for (s_idx = 0; s_idx < fh->cpd; s_idx++) {
			pr = pr_at_index(symtab,fh,s_idx);
			pr->reserved = stripped;
			if (stripped) {
				if (pr->isym != -1) {
					pr->isym *= sizeof(EXTR);
				}
			} else {
				pr->isym = fh->isymBase + pr->isym * sizeof(SYMR);
				if (s_idx == 0 && pr->adr != 0) {
					code_off -= pr->adr;
				}
				pr->adr += code_off;
			}

			/*
			 * Fix line numbers
			 */
			pr->cbLineOffset += fh->cbLineOffset;	/* this one fits */

		}
		/* forget about everything else */
	}

	/*
	 * External symbols. Nothing needed, see macros atop.
	 */
}

