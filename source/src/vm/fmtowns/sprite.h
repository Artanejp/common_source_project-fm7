
#pragma once

#include "../device.h"

#define SIG_TOWNS_SPRITE_HOOK_VLINE     256
#define SIG_TOWNS_SPRITE_SET_LINES      257
#define SIG_TOWNS_SPRITE_TVRAM_ENABLED  258
#define SIG_TOWNS_SPRITE_ANKCG          259

#define SIG_TOWNS_SPRITE_ENABLED        262
#define SIG_TOWNS_SPRITE_BUSY           263
#define SIG_TOWNS_SPRITE_BANK			264
#define SIG_TOWNS_SPRITE_DISP_PAGE1		265
#define SIG_TOWNS_SPRITE_VSYNC          266
#define SIG_TOWNS_SPRITE_FRAME_IN       SIG_TOWNS_SPRITE_VSYNC
#define SIG_TOWNS_SPRITE_RENDER_ENABLED 268
#define SIG_TOWNS_SPRITE_MAX_NUMBERS	269
#define SIG_TOWNS_SPRITE_TEXT_RENDER	270

#define SIG_TOWNS_SPRITE_PEEK_TVRAM     0x00010000
namespace FMTOWNS {
	class FONT_ROMS;
	class TOWNS_CRTC;
	class TOWNS_VRAM;
}

class DEBUGGER;
namespace FMTOWNS {
class TOWNS_SPRITE : public DEVICE
{

protected:
	TOWNS_VRAM *d_vram;
	FONT_ROMS  *d_font;
	TOWNS_CRTC *d_crtc;
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
	bool is_older_sprite;
	double sprite_usec;
	
	int event_busy;

	virtual void __FASTCALL render_sprite(int num,  int x, int y, uint16_t attr, uint16_t color);
	virtual void render_part();
	virtual void __FASTCALL write_reg(uint32_t addr, uint32_t data);
	virtual uint8_t __FASTCALL read_reg(uint32_t addr);
	void check_and_clear_vram();
	virtual uint32_t __FASTCALL get_font_address(uint32_t c, uint8_t &attr);
	virtual void render_text();
	virtual inline double get_sprite_usec(int num)
	{
		__LIKELY_IF(!(is_older_sprite)) {
			int num_limit = max(1024, num);
			num_limit = min(224, num_limit);
			return (1.0e6 / 59.94) / ((double)(num_limit * 2)); // OK?
		}
		return 57.0;
	}
private:
	inline void shift_vector_data(size_t _xstart, size_t _xend, size_t _xshift, csp_vector8<uint16_t> _lbuf[]);
public:
	TOWNS_SPRITE(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_vram = NULL;
		d_font = NULL;
		d_crtc = NULL;
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
	void set_context_font(FONT_ROMS* dev)
	{
		d_font = dev;
	}
	void set_context_crtc(TOWNS_CRTC *p)
	{
		d_crtc = p;
	}
	void set_context_debugger(DEBUGGER *p)
	{
		d_debugger = p;
	}
};

inline void TOWNS_SPRITE::shift_vector_data(size_t _xstart, size_t _xend, size_t _xshift, csp_vector8<uint16_t> _lbuf[])
{
	__UNLIKELY_IF((_xshift < 0) || (_xshift > 1)) {
		return; // NOP
	}
	size_t __lstart = _xstart << _xshift;
	size_t __lend = _xend << _xshift;
	size_t __lwidth = __lend - __lstart;
	csp_vector8<uint16_t> mask_transparent(0x8000);
	__UNLIKELY_IF(__lwidth > 16) {
		__lwidth = 16;
	}
	__UNLIKELY_IF(__lstart >= 16) {
		return; // NOP
	}
	__UNLIKELY_IF(__lend >= 16) {
		__lend = 16;
	}
	__LIKELY_IF((__lstart >= 0) && (__lend >= 0) && (__lwidth > 0)) {
		__LIKELY_IF(__lwidth > 0) {
			if(__lstart > 0) {
				// Shift data.
				if(__lstart >= 8) {
					_lbuf[0] = mask_transparent;
					for(size_t xs = (__lstart - 8), xt = 0 ; xs < (__lend - 8) ; xs++, xt++) {
						_lbuf[0].set(xt, _lbuf[1].at(xs));
					}
					_lbuf[1] = mask_transparent;
				} else {
					// __lstart < 8
					// Set within _lbuf[0]
					size_t __lend2 = (__lend >= 8) ? 8 : __lend;  
					for(size_t xs = __lstart, xt = 0 ; xs < __lend2 ; xs++, xt++) {
						_lbuf[0].set(xt, _lbuf[0].at(xs));
					}
					// shift _lbuf[1]
					if(__lend > 8) {
						size_t __lend3 = __lend - 8;
						for(size_t xs = 0, xt = (8 - __lstart); xs < __lend3; xs++, xt++) {
							_lbuf[0].set(xt, _lbuf[1].at(xs));
						}
						for(size_t xt = __lend3; xt < 8; xt++) {
							_lbuf[1].set(xt, 0x8000);
						}
					} else {
						for(size_t xt = __lend2; xt < 8; xt++) {
							_lbuf[0].set(xt, 0x8000);
						}
						_lbuf[1] = mask_transparent;
					}
				}
			} else if(__lend < 16) { // __lstart == 0 and less width.
				if(__lend < 8) {
					for(size_t xt = __lend; xt < 8; xt++) {
						_lbuf[0].set(xt, 0x8000);
					}
					_lbuf[1] = mask_transparent;
				} else {
					for(size_t xt = (__lend - 8) ; xt < 8; xt++) {
						_lbuf[1].set(xt, 0x8000);
					}
				}
			}
		}
	}
}

}
