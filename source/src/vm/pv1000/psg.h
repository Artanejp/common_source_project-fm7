/*
	CASIO PV-1000 Emulator 'ePV-1000'

	Author : Takeda.Toshiya
	Date   : 2006.11.16 -

	[ psg ]
*/

#ifndef _PSG_H_
#define _PSG_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class PSG : public DEVICE
{
private:
	typedef struct {
		int count;
		int period;
		bool signal;
	} channel_t;
	channel_t ch[3];
	int diff;
	
public:
	PSG(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~PSG() {}
	
	// common functions
	void reset();
	void write_io8(uint32 addr, uint32 data);
	void mix(int32* buffer, int cnt);
	
	// unique function
	void init(int rate);
};

#endif

