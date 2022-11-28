/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

	Author : Takeda.Toshiya
	Date   : 2013.05.01-

	[ keyboard ]
*/

#include "keyboard.h"
#include "sub.h"
#include "../mcs48.h"

#define CAPS	0xfe
#define KANA	0xff

static const uint8_t matrix[15][8] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //	(CMT buttons ???)
	{0x1b, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37}, //	ESC	1	2	3	4	5	6	7
	{0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49}, //	Q	W	E	R	T	Y	U	I
	{0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4a, 0x4b}, //	A	S	D	F	G	H	J	K
	{0x5a, 0x58, 0x43, 0x56, 0x42, 0x4e, 0x4d, 0xbc}, //	Z	X	C	V	B	N	M	,
	{0x38, 0x39, 0x30, 0xbd, 0xde, 0xdc, 0x13, 0x00}, //	8	9	0	-	^	\	BRK	
	{0x00, 0x4f, 0x50, 0xc0, 0xdb, 0x2e, 0x00, 0x00}, //		O	P	@	[	DEL		
	{0x00, 0x4c, 0xbb, 0xba, 0xdd, 0x0d, 0x00, 0x00}, //		L	;	:	]	RET		
	{0x00, 0x09, 0x20, 0xbe, 0xbf, 0xe2, 0x00, 0x00}, //		TAB	SPACE	.	/	_		
	{0x24, 0x67, 0x64, 0x61, 0x60, 0x25, 0x00, 0x00}, //	HOME	N7	N4	N1	N0	LEFT		
	{0x6f, 0x68, 0x65, 0x62, 0x00, 0x27, 0x00, 0x00}, //	N/	N8	N5	N2	N,	RIGHT		
	{0x6a, 0x69, 0x66, 0x63, 0x26, 0x28, 0x00, 0x00}, //	N*	N9	N6	N3	UP	DOWN		
	{0x6d, 0x6b, 0x00, 0x6e, 0x00, 0x6c, 0x00, 0x00}, //	N-	N+	N=	N.		NPRET		
	{0x00, 0x70, 0x71, 0x72, 0x73, 0x74, 0x00, 0x00}, //		F1	F2	F3	F4	F5		
	{0x11, 0x10, KANA, CAPS, 0x12, 0x00, 0x00, 0x00}, //	CTRL	SHIFT	KANA	CAPS	GRAPH			
};

static const uint8_t diode[15] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x1f, //	CTRL	SHIFT	KANA	CAPS	GRAPH
};

void KEYBOARD::initialize()
{
	key_stat = emu->get_key_buffer();
	caps_locked = kana_locked = 0;
	column = 0;
}

/*
	P10-P17	--> KEY COLUMN LO (SELECT=L)
	P20-P26	--> KEY COLUMN HI
	P27	--> INT of SUB CPU
	DB0-7	<-- KEY DATA (PUSH=L)
	T0	<-- L
	T1	<-- H
	INT	<-- H
*/

void KEYBOARD::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case MCS48_PORT_P1:
		column = (column & 0xff00) | (data << 0);
		break;
	case MCS48_PORT_P2:
		column = (column & 0x00ff) | (data << 8);
/*
		{
			static bool prev_signal = true;
			bool cur_signal = ((data & 0x80) != 0);
			if(prev_signal != cur_signal) {
				static uint32_t prev_clk = 0;
				int us = (int)((double)get_passed_clock(prev_clk) * 1000.0 * 1000.0 / 4000000 + 0.5);
				prev_clk = get_current_clock();
				this->out_debug_log(_T("%d\t%d\n"), prev_signal, us);
				prev_signal = cur_signal;
			}
		}
*/
		d_cpu->write_signal(SIG_CPU_IRQ, data, 0x80);
		break;
	}
}

uint32_t KEYBOARD::read_io8(uint32_t addr)
{
	switch(addr) {
	case MCS48_PORT_T0:
#ifdef _X1TURBO_FEATURE
		if(config.keyboard_type == 0) {
			return 1;	// mode A
		} else
#endif
		return 0;		// mode B or GND
	case MCS48_PORT_T1:
		return 1;
	default:
		{
			uint32_t value = 0;
			
			// update key status
			uint8_t key_buf[256];
			memcpy(key_buf, key_stat, sizeof(key_buf));
			
			if(key_buf[VK_INSERT]) {
				key_buf[VK_SHIFT] = key_buf[VK_DELETE] = 1;
			}
			if(key_buf[VK_BACK]) {
				key_buf[VK_DELETE] = 1;
			}
			key_buf[CAPS] = caps_locked;
			key_buf[KANA] = kana_locked;
			
			// get key status of all column
			uint8_t key_map[15];
			memset(key_map, 0, sizeof(key_map));
			
			for(int i = 0; i < 15; i++) {
				for(int j = 0; j < 8; j++) {
					if(key_buf[matrix[i][j]]) {
						key_map[i] |= 1 << j;
					}
				}
			}
			
			// check phantom keys (thanks Mr.Sato)
			for(int i = 0; i < 15; i++) {
				if(!(column & (1 << i))) {
					uint8_t row_hold;
					uint8_t row_reak = key_map[i] & (~diode[i]);
					do {
						row_hold = row_reak;
						for(int c = 0; c < 15; c++) {
							if(c != i){
								uint8_t row_bridge = key_map[c] & (~diode[c]);
								if(row_reak & row_bridge) {
									row_reak |= row_bridge;
								}
								if(row_hold != row_reak) break;
							}
						}
					} while(row_hold != row_reak);
					value |= key_map[i] | row_reak;
				}
			}
			return ~value;
		}
	}
	return 0xff;
}

void KEYBOARD::key_down(int code, bool repeat)
{
	switch(code) {
	case VK_CAPITAL:
		caps_locked ^= 1;
		break;
	case VK_KANA:
		kana_locked ^= 1;
		break;
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
	state_fio->StateValue(caps_locked);
	state_fio->StateValue(kana_locked);
	state_fio->StateValue(column);
	return true;
}

