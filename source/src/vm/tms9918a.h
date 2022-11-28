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

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_TMS9918A_SUPER_IMPOSE	0

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class TMS9918A : public DEVICE
{
private:
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
#endif
	
	// output signals
	outputs_t outputs_irq;
	
	uint8_t vram[TMS9918A_VRAM_SIZE];
	uint8_t screen[192][256];
	uint8_t regs[8], status_reg, read_ahead, first_byte;
	uint16_t vram_addr;
	bool latch, intstat;
	uint16_t color_table, pattern_table, name_table;
	uint16_t sprite_pattern, sprite_attrib;
	uint16_t color_mask, pattern_mask;
#ifdef TMS9918A_SUPER_IMPOSE
	bool now_super_impose;
#endif
	
	void set_intstat(bool val);
	void draw_mode0();
	void draw_mode1();
	void draw_mode2();
	void draw_mode12();
	void draw_mode3();
	void draw_mode23();
	void draw_modebogus();
	void draw_sprites();
	
public:
	TMS9918A(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_irq);
#ifdef TMS9918A_SUPER_IMPOSE
		now_super_impose = false;
#endif
#ifdef USE_DEBUGGER
		d_debugger = NULL;
#endif
		set_device_name(_T("TMS9918A VDP"));
	}
	~TMS9918A() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	// for debugging
	void write_via_debugger_data8(uint32_t addr, uint32_t data);
	uint32_t read_via_debugger_data8(uint32_t addr);
	void write_via_debugger_io8(uint32_t addr, uint32_t data);
	uint32_t read_via_debugger_io8(uint32_t addr);
#ifdef TMS9918A_SUPER_IMPOSE
	void write_signal(int id, uint32_t data, uint32_t mask);
#endif
	void event_vline(int v, int clock);
#ifdef USE_DEBUGGER
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
		return TMS9918A_VRAM_SIZE;
	}
	void write_debug_data8(uint32_t addr, uint32_t data)
	{
		if(addr < TMS9918A_VRAM_SIZE) {
			write_via_debugger_data8(addr, data);
		}
	}
	uint32_t read_debug_data8(uint32_t addr)
	{
		if(addr < TMS9918A_VRAM_SIZE) {
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
#endif
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

