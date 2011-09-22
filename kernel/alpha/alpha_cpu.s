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
 * $Log:	alpha_cpu.s,v $
 * Revision 2.2  93/02/05  07:57:18  danner
 * 	Change mov inst. to or
 * 	[93/01/12            jeffreyh]
 * 	Added ops on MCES.
 * 	[93/01/15            af]
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:10:52  af]
 * 
 * 	Created.
 * 	[92/06/02            af]
 * 
 */
/*
 *	File: alpha_cpu.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	CPU related miscellaneous operations
 *	Includes common operations on the status register (spl)
 *	and other CPU registers, and the FPA registers.
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 */
#include <cpus.h>


#include <mach/alpha/asm.h>
#include <alpha/alpha_cpu.h>
#include <mach/alpha/vm_param.h>
#include <mach/alpha/alpha_instruction.h>
#include <alpha/thread.h>
#include <mach/exception.h>

#include <assym.s>

	.set	noreorder

/*
 *	Object:
 *		wbflush				EXPORTED function
 *
 *		Wait for writes to complete
 *
 *	Arguments:
 *		none
 */
LEAF(wbflush,0)
	or	s0,zero,t1
	or	s1,zero,t2
	mb
	or	t1,zero,s0
	or	t2,zero,s1
	RET
	END(wbflush)


/*
 *	Object:
 *		alpha_fpa_unload		EXPORTED function
 *
 *		Checkpoint FPA status.
 *
 *	Arguments:
 *		state				alpha_float_state *
 *
 *	Assumes FPA *is* currently usable
 */
LEAF(alpha_fpa_unload,1)

	trapb				/* flush fpa pipes */

	stt	$f30,MFS_REGS+30*8(a0)
	mf_fpcr	$f30
	stt	$f29, MFS_REGS+29*8(a0)
	stt	$f28, MFS_REGS+28*8(a0)
	stt	$f27, MFS_REGS+27*8(a0)
	stt	$f30, MFS_CSR(a0)	/* r31, that is */
	stt	$f26, MFS_REGS+26*8(a0)
	stt	$f25, MFS_REGS+25*8(a0)
	stt	$f24, MFS_REGS+24*8(a0)
	stt	$f23, MFS_REGS+23*8(a0)
	stt	$f22, MFS_REGS+22*8(a0)
	stt	$f21, MFS_REGS+21*8(a0)
	stt	$f20, MFS_REGS+20*8(a0)
	stt	$f19, MFS_REGS+19*8(a0)
	stt	$f18, MFS_REGS+18*8(a0)
	stt	$f17, MFS_REGS+17*8(a0)
	stt	$f16, MFS_REGS+16*8(a0)
	stt	$f15, MFS_REGS+15*8(a0)
	stt	$f14, MFS_REGS+14*8(a0)
	stt	$f13, MFS_REGS+13*8(a0)
	stt	$f12, MFS_REGS+12*8(a0)
	stt	$f11, MFS_REGS+11*8(a0)
	stt	$f10, MFS_REGS+10*8(a0)
	stt	$f9,  MFS_REGS+ 9*8(a0)
	stt	$f8,  MFS_REGS+ 8*8(a0)
	stt	$f7,  MFS_REGS+ 7*8(a0)
	stt	$f6,  MFS_REGS+ 6*8(a0)
	stt	$f5,  MFS_REGS+ 5*8(a0)
	stt	$f4,  MFS_REGS+ 4*8(a0)
	stt	$f3,  MFS_REGS+ 3*8(a0)
	stt	$f2,  MFS_REGS+ 2*8(a0)
	stt	$f1,  MFS_REGS+ 1*8(a0)
	stt	$f0,  MFS_REGS+ 0*8(a0)

	/* disable fpa */
	or	zero,zero,a0
	call_pal op_mtpr_fen

	RET
	END(alpha_fpa_unload)

/*
 *	Object:
 *		alpha_fpa_loadup		EXPORTED function
 *
 *		Restore FPA status.
 *
 *	Arguments:
 *		state				alpha_float_state *
 *
 *	Assumes FPA *is* currently usable
 */
LEAF(alpha_fpa_loadup,1)

	/* enable fpa first */
	or	a0,zero,t4		/* safe from PAL code */
	addq	zero,1,a0
	call_pal op_mtpr_fen		/* enable fpa for the kernel */
	or	t4,zero,a0

	ldt	$f30, MFS_CSR(a0)	/* r31, that is */
	ldt	$f29, MFS_REGS+29*8(a0)
	ldt	$f28, MFS_REGS+28*8(a0)
	ldt	$f27, MFS_REGS+27*8(a0)
	mt_fpcr	$f30
	ldt	$f30, MFS_REGS+30*8(a0)
	ldt	$f26, MFS_REGS+26*8(a0)
	ldt	$f25, MFS_REGS+25*8(a0)
	ldt	$f24, MFS_REGS+24*8(a0)
	ldt	$f23, MFS_REGS+23*8(a0)
	ldt	$f22, MFS_REGS+22*8(a0)
	ldt	$f21, MFS_REGS+21*8(a0)
	ldt	$f20, MFS_REGS+20*8(a0)
	ldt	$f19, MFS_REGS+19*8(a0)
	ldt	$f18, MFS_REGS+18*8(a0)
	ldt	$f17, MFS_REGS+17*8(a0)
	ldt	$f16, MFS_REGS+16*8(a0)
	ldt	$f15, MFS_REGS+15*8(a0)
	ldt	$f14, MFS_REGS+14*8(a0)
	ldt	$f13, MFS_REGS+13*8(a0)
	ldt	$f12, MFS_REGS+12*8(a0)
	ldt	$f11, MFS_REGS+11*8(a0)
	ldt	$f10, MFS_REGS+10*8(a0)
	ldt	$f9,  MFS_REGS+ 9*8(a0)
	ldt	$f8,  MFS_REGS+ 8*8(a0)
	ldt	$f7,  MFS_REGS+ 7*8(a0)
	ldt	$f6,  MFS_REGS+ 6*8(a0)
	ldt	$f5,  MFS_REGS+ 5*8(a0)
	ldt	$f4,  MFS_REGS+ 4*8(a0)
	ldt	$f3,  MFS_REGS+ 3*8(a0)
	ldt	$f2,  MFS_REGS+ 2*8(a0)
	ldt	$f1,  MFS_REGS+ 1*8(a0)
	ldt	$f0,  MFS_REGS+ 0*8(a0)

	RET
	END(alpha_fpa_loadup)

/*
 *	Object:
 *		mtpr_fen			EXPORTED function
 *
 *		Brutally enable/disable fpa usage.
 *
 *	Arguments:
 *		a0				boolean_t
 */
LEAF(mtpr_fen,1)
	call_pal op_mtpr_fen		/* enable fpa for the kernel */
	RET
	END(simple_splx)

LEAF(mtpr_usp,1)
	call_pal op_mtpr_usp
	RET
	END(mtpr_usp)
LEAF(mfpr_usp,0)
	call_pal op_mfpr_usp
	RET
	END(mtpr_usp)

LEAF(mfpr_mces,0)
	call_pal op_mfpr_mces
	RET
	END(mfpr_mces)
LEAF(mtpr_mces,1)
	call_pal op_mtpr_mces
	RET
	END(mtpr_mces)

/*
 *	Object:
 *		alpha_swap_ipl			EXPORTED function
 *
 *		Change priority level, return current one
 *
 *	Arguments:
 *		a0				unsigned
 *
 *	Set priority level to the value in a0, returns the current
 *	priority level. Stright call to PAL code.
 */
LEAF(alpha_swap_ipl,1)
	call_pal	op_mtpr_ipl
	RET
	END(simple_splx)

/*
 *	Object:
 *		kdbsplhigh			EXPORTED function
 *
 *		Block all interrupts
 *
 *	Arguments:
 *		none
 *
 *	Returns the previous content of the status register
 *	[Separate from above to allow sstepping]
 *
 *	Object:
 *		kdbsplx				EXPORTED function
 *
 *		Restore priority level
 *
 *	Arguments:
 *		a0				unsigned
 *
 */
LEAF(kdbsplhigh,0)
	addq	zero,ALPHA_IPL_HIGH,a0
XLEAF(kdbsplx,1)
	call_pal	op_mtpr_ipl
	RET
	END(kdbsplhigh)

/*
 *	Object:
 *		setsoftclock			EXPORTED function
 *
 *		Schedule a software clock interrupt
 *
 *	Arguments:
 *		none
 *
 *	Software interrupts are generated by writing into the SIRR
 *	register.  HW clears this bit.
 *	In Mach only one software interrupt is used.
 */
LEAF(setsoftclock,0)
	addq	zero,1,a0
	call_pal op_mtpr_sirr
	RET
	END(setsoftclock)

/*
 *	Object:
 *		cpu_number		EXPORTED function
 *
 *		Return current processor number
 *
 *	Arguments:
 *		none
 *
 *	Use the internal Who-Am-I register.
 */
LEAF(cpu_number,0)
	call_pal op_mfpr_whami
	RET
	END(cpu_number)

/*
 *	Object:
 *		interrupt_processor	EXPORTED function
 *
 *		Send an interrupt to a processor
 *
 *	Arguments:
 *		procnum			int
 *
 */
LEAF(interrupt_processor,1)
	call_pal op_mtpr_ipir
	RET
	END(interrupt_processor)

/*
 *	Object:
 *		current_thread		EXPORTED function
 *
 *		Return current thread
 *
 *	Arguments:
 *		none
 *
 *	Use the internal processor-base register.
 */
#if 0
LEAF(current_thread,0)
	call_pal op_mfpr_prbr
	RET
	END(current_thread)
#endif
/*
 *	Object:
 *		set_current_thread	EXPORTED function
 *
 *		Set the current thread register
 *
 *	Arguments:
 *		thread			thread_t
 *
 *	Use the internal processor-base register.
 */
LEAF(set_current_thread,1)
	call_pal op_mtpr_prbr
	RET
	END(set_current_thread)


/*
 *	Object:
 *		swpctxt			EXPORTED function
 *
 *		Change HW process context
 *
 *	Arguments:
 *		pcb			PHYSICAL struct pcb_hw *
 *		old_pcb			VIRTUAL  struct pcb_hw *
 *
 *	Execute the PAL call.  If old_pcb is non-zero it saves
 *	the current KSP in it.
 */
LEAF(swpctxt,2)
	beq	a1,1f
	stq	sp,0(a1)
1:	call_pal op_swpctxt
	RET
	END(swpctxt)

/*
 *	Object:
 *		tbis			EXPORTED function
 *
 *		Invalidate TLB entry
 *
 *	Arguments:
 *		addr			unsigned long
 *
 */
LEAF(tbis,1)
	call_pal op_mtpr_tbis
	RET
	END(tbis)

/*
 *	Object:
 *		tbiap			EXPORTED function
 *
 *		Invalidate Process-owned TLB entries
 *
 *	Arguments:
 *		none
 *
 */
LEAF(tbiap,0)
	call_pal op_mtpr_tbiap
	RET
	END(tbiap)

/*
 *	Object:
 *		tbia			EXPORTED function
 *
 *		Invalidate all TLB entries
 *
 *	Arguments:
 *		none
 *
 */
LEAF(tbia,0)
	call_pal op_mtpr_tbia
	RET
	END(tbia)


/*
 *	Object:
 *		rpcc			EXPORTED function
 *
 *		Read process cycle counter
 *
 *	Arguments:
 *		none
 *
 */
LEAF(rpcc,0)
	rpcc	v0
	RET
	END(rpcc)

/*
 *	Object:
 *		delay			EXPORTED function
 *		machine_cycles_per_usec	EXPORTED unsigned long
 *
 *		Busy loop for a given number of microseconds
 *
 *	Arguments:
 *		usecs			unsigned long
 *
 *	I thought of using the internal Cycle Counter, but it
 *	is only 32 bits and the masking and overflow issues
 *	would take more than necessary.  Besides, that way
 *	you still do not get an absolute-time delay: you get
 *	screwed across ctxt switches (yes, the kernel is not
 *	pre-emptible right now, but) for instance.
 *	So let's do it as usual.
 */
	.data
	.globl	machine_cycles_per_usec
machine_cycles_per_usec:	.quad	0	/* needs init */

	.text
	.align 5
	.set	noat
LEAF(delay,1)

#define	FIXED_OVERHEAD	0x6c	/* measured on ADU @150 Mhz */
#define	LOOP_OVERHEAD	3	/* measured on ADU @150 Mhz */

	lda	v0,machine_cycles_per_usec
	ldq	v0,0(v0)
	mulq	a0,v0,a0	/* usecs->cycles */
	subq	a0,FIXED_OVERHEAD,a0
1:	subq	a0,LOOP_OVERHEAD,a0
	bgt	a0,1b
	RET

	END(delay)

	.align 5
LEAF(delay_overh,1)
	rpcc	t0

	lda	v0,machine_cycles_per_usec
	ldq	v0,0(v0)
	mulq	a0,v0,a0	/* usecs->cycles */
	subq	a0,FIXED_OVERHEAD,a0
1:	subq	a0,LOOP_OVERHEAD,a0
	bgt	a0,1b

	rpcc	v0
	rpcc	a1
	subq	v0,t0,t0
	subq	a1,v0,v0
	subq	t0,v0,v0
	RET
	END(delay_overh)
	
