/*
	Skelton for retropc emulator

	Origin : Neko Project 2
	Author : Takeda.Toshiya
	Date   : 2015.09.29-

	[ TMS3631 ]
*/

#ifndef _TMS3631_H_
#define _TMS3631_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_TMS3631_ENVELOP1	0
#define SIG_TMS3631_ENVELOP2	1
#define SIG_TMS3631_DATAREG	2
#define SIG_TMS3631_MASKREG	3

class TMS3631 : public DEVICE
{
private:
	uint8 envelop1, envelop2, datareg, maskreg;
	
	struct {
		uint32 freq;
		uint32 count;
	} ch[8];
	uint8 channel;
	bool set_key;
	
	uint32 freq_table[64];
	int vol, feet[16];
	int volume_l, volume_r;
	
public:
	TMS3631(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
	}
	~TMS3631() {}
	
	// common functions
	void reset();
	void write_signal(int id, uint32 data, uint32 mask);
	void mix(int32* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	void initialize_sound(int rate, int volume);
};

#endif

