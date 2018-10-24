/*
	SANYO PHC-25 Emulator 'ePHC-25'
	SEIKO MAP-1010 Emulator 'eMAP-1010'

	Author : Takeda.Toshiya
	Date   : 2010.08.03-

	[ keyboard ]
*/

#include "keyboard.h"

namespace PHC25 {

#ifdef _MAP1010
static const uint8_t key_map[0x50] = {
/*
	7800:	Z	X	C	V	B	N	SPACE	M
	7808:	,	.	/	_	TK0	TK-	TK+	ENTER
	7810:	A	S	D	F	G	H	J	K
	7818:	L	;	:	]	TK1	TK2	TK3	TK=
	7820:	Q	W	E	R	T	Y	U	I
	7828:	O	P	@	[	TK4	TK5	TK6	TK*
	7830:		3	4	5	6	7	8	9
	7838:	0	-	^	\	DEL	TK7	TK8	TK9
	7840:	2		1		F1	F2	F3	F4
	7848:		LEFT	RIGHT	UP	DOWN			TK/
*/
	0x5A, 0x58, 0x43, 0x56, 0x42, 0x4E, 0x20, 0x4D,
	0xBC, 0xBE, 0xBF, 0xE2, 0x60, 0x6D, 0x6B, 0x0D,
	0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4A, 0x4B,
	0x4C, 0xBB, 0xBA, 0xDD, 0x61, 0x62, 0x63, 0x00,
	0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49,
	0x4F, 0x50, 0xC0, 0xDB, 0x64, 0x65, 0x66, 0x6A,
	0x00, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
	0x30, 0xBD, 0xDE, 0xDC, 0x2E, 0x67, 0x68, 0x69,
	0x32, 0x00, 0x31, 0x00, 0x70, 0x71, 0x72, 0x73,
	0x00, 0x25, 0x27, 0x26, 0x28, 0x00, 0x00, 0x6F
};
#else
static const uint8_t key_map[10][8] = {
	{0x31, 0x57, 0x53, 0x58, 0x26, 0x2e, 0xba, 0x00},
	{0x1b, 0x51, 0x41, 0x5a, 0x28, 0x0d, 0xbb, 0xbf},
	{0x33, 0x52, 0x46, 0x56, 0x25, 0xde, 0xdb, 0x00},
	{0x32, 0x45, 0x44, 0x43, 0x27, 0xdc, 0xdd, 0x20},
	{0x35, 0x59, 0x48, 0x4e, 0x72, 0x30, 0x50, 0x00},
	{0x34, 0x54, 0x47, 0x42, 0x73, 0xbd, 0xc0, 0x00},
	{0x36, 0x55, 0x4a, 0x4d, 0x71, 0x39, 0x4f, 0x00},
	{0x37, 0x49, 0x4b, 0xbc, 0x70, 0x38, 0x4c, 0xbe},
	{0x00, 0x12, 0x10, 0x11, 0x00, 0xf0, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};
#endif

void KEYBOARD::initialize()
{
	key_stat = emu->get_key_buffer();
	
	// register event to update the key status
	register_frame_event(this);
}

void KEYBOARD::reset()
{
#ifdef _MAP1010
	kana_pressed = 0;
#else
	memset(status, 0, sizeof(status));
#endif
}

uint32_t KEYBOARD::read_io8(uint32_t addr)
{
#ifdef _MAP1010
	// memory mapped i/o
	if(0x7800 <= addr && addr < 0x7850) {
		return key_stat[key_map[addr - 0x7800]] ? 0 : 1;
	} else if(addr == 0x7850) {
		// LSHIFT
		return key_stat[0x10] ? 0 : 1;
	} else if(addr == 0x7851) {
		// RSHIFT
		return 1;
	} else if(addr == 0x7852) {
		// CTRL
		return key_stat[0x11] ? 0 : 1;
	} else if(addr == 0x7853) {
		// GRAPH
		return key_stat[0x12] ? 0 : 1;
	} else if(addr == 0x785c) {
		// KANA
		if(kana_pressed) {
			kana_pressed--;
			return 0;
		}
		return 1;
	}
	return 0xff;
#else
	return ~status[addr & 0x0f];
#endif
}

void KEYBOARD::event_frame()
{
#ifdef _MAP1010
	if(!kana_pressed && key_stat[0x15]) {
		kana_pressed = 4;
	}
#else
	memset(status, 0, sizeof(status));
	
	for(int i = 0; i < 10; i++) {
		uint8_t val = 0;
		for(int j = 0; j < 8; j++) {
			val |= key_stat[key_map[i][j]] ? (1 << j) : 0;
		}
		status[i] = val;
	}
#endif
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
#ifdef _MAP1010
	state_fio->StateValue(kana_pressed);
#else
	state_fio->StateArray(status, sizeof(status), 1);
#endif
	return true;
}

}
