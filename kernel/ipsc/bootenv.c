/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * Copyright 1988, 1989, 1990, 1991 by Intel Corporation,
 * Santa Clara, California.
 * 
 *                          All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and that
 * both the copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Intel not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 * 
 * INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
 * SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN ACTION OF CONTRACT, NEGLIGENCE, OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */
/*
 * HISTORY
 * $Log:	bootenv.c,v $
 * Revision 2.2  91/12/10  16:32:26  jsb
 * 	New files from Intel
 * 	[91/12/10  16:12:46  jsb]
 * 
 */

/*
 * andyp
 */

#define MAXENVSIZE	1024		/* I checked w/ bootcube sources */

char	**bootenv = 0;

/*
 * Save the Environment!
 *
 *	Apparently, the pointer passed to us points to a single
 *	string with embedded newlines. eg:
 *
 *		char *bnv = "BOOT_MY_NODE=002\nBOOT_INIT_NODE=3\n...";
 *
 *	We need to make it look like an "ordinary" environment.
 */
char **envcopy(bnv)
char *bnv;
{
	static char saveenv[MAXENVSIZE];
	char	**env, *src, *dst, c;
	int	i, len, nl, pspc;

	if (!bnv) {
		env = (char **) saveenv;
		env[0] = 0;
		bootenv = (char **) env;
		return bootenv;
	}

	/*
	 * find the length and the number of newlines
	 */
	len = nl = 0;
	src = bnv;
	while ((c = *src++)) {
		len++;
		if (c == '\n')
			nl++;
	}

	/*
	 * sanity check
	 */
	pspc = (nl + 1) * sizeof(char *);
	if ((len + pspc) > MAXENVSIZE)
		panic("envcopy: bootenv is too big");


	/*
	 * copy the strings, substituting 0 for '\n'.
	 */
	env = (char **) saveenv;
	src = bnv;
	dst = &saveenv[pspc];
	for (i = 0; i < nl; i++) {
		*env++ = dst;
		while ((c = *src++) != '\n')
			*dst++ = c;
		*dst++ = 0;
	}
	*env = 0;

	bootenv = (char **) saveenv;
	return bootenv;
}


/*
 * funny string compare
 */
static char *upto(a, b, t)
char *a, *b, t;
{
	char	c1, c2;

	for (;;) {
		c1 = *a++;
		c2 = *b++;
		if ((c1 == t) && (c2 == 0))
			return a;
		if (c1 != c2)
			return (char *) 0;
		if ((c1 == 0) || (c2 == 0))
			return (char *) 0;
	}
	/*NOTREACHED*/
}


/*
 * similar to getenv()
 */
char *getbootenv(var)
char *var;
{
	char	*val, **env;

	for (env = bootenv; *env; ++env) {
		if ((val = upto(*env, var, '=')))
			return val;
	}

	return (char *) 0;
}


#define	PASS_BOOT_MAGIC	0

/*
 * Collapse the boot environment into one big string.
 */
char *ipsc_boot_environ()
{
	static char saveenv[MAXENVSIZE];
	char	*s, **ep = bootenv;
	int	len;

#if	PASS_BOOT_MAGIC
	s = saveenv;
	s[0] = 0;
	while (*ep) {
		len = strlen(*ep);
		strcpy(s, *ep);
		s[len] = '\n';
		s += (len + 1);
		s[0] = 0;
		ep++;
	}

	return saveenv;
#else	/* PASS_BOOT_MAGIC */
	return (char *) 0;
#endif	/* PASS_BOOT_MAGIC */
}
