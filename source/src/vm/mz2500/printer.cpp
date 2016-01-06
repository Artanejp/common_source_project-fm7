/*
	SHARP MZ-2200 Emulator 'EmuZ-2200'
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2015.12.28-

	[ printer ]
*/

#include "printer.h"

void PRINTER::initialize()
{
	prev_reset = false;
}

void PRINTER::reset()
{
	prev_reset = false;
}

void PRINTER::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0xfe:
		{
			bool new_reset = ((data & 0x40) != 0);
			bool falling = (prev_reset && !new_reset);
			prev_reset =new_reset;
			
			if(falling) {
				d_prn->reset();
			}
		}
		d_prn->write_signal(SIG_PRINTER_STROBE, data, 0x80);
		break;
	case 0xff:
		d_prn->write_signal(SIG_PRINTER_DATA, data, 0xff);
		break;
	}
}

uint32 PRINTER::read_io8(uint32 addr)
{
	switch(addr & 0xff) {
	case 0xfe:
		return 0xf2 | (d_prn->read_signal(SIG_PRINTER_BUSY) ? 1 : 0);
	}
	return 0xff;
}

#define STATE_VERSION	1

void PRINTER::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputBool(prev_reset);
}

bool PRINTER::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	prev_reset = state_fio->FgetBool();
	return true;
}

