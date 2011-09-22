/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	dev_name.c,v $
 * Revision 2.8  93/05/17  15:01:03  rvb
 * 	We need a nomap() of type vm_offset_t
 * 
 * Revision 2.7  92/08/03  17:33:11  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.6  92/05/21  17:08:55  jfriedl
 * 	tried prototypes.
 * 	[92/05/20            jfriedl]
 * 
 * Revision 2.5  91/07/31  17:32:50  dbg
 * 	Fixed to not modify the string passed in.
 * 	[91/07/23            dbg]
 * 
 * Revision 2.4  91/05/14  15:41:13  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:08:34  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:27:34  mrt]
 * 
 * Revision 2.2  89/09/08  11:23:17  dbg
 * 	Moved device-name search routines here from dev_lookup.c
 * 	[89/08/01            dbg]
 * 
 */
/*
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date: 	8/89
 */

#include <device/device_types.h>
#include <device/dev_hdr.h>
#include <device/conf.h>



/*
 * Routines placed in empty entries in the device tables
 */
int nulldev()
{
	return (D_SUCCESS);
}

int nodev()
{
	return (D_INVALID_OPERATION);
}

vm_offset_t
nomap()
{
	return (D_INVALID_OPERATION);
}

/*
 * Name comparison routine.
 * Compares first 'len' characters of 'src'
 * with 'target', which is zero-terminated.
 * Returns TRUE if strings are equal:
 *   src and target are equal in first 'len' characters
 *   next character of target is 0 (end of string).
 */
boolean_t
name_equal(src, len, target)
	register char *	src;
	register int	len;
	register char *	target;
{
	while (--len >= 0)
	    if (*src++ != *target++)
		return FALSE;
	return *target == 0;
}

/*
 * device name lookup
 */
boolean_t dev_name_lookup(name, ops, unit)
	char *		name;
	dev_ops_t	*ops;	/* out */
	int		*unit;	/* out */
{
	/*
	 * Assume that block device names are of the form
	 *
	 * <device_name><unit_number><partition>
	 *
	 * where
	 * <device_name>	is the name in the device table
	 * <unit_number>	is an integer
	 * <partition>		is a letter in [a-h]
	 */

	register char *	cp = name;
	int		len;
	register int	j = 0;
	register int	c;
	dev_ops_t	dev;
	register boolean_t found;

	/*
	 * Find device type name (characters before digit)
	 */
	while ((c = *cp) != '\0' &&
		!(c >= '0' && c <= '9'))
	    cp++;

	len = cp - name;
	if (c != '\0') {
	    /*
	     * Find unit number
	     */
	    while ((c = *cp) != '\0' &&
		    c >= '0' && c <= '9') {
		j = j * 10 + (c - '0');
		cp++;
	    }
	}

	found = FALSE;
	dev_search(dev) {
	    if (name_equal(name, len, dev->d_name)) {
		found = TRUE;
		break;
	    }
	}
	if (!found) {
	    /* name not found - try indirection list */
	    register dev_indirect_t	di;

	    dev_indirect_search(di) {
		if (name_equal(name, len, di->d_name)) {
		    /*
		     * Return device and unit from indirect vector.
		     */
		    *ops = di->d_ops;
		    *unit = di->d_unit;
		    return (TRUE);
		}
	    }
	    /* Not found in either list. */
	    return (FALSE);
	}

	*ops = dev;
	*unit = j;

	/*
	 * Find sub-device number
	 */
	j = dev->d_subdev;
	if (j > 0) {
	    if (c >= 'a' && c < 'a' + j) {
		/*
		 * Minor number is <subdev_count>*unit + letter.
		 */
		*unit = *unit * j + (c - 'a');
	    }
	    else {
		/*
		 * Assume unit A.
		 */
		*unit = *unit * j;
	    }
	}
	return (TRUE);
}

/*
 * Change an entry in the indirection list.
 */
void
dev_set_indirection(name, ops, unit)
	char		*name;
	dev_ops_t	ops;
	int		unit;
{
	register dev_indirect_t di;

	dev_indirect_search(di) {
	    if (!strcmp(di->d_name, name)) {
		di->d_ops = ops;
		di->d_unit = unit;
		break;
	    }
	}
}

boolean_t dev_change_indirect(iname, dname, unit)
char *iname,*dname;
int unit;
{
    struct dev_ops *dp;
    struct dev_indirect *di;
    int found = FALSE;

    dev_search(dp) {
	if (!strcmp(dp->d_name,dname)) {
	    found = TRUE;
	    break;
	}
    }
    if (!found) return FALSE;
    dev_indirect_search(di) {
	if (!strcmp(di->d_name,iname)) {
	    di->d_ops = dp;
	    di->d_unit = unit;
	    return TRUE;
	}
    }
    return FALSE;
}
