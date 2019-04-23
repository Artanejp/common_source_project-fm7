// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    divtlb.h

    Generic virtual TLB implementation.

***************************************************************************/

#ifndef MAME_EMU_DIVTLB_H
#define MAME_EMU_DIVTLB_H

#pragma once

#include "device.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

constexpr uint32_t VTLB_FLAGS_MASK           = 0xff;

constexpr uint32_t VTLB_READ_ALLOWED         = 0x01;     /* (1 << TRANSLATE_READ) */
constexpr uint32_t VTLB_WRITE_ALLOWED        = 0x02;     /* (1 << TRANSLATE_WRITE) */
constexpr uint32_t VTLB_FETCH_ALLOWED        = 0x04;     /* (1 << TRANSLATE_FETCH) */
constexpr uint32_t VTLB_FLAG_VALID           = 0x08;
constexpr uint32_t VTLB_USER_READ_ALLOWED    = 0x10;     /* (1 << TRANSLATE_READ_USER) */
constexpr uint32_t VTLB_USER_WRITE_ALLOWED   = 0x20;     /* (1 << TRANSLATE_WRITE_USER) */
constexpr uint32_t VTLB_USER_FETCH_ALLOWED   = 0x40;     /* (1 << TRANSLATE_FETCH_USER) */
constexpr uint32_t VTLB_FLAG_FIXED           = 0x80;



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* represents an entry in the VTLB */
typedef uint32_t vtlb_entry;
typedef uint32_t offs_t;

// ======================> device_vtlb_interface

class VM_TEMPLATE;
class EMU;
class device_vtlb_interface : public DEVICE
{
protected:
	DEVICE*				m_device;
	// private state
	int				    m_space;            // address space
	int                 m_dynamic;          // number of dynamic entries
	int                 m_fixed;            // number of fixed entries
	int                 m_dynindex;         // index of next dynamic entry
	int                 m_pageshift;        // bits to shift to get page index
	int                 m_addrwidth;        // logical address bus width
	std::vector<offs_t> m_live;             // array of live entries by table index
	std::vector<int>    m_fixedpages;       // number of pages each fixed entry covers
	std::vector<vtlb_entry> m_table;        // table of entries by address
	std::vector<offs_t> m_refcnt;           // table of entry reference counts by address
	vtlb_entry          *m_table_base;      // pointer to m_table[0]
public:
	// construction/destruction
	device_vtlb_interface(VM_TEMPLATE* parent_vm, EMU* parent_emu, DEVICE* parent_device, int space, int fixed_entries = 0, int dynamic_entries = 0);
	virtual ~device_vtlb_interface();

	void initialize();
	void release();
	void reset();
	bool process_state(FILEIO* fio, bool loading);

	// configuration helpers
	void set_vtlb_dynamic_entries(int entries) { m_dynamic = entries; }
	void set_vtlb_fixed_entries(int entries) { m_fixed = entries; }

	// filling
	bool vtlb_fill(offs_t address, int intention);
	void vtlb_load(int entrynum, int numpages, offs_t address, vtlb_entry value);
	void vtlb_dynload(uint32_t index, offs_t address, vtlb_entry value);

	// flushing
	void vtlb_flush_dynamic();
	void vtlb_flush_address(offs_t address);

	// accessors
	const vtlb_entry *vtlb_table() const;
	// Original APIs, must call before initialize().
	void set_vtlb_page_shift(int m) { m_pageshift = m; }
	void set_vtlb_addr_width(int m) { m_addrwidth = m; }
	void set_parent_device(DEVICE* dev) { m_device = dev; }
};


#endif /* MAME_EMU_DIVTLB_H */
