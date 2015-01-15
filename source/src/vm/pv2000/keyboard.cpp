/*
	CASIO PV-2000 Emulator 'EmuGaki'

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "../../fileio.h"

static const int key_map[16][8] = {
	{0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38},	// 1	2	3	4	5	6	7	8
	{0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49},	// Q	W	E	R	T	Y	U	I
	{0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4a, 0x4b},	// A	S	D	F	G	H	J	K
	{0x1d, 0x5a, 0x58, 0x43, 0x56, 0x42, 0x4e, 0x20},	// KANA	Z	X	C	V	B	N	SPACE
	{0x24, 0x00, 0x00, 0xdc, 0x30, 0xde, 0xbd, 0x39},	// HOME	?	?	\	0	^	-	9
	{0x00, 0x00, 0x00, 0x00, 0x50, 0xdb, 0xc0, 0x4f},	// UR	DR	DL	UL	P	[	@	O
	{0x00, 0x00, 0x27, 0x28, 0xbb, 0xdd, 0xba, 0x4c},	// R2	D2	R1	D1	;	]	:	L
	{0x00, 0x00, 0x26, 0x25, 0xbc, 0xbf, 0xbe, 0x4d},	// U2	L2	U1	L1	,	/	.	M
	{0x73, 0x72, 0x71, 0x70, 0xe2, 0x2e, 0x74, 0x0d},	// A3	A2	A1	A0	_	DEL	MODE	RET
	{0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},	// STOP	?	?	?	?	?	?	?
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

void KEYBOARD::initialize()
{
	joy_stat = emu->joy_buffer();
}

void KEYBOARD::reset()
{
	memset(key_stat, 0, sizeof(key_stat));
	key_no = 0;
	intr_enb = false;
}

void KEYBOARD::write_io8(uint32 addr, uint32 data)
{
	intr_enb = (data == 0xf);
	key_no = data & 0xf;
}

uint32 KEYBOARD::read_io8(uint32 addr)
{
	uint32 val = 0;
	
	switch(addr & 0xff) {
	case 0x10:
		if(key_stat[key_map[key_no][7]]) val |= 1;
		if(key_stat[key_map[key_no][6]]) val |= 2;
		if(key_stat[key_map[key_no][5]]) val |= 4;
		if(key_stat[key_map[key_no][4]]) val |= 8;
		return val;
	case 0x20:
		if(key_stat[key_map[key_no][3]]) val |= 1;
		if(key_stat[key_map[key_no][2]]) val |= 2;
		if(key_stat[key_map[key_no][1]]) val |= 4;
		if(key_stat[key_map[key_no][0]]) val |= 8;
		if(key_no == 6) {
			if(joy_stat[0] & 0x02) val |= 1;
			if(joy_stat[0] & 0x08) val |= 2;
			if(joy_stat[1] & 0x02) val |= 4;
			if(joy_stat[1] & 0x08) val |= 8;
		} else if(key_no == 7) {
			if(joy_stat[0] & 0x04) val |= 1;
			if(joy_stat[0] & 0x01) val |= 2;
			if(joy_stat[1] & 0x04) val |= 4;
			if(joy_stat[1] & 0x01) val |= 8;
		} else if(key_no == 8) {
			if(joy_stat[0] & 0x10) val |= 1;
			if(joy_stat[0] & 0x20) val |= 2;
			if(joy_stat[1] & 0x10) val |= 4;
			if(joy_stat[1] & 0x20) val |= 8;
		}
		return val;
	case 0x40:
		if(key_stat[0x11]) val |= 1;	// COLOR (CTRL)
		if(key_stat[0x09]) val |= 2;	// FUNC (TAB)
		if(key_stat[0x10]) val |= 4;	// SHIFT
		return val;
	}
	return 0xff;
}

void KEYBOARD::key_down(int code)
{
	if(!(code == 0x09 || code == 0x10 || code == 0x11)) {
		if(intr_enb) {
			d_cpu->set_intr_line(true, true, 0);
			intr_enb = false;
		}
	}
	if((0x30 <= code && code <= 0x5a) || (0xba <= code && code <= 0xe2)) {
		memset(key_stat + 0x30, 0, 0x5a - 0x30 + 1);
		memset(key_stat + 0xba, 0, 0xe2 - 0xba + 1);
	}
	key_stat[code] = 1;
}

void KEYBOARD::key_up(int code)
{
	key_stat[code] = 0;
}

#define STATE_VERSION	1

void KEYBOARD::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(key_stat, sizeof(key_stat), 1);
	state_fio->FputInt32(key_no);
	state_fio->FputBool(intr_enb);
}

bool KEYBOARD::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(key_stat, sizeof(key_stat), 1);
	key_no = state_fio->FgetInt32();
	intr_enb = state_fio->FgetBool();
	return true;
}

