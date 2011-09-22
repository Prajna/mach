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
 * $Log:	def_pager_setup.c,v $
 * Revision 2.10  93/05/10  19:40:09  rvb
 * 	Changed "" includes to <>
 * 	[93/04/30            mrt]
 * 
 * Revision 2.9  93/05/10  17:44:25  rvb
 * 	Include files specified with quotes dont work properly
 * 	when the C file in in the master directory but the
 * 	include file is in the shadow directory. Change to
 * 	using angle brackets.
 * 	Ian Dall <DALL@hfrd.dsto.gov.au>	4/28/93
 * 	[93/05/10  13:13:04  rvb]
 * 
 * Revision 2.8  93/03/09  10:47:39  danner
 * 	Added extern of kalloc().
 * 	[93/03/05            af]
 * 
 * Revision 2.7  92/02/23  22:55:46  elf
 * 	Removed in kernel history, corrected copyright, condensed out of 
 * 	kernel history.
 * 	[92/02/23            elf]
 * 
 * Revision 2.6  92/02/23  22:25:32  elf
 * 	Moved printouts elsewhere.
 * 	[92/02/23  13:17:52  af]
 * 
 * 	Reflected change in open_file_direct, to page on unstructured
 * 	devices.
 * 	[92/02/22  18:54:38  af]
 * 
 * Revision 2.5  92/02/20  13:42:22  elf
 * 	Added add_paging_file and remove_paging_file.  Not sure this
 * 	was the best place, but here is where the previous code resided.
 * 	[92/02/19  17:32:56  af]
 * 
 * Condensed history:
 * 	Changed gets to safe_gets. (rpd)
 * 	Changed <mach/mach.h> to <mach.h>. (rpd)
 * 	Use server_dir_name instead of root_name. (dbg)
 * 	Convert to run outside of kernel. (dbg)
 * 	Return whether there is a paging partition. (dbg)
 * 
 *
 */
#include <mach.h>

#include <file_io.h>

extern void *kalloc();

/*
 * Create a paging partition given a file name
 */
extern void	create_paging_partition();

kern_return_t
add_paging_file(master_device_port, file_name)
	mach_port_t		master_device_port;
	char			*file_name;
{
	register struct file_direct *fdp;
	register kern_return_t	result;
	struct file     	pfile;
	boolean_t		isa_file;

	bzero((char *) &pfile, sizeof(struct file));

	result = open_file(master_device_port, file_name, &pfile);
	if (result != KERN_SUCCESS)
		return result;

	fdp = (struct file_direct *) kalloc(sizeof *fdp);
	bzero((char *) fdp, sizeof *fdp);

	isa_file = file_is_structured(&pfile);

	result = open_file_direct(pfile.f_dev, fdp, isa_file);
	if (result)
		panic("Can't open paging file %s\n", file_name);

	result = add_file_direct(fdp, &pfile);
	if (result)
		panic("Can't read disk addresses: %d\n", result);

	close_file(&pfile);

	/*
	 * Set up the default paging partition 
	 */
	create_paging_partition(file_name,
				fdp->fd_size * fdp->fd_bsize,
				page_read_file_direct,
				page_write_file_direct,
				(char *) fdp,
				isa_file);

	return result;
}

/*
 * Destroy a paging_partition given a file name
 */
kern_return_t
remove_paging_file(file_name)
	char			*file_name;
{
	struct file_direct	*fdp = 0;
	kern_return_t		kr;

	kr = destroy_paging_partition(file_name, &fdp);
	if (kr == KERN_SUCCESS) {
		remove_file_direct(fdp);
		kfree(fdp, sizeof(*fdp));
	}
	return kr;
}

/*
 * Set up default pager
 */
extern char *strbuild();

boolean_t
default_pager_setup(master_device_port, server_dir_name)
	mach_port_t master_device_port;
	char	*server_dir_name;
{
	register kern_return_t	result;

	char	paging_file_name[MAXPATHLEN+1];

	(void) strbuild(paging_file_name,
			server_dir_name,
			"/paging_file",
			(char *)0);

	while (TRUE) {
	    result = add_paging_file(master_device_port, paging_file_name);
	    if (result == KERN_SUCCESS)
		break;
	    printf("Can't open paging file %s: %d\n",
		   paging_file_name, 
		   result);

	    bzero(paging_file_name, sizeof(paging_file_name));
	    printf("Paging file name ? ");
	    safe_gets(paging_file_name, sizeof(paging_file_name));

	    if (paging_file_name[0] == 0) {
		printf("*** WARNING: running without paging area!\n");
		return FALSE;
	    }
	}

	/*
	 * Our caller will become the default pager - later
	 */

	return TRUE;
}
