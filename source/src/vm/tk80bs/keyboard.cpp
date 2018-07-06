/*
	NEC TK-80BS (COMPO BS/80) Emulator 'eTK-80BS'
	NEC TK-80 Emulator 'eTK-80'
	NEC TK-85 Emulator 'eTK-85'

	Author : Takeda.Toshiya
	Date   : 2008.08.26 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8255.h"

#if defined(_TK80BS)
static const uint8_t matrix[256] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x0a,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0x00,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x00,0x00,0x00,0x00,0x00,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x2a,0x2b,0x00,0x2d,0x2e,0x2f,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3a,0x3b,0x2c,0x2d,0x2e,0x2f,
	0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x5b,0x5c,0x5d,0x5e,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const uint8_t matrix_s[256] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x0a,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0x00,
	0x00,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x2a,0x2b,0x00,0x2d,0x2e,0x2f,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2a,0x2b,0x3c,0x3d,0x3e,0x3f,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x5f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const uint8_t matrix_k[256] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x0a,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0x00,
	0xdc,0xc7,0xcc,0xb1,0xb3,0xb4,0xb5,0xd4,0xd5,0xd6,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0xc1,0xba,0xbf,0xbc,0xb2,0xca,0xb7,0xb8,0xc6,0xcf,0xc9,0xd8,0xd3,0xd0,0xd7,
	0xbe,0xc0,0xbd,0xc4,0xb6,0xc5,0xcb,0xc3,0xbb,0xdd,0xc2,0x00,0x00,0x00,0x00,0x00,
//	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x2a,0x2b,0x00,0x2d,0x2e,0x2f,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xb9,0xda,0xc8,0xce,0xd9,0xd2,
	0xde,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xdf,0xb0,0xd1,0xcd,0x00,
	0x00,0x00,0xdb,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const uint8_t matrix_ks[256] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x0a,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0x00,
	0xa6,0x00,0x00,0xa7,0xa9,0xaa,0xab,0xac,0xad,0xae,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0xa8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xaf,0x00,0x00,0x00,0x00,0x00,
//	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x2a,0x2b,0x00,0x2d,0x2e,0x2f,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xa4,0x00,0xa1,0xa5,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xa2,0x00,0xa3,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
#endif

void KEYBOARD::initialize()
{
	key_stat = emu->get_key_buffer();
#if defined(_TK80BS)
	kb_type = 3;
#endif
}

void KEYBOARD::reset()
{
#if defined(_TK80BS)
	prev_type = prev_brk = prev_kana = 0;
	kana_lock = false;
#endif
	column = 0xff;
}

void KEYBOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	// update TK-80 keyboard
	column = data & mask;
	update_tk80();
}

void KEYBOARD::key_down(int code)
{
#if defined(_TK80BS)
	// get special key
	bool hit_type = (key_stat[0x1b] && !prev_type);	// ESC
	bool hit_brk = (key_stat[0x13] && !prev_brk);	// PAUSE/BREAK
	bool hit_kana = (key_stat[0x15] && !prev_kana);	// KANA
	prev_type = key_stat[0x1b];
	prev_brk = key_stat[0x13];
	prev_kana = key_stat[0x15];
	
	// check keyboard focus
	if(hit_type) {
		// 1 ... TK80BS
		// 2 ... TK80
		// 3 ... BOTH
		if(++kb_type == 4) {
			kb_type = 1;
		}
		if(kb_type == 1){
			emu->out_message(_T("BS Full Keyboard"));
		} else if(kb_type == 2){
			emu->out_message(_T("On-Board Buttons"));
		} else if(kb_type == 3){
			emu->out_message(_T("Both On-Board Buttons and BS Full Keyboard"));
		}
	}
	
	// update TK-80BS keyboard
	if(kb_type & 1) {
		// break
		if(hit_brk) {
			d_cpu->set_intr_line(true, true, 0);
		}
		
		// kana lock
		if(hit_kana) {
			kana_lock = !kana_lock;
		}
		
		// send keycode
		if(kana_lock) {
			if(key_stat[0x10]) {
				code = matrix_ks[code & 0xff];
			} else {
				code = matrix_k[code & 0xff];
			}
		} else {
			if(key_stat[0x10]) {
				code = matrix_s[code & 0xff];
			} else {
				code = matrix[code & 0xff];
			}
		}
		if(code) {
			d_pio_b->write_signal(SIG_I8255_PORT_A, code, 0xff);
		}
	}
#endif
	
	// update TK-80 keyboard
	update_tk80();
}

void KEYBOARD::key_up(int code)
{
#if defined(_TK80BS)
	prev_type = key_stat[0x1b];
	prev_brk = key_stat[0x13];
	prev_kana = key_stat[0x15];
#endif
	
	// update TK-80 keyboard
	update_tk80();
}

uint32_t KEYBOARD::get_intr_ack()
{
	// RST 7
	return 0xff;
}

void KEYBOARD::update_tk80()
{
/*	[RET] [RUN] [STO] [LOA] [RES]
	[ C ] [ D ] [ E ] [ F ] [ADR]
	[ 8 ] [ 9 ] [ A ] [ B ] [RD+]
	[ 4 ] [ 5 ] [ 6 ] [ 7 ] [RD-]
	[ 0 ] [ 1 ] [ 2 ] [ 3 ] [WR+]
*/
	uint32_t val = 0xff;
	
	// keyboard
#if defined(_TK80BS)
	if(kb_type & 2) {
#endif
		if(!(column & 0x10)) {
			if(key_stat[0x30] || key_stat[0x60]) val &= ~0x01;	// 0
			if(key_stat[0x31] || key_stat[0x61]) val &= ~0x02;	// 1
			if(key_stat[0x32] || key_stat[0x62]) val &= ~0x04;	// 2
			if(key_stat[0x33] || key_stat[0x63]) val &= ~0x08;	// 3
			if(key_stat[0x34] || key_stat[0x64]) val &= ~0x10;	// 4
			if(key_stat[0x35] || key_stat[0x65]) val &= ~0x20;	// 5
			if(key_stat[0x36] || key_stat[0x66]) val &= ~0x40;	// 6
			if(key_stat[0x37] || key_stat[0x67]) val &= ~0x80;	// 7
		}
		if(!(column & 0x20)) {
			if(key_stat[0x38] || key_stat[0x68]) val &= ~0x01;	// 8
			if(key_stat[0x39] || key_stat[0x69]) val &= ~0x02;	// 9
			if(key_stat[0x41]                  ) val &= ~0x04;	// A
			if(key_stat[0x42]                  ) val &= ~0x08;	// B
			if(key_stat[0x43]                  ) val &= ~0x10;	// C
			if(key_stat[0x44]                  ) val &= ~0x20;	// D
			if(key_stat[0x45]                  ) val &= ~0x40;	// E
			if(key_stat[0x46]                  ) val &= ~0x80;	// F
		}
		if(!(column & 0x40)) {
			if(key_stat[0x71]                  ) val &= ~0x01;	// RUN		F2
			if(key_stat[0x70]                  ) val &= ~0x02;	// RET		F1
			if(key_stat[0x74]                  ) val &= ~0x04;	// ADRS SET	F5
			if(key_stat[0x76] || key_stat[0x22]) val &= ~0x08;	// READ DECR	F7 or PgDn
			if(key_stat[0x75] || key_stat[0x21]) val &= ~0x10;	// READ INCR	F6 or PgUp
			if(key_stat[0x77] || key_stat[0x0d]) val &= ~0x20;	// WRITE INCR	F8 or Enter
			if(key_stat[0x72]                  ) val &= ~0x40;	// STORE DATA	F3
			if(key_stat[0x73]                  ) val &= ~0x80;	// LOAD DATA	F4
		}
#if defined(_TK80BS)
	}
#endif
	
	// graphical buffons
	if(!(column & 0x10)) {
		if(key_stat[0x80]) val &= ~0x01;	// 0
		if(key_stat[0x81]) val &= ~0x02;	// 1
		if(key_stat[0x82]) val &= ~0x04;	// 2
		if(key_stat[0x83]) val &= ~0x08;	// 3
		if(key_stat[0x84]) val &= ~0x10;	// 4
		if(key_stat[0x85]) val &= ~0x20;	// 5
		if(key_stat[0x86]) val &= ~0x40;	// 6
		if(key_stat[0x87]) val &= ~0x80;	// 7
	}
	if(!(column & 0x20)) {
		if(key_stat[0x88]) val &= ~0x01;	// 8
		if(key_stat[0x89]) val &= ~0x02;	// 9
		if(key_stat[0x8a]) val &= ~0x04;	// A
		if(key_stat[0x8b]) val &= ~0x08;	// B
		if(key_stat[0x8c]) val &= ~0x10;	// C
		if(key_stat[0x8d]) val &= ~0x20;	// D
		if(key_stat[0x8e]) val &= ~0x40;	// E
		if(key_stat[0x8f]) val &= ~0x80;	// F
	}
	if(!(column & 0x40)) {
		if(key_stat[0x99]) val &= ~0x01;	// RUN
		if(key_stat[0x98]) val &= ~0x02;	// RET
		if(key_stat[0x9c]) val &= ~0x04;	// ADRS SET
		if(key_stat[0x9e]) val &= ~0x08;	// READ DECR
		if(key_stat[0x9d]) val &= ~0x10;	// READ INCR
		if(key_stat[0x9f]) val &= ~0x20;	// WRITE INCR
		if(key_stat[0x9a]) val &= ~0x40;	// STORE DATA
		if(key_stat[0x9b]) val &= ~0x80;	// LOAD DATA
	}
	d_pio_t->write_signal(SIG_I8255_PORT_A, val, 0xff);
}

#define STATE_VERSION	2

#include "../../statesub.h"

void KEYBOARD::decl_state()
{
	enter_decl_state(STATE_VERSION);

#if defined(_TK80BS)
	DECL_STATE_ENTRY_UINT8(prev_type);
	DECL_STATE_ENTRY_UINT8(prev_brk);
	DECL_STATE_ENTRY_UINT8(prev_kana);
	DECL_STATE_ENTRY_BOOL(kana_lock);
	DECL_STATE_ENTRY_UINT32(kb_type);
#endif
	DECL_STATE_ENTRY_UINT32(column);
	
	leave_decl_state();
}

void KEYBOARD::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//#if defined(_TK80BS)
//	state_fio->FputUint8(prev_type);
//	state_fio->FputUint8(prev_brk);
//	state_fio->FputUint8(prev_kana);
//	state_fio->FputBool(kana_lock);
//	state_fio->FputUint32(kb_type);
//#endif
//	state_fio->FputUint32(column);
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
//#if defined(_TK80BS)
//	prev_type = state_fio->FgetUint8();
//	prev_brk = state_fio->FgetUint8();
//	prev_kana = state_fio->FgetUint8();
//	kana_lock = state_fio->FgetBool();
//	kb_type = state_fio->FgetUint32();
//#endif
//	column = state_fio->FgetUint32();
	return true;
}

