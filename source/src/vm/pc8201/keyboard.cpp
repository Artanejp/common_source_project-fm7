/*
	NEC PC-8201 Emulator 'ePC-8201'

	Author : Takeda.Toshiya
	Date   : 2009.03.31-

	[ keyboard ]
*/

#include "keyboard.h"

static const int key_map[9][8] = {
	{0x5a, 0x58, 0x43, 0x56, 0x42, 0x4e, 0x4d, 0x4c},	//	Z	X	C	V	B	N	M	L
	{0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4a, 0x4b},	//	A	S	D	F	G	H	J	K
	{0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49},	//	G	W	E	R	T	Y	U	I
#ifdef _PC8201A
	// thanks apaslothy!
	{0x4f, 0x50, 0xbb, 0xdc, 0xbc, 0xbe, 0xbf, 0xdd},	//	O	P	=	\	,	.	/	]
	{0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38},	//	1	2	3	4	5	6	7	8
	{0x39, 0x30, 0xba, 0xde, 0xbd, 0xdb, 0x20, 0x2d},	//	9	0	;	'	-	[	SPACE	INS
	{0x08, 0x26, 0x28, 0x25, 0x27, 0x09, 0x1b, 0x0d},	//	DEL	UP	DOWN	LEFT	RIGHT	TAB	ESC	RET
#else
	{0x4f, 0x50, 0xc0, 0xdc, 0xbc, 0xbe, 0xbf, 0xde},	//	O	P	@	\	,	.	/	^
	{0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38},	//	1	2	3	4	5	6	7	8
	{0x39, 0x30, 0xbb, 0xba, 0xbd, 0x20, 0x2d, 0x08},	//	9	0	;	:	-	SPACE	INS	BS
	{0x26, 0x28, 0x25, 0x27, 0x09, 0x1b, 0x0d, 0x00},	//	UP	DOWN	LEFT	RIGHT	TAB	ESC	RET	
#endif
	{0x70, 0x71, 0x72, 0x73, 0x74, 0x00, 0x00, 0x13},	//	F1	F2	F3	F4	F5			STOP
	{0x10, 0x11, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00}	//	SHIFT	CTRL	GRAPH		(CAPS)			
};

//2e(del) -> 08(bs)+10(shift)

void KEYBOARD::initialize()
{
	key_stat = emu->get_key_buffer();
	column = 0;
	caps = true;
	kana = false;
}

uint32_t KEYBOARD::read_io8(uint32_t addr)
{
	// $E8: keyboard input
	uint8_t val = 0xff;
	for(int i = 0; i < 9; i++) {
		if(!(column & (1 << i))) {
			for(int j = 0; j < 8; j++) {
				if(key_stat[key_map[i][j]]) {
					val &= ~(1 << j);
				}
			}
			if(i == 8) {
				if(caps) {
					val &= ~0x10;
				}
				if(kana) {
					val &= ~8;
				}
			}
		}
	}
	return val;
}

void KEYBOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_KEYBOARD_COLUMN_L) {
		column = (column & 0xff00) | (data & mask);
	} else if(id == SIG_KEYBOARD_COLUMN_H) {
		column = (column & 0xff) | ((data & mask) << 8);
	}
}

void KEYBOARD::key_down(int code)
{
	if(code == 0x14) {
		caps = !caps;
	} else if(code == 0x15) {
		kana = !kana;
	}
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
	state_fio->StateValue(caps);
	state_fio->StateValue(kana);
	return true;
}

