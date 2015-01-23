/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ pac slot 2 ]
*/

#include "pac2.h"
#include "pac2dev.h"
#include "rampac2.h"
#include "kanjipac2.h"
#include "joypac2.h"
#include "../../fileio.h"

void PAC2::initialize()
{
	// slot 4 : ram pack #5
	// slot 3 : ram pack #6
	// slot 2 : kanji rom
	// slot 1 : joystick
	dummy = new PAC2DEV(vm, emu);
	rampac2[0] = new RAMPAC2(vm, emu);
	rampac2[1] = new RAMPAC2(vm, emu);
	kanji = new KANJIPAC2(vm, emu);
	joy = new JOYPAC2(vm, emu);
	
	rampac2[0]->initialize(1);
	rampac2[1]->initialize(2);
	kanji->initialize(3);
	joy->initialize(4);
	
	dev[7] = dummy;
	dev[6] = dummy;
	dev[5] = dummy;
	dev[4] = rampac2[0];
	dev[3] = rampac2[1];
	dev[2] = kanji;
	dev[1] = joy;
	dev[0] = dummy;
	
	sel = 0;
}

void PAC2::release()
{
	delete dummy;
	rampac2[0]->release();
	delete rampac2[0];
	rampac2[1]->release();
	delete rampac2[1];
	delete kanji;
	delete joy;
}

void PAC2::reset()
{
	rampac2[0]->reset();
	rampac2[1]->reset();
	kanji->reset();
}

void PAC2::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0x18:
	case 0x19:
	case 0x1a:
		dev[sel]->write_io8(addr, data);
		break;
	case 0x1b:
		if(data & 0x80) {
			dev[sel]->write_io8(addr, data);
		} else {
			sel = data & 7;
		}
		break;
	}
}

uint32 PAC2::read_io8(uint32 addr)
{
	return dev[sel]->read_io8(addr);
}

void PAC2::open_rampac2(int drv, _TCHAR* file_path)
{
	rampac2[drv]->open_file(file_path);
}

#define STATE_VERSION	1

void PAC2::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputInt32(sel);
	rampac2[0]->save_state(state_fio);
	rampac2[1]->save_state(state_fio);
	kanji->save_state(state_fio);
}

bool PAC2::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	sel = state_fio->FgetInt32();
	if(!rampac2[0]->load_state(state_fio)) {
		return false;
	}
	if(!rampac2[1]->load_state(state_fio)) {
		return false;
	}
	if(!kanji->load_state(state_fio)) {
		return false;
	}
	return true;
}

