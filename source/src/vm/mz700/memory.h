/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2008.06.05 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#if defined(_MZ800)
class DISPLAY;
#endif

class MEMORY : public DEVICE
{
private:
	DEVICE *d_cpu, *d_pit, *d_pio;
#if defined(_MZ800)
	DEVICE *d_pio_int;
#endif
#if defined(_MZ700) || defined(_MZ1500)
	DEVICE *d_joystick;
#endif
	
	// memory
	uint8_t* rbank[32];
	uint8_t* wbank[32];
	uint8_t wdmy[0x800];
	uint8_t rdmy[0x800];
	
	uint8_t ipl[0x1000];	// IPL 4KB
#if defined(_MZ800)
	uint8_t ext[0x2000];	// MZ-800 IPL 8KB
#elif defined(_MZ1500)
	uint8_t ext[0x1800];	// MZ-1500 EXT 6KB
#endif
	uint8_t font[0x1000];	// CGROM 4KB
#if defined(_MZ700)
	uint8_t pcg[0x1000];	// PCG-700 2KB + Lower CGROM 2KB
#elif defined(_MZ1500)
	uint8_t pcg[0x6000];	// MZ-1500 PCG 8KB * 3
#endif
	uint8_t ram[0x10000];	// Main RAM 64KB
#if defined(_MZ800)
	uint8_t vram[0x8000];	// MZ-800 VRAM 32KB
#else
	uint8_t vram[0x1000];	// MZ-700/1500 VRAM 4KB
#endif
	uint8_t mem_bank;
#if defined(_MZ700)
	uint8_t pcg_data;
	uint8_t pcg_addr;
	uint8_t pcg_ctrl;
#elif defined(_MZ800)
	uint8_t wf, rf;
	uint8_t dmd;
	uint32_t vram_addr_top;
	bool is_mz800;
#elif defined(_MZ1500)
	uint8_t pcg_bank;
#endif
	
	void update_map_low();
	void update_map_middle();
	void update_map_high();
#if defined(_MZ800)
	int vram_page_mask(uint8_t f);
	int vram_addr(int addr);
#endif
	
	// crtc
#if defined(_MZ800)
	uint16_t sof;
	uint8_t sw, ssa, sea;
	uint8_t palette_sw, palette[4], palette16[16];
#elif defined(_MZ1500)
	uint8_t priority, palette[8];
#endif
	bool blink, tempo;
	bool blank;
	bool hblank, hsync;
	bool vblank, vsync;
#if defined(_MZ700) || defined(_MZ1500)
	bool blank_vram;
#endif
#if defined(_MZ1500)
	bool blank_pcg;
#endif
	
	void set_blank(bool val);
	void set_hblank(bool val);
	void set_hsync(bool val);
	void set_vblank(bool val);
	void set_vsync(bool val);
	
	// renderer
#if defined(_MZ800)
	uint8_t screen[200][640];
	scrntype_t palette_mz800_pc[16];
#else
	uint8_t screen[200][320];
#endif
	scrntype_t palette_pc[8];
	
#if defined(_MZ800)
	void draw_line_320x200_2bpp(int v);
	void draw_line_320x200_4bpp(int v);
	void draw_line_640x200_1bpp(int v);
	void draw_line_640x200_2bpp(int v);
	void draw_line_mz700(int v);
#endif
	void draw_line(int v);
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
#if defined(_MZ800)
	void update_config();
#endif
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_data8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_data8w(uint32_t addr, int* wait);
	void write_io8(uint32_t addr, uint32_t data);
#if defined(_MZ800)
	uint32_t read_io8(uint32_t addr);
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_pit(DEVICE* device)
	{
		d_pit = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
#if defined(_MZ800)
	void set_context_pio_int(DEVICE* device)
	{
		d_pio_int = device;
	}
#endif
#if defined(_MZ700) || defined(_MZ1500)
	void set_context_joystick(DEVICE* device)
	{
		d_joystick = device;
	}
#endif
	void draw_screen();
};

#endif

