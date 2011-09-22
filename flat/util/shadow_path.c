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
 * $Log:	shadow_path.c,v $
 * Revision 2.1.1.1  94/06/01  10:24:25  rvb
 * 	From BSDSS
 * 
 * Revision 2.2  93/05/11  11:56:54  rvb
 * 	Created
 * 	[93/05/05            rvb]
 * 
 *
 */
/*
 * Make a complete cross product of paths from a list of paths and subdirectories
 * iff the new path exists
 */ 

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>


char	*PATH[100];
struct stat statbuf;
char	buffer[100];
char	*name;
int	v_flag;

main(argc, argv)
register char **argv;
{
register int i, idx = 0;
register char **argvs;

	name = *argv;
	{register char *cp = name;
		while (*cp) if (*cp++ == '/') name = cp;
	}

	for ( argv++ ; --argc ; argv++ ) { register char *token = *argv;
		if (*token++ != '-' || !*token)
			break;
		else { register int flag;
			for ( ; flag = *token++ ; ) 
			switch (flag) {
			case 'v':
				v_flag = 1;
				break;
			default:
				goto usage;
			}
		}
	}
	for (; argc-- ; argv++ ) { register char *token = *argv;
		if (strcmp(token, "-p")) {
			PATH[idx++] = token;
		} else
			break;
	}

	argvs = ++argv;
	if (argc > 0) {
		for (i = 0; i < idx - 1; i++) {
			register char *bp;
			strcpy(buffer,PATH[i]);
			bp = &buffer[strlen(buffer)];
			if (bp[-1] != '/') {
				*bp++ = '/';
			}
		 	for (argv = argvs; *argv; argv++) {
				strcpy(bp, *argv);
				if (stat(buffer, &statbuf) == 0 &&
#ifndef	S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif
				    S_ISDIR(statbuf.st_mode))
					printf("%s ", buffer);
			}
		}
		printf("%s ", PATH[i]);
	} else {
		for (i = 0; i < idx; )
			printf("%s ", PATH[i++]);
	}

	exit(0);
usage:
	fprintf(stderr, "usage: shadow_path path1 .. pathn -p dir1 .. dirn\n");
	exit(1);
}
	
