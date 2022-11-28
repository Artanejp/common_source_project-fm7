/*
	Skelton for retropc emulator

	Author : Takeo.Namiki
	Date   : 2013.10.26-

	[ YM2413 ]
*/

#ifndef _YM2413_H_
#define _YM2413_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_YM2413_MUTE		0

typedef INT16 SAMP;
typedef void (*OPLL_UPDATEHANDLER)(int param,int min_interval_us);
void YM2413SetUpdateHandler(int which, OPLL_UPDATEHANDLER UpdateHandler, int param);

class YM2413 : public DEVICE
{
private:
	uint8_t latch;
	uint8_t reg[0x40];
	bool mute;
	INT16 *buf[2];
	int volume_l, volume_r;
	
public:
	YM2413(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
		set_device_name(_T("YM2413 OPLL"));
	}
	~YM2413() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	
	// unique functions
	void initialize_sound(int rate, int clock, int samples);
};

#endif
