/*
	SEGA SC-3000 Emulator 'eSC-3000'

	Author : Takeda.Toshiya
	Date   : 2010.08.17-

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_SEL	0

class MEMORY : public DEVICE
{
private:
	// memory
	uint8_t cart[0x20000];
	uint8_t ipl[0x2000];	// sf7000
	uint8_t ram[0x10000];
	
	uint8_t wdmy[0x1000];
	uint8_t rdmy[0x1000];
	uint8_t* wbank[16];
	uint8_t* rbank[16];
	
	bool inserted;
	bool ram_selected;
	uint8_t bank[3];
	
	void update_bank();
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void open_cart(const _TCHAR* file_path);
	void close_cart();
	bool is_cart_inserted()
	{
		return inserted;
	}
};

#endif

