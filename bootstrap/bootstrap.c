/* 
 * Mach Operating System
 * Copyright (c) 1992,1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	bootstrap.c,v $
 * Revision 2.10.1.2  94/05/21  13:32:57  rvb
 * 	Temporarily, define __main() ALWAYS.
 * 
 * Revision 2.10.1.1  93/11/29  09:48:23  rvb
 * 	For __386BSD__: define __main()
 * 	[93/10/12            rvb]
 * 
 * Revision 2.10  93/05/10  19:39:52  rvb
 * 	Changed "" includes to <>
 * 	[93/04/30            mrt]
 * 
 * Revision 2.9  93/05/10  17:44:22  rvb
 * 	Include files specified with quotes dont work properly
 * 	when the C file in in the master directory but the
 * 	include file is in the shadow directory. Change to
 * 	using angle brackets.
 * 	Ian Dall <DALL@hfrd.dsto.gov.au>	4/28/93
 * 	[93/05/10  13:12:37  rvb]
 * 
 * Revision 2.8  93/01/14  17:08:39  danner
 * 	Fixed atoi to match ANSI prototype.
 * 	[92/06/30            pds]
 * 	Alpha needs a bigger stack, apparently.
 * 	[92/11/30            af]
 * 
 * Revision 2.7  92/03/03  09:29:37  rpd
 * 	Added default_pager_bootstrap_port and default_pager_exception_port.
 * 	Added partition_init.
 * 	Removed bootstrap request handling.
 * 	[92/03/03            rpd]
 * 
 * Revision 2.6  92/02/20  13:42:19  elf
 * 	Fixed misleading comment.
 * 	[92/02/19  17:33:43  af]
 * 
 * 	Changed '-a' switch into '-q'.
 * 	[92/02/10  17:53:27  af]
 * 
 * Revision 2.5  92/02/19  16:45:24  elf
 * 	Changed '-a' switch into '-q'.
 * 	[92/02/10  17:53:27  af]
 * 
 * Revision 2.4  92/02/19  15:05:57  elf
 * 	Changed gets to safe_gets.
 * 	Added printf_init, panic_init calls.
 * 	[92/02/11            rpd]
 * 
 * Revision 2.3  92/01/14  16:43:07  rpd
 * 	Fixed to deallocate the ports from vm_region,
 * 	task_create, and thread_create.
 * 	[92/01/14            rpd]
 * 	Changed <mach/mach.h> to <mach.h>.
 * 	[92/01/07  13:43:37  rpd]
 * 
 * Revision 2.2  92/01/03  19:55:29  dbg
 * 	Close server directory after it has been found.  Add server
 * 	directory name to command line passed to startup program.
 * 	[91/09/25            dbg]
 * 
 * 	Set stack size to 8 K bytes.
 * 	[91/08/28            dbg]
 * 
 * 	Ask for mach_servers directory if -a switch supplied.
 * 	[91/08/09            dbg]
 * 
 * 	Rewrite to run out of kernel.
 * 	Initialize default pager (to get default pager port) before
 * 	running first user task.
 * 	[91/04/24            dbg]
 * 
 * Revision 2.8  91/02/05  17:25:42  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:11:22  mrt]
 * 
 * Revision 2.7  90/12/14  11:01:58  jsb
 * 	Changes to MACH_CLBOOT support. Use real device port, not a proxy;
 * 	new device forwarding code handles forwarding of requests.
 * 	Have slave not bother starting bootstrap task if there is nothing
 * 	for it to run.
 * 	[90/12/13  21:37:57  jsb]
 * 
 * Revision 2.6  90/09/28  16:55:30  jsb
 * 	Added MACH_CLBOOT support.
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

#include <mach.h>
#include <mach/message.h>

#include <file_io.h>

/*
 *	Use 8 Kbyte stacks instead of the default 64K.
 *	Use 4 Kbyte waiting stacks instead of the default 8K.
 */
#if	defined(alpha)
vm_size_t	cthread_stack_size = 16 * 1024;
#else
vm_size_t	cthread_stack_size = 8 * 1024;
#endif

extern
vm_size_t	cthread_wait_stack_size;

mach_port_t	bootstrap_master_device_port;	/* local name */
mach_port_t	bootstrap_master_host_port;	/* local name */

int	boot_load_program();

char	*root_name = "";
char	server_dir_name[MAXPATHLEN] = "mach_servers";

char	*startup_name = "startup";
char	*emulator_name = "emulator";
char	*slave_name = (char *) 0;

extern void	default_pager();
extern void	default_pager_initialize();
extern void	default_pager_setup();

/* initialized in default_pager_initialize */
extern mach_port_t default_pager_exception_port;
extern mach_port_t default_pager_bootstrap_port;

/*
 * Convert ASCII to integer.
 */
int atoi(str)
	register const char *str;
{
	register int	n;
	register int	c;
	int	is_negative = 0;

	n = 0;
	while (*str == ' ')
	    str++;
	if (*str == '-') {
	    is_negative = 1;
	    str++;
	}
	while ((c = *str++) >= '0' && c <= '9') {
	    n = n * 10 + (c - '0');
	}
	if (is_negative)
	    n = -n;
	return (n);
}

/*
 * Bootstrap task.
 * Runs in user space.
 *
 * Called as 'boot -switches host_port device_port root_name'
 *
 */
__main() {}
main(argc, argv)
	int	argc;
	char	**argv;
{
	register kern_return_t	result;
	task_t			user_task;
	thread_t		user_thread;

	task_t			my_task = mach_task_self();

	char			*flag_string;
	boolean_t		cluster_slave = FALSE;

	boolean_t		ask_server_dir = FALSE;

	/*
	 * Use 4Kbyte cthread wait stacks.
	 */
	cthread_wait_stack_size = 4 * 1024;

	/*
	 * Parse the arguments.
	 */
	if (argc < 5)
	    panic("Not enough arguments");

	/*
	 * Arg 0 is program name
	 */

	/*
	 * Arg 1 is flags
	 */
	if (argv[1][0] != '-') {
	    panic("No flags");
	}
	flag_string = argv[1];

	/*
	 * Arg 2 is host port number
	 */
	bootstrap_master_host_port = atoi(argv[2]);

	/*
	 * Arg 3 is device port number
	 */
	bootstrap_master_device_port = atoi(argv[3]);
 
	/*
	 * Arg 4 is root name
	 */
	root_name = argv[4];

	/*
	 * If there is an arg 5, it should be '-slave':
	 * if so, we are loading slave bootstrap on a slave
	 * node.
	 */
	if (argc > 5 && !strcmp(argv[5], "-slave"))
	    cluster_slave = TRUE;

	printf_init(bootstrap_master_device_port);
	panic_init(bootstrap_master_host_port);

	/*
	 * If the '-q' (query) switch was specified, ask for the
	 * server directory.
	 */

	(void) strbuild(server_dir_name,
			"/dev/",
			root_name,
			"/mach_servers",
			(char *)0);

	if (index(flag_string, 'q'))
	    ask_server_dir = TRUE;

	while (TRUE) {

	    struct file	f;

	    if (ask_server_dir) {
		char new_server_dir[MAXPATHLEN];

		printf("Server directory? [ %s ] ",
			server_dir_name);
		safe_gets(new_server_dir, sizeof(new_server_dir));
		if (new_server_dir[0] != '\0')
		    strcpy(server_dir_name, new_server_dir);
	    }

	    result = open_file(bootstrap_master_device_port,
			       server_dir_name,
			       &f);
	    if (result != 0) {
		printf("Can't open server directory %s: %d\n",
			server_dir_name,
			result);
		ask_server_dir = TRUE;
		continue;
	    }
	    if ((f.i_mode & IFMT) != IFDIR) {
		printf("%s is not a directory\n",
			server_dir_name);
		ask_server_dir = TRUE;
		continue;
	    }
	    /*
	     * Found server directory.
	     */
	    close_file(&f);
	    break;
	}

	/*
	 * Set up the default pager.
	 */
	partition_init();

	default_pager_setup(bootstrap_master_device_port,
			    server_dir_name);

	default_pager_initialize(bootstrap_master_host_port);

	/*
	 * task_set_exception_port and task_set_bootstrap_port
	 * both require a send right.
	 */
	(void) mach_port_insert_right(my_task, default_pager_bootstrap_port,
				      default_pager_bootstrap_port,
				      MACH_MSG_TYPE_MAKE_SEND);
	(void) mach_port_insert_right(my_task, default_pager_exception_port,
				      default_pager_exception_port,
				      MACH_MSG_TYPE_MAKE_SEND);

	/*
	 * Change our exception port.
	 */
	(void) task_set_exception_port(my_task, default_pager_exception_port);

	/*
	 * Create the user task and thread to run the startup file.
	 */
	result = task_create(my_task, FALSE, &user_task);
	if (result != KERN_SUCCESS)
	    panic("task_create %d", result);

	(void) task_set_exception_port(user_task,
				       default_pager_exception_port);
	(void) task_set_bootstrap_port(user_task,
				       default_pager_bootstrap_port);

	result = thread_create(user_task, &user_thread);
	if (result != KERN_SUCCESS)
	    panic("thread_create %d", result);

	/*
	 *	Deallocate the excess send rights.
	 */
	(void) mach_port_deallocate(my_task, default_pager_exception_port);
	(void) mach_port_deallocate(my_task, default_pager_bootstrap_port);

	/*
	 * Load the startup file.
	 * Pass it a command line of
	 * "startup -boot_flags root_name server_dir_name"
	 */
	result = boot_load_program(bootstrap_master_host_port,
				   bootstrap_master_device_port,
				   user_task,
				   user_thread,
				   server_dir_name,
				   (cluster_slave)
					? slave_name
					: startup_name,
				   flag_string,
				   root_name,
				   server_dir_name,
				   (char *)0);
	if (result != 0)
	    panic("boot_load_program %d", result);

	/*
	 * Read emulator symbol table.
	 * Startup symbol table was read inside boot_load_program.
	 */
	read_emulator_symbols(bootstrap_master_host_port,
			      bootstrap_master_device_port,
			      server_dir_name,
			      emulator_name);

	/*
	 * Start up the thread
	 */
	result = thread_resume(user_thread);
	if (result != KERN_SUCCESS)
	    panic("thread_resume %d", result);

	(void) mach_port_deallocate(my_task, user_task);
	(void) mach_port_deallocate(my_task, user_thread);

	{
	    /*
	     * Delete the old stack (containing only the arguments).
	     */
	    vm_offset_t	addr = (vm_offset_t) argv;

	    vm_offset_t		r_addr;
	    vm_size_t		r_size;
	    vm_prot_t		r_protection, r_max_protection;
	    vm_inherit_t	r_inheritance;
	    boolean_t		r_is_shared;
	    memory_object_name_t r_object_name;
	    vm_offset_t		r_offset;
	    kern_return_t	kr;

	    r_addr = addr;

	    kr = vm_region(my_task,
			&r_addr,
			&r_size,
			&r_protection,
			&r_max_protection,
			&r_inheritance,
			&r_is_shared,
			&r_object_name,
			&r_offset);
	    if ((kr == KERN_SUCCESS) && MACH_PORT_VALID(r_object_name))
		(void) mach_port_deallocate(my_task, r_object_name);
	    if ((kr == KERN_SUCCESS) &&
		(r_addr <= addr) &&
		((r_protection & (VM_PROT_READ|VM_PROT_WRITE)) ==
					(VM_PROT_READ|VM_PROT_WRITE)))
		(void) vm_deallocate(my_task, r_addr, r_size);
	}

	/*
	 * Become the default pager
	 */
	default_pager();
	/*NOTREACHED*/
}
