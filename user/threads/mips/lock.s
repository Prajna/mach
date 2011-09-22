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
 * $Log:	lock.s,v $
 * Revision 2.9  93/01/21  12:28:10  danner
 * 	Label movement for RAS
 * 	[93/01/19  16:38:06  bershad]
 * 
 * Revision 2.8  93/01/14  18:05:50  danner
 * 	Fixes for ANSI CPP.
 * 	[92/08/22            jvh]
 * 
 * Revision 2.7  92/07/20  13:33:45  cmaeda
 * 	New implementation using ras technology.
 * 	[92/05/11  14:42:59  cmaeda]
 * 
 * Revision 2.6  91/05/14  17:58:13  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/14  14:20:54  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:38:54  mrt]
 * 
 * Revision 2.4  90/01/22  23:09:49  af
 * 	Conditionalized timing functions.
 * 	[90/01/20  17:34:21  af]
 * 
 * Revision 2.3  89/12/08  19:49:24  rwd
 * 	Reflected rwd's name changes.
 * 	[89/12/06            af]
 * 
 * Revision 2.2  89/11/29  14:19:08  af
 * 	New name for tas, fixed timing functions.
 * 	[89/10/28  12:11:22  af]
 * 
 * 	Turned into a piece of literate programming.
 * 	Therefore:
 * 		Copyright (c) 1989 Alessandro Forin
 * 	[89/07/16            af]
 * 
 * 	Created.
 * 	[89/07/06            af]
 * 
 */

#include <mach/mips/asm.h>
#include <mach/mips/mips_instruction.h>

/*
   The C interface for this function is

	boolean_t
	spin_try_lock[,_emul](m)
	int * m;

   The function has a slightly different semantics than TAS: it will
   return a boolean value that indicates whether the lock was acquired
   or not.  If not, we'll presume that the user will retry after some
   appropriate delay.

   This file provides two alternative implementations of TAS for the MIPS
   architecture. The first relies on kernel emulation, whereby every TAS
   is done in the kernel with interrupts disabled. It's slow, but
   works.  The second relies on kernel level restart of interrupted RMW
   sequences.
 
*/



/*
 KERNEL EMULATION:
   TAS operates on a single register which holds the address
   of the lock. The previous content of the lock will be returned in that
   same register, and the lock itself will contain some non-zero value.
   Since the compiler will never generate this instruction, we'll further
   restrict the instruction to only operate on one particular register,
   register "a0".

   Implementing the user part is trivial. All we need is an assembly
   function that uses the new opcode that we will add to the MIPS
   instruction set.

   We make use here of one more piece of information: the value
   that TAS puts in the lock is the address of the lock itself.
   This makes things fit into four instructions, but adding a
   branch instruction would only waste one extra cycle and buy
   more generality.  A truly general purpose implementation would
   also follow a test&TAS scheme, by testing the content of the
   lock before executing the (expensive) TAS instruction.
   In our case though, it is known that the CThreads wrapping
   for this function already does that before calling the function
   itself.  The unlock primitive is quite strightforward.  Note
   only that the existing code in the CThreads package already
   makes machine-independent assumptions about the value (and size!)
   of a lock, so there really is no choice here.
*/


	.text
	.align	2
	.set	noreorder

LEAF(spin_try_lock_emul)
	move	v0,a0		/* preserve a0's content */
	.word	op_tas		/* do the TAS */
	j	ra		/* return whether the */
	xor	v0,a0,v0	/* lock was acquired */
EXPORT(spin_try_lock_emul_end)
	END(spin_try_lock_emul)





/*

  RCS Implementation:

  This implementation assumes that interrupts do not occur in the
  TAS sequence, and relies on the kernel to return control back to 
  the beginning of the lock acquisition code.  See:
  Mutual Exclusion for Uniprocessors by Bershad. CMU-TR-XX-YY
  for an explanation of what is going on here.
  NOTE: This sequence must be at least as long as the emulated
  sequence above to ensure that the overwrite machinery works.
 */

        .align 2
LEAF(spin_try_lock)
        lw      v0, 0(a0)
EXPORT(spin_try_lock_end)		/* addr of final instruction (next) */
        sw      a0, 0(a0)
        j       ra
        xor     v0, a0, v0
END(spin_try_lock)



/*
 * A real test and set instruction.
 */
LEAF(fast_tas)
        lw      v0, 0(a0)
        j       ra
EXPORT(fast_tas_end)                    /* End RAS */
        sw      a0, 0(a0)
END(fast_tas)


/*
 * Generic unlock.  Can be used by either implementation
 */


LEAF(spin_unlock)
	j	ra
	sw	zero,0(a0)
END(spin_unlock)



