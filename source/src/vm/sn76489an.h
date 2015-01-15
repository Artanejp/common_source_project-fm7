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
	uint16 regs[8];
	int index;
	
	// sound info
	struct {
		int count;
		int period;
		int volume;
		bool signal;
	} ch[4];
	uint32 noise_gen;
	int volume_table[16];
	int diff;
	bool mute, cs, we;
	uint8 val;
	
public:
	SN76489AN(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SN76489AN() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	void write_signal(int id, uint32 data, uint32 mask);
	void mix(int32* buffer, int cnt);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	void init(int rate, int clock, int volume);
};

#endif

