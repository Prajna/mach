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
 * $Log:	db_interface.c,v $
 * Revision 2.19  93/05/15  19:13:00  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.18  93/05/10  21:21:05  rvb
 * 	Removed sys/types.h
 * 	[93/05/06  09:31:18  af]
 * 
 * Revision 2.17  93/01/14  17:51:07  danner
 * 	Solved another byteorder issue. Proper spl typing.
 * 	[92/12/01            af]
 * 
 * Revision 2.16  92/02/19  16:46:48  elf
 * 	Byteorder fix to read_bytes(), from Steve.Ackerman.
 * 	Removed ignorant mod that played with the restart address.
 * 	If you do not know what they do, pls leave things as they are.
 * 	[92/01/29            af]
 * 
 * Revision 2.15  92/01/03  20:41:33  dbg
 * 	Fix (from <Steve.Ackerman@zindigi.msg.uvm.edu>) the kdbputmem
 * 	routine for little endian machines.
 * 	[91/12/26  10:59:59  af]
 * 
 * Revision 2.14  91/12/30  13:36:30  dbg
 * 	kdb_init and read_mips_symtab take no parameters.
 * 	Enter ddb whether or not RB_KDB is set.
 * 	[91/10/30            dbg]
 * 
 * Revision 2.13  91/10/09  16:13:36  af
 * 	Added user space access and check routines.
 * 	Added U*X task information print routines.
 * 	Changed db_trap to db_task_trap.
 * 	[91/09/05            tak]
 * 
 * Revision 2.12  91/08/24  12:22:37  af
 * 	Use generic console calls.
 * 	[91/08/02  03:15:32  af]
 * 
 * Revision 2.11  91/05/14  17:33:28  mrt
 * 	Correcting copyright
 * 
 * Revision 2.10  91/03/16  14:55:31  rpd
 * 	Added resume, continuation arguments to vm_fault.
 * 	[91/02/04            rpd]
 * 	Changed kdb_trap to use db_recover.
 * 	[91/01/13            rpd]
 * 
 * 	Removed db_nofault.
 * 	db_error might return now.
 * 	[91/01/13            rpd]
 * 
 * Revision 2.9  91/02/14  14:35:43  mrt
 * 	Added flag not to invoke vm_fault(), used in crashed kernels.
 * 	[91/02/12  12:15:09  af]
 * 
 * Revision 2.8  91/02/05  17:47:51  mrt
 * 	Added author notices
 * 	[91/02/04  11:21:54  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:25:29  mrt]
 * 
 * Revision 2.7  91/01/08  15:49:22  rpd
 * 	Changed kdb_trap to return TRUE if it actually enters kdb.
 * 	[90/12/23            rpd]
 * 	Fixed to use kdbsplhigh/kdbsplx.
 * 	[90/11/26            rpd]
 * 
 * Revision 2.6  90/12/05  23:37:26  af
 * 
 * 
 * Revision 2.5  90/12/05  20:49:41  af
 * 	Adapts for new drivers, made clear we do not expect
 * 	any environment pointer in kdb_init().
 * 	[90/12/03            af]
 * 
 * Revision 2.4  90/10/25  14:46:35  rwd
 * 	Support for watchpoints.
 * 	[90/10/16            rpd]
 * 
 * Revision 2.3  90/09/09  14:33:17  rpd
 * 	Reversed return code in kdb_intr, so that the do key press does not
 * 	generate garbage chars (if kdb enabled).
 * 	[90/08/30  17:03:22  af]
 * 
 * Revision 2.2  90/08/27  22:07:24  dbg
 * 	Got rid of ddb_regs, exception status is kept on stack.
 * 	I still have to decide to switch over to dbg's simpler
 * 	way to access memory or not. [how do you trace another
 * 	thread's user stack in his way ?]
 * 	Created.
 * 	[90/08/14            af]
 * 
 */
/*
 *	File: db_interface.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	8/90
 *
 *	Interface to new debugger.
 *
 */
#include <sys/reboot.h>

#include <mips/pmap.h>
#include <mips/prom_interface.h>
#include <mips/mips_cpu.h>
#include <machine/machspl.h>	/* for spl */

#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <ddb/db_task_thread.h>

#include <mips/thread.h>
#include <mips/db_machdep.h>
#include <mips/setjmp.h>

#include <chips/serial_defs.h>

#define	DB_MAX_LINE	50	/* max output lines */
#define DB_MAX_WIDTH	132	/* max output colunms */

/*
 *  kdb_init - initialize kernel debugger
 */

kdb_init()
{
	extern db_max_line, db_max_width;

	read_mips_symtab();

	db_max_line = DB_MAX_LINE;
	db_max_width = DB_MAX_WIDTH;

	kdb_enable();
}

/*
 *  kdb_enable - enable kdb usage
 */
kdb_enable()
{
	register struct restart_blk *rb = (struct restart_blk *)RESTART_ADDR;

	extern int kdb_breakpoint(), halt_all_cpus();

	if (boothowto & RB_KDB) {
		rb-> rb_bpaddr = kdb_breakpoint;
	} else
		rb-> rb_bpaddr = halt_all_cpus;

	init_restartblk();
}


/*
 * Received keyboard interrupt sequence.
 */
int	kdbactive = 0;

kdb_kintr()
{
	if (kdbactive == 0) {
		printf("\n\nkernel: keyboard interrupt\n");
		gimmeabreak();
		return 1;
	}
	return 0;
}

#define TRAP_TYPES 17
char	*kdb_trap_names[TRAP_TYPES] = {
					/* Hardware trap codes */
	"Interrupt",
	"TLB mod",
	"TLB miss (read)",
	"TLB miss (write)",
	"Read Address Error",
	"Write Address Error",
	"Instruction Bus Error",
	"Data Bus Error",
	"Syscall",
	"Breakpoint",
	"Illegal Instruction",
	"Coprocessor Unusable",
	"Overflow",
	"13", "14", "15",
					/* Software trap codes */
	"AST"
};

/*
 *  kdb_trap - field a BPT trap
 *
 *  Esp points to the saved register state
 *
 *  Flag is TRUE when we are called from trap() [because of a fatal error],
 *  it is FALSE when we are called from kdb_breakpoint().  Only in this
 *  latter case is the TLB info in *esp valid.
 */
extern jmp_buf_t *db_recover;

kdb_trap(esp, flag)
	register struct mips_saved_state *esp;
{
	int    cause, type;
	spl_t	s;
	struct mips_saved_state *old_exc_frame;
	task_t task;
	unsigned kdbpeek();

	s = kdbsplhigh();	/* if not already */

	cause = esp->cause & CAUSE_EXC_MASK;
	type = cause >> CAUSE_EXC_SHIFT;
	if ((flag == 1) && (db_recover != 0)) {
		/*
		 *	We can recover from this exception.
		 */
		db_printf("Caught ");
		if (type >= TRAP_TYPES)
		    db_printf("type %d", type);
		else
		    db_printf("%s", kdb_trap_names[type]);
		db_printf(" trap, pc = %x, va = %x",
			  esp->pc, esp->bad_address);
		db_error("");
		/*NOTREACHED*/
	}
	if (flag) {
		esp->tlb_low = 0xbad00bad;
		esp->tlb_high = 0xbad00bad;
		esp->tlb_index = 0xbad00bad;
		esp->tlb_context = 0xbad00bad;

		if (flag == 1)
			kdbprinttrap(type, esp);
	}

	/*
	 * Make things reentrant, so that we can debug the debugger
	 */
	old_exc_frame = db_cur_exc_frame;
	db_cur_exc_frame = esp;

	kdbactive++;
	(*console_pollc)(0, TRUE);
	db_task_trap(cause, flag, (esp->sr & SR_KUo) != 0);
	(*console_pollc)(0, FALSE);
	kdbactive--;

	db_cur_exc_frame = old_exc_frame;

	task = (current_thread())? current_thread()->task: TASK_NULL;
	if (!flag && isa_kbreak(kdbpeek(esp->pc, task)))
		esp->pc += 4;	/* No loops! */
	kdbsplx(s);
	return(1);
}

/*
 * Print trap reason.
 */
kdbprinttrap(type, code)
	int	type, code;
{
	printf("kernel: ");
	if (type >= TRAP_TYPES)
	    printf("type %d", type);
	else
	    printf("%s", kdb_trap_names[type]);
	printf(" trap, frame @ %x\n", code);
}

/*
 * Peek/Poke routines
 */
extern unsigned kdbgetmem();/*forward*/

unsigned
kdbpeek(addr, task)
	unsigned int *addr;
	task_t   task;
{
	if (ISA_KUSEG(addr)) {
		if (task == TASK_NULL && 
		    (task = db_current_task()) == TASK_NULL) {
			db_printf("\nbad address %x\n", addr);
			db_error(0);
			/* NOTREACHED */
		}
		(void)kdb_vtophys(task->map, addr, &addr, TRUE);
	}
	/*
	 * 	Kernel address.  If it needs mapping do as above,
	 *	otherwise just get it.
	 */
	else if (ISA_K2SEG(addr)) {
		(void)kdb_vtophys(kernel_map, addr, &addr, TRUE);
	}
	return kdbgetmem(addr);
}

kdbpoke(addr, value, task)
	unsigned int *addr;
	unsigned int value;
	task_t   task;
{
	if (ISA_KUSEG(addr)) {
		if (task == TASK_NULL && 
		    (task = db_current_task()) == TASK_NULL) {
			db_printf("\nbad address %x\n", addr);
			db_error(0);
			/* NOTREACHED */
		}
		(void)kdb_vtophys(task->map, addr, &addr, TRUE);
	}
	/*
	 * 	Kernel address.  If it needs mapping do as above,
	 *	otherwise just get it.
	 */
	else if (ISA_K2SEG(addr)) {
		(void)kdb_vtophys(kernel_map, addr, &addr, TRUE);
	}
	return kdbputmem(addr, value);
}

/*
 *   kdb_vtophys - map addresses by hand, so that TLB is
 *   minimally affected.  But we do take faults, if necessary.
 */
boolean_t db_no_vm_fault = TRUE;

kdb_vtophys(map, addr, paddr, no_ret_error)
	vm_map_t map;
	unsigned int *addr;
	unsigned int **paddr;
	boolean_t no_ret_error;
{
	pmap_t		pmap;
	vm_offset_t	pa;	/* physical address */
	int		ret;

	if (map == VM_MAP_NULL) {
		if (no_ret_error)
			db_error("No VM map");
			/* NOTREACHED */
		return(-1);
	}
	pmap = map->pmap;
retry:
	pa = pmap_resident_extract(pmap, addr);
	if (pa == 0) {
		if (!no_ret_error)
			return(-1);
		if (db_no_vm_fault)
			db_error("Paged out page");

		/* NOTE: we might get hung if map is locked!! */
		ret = vm_fault(map, trunc_page(addr), VM_PROT_READ, FALSE,
			       FALSE, (void (*)()) 0);
		if (ret != KERN_SUCCESS) {
			db_error("Invalid virtual address");
			/* NOTREACHED */
		}
		goto retry;
	}
	*paddr = (unsigned *) PHYS_TO_K0SEG(pa);
	return(0);
}


/*
 *   kdbgetmem - read long word from kernel address space
 */
unsigned
kdbgetmem(addr)
	unsigned char	*addr;
{
	unsigned 		temp = 0;
	int			i;
	extern unsigned		kdb_getiomem();

	if (addr == 0)
		return(-1);

	if (ISA_K1SEG(addr))
		return kdb_getiomem(addr);

	if (mem_size && (K0SEG_TO_PHYS(addr) > mem_size)) {
		db_error("Beyond physical memory");
		return(-1);
	}

#if	BYTE_MSF
	for (i = 0; i <= 3; i++) {
#else	BYTE_MSF
	for (i = 3; i >= 0; i--) {
#endif	BYTE_MSF
		temp <<= 8;
		temp |= addr[i] & 0xFF;
	}
	return temp;
}


/*
 *  kdbputmem - write word to kernel address space
 */

kdbputmem(addr, value)
	unsigned char	*addr;
	unsigned	 value;
{
	int		 i;
	extern		 kdb_putiomem();

	if (addr == 0)
		return 0;

	if (ISA_K1SEG(addr))
		return kdb_putiomem(addr, value);

	if (mem_size && (K0SEG_TO_PHYS(addr) > mem_size))
		return 0;

#if	BYTE_MSF
	for (i = 3; i >= 0; i--) {
#else	BYTE_MSF
	for (i = 0; i < 4; i++) {
#endif	BYTE_MSF
		addr[i] = value & 0xFF;
		value >>= 8;
	}
	/* Might be text, flush I-cache */
	mipscache_Iflush((unsigned)(addr - 4) & ~0x3, 2 * sizeof(int));
}

/*
 * Read bytes (??from kernel address space??) for debugger.
 */

void
db_read_bytes(addr, size, data, task)
	register unsigned *addr;
	register int	   size;
	register unsigned *data;
	task_t   task;
{
	while (size >= 4)
		*data++ = kdbpeek(addr++, task), size -= 4;

	if (size) {
		unsigned tmp;
		register char *dst = (char*)data;

		tmp = kdbpeek(addr, task);
		while (size--) {
#if      BYTE_MSF
			int shift = 24;

			/* want highest -> lowest byte */
			*dst++ = (tmp >> shift) & 0xFF;
			shift -= 8;
#else    BYTE_MSF
			*dst++ = tmp & 0xFF;
			tmp >>= 8;
#endif	 BYTE_MSF
		}
	}
}

/*
 * Write bytes (?? to kernel address space??) for debugger.
 */
void
db_write_bytes(addr, size, data, task)
	register unsigned *addr;
	register int	   size;
	register unsigned *data;
	task_t   task;
{
	register char	*dst;

	while (size >= 4)
		kdbpoke(addr++, *data++, task), size -= 4;
	if (size) {
		unsigned tmp = kdbpeek(addr, task), tmp1 = 0;
		register char *src = (char*)data;

		tmp >>= (size << 3);
		tmp <<= (size << 3);
		while (size--) {
			tmp1 <<= 8;
			tmp1 |= src[size] & 0xff;
		}
		kdbpoke(addr, tmp|tmp1, task);
	}
}

/*
 *	TLB utilities
 */

char *tlbhi_fmt = TLB_HI_FMT;
char *tlblo_fmt = TLB_LO_FMT;

tlbdump(from,to)
{
	int hi,lo, save_hi, save_lo;
	spl_t	s;

	save_hi = tlb_read_tlbhi();
	save_lo = tlb_read_tlblo();

	if ((from == to) && (from == 0))
		to = 63;
	if (from < 0)
		from = 0;
	if (to > 63)
		to = 63;
	do {
		s = kdbsplhigh();
		tlb_read_line(from);
		hi = tlb_read_tlbhi();
		lo = tlb_read_tlblo();
		tlb_write_line(-3, save_hi, save_lo);
		kdbsplx(s);

		if (hi == K1SEG_BASE)
			continue;

		printf("%d: %b -> %b\n", from, hi, tlbhi_fmt, lo, tlblo_fmt);
	} while (from++ < to);
}

tlbpid()
{
	printf("tlbhi=%b\n", tlb_read_tlbhi(), tlbhi_fmt);
}

boolean_t
db_check_access(addr, size, task)
	vm_offset_t	addr;
	register int	size;
	task_t		task;
{
	register	n;
	unsigned	*paddr;
	vm_map_t	map;
	
	if (ISA_KUSEG(addr)) {
		if (task == TASK_NULL && 
		    (task = db_current_task()) == TASK_NULL)
			return(FALSE);
		map = task->map;
	} else if (ISA_K2SEG(addr)) {
		map = kernel_map;
	} else {
		/*
		 * no mapping area
		 * check only memory size (not strict, but OK)
		 */
		if (ISA_K0SEG(addr)) {
			if (mem_size && K0SEG_TO_PHYS(addr) > mem_size)
				return(FALSE);
		}
		return(TRUE);
	}
	while (size > 0) {
		if (ISA_K0SEG(addr)) {
			if (mem_size && K0SEG_TO_PHYS(addr+size) > mem_size)
				return(FALSE);
			break;
		}
		if (kdb_vtophys(map, (unsigned *)addr, &paddr, FALSE) < 0)
			return(FALSE);
		n = mips_trunc_page(addr + MIPS_PGBYTES) - addr;
		if (n > size)
			n = size;
		size -= n;
		addr += n;
	}
	return(TRUE);
}

boolean_t
db_phys_eq(task1, addr1, task2, addr2)
	task_t		task1;
	vm_offset_t	addr1;
	task_t		task2;
	vm_offset_t	addr2;
{
	unsigned	*paddr1, *paddr2;

	if (addr1 >= VM_MAX_ADDRESS || addr2 >= VM_MAX_ADDRESS)
		return(FALSE);
	if ((addr1 & (MIPS_PGBYTES-1)) != (addr2 & (MIPS_PGBYTES-1)))
		return(FALSE);
	if (task1 == TASK_NULL || task1->map == 0 ||
		 task2 == TASK_NULL || task2->map == 0)
		return(FALSE);
	if (kdb_vtophys(task1->map, (unsigned *)addr1, &paddr1, FALSE) < 0 ||
	    kdb_vtophys(task2->map, (unsigned *)addr2, &paddr2, FALSE) < 0)
		return(FALSE);
	return(paddr1 == paddr2);
}

#define DB_USER_STACK_ADDR		(VM_MAX_ADDRESS-MIPS_PGBYTES)
#define DB_NAME_SEARCH_LIMIT		(DB_USER_STACK_ADDR-(MIPS_PGBYTES*3))

static int
db_search_null(task, svaddr, evaddr, skaddr, flag)
	task_t	 task;
	unsigned *svaddr;
	unsigned evaddr;
	unsigned **skaddr;
	int	 flag;
{
	register unsigned vaddr;
	register unsigned *kaddr;

	kaddr = *skaddr;
	for (vaddr = *svaddr; vaddr > evaddr; vaddr -= sizeof(unsigned)) {
		if (vaddr % MIPS_PGBYTES == 0) {
			vaddr -= sizeof(unsigned);
			if (kdb_vtophys(task->map, vaddr, skaddr, FALSE) < 0)
				return(-1);
			kaddr = *skaddr;
		} else {
			vaddr -= sizeof(unsigned);
			kaddr--;
		}
		if ((*kaddr == 0) ^ (flag  == 0)) {
			*svaddr = vaddr;
			*skaddr = kaddr;
			return(0);
		}
	}
	return(-1);
}

void
db_task_name(task)
	task_t		task;
{
	register char *p;
	register n;
	unsigned vaddr, *kaddr;

	vaddr = DB_USER_STACK_ADDR;
	kaddr = 0;

	/*
	 * skip nulls at the end
	 */
	if (db_search_null(task, &vaddr, DB_NAME_SEARCH_LIMIT, &kaddr, 0) < 0) {
		db_printf(DB_NULL_TASK_NAME);
		return;
	}
	/*
	 * search start of args
	 */
	if (db_search_null(task, &vaddr, DB_NAME_SEARCH_LIMIT, &kaddr, 1) < 0) {
		db_printf(DB_NULL_TASK_NAME);
		return;
	}

	n = DB_TASK_NAME_LEN-1;
	p = (char *)kaddr + sizeof(unsigned);
	for (vaddr += sizeof(int); vaddr < DB_USER_STACK_ADDR && n > 0; 
							vaddr++, p++, n--) {
		if (vaddr % MIPS_PGBYTES == 0) {
			(void)kdb_vtophys(task->map, vaddr, &kaddr, FALSE);
			p = (char*)kaddr;
		}
		db_printf("%c", (*p < ' ' || *p > '~')? ' ': *p);
	}
	while (n-- >= 0)	/* compare with >= 0 for one more space */
		db_printf(" ");
}
