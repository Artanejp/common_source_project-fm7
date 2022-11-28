/*
	TOSHIBA PASOPIA Emulator 'EmuPIA'

	Author : Takeda.Toshiya
	Date   : 2006.12.28 -

	[ pac slot 2 ]
*/

#include "pac2.h"
#include "pac2dev.h"
#include "rampac2.h"
#include "kanjipac2.h"
#include "joypac2.h"

void PAC2::initialize()
{
	rampac2 = new RAMPAC2(vm, emu);
	kanji = new KANJIPAC2(vm, emu);
	joy = new JOYPAC2(vm, emu);
	dummy = new PAC2DEV(vm, emu);
	
	rampac2->initialize(1);
	kanji->initialize(1);
	joy->initialize(1);
	dummy->initialize(1);
}

void PAC2::release()
{
	rampac2->release();
	delete rampac2;
	delete kanji;
	delete joy;
	delete dummy;
}

void PAC2::reset()
{
	device_type = config.device_type;
	get_device()->reset();
}

void PAC2::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0x18:
	case 0x19:
	case 0x1a:
	case 0x1b:
		get_device()->write_io8(addr, data);
		break;
	}
}

uint32_t PAC2::read_io8(uint32_t addr)
{
	return get_device()->read_io8(addr);
}

PAC2DEV* PAC2::get_device()
{
	switch(device_type) {
	case DEVICE_RAM_PAC:
		return rampac2;
	case DEVICE_KANJI_ROM:
		return kanji;
	case DEVICE_JOYSTICK:
		return joy;
	}
	return dummy;
}

void PAC2::open_rampac2(const _TCHAR* file_path)
{
	rampac2->open_file(file_path);
}

#define STATE_VERSION	1

bool PAC2::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(device_type);
	return get_device()->process_state(state_fio, loading);
}

