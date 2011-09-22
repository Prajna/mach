/* 
 * Mach Operating System
 * Copyright (c) 1993,1992,1991,1990 Carnegie Mellon University
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
 * $Log:	server.c,v $
 * Revision 2.11  93/11/17  18:48:59  dbg
 * 	Lots of changes to accomodate screwy GCC compiler on 64bit
 * 	machines, who makes improper assumptions about alignments.
 * 	The basic idea is to typecast type descriptors as needed.
 * 	The argSuffix field helps building the proper reference.
 * 	Added WriteMsgFieldRef().  Changed WriteArgSize to take
 * 	alignment specifier, and use it.
 * 	Added WriteArgSizeStright().
 * 	WriteAdjustMsgSize now needs to look-ahead for alignment requirements.
 * 	See comments within for 64bit-specific twist.
 * 	[93/09/14  12:28:33  af]
 * 
 * Revision 2.10  93/05/10  17:49:43  rvb
 * 	Fix include to use < vs " for new ode shadowing
 * 	[93/05/10  10:28:50  rvb]
 * 
 * Revision 2.9  93/01/14  17:58:44  danner
 * 	Removed #include of string.h from WriteIncludes.
 * 	[92/12/14            pds]
 * 	Converted file and generated stub code to ANSI C.
 * 	Changed code to always use static message types.
 * 	[92/12/08            pds]
 * 
 * 	Added parentheses to expression used to round the size of in-line
 * 	arrays in order to make GCC happy.
 * 	Fixed bug checking for the Hi-C compiler: hc -> __HC__.
 * 	Made text on #endif/#else lines into comments.
 * 	Added declartions for mig_deallocate and mig_strncpy.
 * 	[92/08/06            pds]
 * 
 * 	Added #include of string.h to WriteIncludes.
 * 	[92/06/13            pds]
 * 
 * Revision 2.8  92/01/14  16:46:39  rpd
 * 	Modified WriteInitializeCount, WriteExtractArg
 * 	for the revised CountInOut implementation.
 * 	Fixed Indefinite code generation, to allow short type descriptors.
 * 	Added deallocate bit handling to Indefinite code generation.
 * 	[92/01/08            rpd]
 * 
 * Revision 2.7  92/01/03  20:30:09  dbg
 * 	Generate <subsystem>_server_routine to return unpacking function
 * 	pointer.
 * 	[91/11/11            dbg]
 * 
 * 	For inline variable-length arrays that are Out parameters, allow
 * 	passing the user's count argument to the server as an InOut
 * 	parameter.
 * 	[91/11/11            dbg]
 * 
 * 	Redo handling of OUT arrays that are passed in-line or
 * 	out-of-line.  Treat more like out-of-line arrays:
 * 	user allocates buffer and pointer
 * 	fills in pointer with buffer address
 * 	passes pointer to stub
 * 	stub copies data to *pointer, or changes pointer
 * 	User can always use *pointer.
 * 
 * 	Change argByReferenceUser to a field in argument_t.
 * 	[91/09/04            dbg]
 * 
 * Revision 2.6  91/08/28  11:17:21  jsb
 * 	Replaced ServerProcName with ServerDemux.
 * 	[91/08/13            rpd]
 * 
 * 	Removed Camelot and TrapRoutine support.
 * 	Changed MsgKind to MsgSeqno.
 * 	[91/08/12            rpd]
 * 
 * Revision 2.5  91/07/31  18:10:51  dbg
 * 	Allow indefinite-length variable arrays.  They may be copied
 * 	either in-line or out-of-line, depending on size.
 * 
 * 	Copy variable-length C Strings with mig_strncpy, to combine
 * 	'strcpy' and 'strlen' operations.
 * 
 * 	New method for advancing request message pointer past
 * 	variable-length arguments.  We no longer have to know the order
 * 	of variable-length arguments and their count arguments.
 * 
 * 	Remove redundant assignments (to msgh_simple, msgh_size) in
 * 	generated code.
 * 	[91/07/17            dbg]
 * 
 * Revision 2.4  91/06/25  10:31:51  rpd
 * 	Cast request and reply ports to ipc_port_t in KernelServer stubs.
 * 	[91/05/27            rpd]
 * 
 * Revision 2.3  91/02/05  17:55:37  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:55:30  mrt]
 * 
 * Revision 2.2  90/06/02  15:05:29  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:13:12  rpd]
 * 
 * 07-Apr-89  Richard Draves (rpd) at Carnegie-Mellon University
 *	Extensive revamping.  Added polymorphic arguments.
 *	Allow multiple variable-sized inline arguments in messages.
 *
 * 18-Oct-88  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Set the local port in the server reply message to
 *	MACH_PORT_NULL for greater efficiency and to make Camelot
 *	happy.
 *
 * 18-Apr-88  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Changed call to WriteLocalVarDecl in WriteMsgVarDecl
 *	to write out the parameters for the C++ code to a call
 *	a new routine WriteServerVarDecl which includes the *
 *	for reference variable, but uses the transType if it
 *	exists.
 *
 * 27-Feb-88  Richard Draves (rpd) at Carnegie-Mellon University
 *	Changed reply message initialization for camelot interfaces.
 *	Now we assume camelot interfaces are all camelotroutines and
 *	always initialize the dummy field & tid field.  This fixes
 *	the wrapper-server-call bug in distributed transactions.
 *
 * 23-Feb-88  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Changed the include of camelot_types.h to cam/camelot_types.h
 *
 * 19-Feb-88  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Fixed WriteDestroyArg to not call the destructor
 *	function on any in/out args.
 *
 *  4-Feb-88  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Fixed dld's code to write out parameter list to
 *	use WriteLocalVarDecl to get transType or ServType if
 *	they exist.
 *
 * 19-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Change variable-length inline array declarations to use
 *	maximum size specified to Mig.  Make message variable
 *	length if the last item in the message is variable-length
 *	and inline.  Use argMultipler field to convert between
 *	argument and IPC element counts.
 *
 * 18-Jan-88  David Detlefs (dld) at Carnegie-Mellon University
 *	Modified to produce C++ compatible code via #ifdefs.
 *	All changes have to do with argument declarations.
 *
 *  2-Dec-87  David Golub (dbg) at Carnegie-Mellon University
 *	Added destructor function for IN arguments to server.
 *
 * 18-Nov-87  Jeffrey Eppinger (jle) at Carnegie-Mellon University
 *	Changed to typedef "novalue" as "void" if we're using hc.
 *
 * 17-Sep-87  Bennet Yee (bsy) at Carnegie-Mellon University
 *	Added _<system>SymTab{Base|End} for use with security
 *	dispatch routine.  It is neccessary for the authorization
 *	system to know the operations by symbolic names.
 *	It is harmless to user code as it only means an extra
 *	array if it is accidentally turned on.
 *
 * 24-Aug-87  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Corrected the setting of retcode for CamelotRoutines.
 *
 * 21-Aug-87  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Added deallocflag to call to WritePackArgType.
 *
 * 14-Aug-87  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Moved type declarations and assignments for DummyType 
 *	and tidType to server demux routine. Automatically
 *	include camelot_types.h and msg_types.h for interfaces 
 *	containing camelotRoutines.
 *
 *  8-Jun-87  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Removed #include of sys/types.h and strings.h from WriteIncludes.
 *	Changed the KERNEL include from ../h to sys/
 *	Removed extern from WriteServer to make hi-c happy
 *
 * 28-May-87  Richard Draves (rpd) at Carnegie-Mellon University
 *	Created.
 */

#include <assert.h>

#include <mach/message.h>

#include <write.h>
#include <utils.h>
#include <global.h>
#include <error.h>
#include <cross64.h>

static void
WriteIncludes(FILE *file)
{
    fprintf(file, "#define EXPORT_BOOLEAN\n");
    fprintf(file, "#include <mach/boolean.h>\n");
    fprintf(file, "#include <mach/kern_return.h>\n");
    fprintf(file, "#include <mach/message.h>\n");
    fprintf(file, "#include <mach/mig_errors.h>\n");
    fprintf(file, "#include <mach/mig_support.h>\n");
    if (IsKernelServer)
	fprintf(file, "#include <ipc/ipc_port.h>\n");
    fprintf(file, "\n");
}

static void
WriteGlobalDecls(FILE *file)
{
    if (RCSId != strNULL)
	WriteRCSDecl(file, strconcat(SubsystemName, "_server"), RCSId);

    /* Used for locations in the request message, *not* reply message.
       Reply message locations aren't dependent on IsKernelServer. */

    if (IsKernelServer)
    {
	fprintf(file, "#define msgh_request_port\tmsgh_remote_port\n");
	fprintf(file, "#define MACH_MSGH_BITS_REQUEST(bits)");
	fprintf(file, "\tMACH_MSGH_BITS_REMOTE(bits)\n");
	fprintf(file, "#define msgh_reply_port\t\tmsgh_local_port\n");
	fprintf(file, "#define MACH_MSGH_BITS_REPLY(bits)");
	fprintf(file, "\tMACH_MSGH_BITS_LOCAL(bits)\n");
    }
    else
    {
	fprintf(file, "#define msgh_request_port\tmsgh_local_port\n");
	fprintf(file, "#define MACH_MSGH_BITS_REQUEST(bits)");
	fprintf(file, "\tMACH_MSGH_BITS_LOCAL(bits)\n");
	fprintf(file, "#define msgh_reply_port\t\tmsgh_remote_port\n");
	fprintf(file, "#define MACH_MSGH_BITS_REPLY(bits)");
	fprintf(file, "\tMACH_MSGH_BITS_REMOTE(bits)\n");
    }
    fprintf(file, "\n");
}

static void
WriteProlog(FILE *file)
{
    fprintf(file, "/* Module %s */\n", SubsystemName);
    fprintf(file, "\n");
    
    WriteIncludes(file);
    WriteBogusDefines(file);
    WriteGlobalDecls(file);
}


static void
WriteSymTabEntries(FILE *file, const statement_t *stats)
{
    register const statement_t *stat;
    register u_int current = 0;

    for (stat = stats; stat != stNULL; stat = stat->stNext)
	if (stat->stKind == skRoutine) {
	    register	num = stat->stRoutine->rtNumber;
	    const char	*name = stat->stRoutine->rtName;

	    while (++current <= num)
		fprintf(file,"\t\t\t{ \"\", 0, 0 },\n");
	    fprintf(file, "\t{ \"%s\", %d, _X%s },\n",
	    	name,
		SubsystemBase + current - 1,
		name);
	}
    while (++current <= rtNumber)
	fprintf(file,"\t{ \"\", 0, 0 },\n");
}

static void
WriteArrayEntries(FILE *file, const statement_t *stats)
{
    register u_int current = 0;
    register const statement_t *stat;

    for (stat = stats; stat != stNULL; stat = stat->stNext)
	if (stat->stKind == skRoutine)
	{
	    register const routine_t *rt = stat->stRoutine;

	    while (current++ < rt->rtNumber)
		fprintf(file, "\t\t0,\n");
	    fprintf(file, "\t\t_X%s,\n", rt->rtName);
	}
    while (current++ < rtNumber)
	fprintf(file, "\t\t\t0,\n");
}

static void
WriteEpilog(FILE *file, const statement_t *stats)
{
    fprintf(file, "\n");

    /*
     * First, the symbol table
     */
     fprintf(file, "static mig_routine_t %s_routines[] = {\n", ServerDemux);

     WriteArrayEntries(file, stats);

     fprintf(file, "};\n");
     fprintf(file, "\n");

     /*
      * Then, the server routine
      */
    fprintf(file, "mig_external boolean_t %s\n", ServerDemux);
    fprintf(file, "\t(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)\n");

    fprintf(file, "{\n");
    fprintf(file, "\tregister mach_msg_header_t *InP =  InHeadP;\n");

    fprintf(file, "\tregister mig_reply_header_t *OutP = (mig_reply_header_t *) OutHeadP;\n");

    fprintf(file, "\n");

    WriteStaticDecl(file, itRetCodeType,
		    itRetCodeType->itDeallocate, itRetCodeType->itLongForm,
		    !IsKernelServer, "RetCodeType");
    fprintf(file, "\n");

    fprintf(file, "\tregister mig_routine_t routine;\n");
    fprintf(file, "\n");

    fprintf(file, "\tOutP->Head.msgh_bits = ");
    fprintf(file, "MACH_MSGH_BITS(MACH_MSGH_BITS_REPLY(InP->msgh_bits), 0);\n");
    fprintf(file, "\tOutP->Head.msgh_size = sizeof *OutP;\n");
    fprintf(file, "\tOutP->Head.msgh_remote_port = InP->msgh_reply_port;\n");
    fprintf(file, "\tOutP->Head.msgh_local_port = MACH_PORT_NULL;\n");
    fprintf(file, "\tOutP->Head.msgh_seqno = 0;\n");
    fprintf(file, "\tOutP->Head.msgh_id = InP->msgh_id + 100;\n");
    fprintf(file, "\n");
    WritePackMsgType(file, itRetCodeType,
		     itRetCodeType->itDeallocate, itRetCodeType->itLongForm,
		     !IsKernelServer, "OutP->RetCodeType", "RetCodeType");
    fprintf(file, "\n");

    fprintf(file, "\tif ((InP->msgh_id > %d) || (InP->msgh_id < %d) ||\n",
	    SubsystemBase + rtNumber - 1, SubsystemBase);
    fprintf(file, "\t    ((routine = %s_routines[InP->msgh_id - %d]) == 0)) {\n",
	    ServerDemux, SubsystemBase);
    fprintf(file, "\t\tOutP->RetCode = MIG_BAD_ID;\n");
    fprintf(file, "\t\treturn FALSE;\n");
    fprintf(file, "\t}\n");

    /* Call appropriate routine */
    fprintf(file, "\t(*routine) (InP, &OutP->Head);\n");
    fprintf(file, "\treturn TRUE;\n");
    fprintf(file, "}\n");
    fprintf(file, "\n");

    /*
     * Then, the <subsystem>_server_routine routine
     */
    fprintf(file, "mig_external mig_routine_t %s_routine\n", ServerDemux);
    fprintf(file, "\t(const mach_msg_header_t *InHeadP)\n");

    fprintf(file, "{\n");
    fprintf(file, "\tregister int msgh_id;\n");
    fprintf(file, "\n");
    fprintf(file, "\tmsgh_id = InHeadP->msgh_id - %d;\n", SubsystemBase);
    fprintf(file, "\n");
    fprintf(file, "\tif ((msgh_id > %d) || (msgh_id < 0))\n",
	    rtNumber - 1);
    fprintf(file, "\t\treturn 0;\n");
    fprintf(file, "\n");
    fprintf(file, "\treturn %s_routines[msgh_id];\n", ServerDemux);
    fprintf(file, "}\n");
    fprintf(file, "\n");

    /* symtab */

    if (GenSymTab) {
	fprintf(file,"\nmig_symtab_t _%sSymTab[] = {\n",SubsystemName);
	WriteSymTabEntries(file,stats);
	fprintf(file,"};\n");
	fprintf(file,"int _%sSymTabBase = %d;\n",SubsystemName,SubsystemBase);
	fprintf(file,"int _%sSymTabEnd = %d;\n",SubsystemName,SubsystemBase+rtNumber);
    }
}

/*
 *  Returns the return type of the server-side work function.
 *  Suitable for "extern %s serverfunc()".
 */
static const char *
ServerSideType(const routine_t *rt)
{
    if (rt->rtServerReturn == argNULL)
	return "void";
    else
	return rt->rtServerReturn->argType->itTransType;
}

static void
WriteLocalVarDecl(FILE *file, register const argument_t *arg)
{
    register const ipc_type_t *it = arg->argType;

    if (it->itInLine && it->itVarArray)
    {
	register const ipc_type_t *btype = it->itElement;

	fprintf(file, "\t%s %s[%d]", btype->itTransType,
		arg->argVarName, it->itNumber/btype->itNumber);
    }
    else
	fprintf(file, "\t%s %s", it->itTransType, arg->argVarName);
}

static void
WriteLocalPtrDecl(FILE *file, register const argument_t *arg)
{
    fprintf(file, "\t%s *%sP",
		FetchServerType(arg->argType->itElement),
		arg->argVarName);
}

static void
WriteServerArgDecl(FILE *file, const argument_t *arg)
{
    fprintf(file, "%s %s%s",
	    arg->argType->itTransType,
	    arg->argByReferenceServer ? "*" : "",
	    arg->argVarName);
}

/*
 *  Writes the local variable declarations which are always
 *  present:  InP, OutP, the server-side work function.
 */
static void
WriteVarDecls(FILE *file, const routine_t *rt)
{
    int i;
    boolean_t NeedMsghSize = FALSE;
    boolean_t NeedMsghSizeDelta = FALSE;

    fprintf(file, "\tregister Request *In0P = (Request *) InHeadP;\n");
    for (i = 1; i <= rt->rtMaxRequestPos; i++)
	fprintf(file, "\tregister Request *In%dP;\n", i);
    fprintf(file, "\tregister Reply *OutP = (Reply *) OutHeadP;\n");

    fprintf(file, "\tmig_external %s %s\n",
	    ServerSideType(rt), rt->rtServerName);
    fprintf(file, "\t\t(");
    WriteList(file, rt->rtArgs, WriteServerArgDecl, akbServerArg, ", ", "");
    fprintf(file, ");\n");
    fprintf(file, "\n");

    if (!rt->rtSimpleFixedReply)
	fprintf(file, "\tboolean_t msgh_simple;\n");
    else if (!rt->rtSimpleCheckRequest)
    {
	fprintf(file, "#if\tTypeCheck\n");
	fprintf(file, "\tboolean_t msgh_simple;\n");
	fprintf(file, "#endif\t/* TypeCheck */\n");
	fprintf(file, "\n");
    }

    /* if either request or reply is variable, we may need
       msgh_size_delta and msgh_size */

    if (rt->rtNumRequestVar > 0)
	NeedMsghSize = TRUE;
    if (rt->rtMaxRequestPos > 0)
	NeedMsghSizeDelta = TRUE;

    if (rt->rtNumReplyVar > 1)
	NeedMsghSize = TRUE;
    if (rt->rtMaxReplyPos > 0)
	NeedMsghSizeDelta = TRUE;

    if (NeedMsghSize)
	fprintf(file, "\tunsigned int msgh_size;\n");
    if (NeedMsghSizeDelta)
	fprintf(file, "\tunsigned int msgh_size_delta;\n");

    if (NeedMsghSize || NeedMsghSizeDelta)
	fprintf(file, "\n");
}

static void
WriteMsgError(FILE *file, const char *error_msg)
{
    fprintf(file, "\t\t{ OutP->RetCode = %s; return; }\n", error_msg);
}

static void
WriteReplyInit(FILE *file, const routine_t *rt)
{
    boolean_t printed_nl = FALSE;

    if (rt->rtSimpleFixedReply)
    {
	if (!rt->rtSimpleSendReply) /* complex reply message */
	{
	    printed_nl = TRUE;
	    fprintf(file, "\n");
	    fprintf(file,
		"\tOutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;\n");
	}
    }
    else
    {
	printed_nl = TRUE;
	fprintf(file, "\n");
	fprintf(file, "\tmsgh_simple = %s;\n", 
			  strbool(rt->rtSimpleSendReply));
    }

    if (rt->rtNumReplyVar == 0)
    {
	if (!printed_nl)
	    fprintf(file, "\n");
	fprintf(file, "\tOutP->Head.msgh_size = %d;\n", rt->rtReplySize);
    }
}

static void
WriteReplyHead(FILE *file, const routine_t *rt)
{
    if ((!rt->rtSimpleFixedReply) ||
	(rt->rtNumReplyVar > 1))
    {
	fprintf(file, "\n");
	if (rt->rtMaxReplyPos > 0)
	    fprintf(file, "\tOutP = (Reply *) OutHeadP;\n");
    }

    if (!rt->rtSimpleFixedReply)
    {
	fprintf(file, "\tif (!msgh_simple)\n");
	fprintf(file,
		"\t\tOutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;\n");
    }
    if (rt->rtNumReplyVar > 1)
	fprintf(file, "\tOutP->Head.msgh_size = msgh_size;\n");
}

static void
WriteCheckHead(FILE *file, const routine_t *rt)
{
    fprintf(file, "#if\tTypeCheck\n");
    if (rt->rtNumRequestVar > 0)
	fprintf(file, "\tmsgh_size = In0P->Head.msgh_size;\n");
    if (!rt->rtSimpleCheckRequest)
	fprintf(file, "\tmsgh_simple = !(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);\n");

    if (rt->rtNumRequestVar > 0)
	fprintf(file, "\tif ((msgh_size < %d)",
		rt->rtRequestSize);
    else
	fprintf(file, "\tif ((In0P->Head.msgh_size != %d)",
		rt->rtRequestSize);

    if (rt->rtSimpleCheckRequest)
	fprintf(file, " ||\n\t    %s(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX)",
		rt->rtSimpleReceiveRequest ? "" : "!");
    fprintf(file, ")\n");
    WriteMsgError(file, "MIG_BAD_ARGUMENTS");
    fprintf(file, "#endif\t/* TypeCheck */\n");
    fprintf(file, "\n");
}

static void
WriteTypeCheck(FILE *file, register const argument_t *arg)
{
    register const ipc_type_t *it = arg->argType;
    register const routine_t *rt = arg->argRoutine;

    fprintf(file, "#if\tTypeCheck\n");
    if (akCheck(arg->argKind, akbRequestQC))
	fprintf(file, "\tif (* (int *) &In%dP->%s != * (int *) &%sCheck)\n",
		arg->argRequestPos, arg->argTTName, arg->argVarName);
    else
    {
	fprintf(file, "\tif (");
	if (!it->itIndefinite) {
	    fprintf(file, "(((mach_msg_type_t*)&In%dP->%s%s)->msgt_inline != %s) ||\n\t    ",
		arg->argRequestPos, arg->argTTName,
		arg->argLongForm ? ".msgtl_header" : "",
		strbool(it->itInLine));
	}
	fprintf(file, "(((mach_msg_type_t*)&In%dP->%s%s)->msgt_longform != %s) ||\n",
		arg->argRequestPos, arg->argTTName,
		arg->argLongForm ? ".msgtl_header" : "",
		strbool(arg->argLongForm));
	if (it->itOutName == MACH_MSG_TYPE_POLYMORPHIC)
	{
	    if (!rt->rtSimpleCheckRequest)
		fprintf(file, "\t    (MACH_MSG_TYPE_PORT_ANY(%sIn%dP->%s%smsgt%s_name) && msgh_simple) ||\n",
			arg->argLongForm ? "" : "((mach_msg_type_t*)&",
			arg->argRequestPos, arg->argTTName,
			arg->argLongForm ? "." : ")->",
			arg->argLongForm ? "l" : "");
	}
	else
	    fprintf(file, "\t    (%sIn%dP->%s%smsgt%s_name != %s) ||\n",
		    arg->argLongForm ? "" : "((mach_msg_type_t*)&",
		    arg->argRequestPos, arg->argTTName,
		    arg->argLongForm ? "." : ")->",
		    arg->argLongForm ? "l" : "",
		    it->itOutNameStr);
	if (!it->itVarArray)
	    fprintf(file, "\t    (%sIn%dP->%s%smsgt%s_number != %d) ||\n",
		    arg->argLongForm ? "" : "((mach_msg_type_t*)&",
		    arg->argRequestPos, arg->argTTName,
		    arg->argLongForm ? "." : ")->",
		    arg->argLongForm ? "l" : "",
		    it->itNumber);
	fprintf(file, "\t    (%sIn%dP->%s%smsgt%s_size != %d))\n",
		arg->argLongForm ? "" : "((mach_msg_type_t*)&",
		arg->argRequestPos, arg->argTTName,
		arg->argLongForm ? "." : ")->",
		arg->argLongForm ? "l" : "",
		it->itSize);
    }
    WriteMsgError(file, "MIG_BAD_ARGUMENTS");
    fprintf(file, "#endif\t/* TypeCheck */\n");
    fprintf(file, "\n");
}

static void
WriteMsgFieldRef(
    FILE		*file,
    register const argument_t *arg,
    const_string_t	fmt,
    const_string_t	fldname)
{
    if (arg->argSuffix && !arg->argParent->argLongForm)
	fprintf(file, "((mach_msg_type_t*)&%s%s)->%s",
		fmt, fldname, arg->argSuffix);
    else
	fprintf(file, "%s%s", fmt, arg->argMsgField);
}

static void
WriteCheckArgSize(FILE *file, register const argument_t *arg)
{
    register const ipc_type_t *ptype = arg->argType;
    register const ipc_type_t *btype = ptype->itElement;
    const argument_t *count = arg->argCount;
    int multiplier = btype->itTypeSize / btype->itNumber;

    if (ptype->itIndefinite) {
	/*
	 * Check descriptor.  If out-of-line, use standard size.
	 */
	fprintf(file, "(((mach_msg_type_t*)&In%dP->%s%s)->msgt_inline) ? ",
		arg->argRequestPos,
		arg->argTTName,
		arg->argLongForm ? ".msgtl_header" : "");
    }

    if (btype->itTypeSize % 4 != 0)
	fprintf(file, "(");

    if (multiplier > 1)
	fprintf(file, "%d * ", multiplier);

    if (count->argSuffix && !arg->argLongForm)
	fprintf(file, "(((mach_msg_type_t*)&(In%dP->%s))->%s)",
		arg->argRequestPos, /*yech!*/arg->argTTName, count->argSuffix);
    else
	fprintf(file, "In%dP->%s", arg->argRequestPos, count->argMsgField);


    /* If the base type size of the data field isn`t a multiple of 4,
       we have to round up. */
    if (btype->itTypeSize % 4 != 0)
	fprintf(file, " + 3) & ~3");

    if (ptype->itIndefinite) {
	fprintf(file, " : sizeof(%s *)", FetchServerType(btype));
    }
}

static void
WriteCheckMsgSize(FILE *file, register const argument_t *arg)
{
    register const routine_t *rt = arg->argRoutine;

    /* If there aren't any more In args after this, then
       we can use the msgh_size_delta value directly in
       the TypeCheck conditional. */

    if (arg->argRequestPos == rt->rtMaxRequestPos)
    {
	fprintf(file, "#if\tTypeCheck\n");
	fprintf(file, "\tif (msgh_size != %d + (", rt->rtRequestSize);
	WriteCheckArgSize(file, arg);
	fprintf(file, "))\n");

	WriteMsgError(file, "MIG_BAD_ARGUMENTS");
	fprintf(file, "#endif\t/* TypeCheck */\n");
    }
    else
    {
	/* If there aren't any more variable-sized arguments after this,
	   then we must check for exact msg-size and we don't need to
	   update msgh_size. */

	boolean_t LastVarArg = arg->argRequestPos+1 == rt->rtNumRequestVar;

	/* calculate the actual size in bytes of the data field.  note
	   that this quantity must be a multiple of four.  hence, if
	   the base type size isn't a multiple of four, we have to
	   round up.  note also that btype->itNumber must
	   divide btype->itTypeSize (see itCalculateSizeInfo). */

	fprintf(file, "\tmsgh_size_delta = ");
	WriteCheckArgSize(file, arg);
	fprintf(file, ";\n");
	fprintf(file, "#if\tTypeCheck\n");

	/* Don't decrement msgh_size until we've checked that
	   it won't underflow. */

	if (LastVarArg)
	    fprintf(file, "\tif (msgh_size != %d + msgh_size_delta)\n",
		rt->rtRequestSize);
	else
	    fprintf(file, "\tif (msgh_size < %d + msgh_size_delta)\n",
		rt->rtRequestSize);
	WriteMsgError(file, "MIG_BAD_ARGUMENTS");

	if (!LastVarArg)
	    fprintf(file, "\tmsgh_size -= msgh_size_delta;\n");

	fprintf(file, "#endif\t/* TypeCheck */\n");
    }
    fprintf(file, "\n");
}

static const char *
InArgMsgField(register const argument_t *arg)
{
    static char buffer[100];

    /*
     *	Inside the kernel, the request and reply port fields
     *	really hold ipc_port_t values, not mach_port_t values.
     *	Hence we must cast the values.
     */

    if (IsKernelServer &&
	((akIdent(arg->argKind) == akeRequestPort) ||
	 (akIdent(arg->argKind) == akeReplyPort)))
	sprintf(buffer, "(ipc_port_t) In%dP->%s",
		arg->argRequestPos, arg->argMsgField);
    else if (arg->argSuffix && !arg->argParent->argLongForm)
	sprintf(buffer, "((mach_msg_type_t*)&(In%dP->%s))->%s",
		arg->argRequestPos, arg->argParent->argTTName, arg->argSuffix);
    else
	sprintf(buffer, "In%dP->%s",
		arg->argRequestPos, arg->argMsgField);

    return buffer;
}

static void
WriteExtractArgValue(FILE *file, register const argument_t *arg)
{
    register const ipc_type_t *it = arg->argType;

    if (arg->argMultiplier > 1)
	WriteCopyType(file, it, "%s", "/* %s */ %s / %d",
		      arg->argVarName, InArgMsgField(arg), arg->argMultiplier);
    else if (it->itInTrans != strNULL)
	WriteCopyType(file, it, "%s", "/* %s */ %s(%s)",
		      arg->argVarName, it->itInTrans, InArgMsgField(arg));
    else
	WriteCopyType(file, it, "%s", "/* %s */ %s",
		      arg->argVarName, InArgMsgField(arg));
    fprintf(file, "\n");
}

static void
WriteInitializeCount(FILE *file, register const argument_t *arg)
{
    register const ipc_type_t *ptype = arg->argParent->argType;
    register const ipc_type_t *btype = ptype->itElement;

    /*
     *	Initialize 'count' argument for variable-length inline OUT parameter
     *	with maximum allowed number of elements.
     */

    fprintf(file, "\t%s = %d;\n", arg->argVarName,
	    ptype->itNumber/btype->itNumber);

    /*
     *	If the user passed in a count, then we use the minimum.
     *	We can't let the user completely override our maximum,
     *	or the user might convince the server to overwrite the buffer.
     */

    if (arg->argCInOut != argNULL) {
	const char *msgfield = InArgMsgField(arg->argCInOut);

	fprintf(file, "\tif (%s < %s)\n", msgfield, arg->argVarName);
	fprintf(file, "\t\t%s = %s;\n", arg->argVarName, msgfield);
    }

    fprintf(file, "\n");
}

static void
WriteInitializePtr(FILE *file, register const argument_t *arg)
{
    if (akCheck(arg->argKind, akbVarNeeded))
	fprintf(file, "\t%sP = %s;\n",
		arg->argVarName, arg->argVarName);
    else
	fprintf(file, "\t%sP = OutP->%s;\n",
		arg->argVarName, arg->argMsgField);
}

static void
WriteTypeCheckArg(FILE *file, register const argument_t *arg)
{
    if (akCheck(arg->argKind, akbRequest)) {
	WriteTypeCheck(file, arg);

	if (akCheck(arg->argKind, akbVariable))
	    WriteCheckMsgSize(file, arg);
    }
}

static void
WriteAdjustRequestMsgPtr(FILE *file, register const argument_t *arg)
{
    register const ipc_type_t *ptype = arg->argType;

    fprintf(file,
	"\tIn%dP = (Request *) ((char *) In%dP + msgh_size_delta - %d);\n\n",
	arg->argRequestPos+1, arg->argRequestPos,
	ptype->itTypeSize + ptype->itPadSize);
}

static void
WriteTypeCheckRequestArgs(FILE *file, register const routine_t *rt)
{
    register const argument_t *arg;
    register const argument_t *lastVarArg;

    lastVarArg = argNULL;
    for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {

	/*
	 * Advance message pointer if the last request argument was
	 * variable-length and the request position will change.
	 */
	if (lastVarArg != argNULL &&
	    lastVarArg->argRequestPos < arg->argRequestPos)
	{
	    WriteAdjustRequestMsgPtr(file, lastVarArg);
	    lastVarArg = argNULL;
	}

	/*
	 * Type-check the argument.
	 */
	WriteTypeCheckArg(file, arg);

	/*
	 * Remember whether this was variable-length.
	 */
	if (akCheckAll(arg->argKind, akbVariable|akbRequest))
	    lastVarArg = arg;
    }
}

static void
WriteExtractArg(FILE *file, register const argument_t *arg)
{
    if (akCheckAll(arg->argKind, akbSendRcv|akbVarNeeded))
	WriteExtractArgValue(file, arg);

    if ((akIdent(arg->argKind) == akeCount) &&
	akCheck(arg->argKind, akbReturnSnd))
    {
	register ipc_type_t *ptype = arg->argParent->argType;

	if (ptype->itInLine && ptype->itVarArray)
	    WriteInitializeCount(file, arg);
    }

    if (akCheckAll(arg->argKind, akbReturnSnd|akbPointer))
	WriteInitializePtr(file, arg);
}

static void
WriteServerCallArg(FILE *file, register const argument_t *arg)
{
    const ipc_type_t *it = arg->argType;
    boolean_t NeedClose = FALSE;

    if (arg->argByReferenceServer)
	fprintf(file, "&");

    if ((it->itInTrans != strNULL) &&
	akCheck(arg->argKind, akbSendRcv) &&
	!akCheck(arg->argKind, akbVarNeeded))
    {
	fprintf(file, "%s(", it->itInTrans);
	NeedClose = TRUE;
    }

    if (akCheck(arg->argKind, akbPointer))
	fprintf(file, "%sP", arg->argVarName);
    else if (akCheck(arg->argKind, akbVarNeeded))
	fprintf(file, "%s", arg->argVarName);
    else if (akCheck(arg->argKind, akbSendRcv)) {
	if (akCheck(arg->argKind, akbIndefinite)) {
	    fprintf(file, "(((mach_msg_type_t*)&In%dP->%s%s)->msgt_inline) ",
		    arg->argRequestPos,
		    arg->argTTName,
		    arg->argLongForm ? ".msgtl_header" : "");
	    fprintf(file, "? %s ", InArgMsgField(arg));
	    fprintf(file, ": *((%s **)%s)",
			FetchServerType(arg->argType->itElement),
			InArgMsgField(arg));
	}
	else
	    fprintf(file, "%s", InArgMsgField(arg));
    }
    else
	WriteMsgFieldRef(file, arg, "OutP->", arg->argTTName);

    if (NeedClose)
	fprintf(file, ")");

    if (!arg->argByReferenceServer && (arg->argMultiplier > 1))
	fprintf(file, " / %d", arg->argMultiplier);
}

static void
WriteDestroyArg(FILE *file, register const argument_t *arg)
{
    register const ipc_type_t *it = arg->argType;

    if (akCheck(arg->argKind, akbIndefinite)) {
	/*
	 * Deallocate only if out-of-line.
	 */
	argument_t *count = arg->argCount;
	ipc_type_t *btype = it->itElement;
	int	multiplier = btype->itTypeSize / btype->itNumber;

	fprintf(file, "\tif (!((mach_msg_type_t*)&In%dP->%s%s)->msgt_inline)\n",
		arg->argRequestPos,
		arg->argTTName,
		arg->argLongForm ? ".msgtl_header" : "");
	fprintf(file, "\t\tmig_deallocate(* (vm_offset_t *) %s, ",
		InArgMsgField(arg));
	if (multiplier > 1)
	    fprintf(file, "%d * ", multiplier);
	fprintf(file, " %s);\n", InArgMsgField(count));
    } else {
	if (akCheck(arg->argKind, akbVarNeeded))
	    fprintf(file, "\t%s(%s);\n", it->itDestructor, arg->argVarName);
	else
	    fprintf(file, "\t%s(%s);\n", it->itDestructor,
		InArgMsgField(arg));
    }
}

static void
WriteDestroyPortArg(FILE *file, register const argument_t *arg)
{
    register const ipc_type_t *it = arg->argType;

    /*
     *	If a translated port argument occurs in the body of a request
     *	message, and the message is successfully processed, then the
     *	port right should be deallocated.  However, the called function
     *	didn't see the port right; it saw the translation.  So we have
     *	to release the port right for it.
     */

    if ((it->itInTrans != strNULL) &&
	(it->itOutName == MACH_MSG_TYPE_PORT_SEND))
    {
	fprintf(file, "\n");
	fprintf(file, "\tif (IP_VALID(%s))\n", InArgMsgField(arg));
	fprintf(file, "\t\tipc_port_release_send(%s);\n", InArgMsgField(arg));
    }
}

/*
 * Check whether WriteDestroyPortArg would generate any code for arg.
 */
static boolean_t
CheckDestroyPortArg(register const argument_t *arg)
{
    register const ipc_type_t *it = arg->argType;

    if ((it->itInTrans != strNULL) &&
	(it->itOutName == MACH_MSG_TYPE_PORT_SEND))
    {
	return TRUE;
    }
    return FALSE;
}

static void
WriteServerCall(FILE *file, const routine_t *rt)
{
    boolean_t NeedClose = FALSE;

    fprintf(file, "\t");
    if (rt->rtServerReturn != argNULL)
    {
	const argument_t *arg = rt->rtServerReturn;
	const ipc_type_t *it = arg->argType;

	WriteMsgFieldRef(file, arg, "OutP->", arg->argTTName);
	fprintf(file, " = ");
	if (it->itOutTrans != strNULL)
	{
	    fprintf(file, "%s(", it->itOutTrans);
	    NeedClose = TRUE;
	}
    }
    fprintf(file, "%s(", rt->rtServerName);
    WriteList(file, rt->rtArgs, WriteServerCallArg, akbServerArg, ", ", "");
    if (NeedClose)
	fprintf(file, ")");
    fprintf(file, ");\n");
}

static void
WriteGetReturnValue(FILE *file, register const routine_t *rt)
{
    if (rt->rtServerReturn != rt->rtRetCode)
	fprintf(file, "\tOutP->%s = KERN_SUCCESS;\n",
		rt->rtRetCode->argMsgField);
}

static void
WriteCheckReturnValue(FILE *file, register const routine_t *rt)
{
    if (rt->rtServerReturn == rt->rtRetCode)
    {
	fprintf(file, "\tif (OutP->%s != KERN_SUCCESS)\n",
		rt->rtRetCode->argMsgField);
	fprintf(file, "\t\treturn;\n");
    }
}

static void
WritePackArgType(FILE *file, register const argument_t *arg)
{
    fprintf(file, "\n");

    WritePackMsgType(file, arg->argType,
		     arg->argType->itIndefinite ? d_NO : arg->argDeallocate,
		     arg->argLongForm, !IsKernelServer,
		     "OutP->%s", "%s", arg->argTTName);
}

static void
WritePackArgValue(FILE *file, register const argument_t *arg)
{
    register const ipc_type_t *it = arg->argType;

    fprintf(file, "\n");

    if (it->itInLine && it->itVarArray) {

	if (it->itString) {
	    /*
	     *	Copy variable-size C string with mig_strncpy.
	     *	Save the string length (+ 1 for trailing 0)
	     *	in the argument`s count field.
	     */
	    fprintf(file,
		"\tOutP->%s = mig_strncpy(OutP->%s, %s, %d);\n",
		arg->argCount->argMsgField,
		arg->argMsgField,
		arg->argVarName,
		it->itNumber);
	}
	else {
	    register argument_t *count = arg->argCount;
	    register ipc_type_t *btype = it->itElement;

	    /* Note btype->itNumber == count->argMultiplier */

	    if (it->itIndefinite) {
		/*
		 * If we are packing argument, it must be from
		 * a local variable.
		 */
		fprintf(file, "\tif (%sP != %s) {\n",
			arg->argVarName,
			arg->argVarName);
		fprintf(file, "\t\t((mach_msg_type_t*)&OutP->%s%s)->msgt_inline = FALSE;\n",
			arg->argTTName,
			arg->argLongForm ? ".msgtl_header" : "");
		if (arg->argDeallocate == d_YES)
		    fprintf(file, "\t\t((mach_msg_type_t*)&OutP->%s%s)->msgt_deallocate = TRUE;\n",
			    arg->argTTName,
			    arg->argLongForm ? ".msgtl_header" : "");
		else if (arg->argDeallocate == d_MAYBE)
		    fprintf(file, "\t\t((mach_msg_type_t*)&OutP->%s%s)->msgt_deallocate = %s;\n",
			    arg->argTTName,
			    arg->argLongForm ? ".msgtl_header" : "",
			    arg->argDealloc->argVarName);
		fprintf(file, "\t\t*((%s **)OutP->%s) = %sP;\n",
			FetchServerType(btype),
			arg->argMsgField,
			arg->argVarName);
		if (!arg->argRoutine->rtSimpleFixedReply)
		    fprintf(file, "\t\tmsgh_simple = FALSE;\n");
		fprintf(file, "\t}\n\telse {\n\t");
	    }
	    fprintf(file, "\tmemcpy(OutP->%s, %s, ",
		    arg->argMsgField, arg->argVarName);
	    if (btype->itTypeSize > 1)
		fprintf(file, "%d * ",
			btype->itTypeSize);
	    fprintf(file, "%s);\n",
		count->argVarName);
	    if (it->itIndefinite)
		fprintf(file, "\t}\n");
	}
    }
    else if (arg->argMultiplier > 1)
	WriteCopyType(file, it, "OutP->%s", "/* %s */ %d * %s",
		      arg->argMsgField,
		      arg->argMultiplier,
		      arg->argVarName);
    else if (it->itOutTrans != strNULL)
	WriteCopyType(file, it, "OutP->%s", "/* %s */ %s(%s)",
		      arg->argMsgField, it->itOutTrans, arg->argVarName);
    else
	WriteCopyType(file, it, "OutP->%s", "/* %s */ %s",
		      arg->argMsgField, arg->argVarName);
}

static void
WriteCopyArgValue(FILE *file, register const argument_t *arg)
{
    fprintf(file, "\n");
    WriteCopyType(file, arg->argType, "/* %d */ OutP->%s", "In%dP->%s",
		  arg->argRequestPos, arg->argMsgField);
}

static void
WriteAdjustMsgSimple(FILE *file, register const argument_t *arg)
{
    /* akbVarNeeded must be on */

    if (!arg->argRoutine->rtSimpleFixedReply)
    {
	fprintf(file, "\n");
	fprintf(file, "\tif (MACH_MSG_TYPE_PORT_ANY(%s))\n", arg->argVarName);
	fprintf(file, "\t\tmsgh_simple = FALSE;\n");
    }
}

static void
WriteAdjustMsgCircular(FILE *file, register const argument_t *arg)
{
    fprintf(file, "\n");

    if (arg->argType->itOutName == MACH_MSG_TYPE_POLYMORPHIC)
	fprintf(file, "\tif (%s == MACH_MSG_TYPE_PORT_RECEIVE)\n",
		arg->argPoly->argVarName);

    /*
     *	The carried port right can be accessed in OutP->XXXX.  Normally
     *	the server function stuffs it directly there.  If it is InOut,
     *	then it has already been copied into the reply message.
     *	If the server function deposited it into a variable (perhaps
     *	because the reply message is variable-sized) then it has already
     *	been copied into the reply message.  Note we must use InHeadP
     *	(or In0P->Head) and OutHeadP to access the message headers,
     *	because of the variable-sized messages.
     */

    fprintf(file, "\tif (IP_VALID((ipc_port_t) InHeadP->msgh_reply_port) &&\n");
    fprintf(file, "\t    IP_VALID((ipc_port_t) OutP->%s) &&\n", arg->argMsgField);
    fprintf(file, "\t    ipc_port_check_circularity((ipc_port_t) OutP->%s, (ipc_port_t) InHeadP->msgh_reply_port))\n", arg->argMsgField);
    fprintf(file, "\t\tOutHeadP->msgh_bits |= MACH_MSGH_BITS_CIRCULAR;\n");
}

/*
 * Calculate the size of a variable-length message field.
 */
static void
WriteArgSize(
    FILE	*file,
    register const argument_t *arg,
    boolean_t	longalign)
{
    register const ipc_type_t *ptype = arg->argType;
    register int bsize = ptype->itElement->itTypeSize;
    register const argument_t *count = arg->argCount;
    boolean_t needs_rounding;

    if (ptype->itIndefinite) {
	/*
	 * Check descriptor.  If out-of-line, use standard size.
	 */
	fprintf(file, "(((mach_msg_type_t*)&OutP->%s%s)->msgt_inline) ? ",
		arg->argTTName,
		arg->argLongForm ? ".msgtl_header" : "");
    }

    needs_rounding = (longalign) ? (bsize % word_size != 0) : (bsize % sizeof_mach_msg_type_t != 0);
    if (needs_rounding)
	fprintf(file, "(");

    if (bsize > 1)
	fprintf(file, "%d * ", bsize);
    if (ptype->itString)
	/* get count from descriptor in message */
	WriteMsgFieldRef( file, count, "OutP->", arg->argTTName);
    else
	/* get count from argument */
	fprintf(file, "%s", count->argVarName);

    /*
     * If the base type size is not a multiple of sizeof(int) [4],
     * we have to round up.  What this means is that this data
     * is followed by a type descriptor, which must be aligned.
     * "longalign" tells us wether this is a short or long one.
     */
    if (needs_rounding) {
	char *l = (longalign) ? "natural_t" : "mach_msg_type_t";
	fprintf(file, " + (sizeof(%s)-1)) & ~(sizeof(%s)-1)", l, l);
    }

    if (ptype->itIndefinite) {
	fprintf(file, " : sizeof(%s *)",
		FetchServerType(ptype->itElement));
    }
}

static void
WriteArgSizeStright(
    FILE        *file,
    register const argument_t *arg)
{
    register const ipc_type_t *ptype = arg->argType;
    register int bsize = ptype->itElement->itTypeSize;
    register const argument_t *count = arg->argCount;

    if (ptype->itIndefinite) {
	/*
	 * Check descriptor.  If out-of-line, use standard size.
	 */
	fprintf(file, "(((mach_msg_type_t*)&OutP->%s%s)->msgt_inline) ? ",
		arg->argTTName,
		arg->argLongForm ? ".msgtl_header" : "");
    }

    if (bsize > 1)
	fprintf(file, "%d * ", bsize);
    if (ptype->itString)
	/* get count from descriptor in message */
	WriteMsgFieldRef( file, count, "OutP->", arg->argTTName);
    else
	/* get count from argument */
	fprintf(file, "%s", count->argVarName);

    if (ptype->itIndefinite) {
	fprintf(file, " : sizeof(%s *)",
		FetchServerType(ptype->itElement));
    }
}


/*
 * Adjust message size and advance reply pointer.
 * Called after packing a variable-length argument that
 * has more arguments following.
 */
static void
WriteAdjustMsgSize(
    FILE	*file,
    register const argument_t *arg,
    const argument_t	*next_arg)
{
    register routine_t *rt = arg->argRoutine;
    register ipc_type_t *ptype = arg->argType;

    /* There are more Out arguments.  We need to adjust msgh_size
       and advance OutP, so we save the size of the current field
       in msgh_size_delta. */

    fprintf(file, "\tmsgh_size_delta = ");
    WriteArgSize(file, arg, next_arg->argLongForm);
    fprintf(file, ";\n");

    /* On the Alpha and similar machines, a long type leaves some
       padding behind, which is ambiguous to the kernel in some
       instances.  The (ugly) fix is to agree on what a padding
       looks like, with the kernel.  Turning all bits on has the
       side effect of turning on the 'longform' bit, which the
       code in the kernel uses as trigger... */

    if (!arg->argLongForm && next_arg->argLongForm) {
	fprintf(file, "\tif (sizeof(natural_t) > sizeof(mach_msg_type_t)) {\n");
	fprintf(file, "\t\tint nb = ");
	WriteArgSizeStright(file, arg);
	fprintf(file,
	    ";\n\t\tint fractal = sizeof(natural_t) - (nb & (sizeof(natural_t)-1));\n");
	fprintf(file, "\t\tif (fractal > (sizeof(natural_t) - sizeof(mach_msg_type_t)))\n");
	fprintf(file, "#if 0 /* couldbe */\n");
	fprintf(file, "\t\t\tmemset(&OutP->%s[nb], -1, fractal);\n", arg->argMsgField);
	fprintf(file, "#else /* couldbe */\n");
	fprintf(file, "\t\t\t{register char *p = (char *)&OutP->%s[nb];\n", arg->argMsgField);
	fprintf(file, "\t\t\t   while (fractal--) *p++ = -1;}\n");
	fprintf(file, "#endif /* couldbe */\n");
	fprintf(file, "\t}\n");
    }

    if (rt->rtNumReplyVar == 1)
	/* We can still address the message header directly.  Fill
	   in the size field. */

	fprintf(file, "\tOutP->Head.msgh_size = %d + msgh_size_delta;\n",
			rt->rtReplySize);
    else
    if (arg->argReplyPos == 0)
	/* First variable-length argument.  The previous msgh_size value
	   is the minimum reply size. */

	fprintf(file, "\tmsgh_size = %d + msgh_size_delta;\n",
		rt->rtReplySize);
    else
	fprintf(file, "\tmsgh_size += msgh_size_delta;\n");

    fprintf(file,
	"\tOutP = (Reply *) ((char *) OutP + msgh_size_delta - %d);\n",
	ptype->itTypeSize + ptype->itPadSize);
}

/*
 * Calculate the size of the message.  Called after the
 * last argument has been packed.
 */
static void
WriteFinishMsgSize(FILE *file, register const argument_t *arg)
{
    /* No more Out arguments.  If this is the only variable Out
       argument, we can assign to msgh_size directly. */

    if (arg->argReplyPos == 0) {
	fprintf(file, "\tOutP->Head.msgh_size = %d + (",
			arg->argRoutine->rtReplySize);
	WriteArgSize(file, arg, FALSE);
	fprintf(file, ");\n");
    }
    else {
	fprintf(file, "\tmsgh_size += ");
	WriteArgSize(file, arg, FALSE);
	fprintf(file, ";\n");
    }
}

static void
WritePackArg(FILE *file, register const argument_t *arg)
{
    if (akCheck(arg->argKind, akbReplyInit))
	WritePackArgType(file, arg);

    if ((akIdent(arg->argKind) == akePoly) &&
	akCheck(arg->argKind, akbReturnSnd))
	WriteAdjustMsgSimple(file, arg);

    if (akCheckAll(arg->argKind, akbReturnSnd|akbVarNeeded))
	WritePackArgValue(file, arg);
    else if (akCheckAll(arg->argKind, akbReturnSnd|akbVariable)) {
	register const ipc_type_t *it = arg->argType;

	if (it->itString) {
	    /* Need to call strlen to calculate the size of the argument. */
	    WriteMsgFieldRef( file, arg->argCount, "\tOutP->", arg->argTTName);
	    fprintf(file, " = strlen(OutP->%s) + 1;\n", arg->argMsgField);
	} else if (it->itIndefinite) {
	    /*
	     * We know that array is in reply message.
	     */
	    fprintf(file, "\tif (%sP != OutP->%s) {\n",
			arg->argVarName,
			arg->argMsgField);
	    fprintf(file, "\t\t((mach_msg_type_t*)&OutP->%s%s)->msgt_inline = FALSE;\n",
		    arg->argTTName,
		    arg->argLongForm ? ".msgtl_header" : "");
	    if (arg->argDeallocate == d_YES)
		fprintf(file, "\t\t((mach_msg_type_t*)&OutP->%s%s)->msgt_deallocate = TRUE;\n",
			arg->argTTName,
			arg->argLongForm ? ".msgtl_header" : "");
	    else if (arg->argDeallocate == d_MAYBE)
		fprintf(file, "\t\t((mach_msg_type_t*)&OutP->%s%s)->msgt_deallocate = %s;\n",
			arg->argTTName,
			arg->argLongForm ? ".msgtl_header" : "",
			arg->argDealloc->argVarName);
	    fprintf(file, "\t\t*((%s **)OutP->%s) = %sP;\n",
			FetchServerType(it->itElement),
			arg->argMsgField,
			arg->argVarName);
	    if (!arg->argRoutine->rtSimpleFixedReply)
		fprintf(file, "\t\tmsgh_simple = FALSE;\n");
	    fprintf(file, "\t}\n");
	}
    }

    if (akCheck(arg->argKind, akbReplyCopy))
	WriteCopyArgValue(file, arg);

    /*
     *	If this is a KernelServer, and the reply message contains
     *	a receive right, we must check for the possibility of a
     *	port/message circularity.  If queueing the reply message
     *	would cause a circularity, we mark the reply message
     *	with the circular bit.
     */

    if (IsKernelServer &&
	akCheck(arg->argKind, akbReturnSnd) &&
	((arg->argType->itOutName == MACH_MSG_TYPE_PORT_RECEIVE) ||
	 (arg->argType->itOutName == MACH_MSG_TYPE_POLYMORPHIC)))
	WriteAdjustMsgCircular(file, arg);
}

/*
 * Handle reply arguments - fill in message types and copy arguments
 * that need to be copied.
 */
static void
WritePackReplyArgs(FILE *file, register const routine_t *rt)
{
    register const argument_t *arg;
    register const argument_t *lastVarArg;

    lastVarArg = argNULL;
    for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {

	/*
	 * Adjust message size and advance message pointer if
	 * the last reply argument was variable-length and the
	 * request position will change.
	 */
	if (lastVarArg != argNULL &&
	    lastVarArg->argReplyPos < arg->argReplyPos)
	{
	    WriteAdjustMsgSize(file, lastVarArg, arg);
	    lastVarArg = argNULL;
	}

	/*
	 * Copy the argument
	 */
	WritePackArg(file, arg);

	/*
	 * Remember whether this was variable-length.
	 */
	if (akCheckAll(arg->argKind, akbReturnSnd|akbVariable))
	    lastVarArg = arg;
    }

    /*
     * Finish the message size.
     */
    if (lastVarArg != argNULL)
	WriteFinishMsgSize(file, lastVarArg);
}

static void
WriteFieldDecl(FILE *file, const argument_t *arg)
{
    WriteFieldDeclPrim(file, arg, FetchServerType);
}

static void
WriteRoutine(FILE *file, register const routine_t *rt)
{
    fprintf(file, "\n");

    fprintf(file, "/* %s %s */\n", rtRoutineKindToStr(rt->rtKind), rt->rtName);
    fprintf(file, "mig_internal void _X%s\n", rt->rtName);
    fprintf(file, "\t(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)\n");

    fprintf(file, "{\n");
    WriteStructDecl(file, rt->rtArgs, WriteFieldDecl, akbRequest, "Request");
    WriteStructDecl(file, rt->rtArgs, WriteFieldDecl, akbReply, "Reply");

    WriteVarDecls(file, rt);

    WriteList(file, rt->rtArgs, WriteCheckDecl, akbRequestQC, "\n", "\n");
    WriteList(file, rt->rtArgs,
	      IsKernelServer ? WriteTypeDeclOut : WriteTypeDeclIn,
	      akbReplyInit, "\n", "\n");

    WriteList(file, rt->rtArgs, WriteLocalVarDecl,
	      akbVarNeeded, ";\n", ";\n\n");
    WriteList(file, rt->rtArgs, WriteLocalPtrDecl,
	      akbPointer, ";\n", ";\n\n");

    WriteCheckHead(file, rt);

    WriteTypeCheckRequestArgs(file, rt);
    WriteList(file, rt->rtArgs, WriteExtractArg, akbNone, "", "");

    WriteServerCall(file, rt);
    WriteGetReturnValue(file, rt);

    WriteReverseList(file, rt->rtArgs, WriteDestroyArg, akbDestroy, "", "");

    /*
     * For one-way routines, it doesn`t make sense to check the return
     * code, because we return immediately afterwards.  However,
     * kernel servers may want to deallocate port arguments - and the
     * deallocation must not be done if the return code is not KERN_SUCCESS.
     */
    if (rt->rtOneWay || rt->rtNoReplyArgs)
    {
	if (IsKernelServer)
	{
	    if (rtCheckMaskFunction(rt->rtArgs, akbSendBody|akbSendRcv,
				CheckDestroyPortArg))
	    {
		WriteCheckReturnValue(file, rt);
	    }
	    WriteReverseList(file, rt->rtArgs, WriteDestroyPortArg,
			 akbSendBody|akbSendRcv, "", "");
	}
    }
    else
    {
	WriteCheckReturnValue(file, rt);

	if (IsKernelServer)
	    WriteReverseList(file, rt->rtArgs, WriteDestroyPortArg,
			 akbSendBody|akbSendRcv, "", "");

	WriteReplyInit(file, rt);
	WritePackReplyArgs(file, rt);
	WriteReplyHead(file, rt);
    }

    fprintf(file, "}\n");
}

void
WriteServer(FILE *file, const statement_t *stats)
{
    register const statement_t *stat;

    WriteProlog(file);
    for (stat = stats; stat != stNULL; stat = stat->stNext)
	switch (stat->stKind)
	{
	  case skRoutine:
	    WriteRoutine(file, stat->stRoutine);
	    break;
	  case skImport:
	  case skSImport:
	    WriteImport(file, stat->stFileName);
	    break;
	  case skUImport:
	    break;
	  default:
	    fatal("WriteServer(): bad statement_kind_t (%d)",
		  (int) stat->stKind);
	}
    WriteEpilog(file, stats);
}
