/*
	CASIO PV-1000 Emulator 'ePV-1000'

	Author : Takeda.Toshiya
	Date   : 2006.11.16 -

	[ joystick ]
*/

#include "joystick.h"
#include "../../fileio.h"

void JOYSTICK::initialize()
{
	key = emu->key_buffer();
	joy = emu->joy_buffer();
	
	// register event to interrupt
	register_frame_event(this);
}

void JOYSTICK::reset()
{
	status = 0;
}

void JOYSTICK::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0xfc:
//		status = data;
		break;
	case 0xfd:
		column = data;
		status |= 2;
		break;
	}
//	emu->out_debug_log(_T("OUT\t%2x, %2x\n"), addr & 0xff, data);
}

uint32 JOYSTICK::read_io8(uint32 addr)
{
	uint32 val = 0xff;
	
	switch(addr & 0xff) {
	case 0xfc:
		val = status;
		status &= ~1;
		break;
	case 0xfd:
		val = 0;
		if(column & 1) {
			if(joy[0] & 0x40) val |= 1;	// #1 select
			if(joy[0] & 0x80) val |= 2;	// #1 start
			if(joy[1] & 0x40) val |= 4;	// #2 select
			if(joy[1] & 0x80) val |= 8;	// #2 start
		}
		if(column & 2) {
			if(joy[0] & 0x02) val |= 1;	// #1 down
			if(joy[0] & 0x08) val |= 2;	// #1 right
			if(joy[1] & 0x02) val |= 4;	// #2 down
			if(joy[1] & 0x08) val |= 8;	// #2 right
		}
		if(column & 4) {
			if(joy[0] & 0x04) val |= 1;	// #1 left
			if(joy[0] & 0x01) val |= 2;	// #1 up
			if(joy[1] & 0x04) val |= 4;	// #2 left
			if(joy[1] & 0x01) val |= 8;	// #2 up
		}
		if(column & 8) {
			if(joy[0] & 0x10) val |= 1;	// #1 trig1
			if(joy[0] & 0x20) val |= 2;	// #1 trig2
			if(joy[1] & 0x10) val |= 4;	// #2 trig1
			if(joy[1] & 0x20) val |= 8;	// #2 trig2
		}
//		status &= ~2;
		break;
	}
//	emu->out_debug_log(_T("IN\t%2x, %2x\n"), addr & 0xff, val);
	return val;
}

void JOYSTICK::event_frame()
{
	status |= 1;
}

#define STATE_VERSION	1

void JOYSTICK::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint8(column);
	state_fio->FputUint8(status);
}

bool JOYSTICK::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	column = state_fio->FgetUint8();
	status = state_fio->FgetUint8();
	return true;
}

