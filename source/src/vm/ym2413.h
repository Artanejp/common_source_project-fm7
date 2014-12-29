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

typedef INT16 SAMP;
typedef void (*OPLL_UPDATEHANDLER)(int param,int min_interval_us);
void YM2413SetUpdateHandler(int which, OPLL_UPDATEHANDLER UpdateHandler, int param);

class YM2413 : public DEVICE
{
private:
	uint8 latch;
	uint8 reg[0x40];
	bool mute;
	INT16 *buf[2];
public:
	YM2413(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~YM2413() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void mix(int32* buffer, int cnt);
	
	// unique functions
	void init(int rate, int clock, int samples);
};

#endif
