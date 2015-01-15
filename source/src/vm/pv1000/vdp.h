/*
	CASIO PV-1000 Emulator 'ePV-1000'

	Author : Takeda.Toshiya
	Date   : 2006.11.16 -

	[ video processor ]
*/

#ifndef _VDP_H_
#define _VDP_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class VDP : public DEVICE
{
private:
	DEVICE* d_cpu;
	
	uint8 bg[192][256];
	uint8* vram;
	uint8* pcg;
	uint8* pattern;
	uint8* base;
	bool force_pattern;
	
	void draw_pattern(int x8, int y8, uint16 top);
	void draw_pcg(int x8, int y8, uint16 top);
	
public:
	VDP(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~VDP() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	void event_callback(int event_id, int err);
	void event_vline(int v, int clock);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_memory_ptr(uint8* ptr)
	{
		base = ptr;
		vram = ptr + 0xb800;
		pcg = ptr + 0xbc00;
		pattern = ptr;
	}
	void draw_screen();
};

#endif
