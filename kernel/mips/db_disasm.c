/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
 * $Log:	db_disasm.c,v $
 * Revision 2.7  93/01/14  17:51:00  danner
 * 	Added include of mach/std_types.h
 * 	[92/12/10  20:06:49  af]
 * 
 * Revision 2.6  91/10/09  16:13:20  af
 * 	 Revision 2.5.1.1  91/10/05  13:11:59  jeffreyh
 * 	 	Changed db_get_value and db_printsym to db_get_task_value and
 * 	 	  db_task_printsym, and passed task or thread parameter to
 * 	 	  some function calls.
 * 	 	[91/09/05            tak]
 * 
 * Revision 2.5.1.1  91/10/05  13:11:59  jeffreyh
 * 	Changed db_get_value and db_printsym to db_get_task_value and
 * 	  db_task_printsym, and passed task or thread parameter to
 * 	  some function calls.
 * 	[91/09/05            tak]
 * 
 * Revision 2.5  91/08/24  12:22:29  af
 * 	We are supposed to print a newline before returning from db_disasm().
 * 	[91/08/02  16:25:54  af]
 * 
 * Revision 2.4  91/05/14  17:33:20  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:47:46  mrt
 * 	Added author notices
 * 	[91/02/04  11:21:47  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:25:22  mrt]
 * 
 * Revision 2.2  90/08/27  22:07:14  dbg
 * 	Change calling sequence of db_disasm.
 * 	[90/08/27            dbg]
 * 
 * 	Fixed includes, reflected changes in db_printsym()'s interface.
 * 	[90/08/20  10:17:43  af]
 * 
 * 	Created, from my old KDB code.  History summary:
 * 		Well, I did find the time to switch over to the MI kdb.
 * 		[90/01/20            af]
 * 		Created.
 * 		[89/08/08            af]
 * 
 */

/*
 *	File: db_disasm.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	8/89
 *
 *	Disassembler for MIPS
 */

#include <mach/mips/mips_instruction.h>
#include <mach/std_types.h>
#include <machine/db_machdep.h>		/* type definitions */
#include <ddb/db_sym.h>
#include <kern/task.h>
#include <kern/thread.h>

static char *op_name[64] = {
/* 0 */	"spec",	"bcond","j",	"jal",	"beq",	"bne",	"blez",	"bgtz",
/* 8 */	"addi",	"addiu","slti",	"sltiu","andi",	"ori",	"xori",	"lui",
/*16 */	"cop0",	"cop1",	"cop2",	"cop3",	"op50",	"op54",	"op58",	"op5c",
/*24 */	"op60",	"op64",	"op68",	"op6c",	"op70",	"op74",	"op78",	"op7c",
/*32 */	"lb",	"lh",	"lwl",	"lw",	"lbu",	"lhu",	"lwr",	"ld",
/*40 */	"sb",	"sh",	"swl",	"sw",	"opb0",	"opb4",	"swr",	"sd",
/*48 */	"lwc0",	"lwc1",	"lwc2",	"lwc3",	"ldc0",	"ldc1",	"ldc2",	"ldc3",
/*56 */	"swc0",	"swc1",	"swc2",	"swc3",	"sdc0",	"sdc1",	"sdc2",	"sdc3"
};

static char *spec_name[64] = {
/* 0 */	"sll",	"spec01","srl",	"sra",	"sllv",	"spec05","srlv","srav",
/* 8 */	"jr",	"jalr",	"spec12","spec13","syscall","break","spec16","tas",
/*16 */	"mfhi",	"mthi",	"mflo",	"mtlo",	"spec24","spec25","spec26","spec27",
/*24 */	"mult",	"multu","div",	"divu",	"spec34","spec35","spec36","spec37",
/*32 */	"add",	"addu",	"sub",	"subu",	"and",	"or",	"xor",	"nor",
/*40 */	"spec50","spec51","slt","sltu",	"spec54","spec55","spec56","spec57",
/*48 */	"spec60","spec61","spec62","spec63","spec64","spec65","spec66","spec67",
/*56 */	"spec70","spec71","spec72","spec73","spec74","spec75","spec76","spec77"
};

static char *bcond_name[32] = {
/* 0 */	"bltz",	"bgez", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 16 */ "bltzal", "bgezal",
};

static char *cop1_name[64] = {
/* 0 */	"fadd",	"fsub",	"fmpy",	"fdiv",	"fsqrt","fabs",	"fmov",	"fneg",
/* 8 */	"fop08","fop09","fop0a","fop0b","fop0c","fop0d","fop0e","fop0f",
/*16 */	"fop10","fop11","fop12","fop13","fop14","fop15","fop16","fop17",
/*24 */	"fop18","fop19","fop1a","fop1b","fop1c","fop1d","fop1e","fop1f",
/*32 */	"fcvts","fcvtd","fcvte","fop23","fcvtw","fop25","fop26","fop27",
/*40 */	"fop28","fop29","fop2a","fop2b","fop2c","fop2d","fop2e","fop2f",
/*48 */	"fcmp.f","fcmp.un","fcmp.eq","fcmp.ueq","fcmp.olt","fcmp.ult",
	"fcmp.ole","fcmp.ule",
/*56 */	"fcmp.sf","fcmp.ngle","fcmp.seq","fcmp.ngl","fcmp.lt","fcmp.nge",
	"fcmp.le","fcmp.ngt"
};

static char *fmt_name[16] = {
	"s",	"d",	"e",	"fmt3",
	"w",	"fmt5",	"fmt6",	"fmt7",
	"fmt8",	"fmt9",	"fmta",	"fmtb",
	"fmtc",	"fmtd",	"fmte",	"fmtf"
};

static char *sbregister_name[32] = {
		"zero",	"at",	"v0",	"v1",	"a0",	"a1",	"a2",	"a3",
		"t0",	"t1",	"t2",	"t3",	"t4",	"t5",	"t6",	"t7",
		"s0",	"s1",	"s2",	"s3",	"s4",	"s5",	"s6",	"s7",
		"t8",	"t9",	"k0",	"k1",	"gp",	"sp",	"s8",	"ra"
};

static char *c0_opname[32] = {
	"c0op0","tlbr",	"tlbwi","c0op3","c0op4","c0op5","tlbwr","c0op7",
	"tlbp",	"c0op9","c0op10","c0op11","c0op12","c0op13","c0op14","c0op15",
	"rfe",	"c0op17","c0op18","c0op19","c0op20","c0op21","c0op22","c0op23",
	"c0op24","c0op25","c0op26","c0op27","c0op28","c0op29","c0op30","c0op31"
};

static char *c0_reg[32] = {
	"index","random","tlblo","c0r3","context","c0r5","c0r6","c0r7",
	"badvaddr","c0r9","tlbhi","c0r11","sr",	"cause","epc",	"c0r15",
	"c0r16","c0r17","c0r18","c0r19","c0r20","c0r21","c0r22","c0r23",
	"c0r24","c0r25","c0r26","c0r27","c0r28","c0r29","c0r30","c0r31"
};

static int regcount;		/* how many regs used in this inst */
static int regnum[6];		/* which regs used in this inst */


static char *
register_name (ireg)
{
	int	i;

	for (i = 0; i < regcount; i++)
		if (regnum[i] == ireg)
			break;
	if (i >= regcount)
		regnum[regcount++] = ireg;
	return (sbregister_name[ireg]);
}

#define	peek_inst(addr,task)		db_get_task_value(addr,4,0,task)

/*
 * Disassemble instruction at 'loc'.  'altfmt' specifies an
 * (optional) alternate format.  Return address of start of
 * next instruction.
 */
db_addr_t
db_disasm(loc, altfmt, task)
	db_addr_t	loc;
	boolean_t	altfmt;
	task_t		task;
{
	loc += mips_print_instruction(loc, peek_inst(loc, task), altfmt, task);
	return (loc);
}

mips_print_instruction(iadr, i, showregs, task)
	unsigned iadr;
	mips_instruction i;
	task_t	 task;
{
	char *s;
	int print_next;
	int ireg;
	int ibytes;
	short signed_immediate;
	unsigned unsigned_immediate;

	regcount = 0;
	print_next = 0;
	ibytes = 4;


	switch (i.j_format.opcode) {
	case op_special:
		if (i.bits == 0) {
			db_printf("nop");
			break;
		}
		else if (i.r_format.func == op_addu && i.r_format.rt == 0) {
			db_printf("move\t%s,%s",
			    register_name(i.r_format.rd),
			    register_name(i.r_format.rs));
			break;
		}
		db_printf("%s", spec_name[i.r_format.func]);
		switch (i.r_format.func) {
		case op_sll:
		case op_srl:
		case op_sra:
			db_printf("\t%s,%s,%d",
			    register_name(i.r_format.rd),
			    register_name(i.r_format.rt),
			    i.r_format.shamt);
			break;
		case op_sllv:
		case op_srlv:
		case op_srav:
			db_printf("\t%s,%s,%s",
			    register_name(i.r_format.rd),
			    register_name(i.r_format.rt),
			    register_name(i.r_format.rs));
			break;
		case op_mfhi:
		case op_mflo:
			db_printf("\t%s", register_name(i.r_format.rd));
			break;
		case op_jr:
		case op_jalr:
			print_next = 1;
			/* fall through */
		case op_mtlo:
		case op_mthi:
			db_printf("\t%s", register_name(i.r_format.rs));
			break;
		case op_mult:
		case op_multu:
		case op_div:
		case op_divu:
			db_printf("\t%s,%s",
			    register_name(i.r_format.rs),
			    register_name(i.r_format.rt));
			break;
		case op_syscall:
			break;
		case op_break:
			db_printf("\t%d", i.r_format.rs*32+i.r_format.rt);
			break;
		case op_tas:
			db_printf("\t%s", register_name(4));
			break;
		default:
			db_printf("\t%s,%s,%s",
			    register_name(i.r_format.rd),
			    register_name(i.r_format.rs),
			    register_name(i.r_format.rt));
			break;
		};
		break;
	case op_bcond:
		db_printf("%s\t%s,",
		    bcond_name[i.i_format.rt],
		    register_name(i.i_format.rs));
		goto branch_displacement;
	case op_blez:
	case op_bgtz:
		db_printf("%s\t%s,", op_name[i.i_format.opcode],
		    register_name(i.i_format.rs));
		goto branch_displacement;
	case op_beq:
		if (i.i_format.rs == 0 && i.i_format.rt == 0) {
			db_printf("b\t");
			goto branch_displacement;
		}
		/* fall through */
	case op_bne:
		db_printf("%s\t%s,%s,", op_name[i.i_format.opcode],
		    register_name(i.i_format.rs),
		    register_name(i.i_format.rt));
branch_displacement:
		signed_immediate = i.i_format.simmediate;
		db_task_printsym(iadr+4+(signed_immediate<<2),
				 DB_STGY_PROC, task);
		print_next = 1;
		break;
	case op_cop0:
		switch (i.r_format.rs) {
		case op_bc:
			db_printf("bc0%c\t", "ft"[i.r_format.rt]);
			goto branch_displacement;
		case op_mtc:
			db_printf("mtc0\t%s,%s",
			    register_name(i.r_format.rt),
			    c0_reg[i.f_format.fs]);
			break;
		case op_mfc:
			db_printf("mfc0\t%s,%s",
			    register_name(i.r_format.rt),
			    c0_reg[i.f_format.fs]);
			break;
		default:
			db_printf("c0\t%s", c0_opname[i.f_format.func]);
			break;
		};
		break;
	case op_cop1:
		switch (i.r_format.rs) {
		case op_bc:
			db_printf("bc1%c\t", "ft"[i.r_format.rt]);
			goto branch_displacement;
		case op_mtc:
			db_printf("mtc1\t%s,f%d",
			    register_name(i.r_format.rt),
			    i.f_format.fs);
			break;
		case op_mfc:
			db_printf("mfc1\t%s,f%d",
			    register_name(i.r_format.rt),
			    i.f_format.fs);
			break;
		default:
			db_printf("%s.%s\tf%d,f%d,f%d",
			    cop1_name[i.f_format.func],
			    fmt_name[i.f_format.fmt],
			    i.f_format.fd, i.f_format.fs, i.f_format.ft);
			break;
		};
		break;

	case op_j:
	case op_jal:
		db_printf("%s\t", op_name[i.j_format.opcode]);
		unsigned_immediate = ((iadr+4)&~((1<<28)-1)) +(i.j_format.target<<2);
		db_task_printsym(unsigned_immediate, DB_STGY_PROC, task);
		print_next = 1;
		break;

	case op_swc1:
	case op_lwc1:
		db_printf("%s\tf%d,", op_name[i.i_format.opcode],
		    i.i_format.rt);
		goto loadstore;

	case op_lb:
	case op_lh:
	case op_lw:
	case op_lbu:
	case op_lhu:
	case op_sb:
	case op_sh:
	case op_sw:
		db_printf("%s\t%s,", op_name[i.i_format.opcode],
		    register_name(i.i_format.rt));
loadstore:
		signed_immediate = i.i_format.simmediate;
		db_printf("%Z(%s)", signed_immediate,
			register_name(i.i_format.rs));
		if (showregs && task == TASK_NULL) {
			signed_immediate = i.i_format.simmediate;
			db_printf(" <%X>", signed_immediate +
			    db_getreg_val(i.i_format.rs));
		}
		break;

	case op_ori:
	case op_xori:
		if (i.i_format.rs == 0) {
			db_printf("li\t%s,%X",
			    register_name(i.i_format.rt),
			    i.i_format.uimmediate);
			break;
		}
		/* fall through */
	case op_andi:
		db_printf("%s\t%s,%s,%X", op_name[i.i_format.opcode],
		    register_name(i.i_format.rt),
		    register_name(i.i_format.rs),
		    i.i_format.uimmediate);
		break;
	case op_lui:
		db_printf("%s\t%s,%X", op_name[i.i_format.opcode],
		    register_name(i.i_format.rt),
		    i.i_format.uimmediate);
		break;
	case op_addi:
	case op_addiu:
		if (i.i_format.rs == 0) {
			signed_immediate = i.i_format.simmediate;
 			db_printf("li\t%s,%X",
			    register_name(i.i_format.rt),
			    signed_immediate);
			break;
		}
		/* fall through */
	default:
		signed_immediate = i.i_format.simmediate;
		db_printf("%s\t%s,%s,%Z", op_name[i.i_format.opcode],
		    register_name(i.i_format.rt),
		    register_name(i.i_format.rs),
		    signed_immediate);
		break;
	}

	/* print out the registers use in this inst */
	if (showregs && regcount > 0 && task == TASK_NULL) {
		db_printf("\t<");
		for (ireg = 0; ireg < regcount; ireg++) {
			if (ireg != 0)
				db_printf(",");
			db_printf("%s=%X",
			    sbregister_name[regnum[ireg]],
			    db_getreg_val(regnum[ireg]));
		}
		db_printf(">");
	}
	if (print_next) {
		db_printf("\n\t");
		mips_print_instruction(iadr+4, peek_inst(iadr+4, task),
					showregs, task);
		ibytes += 4;
	}
	db_printf("\n");
	return(ibytes);
}

