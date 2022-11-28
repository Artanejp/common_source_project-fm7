/*
	TOSHIBA EX-80 Emulator 'eEX-80'

	Author : Takeda.Toshiya
	Date   : 2015.12.10-

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8255.h"

void KEYBOARD::initialize()
{
	column = 0xff;
	register_frame_event(this);
}

void KEYBOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	column = data & mask;
	event_frame();
}

void KEYBOARD::event_frame()
{
	const uint8_t* key_stat = emu->get_key_buffer();
	uint32_t val = 0xff;
	
	if(!(column & 0x10)) {
		if(key_stat[0x80] || key_stat[0x30] || key_stat[0x60]) val &= ~0x01;	// 0
		if(key_stat[0x81] || key_stat[0x31] || key_stat[0x61]) val &= ~0x02;	// 1
		if(key_stat[0x82] || key_stat[0x32] || key_stat[0x62]) val &= ~0x04;	// 2
		if(key_stat[0x83] || key_stat[0x33] || key_stat[0x63]) val &= ~0x08;	// 3
		if(key_stat[0x84] || key_stat[0x34] || key_stat[0x64]) val &= ~0x10;	// 4
		if(key_stat[0x85] || key_stat[0x35] || key_stat[0x65]) val &= ~0x20;	// 5
		if(key_stat[0x86] || key_stat[0x36] || key_stat[0x66]) val &= ~0x40;	// 6
		if(key_stat[0x87] || key_stat[0x37] || key_stat[0x67]) val &= ~0x80;	// 7
	}
	if(!(column & 0x20)) {
		if(key_stat[0x88] || key_stat[0x38] || key_stat[0x68]) val &= ~0x01;	// 8
		if(key_stat[0x89] || key_stat[0x39] || key_stat[0x69]) val &= ~0x02;	// 9
		if(key_stat[0x8a] || key_stat[0x41]                  ) val &= ~0x04;	// A
		if(key_stat[0x8b] || key_stat[0x42]                  ) val &= ~0x08;	// B
		if(key_stat[0x8c] || key_stat[0x43]                  ) val &= ~0x10;	// C
		if(key_stat[0x8d] || key_stat[0x44]                  ) val &= ~0x20;	// D
		if(key_stat[0x8e] || key_stat[0x45]                  ) val &= ~0x40;	// E
		if(key_stat[0x8f] || key_stat[0x46]                  ) val &= ~0x80;	// F
	}
	if(!(column & 0x40)) {
		if(key_stat[0x98] || key_stat[0x70]                  ) val &= ~0x02;	// RET ... F1
		if(key_stat[0x99] || key_stat[0x71]                  ) val &= ~0x01;	// RUN ... F2
		if(key_stat[0x9a] || key_stat[0x72]                  ) val &= ~0x40;	// SDA ... F3
		if(key_stat[0x9b] || key_stat[0x73]                  ) val &= ~0x80;	// LDA ... F4
		if(key_stat[0x9c] || key_stat[0x74]                  ) val &= ~0x04;	// ADR ... F5
		if(key_stat[0x9d] || key_stat[0x75] || key_stat[0x21]) val &= ~0x10;	// RIC ... F6 or PgUp
		if(key_stat[0x9e] || key_stat[0x76] || key_stat[0x22]) val &= ~0x08;	// RDC ... F7 or PgDn
		if(key_stat[0x9f] || key_stat[0x77] || key_stat[0x0d]) val &= ~0x20;	// WIC ... F8 or Enter
	}
	d_pio->write_signal(SIG_I8255_PORT_A, val, 0xff);
}

#define STATE_VERSION	1

bool KEYBOARD::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(column);
	return true;
}

