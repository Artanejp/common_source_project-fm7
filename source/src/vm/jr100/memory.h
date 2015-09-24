/*
	National JR-100 Emulator 'eJR-100'

	Author : Takeda.Toshiya
	Date   : 2015.08.27-

	[ memory bus ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_VIA_PORT_A	0
#define SIG_MEMORY_VIA_PORT_B	1

class MEMORY : public DEVICE
{
private:
	// contexts
	DEVICE *d_via;
	uint8* key_stat;
	uint32* joy_stat;
	
	// memory
	uint8 ram[0x8000];
	uint8 vram[0x400];
	uint8 rom[0x2000];
	
	uint8 wdmy[0x2000];
	uint8 rdmy[0x2000];
	uint8* wbank[8];
	uint8* rbank[8];
	
	int key_column;
	bool cmode;
	scrntype palette_pc[2];
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_frame();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_via(DEVICE* device)
	{
		d_via = device;
	}
	void draw_screen();
};

#endif

