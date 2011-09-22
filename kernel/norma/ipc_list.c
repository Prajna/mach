/* 
 * Mach Operating System
 * Copyright (c) 1991,1992 Carnegie Mellon University
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
 * $Log:	ipc_list.c,v $
 * Revision 2.3  93/05/15  19:35:22  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.2  92/03/10  16:27:38  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:49:22  jsb]
 * 
 * Revision 2.1.1.7.1.5  92/02/22  10:44:15  jsb
 * 	Temporary workaround: changed "not tabled" panic to a printf.
 * 
 * Revision 2.1.1.7.1.4  92/02/21  11:24:24  jsb
 * 	Added new routine norma_port_remove_try.
 * 	[92/02/20  17:15:49  jsb]
 * 
 * 	Added db_show_{uid,all_uids}_verbose.
 * 	[92/02/20  10:33:47  jsb]
 * 
 * Revision 2.1.1.7.1.3  92/01/21  21:51:19  jsb
 * 	Now uses hash table. Now uses netipc lock instead of spls.
 * 	Removed seqno debugging hack. Added norma_port_iterate and
 * 	reimplemented db_show_uid (nee norma_list_all_ports) with it.
 * 	Added support for show all {proxies, principals}. Use kdbprintf
 * 	instead of printf for these routines for pagination.
 * 	[92/01/16  22:46:09  jsb]
 * 
 * Revision 2.1.1.7.1.2  92/01/03  16:37:27  jsb
 * 	Use ipc_port_release instead of ip_release to allow port deallocation.
 * 	[91/12/31  21:42:40  jsb]
 * 
 * 	Added norma_port_tabled function.
 * 	[91/12/31  17:17:02  jsb]
 * 
 * 	In norma_port_remove, refuse to remove a proxy that started out as a
 * 	principal, since our forwarding algorithms require it to stay around.
 * 	[91/12/31  12:19:08  jsb]
 * 
 * 	Hacked norma_new_uid to incorporate incarnation in generated uids.
 * 	Threw in a stub for norma_ipc_cleanup_incarnation.
 * 	[91/12/29  16:08:32  jsb]
 * 
 * 	Lots of ugly but useful debugging support.
 * 	[91/12/28  18:45:37  jsb]
 * 
 * 	Made norma_list_all_ports print port addresses as well as uids.
 * 	[91/12/25  16:57:46  jsb]
 * 
 * 	First checkin. Split from norma/ipc_transit.c.
 * 	[91/12/24  14:26:58  jsb]
 * 
 * Revision 2.1.1.7.1.1  92/01/03  08:54:06  jsb
 * 	First NORMA branch checkin.
 * 
 * 
 */
/*
 *	File:	norma/ipc_list.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Maintains list of norma ports.
 */

#include <machine/machspl.h>
#include <ipc/ipc_port.h>
#include <norma/ipc_node.h>

extern void netipc_thread_lock();
extern void netipc_thread_unlock();

/*
 * Definitions for norma special ports.
 */
unsigned long		lid_counter = 0;
extern unsigned long	node_incarnation;
extern ipc_port_t	host_special_port[MAX_SPECIAL_ID];

/*
 * Allocate a uid for a newly exported port.
 */
unsigned long
norma_new_uid_locked()
{
	if (lid_counter == 0) {
		/*
		 * XXX
		 * This is a hack to work the incarnation into the lid
		 * so that we don't recreate the same uids. I still need
		 * to figure out how to do norma_ipc_cleanup_incarnation
		 * for real.
		 */
		lid_counter = MAX_SPECIAL_ID +
		    (node_incarnation % 16) * ((IP_NORMA_MAX_LID + 1) / 16);
		printf1("*** first uid=%x\n",
			IP_NORMA_UID(node_self(), lid_counter));
	} else if (lid_counter == IP_NORMA_MAX_LID) {
		panic("norma_new_uid: ran out of local ids\n");
	}
	return IP_NORMA_UID(node_self(), lid_counter++);
}

norma_ipc_cleanup_incarnation(remote)
	unsigned long remote;
{
	/*
	 * XXX
	 * What exactly do we clean up???
	 */
}

#define	HASH_SIZE	16		/* must be power of two */
#define	HASH_MASK	(HASH_SIZE - 1)
#define	UID_HASH(uid)	((IP_NORMA_NODE(uid) + IP_NORMA_LID(uid)) & HASH_MASK)

ipc_port_t	norma_port_table[HASH_SIZE];

int c_norma_port_lookup = 0;
int c_norma_port_insert = 0;
int c_norma_port_remove = 0;

/*
 * We currently use a fairly conservative locking strategy, in which we
 * always grab the netipc_lock before inserts, removals, and lookups.
 * We could do better once we move port removal out of norma_ipc_receive_dest,
 * at which point only port lookups would be done at interrupt level.
 * At this point, we could use a simple lock for lookups and only
 * use netipc_lock (in combination with the simple lock) for inserts and
 * removals. Currently this doesn't buy you much because send-once rights
 * are continually being inserted and removed; if we added a bit of laziness
 * to send-once right handling, then presumably lookups would dominate
 * inserts and removals.
 */

boolean_t
norma_port_tabled(port)
	ipc_port_t port;
{
	return (port->ip_norma_next != port);
}

ipc_port_t
norma_port_lookup_locked(uid)
	register unsigned long uid;
{
	register ipc_port_t p;

	c_norma_port_lookup++;
	if (uid == 0) {
		return IP_NULL;
	}
	for (p = norma_port_table[UID_HASH(uid)]; p; p = p->ip_norma_next) {
		if (p->ip_norma_uid == uid) {
			return p;
		}
	}
	return IP_NULL;
}

void
norma_port_insert_locked(port)
	register ipc_port_t port;
{
	register ipc_port_t *bucket;
	register unsigned long uid;

	c_norma_port_insert++;
	if (norma_port_tabled(port)) {
		panic("norma_port_insert: tabled!\n");
		return;
	}
	uid = port->ip_norma_uid;
	assert(uid != 0);
	bucket = &norma_port_table[UID_HASH(uid)];
	port->ip_norma_next = *bucket;
	*bucket = port;
	ip_reference(port);
}

void
norma_port_remove_locked(port)
	register ipc_port_t port;
{
	register unsigned long uid;
	register ipc_port_t p, *pp;

	c_norma_port_remove++;
	if (! norma_port_tabled(port)) {
		/* XXX should be a panic */
		printf("norma_port_remote: not tabled!\n");
		return;
	}
	uid = port->ip_norma_uid;
	assert(uid != 0);

	/*
	 * This is kind of gross.
	 * We cannot remove a proxy that started out as a principal;
	 * it must stay around to do forwarding.
	 */
	if (port->ip_norma_is_proxy && IP_NORMA_NODE(uid) == node_self()) {
		printf1("norma_port_remove: not removing port %x\n", port);
		return;
	}

	/*
	 * We can go ahead now.
	 */
	pp = &norma_port_table[UID_HASH(uid)];
	for (p = *pp; p; pp = &p->ip_norma_next, p = *pp) {
		if (p == port) {
			*pp = port->ip_norma_next;
			port->ip_norma_next = port;
			printf1("norma_port_remove(0x%x:%x): refs %d\n",
				port, uid, port->ip_references - 1);
			ipc_port_release(port);
			return;
		}
	}
	panic("norma_port_remove(0x%x:%x): not found\n", port, uid);
}

ipc_port_t
norma_port_lookup(uid)
	register unsigned long uid;
{
	register ipc_port_t port;

	assert(! netipc_locked());
	netipc_thread_lock();
	port = norma_port_lookup_locked(uid);
	netipc_thread_unlock();
	return port;
}

void
norma_port_insert(port)
	register ipc_port_t port;
{
	assert(! netipc_locked());
	netipc_thread_lock();
	norma_port_insert_locked(port);
	netipc_thread_unlock();
}

void
norma_port_remove(port)
	register ipc_port_t port;
{
	assert(! netipc_locked());
	netipc_thread_lock();
	norma_port_remove_locked(port);
	netipc_thread_unlock();
}

void
norma_port_remove_try(port)
	register ipc_port_t port;
{
	if (port->ip_srights == 0 && port->ip_sorights == 0) {
		printf1("norma_port_remove_try: releasing 0x%x:%x\n",
			port, port->ip_norma_uid);
		assert(! netipc_locked());
		netipc_thread_lock();
		norma_port_remove_locked(port);
		netipc_thread_unlock();
	}
}

unsigned long
norma_new_uid()
{
	register unsigned long uid;

	assert(! netipc_locked());
	netipc_thread_lock();
	uid = norma_new_uid_locked();
	netipc_thread_unlock();
	return uid;
}

norma_port_table_statistics()
{
	int i, total, average, len, longest;
	int l1, l2, l3;
	ipc_port_t p;

	total = 0;
	for (i = 0; i < HASH_SIZE; i++) {
		for (p = norma_port_table[i]; p; p = p->ip_norma_next) {
			total++;
		}
	}
	average = total / HASH_SIZE;
	if (average == 0) {
		average = 1;
	}
	longest = l1 = l2 = l3 = 0;
	for (i = 0; i < HASH_SIZE; i++) {
		len = 0;
		for (p = norma_port_table[i]; p; p = p->ip_norma_next) {
			len++;
		}
		if (longest < len) {
			longest = len;
		}
		if (len > average) {
			l1++;
			if (len > 2 * average) {
				l2++;
				if (len > 3 * average) {
					l3++;
				}
			}
		}
	}
	kdbprintf("%d ports, %d buckets, average %d per bucket\n",
		  total, HASH_SIZE, total / HASH_SIZE);
	kdbprintf("%d chains longer than %d, %d > %d, %d > %d; longest = %d\n",
		  l1, average, l2, 2 * average, l3, 3 * average, longest);
}

norma_port_iterate(function, args)
	void (*function)();
	void *args;
{
	int i;
	ipc_port_t p;

	for (i = 0; i < HASH_SIZE; i++) {
		for (p = norma_port_table[i]; p; p = p->ip_norma_next) {
			(*function)(p, args);
		}
	}
	(*function)(IP_NULL, args);
}

#include <ddb/db_sym.h>

void
db_show_uid(port, args)
	ipc_port_t port;
	void *args;
{
	int *countp = (int *) args;

	if (port == IP_NULL) {
		printf("\n");
		return;
	}
	if (++*countp == 4) {
		kdbprintf("%x:%x\n", port, port->ip_norma_uid);
		*countp = 0;
	} else {
		kdbprintf("%x:%x ", port, port->ip_norma_uid);
	}
}

void
db_show_uid_verbose(port, args)
	ipc_port_t port;
	void *args;
{
	if (port == IP_NULL) {
		return;
	}
	kdbprintf("%x:%x %6d ",
		  port, port->ip_norma_uid, port->ip_norma_spare2);
	db_printsym(port->ip_norma_spare1, DB_STGY_PROC);
	kdbprintf("\n");
}

void
db_show_proxy(port, args)
	ipc_port_t port;
	void *args;
{
	if (port == IP_NULL || port->ip_norma_is_proxy) {
		db_show_uid(port, args);
	}
}

void
db_show_principal(port, args)
	ipc_port_t port;
	void *args;
{
	if (port == IP_NULL || ! port->ip_norma_is_proxy) {
		db_show_uid(port, args);
	}
}

db_show_all_uids()
{
	int count = 0;

	norma_port_table_statistics();
	norma_port_iterate(db_show_uid, (void *) &count);
}

db_show_all_uids_verbose()
{
	norma_port_table_statistics();
	norma_port_iterate(db_show_uid_verbose, (void *) 0);
}

db_show_all_proxies()
{
	int count = 0;

	norma_port_iterate(db_show_proxy, (void *) &count);
}

db_show_all_principals()
{
	int count = 0;

	norma_port_iterate(db_show_principal, (void *) &count);
}
