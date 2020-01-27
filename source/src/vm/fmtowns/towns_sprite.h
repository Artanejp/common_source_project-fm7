
#pragma once

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_TOWNS_SPRITE_HOOK_VLINE  256
#define SIG_TOWNS_SPRITE_SET_LINES   257
#define SIG_TOWNS_SPRITE_SHADOW_RAM  258
#define SIG_TOWNS_SPRITE_BUSY        259
#define SIG_TOWNS_SPRITE_CALL_HSYNC  260
#define SIG_TOWNS_SPRITE_CALL_VSTART 261
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
	uint16_t reg_ctrl;
   
	bool reg_spen;
	uint16_t reg_index;
	uint8_t pattern_ram[0x20000];
	uint8_t ram[0x3000];

	uint16_t reg_voffset;
	uint16_t reg_hoffset;
	bool disp_page0;
	bool disp_page1;
	int frame_sprite_count;
	bool sprite_enabled;
	
	bool now_transferring;
	
	int render_num;
	int render_mod;
	int render_lines;
	
	bool split_rendering;
	int max_sprite_per_frame;

	bool tvram_enabled;
	bool shadow_memory_enabled;

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

	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);

	void __FASTCALL write_memory_mapped_io8(uint32_t addr, uint32_t data);
	void __FASTCALL write_memory_mapped_io16(uint32_t addr, uint32_t data);
	void __FASTCALL write_memory_mapped_io32(uint32_t addr, uint32_t data);
	
	uint32_t __FASTCALL read_memory_mapped_io8(uint32_t addr);
	uint32_t __FASTCALL read_memory_mapped_io16(uint32_t addr);
	uint32_t __FASTCALL read_memory_mapped_io32(uint32_t addr);

	void reset();
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	void initialize();
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
	void set_context_vram(TOWNS_VRAM *p)
	{
		d_vram = p;
	}
};
}
