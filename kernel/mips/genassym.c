/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	genassym.c,v $
 * Revision 2.10  92/01/03  20:23:46  dbg
 * 	Remove fixed lower bound on emulated system call number.
 * 	[91/11/01            dbg]
 * 
 * Revision 2.9  91/05/14  17:34:27  mrt
 * 	Correcting copyright
 * 
 * Revision 2.8  91/03/16  14:56:04  rpd
 * 	Removed MKS_SR.
 * 	[91/03/01            rpd]
 * 
 * 	Added mips_float_state definitions.
 * 	[91/02/18            rpd]
 * 
 * 	Added PCB_SIZE.
 * 	[91/02/01            rpd]
 * 	Removed pcache support.
 * 	[91/01/10            rpd]
 * 
 * Revision 2.7  91/02/05  17:48:38  mrt
 * 	Corrected copyright
 * 	[91/02/04  12:29:40  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:26:12  mrt]
 * 
 * Revision 2.6  91/01/08  15:49:55  rpd
 * 	Added mips_stack_base.
 * 	[91/01/02            rpd]
 * 
 * 	Changed to consistent MKS_, MSS_, MEL_, MMS_ prefixes.
 * 	Split mips_machine_state off of mips_kernel_state.
 * 	[90/12/30            rpd]
 * 	Added PCB_SIZE.
 * 	[90/11/12            rpd]
 * 
 * Revision 2.5  90/08/27  22:08:16  dbg
 * 	Reflected back changes to PCB and saved status structures.
 * 	[90/08/20  10:28:37  af]
 * 
 * Revision 2.4  90/08/07  22:29:12  rpd
 * 	Added support for pmap_pcache.
 * 	[90/08/07  15:24:54  af]
 * 
 * Revision 2.2.3.1  90/05/30  16:48:16  af
 * 	Added support for pmap_pcache.
 * 
 * Revision 2.3  90/06/02  15:02:08  rpd
 * 	Removed include of confdep.h.
 * 	[90/03/26  22:48:54  rpd]
 * 
 * Revision 2.2  89/11/29  14:13:00  af
 * 	Rewrote for pure kernel.
 * 	[89/10/04            af]
 * 
 * Revision 2.1  89/05/30  12:55:39  rvb
 * Created.
 * 
 * 20-Apr-89  Alessandro Forin (af) at Carnegie-Mellon University
 *	Thrown away MACH_TIME and CNT_TLBMISS.
 *
 * 12-Jan-89  Alessandro Forin (af) at Carnegie-Mellon University
 *	Created, from Vax sources.
 */
/*
 *	File:	genassym.c
 *	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	1989
 *
 *	File defines numerical values for various structure
 *	offsets needed by locore. The concept of such a file
 *	derives from 4.3BSD.
 */


#include <mach/mips/thread_status.h>
#include <kern/thread.h>
#include <kern/task.h>
#include <kern/syscall_emulation.h>
#include <vm/vm_map.h>
#include <mips/pmap.h>
#include <mips/context.h>
#include <mips/thread.h>

main()
{
	struct thread *thread = (struct thread *) 0;
	struct task *task = (struct task *) 0;
	struct vm_map *vm_map = (struct vm_map *) 0;
	struct pmap *pmap = (struct pmap *) 0;
	struct mips_kernel_state *mks = (struct mips_kernel_state *)0;
	struct mips_saved_state *mss = (struct mips_saved_state *)0;
	struct mips_exception_link *link = (struct mips_exception_link *)0;
	struct mips_machine_state *mms = (struct mips_machine_state *)0;
	struct mips_float_state *mfs = (struct mips_float_state *)0;
	struct mips_stack_base *mbs = (struct mips_stack_base *)0;
	jmp_buf *jmp = (jmp_buf *)0;
	struct eml_dispatch *disp = (struct eml_dispatch *)0;

	printf("#ifdef ASSEMBLER\n");
	printf("#define\tNBPW %d\n", sizeof(int));

	printf("#define\tPCB_SIZE %d\n", sizeof(struct pcb));

	printf("#define\tMKS_SIZE %d\n", sizeof *mks);
	printf("#define\tMKS_S0 %d\n", &mks->s0);
	printf("#define\tMKS_S1 %d\n", &mks->s1);
	printf("#define\tMKS_S2 %d\n", &mks->s2);
	printf("#define\tMKS_S3 %d\n", &mks->s3);
	printf("#define\tMKS_S4 %d\n", &mks->s4);
	printf("#define\tMKS_S5 %d\n", &mks->s5);
	printf("#define\tMKS_S6 %d\n", &mks->s6);
	printf("#define\tMKS_S7 %d\n", &mks->s7);
	printf("#define\tMKS_SP %d\n", &mks->sp);
	printf("#define\tMKS_FP %d\n", &mks->fp);
	printf("#define\tMKS_PC %d\n", &mks->pc);

	printf("#define\tMMS_SIZE %d\n", sizeof *mms);
	printf("#define\tMMS_MFS %d\n", &mms->mfs);
	printf("#define\tMMS_MSSS %d\n", &mms->msss);

	printf("#define\tMFS_SIZE %d\n", sizeof *mfs);
	printf("#define\tMFS_REGS %d\n", &mfs->r0);
	printf("#define\tMFS_CSR %d\n", &mfs->csr);
	printf("#define\tMFS_EIR %d\n", &mfs->eir);

	printf("#define\tMSB_SIZE %d\n", sizeof *mbs);
	printf("#define\tMSB_NEXT %d\n", &mbs->next);
	printf("#define\tMSB_PAGE %d\n", &mbs->page);

	printf("#define\tMSS_SIZE %d\n", sizeof *mss);
	printf("#define\tMSS_TLB_LO %d\n", &mss->tlb_low);
	printf("#define\tMSS_TLB_HI %d\n", &mss->tlb_high);
	printf("#define\tMSS_TLB_INX %d\n", &mss->tlb_index);
	printf("#define\tMSS_TLB_CTX %d\n", &mss->tlb_context);
	printf("#define\tMSS_AT %d\n", &mss->at);
	printf("#define\tMSS_V0 %d\n", &mss->v0);
	printf("#define\tMSS_V1 %d\n", &mss->v1);
	printf("#define\tMSS_A0 %d\n", &mss->a0);
	printf("#define\tMSS_A1 %d\n", &mss->a1);
	printf("#define\tMSS_A2 %d\n", &mss->a2);
	printf("#define\tMSS_A3 %d\n", &mss->a3);
	printf("#define\tMSS_T0 %d\n", &mss->t0);
	printf("#define\tMSS_T1 %d\n", &mss->t1);
	printf("#define\tMSS_T2 %d\n", &mss->t2);
	printf("#define\tMSS_T3 %d\n", &mss->t3);
	printf("#define\tMSS_T4 %d\n", &mss->t4);
	printf("#define\tMSS_T5 %d\n", &mss->t5);
	printf("#define\tMSS_T6 %d\n", &mss->t6);
	printf("#define\tMSS_T7 %d\n", &mss->t7);
	printf("#define\tMSS_S0 %d\n", &mss->s0);
	printf("#define\tMSS_S1 %d\n", &mss->s1);
	printf("#define\tMSS_S2 %d\n", &mss->s2);
	printf("#define\tMSS_S3 %d\n", &mss->s3);
	printf("#define\tMSS_S4 %d\n", &mss->s4);
	printf("#define\tMSS_S5 %d\n", &mss->s5);
	printf("#define\tMSS_S6 %d\n", &mss->s6);
	printf("#define\tMSS_S7 %d\n", &mss->s7);
	printf("#define\tMSS_T8 %d\n", &mss->t8);
	printf("#define\tMSS_T9 %d\n", &mss->t9);
	printf("#define\tMSS_K0 %d\n", &mss->k0);
	printf("#define\tMSS_K1 %d\n", &mss->k1);
	printf("#define\tMSS_GP %d\n", &mss->gp);
	printf("#define\tMSS_SP %d\n", &mss->sp);
	printf("#define\tMSS_FP %d\n", &mss->fp);
	printf("#define\tMSS_RA %d\n", &mss->ra);
	printf("#define\tMSS_SR %d\n", &mss->sr);
	printf("#define\tMSS_LO %d\n", &mss->mdlo);
	printf("#define\tMSS_HI %d\n", &mss->mdhi);
	printf("#define\tMSS_BAD %d\n", &mss->bad_address);
	printf("#define\tMSS_CAUSE %d\n", &mss->cause);
	printf("#define\tMSS_PC %d\n", &mss->pc);

	printf("#define\tMEL_SIZE %d\n", sizeof *link);
	printf("#define\tMEL_ARG0 %d\n", &link->arg0);
	printf("#define\tMEL_ARG1 %d\n", &link->arg1);
	printf("#define\tMEL_ARG2 %d\n", &link->arg2);
	printf("#define\tMEL_ARG3 %d\n", &link->arg3);
	printf("#define\tMEL_EFRAME %d\n", &link->eframe);

	printf("#define\tJB_SIZE %d\n", sizeof *jmp);
	printf("#define\tJB_S0 %d\n", &jmp->s0);
	printf("#define\tJB_S1 %d\n", &jmp->s1);
	printf("#define\tJB_S2 %d\n", &jmp->s2);
	printf("#define\tJB_S3 %d\n", &jmp->s3);
	printf("#define\tJB_S4 %d\n", &jmp->s4);
	printf("#define\tJB_S5 %d\n", &jmp->s5);
	printf("#define\tJB_S6 %d\n", &jmp->s6);
	printf("#define\tJB_S7 %d\n", &jmp->s7);
	printf("#define\tJB_SP %d\n", &jmp->sp);
	printf("#define\tJB_FP %d\n", &jmp->fp);
	printf("#define\tJB_PC %d\n", &jmp->pc);
	printf("#define\tJB_SR %d\n", &jmp->sr);

	printf("#define\tTHREAD_PCB %d\n", &thread->pcb);
	printf("#define\tTHREAD_RECOVER %d\n", &thread->recover);
	printf("#define\tTHREAD_TASK %d\n", &thread->task);
	printf("#define\tTHREAD_AST %d\n", &thread->ast);
	printf("#define\tTHREAD_KERNEL_STACK %d\n", &thread->kernel_stack);
	printf("#define\tTHREAD_SWAP_FUNC %d\n", &thread->swap_func);

	printf("#define\tTASK_MAP %d\n", &task->map);
	printf("#define\tMAP_PMAP %d\n", &vm_map->pmap);
	printf("#define\tPMAP_PID %d\n", &pmap->pid);
	printf("#define\tPMAP_PTEBASE %d\n", &pmap->ptebase);

	printf("#define\tEML_DISPATCH 0x%x\n", &task->eml_dispatch);
	printf("#define\tDISP_MIN 0x%x\n", &disp->disp_min);
	printf("#define\tDISP_COUNT 0x%x\n", &disp->disp_count);
	printf("#define\tDISP_VECTOR 0x%x\n", &disp->disp_vector[0]);

	printf("#endif ASSEMBLER\n");
	exit (0);
}
