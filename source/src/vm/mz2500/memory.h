/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.11.24 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_HBLANK	0
#define SIG_MEMORY_VBLANK	1

class MEMORY : public DEVICE
{
private:
	DEVICE *d_cpu, *d_crtc;
	
	uint8* rbank[32];
	uint8* wbank[32];
	uint8 wdmy[0x800];
	uint8 rdmy[0x800];
	uint8 ram[0x40000];	// Main RAM 256KB
	uint8 vram[0x20000];	// VRAM 128KB
	uint8 tvram[0x1800];	// Text VRAM 6KB
	uint8 pcg[0x2000];	// PCG 0-3 8KB
	uint8 ipl[0x8000];	// IPL 32KB
	uint8 dic[0x40000];	// Dictionary ROM 256KB
	uint8 kanji[0x40000];	// Kanji ROM (low) / Kanji ROM (high) 128KB + 128KB
	uint8 phone[0x8000];	// Phone ROM 32KB
	
	uint8 bank;
	uint8 page[8];
	int page_type[8];
	int page_wait[8];
	bool is_vram[8];
	uint8 dic_bank;
	uint8 kanji_bank;
	bool blank, hblank, vblank, busreq;
	
	void set_map(uint8 data);
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void special_reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_data8w(uint32 addr, uint32 data, int* wait);
	uint32 read_data8w(uint32 addr, int* wait);
	uint32 fetch_op(uint32 addr, int* wait);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unitque function
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
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

