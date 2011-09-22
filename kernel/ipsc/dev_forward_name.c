/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * Copyright 1988, 1989, 1990, 1991 by Intel Corporation,
 * Santa Clara, California.
 * 
 *                          All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and that
 * both the copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Intel not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 * 
 * INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
 * SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN ACTION OF CONTRACT, NEGLIGENCE, OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */
/*
 * HISTORY
 * $Log:	dev_forward_name.c,v $
 * Revision 2.7  91/12/10  16:32:30  jsb
 * 	Fixes from Intel
 * 	[91/12/10  15:33:38  jsb]
 * 
 * Revision 2.6  91/08/03  18:18:42  jsb
 * 	Removed norma conditionals; use conf/file technology instead.
 * 	[91/07/27  18:16:09  jsb]
 * 
 * Revision 2.5  91/06/18  20:52:48  jsb
 * 	New copyright from Intel.
 * 	[91/06/18  19:05:03  jsb]
 * 
 * Revision 2.4  91/06/17  15:46:43  jsb
 * 	Renamed NORMA conditionals. Modified for non-integrated I/O nodes.
 * 	[91/06/17  13:48:13  jsb]
 * 
 * Revision 2.3  91/06/06  17:06:32  jsb
 * 	Add entries to compensate for ux's generation of names like sd0i
 * 	when it should be generating names like sd1a.
 * 	(This is a stopgap, not a real fix.)
 * 	[91/05/13  17:29:24  jsb]
 * 
 * Revision 2.2.1.1  90/12/17  23:09:42  jsb
 * 	Add entries to compensate for ux's generation of names like sd0i
 * 	when it should be generating names like sd1a.
 * 	(This is a stopgap, not a real fix.)
 * 
 * Revision 2.2  90/12/14  11:01:43  jsb
 * 	First checkin.
 * 	[90/12/14  09:37:57  jsb]
 * 
 */

struct dev_forward_entry {
	char *	global_name;
	char *	local_name;
} dev_forward_table[] = {
	"sd0a",		"<130>sd0a",
	"sd0b",		"<130>sd0b",
	"sd0c",		"<130>sd0c",
	"sd0d",		"<130>sd0d",
	"sd0e",		"<130>sd0e",
	"sd0f",		"<130>sd0f",
	"sd0g",		"<130>sd0g",
	"sd0h",		"<130>sd0h",

	"sd0i",		"<130>sd0a",
	"sd0j",		"<130>sd0b",
	"sd0k",		"<130>sd0c",
	"sd0l",		"<130>sd0d",
	"sd0m",		"<130>sd0e",
	"sd0n",		"<130>sd0f",
	"sd0o",		"<130>sd0g",
	"sd0p",		"<130>sd0h",

	"sd1a",		"<130>sd0a",
	"sd1b",		"<130>sd0b",
	"sd1c",		"<130>sd0c",
	"sd1d",		"<130>sd0d",
	"sd1e",		"<130>sd0e",
	"sd1f",		"<130>sd0f",
	"sd1g",		"<130>sd0g",
	"sd1h",		"<130>sd0h",

	"md0a",		"<131>md0a",

	"cnp0",		"<131>cnp0",

	0,		0,
};

char *
dev_forward_name(name, namebuf, namelen)
	char *name;
	char *namebuf;
	int namelen;
{
	struct dev_forward_entry *dfe;
	
	for (dfe = &dev_forward_table[0]; dfe->global_name; dfe++) {
		if (! strcmp(name, dfe->global_name)) {
			strncpy(namebuf, dfe->local_name, namelen);
			return namebuf;
		}
	}
	return name;
}
