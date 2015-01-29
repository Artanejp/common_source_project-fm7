/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.13 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MEMORY : public DEVICE
{
private:
	DEVICE* d_crtc;
	
	uint8* rbank[8192];	// 16MB / 2KB
	uint8* wbank[8192];
	uint8 wdmy[0x800];
	uint8 rdmy[0x800];
	uint8 ram[0xc0000];	// Main RAM 768KB
	uint8 ext[0x600000];	// Ext RAM 6MB
	uint8 vram[0x80000];	// VRAM 512KB
	uint8 tvram[0x2000];	// Text VRAM 6KB + dummy 2KB
	uint8 pcg[0x4000];	// PCG 8KB (even addr only)
	uint8 ipl[0x10000];	// IPL 64KB
	uint8 dic[0x40000];	// Dictionary ROM 256KB
	uint8 kanji[0x80000];	// Kanji ROM 256KB (even addr only)
	
	uint32 mem_window;
	bool ipl_enabled;
	uint8 vram_bank, dic_bank, kanji_bank;
	
	void update_vram_map();
	void update_dic_map();
	void update_kanji_map();
	void update_ipl_map();
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_dma_data8(uint32 addr, uint32 data);
	uint32 read_dma_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unitque functions
	void set_context_crtc(DEVICE* device)
	{
		d_crtc = device;
	}
	uint8* get_vram()
	{
		return vram;
	}
	uint8* get_tvram()
	{
		return tvram;
	}
	uint8* get_kanji()
	{
		return kanji;
	}
	uint8* get_pcg()
	{
		return pcg;
	}
};

#endif

