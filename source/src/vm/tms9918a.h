/*
	Skelton for retropc emulator

	Origin : MAME TMS9928A Core
	Author : Takeda.Toshiya
	Date   : 2006.08.18 -
	         2007.07.21 -

	[ TMS9918A ]
*/

#ifndef _TMS9918A_H_
#define _TMS9918A_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

#define SIG_TMS9918A_SUPER_IMPOSE	0

class DEBUGGER;
class TMS9918A : public DEVICE
{
private:
	DEBUGGER *d_debugger;
	
	// output signals
	outputs_t outputs_irq;
	
	//uint8_t vram[TMS9918A_VRAM_SIZE];
	uint8_t *vram;
	uint8_t screen[192][256];
	uint8_t regs[8], status_reg, read_ahead, first_byte;
	uint16_t vram_addr;
	bool latch, intstat;
	uint16_t color_table, pattern_table, name_table;
	uint16_t sprite_pattern, sprite_attrib;
	uint16_t color_mask, pattern_mask;
//#ifdef TMS9918A_SUPER_IMPOSE
	bool now_super_impose;
//#endif

	bool _use_alpha_blending_to_impose;
	bool _tms9918a_super_impose;
	bool _tms9918a_limit_sprites;
	uint32_t _VRAM_SIZE;
	uint32_t _ADDR_MASK;
	int _SCREEN_WIDTH;
	const scrntype_t *palette_pc;
	
	void set_intstat(bool val);
	void draw_mode0();
	void draw_mode1();
	void draw_mode2();
	void draw_mode12();
	void draw_mode3();
	void draw_mode23();
	void draw_modebogus();
	void draw_sprites();

	inline void draw_screen_512_impose();
	inline void draw_screen_512_nonimpose();
	inline void draw_screen_256_impose();
	inline void draw_screen_256_nonimpose();
	
public:
	TMS9918A(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_irq);
		vram = NULL;
//#ifdef TMS9918A_SUPER_IMPOSE
		now_super_impose = false;
//#endif
		_use_alpha_blending_to_impose = false;
		_tms9918a_super_impose = _tms9918a_limit_sprites = false;
		_VRAM_SIZE = 0x4000;
		_ADDR_MASK = 0x3fff;
		d_debugger = NULL;
		set_device_name(_T("TMS9918A VDP"));
	}
	~TMS9918A() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	// for debugging
	void write_via_debugger_data8(uint32_t addr, uint32_t data);
	uint32_t read_via_debugger_data8(uint32_t addr);
	void write_via_debugger_io8(uint32_t addr, uint32_t data);
	uint32_t read_via_debugger_io8(uint32_t addr);
//#ifdef TMS9918A_SUPER_IMPOSE
	void write_signal(int id, uint32_t data, uint32_t mask);
//#endif
	void event_vline(int v, int clock);
//#ifdef USE_DEBUGGER
	bool is_debugger_available()
	{
		return true;
	}
	void *get_debugger()
 	{
		return d_debugger;
	}
	uint64_t get_debug_data_addr_space()
	{
		return _VRAM_SIZE;
 	}
	void write_debug_data8(uint32_t addr, uint32_t data)
	{
		if(addr < _VRAM_SIZE) {
			write_via_debugger_data8(addr, data);
		}
	}
	uint32_t read_debug_data8(uint32_t addr)
	{
		if(addr < _VRAM_SIZE) {
			return read_via_debugger_data8(addr);
		}
		return 0;
	}
	void write_debug_io8(uint32_t addr, uint32_t data)
	{
		if(addr < 8) {
			write_via_debugger_io8(addr, data);
		}
	}
	uint32_t read_debug_io8(uint32_t addr)
	{
		if(addr < 8) {
			return read_via_debugger_io8(addr);
		}
		return 0;
 	}
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
//#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
	void draw_screen();
};

#endif

