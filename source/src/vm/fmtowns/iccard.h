/*
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2020.08.10-

	[FM-Towns IC CARD]
*/

#pragma once

#include "device.h"

class FILEIO;

namespace FMTOWNS {
	
class TOWNS_ICCARD: public DEVICE {
protected:
	uint32_t limit_size;
	bool is_rom; // 
	bool is_dirty;

	// 048Ah
	bool card_changed;
	uint8_t batt_level;
	bool eeprom_ready;
	uint8_t card_state;
	bool write_protect;

	_TCHAR filename[_MAX_PATH];

	uint8_t *mem;
	uint32_t mem_size;
	bool new_alloc(uint32_t new_size);

public:
	TOWNS_ICCARD(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(filename, 0x00, sizeof(filename));
		limit_size = 16 * 0x100000; // 16MB
		mem_size = 0;
		mem = NULL;
		set_device_name(_T("TOWNS IC CARD"));
	}
	~TOWNS_ICCARD() {}
	
	virtual void initialize();
	virtual void release();

	virtual uint32_t __FASTCALL read_memory_mapped_io8(uint32_t addr);
	virtual void __FASTCALL write_memory_mapped_io8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_io8(uint32_t addr);
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data);

	virtual bool open_cart(const _TCHAR *file_path);
	virtual bool close_cart();

	virtual bool process_state(FILEIO* state_fio, bool loading);

	bool is_cart_inserted()
	{
		return (card_state == 0x00) ? true : false;
	}
};

}

