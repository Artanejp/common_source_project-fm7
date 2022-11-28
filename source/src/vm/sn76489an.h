/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ SN76489AN ]
*/

#ifndef _SN76489AN_H_
#define _SN76489AN_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_SN76489AN_MUTE	0
#define SIG_SN76489AN_DATA	1
#define SIG_SN76489AN_CS	2
#define SIG_SN76489AN_WE	3

class SN76489AN : public DEVICE
{
private:
	// register
	uint16_t regs[8];
	int index;
	
	// sound info
	struct {
		int count;
		int period;
		int volume;
		bool signal;
	} ch[4];
	uint32_t noise_gen;
	int volume_table[16];
	int diff;
	bool mute, cs, we;
	uint8_t val;
	int volume_l, volume_r;
	
public:
	SN76489AN(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
#ifdef HAS_SN76489
		set_device_name(_T("SN76489 PSG"));
#else
		set_device_name(_T("SN76489AN PSG"));
#endif
	}
	~SN76489AN() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void initialize_sound(int rate, int clock, int volume);
};

#endif

