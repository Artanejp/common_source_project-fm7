/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'

	Author : Takeda.Toshiya
	Date   : 2013.03.14-

	I-O DATA PIO-3039 Emulator

	Author : Mr.Suga
	Data   : 2017.02.22-

	[ memory/crtc ]
*/

#ifndef _MEMORY_80B_H_
#define _MEMORY_80B_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_VRAM_SEL	0
#define SIG_CRTC_WIDTH80	1
#define SIG_CRTC_REVERSE	2
#define SIG_CRTC_VGATE		3

class Z80;

namespace MZ80B {

class MEMORY : public DEVICE
{
private:
	Z80 *d_cpu;
	DEVICE *d_pio;
	
	// memory
	uint8_t* rbank[32];
	uint8_t* wbank[32];
	bool is_vram[32];
	uint8_t wdmy[0x800];
	uint8_t rdmy[0x800];
	uint8_t ram[0x10000];
#ifndef _MZ80B
	uint8_t vram[0x10000];	// 0x4000 * (3 pages + dummy)
#else
	uint8_t vram[0xc000];	// 0x4000 * (2 pages + dummy)
#endif
	uint8_t tvram[0x1000];
	uint8_t ipl[0x800];
	
	bool ipl_selected;
	uint8_t vram_sel, vram_page;
	void update_vram_map();
	
	// crtc
#ifndef _MZ80B
	scrntype_t palette_color[8];
#endif
	scrntype_t palette_green[2];
	uint8_t font[0x800];
	uint8_t screen_txt[200][640];
	uint8_t screen_gra[200][640];
	
	uint8_t back_color, text_color, vram_mask;
	bool width80, reverse;
	bool vgate;
	bool hblank;
	
#ifdef _MZ80B
	// PIO-3039
	scrntype_t pio3039_color[8];
	uint8_t pio3039_palette[8];
	uint8_t screen_80b_green[200][640];
	uint8_t screen_80b_vram1[200][640];
	uint8_t screen_80b_vram2[200][640];
	
	bool pio3039_txt_sw;
	uint8_t pio3039_data;
#else
	void update_green_palette();
#endif
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus(MZ80B/2000/2200)"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize() override;
	void reset() override;
	void special_reset(int num) override;
	void __FASTCALL write_data8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_data8(uint32_t addr) override;
	uint32_t __FASTCALL fetch_op(uint32_t addr, int *wait) override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	void event_vline(int v, int clock) override;
	void __FASTCALL event_callback(int event_id, int err) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// unique function
	void set_context_cpu(Z80* device)
	{
		d_cpu = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void load_dat_image(const _TCHAR* file_path);
	bool load_mzt_image(const _TCHAR* file_path);
	void draw_screen();
};

}
#endif
