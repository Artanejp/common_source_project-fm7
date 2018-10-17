/*
	YAMAHA YIS Emulator 'eYIS'

	Author : Takeda.Toshiya
	Date   : 2017.04.20-

	[ keyboard ]
*/

#include "keyboard.h"

void KEYBOARD::initialize()
{
	key_stat = emu->get_key_buffer();
	column = 0;
	caps_locked = true;
	kana_locked = false;
}

void KEYBOARD::reset()
{
}

void KEYBOARD::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0xf030:
		column = data;
		break;
	}
}

uint32_t KEYBOARD::read_io8(uint32_t addr)
{
	uint32_t data = 0;
	
	switch(addr) {
	case 0xf030:
		if(column & 0x01) {
			if(kana_locked   ) data |= 0x80; // KANA-LOCK
			if(key_stat[0x11]) data |= 0x40; // CTRL
			if(key_stat[0x1b]) data |= 0x20; // ESC
			if(key_stat[0x12]) data |= 0x10; // GRAPH
			if(key_stat[0x09]) data |= 0x08; // TAB
			if(key_stat[0xa0]) data |= 0x04; // SHIFT(LEFT)
			if(key_stat[0xa1]) data |= 0x02; // SHIFT(RIGHT)
			if(caps_locked   ) data |= 0x01; // CAPS-LOCK
		}
		if(column & 0x02) {
			if(key_stat[0x51]) data |= 0x80; // Q
			if(key_stat[0x41]) data |= 0x40; // A
			if(key_stat[0x5a]) data |= 0x20; // Z
			if(key_stat[0x20]) data |= 0x10; // SPACE
			if(key_stat[0xbf]) data |= 0x08; // /
			if(key_stat[0xbb]) data |= 0x04; // ;
			if(key_stat[0x50]) data |= 0x02; // P
		}
		if(column & 0x04) {
			if(key_stat[0x58]) data |= 0x80; // X
			if(key_stat[0x43]) data |= 0x40; // C
			if(key_stat[0x56]) data |= 0x20; // V
			if(key_stat[0x42]) data |= 0x10; // B
			if(key_stat[0x4e]) data |= 0x08; // N
			if(key_stat[0x4d]) data |= 0x04; // M
			if(key_stat[0xbc]) data |= 0x02; // ,
		}
		if(column & 0x08) {
			if(key_stat[0x53]) data |= 0x80; // S
			if(key_stat[0x44]) data |= 0x40; // D
			if(key_stat[0x46]) data |= 0x20; // F
			if(key_stat[0x47]) data |= 0x10; // G
			if(key_stat[0x48]) data |= 0x08; // H
			if(key_stat[0x4a]) data |= 0x04; // J
			if(key_stat[0x4b]) data |= 0x02; // K
		}
		if(column & 0x10) {
			if(key_stat[0x57]) data |= 0x80; // W
			if(key_stat[0x45]) data |= 0x40; // E
			if(key_stat[0x52]) data |= 0x20; // R
			if(key_stat[0x54]) data |= 0x10; // T
			if(key_stat[0x59]) data |= 0x08; // Y
			if(key_stat[0x55]) data |= 0x04; // U
			if(key_stat[0x49]) data |= 0x02; // I
		}
		if(column & 0x20) {
			if(key_stat[0xbe]) data |= 0x80; // .
			if(key_stat[0x4c]) data |= 0x40; // L
			if(key_stat[0x4f]) data |= 0x20; // O
			if(key_stat[0x08]) data |= 0x10; // LF -> BACK SPACE
			if(key_stat[0x0d]) data |= 0x08; // RETURN
		}
		if(column & 0x40) {
			if(key_stat[0x38]) data |= 0x80; // 8
			if(key_stat[0x39]) data |= 0x40; // 9
			if(key_stat[0x30]) data |= 0x20; // 0
			if(key_stat[0xba]) data |= 0x10; // :
			if(key_stat[0xbd]) data |= 0x08; // -
			if(key_stat[0x2e]) data |= 0x04; // RUB OUT -> DEL
		}
		if(column & 0x80) {
			if(key_stat[0x31]) data |= 0x80; // 1
			if(key_stat[0x32]) data |= 0x40; // 2
			if(key_stat[0x33]) data |= 0x20; // 3
			if(key_stat[0x34]) data |= 0x10; // 4
			if(key_stat[0x35]) data |= 0x08; // 5
			if(key_stat[0x36]) data |= 0x04; // 6
			if(key_stat[0x37]) data |= 0x02; // 7
		}
		return data;
	case 0xf031:
		if(column & 0x04) {
			if(key_stat[0x25]) data |= 0x80; // LEFT
			if(key_stat[0x26]) data |= 0x40; // UP
			if(key_stat[0x28]) data |= 0x20; // DOWN
			if(key_stat[0x27]) data |= 0x10; // RIGHT
		}
		if(column & 0x08) {
			if(key_stat[0x75]) data |= 0x80; // F6
			if(key_stat[0x74]) data |= 0x40; // F5
			if(key_stat[0x73]) data |= 0x20; // F4
			if(key_stat[0x72]) data |= 0x10; // F3
			if(key_stat[0x71]) data |= 0x08; // F2
			if(key_stat[0x70]) data |= 0x04; // F1
		}
		if(column & 0x10) {
			if(key_stat[0x7b]) data |= 0x80; // F12
			if(key_stat[0x7a]) data |= 0x40; // F11
			if(key_stat[0x79]) data |= 0x20; // F10
			if(key_stat[0x78]) data |= 0x10; // F9
			if(key_stat[0x77]) data |= 0x08; // F8
			if(key_stat[0x76]) data |= 0x04; // F7
		}
		if(column & 0x20) {
			if(key_stat[0x6e]) data |= 0x80; // NUMPAD .
			if(key_stat[0x6c]) data |= 0x08; // NUMPAD ENTER -> VK_SEPARATOR
		}
		if(column & 0x40) {
			if(key_stat[0x68]) data |= 0x80; // NUMPAD 8
			if(key_stat[0x69]) data |= 0x40; // NUMPAD 9
			if(key_stat[0x60]) data |= 0x20; // NUMPAD 0
		}
		if(column & 0x80) {
			if(key_stat[0x61]) data |= 0x80; // NUMPAD 1
			if(key_stat[0x62]) data |= 0x40; // NUMPAD 2
			if(key_stat[0x63]) data |= 0x20; // NUMPAD 3
			if(key_stat[0x64]) data |= 0x10; // NUMPAD 4
			if(key_stat[0x65]) data |= 0x08; // NUMPAD 5
			if(key_stat[0x66]) data |= 0x04; // NUMPAD 6
			if(key_stat[0x67]) data |= 0x02; // NUMPAD 7
		}
		return data;
	}
	return 0xff;
}

void KEYBOARD::key_down(int code)
{
	switch(code) {
	case VK_CAPITAL:
		caps_locked = !caps_locked;
		break;
	case VK_KANA:
		kana_locked = !kana_locked;
		break;
	case VK_HOME: // CLEAR -> HOME
		d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
		break;
	}
}

#define STATE_VERSION	1

#include "../../statesub.h"

void KEYBOARD::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
	DECL_STATE_ENTRY_UINT8(column);
	DECL_STATE_ENTRY_BOOL(caps_locked);
	DECL_STATE_ENTRY_BOOL(kana_locked);

	leave_decl_state();
}

void KEYBOARD::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->FputUint8(column);
//	state_fio->FputBool(caps_locked);
//	state_fio->FputBool(kana_locked);
}

bool KEYBOARD::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) {
		return false;
	}
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//	column = state_fio->FgetUint8();
//	caps_locked = state_fio->FgetBool();
//	kana_locked = state_fio->FgetBool();
	return true;
}

bool KEYBOARD::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateUint8(column);
	state_fio->StateBool(caps_locked);
	state_fio->StateBool(kana_locked);
	return true;
}
