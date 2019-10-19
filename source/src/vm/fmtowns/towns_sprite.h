
#pragma once

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_TOWNS_SPRITE_HOOK_VLINE 256
#define SIG_TOWNS_SPRITE_SET_LINES  257

namespace FMTOWNS {
	class TOWNS_VRAM;
}

namespace FMTOWNS {
class TOWNS_SPRITE : public DEVICE
{

protected:
	TOWNS_VRAM *d_vram;
	// REGISTERS
	uint8_t reg_addr;
	uint8_t reg_data[8];
	// #0, #1
	bool reg_spen;
	uint16_t reg_index;
	uint8_t pattern_ram[0x20000];

	uint16_t reg_voffset;
	uint16_t reg_hoffset;
	bool disp_page0;
	bool disp_page1;
	
	bool now_transferring;
	
	int render_num;
	int render_mod;
	int render_lines;
	bool sprite_enabled;
	
	bool split_rendering;
	int max_sprite_per_frame;

	void __FASTCALL render_sprite(int num,  int x, int y, uint16_t attr, uint16_t color);
	void render_full();
	void render_part(int start, int end);
	void do_vline_hook(int line);

public:
	TOWNS_SPRITE(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_vram = NULL;
		set_device_name(_T("SPRITE"));
	}
	~TOWNS_SPRITE() {}

	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);

	void write_data8(uint32_t addr, uint32_t data);
	void write_data16(uint32_t addr, uint32_t data);
	void write_data32(uint32_t addr, uint32_t data);
	
	uint32_t read_data8(uint32_t addr);
	uint32_t read_data16(uint32_t addr);
	uint32_t read_data32(uint32_t addr);

	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void initialize();
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
	void set_context_vram(TOWNS_VRAM *p)
	{
		d_vram = p;
	}
};
}
