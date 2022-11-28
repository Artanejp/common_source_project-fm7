/*
	SORD M23 Emulator 'Emu23'

	Author : Takeda.Toshiya
	Date   : 2022.05.21-

	[ keyboard ]
*/

#include "keyboard.h"

/*
	S1	-> F8
	S2	-> F9
	LF	-> F12
	NUM 000	-> Home
	NUM =	-> End
	NUM C	-> PgUp
	NUM E	-> PgDn
*/
static const int table[16][8] = {
	{0x01, 0x00, 0x1b, 0x09, 0x11, 0x10, 0x00, 0x00}, //	S1		ESC	TAB	CTRL	SHIFT		
	{0x02, 0x00, 0x31, 0x51, 0x41, 0x5a, 0x00, 0x00}, //	S2		1	Q	A	Z		
	{0x03, 0x00, 0x32, 0x57, 0x53, 0x58, 0x25, 0x28}, //	SMALL		2	W	S	X	LEFT	DOWN
	{0x04, 0x00, 0x33, 0x45, 0x44, 0x43, 0x27, 0x26}, //	KANA		3	E	D	C	RIGHT	UP
	{0x00, 0x00, 0x34, 0x52, 0x46, 0x56, 0x6b, 0x6f}, //			4	R	F	V	NUM +	NUM /
	{0x00, 0x00, 0x35, 0x54, 0x47, 0x42, 0x6d, 0x6a}, //			5	T	G	B	NUM -	NUM *
	{0x00, 0x00, 0x36, 0x59, 0x48, 0x20, 0x67, 0x23}, //			6	Y	H	SPACE	NUM 7	NUM =
	{0x00, 0x00, 0x37, 0x55, 0x4a, 0x4e, 0x68, 0x69}, //			7	U	J	N	NUM 8	NUM 9
	{0x00, 0x70, 0x38, 0x49, 0x4b, 0x4d, 0x64, 0x21}, //		F1	8	I	K	M	NUM 4	NUM C
	{0x00, 0x71, 0x39, 0x4f, 0x4c, 0xbc, 0x65, 0x66}, //		F2	9	O	L	,	NUM 5	NUM 6
	{0x00, 0x72, 0x30, 0x50, 0xbb, 0xbe, 0x61, 0x63}, //		F3	0	P	;	.	NUM 1	NUM 3
	{0x00, 0x73, 0xbd, 0xc0, 0xba, 0xbf, 0x62, 0x22}, //		F4	-	@	:	/	NUM 2	NUM E
	{0x00, 0x74, 0xde, 0xdb, 0xdd, 0xe2, 0x60, 0x6e}, //		F5	^	[	]	_	NUM 0	NUM .
	{0x00, 0x75, 0xdc, 0x00, 0x00, 0x00, 0x24, 0x00}, //		F6	\				NUM 000	
	{0x00, 0x76, 0x08, 0x0d, 0x7b, 0x00, 0x00, 0x00}, //		F7	DEL	RETURN	LF			
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

void KEYBOARD::initialize()
{
	memset(key_locked, 0, sizeof(key_locked));
	memset(led_status, 0, sizeof(led_status));
	memset(buffer, 0, sizeof(buffer));
	buffer_ptr = 0;
}

void KEYBOARD::write_io8(uint32_t addr, uint32_t data)
{

	switch(addr & 0xff) {
	case 0xe0:
	case 0xe1:
	case 0xe2:
	case 0xe3:
		led_status[addr & 3] = ((data & 1) != 0);
		break;
	case 0xef:
		buffer[(buffer_ptr++) & 0x1ff] = data;
		break;
	}
}

uint32_t KEYBOARD::read_io8(uint32_t addr)
{
	uint8_t key_stat[256];
	uint32_t val = 0;
	
	switch(addr & 0xff) {
	case 0xe0:
	case 0xe1:
	case 0xe2:
	case 0xe3:
	case 0xe4:
	case 0xe5:
	case 0xe6:
	case 0xe7:
	case 0xe8:
	case 0xe9:
	case 0xea:
	case 0xeb:
	case 0xec:
	case 0xed:
	case 0xee:
		memcpy(key_stat, emu->get_key_buffer(), 256);
		key_stat[0] = 0;
		key_stat[1] = key_locked[0];
		key_stat[2] = key_locked[1];
		key_stat[3] = key_locked[2];
		key_stat[4] = key_locked[3];
		
		for(int i = 0; i < 8; i++) {
			if(key_stat[table[addr & 0x0f][i]]) {
				val |= 1 << i;
			}
		}
		return val;
	case 0xef:
		return config.keyboard_type ? 1 : 0;
	}
	return 0xff;
}

void KEYBOARD::key_down(int code)
{
	switch(code) {
	case 0x77: // F8 -> S1
		key_locked[0] = !key_locked[0];
		break;
	case 0x78: // F9 -> S2
		key_locked[1] = !key_locked[1];
		break;
	case 0x14: // CapsLock -> SMALL
		key_locked[2] = !key_locked[2];
		break;
	case 0x15: // KANA
		key_locked[3] = !key_locked[3];
		break;
	}
}

#define STATE_VERSION	2

bool KEYBOARD::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(key_locked, sizeof(key_locked), 1);
	state_fio->StateArray(led_status, sizeof(led_status), 1);
	state_fio->StateArray(buffer, sizeof(buffer), 1);
	state_fio->StateValue(buffer_ptr);
	return true;
}

