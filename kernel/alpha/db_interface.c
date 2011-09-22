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
 * $Log:	db_interface.c,v $
 * Revision 2.3  93/05/15  19:11:09  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.2  93/02/05  07:57:50  danner
 * 	Made a mental note to fix watchpoints, later on.
 * 	[93/02/04  00:43:50  af]
 * 
 * 	MP debugging works now.
 * 	[93/01/15            af]
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:13:59  af]
 * 
 * 	First cut at using new MI multiP code. Not fully satisfactory.
 * 	[92/12/16  12:47:05  af]
 * 
 * 	Created.
 * 	[92/06/02            af]
 * 
 */
/*
 *	File: db_interface.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Interface to kernel debugger.
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 */

#include <sys/reboot.h>

#include <alpha/pmap.h>
#include <alpha/prom_interface.h>
#include <alpha/alpha_cpu.h>
#include <alpha/frame.h>
#include <alpha/trap.h>

#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <ddb/db_task_thread.h>

#include <alpha/thread.h>
#include <alpha/db_machdep.h>
#include <alpha/setjmp.h>
#include <alpha/machspl.h>	/* spls */

#include <sys/types.h>
#include <chips/serial_defs.h>

#define	DB_MAX_LINE	50	/* max output lines */
#define DB_MAX_WIDTH	132	/* max output colunms */

/*
 *  kdb_init - initialize kernel debugger
 */

kdb_init(gcc_compiled)
	boolean_t	gcc_compiled;
{
	extern db_max_line, db_max_width;

	if (gcc_compiled)
		ddb_init();
	else
		read_alpha_symtab();

	db_max_line = DB_MAX_LINE;
	db_max_width = DB_MAX_WIDTH;

}

/*
 * Asked from some other processor to enter debugger.
 * Just do it. [name is known to MI code].
 * Also, from keyboard driver.
 */

kdb_kintr()
{
	gimmeabreak();
}

char	*kdb_trap_names[TRAP_TYPES+1] = {
				/* see trap.h */
	"undefined(0)",				/* 0 */
	"fpa disabled",				/* 1 */
	"protection fault",			/* 2 */
	"translation invalid",			/* 3 */
	"read fault",				/* 4 */
	"write fault",				/* 5 */
	"execute fault",			/* 6 */
	"arithmetic trap",			/* 7 */
	"AST (kernel)",				/* 8 */
	"AST (executive)",			/* 9 */
	"AST (supervisor)",			/* 10 */
	"AST (user)",				/* 11 */
	"unaligned access",			/* 12 */
	"bpt",					/* 13 */
	"bug",					/* 14 */
	"illegal instruction",			/* 15 */
	"illegal PAL call",			/* 16 */
	"chmk",					/* 17 */
	"chme",					/* 18 */
	"chms",					/* 19 */
	"chmu",					/* 20 */
	"soft int",				/* 21 */
	"sce",					/* 22 */
	"pce",					/* 23 */
	"pfail",				/* 24 */
	"scheck",				/* 25 */
	"pcheck",				/* 26 */
	"unexpected",				/* 27 */
	"bad interrupt"				/* 28 */
};

/*
 *  kdb_trap - field a BPT trap
 *
 *  Esp points to the saved register state
 *
 *  Flag is TRUE when we are called from trap() [because of a fatal error],
 *  it is FALSE when we are called from kdb_breakpoint().
 */
extern jmp_buf_t *db_recover;

kdb_trap(esp, flag)
	register struct alpha_saved_state *esp;
{
	long	type;
	spl_t	s;
	struct alpha_saved_state *old_exc_frame;
	thread_t thread;
	task_t task;
	db_expr_t kdbpeek();
	register db_addr_t	ps;


	s = kdbsplhigh();	/* if not already */

	type = esp->cause;
	if ((flag == 1) && (db_recover != 0)) {
		/*
		 *	We can recover from this exception.
		 */
		db_printf("Caught ");
		if (type > TRAP_TYPES)
		    db_printf("type %d", type);
		else
		    db_printf("%s", kdb_trap_names[type]);
		db_printf(" trap, pc = %X, va = %X\n",
			  esp->saved_frame.saved_pc, esp->bad_address);
		db_error("");
		/*NOTREACHED*/
	}

	esp->hw_pcb.usp = mfpr_usp();
	ps = esp->saved_frame.saved_ps;
	thread = current_thread();
	task = (thread) ? thread->task: TASK_NULL;

/* XXX flag==2 --> watchpoints, which are broken right now */

	if (flag) {
		kdbprinttrap(type, esp);
		/*
		 * Must backup the PC in a variety of cases
		 */
		if ((type == T_ARITHMETIC) ||
		   ((type >= T_UNALIGNED) && (type <= T_CHMU)))
			esp->saved_frame.saved_pc -= 4;
		/*
		 * To get a sensible trace, needs to recover trapped SP
		 */
		if (alpha_user_mode(ps)) {
			esp->sp = esp->hw_pcb.usp;
		} else {
			esp->sp += sizeof(struct trap_frame) + (ps >> 56);
		}
	} else {
		/*
		 * Must check for regular userland bpt/sstep.
		 * We do it by noticing that we knew nothing
		 * about this bpt, and came from user mode.
		 */
		db_addr_t	addr = (db_addr_t)esp->saved_frame.saved_pc;

		if ((type == T_BP) && alpha_user_mode(ps) &&
		    (db_find_breakpoint(task, addr) == 0)) {

			/*
			 * Still must check against kernelmode sstep
			 */
			if ((db_find_temp_breakpoint(task, addr) == 0) ||
			    (thread && thread->pcb->mms.msss)) {
#if	DEBUG
db_printf("{User bpt/sstep @%X}", addr);
#endif
				esp->framep->saved_pc -= sizeof(alpha_instruction);
				esp->framep = &esp->saved_frame;
				kdbsplx(s);
				trap( esp, 0, 0, T_BP);
				return;
			}
		}
	}

#if	(NCPUS>1)
	if (db_enter())
#endif
	{
		/*
		 * Make things reentrant, so that we can debug the debugger
		 */
		old_exc_frame = db_cur_exc_frame;
		db_cur_exc_frame = esp;

		(*console_pollc)(0, TRUE);
		db_task_trap(type, flag, alpha_user_mode(ps));
		(*console_pollc)(0, FALSE);

		db_cur_exc_frame = old_exc_frame;

	}
#if	(NCPUS>1)
	db_leave();
#endif

	/* No loops inside debugger! */
	if (!flag) {
		register alpha_instruction ins;
		ins.bits = kdbpeek(esp->saved_frame.saved_pc, task);
		if (isa_kbreak(ins.bits))
			esp->saved_frame.saved_pc += 4;
	}

	kdbsplx(s);
	return(1);
}

/*
 * Print trap reason.
 */
kdbprinttrap(type, esp)
	int		type;
	vm_offset_t	esp;
{
	printf("kernel: ");
	if (type > TRAP_TYPES)
	    printf("type %d", type);
	else
	    printf("%s", kdb_trap_names[type]);
	printf(" trap, frame @ %#X\n", esp);
}

/*
 * Peek/Poke routines
 */
extern db_expr_t kdbgetmem();/*forward*/

db_expr_t
kdbpeek(addr, task)
	vm_offset_t	addr;
	task_t   	task;
{
	if (ISA_KUSEG(addr)) {
		if (task == TASK_NULL && 
		    (task = db_current_task()) == TASK_NULL) {
			db_printf("\nbad address %X\n", addr);
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
	vm_offset_t	addr;
	db_expr_t	value;
	task_t   task;
{
	if (ISA_KUSEG(addr)) {
		if (task == TASK_NULL && 
		    (task = db_current_task()) == TASK_NULL) {
			db_printf("\nbad address %X\n", addr);
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
	vm_map_t	map;
	vm_offset_t	addr;
	vm_offset_t	*paddr;
	boolean_t	no_ret_error;
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
	*paddr = (vm_offset_t) PHYS_TO_K0SEG(pa);
	return(0);
}


/*
 *   kdbgetmem - read long word from kernel address space
 */
boolean_t aduio = FALSE;

db_expr_t
kdbgetmem(addr)
	unsigned char	*addr;
{
	db_expr_t		temp = 0;
	int			i;
	extern db_expr_t	kdb_getiomem();

	if (addr == 0)
		return(-1);

	if (ISA_K1SEG(addr))
		return kdb_getiomem(addr);

	if (mem_size && (K0SEG_TO_PHYS(addr) > mem_size)) {
if (aduio && (((vm_offset_t)addr & 0x7) == 0)) return *((natural_t*)addr);
		db_error("Beyond physical memory");
		return(-1);
	}

	for (i = sizeof(db_expr_t)-1; i >= 0; i--) {
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
	db_expr_t	 value;
{
	int		 i;
	extern		 kdb_putiomem();

	if (addr == 0)
		return 0;

	if (ISA_K1SEG(addr))
		return kdb_putiomem(addr, value);

	if (mem_size && (K0SEG_TO_PHYS(addr) > mem_size))
		return 0;

	for (i = 0; i < sizeof(db_expr_t); i++) {
		addr[i] = value & 0xFF;
		value >>= 8;
	}
	/* Might be text, flush I-cache */
#if	DEBUG
{
extern unsigned char start[], etext[];
if (addr >= start && addr < etext)
	db_printf("{changed inst @%#X}\n", addr);
}
#endif
	alphacache_Iflush();
}

/*
 * Read bytes (??from kernel address space??) for debugger.
 */

void
db_read_bytes(addr, size, data, task)
	register db_expr_t	*addr;
	register int   		size;
	register db_expr_t	*data;
	task_t			task;
{
if (addr < (db_expr_t*)100) gimmeabreak();

	while (size >= sizeof(db_expr_t))
		*data++ = kdbpeek(addr++, task), size -= sizeof(db_expr_t);

	if (size) {
		db_expr_t tmp;
		register char *dst = (char*)data;

		tmp = kdbpeek(addr, task);
		while (size--) {
			*dst++ = tmp & 0xFF;
			tmp >>= 8;
		}
	}
}

/*
 * Write bytes (?? to kernel address space??) for debugger.
 */
void
db_write_bytes(addr, size, data, task)
	register db_expr_t	*addr;
	register int   		size;
	register db_expr_t	*data;
	task_t			task;
{
	register char	*dst;

	while (size >= sizeof(db_expr_t))
		kdbpoke(addr++, *data++, task), size -= sizeof(db_expr_t);
	if (size) {
		db_expr_t tmp, tmp1;
		register unsigned char *src = (unsigned char*)data;

		/* tmp is what we do not clobber */
		tmp = kdbpeek(addr, task);
		tmp >>= (size << 3);
		tmp <<= (size << 3);
		tmp1 = 0;
		while (size--) {
			tmp1 <<= 8;
			tmp1 |= src[size];
		}
		kdbpoke(addr, tmp|tmp1, task);
	}
}

/*
 * From OSF.
 */
boolean_t
db_check_access(addr, size, task)
	vm_offset_t	addr;
	register int	size;
	task_t		task;
{
	vm_offset_t	*paddr;
	vm_map_t	map;
	register vm_offset_t	n;
	
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
		if (kdb_vtophys(map, addr, &paddr, FALSE) < 0)
			return(FALSE);
		n = alpha_trunc_page(addr + ALPHA_PGBYTES) - addr;
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
	vm_offset_t	paddr1, paddr2;

	if (addr1 >= VM_MAX_ADDRESS || addr2 >= VM_MAX_ADDRESS)
		return(FALSE);
	if ((addr1 & (ALPHA_PGBYTES-1)) != (addr2 & (ALPHA_PGBYTES-1)))
		return(FALSE);
	if (task1 == TASK_NULL || task1->map == 0 ||
		 task2 == TASK_NULL || task2->map == 0)
		return(FALSE);
	if (kdb_vtophys(task1->map, addr1, &paddr1, FALSE) < 0 ||
	    kdb_vtophys(task2->map, addr2, &paddr2, FALSE) < 0)
		return(FALSE);
	return(paddr1 == paddr2);
}

#define DB_USER_STACK_ADDR		(VM_MAX_ADDRESS-ALPHA_PGBYTES)
#define DB_NAME_SEARCH_LIMIT		(DB_USER_STACK_ADDR-(ALPHA_PGBYTES*3))

static int
db_search_null(task, svaddr, evaddr, skaddr, flag)
	task_t		task;
	vm_offset_t	*svaddr;
	vm_offset_t	evaddr;
	vm_offset_t	*skaddr;
	int		flag;
{
	register vm_offset_t vaddr;
	register vm_offset_t kaddr;

	kaddr = *skaddr;
	for (vaddr = *svaddr; vaddr > evaddr; vaddr -= sizeof(vm_offset_t)) {
		if (vaddr % ALPHA_PGBYTES == 0) {
			vaddr -= sizeof(vm_offset_t);
			if (kdb_vtophys(task->map, vaddr, skaddr, FALSE) < 0)
				return(-1);
			kaddr = *skaddr;
		} else {
			vaddr -= sizeof(vm_offset_t);
			kaddr--;
		}
		if ((*(long*)kaddr == 0) ^ (flag  == 0)) {
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
	vm_offset_t vaddr, kaddr;

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
	p = (char *)kaddr + sizeof(vm_offset_t);
	for (vaddr += sizeof(int); vaddr < DB_USER_STACK_ADDR && n > 0; 
							vaddr++, p++, n--) {
		if (vaddr % ALPHA_PGBYTES == 0) {
			(void)kdb_vtophys(task->map, vaddr, &kaddr, FALSE);
			p = (char*)kaddr;
		}
		db_printf("%c", (*p < ' ' || *p > '~')? ' ': *p);
	}
	while (n-- >= 0)	/* compare with >= 0 for one more space */
		db_printf(" ");
}
