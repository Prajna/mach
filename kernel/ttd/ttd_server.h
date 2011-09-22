/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
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
 * Prototypes for ttd_server.c
 *
 * HISTORY:
 * $Log:	ttd_server.h,v $
 * Revision 2.2  93/05/10  23:25:01  rvb
 * 	Checkin for MK80 branch.
 * 	[93/05/10  15:09:23  grm]
 * 
 * Revision 2.1.2.2  93/04/20  11:07:57  grm
 * 	Added extern for break_set.
 * 	[93/04/20            grm]
 * 
 * Revision 2.1.2.1  93/03/03  14:41:33  grm
 * 	Interface changed.  Version that works.
 * 	[93/03/03            grm]
 * 
 * Revision 2.1.1.1  93/01/28  15:18:08  grm
 * 	Initial Version.
 * 
 *
 */

#ifndef	_TTD_SERVER_H_
#define	_TTD_SERVER_H_

#include <mach/boolean.h>
#include <mach/mach_types.h>

extern void kttd_stop_target(void);

extern boolean_t break_set(ttd_address addr,
			   ttd_saved_inst *inst);

extern boolean_t kttd_single_step(void);
extern boolean_t kttd_clear_single_step(void);
extern boolean_t kttd_in_single_step(void);

extern void ttd_service_request(void);

extern void ttd_server_initialize(void);

#endif	/* _TTD_SERVER_H_ */
