/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * Copyright (c) 1991 IBM Corporation 
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation,
 * and that the name IBM not be used in advertising or publicity 
 * pertaining to distribution of the software without specific, written
 * prior permission.
 * 
 * CARNEGIE MELLON AND IBM ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON AND IBM DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	kd_mouse_abios.h,v $
 * Revision 2.3  93/03/11  14:09:29  danner
 * 	u_long -> u_int
 * 	[93/03/09            danner]
 * 
 * Revision 2.2  93/02/04  08:01:08  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */

/* @(#)kd_mouse_abios.h       1.9  @(#)kd_mouse_abios.h	1.9 3/8/90 19:40:12 */
/*
 * COMPONENT_NAME: SYSXMOUSE mouse driver
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 */                                                                   

#ifndef _H_MOUSEABIOS
#define _H_MOUSEABIOS

/* ABIOS mouse specific function codes.
** 	- see <sys/abios.h> for generic ABIOS request defines
*/
#define ABIOS_MOUSE_RESET_POINTING_DEVICE		0x05
#define ABIOS_MOUSE_ENABLE_POINTING_DEVICE		0x06
#define ABIOS_MOUSE_DISABLE_POINTING_DEVICE		0x07
#define ABIOS_MOUSE_POINTING_DEVICE_CONTINUOUS_READ	0x08
#define ABIOS_MOUSE_SET_SAMPLE_RATE			0x0B
#define ABIOS_MOUSE_SET_RESOLUTION			0x0C
#define ABIOS_MOUSE_SET_SCALING_FACTOR			0x0D
#define ABIOS_MOUSE_READ_POINTING_DEVICE_IDENTIFICATION_CODE	0x0E

/* ABIOS mouse specific return codes.
*/
#define ABIOS_MOUSE_RC_DONE			0x0000
#define ABIOS_MOUSE_RC_STAGE_ON_INT		0x0001
#define ABIOS_MOUSE_RC_STAGE_ON_TIME		0x0002
#define ABIOS_MOUSE_RC_NOT_MY_INT		0x0005
#define ABIOS_MOUSE_RC_ATTENTION		0x0009
#define ABIOS_MOUSE_RC_DEVICE_IN_USE		0x8000
#define ABIOS_MOUSE_RC_RESEND			0x8001
#define ABIOS_MOUSE_RC_TWO_CONSECUTIVE_RESENDS_FOUND	0x8002
#define ABIOS_MOUSE_RC_SYSTEM_LOCK		0x8003
#define ABIOS_MOUSE_RC_INVALID_LOGICAL_ID	0xC000
#define ABIOS_MOUSE_RC_INVALID_FUNCTION		0xC001
#define ABIOS_MOUSE_RC_INVALID_UNIT_NUMBER	0xC003
#define ABIOS_MOUSE_RC_INVALID_REQUEST_BLOCK_LENGTH	0xC004
#define ABIOS_MOUSE_RC_INVALID_MOUSE_CONTROLLER_PARAMETER	0xC005

/*  Set up mouse ABIOS request block, assuming request block length
** of MOUSE_REQUEST_BLOCK_LEN+0x10 bytes.  This should be sufficient.
** The real value can be found by issuing ABIOS_LOGICAL_PARAMETER
** function.
*/
struct Device_params
{
	u_char	interface_status;
	u_char	data_package_size;
	u_short	flag_word;
	u_short	current_resolution;
	u_short	current_sample_rate;
	u_int	time_to_wait;
};

struct Pointing_device_data
{
	u_char	reserved[0x0c];
	u_char	status;
	u_char	deltax;
	u_char 	deltay;
	u_char	reserved2[0x09];
};

struct Set_scaling_factor
{
        u_char  scaling_factor;
	u_char	reserved[7];
	u_int	time_to_wait;
	u_char	reserved2[12];
	u_char	retry_mode_enable;
	u_char	mouse_data_buffer_index;
	u_short	retry_parameters;
	u_short	reserved3;
	u_char	count_of_status_bytes_in_RB;
	u_char	current_interrupt_level;
	u_char	arbitration_level;
	u_short	device_id;
	u_short	keyboard_data_register;
	u_short	the_8042_status_register;
	u_char	reserved4;
	u_char	pointer_to_beginning_of_request_block_status_area;
};

struct Pointing_device_id_code
{
	u_char	auxiliary_device_identification_code;
        u_char  reserved[7];
	u_int	time_to_wait;
        u_char  reserved2[12];
	u_char	retry_mode_enable;
	u_char	mouse_data_buffer_index;
	u_short	retry_parameters;
	u_short	reserved3;
	u_char	count_of_status_bytes_in_RB;
	u_char	current_interrupt_level;
	u_char	arbitration_level;
	u_short	device_id;
	u_short	keyboard_status_register;
	u_short	the_8042_status_register;
	u_char	reserved4;
	u_char	pointer_to_beginning_of_request_block_status_area;
};

struct Set_sample_rate
{
     u_char   reserved[2];
     u_short  sample_rate;
     u_char   reserved2[4];
     u_int   time_to_wait;
   };

struct Set_resolution
{
     u_char   reserved[2];
     u_short  resolution;
     u_char   reserved2[4];
     u_int   time_to_wait;
     u_char   reserved3[4];
   };

#define MOUSE_REQUEST_BLOCK_LEN		64
struct	Mouse_request {
	struct Request_header request_header;	/* 0x00-0x0f abios.h */
	union {
		struct Logical_id_params	logical_id_params;
		struct Device_params		device_params;
		struct Pointing_device_data	pointing_device_data;
		struct Set_scaling_factor	set_scaling_factor;
		struct Pointing_device_id_code	pointing_device_id_code;
                struct Set_sample_rate          set_sample_rate;
                struct Set_resolution           set_resolution;
		u_char				uc[MOUSE_REQUEST_BLOCK_LEN];
	} un;
	short	r_flags;
};

/* Mouse_request.r_flags */
#define MS_NOERR	0x0001
#define MS_NOUNIT	0x0002
#define MS_DEFLT	(0x0000)

/* ABIOS_LOGICAL_PARAMETER (0x01)
**	- Always returns 0.
**	- Does not interrupt.
** INPUT: see mouse_cmd.c
** OUTPUT: see <sys/abios.h>
*/
#define MOUSE_SET_RESERVED_ABIOS_LOGICAL_PARAMETER(rb) \
	*( (u_short *) &(rb).un.uc[0x0a])  = 0; \
	*( (u_short *) &(rb).un.uc[0x0c])  = 0; \
	*( (u_short *) &(rb).un.uc[0x0e])  = 0;


/* ABIOS_READ_PARAMETER (0x03)
**	- function which returns info specific to the selected drive.
**	- only returns 0
**	- does not interrupt
** INPUT: drive number
**
*/
#define MOUSE_SET_RESERVED_ABIOS_READ_PARAMETER(rb) \
	*( (u_short *) &(rb).un.uc[0x0c])  = 0;

/* OUTPUT */
#define MOUSE_TIME_TO_WAIT(rb) *( (u_int *) &(rb).un.uc[0x08])
#define r_interface_status	un.device_params.interface_status
#define r_3_data_package_size	un.device_params.data_package_size
#define r_flag_word		un.device_params.flag_word
#define r_current_resolution	un.device_params.current_resolution
#define r_current_sample_rate	un.device_params.current_sample_rate

/* For ABIOS_MOUSE_RESET_POINTING_DEVICE (0x05)
**	- resets the mouse to an initial state
*/
#define MOUSE_SET_RESERVED_ABIOS_RESET_POINTING_DEVICE(rb) \
	*( (u_short *) &(rb).un.uc[0x0c]) = 0;
/* OUTPUT */
#define r_pointing_device_completion_code	un.uc[0x00]
#define r_pointing_device_identification_code	un.uc[0x01]


/* For MOUSE_ABIOS_ENABLE_POINTING_DEVICE (0x06) */
#define MOUSE_SET_RESERVED_ABIOS_ENABLE_POINTING_DEVICE(rb) \
	*((u_short *) &(rb).un.uc[0x0c]) = 0;

/* For MOUSE_ABIOS_DISABLE_POINTING_DEVICE (0x07) */
#define MOUSE_SET_RESERVED_ABIOS_DISABLE_POINTING_DEVICE(rb) \
	*((u_short *) &(rb).un.uc[0x0c]) = 0;

/* ABIOS_MOUSE_POINTING_DEVICE_CONTINUOUS_READ (0x08) */
#define MOUSE_SET_RESERVED_ABIOS_POINTING_DEVICE_CONTINUOUS_READ(rb) \
	*( (u_int *) &(rb).un.uc[0x02]) = 0;
/* INPUT */
#define r_8_data_package_size	un.uc[0x00]
/* OUTPUT */
#define r_pointing_device_data_status	un.pointing_device_data.status
#define r_pointing_device_data_deltax	un.pointing_device_data.deltax
#define r_pointing_device_data_deltay	un.pointing_device_data.deltay

/* ABIOS_MOUSE_SET_SAMPLE_RATE (0x0b) */
#define MOUSE_SET_RESERVED_ABIOS_MOUSE_SET_SAMPLE_RATE(rb)
/* INPUT: */
#define r_sample_rate  un.set_sample_rate.sample_rate

/* ABIOS_MOUSE_SET_RESOLUTION (0x0c) */
#define MOUSE_SET_RESERVED_ABIOS_SET_RESOLUTION(rb) \
	*( (u_short *) &(rb).un.uc[0x0c]) = 0;
/* INPUT: */
#define r_resolution  un.set_resolution.resolution

/* ABIOS_MOUSE_SET_SCALING_FACTOR (0x0d) */
#define MOUSE_SET_RESERVED_ABIOS_SET_SCALING_FACTOR(rb) \
	*((u_short *) &(rb).un.uc[0x0c]) = 0;
/* INPUT: */
#define r_scaling_factor  un.set_scaling_factor.scaling_factor
/* OUTPUT */
#define	r_D_retry_mode_enable	un.set_scaling_factor.retry_mode_enable
#define	r_D_mouse_data_buffer_index \
	un.set_scaling_factor.mouse_data_buffer_index
#define r_D_retry_parameters	un.set_scaling_factor.retry_parameters
#define r_D_count_of_status_bytes_in_RB \
	un.set_scaling_factor.count_of_status_bytes_in_RB
#define r_D_current_interrupt_level \
	un.set_scaling_factor.current_interrupt_level
#define r_D_arbitration_level	un.set_scaling_factor.arbitration_level
#define r_D_device_id	un.set_scaling_factor.device_id
#define r_D_keyboard_status_register \
	un.set_scaling_factor.keyboard_status_register
#define r_D_8042_status_register \
	un.set_scaling_factor.the_8042_status_register
#define r_D_pointer_to_beginning_of_request_block_status_area \
	un.set_scaling_factor.pointer_to_beginning_of_request_block_status_area

/* ABIOS_MOUSE_READ_POINTING_DEVICE_IDENTIFICATION_CODE (0x0e) */
#define MOUSE_SET_RESERVED_ABIOS_READ_POINTING_DEVICE_IDENTIFICATION_CODE(rb) \
	*((u_short *) &(rb).un.uc[0x0c]) = 0;
/* OUTPUT */
#define r_auxiliary_device_identification_code \
        un.pointing_device_id_code.auxiliary_device_identification_code
#define	r_E_retry_mode_enable	un.pointing_device_id_code.retry_mode_enable
#define	r_E_mouse_data_buffer_index \
	un.pointing_device_id_code.mouse_data_buffer_index
#define r_E_retry_parameters	un.pointing_device_id_code.retry_parameters
#define r_E_count_of_status_bytes_in_RB \
	un.pointing_device_id_code.count_of_status_bytes_in_RB
#define r_E_current_interrupt_level \
	un.pointing_device_id_code.current_interrupt_level
#define r_E_arbitration_level	un.pointing_device_id_code.arbitration_level
#define r_E_device_id	un.pointing_device_id_code.device_id
#define r_E_keyboard_status_register \
	un.pointing_device_id_code.keyboard_status_register
#define r_E_8042_status_register \
	un.pointing_device_id_code.the_8042_status_register
#define r_E_pointer_to_beginning_of_request_block_status_area \
un.pointing_device_id_code.pointer_to_beginning_of_request_block_status_area
	
#endif /* _H_MOUSEABIOS */
