/* 
 * Mach Operating System
 * Copyright (c) 1993,1992 Carnegie Mellon University
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
 * Revision 2.5  93/02/05  08:18:25  danner
 * 	Version 2.
 * 
 * Revision 2.4  93/02/04  07:55:13  danner
 * 	Fixpri fixes.
 *
 * Revision 2.3  93/01/19  08:59:27  danner
 * 	Locks are now 64bits.
 * 	[92/12/30            af]
 * 
 * Revision 2.2  93/01/14  17:13:12  danner
 * 	Created.
 * 	[92/05/31            af]
 * 
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

#include <cpus.h>
#include <mach_fixpri.h>

#include <mach/std_types.h>
#include <mach/alpha/thread_status.h>
#include <kern/thread.h>
#include <kern/task.h>
#include <kern/syscall_emulation.h>
#include <vm/vm_map.h>
#include <alpha/pmap.h>
#include <alpha/context.h>
#include <alpha/thread.h>
#include <alpha/prom_interface.h>

main()
{
	struct thread *thread = (struct thread *) 0;
	struct task *task = (struct task *) 0;
	struct vm_map *vm_map = (struct vm_map *) 0;
	struct pmap *pmap = (struct pmap *) 0;
	struct alpha_kernel_state *alpha_ks = (struct alpha_kernel_state *)0;
	struct alpha_saved_state *alpha_ss = (struct alpha_saved_state *)0;
	struct alpha_exception_link *link = (struct alpha_exception_link *)0;
	struct alpha_machine_state *alpha_ms = (struct alpha_machine_state *)0;
	struct alpha_float_state *alpha_fs = (struct alpha_float_state *)0;
	struct alpha_stack_base *alpha_sb = (struct alpha_stack_base *)0;
	struct trap_frame *trap = (struct trap_frame *)0;
	jmp_buf *jmp = (jmp_buf *)0;
	struct eml_dispatch *disp = (struct eml_dispatch *)0;
	struct restart_blk *rst = (struct restart_blk *)0;

	printf("#ifdef ASSEMBLER\n");

	/*
	 * This part of the file is when compiling native.
	 */
	printf("#if (%d==8)\n", sizeof(vm_offset_t));

	printf("#define\tNBPW %d\n", sizeof(vm_offset_t));

	printf("#define\tPCB_SIZE %d\n", sizeof(struct pcb));

	printf("#define\tMKS_SIZE %d\n", sizeof *alpha_ks);
	printf("#define\tMKS_S0 %d\n", &alpha_ks->s0);
	printf("#define\tMKS_S1 %d\n", &alpha_ks->s1);
	printf("#define\tMKS_S2 %d\n", &alpha_ks->s2);
	printf("#define\tMKS_S3 %d\n", &alpha_ks->s3);
	printf("#define\tMKS_S4 %d\n", &alpha_ks->s4);
	printf("#define\tMKS_S5 %d\n", &alpha_ks->s5);
	printf("#define\tMKS_S6 %d\n", &alpha_ks->s6);
	printf("#define\tMKS_SP %d\n", &alpha_ks->sp);
	printf("#define\tMKS_PC %d\n", &alpha_ks->pc);

	printf("#define\tMMS_SIZE %d\n", sizeof *alpha_ms);
	printf("#define\tMMS_MFS %d\n", &alpha_ms->mfs);
	printf("#define\tMMS_MSSS %d\n", &alpha_ms->msss);

	printf("#define\tMFS_SIZE %d\n", sizeof *alpha_fs);
	printf("#define\tMFS_REGS %d\n", &alpha_fs->r0);
	printf("#define\tMFS_CSR %d\n", &alpha_fs->csr);

	printf("#define\tMSB_SIZE %d\n", sizeof *alpha_sb);
	printf("#define\tMSB_NEXT %d\n", &alpha_sb->next);
	printf("#define\tMSB_PAGE %d\n", &alpha_sb->page);
	printf("#define\tMSB_PCB %d\n", &alpha_sb->pcb);

	/* comistorical note:
	 * This would have been ASS_SIZE, so 'M' is for Machine
	 */
	printf("#define\tMSS_SIZE %d\n", sizeof *alpha_ss);
	printf("#define\tMSS_KSP %d\n", &alpha_ss->hw_pcb.ksp);
	printf("#define\tMSS_USP %d\n", &alpha_ss->hw_pcb.usp);
	printf("#define\tMSS_ASN %d\n", &alpha_ss->hw_pcb.asn);
	printf("#define\tMSS_FRAMEP %d\n", &alpha_ss->framep);
	printf("#define\tMSS_V0 %d\n", &alpha_ss->v0);
	printf("#define\tMSS_T0 %d\n", &alpha_ss->t0);
	printf("#define\tMSS_T7 %d\n", &alpha_ss->t7);
	printf("#define\tMSS_S0 %d\n", &alpha_ss->s0);
	printf("#define\tMSS_S1 %d\n", &alpha_ss->s1);
	printf("#define\tMSS_S2 %d\n", &alpha_ss->s2);
	printf("#define\tMSS_S3 %d\n", &alpha_ss->s3);
	printf("#define\tMSS_S4 %d\n", &alpha_ss->s4);
	printf("#define\tMSS_S5 %d\n", &alpha_ss->s5);
	printf("#define\tMSS_S6 %d\n", &alpha_ss->s6);
	printf("#define\tMSS_A0 %d\n", &alpha_ss->a0);
	printf("#define\tMSS_A1 %d\n", &alpha_ss->a1);
	printf("#define\tMSS_A2 %d\n", &alpha_ss->a2);
	printf("#define\tMSS_A3 %d\n", &alpha_ss->a3);
	printf("#define\tMSS_A4 %d\n", &alpha_ss->a4);
	printf("#define\tMSS_A5 %d\n", &alpha_ss->a5);
	printf("#define\tMSS_T8 %d\n", &alpha_ss->t8);
	printf("#define\tMSS_T9 %d\n", &alpha_ss->t9);
	printf("#define\tMSS_T10 %d\n", &alpha_ss->t10);
	printf("#define\tMSS_T11 %d\n", &alpha_ss->t11);
	printf("#define\tMSS_RA %d\n", &alpha_ss->ra);
	printf("#define\tMSS_T12 %d\n", &alpha_ss->t12);
	printf("#define\tMSS_AT %d\n", &alpha_ss->at);
	printf("#define\tMSS_GP %d\n", &alpha_ss->gp);
	printf("#define\tMSS_SP %d\n", &alpha_ss->sp);
	printf("#define\tMSS_BAD %d\n", &alpha_ss->bad_address);
	printf("#define\tMSS_CAUSE %d\n", &alpha_ss->cause);
	printf("#define\tMSS_SAVEDF %d\n", &alpha_ss->saved_frame);
	printf("#define\tMSS_T1 %d\n", &alpha_ss->saved_frame.saved_r2);
	printf("#define\tMSS_T2 %d\n", &alpha_ss->saved_frame.saved_r3);
	printf("#define\tMSS_T3 %d\n", &alpha_ss->saved_frame.saved_r4);
	printf("#define\tMSS_T4 %d\n", &alpha_ss->saved_frame.saved_r5);
	printf("#define\tMSS_T5 %d\n", &alpha_ss->saved_frame.saved_r6);
	printf("#define\tMSS_T6 %d\n", &alpha_ss->saved_frame.saved_r7);
	printf("#define\tMSS_PC %d\n", &alpha_ss->saved_frame.saved_pc);
	printf("#define\tMSS_PS %d\n", &alpha_ss->saved_frame.saved_ps);

	printf("#define\tMEL_SIZE %d\n", sizeof *link);
	printf("#define\tMEL_EFRAME %d\n", &link->eframe);
	printf("#define\tMEL_TF %d\n", &link->tf);

	printf("#define\tTF_SIZE %d\n", sizeof *trap);
	printf("#define\tTF_R2 %d\n", &trap->saved_r2);
	printf("#define\tTF_R3 %d\n", &trap->saved_r3);
	printf("#define\tTF_R4 %d\n", &trap->saved_r4);
	printf("#define\tTF_R5 %d\n", &trap->saved_r5);
	printf("#define\tTF_R6 %d\n", &trap->saved_r6);
	printf("#define\tTF_R7 %d\n", &trap->saved_r7);
	printf("#define\tTF_PC %d\n", &trap->saved_pc);
	printf("#define\tTF_PS %d\n", &trap->saved_ps);

	printf("#define\tJB_SIZE %d\n", sizeof *jmp);
	printf("#define\tJB_S0 %d\n", &jmp->s0);
	printf("#define\tJB_S1 %d\n", &jmp->s1);
	printf("#define\tJB_S2 %d\n", &jmp->s2);
	printf("#define\tJB_S3 %d\n", &jmp->s3);
	printf("#define\tJB_S4 %d\n", &jmp->s4);
	printf("#define\tJB_S5 %d\n", &jmp->s5);
	printf("#define\tJB_S6 %d\n", &jmp->s6);
	printf("#define\tJB_SP %d\n", &jmp->sp);
	printf("#define\tJB_PC %d\n", &jmp->pc);
	printf("#define\tJB_PS %d\n", &jmp->ps);

	printf("#define\tTHREAD_TASK %d\n", &thread->task);
	printf("#define\tTHREAD_PCB %d\n", &thread->pcb);
	printf("#define\tTHREAD_KERNEL_STACK %d\n", &thread->kernel_stack);
	printf("#define\tTHREAD_SWAP_FUNC %d\n", &thread->swap_func);
	printf("#define\tTHREAD_RECOVER %d\n", &thread->recover);

	printf("#define\tPMAP_PID %d\n", &pmap->pid);

	printf("#define\tMAP_PMAP %d\n", &vm_map->pmap);

	printf("#define\tTASK_MAP %d\n", &task->map);
	printf("#define\tEML_DISPATCH 0x%x\n", &task->eml_dispatch);

	printf("#define\tDISP_COUNT 0x%x\n", &disp->disp_count);
	printf("#define\tDISP_MIN 0x%x\n", &disp->disp_min);
	printf("#define\tDISP_VECTOR 0x%x\n", &disp->disp_vector[0]);

	printf("#define\tHWRPB_PRIMARY_ID 0x%x\n", &rst->primary_cpu_id);

	/*
	 * This part of the file is when cross-compiling.
	 * A lot ugly, but goes away when port complete.
	 */
	printf("#endif /* (%d==8) */\n", sizeof(vm_offset_t));
	printf("#if (%d==4)\n", sizeof(vm_offset_t));

	printf("#define\tNBPW %d\n", sizeof(vm_offset_t) * 2);

#if notbecausepaddedexplicitly
	printf("#define\tPCB_SIZE %d\n", sizeof(struct pcb) * 2);
#else
	printf("#define\tPCB_SIZE %d\n", sizeof(struct pcb));
#endif

	printf("#define\tMKS_SIZE %d\n", (sizeof *alpha_ks) * 2);
	printf("#define\tMKS_S0 %d\n", ((unsigned long)&alpha_ks->s0) * 2);
	printf("#define\tMKS_S1 %d\n", ((unsigned long)&alpha_ks->s1) * 2);
	printf("#define\tMKS_S2 %d\n", ((unsigned long)&alpha_ks->s2) * 2);
	printf("#define\tMKS_S3 %d\n", ((unsigned long)&alpha_ks->s3) * 2);
	printf("#define\tMKS_S4 %d\n", ((unsigned long)&alpha_ks->s4) * 2);
	printf("#define\tMKS_S5 %d\n", ((unsigned long)&alpha_ks->s5) * 2);
	printf("#define\tMKS_S6 %d\n", ((unsigned long)&alpha_ks->s6) * 2);
	printf("#define\tMKS_SP %d\n", ((unsigned long)&alpha_ks->sp) * 2);
	printf("#define\tMKS_PC %d\n", ((unsigned long)&alpha_ks->pc) * 2);

	printf("#define\tMMS_SIZE %d\n",(sizeof *alpha_ms) * 2);
	printf("#define\tMMS_MFS %d\n", ((unsigned long)&alpha_ms->mfs) * 2);
	printf("#define\tMMS_MSSS %d\n", ((unsigned long)&alpha_ms->msss) * 2);

	printf("#define\tMFS_SIZE %d\n", (sizeof *alpha_fs) * 2);
	printf("#define\tMFS_REGS %d\n", ((unsigned long)&alpha_fs->r0) * 2);
	printf("#define\tMFS_CSR %d\n", ((unsigned long)&alpha_fs->csr) * 2);

	printf("#define\tMSB_SIZE %d\n", (sizeof *alpha_sb) /* * 2 padded*/);
	printf("#define\tMSB_NEXT %d\n", ((unsigned long)&alpha_sb->next) * 2);
	printf("#define\tMSB_PAGE %d\n", ((unsigned long)&alpha_sb->page) * 2);
	printf("#define\tMSB_PCB %d\n", ((unsigned long)&alpha_sb->pcb) * 2);

	printf("#define\tMSS_SIZE %d\n", (sizeof *alpha_ss) * 2);
	printf("#define\tMSS_KSP %d\n", ((unsigned long)&alpha_ss->hw_pcb.ksp)*2);
	printf("#define\tMSS_USP %d\n", ((unsigned long)&alpha_ss->hw_pcb.usp)*2);
	printf("#define\tMSS_ASN %d\n", ((unsigned long)&alpha_ss->hw_pcb.asn)*2);
	printf("#define\tMSS_FRAMEP %d\n", ((unsigned long)&alpha_ss->framep)*2);
	printf("#define\tMSS_V0 %d\n", ((unsigned long)&alpha_ss->v0) * 2);
	printf("#define\tMSS_T0 %d\n", ((unsigned long)&alpha_ss->t0) * 2);
	printf("#define\tMSS_T7 %d\n", ((unsigned long)&alpha_ss->t7) * 2);
	printf("#define\tMSS_S0 %d\n", ((unsigned long)&alpha_ss->s0) * 2);
	printf("#define\tMSS_S1 %d\n", ((unsigned long)&alpha_ss->s1) * 2);
	printf("#define\tMSS_S2 %d\n", ((unsigned long)&alpha_ss->s2) * 2);
	printf("#define\tMSS_S3 %d\n", ((unsigned long)&alpha_ss->s3) * 2);
	printf("#define\tMSS_S4 %d\n", ((unsigned long)&alpha_ss->s4) * 2);
	printf("#define\tMSS_S5 %d\n", ((unsigned long)&alpha_ss->s5) * 2);
	printf("#define\tMSS_S6 %d\n", ((unsigned long)&alpha_ss->s6) * 2);
	printf("#define\tMSS_A0 %d\n", ((unsigned long)&alpha_ss->a0) * 2);
	printf("#define\tMSS_A1 %d\n", ((unsigned long)&alpha_ss->a1) * 2);
	printf("#define\tMSS_A2 %d\n", ((unsigned long)&alpha_ss->a2) * 2);
	printf("#define\tMSS_A3 %d\n", ((unsigned long)&alpha_ss->a3) * 2);
	printf("#define\tMSS_A4 %d\n", ((unsigned long)&alpha_ss->a4) * 2);
	printf("#define\tMSS_A5 %d\n", ((unsigned long)&alpha_ss->a5) * 2);
	printf("#define\tMSS_T8 %d\n", ((unsigned long)&alpha_ss->t8) * 2);
	printf("#define\tMSS_T9 %d\n", ((unsigned long)&alpha_ss->t9) * 2);
	printf("#define\tMSS_T10 %d\n", ((unsigned long)&alpha_ss->t10) * 2);
	printf("#define\tMSS_T11 %d\n", ((unsigned long)&alpha_ss->t11) * 2);
	printf("#define\tMSS_RA %d\n", ((unsigned long)&alpha_ss->ra) * 2);
	printf("#define\tMSS_T12 %d\n", ((unsigned long)&alpha_ss->t12) * 2);
	printf("#define\tMSS_AT %d\n", ((unsigned long)&alpha_ss->at) * 2);
	printf("#define\tMSS_GP %d\n", ((unsigned long)&alpha_ss->gp) * 2);
	printf("#define\tMSS_SP %d\n", ((unsigned long)&alpha_ss->sp) * 2);
	printf("#define\tMSS_BAD %d\n", ((unsigned long)&alpha_ss->bad_address) * 2);
	printf("#define\tMSS_CAUSE %d\n", ((unsigned long)&alpha_ss->cause) * 2);
	printf("#define\tMSS_SAVEDF %d\n", ((unsigned long)&alpha_ss->saved_frame)*2);
	printf("#define\tMSS_T1 %d\n", ((unsigned long)&alpha_ss->saved_frame.saved_r2)*2);
	printf("#define\tMSS_T2 %d\n", ((unsigned long)&alpha_ss->saved_frame.saved_r3)*2);
	printf("#define\tMSS_T3 %d\n", ((unsigned long)&alpha_ss->saved_frame.saved_r4)*2);
	printf("#define\tMSS_T4 %d\n", ((unsigned long)&alpha_ss->saved_frame.saved_r5)*2);
	printf("#define\tMSS_T5 %d\n", ((unsigned long)&alpha_ss->saved_frame.saved_r6)*2);
	printf("#define\tMSS_T6 %d\n", ((unsigned long)&alpha_ss->saved_frame.saved_r7)*2);
	printf("#define\tMSS_PC %d\n", ((unsigned long)&alpha_ss->saved_frame.saved_pc)*2);
	printf("#define\tMSS_PS %d\n", ((unsigned long)&alpha_ss->saved_frame.saved_ps)*2);

	printf("#define\tMEL_SIZE %d\n", (sizeof *link) * 2);
	printf("#define\tMEL_EFRAME %d\n", ((unsigned long)&link->eframe) * 2);
	printf("#define\tMEL_TF %d\n", ((unsigned long)&link->tf) * 2);

	printf("#define\tTF_SIZE %d\n", (sizeof *trap) * 2);
	printf("#define\tTF_R2 %d\n", ((unsigned long)&trap->saved_r2) * 2);
	printf("#define\tTF_R3 %d\n", ((unsigned long)&trap->saved_r3) * 2);
	printf("#define\tTF_R4 %d\n", ((unsigned long)&trap->saved_r4) * 2);
	printf("#define\tTF_R5 %d\n", ((unsigned long)&trap->saved_r5) * 2);
	printf("#define\tTF_R6 %d\n", ((unsigned long)&trap->saved_r6) * 2);
	printf("#define\tTF_R7 %d\n", ((unsigned long)&trap->saved_r7) * 2);
	printf("#define\tTF_PC %d\n", ((unsigned long)&trap->saved_pc) * 2);
	printf("#define\tTF_PS %d\n", ((unsigned long)&trap->saved_ps) * 2);

	printf("#define\tJB_SIZE %d\n", (sizeof *jmp) * 2);
	printf("#define\tJB_S0 %d\n", ((unsigned long)&jmp->s0) * 2);
	printf("#define\tJB_S1 %d\n", ((unsigned long)&jmp->s1) * 2);
	printf("#define\tJB_S2 %d\n", ((unsigned long)&jmp->s2) * 2);
	printf("#define\tJB_S3 %d\n", ((unsigned long)&jmp->s3) * 2);
	printf("#define\tJB_S4 %d\n", ((unsigned long)&jmp->s4) * 2);
	printf("#define\tJB_S5 %d\n", ((unsigned long)&jmp->s5) * 2);
	printf("#define\tJB_S6 %d\n", ((unsigned long)&jmp->s6) * 2);
	printf("#define\tJB_SP %d\n", ((unsigned long)&jmp->sp) * 2);
	printf("#define\tJB_PC %d\n", ((unsigned long)&jmp->pc) * 2);
	printf("#define\tJB_PS %d\n", ((unsigned long)&jmp->ps) * 2);

	/* The rest of it is a much tougher call. */
#define check(x,t,v)						\
	if ((sizeof(vm_offset_t) == 4) && (x) != (t *)(v)) {	\
		printf("Fix genassym and try again\n");		\
		printf("because %d != %d\n", x, v);		\
		exit(1);					\
	} else

	check(&thread->task,task_t,(8+4))
	printf("#define\tTHREAD_TASK %d\n", (16+8));
#if	(NCPUS>1)
	check(&thread->pcb,pcb_t,(8+4+4+8+8+4+4))
	printf("#define\tTHREAD_PCB %d\n", 16+8+8+16+16+8+4+4/*align*/);
	check(&thread->kernel_stack,vm_offset_t,(8+4+4+8+8+4+4+4))
	printf("#define\tTHREAD_KERNEL_STACK %d\n",
		16+8+8+16+16+8+8 +8);
	/* aurgh */
	check((char*)&thread->swap_func,char,(8+4+4+8+8+4+4 +4+4+4))
	printf("#define\tTHREAD_SWAP_FUNC %d\n",
		16+8+8+16+16+8+8 +8 +8+8);
	check(&thread->recover,vm_offset_t,(8+4+4+8+8+4+4 +4+4+4 +4+4+4+4+4+4+4+4
		+(MACH_FIXPRI*8) +4+4+4+4+4))
	printf("#define\tTHREAD_RECOVER %d\n",
		16+8+8+16+16+8+8 +8+8+8+8 +8+4+4+4+4+4+4+4 +(MACH_FIXPRI*8) +4+4+4+8/*align*/);
#else
	check(&thread->pcb,pcb_t,(8+4+4+8+8+4))
	printf("#define\tTHREAD_PCB %d\n", 16+8+8+16+16+8/*align*/);
	check(&thread->kernel_stack,vm_offset_t,(8+4+4+8+8+4+4))
	printf("#define\tTHREAD_KERNEL_STACK %d\n",
		16+8+8+16+16+8/*align*/+8);
	/* aurgh */
	check((char*)&thread->swap_func,char,(8+4+4+8+8+4+4+4+4))
	printf("#define\tTHREAD_SWAP_FUNC %d\n",
		16+8+8+16+16+8/*align*/ +8 +8+8);
	check(&thread->recover,vm_offset_t,(8+4+4+8+8+4 +4+4+4 +4+4+4+4+4+4+4+4
		+(MACH_FIXPRI*8) +4+4+4+4+4))
	printf("#define\tTHREAD_RECOVER %d\n",
		16+8+8+16+16+8/*align*/+8+8+8+8 +8+4+4+4+4+4+4+4  +(MACH_FIXPRI*8)
		+4+4+4+8/*align*/);
#endif

	check(&pmap->pid,int,4)
	printf("#define\tPMAP_PID %d\n", 8);

#if	(NCPUS>1)
	check(&vm_map->pmap,pmap_t,4+4+4 +4+4+4+4 +4+4)
	printf("#define\tMAP_PMAP %d\n", 8+8/*align*/+8 +8+8+8+8 +4+4);
#else
	check(&vm_map->pmap,pmap_t,4+4 +4+4+4+4 +4+4)
	printf("#define\tMAP_PMAP %d\n", 8+8/*align*/ +8+8+8+8 +4+4);
#endif

#if	(NCPUS>1)
	check(&task->map,vm_map_t, 4+4+4)
	printf("#define\tTASK_MAP %d\n", 8+4+4);
	check(&task->eml_dispatch,struct eml_dispatch *, 4+4+4 +4+8+4 +4+8+4+4+4 +4+4 +8+8 +4+4+4+4+4+ (4*TASK_PORT_REGISTER_MAX) +4)
	printf("#define\tEML_DISPATCH 0x%x\n", 8+4+4 +8+16+4 +4+16+8+4+4 +4+4 +16+16 +8+8+8+8+8+ (8*TASK_PORT_REGISTER_MAX) +8);
#else
	check(&task->map,vm_map_t, 4+4)
	printf("#define\tTASK_MAP %d\n", 4+4);
	check(&task->eml_dispatch,struct eml_dispatch *, 4+4 +4+8+4+4+8+4+4+4+4+4+8+8+4+4+4+4+ (4*TASK_PORT_REGISTER_MAX) +4)
	printf("#define\tEML_DISPATCH 0x%x\n", 4+4 +8+16+4+4+16+8+4+4+4+4+16+16+8+8+8+8+ (8*TASK_PORT_REGISTER_MAX) +8);
#endif

#if	(NCPUS>1)
	check(&disp->disp_count,int,4+4)
	printf("#define\tDISP_COUNT 0x%x\n", 8+4);
	check(&disp->disp_min,int,4+4+4)
	printf("#define\tDISP_MIN 0x%x\n", 8+4+4);
	check(&disp->disp_vector[0],eml_routine_t,4+4+4+4)
	printf("#define\tDISP_VECTOR 0x%x\n", 8+4+4+4);
#else
	check(&disp->disp_count,int,4)
	printf("#define\tDISP_COUNT 0x%x\n", 4);
	check(&disp->disp_min,int,4+4)
	printf("#define\tDISP_MIN 0x%x\n", 4+4);
	check(&disp->disp_vector[0],eml_routine_t,4+4+4)
	printf("#define\tDISP_VECTOR 0x%x\n", 4+4+8/*align*/);
#endif

	check(&rst->primary_cpu_id,natural_t,4+8+4+4)
	printf("#define\tHWRPB_PRIMARY_ID 0x%x\n", 8+8+8+8);

	printf("#endif (%d==4)\n", sizeof(vm_offset_t));

	printf("#endif /* ASSEMBLER */\n");
	exit (0);
}
