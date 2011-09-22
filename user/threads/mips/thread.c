/* 
 * Mach Operating System
 * Copyright (c) 1993-1989 Carnegie Mellon University
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
 * $Log:	thread.c,v $
 * Revision 2.12  93/05/17  10:28:03  rvb
 * 	Type casts, etc to quiet gcc 2.3.3 warnings
 * 
 * Revision 2.11  93/05/14  15:10:36  rvb
 * 	#include string.h->strings.h; kill stdlib.h
 * 	[93/05/14            rvb]
 * 
 * Revision 2.10  93/02/04  14:55:54  mrt
 * 	The sins of the advisor are visited on the student. 
 * 	[93/02/04            pds]
 * 
 * Revision 2.9  93/02/02  21:55:16  mrt
 * 	Changed include of mach/mach.h to mach.h
 * 	[93/02/02            mrt]
 * 
 * Revision 2.8  93/01/21  12:28:19  danner
 * 	Use new interface for ras control
 * 	[93/01/19  16:38:25  bershad]
 * 
 * Revision 2.7  93/01/14  18:05:54  danner
 * 	Corrected type casts; bcopy -> memcpy.
 * 	[93/01/10            danner]
 * 	Converted file to ANSI C. 
 * 	Fixed argument types. 
 * 	[92/12/18            pds]
 * 
 * Revision 2.6  92/07/20  13:33:49  cmaeda
 * 	New initialization code for mutexes:
 * 	First try using an ras, then fall back on kernel-emulated tas.
 * 	[92/05/11  14:44:24  cmaeda]
 * 
 * Revision 2.5  91/05/14  17:58:25  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/14  14:20:59  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:39:03  mrt]
 * 
 * Revision 2.3  89/12/08  19:49:44  rwd
 * 	Changed to take an arbitrary starting routine and an arbitrary
 * 	thread, for use with rwd's new merged coroutine/thread kernel
 * 	implementation.  Removed conditionals.
 * 	[89/12/06            af]
 * 
 * Revision 2.2  89/11/29  14:19:12  af
 * 	Created.
 * 	[89/07/06            af]
 * 
 */
/*
 * mips/thread.c
 *
 * Cproc startup for MIPS Cthreads implementation.
 */


#include <mach/task_info.h>
#include <cthreads.h>
#include "cthread_internals.h"
#include <mach.h>
#include <strings.h>

/*
 * Set up the initial state of a MACH thread so that it will invoke
 * routine(child) when it is resumed.
 */
void
cproc_setup(register cproc_t child, thread_t thread, void (*routine)(cproc_t))
{
	register int *top = (int *) (child->stack_base + child->stack_size);
	struct mips_thread_state state;
	register struct mips_thread_state *ts = &state;
	kern_return_t r;
	extern int _gp;		/* ld(1) defines this */

	/*
	 * Set up MIPS call frame and registers.
	 * See MIPS Assembly Language Reference Book.
	 */
	memset(ts, 0, sizeof(struct mips_thread_state));

	/*
	 * Set pc to procedure entry, pass one arg in register,
	 * allocate the standard 4 regsave stack frame.
	 * Give as GP value to the thread the same we have.
	 */
	ts->pc = (int) routine;
	ts->r4 = (int) child;
	ts->r29 = ((int) top) - 4 * sizeof(int);
	ts->r28 = (int) &_gp;

	MACH_CALL(thread_set_state(thread,MIPS_THREAD_STATE,(thread_state_t) &state,MIPS_THREAD_STATE_COUNT),r);
}

spin_try_lock_overwrite()
{
 	extern char spin_try_lock_emul[];
 	extern char spin_try_lock_emul_end[];
 
 	kern_return_t status;
 	char *etype;
 	vm_machine_attribute_val_t      attr_value;
 	int sz;
 
 	sz = (int)spin_try_lock_emul_end - (int)spin_try_lock_emul;
 
 	status = vm_protect(mach_task_self(), (vm_address_t) spin_try_lock,
 			    sz,
 			    0, VM_PROT_ALL);
 	if (status != KERN_SUCCESS)   {
 		etype = "tas_overwrite protect write";
 		goto error;
 	}
 
	memcpy(spin_try_lock, spin_try_lock_emul, sz);
 
 	status = vm_protect(mach_task_self(), (vm_address_t) spin_try_lock, sz,
 			    0, VM_PROT_READ|VM_PROT_EXECUTE);
 	if (status != KERN_SUCCESS)   {
 		etype = "tas_overwrite protect execute";
 		goto error;
 	}
 
 	/* I'm gonna flush that cache right out of my hair... */
 	attr_value = MATTR_VAL_CACHE_FLUSH;
 	status = vm_machine_attribute(mach_task_self(),
 				      (vm_address_t) spin_try_lock,
 				      sz,
 				      MATTR_CACHE,
 				      &attr_value);
 	if (status != KERN_SUCCESS)  {
 		etype = "tas_overwrite cache flush";
 		goto error;
 	}
	
 	return;
 
    error:
 	/*mach_error(etype, status);*/
 	printf("spin_try_lock_overwrite: %s %d\n", etype, status);
 
 	exit(-1);
}
 
 
int cthread_trap_lock;      /* OVERWRITE THIS FOR OLD BEHAVIOR */
 
unsigned int cthread_spin_try_lock_count = 0;
 
spin_try_lock_init()
{
 	kern_return_t status;
 	extern char spin_try_lock_end[];
 	vm_offset_t spin_try_lock_pc = (vm_offset_t)spin_try_lock;
 	vm_offset_t spin_try_lock_pc_end = (vm_offset_t)spin_try_lock_end;
 
 	if (cthread_trap_lock)  {
 		spin_try_lock_overwrite();
 		return;
 	}
	status = task_ras_control(mach_task_self(),
				  spin_try_lock_pc,
				  spin_try_lock_pc_end,
				  TASK_RAS_CONTROL_INSTALL_ONE);
	if (status != KERN_SUCCESS)  {
 		spin_try_lock_overwrite();
 	}
}

mips_cthread_md_init()
{
 	spin_try_lock_init();
}

