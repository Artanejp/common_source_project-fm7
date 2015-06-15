/*
 * FM-7 Main I/O [joystick.cpp]
 *  - Joystick
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Jun 16, 2015 : Initial, split from sound.cpp.
 *
 */
#include "fm7_mainio.h"
#include "./joystick.h"
#include "../../config.h"

JOYSTICK::JOYSTICK(VM *parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
	p_vm = parent_vm;
	p_emu = parent_emu;
	rawdata = NULL;
	opn = NULL;
}

JOYSTICK::~JOYSTICK()
{
}

void JOYSTICK::initialize()
{
	rawdata = p_emu->joy_buffer();
	register_frame_event(this);
	emulate_mouse[0] = emulate_mouse[1] = false;
	joydata[0] = joydata[1] = 0xff;
}

void JOYSTICK::reset()
{
	joydata[0] = joydata[1] = 0xff;
}

void JOYSTICK::event_frame()
{
	int ch;
	uint32 val = 0xff;
	if(rawdata == NULL) return;
	for(ch = 0; ch < 2; ch++) {
		if(!emulate_mouse[ch]) { // Joystick
			val = ~rawdata[ch];
			if((val & 0x40) == 0) val &= ~0x10;  // Button A'
			if((val & 0x80) == 0) val &= ~0x20;  // Button B'
			val |= 0xc0;
			joydata[ch] = val;
		} else { // MOUSE
		}
	}
}

uint32 JOYSTICK::read_data8(uint32 addr)
{
	uint32 val = 0xff;
	if(opn == NULL) return 0xff;
	
	switch(addr) {
		case 0:
			opn->write_io8(0, 0x0f);
			switch(opn->read_io8(1) & 0xf0) {
				case 0x20:
					val = joydata[0];
					break;
				case 0x50:
					val = joydata[1];
					break;
			}
	}
	return val;
}

void JOYSTICK::write_data8(uint32 addr, uint32 data)
{

}

void JOYSTICK::write_signal(int id, uint32 data, uint32 mask)
{
	uint32 val = data & mask;
	bool val_b = (val != 0);
	switch(id) {
		case FM7_JOYSTICK_EMULATE_MOUSE_0:
			emulate_mouse[0] = val_b;
			break;
		case FM7_JOYSTICK_EMULATE_MOUSE_1:
			emulate_mouse[1] = val_b;
			break;
	}
}

#define STATE_VERSION 1
void JOYSTICK::save_state(FILEIO *state_fio)
{
	int ch;
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	// Version 1
	for(ch = 0; ch < 2; ch++) {
		state_fio->FputBool(emulate_mouse[ch]);
		state_fio->FputUint32(joydata[ch]);
	}
	// After Version2.
}

bool JOYSTICK::load_state(FILEIO *state_fio)
{
	uint32 version = state_fio->FgetUint32();
	uint32 devid = state_fio->FgetUint32();
	bool stat = false;
	int ch;
	if(version >= 1) {
		for(ch = 0; ch < 2; ch++) {
			state_fio->FputBool(emulate_mouse[ch]);
			state_fio->FputUint32(joydata[ch]);
		}
		if(version == 1) stat = true;
	}
	// After version 2.
	return stat;
}
		
	
