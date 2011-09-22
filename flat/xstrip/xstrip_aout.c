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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	xstrip_aout.c,v $
 * Revision 2.1.1.1  94/06/01  10:24:48  rvb
 * 	From BSDSS
 * 
 * Revision 2.2  93/05/12  13:28:06  rvb
 * 	Added a few #ifdef __386BSD__s
 * 
 * Revision 2.4  93/03/23  10:16:23  mrt
 * 	Added keeping of N_FRAME symbols, if defined.
 * 	Print out how big the file would be if "stripped =",
 * 	coherent with the original intent (and mips version).
 * 	Works on alpha (native).  Spots. Made pds's changes
 * 	64-bit safe. (size_t is not the same as int)
 * 	[93/02/22            af]
 * 
 * Revision 2.3  93/03/21  18:12:45  mrt
 * 	Added conditional defintions of SEEK_CUR and SEEK_SET
 * 	so that we can compile with older versios of stdio.h
 * 	[93/03/20            mrt]
 * 
 * Revision 2.2  93/02/03  18:02:21  mrt
 * 	Moved from user, added pds's changes for gdb compatibility
 * 
 * Revision 2.4.1.1  92/12/23  17:46:50  pds
 * 	Converted file to ANSI C.
 * 	[92/12/23            pds]
 * 
 * Revision 2.4  92/01/22  22:53:03  rpd
 * 	Added code to truncate the N_FUN symbols, in support of keeping
 * 	the string table small.
 * 	[92/01/22            danner]
 * 
 * Revision 2.3  92/01/17  14:25:36  rpd
 * 	Added include "ccdep.h" for compiler dependent definition.
 * 	Changed to keep non-external and N_FUN symbols as a default.
 * 	Added options to remove non-externl symbols and to keep
 * 	  function's local symbol.
 * 	[91/08/29            tak]
 * 
 * Revision 2.2  91/08/29  16:47:37  jsb
 * 	First checkin.
 * 
 */
/*
 *	File:	xstrip_aout.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <memory.h>
#ifndef	__386BSD__
#include <sysent.h>
#endif
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#ifdef	__386BSD__
#include <sys/time.h>
#endif
#include <sys/resource.h>
#include <a.out.h>
#include <stab.h>

#include "ccdep.h"

#ifndef SEEK_SET
/* These three are almost always defined together so one test should do. */
#define SEEK_CUR (1)
#define SEEK_END (2)
#define SEEK_SET (0)
#endif

int delete_non_external = 0;	/* delete non external symbol */
int keep_func_local = 0;	/* keep function's local symbol */

/*
 * Convert
 *	../../../../../src/latest/kernel/vm/vm_map.c
 * to
 *	vm/vm_map.c
 * by going to end, backing up two slashes, and skipping one.
 */
void
trim_filename(char **sp)
{
	char *s = *sp;
	char *t;
	int scount = 0;

	if (*s == '\0') return;

	for (t = s; *t; t++)
		continue;

	for (; t > s; t--)
		if (*t == '/' && ++scount == 2) {
			*sp = t + 1;
			return;
		}
}

/*
  compilers emit function stab references that look like
  function_name:{F/f}<type>. We want to blast everything from
  the colon onwards.
*/
void
trunc_funname(char *s)
{
	while (*s != '\0' && *s!=':')
		s++;

	*s = '\0';
	return;
}

/*
 * Unrestrict data size limit, since we malloc large things.
 */
void
unlimit_data(void)
{
	struct rlimit rl;

	if (getrlimit(RLIMIT_DATA, &rl) != -1) {
		rl.rlim_cur = rl.rlim_max;
		setrlimit(RLIMIT_DATA, &rl);
	}
}

off_t
f_current_position(int fd)
{
	off_t rv;

	rv = lseek(fd, 0, SEEK_CUR);
	if (rv == -1) {
		perror("f_current_position: lseek");
		exit(1);
	}
	return rv;
}

off_t
f_size(int fd)
{
	struct stat st;
	int rv;

	rv = fstat(fd, &st);
	if (rv == -1) {
		perror("f_size: fstat");
		exit(1);
	}
	return st.st_size;
}

void
read_header(int fd, struct exec *exec, off_t *symtab_offset,
	    unsigned int *symtab_size, off_t *strtab_offset,
	    unsigned int *strtab_size)
{
	read(fd, (char *)exec, sizeof(*exec));
	if (N_BADMAG(*exec)) {
		fprintf(stderr, "bad magic\n");
		exit(1);
	}
	*symtab_offset = N_SYMOFF(*exec);
	*symtab_size = exec->a_syms;
	if (symtab_size == 0) {
		fprintf(stderr, "no name list\n");
		exit(1);
	}
	if (N_STROFF(*exec) + sizeof(off_t) > f_size(fd)) {
		fprintf(stderr, "no string table\n");
		exit(1);
	}
	*strtab_offset = *symtab_offset + *symtab_size;
	lseek(fd, *strtab_offset, SEEK_SET);
	read(fd, strtab_size, sizeof(*strtab_size));
}

char *
read_table(int fd, off_t offset, unsigned int size)
{
	char *table;
	int rv;

	table = (char *) malloc((size_t)size);
	if (table == 0) {
		fprintf(stderr, "read_table: out of memory\n");
		exit(1);
	}
	lseek(fd, offset, SEEK_SET);
	rv = read(fd, table, size);
	if (rv < 0) {
		perror("read_table: read");
		exit(1);
	}
	if (rv != size) {
		fprintf(stderr, "read_table: short read\n");
		exit(1);
	}
	return table;
}

void
symtab_strip(struct nlist **symtab, unsigned int *symtab_size, const char *strtab)
{
	register struct nlist *sp, *ep, *np;
	struct nlist *new_symtab;

	new_symtab = (struct nlist *) malloc(*symtab_size);
	if (new_symtab == 0) {
		fprintf(stderr, "symtab_strip: out of memory\n");
		exit(1);
	}
	ep = &(*symtab)[*symtab_size / sizeof(struct nlist)];
	np = new_symtab;
	for (sp = *symtab; sp < ep; sp++) {
		if (sp->n_type & N_STAB) {
			if ((sp->n_type & 0xff) == N_SLINE ||
#ifdef	N_FRAME
			    (sp->n_type & 0xff) == N_FRAME ||
#endif
			    (sp->n_type & 0xff) == N_SO ||
			    (sp->n_type & 0xff) == N_FUN) {
				*np++ = *sp;
				continue;
			}
		}
		if (sp->n_type & N_EXT) {
			*np++ = *sp;
			continue;
		} else if (delete_non_external)
			continue;
		switch(sp->n_type) {
		case N_TEXT:
			/*
			 * Ignore compiler ID symbols like "gcc_compiled."
			 * Note:
			 *	 Since GCC produces a file name as a text
			 *	 symbol, the file name symbol is kept.
			 */
			if (strcmp(sp->n_un.n_strx+strtab, COMPILER_ID) != 0) {
				*np++ = *sp;
			}
			continue;
		case N_DATA:
		case N_BSS:
			/*
			 * Ignore function local static symbols to eliminate
			 * a lot of symbols produced by MIG.
			 */
			if (keep_func_local || 
				!func_local(sp->n_un.n_strx+strtab)) {
				*np++ = *sp;
			}
			continue;
		}
	}
	printf("stripped %d bytes from symbol table\n",
	       *symtab_size - (np - new_symtab) * sizeof(*np));
	free(*symtab);
	*symtab = new_symtab;
	*symtab_size = (np - new_symtab) * sizeof(*np);
}

void
strtab_strip(char **strtab, unsigned int *strtab_size, struct nlist *symtab,
	     unsigned int symtab_size)
{
	register struct nlist *sp, *ep;
	char *new_strtab, *s;
	unsigned int new_strtab_size, len;

	new_strtab = (char *) malloc(*strtab_size);
	if (new_strtab == 0) {
		fprintf(stderr, "strtab_strip: out of memory\n");
		exit(1);
	}
	new_strtab_size = (unsigned int) sizeof(new_strtab_size);

	ep = &symtab[symtab_size / sizeof(*ep)];
	for (sp = symtab; sp < ep; sp++) {
		if (sp->n_un.n_strx == 0)
			continue;

		s = sp->n_un.n_strx + *strtab;
		if (sp->n_type & N_STAB) {
			if ((sp->n_type & 0xff) == N_SO)
				trim_filename(&s);
			else if ((sp->n_type & 0xff) == N_FUN)
				trunc_funname(s);
		}
		len = strlen(s) + 1;
		memcpy(new_strtab + new_strtab_size, s, len);
		sp->n_un.n_strx = new_strtab_size;
		new_strtab_size += len;
	}
	printf("stripped %d bytes from string table\n",
	       *strtab_size - new_strtab_size);
	* (unsigned int *) new_strtab = new_strtab_size;
	free(*strtab);
	*strtab = new_strtab;
	*strtab_size = new_strtab_size;
}

void
write_header(int fd, struct exec *exec, size_t symtab_size)
{
	int rv;

	exec->a_syms = symtab_size;
	lseek(fd, offsetof(struct exec, a_syms), SEEK_SET);
	rv = write(fd, &exec->a_syms, sizeof(exec->a_syms));
	if (rv < 0) {
		perror("write_header: write");
		exit(1);
	}
	if (rv != sizeof(exec->a_syms)) {
		fprintf(stderr, "write_header: short write");
		exit(1);
	}
}

void
write_table(int fd, void *table, off_t offset, size_t size)
{
	int rv;

	lseek(fd, offset, SEEK_SET);
	rv = write(fd, table, size);
	if (rv < 0) {
		perror("write_table: write");
		exit(1);
	}
	if (rv != size) {
		fprintf(stderr, "write_table: short write\n");
		exit(1);
	}
	free(table);
}

main(int argc, char **argv)
{
	struct exec exec;
	int fd;
	char *strtab;
	struct nlist *symtab;
	off_t symtab_offset, strtab_offset;
	unsigned int symtab_size;	/* NB: comes from file, 4 bytes! */
	unsigned int strtab_size;	/* NB: comes from file, 4 bytes! */
	const char *file;

	unlimit_data();

	/*
	 * Open file
	 */
	if (argc != 2 && (argc != 3 || argv[1][0] != '-')) {
		fprintf(stderr, "usage: %s [-xl] filename\n", argv[0]);
		exit(1);
	}
	if (argc == 3) {
		switch(argv[1][1]) {
		case 'x':
			delete_non_external++;
			break;
		case 'l':
			keep_func_local++;
			break;
		default:
			fprintf(stderr, "unknown option %s\n", argv[1]);
			exit(1);
		}
		file = argv[2];
	} else
		file = argv[1];
		
		
	fd = open(file, O_RDWR, 0);
	if (fd < 0) {
		perror(file);
		exit(1);
	}

	/*
	 * Read header.
	 */
	read_header(fd, &exec, &symtab_offset, &symtab_size,
		    &strtab_offset, &strtab_size);

	/*
	 * Read symbol and string table and process symbol table.
	 */
	symtab = (struct nlist *) read_table(fd, symtab_offset, symtab_size);
	strtab = read_table(fd, strtab_offset, strtab_size);
	symtab_strip(&symtab, &symtab_size, strtab);

	/*
	 * Process string table.
	 */
	strtab_strip(&strtab, &strtab_size, symtab, symtab_size);

	/*
	 * Write header, symbol table, and string table.
	 */
	write_header(fd, &exec, symtab_size);
	write_table(fd, symtab, symtab_offset, (size_t)symtab_size);
	strtab_offset = f_current_position(fd);
	write_table(fd, strtab, strtab_offset, strtab_size);

	/*
	 * Truncate file at current file pointer position, close, and exit.
	 */
	printf("%s: truncating from %d to %d (stripped = %d)\n",
	       file,
	       f_size(fd),
	       f_current_position(fd),
	       symtab_offset);
	ftruncate(fd, f_current_position(fd));
	close(fd);
	exit(0);
}
