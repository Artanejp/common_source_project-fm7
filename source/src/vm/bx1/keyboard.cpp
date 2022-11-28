/*
	CANON BX-1 Emulator 'eBX-1'

	Author : Takeda.Toshiya
	Date   : 2020.08.22-

	[ keyboard ]
*/

#include "keyboard.h"
#include "../../fifo.h"

static const int table[256] = {
	  -1,	// 00 
	  -1,	// 01 
	  -1,	// 02 
	  -1,	// 03 
	  -1,	// 04 
	  -1,	// 05 
	  -1,	// 06 
	  -1,	// 07 
	  -1,	// 08 Back
	0x13,	// 09 Tab	INST
	  -1,	// 0A 
	  -1,	// 0B 
	  -1,	// 0C 
	0x42,	// 0D Enter	CR/LF
	  -1,	// 0E 
	  -1,	// 0F 
	  -1,	// 10 Shift
	  -1,	// 11 Ctrl
	  -1,	// 12 Alt
	  -1,	// 13 Pause
	0x12,	// 14 Caps	CTRL
	  -1,	// 15 Kana
	  -1,	// 16 
	  -1,	// 17 
	  -1,	// 18 
	  -1,	// 19 Kanji
	  -1,	// 1A 
	0x4F,	// 1B Escape	CLEAR
	  -1,	// 1C Convert
	  -1,	// 1D NonConv
	  -1,	// 1E 
	  -1,	// 1F 
	0x41,	// 20 Space	SPACE
	0x49,	// 21 PgUp	SQR
	0x4A,	// 22 PgDwn	)
	0x46,	// 23 End	(
	0x45,	// 24 Home	alpha^x
	0x63,	// 25 Left
	0x62,	// 26 Up
	0x65,	// 27 Right
	0x64,	// 28 Down
	  -1,	// 29 
	  -1,	// 2A 
	  -1,	// 2B 
	  -1,	// 2C 
	0x60,	// 2D Ins	INSERT
	0x61,	// 2E Del	DELETE
	  -1,	// 2F 
	0x3B,	// 30 0
	0x17,	// 31 1
	0x1B,	// 32 2
	0x1F,	// 33 3
	0x23,	// 34 4
	0x27,	// 35 5
	0x2B,	// 36 6
	0x2F,	// 37 7
	0x33,	// 38 8
	0x37,	// 39 9
	  -1,	// 3A 
	  -1,	// 3B 
	  -1,	// 3C 
	  -1,	// 3D 
	  -1,	// 3E 
	  -1,	// 3F 
	  -1,	// 40 
	0x15,	// 41 A
	0x24,	// 42 B
	0x1C,	// 43 C
	0x1D,	// 44 D
	0x1E,	// 45 E
	0x21,	// 46 F
	0x25,	// 47 G
	0x29,	// 48 H
	0x32,	// 49 I
	0x2D,	// 4A J
	0x31,	// 4B K
	0x35,	// 4C L
	0x2C,	// 4D M
	0x28,	// 4E N
	0x36,	// 4F O
	0x3A,	// 50 P
	0x16,	// 51 Q
	0x22,	// 52 R
	0x19,	// 53 S
	0x26,	// 54 T
	0x2E,	// 55 U
	0x20,	// 56 V
	0x1A,	// 57 W
	0x18,	// 58 X
	0x2A,	// 59 Y
	0x14,	// 5A Z
	  -1,	// 5B 
	  -1,	// 5C 
	  -1,	// 5D 
	  -1,	// 5E 
	  -1,	// 5F 
	0x0D,	// 60 NUM 0
	0x01,	// 61 NUM 1
	0x02,	// 62 NUM 2
	0x03,	// 63 NUM 3
	0x04,	// 64 NUM 4
	0x05,	// 65 NUM 5
	0x06,	// 66 NUM 6
	0x07,	// 67 NUM 7
	0x08,	// 68 NUM 8
	0x09,	// 69 NUM 9
	0x47,	// 6A NUM *
	0x48,	// 6B NUM +
	0x4D,	// 6C NUM Ent	=
	0x4C,	// 6D NUM -
	0x0E,	// 6E NUM .
	0x4B,	// 6F NUM /
	0x51,	// 70 F1	RUN
	0x53,	// 71 F2	RENAME
	0x55,	// 72 F3	PROTECT
	0x57,	// 73 F4	PROG-LIST
	0x59,	// 74 F5	STOP
	0x5B,	// 75 F6	CONDENSE
	0x5D,	// 76 F7	LINE-NO.
	  -1,	// 77 F8
	0x0A,	// 78 F9	EXP
	0x0B,	// 79 F10	SIGN CHG
	0x0C,	// 7A F11	CE
	0x0F,	// 7B F12	START ?
	0x50,	// 7C F13	PROGRAM
	0x52,	// 7D F14	NEW
	0x54,	// 7E F15	SECURE
	0x56,	// 7F F16	OPERATE
	0x58,	// 80 F17	TRACE
	0x5A,	// 81 F18	DISK-LIST
	0x5C,	// 82 F19	RECALL
	  -1,	// 83 F20
	  -1,	// 84 F21
	  -1,	// 85 F22
	  -1,	// 86 F23
	  -1,	// 87 F24
	  -1,	// 88 
	  -1,	// 89 
	  -1,	// 8A 
	  -1,	// 8B 
	  -1,	// 8C 
	  -1,	// 8D 
	  -1,	// 8E 
	  -1,	// 8F 
	  -1,	// 90 
	0x4E,	// 91 ScrLk	PAPER FEED
	  -1,	// 92 
	  -1,	// 93 
	  -1,	// 94 
	  -1,	// 95 
	  -1,	// 96 
	  -1,	// 97 
	  -1,	// 98 
	  -1,	// 99 
	  -1,	// 9A 
	  -1,	// 9B 
	  -1,	// 9C 
	  -1,	// 9D 
	  -1,	// 9E 
	  -1,	// 9F 
	0x11,	// A0 L Shift	SML
	  -1,	// A1 R Shift	(Switch F1-F8)
	0x10,	// A2 L Ctrl	CAP
	0x3C,	// A3 R Ctrl	UC
	  -1,	// A4 L Alt
	  -1,	// A5 R Alt
	  -1,	// A6 
	  -1,	// A7 
	  -1,	// A8 
	  -1,	// A9 
	  -1,	// AA 
	  -1,	// AB 
	  -1,	// AC 
	  -1,	// AD 
	  -1,	// AE 
	  -1,	// AF 
	  -1,	// B0 
	  -1,	// B1 
	  -1,	// B2 
	  -1,	// B3 
	  -1,	// B4 
	  -1,	// B5 
	  -1,	// B6 
	  -1,	// B7 
	  -1,	// B8 
	  -1,	// B9 
	0x3D,	// BA :
	0x39,	// BB ;
	0x30,	// BC ,
	0x3F,	// BD -
	0x34,	// BE .
	0x38,	// BF /
	0x3E,	// C0 @
	  -1,	// C1 
	  -1,	// C2 
	  -1,	// C3 
	  -1,	// C4 
	  -1,	// C5 
	  -1,	// C6 
	  -1,	// C7 
	  -1,	// C8 
	  -1,	// C9 
	  -1,	// CA 
	  -1,	// CB 
	  -1,	// CC 
	  -1,	// CD 
	  -1,	// CE 
	  -1,	// CF 
	  -1,	// D0 
	  -1,	// D1 
	  -1,	// D2 
	  -1,	// D3 
	  -1,	// D4 
	  -1,	// D5 
	  -1,	// D6 
	  -1,	// D7 
	  -1,	// D8 
	  -1,	// D9 
	  -1,	// DA 
	0x46,	// DB [
	0x44,	// DC Yen
	0x4A,	// DD ]
	0x43,	// DE ^
	  -1,	// DF 
	  -1,	// E0 
	  -1,	// E1 
	  -1,	// E2 _
	  -1,	// E3 
	  -1,	// E4 
	  -1,	// E5 
	  -1,	// E6 
	  -1,	// E7 
	  -1,	// E8 
	  -1,	// E9 
	  -1,	// EA 
	  -1,	// EB 
	  -1,	// EC 
	  -1,	// ED 
	  -1,	// EE 
	  -1,	// EF 
	  -1,	// F0 
	  -1,	// F1 
	  -1,	// F2 
	  -1,	// F3 
	  -1,	// F4 
	  -1,	// F5 
	  -1,	// F6 
	  -1,	// F7 
	  -1,	// F8 
	  -1,	// F9 
	  -1,	// FA 
	  -1,	// FB 
	  -1,	// FC 
	  -1,	// FD 
	  -1,	// FE 
	  -1,	// FF 
};

void KEYBOARD::initialize()
{
	fifo_down = new FIFO(8);
}

void KEYBOARD::release()
{
	fifo_down->release();
	delete fifo_down;
}

void KEYBOARD::reset()
{
	fifo_down->clear();
}

uint32_t KEYBOARD::read_io8(uint32_t addr)
{
	uint32_t value = 0xff;
	
	switch(addr & 0xffff) {
	case 0xe121:
		if(!fifo_down->empty()) {
			value = fifo_down->read_not_remove(0) | 0x80;
		} else {
			value = 0;
		}
		break;
	case 0xe122:
		// bit7: JPN/ENG Jumper?
		// bit5: PROG.SELECT
		fifo_down->read();
		value  = (config.dipswitch & 2) ? 0 : 0x20;
		value |= (config.dipswitch & 4) ? 0 : 0x80;
		break;
	}
	return value;
}

void KEYBOARD::key_down(int code)
{
	if(code >= 0x70 && code <= 0x7b && emu->get_key_buffer()[0xa1]) {
		// Right Shift + F1-F12 -> F13-F24
		code += 12;
	}
	if(table[code & 0xff] != -1) {
		fifo_down->write(table[code & 0xff]);
	}
}

void KEYBOARD::key_up(int code)
{
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
	if(!fifo_down->process_state((void *)state_fio, loading)) {
		return false;
	}
	return true;
}

