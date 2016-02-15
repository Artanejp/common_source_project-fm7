/*
	HITACH BASIC Master Jr Emulator 'eBASICMasterJr'

	Author : Takeda.Toshiya
	Date   : 2015.08.28-

	[ memory bus ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_DATAREC_EAR	0

class MEMORY : public DEVICE
{
private:
	// contexts
	DEVICE *d_drec, *d_cpu, *d_pia;
	const uint8* key_stat;
	
	// memory
	uint8 ram[0x10000];
	uint8 basic[0x3000];
	uint8 printer[0x800];
	uint8 monitor[0x1000];
	uint8 font[0x800];
	
	uint8 wdmy[0x800];
	uint8 rdmy[0x800];
	uint8* wbank[32];
	uint8* rbank[32];
	
	uint8 memory_bank;
	void update_bank();
	
	uint8 color_table[0x300];
	uint8 char_color, back_color, mp1710_enb;
	uint8 screen_mode;
	bool screen_reversed;
	scrntype palette_pc[8];
	
	bool drec_in;
	
	uint8 key_column, key_data;
	bool nmi_enb;
	bool break_pressed;
	
	uint8 sound_sample;
	double sound_accum;
	uint32 sound_clock;
	uint32 sound_mix_clock;
	int volume_l, volume_r;
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_frame();
	void mix(int32* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_pia(DEVICE* device)
	{
		d_pia = device;
	}
	void key_down(int code);
	void draw_screen();
};

#endif

