/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

	Author : Takeda.Toshiya
	Date   : 2009.03.14-

	[ 8bit i/o bus ]
*/

#ifndef _IOBUS_H_
#define _IOBUS_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_IOBUS_MODE	0

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class IOBUS : public DEVICE
{
private:
	DEVICE *d_cpu, *d_display, *d_io;
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
#endif
	
	// vram
#ifdef _X1TURBO_FEATURE
	uint8_t vram[0x18000];
#else
	uint8_t vram[0xc000];
#endif
	bool vram_mode, signal;
	
	int vram_ofs_b;
	int vram_ofs_r;
	int vram_ofs_g;
	
	uint8_t vdisp;
	
	uint32_t prev_clock, vram_wait_index;
	bool column40;
#ifdef _X1TURBO_FEATURE
	uint8_t crtc_regs[18];
	int crtc_ch;
	bool hireso;
#ifdef _X1TURBOZ
	uint8_t zmode1;
	uint8_t zmode2;
#endif
#endif
	void write_port8(uint32_t addr, uint32_t data, bool is_dma, int* wait);
	uint32_t read_port8(uint32_t addr, bool is_dma, int* wait);
	int get_vram_wait();
	
public:
	IOBUS(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("I/O Bus (VRAM)"));
	}
	~IOBUS() {}
	
	// common functions
	void initialize();
	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void write_io8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_io8w(uint32_t addr, int* wait);
	void write_dma_io8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_dma_io8w(uint32_t addr, int* wait);
	// for debugging vram
	void write_via_debugger_data8(uint32_t addr, uint32_t data);
	uint32_t read_via_debugger_data8(uint32_t addr);
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
		return sizeof(vram);
	}
	void write_debug_data8(uint32_t addr, uint32_t data)
	{
		if(addr < sizeof(vram)) {
			write_via_debugger_data8(addr, data);
		}
	}
	uint32_t read_debug_data8(uint32_t addr)
	{
		if(addr < sizeof(vram)) {
			return read_via_debugger_data8(addr);
		}
		return 0;
	}
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_display(DEVICE* device)
	{
		d_display = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	uint8_t* get_vram()
	{
		return vram;
	}
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
};

#endif

