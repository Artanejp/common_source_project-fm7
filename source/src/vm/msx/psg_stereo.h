/*
	Common Source Code Project
	MSX Series (experimental)

	Author : umaiboux
	Date   : 2016.03.xx-

	[ PSG Stereo ]
*/

#ifndef _PSG_STEREO_H_
#define _PSG_STEREO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

//#include "../ym2203.h"
#include "../ay_3_891x.h"

class PSG_STEREO : public DEVICE
{
private:
//	YM2203* d_psg[3];
	AY_3_891X* d_psg[3];
	uint8_t rreg[14];
	uint8_t ch;
	void sound(int dev, int reg, int val);
	int m_stereo;
	int m_decibel_l;
	int m_decibel_r;

public:
	PSG_STEREO(VM_TEMPLATE* parent_vm, EMU* parent_emu);
	~PSG_STEREO() {}
	
	// common functions
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void initialize();
	void release();
	void reset();
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	void update_config();
	
	// unique functions
	void initialize_sound(int rate, int clock, int samples, int decibel_fm, int decibel_psg);
//	void set_context_psg(YM2203* device)
	void set_context_psg(AY_3_891X* device)
	{
		d_psg[0] = device;
	}
};

#endif
