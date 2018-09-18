
#ifndef _TOWNS_SPRITE_H_
#define _TOWNS_SPRITE_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_FMTOWNS_SPRITE_RENDER 256
#define SIG_FMTOWNS_SPRITE_UPDATE_FRAMEBUFFER 257
#define SIG_FMTOWNS_RENDER_SPRITE_CHANGE_BANK 258
#define SIG_FMTOWNS_RENDER_SPRITE_ENABLED     259
#define SIG_FMTOWNS_RENDER_SPRITE_RESET       260
#define SIG_FMTOWNS_RENDER_SPRITE_SET_LIMIT   261
#define SIG_FMTOWNS_RENDER_SPRITE_SET_NUM     262
#define SIG_FMTOWNS_RENDER_SPRITE_CLEAR_VRAM  263

class TOWNS_VRAM;

#define TOWNS_SPRITE_CACHE_NUM 512

class TOWNS_SPRITE : public DEVICE
{
private:
	TOWNS_VRAM *vram_head;

	// REGISTERS
	uint8_t reg_addr;
	uint8_t reg_data[8];
	// #0, #1
	bool reg_spen;
	uint16_t reg_index;

	uint16_t reg_voffset;
	uint16_t reg_hoffset;
	bool disp_page0;
	bool disp_page1;
	
	int32_t splite_limit;

	uint16_t index_ram[4096]; // 1024 * 4
	uint16_t color_ram[4096]; // 16 * 256
	uint8_t pattern_ram[(65536 - (4096 * 2)) * 2];

	bool pattern_cached[((65536 - (4096 * 2)) * 2) / (8 * 16)];
	bool color_cached[256];
protected:

	
public:
	TOWNS_SPRITE(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
		set_device_name(_T("SPRITE"));
		vram_head = NULL;
		framebuffer = NULL;
		
	}
	~TOWNS_SPRITE() {}
	

	void write_data8(uint32_t addr, uint32_t data);
	void write_data16(uint32_t addr, uint32_t data);
	void write_data32(uint32_t addr, uint32_t data);
	
	uint32_t read_data8(uint32_t addr);
	uint32_t read_data16(uint32_t addr);
	uint32_t read_data32(uint32_t addr);

	void write_signal(int id, uint32_t data, uint32_t mask);
	
	void initialize();

	void set_context_vram(TOWNS_VRAM *p)
	{
		vram_head = p;
	}
	void save_state(FILEIO *fio);
	bool load_stste(FILEIO *fio);
};

#endif /* _TOWNS_SPRITE_H */
