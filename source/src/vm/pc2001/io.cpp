/*
	CANON X-07 Emulator 'eX-07'

	Origin : J.Brigaud
	Author : Takeda.Toshiya
	Date   : 2007.12.26 -

	[ i/o ]
*/

#include "io.h"
#include "../datarec.h"
#include "../upd16434.h"
#include "../upd7810.h"
#include "../mame/emu/cpu/upd7810/upd7810.h"

void IO::reset()
{
	memset(key_stat, 0, sizeof(key_stat));
	port_a = 0;
	drec_in = false;
	key_strobe = 0xffff;
}

void IO::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case UPD7807_PORTA:
//		emu->out_debug_log("OUT8\tPA, %02x\n",  data);
		d_drec->write_signal(SIG_DATAREC_MIC, data, 0x10);
		port_a = data;
		break;
		
	case UPD7807_PORTB:
//		emu->out_debug_log("OUT8\tPB, %02x\n",  data);
		d_drec->write_signal(SIG_DATAREC_REMOTE, data, 0x80);
		break;
		
	case UPD7807_PORTS:
//		emu->out_debug_log("OUT8\tPS, %02x\n",  data);
		if(port_a & 0x40) {
			// output to printer
		} else if(port_a & 0x04) {
			// lcd command
			d_lcd[port_a & 0x03]->instruction(data);
		} else {
			// lcd data
			d_lcd[port_a & 0x03]->data(data);
		}
		break;
	}
}

uint32_t IO::read_io8(uint32_t addr)
{
	switch(addr) {
	case UPD7807_PORTB:
//		emu->out_debug_log("IN8\tPB\n");
		return (drec_in ? 0x80 : 0) | ((port_a & 0x40) ? 0 : 0x02);
		
	case UPD7807_PORTC:
//		emu->out_debug_log("IN8\tPC\n");
		return get_key();
		
	case UPD7807_PORTS:
//		emu->out_debug_log("IN8\tPS\n");
		return 0;
	}
	return 0xff;
}

void IO::write_io16(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case UPD7807_PORTE:
//		emu->out_debug_log("OUT16\tPE, %04x\n",  data);
		key_strobe = data;
		break;
	}
}

void IO::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_IO_DREC_IN:
		drec_in = ((data & mask) != 0);
		break;
		
	case SIG_IO_RTC_IN:
		rtc_in = ((data & mask) != 0);
		break;
	}
}

void IO::key_down(int code)
{
	key_stat[code] = 8;
}

void IO::key_up(int code)
{
	key_stat[code] = 0;
}

uint8_t IO::get_key()
{
	uint8_t data = 0x3f;
	
	if(!(key_strobe & 0x0001)) {
		if(key_hit(0x11)) data &= ~0x02;	// CTRL
		if(key_hit(0x10)) data &= ~0x04;	// SHIFT
	}
	if(!(key_strobe & 0x0002)) {
		if(key_hit(0x51)) data &= ~0x01;	// Q
		if(key_hit(0x41)) data &= ~0x02;	// A
//		if(key_hit(0x00)) data &= ~0x04;
		if(key_hit(0x60)) data &= ~0x08;	// NUMPAD 0
		if(key_hit(0x31)) data &= ~0x10;	// 1
		if(key_hit(0x5a)) data &= ~0x20;	// Z
	}
	if(!(key_strobe & 0x0004)) {
		if(key_hit(0x57)) data &= ~0x01;	// W
		if(key_hit(0x53)) data &= ~0x02;	// S
		if(key_hit(0x58)) data &= ~0x04;	// X
		if(key_hit(0x61)) data &= ~0x08;	// NUMPAD 1
		if(key_hit(0x32)) data &= ~0x10;	// 2
		if(key_hit(0x70)) data &= ~0x20;	// F1
	}
	if(!(key_strobe & 0x0008)) {
		if(key_hit(0x6c)) data &= ~0x01;	// NUMPAD , (does not exist in the standard keyboard)
		if(key_hit(0x44)) data &= ~0x02;	// D
		if(key_hit(0x43)) data &= ~0x04;	// C
		if(key_hit(0x62)) data &= ~0x08;	// NUMPAD 2
		if(key_hit(0x33)) data &= ~0x10;	// 3
		if(key_hit(0x71)) data &= ~0x20;	// F2
	}
	if(!(key_strobe & 0x0010)) {
		if(key_hit(0x52)) data &= ~0x01;	// R
		if(key_hit(0x46)) data &= ~0x02;	// F
		if(key_hit(0x56)) data &= ~0x04;	// V
		if(key_hit(0x63)) data &= ~0x08;	// NUMPAD 3
		if(key_hit(0x34)) data &= ~0x10;	// 4
		if(key_hit(0x72)) data &= ~0x20;	// F3
	}
	if(!(key_strobe & 0x0020)) {
		if(key_hit(0x54)) data &= ~0x01;	// T
		if(key_hit(0x47)) data &= ~0x02;	// G
		if(key_hit(0x42)) data &= ~0x04;	// B
		if(key_hit(0x64)) data &= ~0x08;	// NUMPAD 4
		if(key_hit(0x35)) data &= ~0x10;	// 5
		if(key_hit(0x73)) data &= ~0x20;	// F4
	}
	if(!(key_strobe & 0x0040)) {
		if(key_hit(0x59)) data &= ~0x01;	// Y
		if(key_hit(0x48)) data &= ~0x02;	// H
		if(key_hit(0x4e)) data &= ~0x04;	// N
		if(key_hit(0x65)) data &= ~0x08;	// NUMPAD 5
		if(key_hit(0x36)) data &= ~0x10;	// 6
		if(key_hit(0xbc)) data &= ~0x20;	// ,
	}
	if(!(key_strobe & 0x0080)) {
		if(key_hit(0x55)) data &= ~0x01;	// U
		if(key_hit(0x4a)) data &= ~0x02;	// J
		if(key_hit(0x4d)) data &= ~0x04;	// M
		if(key_hit(0x66)) data &= ~0x08;	// NUMPAD 6
		if(key_hit(0x37)) data &= ~0x10;	// 7
		if(key_hit(0xbe)) data &= ~0x20;	// .
	}
	if(!(key_strobe & 0x0100)) {
		if(key_hit(0x49)) data &= ~0x01;	// I
		if(key_hit(0x4b)) data &= ~0x02;	// K
		if(key_hit(0x6f)) data &= ~0x04;	// NUMPAD /
		if(key_hit(0x67)) data &= ~0x08;	// NUMPAD 7
		if(key_hit(0x38)) data &= ~0x10;	// 8
		if(key_hit(0xbf)) data &= ~0x20;	// /
	}
	if(!(key_strobe & 0x0200)) {
		if(key_hit(0x4f)) data &= ~0x01;	// O
		if(key_hit(0x4c)) data &= ~0x02;	// L
		if(key_hit(0x6a)) data &= ~0x04;	// NUMPAD *
		if(key_hit(0x68)) data &= ~0x08;	// NUMPAD 8
		if(key_hit(0x39)) data &= ~0x10;	// 9
		if(key_hit(0xbb)) data &= ~0x20;	// ;
	}
	if(!(key_strobe & 0x0400)) {
		if(key_hit(0x50)) data &= ~0x01;	// P
		if(key_hit(0xdc)) data &= ~0x02;	// YEN
		if(key_hit(0x6d)) data &= ~0x04;	// NUMPAD -
		if(key_hit(0x69)) data &= ~0x08;	// NUMPAD 9
		if(key_hit(0x30)) data &= ~0x10;	// 0
		if(key_hit(0xba)) data &= ~0x20;	// :
	}
	if(!(key_strobe & 0x0800)) {
		if(key_hit(0xc0)) data &= ~0x01;	// @
//		if(key_hit(0x00)) data &= ~0x02;
		if(key_hit(0x6b)) data &= ~0x04;	// NUMPAD +
		if(key_hit(0x45)) data &= ~0x08;	// E
		if(key_hit(0xbd)) data &= ~0x10;	// -
		if(key_hit(0xdd)) data &= ~0x20;	// ]
	}
	if(!(key_strobe & 0x1000)) {
		if(key_hit(0xde)) data &= ~0x01;	// ^
		if(key_hit(0x20)) data &= ~0x02;	// SPACE
		if(key_hit(0x6e)) data &= ~0x04;	// NUMPAD .
		if(key_hit(0x26)) data &= ~0x08;	// UP
		if(key_hit(0xdb)) data &= ~0x10;	// [
		if(key_hit(0xe2)) data &= ~0x20;	// _
	}
	if(!(key_strobe & 0x2000)) {
		if(key_hit(0x2e)) data &= ~0x01;	// DEL
		if(key_hit(0x2d)) data &= ~0x02;	// INS
//		if(key_hit(0x00)) data &= ~0x04;
		if(key_hit(0x28)) data &= ~0x08;	// DOWN
		if(key_hit(0x25)) data &= ~0x10;	// LEFT
		if(key_hit(0x27)) data &= ~0x20;	// RIGHT
	}
	if(!(key_strobe & 0x4000)) {
//		if(key_hit(0x00)) data &= ~0x01;
		if(key_hit(0x0d)) data &= ~0x02;	// RETURN
//		if(key_hit(0x00)) data &= ~0x04;
		if(key_hit(0x15)) data &= ~0x08;	// KANA
		if(key_hit(0x24)) data &= ~0x10;	// CLR -> HOME
		if(key_hit(0x74)) data &= ~0x20;	// F5
	}
	return data;
}

bool IO::key_hit(int code)
{
	bool value = (key_stat[code] != 0);
	if(!(code == 0x10 || code == 0x11) && key_stat[code] != 0) {
		key_stat[code]--;
	}
	return value;
}

#define STATE_VERSION	1

void IO::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
}

bool IO::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	return true;
}

