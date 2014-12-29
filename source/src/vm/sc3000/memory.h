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
	uint8 cart[0x8000];
	uint8 ipl[0x2000];	// sf7000
	uint8 ram[0x10000];
	
	uint8 wdmy[0x1000];
	uint8 rdmy[0x1000];
	uint8* wbank[16];
	uint8* rbank[16];
	
	bool inserted;
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void open_cart(_TCHAR* file_path);
	void close_cart();
	bool cart_inserted()
	{
		return inserted;
	}
};

#endif

