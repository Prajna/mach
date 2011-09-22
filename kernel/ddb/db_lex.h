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
 * $Log:	db_lex.h,v $
 * Revision 2.6  93/01/14  17:25:13  danner
 * 	64bit cleanup.
 * 	[92/11/30            af]
 * 
 * Revision 2.5  91/10/09  16:00:48  af
 * 	 Revision 2.4.3.1  91/10/05  13:06:34  jeffreyh
 * 	 	Added db_lex_context structure and some routine declarations
 * 	 	  for macro and conditinal command.
 * 	 	Added relational operator tokens etc. for condition expression.
 * 	 	Changed TOK_STRING_SIZE from 120 to 64, and defined
 * 	 	  DB_LEX_LINE_SIZE as 256 which was previously embedded
 * 	 	  in db_lex.c as 120.
 * 	 	[91/08/29            tak]
 * 	Revision 2.4.1 91/07/15  09:30:00  tak
 * 		Added db_lex_context for macro support
 * 		Added some lexical constants to support logical expression etc.
 * 		[91/05/15  13:55:00  tak]
 * 
 * Revision 2.4.3.1  91/10/05  13:06:34  jeffreyh
 * 	Added db_lex_context structure and some routine declarations
 * 	  for macro and conditinal command.
 * 	Added relational operator tokens etc. for condition expression.
 * 	Changed TOK_STRING_SIZE from 120 to 64, and defined
 * 	  DB_LEX_LINE_SIZE as 256 which was previously embedded
 * 	  in db_lex.c as 120.
 * 	[91/08/29            tak]
 * 
 * Revision 2.4.1 91/07/15  09:30:00  tak
 *	Added db_lex_context for macro support
 *	Added some lexical constants to support logical expression etc.
 *	[91/05/15  13:55:00  tak]
 *
 * Revision 2.4  91/05/14  15:34:38  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:06:41  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  16:18:28  mrt]
 * 
 * Revision 2.2  90/08/27  21:51:16  dbg
 * 	Add 'dotdot' token.
 * 	[90/08/22            dbg]
 * 	Export db_flush_lex.
 * 	[90/08/07            dbg]
 * 	Created.
 * 	[90/07/25            dbg]
 * 
 */
/*
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date:	7/90
 */
/*
 * Lexical analyzer.
 */

#define	TOK_STRING_SIZE		64 
#define DB_LEX_LINE_SIZE	256

struct db_lex_context {
	int  l_char;		/* peek char */
	int  l_token;		/* peek token */
	char *l_ptr;		/* line pointer */
	char *l_eptr;		/* line end pointer */
};

extern int	db_read_line(/* char *rep_str */);
extern void	db_flush_line();
extern int	db_read_char();
extern void	db_unread_char(/* char c */);
extern int	db_read_token();
extern void	db_unread_token(/* int t */);
extern void	db_flush_lex();
extern void	db_switch_input(/* char *, int */);
extern void	db_save_lex_context(/* struct db_lex_context * */);
extern void	db_restore_lex_context(/* struct db_lex_context * */);
extern void	db_skip_to_eol();

extern db_expr_t db_tok_number;
extern char	db_tok_string[TOK_STRING_SIZE];
extern db_expr_t db_radix;

#define	tEOF		(-1)
#define	tEOL		1
#define	tNUMBER		2
#define	tIDENT		3
#define	tPLUS		4
#define	tMINUS		5
#define	tDOT		6
#define	tSTAR		7
#define	tSLASH		8
#define	tEQ		9
#define	tLPAREN		10
#define	tRPAREN		11
#define	tPCT		12
#define	tHASH		13
#define	tCOMMA		14
#define	tQUOTE		15
#define	tDOLLAR		16
#define	tEXCL		17
#define	tSHIFT_L	18
#define	tSHIFT_R	19
#define	tDOTDOT		20
#define tSEMI_COLON	21
#define tLOG_EQ		22
#define tLOG_NOT_EQ	23
#define tLESS		24
#define tLESS_EQ	25
#define tGREATER	26
#define tGREATER_EQ	27
#define tBIT_AND	28
#define tBIT_OR		29
#define tLOG_AND	30
#define tLOG_OR		31
#define tSTRING		32
#define tQUESTION	33
