/* 
 * Mach Operating System
 * Copyright (c) 1992-1989 Carnegie Mellon University
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
 * $Log:	bootstrap.c,v $
 * Revision 2.31  93/03/26  17:54:41  mrt
 * 	Changed boot_map to be palatable to vm_object_page_map().
 * 	ANSIfied (except for varargs functions).
 * 	[93/03/23            af]
 * 
 * Revision 2.30  93/03/09  12:26:38  danner
 * 	String protos.
 * 	[93/03/07            af]
 * 
 * Revision 2.29  93/02/05  07:51:14  danner
 * 	Alpha like mips, has machdep init flags.
 * 	[93/02/04  01:56:09  af]
 * 
 * 	64bit cleanup.  Added a magic number to the boot label.
 * 	Changed ovbcopy_ints to be quicker where it can.
 * 	[92/11/30            af]
 * 
 * Revision 2.28  92/08/03  17:36:37  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.27  92/05/21  17:12:55  jfriedl
 * 	Removed unused variable 'port' from bootstrap_create().
 * 	Cleanup to quiet gcc warnings.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.26  92/05/04  11:26:09  danner
 * 	Set load_bootstrap_symbols from bootstrap_symbols option file. 
 * 	[92/05/03            danner]
 * 
 * Revision 2.25  92/04/01  19:33:06  rpd
 * 	Fixed ovbcopy_ints to handle zero size.
 * 	[92/03/13            rpd]
 * 
 * Revision 2.24  92/02/26  13:12:51  elf
 * 	Added protect in copy_bootstrap against zero size bss. 
 * 	[92/02/26            danner]
 * 
 * Revision 2.23  92/02/19  16:46:30  elf
 * 	Change -a switch into -q switch.
 * 	Do not load the default-pager's symtable by default,
 * 	it gets in the way of debugging UX.
 * 	[92/02/10  17:50:23  af]
 * 
 * Revision 2.22  92/02/18  18:00:12  elf
 * 	Added global load_fault_in_text to force the faulting in of the
 * 	bootstrap task text. Useful for debugging.
 * 	[92/02/14            danner]
 * 
 * Revision 2.21  92/01/03  20:13:43  dbg
 * 	Move bootstrap code out to user space.
 * 	Mac2 and iPSC-dependent code must be moved there also.
 * 	[91/12/18            dbg]
 * 
 * Revision 2.20  91/12/10  16:32:40  jsb
 * 	Fixes from Intel
 * 	[91/12/10  15:51:50  jsb]
 * 
 * Revision 2.19  91/11/12  11:51:53  rvb
 * 	Added task_insert_send_right.
 * 	Changed BOOTSTRAP_MAP_SIZE to 4 meg.
 * 	[91/11/12            rpd]
 * 
 * Revision 2.18  91/09/12  16:37:49  bohman
 * 	Made bootstrap task call mac2 machine dependent code before running
 * 	'startup', which is loaded from the UX file system.  This needs to
 * 	be handled more generally in the future.
 * 	[91/09/11  17:07:59  bohman]
 * 
 * Revision 2.17  91/08/28  11:14:22  jsb
 * 	Changed msgh_kind to msgh_seqno.
 * 	[91/08/10            rpd]
 * 
 * Revision 2.16  91/08/03  18:18:45  jsb
 * 	Moved bootstrap query earlier. Removed all NORMA specific code.
 * 	[91/07/25  18:25:35  jsb]
 * 
 * Revision 2.15  91/07/31  17:44:14  dbg
 * 	Pass host port to boot_load_program and read_emulator_symbols.
 * 	[91/07/30  17:02:40  dbg]
 * 
 * Revision 2.14  91/07/01  08:24:54  jsb
 * 	Removed notion of master/slave. Instead, refuse to start up
 * 	a bootstrap task whenever startup_name is null.
 * 	[91/06/29  16:48:14  jsb]
 * 
 * Revision 2.13  91/06/19  11:55:57  rvb
 * 	Ask for startup program to override default.
 * 	[91/06/18  21:39:17  rvb]
 * 
 * Revision 2.12  91/06/17  15:46:51  jsb
 * 	Renamed NORMA conditionals.
 * 	[91/06/17  10:49:04  jsb]
 * 
 * Revision 2.11  91/06/06  17:06:53  jsb
 * 	Allow slaves to act as masters (for now).
 * 	[91/05/13  17:36:17  jsb]
 * 
 * Revision 2.10  91/05/18  14:31:32  rpd
 * 	Added argument to kernel_thread.
 * 	[91/04/03            rpd]
 * 
 * Revision 2.9  91/05/14  16:40:06  mrt
 * 	Correcting copyright
 * 
 * Revision 2.8  91/02/05  17:25:42  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:11:22  mrt]
 * 
 * Revision 2.7  90/12/14  11:01:58  jsb
 * 	Changes to NORMA_BOOT support. Use real device port, not a proxy;
 * 	new device forwarding code handles forwarding of requests.
 * 	Have slave not bother starting bootstrap task if there is nothing
 * 	for it to run.
 * 	[90/12/13  21:37:57  jsb]
 * 
 * Revision 2.6  90/09/28  16:55:30  jsb
 * 	Added NORMA_BOOT support.
 * 	[90/09/28  14:04:43  jsb]
 * 
 * Revision 2.5  90/06/02  14:53:39  rpd
 * 	Load emulator symbols.
 * 	[90/05/11  16:58:37  rpd]
 * 
 * 	Made bootstrap_task available externally.
 * 	[90/04/05            rpd]
 * 	Converted to new IPC.
 * 	[90/03/26  22:03:39  rpd]
 * 
 * Revision 2.4  90/01/11  11:43:02  dbg
 * 	Initialize bootstrap print routines.  Remove port number
 * 	printout.
 * 	[89/12/20            dbg]
 * 
 * Revision 2.3  89/11/29  14:09:01  af
 * 	Enlarged the bootstrap task's map to accomodate some unnamed
 * 	greedy RISC box.  Sigh.
 * 	[89/11/07            af]
 * 	Made root_name and startup_name non-preallocated, so that
 * 	they can be changed at boot time on those machines like
 * 	mips and Sun where the boot loader passes command line
 * 	arguments to the kernel.
 * 	[89/10/28            af]
 * 
 * Revision 2.2  89/09/08  11:25:02  dbg
 * 	Pass root partition name to default_pager_setup.
 * 	[89/08/31            dbg]
 * 
 * 	Assume that device service has already been created.
 * 	Create bootstrap task here and give it the host and
 * 	device ports.
 * 	[89/08/01            dbg]
 * 
 * 	Call default_pager_setup.
 * 	[89/07/11            dbg]
 * 
 * 12-Apr-89  David Golub (dbg) at Carnegie-Mellon University
 *	Removed console_port.
 *
 */
/*
 * Bootstrap the various built-in servers.
 */
#include <mach_kdb.h>
#include <bootstrap_symbols.h>

#include <mach/port.h>
#include <mach/message.h>
#include <mach/vm_param.h>
#include <ipc/ipc_port.h>
#include <kern/host.h>
#include <kern/strings.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <vm/vm_kern.h>
#include <device/device_port.h>

#include <sys/varargs.h>

#include <mach/boot_info.h>

#if	MACH_KDB
#include <machine/db_machdep.h>
#include <ddb/db_sym.h>
#endif

/*
 *	Bootstrap image is moved out of BSS at startup.
 */

vm_offset_t	boot_start = 0;		/* pointer to bootstrap image */
vm_size_t	boot_size = 0;		/* size of bootstrap image */
vm_offset_t	load_info_start = 0;	/* pointer to bootstrap load info */
vm_size_t	load_info_size = 0;	/* size of bootstrap load info */
vm_offset_t	kern_sym_start = 0;	/* pointer to kernel symbols */
vm_size_t	kern_sym_size = 0;	/* size of kernel symbols */

#if	DEBUG
load_info_print()
{
	struct loader_info *lp = (struct loader_info *)load_info_start;

	printf("Load info: text (%#x, %#x, %#x)\n",
		lp->text_start, lp->text_size, lp->text_offset);
	printf("           data (%#x, %#x, %#x)\n",
		lp->data_start, lp->data_size, lp->data_offset);
	printf("           bss  (%#x)\n", lp->bss_size);
	printf("           syms (%#x, %#x)\n",
		lp->sym_offset, lp->sym_size);
	printf("	   entry(%#x, %#x)\n",
		lp->entry_1, lp->entry_2);
}
#endif

/*
 *	Moves kernel symbol table, bootstrap image, and bootstrap
 *	load information out of BSS at startup.  Returns the
 *	first unused address.
 *
 *	PAGE_SIZE must be set.
 *
 *	On some machines, this code must run before the page
 *	tables are set up, and therefore must be re-created
 *	in assembly language.
 */

void
ovbcopy_ints(
	vm_offset_t src,
	vm_offset_t dst,
	vm_size_t   size)
{
	register vm_size_t *srcp;
	register vm_size_t *dstp;
	register unsigned int count;

	srcp = (vm_size_t *)(src + size);
	dstp = (vm_size_t *)(dst + size);
	count = size / sizeof(vm_size_t);

	while (count-- != 0)
	    *--dstp = *--srcp;
}

extern char	edata[];	/* start of BSS */
extern char	end[];		/* end of BSS */

vm_offset_t
move_bootstrap()
{
	register struct boot_info *bi = (struct boot_info *)edata;

	/*
	 * Tolerate some "imprecision" in a certain linker
	 */
	if (bi->magic_number != MACH_BOOT_INFO_MAGIC) {
		register vm_size_t	*addr, *erange;

		addr   = (vm_size_t *)edata;
		erange = (vm_size_t *)(edata + PAGE_SIZE);
		while (addr < erange) {
			bi = (struct boot_info *) ++addr;
			if (bi->magic_number == MACH_BOOT_INFO_MAGIC)
				break;
		}
		if (bi->magic_number != MACH_BOOT_INFO_MAGIC)
			return 0;	/* good luck.. */
	}

	kern_sym_start = (vm_offset_t) end;
	kern_sym_size  = bi->sym_size;
	/*
	 * Align start of bootstrap on page boundary,
	 * to allow mapping into user space.
	 */
	boot_start = round_page(kern_sym_start + kern_sym_size);
	boot_size  = bi->boot_size;
	load_info_start = boot_start + boot_size;
	load_info_size  = bi->load_info_size;

	ovbcopy_ints((vm_offset_t)bi + sizeof(struct boot_info) + kern_sym_size,
		     boot_start,
		     boot_size + load_info_size);

	ovbcopy_ints((vm_offset_t)bi + sizeof(struct boot_info),
		     kern_sym_start,
		     kern_sym_size);

	return boot_start + boot_size + load_info_size;
}

/*

 */
mach_port_t	boot_device_port;	/* local name */
mach_port_t	boot_host_port;		/* local name */

void	user_bootstrap();	/* forward */

extern char	*root_name;

mach_port_t
task_insert_send_right(
	task_t task,
	ipc_port_t port)
{
	mach_port_t name;

	for (name = 1;; name++) {
		kern_return_t kr;

		kr = mach_port_insert_right(task->itk_space, name,
			    (ipc_object_t)port, MACH_MSG_TYPE_PORT_SEND);
		if (kr == KERN_SUCCESS)
			break;
		assert(kr == KERN_NAME_EXISTS);
	}

	return name;
}

void bootstrap_create()
{
	task_t		bootstrap_task;
	thread_t	bootstrap_thread;

	if (boot_size == 0) {
		printf("Not starting bootstrap task.\n");
		return;
	}

	/*
	 * Create the bootstrap task.
	 */

	(void) task_create(TASK_NULL, FALSE, &bootstrap_task);
	(void) thread_create(bootstrap_task, &bootstrap_thread);

	/*
	 * Insert send rights to the master host and device ports.
	 */

	boot_host_port =
		task_insert_send_right(bootstrap_task,
			ipc_port_make_send(realhost.host_priv_self));

	boot_device_port =
		task_insert_send_right(bootstrap_task,
			ipc_port_make_send(master_device_port));

	/*
	 * Start the bootstrap thread.
	 */
	thread_start(bootstrap_thread, user_bootstrap);
	(void) thread_resume(bootstrap_thread);
}

/*
 * The following code runs as the kernel mode portion of the
 * first user thread.
 */

/*
 * Convert an unsigned integer to its decimal representation.
 */
void
itoa(
	char		*str,
	vm_size_t	num)
{
	char	buf[sizeof(vm_size_t)*2+3];
	register char *np;

	np = buf + sizeof(buf);
	*--np = 0;

	do {
	    *--np = '0' + num % 10;
	    num /= 10;
	} while (num != 0);

	strcpy(str, np);
}

/*
 * Parse the boot flags into an argument string.
 * Format as a standard flag argument: '-qsdn...'
 */
#include <sys/reboot.h>

static void
get_boot_flags(
	char	str[])	/* OUT */
{
	register char *cp;
	register int	bflag;

#if	defined(mips) || defined(alpha)

	extern char *machine_get_boot_flags();
	cp = machine_get_boot_flags(str);
#else
	cp = str;
	*cp++ = '-';
#endif

	bflag = boothowto;

	if (bflag & RB_ASKNAME)
	    *cp++ = 'q';
	if (bflag & RB_SINGLE)
	    *cp++ = 's';
#if	MACH_KDB
	if (bflag & RB_KDB)
	    *cp++ = 'd';
#endif	MACH_KDB
	if (bflag & RB_INITNAME)
	    *cp++ = 'n';

	if (cp == &str[1])	/* no flags */
	    *cp++ = 'x';
	*cp = '\0';
}

/*
 * Copy boot_data (executable) to the user portion of this task.
 */
boolean_t	load_protect_text = TRUE;
#if MACH_KDB
		/* if set, fault in the text segment */
boolean_t	load_fault_in_text = TRUE;
#endif

vm_offset_t
boot_map(
	void *		data,	/* private data */
	vm_offset_t	offset)	/* offset to map */
{
	vm_offset_t	start_offset = (vm_offset_t) data;

	return pmap_extract(kernel_pmap, start_offset + offset);
}


#if BOOTSTRAP_SYMBOLS
boolean_t load_bootstrap_symbols = TRUE;
#else
boolean_t load_bootstrap_symbols = FALSE;
#endif



void
copy_bootstrap(
	vm_offset_t		*entry)
{
	struct loader_info	*lp;
	vm_offset_t		text_page_start,
				text_page_end,
				data_page_start,
				bss_start,
				bss_page_start,
				bss_page_end;

	register vm_map_t	user_map = current_task()->map;
	vm_object_t		boot_object;
	vm_size_t		bss_size;

	/*
	 * Point to bootstrap load information.
	 */
	lp = (struct loader_info *)load_info_start;

	/*
	 * We assume that makeboot has aligned the various
	 * pieces of the bootstrap image on page boundaries.
	 */
	assert(lp->text_start == trunc_page(lp->text_start));
	assert(lp->data_start == trunc_page(lp->data_start));
	assert(lp->text_offset == trunc_page(lp->text_offset));
	assert(lp->data_offset == trunc_page(lp->data_offset)
	    || lp->data_offset == lp->text_offset + lp->text_size);

	/*
	 * Find how much virtual space we have to allocate.
	 */
	text_page_start = trunc_page(lp->text_start);
	text_page_end   = round_page(lp->text_start + lp->text_size);
	data_page_start = trunc_page(lp->data_start);
	bss_start	= lp->data_start + lp->data_size;
	bss_page_start  = trunc_page(bss_start);
	bss_page_end    = round_page(bss_start + lp->bss_size);
	bss_size	= bss_page_end - bss_page_start;


	/*
	 * Create an object that maps the pages in the
	 * bootstrap image.  Map only until the end of the last
	 * whole page.
	 */
	boot_size = lp->data_offset + bss_page_start - lp->data_start;
	boot_object = vm_object_allocate(boot_size);
	vm_object_page_map(boot_object,
			   (vm_offset_t) 0,	/* from boot_start */
			   boot_size,
			   boot_map,
			   (char *)boot_start);

	/*
	 * Map the text and data from the boot image into
	 * the user task.  Map the data area only through
	 * the last whole page of data - the next page of
	 * data is split between data and bss, and must be
	 * partially cleared.
	 */

	if (text_page_end >= data_page_start) {
	    /*
	     * One contiguous area for text and data.
	     */
	    (void) vm_map_enter(user_map,
			&text_page_start,
			(vm_size_t) (bss_page_start - text_page_start),
			(vm_offset_t) 0, FALSE,
			boot_object,
			lp->text_offset,
			FALSE,
			VM_PROT_READ | VM_PROT_WRITE,
			VM_PROT_ALL,
			VM_INHERIT_DEFAULT);

	}
	else {
	    /*
	     * Separated text and data areas.
	     */
	    (void) vm_map_enter(user_map,
			&text_page_start,
			(vm_size_t)(text_page_end - text_page_start),
			(vm_offset_t) 0, FALSE,
			boot_object,
			lp->text_offset,
			FALSE,
			VM_PROT_READ | VM_PROT_WRITE,
			VM_PROT_ALL,
			VM_INHERIT_DEFAULT);

	    (void) vm_map_enter(user_map,
			&data_page_start,
			(vm_size_t)(bss_page_start - data_page_start),
			(vm_offset_t) 0, FALSE,
			boot_object,
			lp->data_offset,
			FALSE,
			VM_PROT_READ | VM_PROT_WRITE,
			VM_PROT_ALL,
			VM_INHERIT_DEFAULT);
	}


	/*
	 * Allocate the remainder of the data segment and all
	 * of the BSS. Protect against zero size bss.
	 */
	if (bss_size)
	  {
	    (void) vm_allocate(user_map, &bss_page_start, 
			       (vm_size_t)(bss_size), FALSE);

	    /*
	     * If the data segment does not end on a VM page boundary,
	     * we copy the end of the data segment onto a new page
	     * so that the bss segment will be zero, and so that
	     * we do not overwrite the bootstrap symbol table.
	     */
	    if (bss_start > bss_page_start) {
	      (void) copyout((char *) boot_start +
			     lp->data_offset +
			     lp->data_size -
			     (bss_start - bss_page_start),
			     (char *)bss_page_start,
			     bss_start - bss_page_start);
	    }
	  } 

	/*
	 * Protect the text.
	 */
	if (load_protect_text)
	    (void) vm_protect(user_map,
			text_page_start,
			(vm_size_t)(trunc_page(lp->text_start+lp->text_size)
				- text_page_start),
			FALSE,
			VM_PROT_READ|VM_PROT_EXECUTE);

#if	MACH_KDB
	/*
	 * Enter the bootstrap symbol table.
	 */

	if (load_bootstrap_symbols)
	(void) X_db_sym_init(
		(char*) boot_start+lp->sym_offset,
		(char*) boot_start+lp->sym_offset+lp->sym_size,
		"bootstrap",
		(char *) user_map);

	if (load_fault_in_text)
	  {
	    vm_offset_t lenp = round_page(lp->text_start+lp->text_size) -
	      		     trunc_page(lp->text_start);
	    vm_offset_t i = 0;

	    while (i < lenp)
	      {
		vm_fault(user_map, text_page_start +i, 
		        load_protect_text ?  
			 VM_PROT_READ|VM_PROT_EXECUTE :
			 VM_PROT_READ|VM_PROT_EXECUTE | VM_PROT_WRITE,
			 0,0,0);
		i = round_page (i+1);
	      }
	  }


#endif	MACH_KDB
	/*
	 * Return the entry points.
	 */
	entry[0] = lp->entry_1;
	entry[1] = lp->entry_2;
}

/*
 * Allocate the stack, and build the argument list.
 */
extern vm_offset_t	user_stack_low();
extern vm_offset_t	set_user_regs();

void
build_args_and_stack(entry, va_alist)
	vm_offset_t	entry;
	va_dcl
{
	vm_offset_t	stack_base;
	vm_size_t	stack_size;
	va_list		argv_ptr;
	register
	char *		arg_ptr;
	int		arg_len;
	int		arg_count;
	register
	char *		arg_pos;
	int		arg_item_len;
	char *		string_pos;
	char *		zero = (char *)0;

#define	STACK_SIZE	(64*1024)

	/*
	 * Calculate the size of the argument list.
	 */
	va_start(argv_ptr);
	arg_len = 0;
	arg_count = 0;
	for (;;) {
	    arg_ptr = va_arg(argv_ptr, char *);
	    if (arg_ptr == 0)
		break;
	    arg_count++;
	    arg_len += strlen(arg_ptr) + 1;
	}
	va_end(argv_ptr);

	/*
	 * Add space for:
	 *	arg count
	 *	pointers to arguments
	 *	trailing 0 pointer
	 *	dummy 0 pointer to environment variables
	 *	and align to integer boundary
	 */
	arg_len += sizeof(integer_t)
		 + (2 + arg_count) * sizeof(char *);
	arg_len = (arg_len + sizeof(integer_t) - 1) & ~(sizeof(integer_t)-1);

	/*
	 * Allocate the stack.
	 */
	stack_size = round_page(STACK_SIZE);
	stack_base = user_stack_low(stack_size);
	(void) vm_allocate(current_task()->map,
			&stack_base,
			stack_size,
			FALSE);

	arg_pos = (char *)
		set_user_regs(stack_base, stack_size, entry, arg_len);

	/*
	 * Start the strings after the arg-count and pointers
	 */
	string_pos = arg_pos
		+ sizeof(integer_t)
		+ arg_count * sizeof(char *)
		+ 2 * sizeof(char *);

	/*
	 * first the argument count
	 */
	(void) copyout((char *)&arg_count,
			arg_pos,
			sizeof(integer_t));
	arg_pos += sizeof(integer_t);

	/*
	 * Then the strings and string pointers for each argument
	 */
	va_start(argv_ptr);
	while (--arg_count >= 0) {
	    arg_ptr = va_arg(argv_ptr, char *);
	    arg_item_len = strlen(arg_ptr) + 1; /* include trailing 0 */

	    /* set string pointer */
	    (void) copyout((char *)&string_pos,
			arg_pos,
			sizeof (char *));
	    arg_pos += sizeof(char *);

	    /* copy string */
	    (void) copyout(arg_ptr, string_pos, arg_item_len);
	    string_pos += arg_item_len;
	}
	va_end(argv_ptr);

	/*
	 * last, the trailing 0 argument and a null environment pointer.
	 */
	(void) copyout((char *)&zero, arg_pos, sizeof(char *));
	arg_pos += sizeof(char *);
	(void) copyout((char *)&zero, arg_pos, sizeof(char *));
}

void user_bootstrap()
{
	vm_offset_t	entry[2];

	char	host_string[12];
	char	device_string[12];
	char	flag_string[12];

	/*
	 * Copy the bootstrap code from boot_data to the user task.
	 */
	copy_bootstrap(entry);

	/*
	 * Convert the host and device ports to strings,
	 * to put in the argument list.
	 */
	itoa(host_string, boot_host_port);
	itoa(device_string, boot_device_port);

	/*
	 * Get the boot flags, also
	 */
	get_boot_flags(flag_string);

	/*
	 * Build the argument list and insert in the user task.
	 * Argument list is
	 * "bootstrap -<boothowto> <host_port> <device_port> <root_name>"
	 */
	build_args_and_stack(entry,
			"bootstrap",
			flag_string,
			host_string,
			device_string,
			root_name,
			(char *)0);

	/*
	 * Exit to user thread.
	 */
	thread_bootstrap_return();
	/*NOTREACHED*/
}

