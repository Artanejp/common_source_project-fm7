// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    vtlb.h

    Generic virtual TLB implementation.

***************************************************************************/

#pragma once

#ifndef __LIB_I386_VTLB_H__
#define __LIB_I386_VTLB_H__

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define VTLB_FLAGS_MASK             0xff

#define VTLB_READ_ALLOWED           0x01        /* (1 << TRANSLATE_READ) */
#define VTLB_WRITE_ALLOWED          0x02        /* (1 << TRANSLATE_WRITE) */
#define VTLB_FETCH_ALLOWED          0x04        /* (1 << TRANSLATE_FETCH) */
#define VTLB_FLAG_VALID             0x08
#define VTLB_USER_READ_ALLOWED      0x10        /* (1 << TRANSLATE_READ_USER) */
#define VTLB_USER_WRITE_ALLOWED     0x20        /* (1 << TRANSLATE_WRITE_USER) */
#define VTLB_USER_FETCH_ALLOWED     0x40        /* (1 << TRANSLATE_FETCH_USER) */
#define VTLB_FLAG_FIXED             0x80



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* represents an entry in the VTLB */
typedef UINT32 vtlb_entry;

/* opaque structure describing VTLB state */
struct vtlb_state;

#endif /* __VTLB_H__ */
