
#pragma once

#include "../device.h"

#define SIG_TOWNS_SPRITE_HOOK_VLINE     256
#define SIG_TOWNS_SPRITE_SET_LINES      257
#define SIG_TOWNS_SPRITE_TVRAM_ENABLED  258
#define SIG_TOWNS_SPRITE_ANKCG          259

#define SIG_TOWNS_SPRITE_ENABLED        262
#define SIG_TOWNS_SPRITE_BUSY           263
#define SIG_TOWNS_SPRITE_DISP_PAGE0		264
#define SIG_TOWNS_SPRITE_DISP_PAGE1		265
#define SIG_TOWNS_SPRITE_VSYNC          266
#define SIG_TOWNS_SPRITE_FRAME_IN       SIG_TOWNS_SPRITE_VSYNC
#define SIG_TOWNS_SPRITE_RENDER_ENABLED 268

#define SIG_TOWNS_SPRITE_PEEK_TVRAM     0x00010000
namespace FMTOWNS {
	class TOWNS_VRAM;
}

class DEBUGGER;
namespace FMTOWNS {
class TOWNS_SPRITE : public DEVICE
{

protected:
	TOWNS_VRAM *d_vram;
//	DEVICE *d_font;
//	DEVICE *d_crtc;
	DEBUGGER *d_debugger;
	// REGISTERS
	uint8_t reg_addr;
	uint8_t reg_data[8];
	// #0, #1
	uint16_t reg_ctrl;

	bool reg_spen;
	uint16_t reg_index;
	uint8_t pattern_ram[0x20000];
//	uint8_t ram[0x3000];

	uint16_t reg_voffset;
	uint16_t reg_hoffset;
	bool disp_page0;
	bool disp_page1;
	bool draw_page1;

	bool frame_out;
	bool sprite_enabled;
	bool sprite_busy;
	bool page_changed;

	int render_num;
	int max_sprite_per_frame;

	bool tvram_enabled;
	bool tvram_enabled_bak;

	bool ankcg_enabled;
	int event_busy;

	virtual void __FASTCALL render_sprite(int num,  int x, int y, uint16_t attr, uint16_t color);
	virtual void render_part();
	virtual void __FASTCALL write_reg(uint32_t addr, uint32_t data);
	virtual uint8_t __FASTCALL read_reg(uint32_t addr);
	void check_and_clear_vram();

public:
	TOWNS_SPRITE(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_vram = NULL;
//		d_font = NULL;
//		d_crtc = NULL;
		set_device_name(_T("SPRITE"));
	}
	~TOWNS_SPRITE() {}

	void reset() override;
	void initialize() override;

	void event_frame() override;
	void event_pre_frame() override;

	void __FASTCALL event_callback(int id, int err) override;

	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;

	uint32_t __FASTCALL read_dma_data8w(uint32_t addr, int* wait) override;
	uint32_t __FASTCALL read_dma_data16w(uint32_t addr, int* wait) override;
	void __FASTCALL write_dma_data8w(uint32_t addr, uint32_t data, int* wait) override;
	void __FASTCALL write_dma_data16w(uint32_t addr, uint32_t data, int* wait) override;

	uint32_t __FASTCALL read_memory_mapped_io8w(uint32_t addr, int* wait) override;
	uint32_t __FASTCALL read_memory_mapped_io16w(uint32_t addr, int* wait) override;
	uint32_t __FASTCALL read_memory_mapped_io32w(uint32_t addr, int* wait) override;

	void __FASTCALL write_memory_mapped_io8w(uint32_t addr, uint32_t data, int* wait) override;
	void __FASTCALL write_memory_mapped_io16w(uint32_t addr, uint32_t data, int* wait) override;
	void __FASTCALL write_memory_mapped_io32w(uint32_t addr, uint32_t data, int* wait) override;

	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	uint32_t __FASTCALL read_signal(int id) override;

	bool is_debugger_available() override
	{
		return true;
	}
	void *get_debugger() override
	{
		return d_debugger;
	}
	uint64_t get_debug_data_addr_space() override
	{
		return 0x20000;
	}
	uint32_t __FASTCALL read_debug_data8(uint32_t addr) override
	{
		if(addr >= 0x20000) {
			return 0x00;
		}
		return pattern_ram[addr];
	}
	void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data) override
	{
		if(addr >= 0x20000) {
			return;
		}
		if(addr < 0x1000) {
			tvram_enabled = true;
			tvram_enabled_bak = true;
		} else if((addr >= 0x2000) && (addr < 0x3000)) {
			tvram_enabled = true;
			tvram_enabled_bak = true;
		}
		pattern_ram[addr] = (uint8_t)data;
	}
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len) override;
	bool write_debug_reg(const _TCHAR *reg, uint32_t data) override;

	bool process_state(FILEIO* state_fio, bool loading) override;
	/*
	  Unique functions
	*/
	void set_context_vram(TOWNS_VRAM *p)
	{
		d_vram = p;
	}
//	void set_context_font(DEVICE *p)
//	{
//		d_font = p;
//	}
//	void set_context_crtc(DEVICE *p)
//	{
//		d_crtc = p;
//	}
	void set_context_debugger(DEBUGGER *p)
	{
		d_debugger = p;
	}
	inline void __FASTCALL get_tvram_snapshot(uint8_t *p)
	{
		__LIKELY_IF(p != NULL) {
			memcpy(&(p[0x0000]), &(pattern_ram[0x0000]), 0x4000);
		}
	}
};
}
