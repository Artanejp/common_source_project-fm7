/*
	BANDAI RX-78 Emulator 'eRX-78'

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
	// memory
	uint8 ipl[0x2000];
	uint8 cart[0x4000];
	uint8 ram[0x4000];
	uint8 ext[0x5000];
	uint8 vram[0x2000 * 6];
	
	uint8 wdmy[0x1000];
	uint8 rdmy[0x1000];
	uint8* wbank[16];
	uint8* rbank[16];
	uint8* vbank[6];
	
	int rpage, wpage;
	bool inserted;
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	
	// unique functions
	void open_cart(_TCHAR* file_path);
	void close_cart();
	bool cart_inserted()
	{
		return inserted;
	}
	uint8* get_vram() { return vram; }
};

#endif

