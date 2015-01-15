/*
	EPOCH Super Cassette Vision Emulator 'eSCV'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MEMORY : public DEVICE
{
private:
	DEVICE* d_sound;
	
	// memory
	_TCHAR save_path[_MAX_PATH];
	
	struct {
		char id[4];	// SCV^Z
		uint8 ctype;	// 0=16KB,32KB,32K+8KB ROM, bankswitched by PC5
				// 1=32KB ROM+8KB SRAM, bank switched by PC5
				// 2=32KB+32KB,32KB+32KB+32KB+32KB ROM, bank switched by PC5,PC6
				// 3=32KB+32KB ROM, bank switched by PC6
		uint8 dummy[11];
	} header;
	bool inserted;
	uint32 sram_crc32;
	
	uint8* wbank[0x200];
	uint8* rbank[0x200];
	uint8 bios[0x1000];
	uint8 vram[0x2000];
	uint8 wreg[0x80];
	uint8 cart[0x8000*4];
	uint8 sram[0x2000];
	uint8 wdmy[0x80];
	uint8 rdmy[0x80];
	
	uint8 cur_bank;
	void set_bank(uint8 bank);
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_data8w(uint32 addr, uint32 data, int* wait);
	uint32 read_data8w(uint32 addr, int* wait);
	void write_io8(uint32 addr, uint32 data);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void open_cart(_TCHAR* file_path);
	void close_cart();
	bool cart_inserted()
	{
		return inserted;
	}
	void set_context_sound(DEVICE* device)
	{
		d_sound = device;
	}
	uint8* get_font()
	{
		return bios + 0x200;
	}
	uint8* get_vram()
	{
		return vram;
	}
};

#endif
