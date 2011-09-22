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
 * $Log:	header.c,v $
 * Revision 2.9  93/05/10  17:49:12  rvb
 * 	Fix include to use < vs " for new ode shadowing
 * 	[93/05/10  10:27:51  rvb]
 * 
 * Revision 2.8  93/01/14  17:58:04  danner
 * 	Made text on #endif lines into comments.
 * 	[92/12/14            pds]
 * 	Converted file and generated stub code to ANSI C.
 * 	[92/12/08            pds]
 * 
 * Revision 2.7  91/08/28  11:16:58  jsb
 * 	Removed TrapRoutine support.
 * 	[91/08/12            rpd]
 * 
 * Revision 2.6  91/06/26  14:39:37  rpd
 * 	Removed the user initialization function and InitRoutineName.
 * 	Fixed to use different symbolic constants to protect
 * 	the user and server header files against multiple inclusion.
 * 	[91/06/26            rpd]
 * 
 * Revision 2.5  91/06/25  10:31:21  rpd
 * 	Restored prototype generation.
 * 	Changed WriteHeader to WriteUserHeader.
 * 	Added WriteServerHeader.
 * 	[91/05/23            rpd]
 * 
 * Revision 2.4  91/02/05  17:54:33  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:54:19  mrt]
 * 
 * Revision 2.3  90/12/20  17:04:47  jeffreyh
 * 	Commented out code for prototype generation. This is a temporary solution
 * 	to the longer term problem of the need 
 * 	for the generation of both a client and a server header file 
 * 	that have correct prototypes for strict ansi c and c++. The
 * 	prototypes generated before anly were for the client and broke kernel
 * 	files that included them if compiled under standard gcc
 * 	[90/12/07            jeffreyh]
 * 
 * Revision 2.2  90/06/02  15:04:46  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:11:06  rpd]
 * 
 * 07-Apr-89  Richard Draves (rpd) at Carnegie-Mellon University
 *	Extensive revamping.  Added polymorphic arguments.
 *	Allow multiple variable-sized inline arguments in messages.
 *
 *  8-Jul-88  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Conditionally defined mig_external to be extern and then defined
 *	all functions  with the storage class mig_external.
 *	Mig_external can be changed
 *	when the generated code is compiled.
 *
 * 18-Jan-88  David Detlefs (dld) at Carnegie-Mellon University
 *	Modified to produce C++ compatible code via #ifdefs.
 *	All changes have to do with argument declarations.
 *
 *  3-Aug-87  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Revision to make multi-threaded use work. Removed definitions for
 * 	alloc_reply_port and init_msg_type as these routines are 
 * 	no longer generated.
 *
 * 30-Jul-87  Mary Thompson (mrt) at Carnegie-Mellon University
 * 	Made changes to generate conditional code for C++ parameter lists
 *
 * 29-Jul-87  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Changed WriteRoutine to produce conditional argument
 *	lists for C++
 *
 *  8-Jun-87  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Changed the KERNEL include from ../h to sys/
 *	Removed extern from WriteHeader to make hi-c happy
 *
 * 28-May-87  Richard Draves (rpd) at Carnegie-Mellon University
 *	Created.
 */

#include <write.h>
#include <utils.h>
#include <global.h>
#include <error.h>

static void
WriteIncludes(FILE *file)
{
    fprintf(file, "#include <mach/kern_return.h>\n");
    fprintf(file, "#include <mach/port.h>\n");
    fprintf(file, "#include <mach/message.h>\n");
    fprintf(file, "\n");
}

static void
WriteDefines(FILE *file)
{
}

static void
WriteMigExternal(FILE *file)
{
    fprintf(file, "#ifdef\tmig_external\n");
    fprintf(file, "mig_external\n");
    fprintf(file, "#else\n");
    fprintf(file, "extern\n");
    fprintf(file, "#endif\n");
}

static void
WriteProlog(FILE *file, const char *protect)
{
    if (protect != strNULL) {
	fprintf(file, "#ifndef\t_%s\n", protect);
	fprintf(file, "#define\t_%s\n", protect);
	fprintf(file, "\n");
    }

    fprintf(file, "/* Module %s */\n", SubsystemName);
    fprintf(file, "\n");

    WriteIncludes(file);
    WriteDefines(file);
}

static void
WriteEpilog(FILE *file, const char *protect)
{
    if (protect != strNULL) {
	fprintf(file, "\n");
	fprintf(file, "#endif\t/* not defined(_%s) */\n", protect);
    }
}

static void
WriteUserRoutine(FILE *file, const routine_t *rt)
{
    fprintf(file, "\n");
    fprintf(file, "/* %s %s */\n", rtRoutineKindToStr(rt->rtKind), rt->rtName);
    WriteMigExternal(file);
    fprintf(file, "%s %s\n", ReturnTypeStr(rt), rt->rtUserName);
    fprintf(file, "#if\t%s\n", LintLib);
    fprintf(file, "    (");
    WriteList(file, rt->rtArgs, WriteNameDecl, akbUserArg, ", " , "");
    fprintf(file, ")\n");
    WriteList(file, rt->rtArgs, WriteUserVarDecl, akbUserArg, ";\n", ";\n");
    fprintf(file, "{ ");
    if (!rt->rtProcedure)
	fprintf(file, "return ");
    fprintf(file, "%s(", rt->rtUserName);
    WriteList(file, rt->rtArgs, WriteNameDecl, akbUserArg, ", ", "");
    fprintf(file, "); }\n");
    fprintf(file, "#else\n");
    fprintf(file, "(\n");
    WriteList(file, rt->rtArgs, WriteUserVarDecl, akbUserArg, ",\n", "\n");
    fprintf(file, ");\n");
    fprintf(file, "#endif\n");
}

void
WriteUserHeader(FILE *file, const statement_t *stats)
{
    register const statement_t *stat;
    const char *protect = strconcat(SubsystemName, "_user_");

    WriteProlog(file, protect);
    for (stat = stats; stat != stNULL; stat = stat->stNext)
	switch (stat->stKind)
	{
	  case skRoutine:
	    WriteUserRoutine(file, stat->stRoutine);
	    break;
	  case skImport:
	  case skUImport:
	    WriteImport(file, stat->stFileName);
	    break;
	  case skSImport:
	    break;
	  default:
	    fatal("WriteHeader(): bad statement_kind_t (%d)",
		  (int) stat->stKind);
	}
    WriteEpilog(file, protect);
}

static void
WriteServerRoutine(FILE *file, const routine_t *rt)
{
    fprintf(file, "\n");
    fprintf(file, "/* %s %s */\n", rtRoutineKindToStr(rt->rtKind), rt->rtName);
    WriteMigExternal(file);
    fprintf(file, "%s %s\n", ReturnTypeStr(rt), rt->rtServerName);
    fprintf(file, "#if\t%s\n", LintLib);
    fprintf(file, "    (");
    WriteList(file, rt->rtArgs, WriteNameDecl, akbServerArg, ", " , "");
    fprintf(file, ")\n");
    WriteList(file, rt->rtArgs, WriteServerVarDecl,
	      akbServerArg, ";\n", ";\n");
    fprintf(file, "{ ");
    if (!rt->rtProcedure)
	fprintf(file, "return ");
    fprintf(file, "%s(", rt->rtServerName);
    WriteList(file, rt->rtArgs, WriteNameDecl, akbServerArg, ", ", "");
    fprintf(file, "); }\n");
    fprintf(file, "#else\n");
    fprintf(file, "(\n");
    WriteList(file, rt->rtArgs, WriteServerVarDecl,
	      akbServerArg, ",\n", "\n");
    fprintf(file, ");\n");
    fprintf(file, "#endif\n");
}

void
WriteServerHeader(FILE *file, const statement_t *stats)
{
    register const statement_t *stat;
    const char *protect = strconcat(SubsystemName, "_server_");

    WriteProlog(file, protect);
    for (stat = stats; stat != stNULL; stat = stat->stNext)
	switch (stat->stKind)
	{
	  case skRoutine:
	    WriteServerRoutine(file, stat->stRoutine);
	    break;
	  case skImport:
	  case skSImport:
	    WriteImport(file, stat->stFileName);
	    break;
	  case skUImport:
	    break;
	  default:
	    fatal("WriteServerHeader(): bad statement_kind_t (%d)",
		  (int) stat->stKind);
	}
    WriteEpilog(file, protect);
}

static void
WriteInternalRedefine(FILE *file, register const routine_t *rt)
{
    fprintf(file, "#define %s %s_external\n", rt->rtUserName, rt->rtUserName);
}

void
WriteInternalHeader(FILE *file, const statement_t *stats)
{
    register const statement_t *stat;

    for (stat = stats; stat != stNULL; stat = stat->stNext)
	switch (stat->stKind)
	{
	  case skRoutine:
	    WriteInternalRedefine(file, stat->stRoutine);
	    break;
	  case skImport:
	  case skUImport:
	  case skSImport:
	    break;
	  default:
	    fatal("WriteInternalHeader(): bad statement_kind_t (%d)",
		  (int) stat->stKind);
	}
}
