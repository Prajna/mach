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
 * $Log:	xmm_invoke.c,v $
 * Revision 2.2  92/03/10  16:29:22  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:51:30  jsb]
 * 
 * Revision 2.1.2.1  92/02/21  11:27:27  jsb
 * 	Removed reference counting from {M,K}_{REFERENCE,RELEASE} macros.
 * 	Added check for null obj in {M,K}_CALL macros.
 * 	Added call to xmm_obj_unlink to M_TERMINATE.
 * 	[92/02/16  14:17:12  jsb]
 * 
 * 	First checkin.
 * 	[92/02/09  14:17:05  jsb]
 * 
 */
/*
 *	File:	norma/xmm_invoke.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1992
 *
 *	Xmm invocation routines.
 */

#include <norma/xmm_obj.h>

#if	!XMM_USE_MACROS

/*
 * I've considered automatically (i.e., in these macros) grabbing and
 * release obj or class locks, but I don't see a good protocol for
 * doing so.
 */

#define	M_REFERENCE\
	kern_return_t kr;\
	register xmm_obj_t m_mobj;\
\
	m_mobj = mobj->m_mobj;\
	if (m_mobj == XMM_OBJ_NULL) {\
		panic("m_invoke: null obj");\
		return KERN_FAILURE;\
	}\
    	kr = /* function call that follows */

#define	M_RELEASE\
	return kr;

#define	K_REFERENCE\
	kern_return_t kr;\
	register xmm_obj_t k_kobj;\
\
	k_kobj = kobj->k_kobj;\
	if (k_kobj == XMM_OBJ_NULL) {\
		panic("k_invoke: null obj");\
		return KERN_FAILURE;\
	}\
    	kr = /* function call that follows */

#define	K_RELEASE\
	return kr;

#define	R_REFERENCE\
	kern_return_t kr;\
\
    	kr = /* function call that follows */

#define	R_RELEASE\
	return kr;

#if	__STDC__
#define	M_CALL(method)	(*m_mobj->class->m_ ## method)
#define	K_CALL(method)	(*k_kobj->class->k_ ## method)
#define	R_CALL(method)	(*xmm_m_reply(reply)->m_ ## method)
#else	__STDC__
#define	M_CALL(method)	(*m_mobj->class->m_/**/method)
#define	K_CALL(method)	(*k_kobj->class->k_/**/method)
#define	R_CALL(method)	(*xmm_m_reply(reply)->m_/**/method)
#endif	__STDC__

kern_return_t
M_INIT(mobj, k_kobj, pagesize, internal, size)
	xmm_obj_t mobj;
	xmm_obj_t k_kobj;
	vm_size_t pagesize;
	boolean_t internal;
	vm_size_t size;
{
	M_REFERENCE
	M_CALL(init)(m_mobj, k_kobj, pagesize, internal, size);
	M_RELEASE
}

kern_return_t
M_TERMINATE(mobj, kobj)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
{
	M_REFERENCE
	M_CALL(terminate)(m_mobj, kobj->m_kobj);
	xmm_obj_unlink(mobj, kobj);
	M_RELEASE
}

kern_return_t
M_COPY(mobj, kobj, offset, length, new_mobj)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	xmm_obj_t new_mobj;
{
	M_REFERENCE
	M_CALL(copy)(m_mobj, kobj->m_kobj, offset, length, new_mobj);
	M_RELEASE
}

kern_return_t
M_DATA_REQUEST(mobj, kobj, offset, length, desired_access)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
	M_REFERENCE
	M_CALL(data_request)(m_mobj, kobj->m_kobj, offset, length,
			     desired_access);
	M_RELEASE
}

kern_return_t
M_DATA_UNLOCK(mobj, kobj, offset, length, desired_access)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	vm_prot_t desired_access;
{
	M_REFERENCE
	M_CALL(data_unlock)(m_mobj, kobj->m_kobj, offset, length,
			    desired_access);
	M_RELEASE
}

kern_return_t
M_DATA_WRITE(mobj, kobj, offset, data, length)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_offset_t data;
	vm_size_t length;
{
	M_REFERENCE
	M_CALL(data_write)(m_mobj, kobj->m_kobj, offset, data, length);
	M_RELEASE
}

kern_return_t
M_LOCK_COMPLETED(reply, offset, length)
	xmm_reply_t reply;
	vm_offset_t offset;
	vm_size_t length;
{
	R_REFERENCE
	R_CALL(lock_completed)(reply, offset, length);
	R_RELEASE
}

kern_return_t
M_SUPPLY_COMPLETED(reply, offset, length, result, error_offset)
	xmm_reply_t reply;
	vm_offset_t offset;
	vm_size_t length;
	kern_return_t result;
	vm_offset_t error_offset;
{
	R_REFERENCE
	R_CALL(supply_completed)(reply, offset, length, result, error_offset);
	R_RELEASE
}

kern_return_t
M_DATA_RETURN(mobj, kobj, offset, data, length)
	xmm_obj_t mobj;
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_offset_t data;
	vm_size_t length;
{
	M_REFERENCE
	M_CALL(data_return)(m_mobj, kobj->m_kobj, offset, data, length);
	M_RELEASE
}

kern_return_t
M_CHANGE_COMPLETED(reply, may_cache, copy_strategy)
	xmm_reply_t reply;
	boolean_t may_cache;
	memory_object_copy_strategy_t copy_strategy;
{
	R_REFERENCE
	R_CALL(change_completed)(reply, may_cache, copy_strategy);
	R_RELEASE
}

kern_return_t
K_DATA_UNAVAILABLE(kobj, offset, length)
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
{
	K_REFERENCE
	K_CALL(data_unavailable)(k_kobj, offset, length);
	K_RELEASE
}

kern_return_t
K_GET_ATTRIBUTES(kobj, object_ready, may_cache, copy_strategy)
	xmm_obj_t kobj;
	boolean_t *object_ready;
	boolean_t *may_cache;
	memory_object_copy_strategy_t *copy_strategy;
{
	K_REFERENCE
	K_CALL(get_attributes)(k_kobj, object_ready, may_cache, copy_strategy);
	K_RELEASE
}

kern_return_t
K_LOCK_REQUEST(kobj, offset, length, should_clean, should_flush, lock_value,
	       reply)
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	boolean_t should_clean;
	boolean_t should_flush;
	vm_prot_t lock_value;
	xmm_reply_t reply;
{
	K_REFERENCE
	K_CALL(lock_request)(k_kobj, offset, length, should_clean,
			     should_flush, lock_value, xmm_k_reply(reply));
	K_RELEASE
}

kern_return_t
K_DATA_ERROR(kobj, offset, length, error_value)
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_size_t length;
	kern_return_t error_value;
{
	K_REFERENCE
	K_CALL(data_error)(k_kobj, offset, length, error_value);
	K_RELEASE
}

kern_return_t
K_SET_READY(kobj, object_ready, may_cache, copy_strategy, use_old_pageout,
	    memory_object_name, reply)
	xmm_obj_t kobj;
	boolean_t object_ready;
	boolean_t may_cache;
	memory_object_copy_strategy_t copy_strategy;
	boolean_t use_old_pageout;
	ipc_port_t memory_object_name;
	xmm_reply_t reply;
{
	K_REFERENCE
	K_CALL(set_ready)(k_kobj, object_ready, may_cache, copy_strategy,
			  use_old_pageout, memory_object_name,
			  xmm_k_reply(reply));
	K_RELEASE
}

kern_return_t
K_DESTROY(kobj, reason)
	xmm_obj_t kobj;
	kern_return_t reason;
{
	K_REFERENCE
	K_CALL(destroy)(k_kobj, reason);
	K_RELEASE
}

kern_return_t
K_DATA_SUPPLY(kobj, offset, data, length, lock_value, precious, reply)
	xmm_obj_t kobj;
	vm_offset_t offset;
	vm_offset_t data;
	vm_size_t length;
	vm_prot_t lock_value;
	boolean_t precious;
	xmm_reply_t reply;
{
	K_REFERENCE
	K_CALL(data_supply)(k_kobj, offset, data, length, lock_value, precious,
			    xmm_k_reply(reply));
	K_RELEASE
}

kern_return_t
_K_DESTROY(kobj, reason)
	xmm_obj_t kobj;
	kern_return_t reason;
{
	return (*kobj->class->k_destroy)(kobj, reason);
}

#else	!XMM_USE_MACROS
#endif	!XMM_USE_MACROS
