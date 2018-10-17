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
	const uint8_t* key_stat;
	
	// memory
	uint8_t ram[0x10000];
	uint8_t basic[0x3000];
	uint8_t printer[0x800];
	uint8_t monitor[0x1000];
	uint8_t font[0x800];
	
	uint8_t wdmy[0x800];
	uint8_t rdmy[0x800];
	uint8_t* wbank[32];
	uint8_t* rbank[32];
	
	uint8_t memory_bank;
	void update_bank();
	
	uint8_t color_table[0x300];
	uint8_t char_color, back_color, mp1710_enb;
	uint8_t screen_mode;
	bool screen_reversed;
	scrntype_t palette_pc[8];
	
	bool drec_bit, drec_in;
	uint32_t drec_clock;
	
	uint8_t key_column, key_data;
	bool nmi_enb;
	bool break_pressed;
	
	uint8_t sound_sample;
	double sound_accum;
	uint32_t sound_clock;
	uint32_t sound_mix_clock;
	int volume_l, volume_r;
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	bool process_state(FILEIO* state_fio, bool loading);
	
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

