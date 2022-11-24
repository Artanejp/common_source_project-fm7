/*
	CASIO FX-9000P Emulator 'eFX-9000P'

	Author : Takeda.Toshiya
	Date   : 2022.03.25-

	[ i/o ]
*/

#include "io.h"
#include "../hd46505.h"
#include "../../fifo.h"

#define EVENT_1SEC	0

/*
	0	1	2	3	4	5	6	7	8	9	A	B	C	D	E	F
00	"	$	(	=	>	UP	ANS	CLS	#	%	)	<	LEFT	DOWN	STAT	RIGHT
10	Q	E	T	U	O	7	9	;	W	R	Y	I	P	8	/	:
20	A	D	G	J	L	4	6	C/S	S	F	H	K	ENTER	5	*	COMP
30	X	V	N	,	SPACE	1	-	+	Z	C	B	M	.	2	3	0
*/

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
	  -1,	// 09 Tab
	  -1,	// 0A 
	  -1,	// 0B 
	  -1,	// 0C 
	0x2c,	// 0D Enter
	  -1,	// 0E 
	  -1,	// 0F 
	  -1,	// 10 Shift
	  -1,	// 11 Ctrl
	  -1,	// 12 Alt
	  -1,	// 13 Pause
	  -1,	// 14 Caps
	  -1,	// 15 Kana
	  -1,	// 16 
	  -1,	// 17 
	  -1,	// 18 
	  -1,	// 19 Kanji
	  -1,	// 1A 
	  -1,	// 1B Escape
	  -1,	// 1C Convert
	  -1,	// 1D NonConv
	  -1,	// 1E 
	  -1,	// 1F 
	0x34,	// 20 Space
	0x06,	// 21 PgUp	ANS
	0x0e,	// 22 PgDwn	STAT
	0x27,	// 23 End	C/S
	0x07,	// 24 Home	CLS
	0x0c,	// 25 Left
	0x05,	// 26 Up
	0x0f,	// 27 Right
	0x0d,	// 28 Down
	  -1,	// 29 
	  -1,	// 2A 
	  -1,	// 2B 
	  -1,	// 2C 
	  -1,	// 2D Ins
	  -1,	// 2E Del
	  -1,	// 2F 
	0x04,	// 30 0		>
	  -1,	// 31 1
	0x00,	// 32 2		"
	0x08,	// 33 3		#
	0x01,	// 34 4		$
	0x09,	// 35 5		%
	0x02,	// 36 6		(
	0x0a,	// 37 7		)
	0x03,	// 38 8		=
	0x0b,	// 39 9		<
	  -1,	// 3A 
	  -1,	// 3B 
	  -1,	// 3C 
	  -1,	// 3D 
	  -1,	// 3E 
	  -1,	// 3F 
	  -1,	// 40 
	0x20,	// 41 A
	0x3a,	// 42 B
	0x39,	// 43 C
	0x21,	// 44 D
	0x11,	// 45 E
	0x29,	// 46 F
	0x22,	// 47 G
	0x2a,	// 48 H
	0x1b,	// 49 I
	0x23,	// 4A J
	0x2b,	// 4B K
	0x24,	// 4C L
	0x3b,	// 4D M
	0x32,	// 4E N
	0x14,	// 4F O
	0x1c,	// 50 P
	0x10,	// 51 Q
	0x19,	// 52 R
	0x28,	// 53 S
	0x12,	// 54 T
	0x13,	// 55 U
	0x31,	// 56 V
	0x18,	// 57 W
	0x30,	// 58 X
	0x1a,	// 59 Y
	0x38,	// 5A Z
	  -1,	// 5B 
	  -1,	// 5C 
	  -1,	// 5D 
	  -1,	// 5E 
	  -1,	// 5F 
	0x3f,	// 60 NUM 0
	0x35,	// 61 NUM 1
	0x3d,	// 62 NUM 2
	0x3e,	// 63 NUM 3
	0x25,	// 64 NUM 4
	0x2d,	// 65 NUM 5
	0x26,	// 66 NUM 6
	0x15,	// 67 NUM 7
	0x1d,	// 68 NUM 8
	0x16,	// 69 NUM 9
	0x2e,	// 6A NUM *
	0x37,	// 6B NUM +
	0x2f,	// 6C NUM Ent	COMP
	0x36,	// 6D NUM -
	0x3c,	// 6E NUM .
	0x1e,	// 6F NUM /
#if 1
	0x35,	// 70 F1	1
	0x3d,	// 71 F2	2
	0x3e,	// 72 F3	3
	0x25,	// 73 F4	4
	0x2d,	// 74 F5	5
	0x26,	// 75 F6	6
	0x15,	// 76 F7	7
	0x1d,	// 77 F8	8
	0x16,	// 78 F9	9
	0x3f,	// 79 F10	0
#else
	  -1,	// 70 F1
	  -1,	// 71 F2
	  -1,	// 72 F3
	  -1,	// 73 F4
	  -1,	// 74 F5
	  -1,	// 75 F6
	  -1,	// 76 F7
	  -1,	// 77 F8
	  -1,	// 78 F9
	  -1,	// 79 F10
#endif
	  -1,	// 7A F11
	  -1,	// 7B F12
	  -1,	// 7C F13
	  -1,	// 7D F14
	  -1,	// 7E F15
	  -1,	// 7F F16
	  -1,	// 80 F17
	  -1,	// 81 F18
	  -1,	// 82 F19
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
	  -1,	// 91 ScrLk
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
	  -1,	// A0 L Shift
	  -1,	// A1 R Shift
	  -1,	// A2 L Ctrl
	0x2f,	// A3 R Ctrl	COMP
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
	0x1f,	// BA :
	0x17,	// BB ;
	0x33,	// BC ,
	  -1,	// BD -
	0x3c,	// BE .
	  -1,	// BF /
	  -1,	// C0 @
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
	  -1,	// DB [
	  -1,	// DC Yen
	  -1,	// DD ]
	  -1,	// DE ^
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

void IO::initialize()
{
	key_fifo = new FIFO(8);
	register_frame_event(this);
	
	get_host_time(&cur_time);
	register_event(this, EVENT_1SEC, 1000000, true, NULL);
}

void IO::release()
{
	key_fifo->release();
	delete key_fifo;
}

void IO::reset()
{
	key_fifo->clear();
	key_data = 0xff;
	
	crtc_blink = crtc_disp = 0;
	
	cmt_ear = false;
	op1_addr = op1_data = 0;
}

void IO::event_frame()
{
	if(!key_fifo->empty()) {
		key_data = key_fifo->read();
		d_cpu->write_signal(SIG_CPU_IRQ, 1, 1);
	}
	crtc_blink++;
}

void IO::event_callback(int event_id, int err)
{
	if(event_id == EVENT_1SEC) {
		if(cur_time.initialized) {
			cur_time.increment();
		} else {
			get_host_time(&cur_time);	// resync
			cur_time.initialized = true;
		}
	}
}

void IO::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_IO_DISP) {
		crtc_disp = ((data & mask) != 0);
	} else if(id == SIG_IO_EAR) {
		cmt_ear = ((data & mask) != 0);
	}
}

void IO::write_io8(uint32_t addr, uint32_t data)
{
	bool unknown = false;
	
	switch(addr) {
	case 0x6002:
		d_crtc->write_io8(1, data);
		break;
	case 0x6003:
		d_crtc->write_io8(0, data);
		break;
	default:
		// OP-1
		if((addr & 0xf000) == 0x6000 && (addr & 0xff00) != 0x6000) {
			switch(addr & 0xff00) {
			case 0x6300:
				// RTC: A4-A6 seems to be used for strobe signals ???
				if((op1_data & 0xf0) == 0x30 && (data & 0xf0) == 0x10) {
					set_rtc(addr, data);
				}
				break;
			default:
				unknown = true;
				break;
			}
			op1_addr = addr;
			op1_data = data;
		} else {
			unknown = true;
		}
		break;
	}
#ifdef _IO_DEBUG_LOG
	_TCHAR tmp[256] = {0};
	if(unknown) {
		my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("UNKNOWN:\t"));
	}
	my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("%04x\tOUT8\t%04x,%02x\n"), get_cpu_pc(0), addr, data & 0xff);
	this->out_debug_log(tmp);
#endif
}

uint32_t IO::read_io8(uint32_t addr)
{
	uint32_t value = 0xff;
	bool unknown = false;
	
	switch(addr) {
	case 0x6001:
		// bit7: CRTC DISP
		// bit0: SHIFT
		value = (crtc_disp ? 0 : 0x80) | 0x7e | (key_data & 1);
		break;
	case 0x6002:
		value = d_crtc->read_io8(1);
		break;
	case 0x6003:
		value = d_crtc->read_io8(0);
		break;
	case 0x6004:
		value = key_data & ~3;
		break;
	case 0x6500:
		// OP-1
		switch(op1_addr & 0xff00) {
		case 0x6300:
			// RTC
			value = get_rtc(op1_addr);
			break;
		default:
			unknown = true;
			break;
		}
		break;
	default:
		unknown = true;
		break;
	}
#ifdef _IO_DEBUG_LOG
	_TCHAR tmp[256] = {0};
	if(unknown) {
		my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("UNKNOWN:\t"));
	}
	my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("%04x\tIN8\t%04x = %02x\n"), get_cpu_pc(0), addr, value & 0xff);
	this->out_debug_log(tmp);
#endif
	return value;
}

void IO::key_down(int code)
{
	if((code = table[code]) != -1) {
		key_fifo->write((code << 2) | (emu->get_key_buffer()[0x10] ? 0 : 1));
	}
}

void IO::draw_screen(uint8_t *vram)
{
	scrntype_t green = RGB_COLOR(0, 255, 0);
	uint8_t screen[128][256];
	uint8_t *regs = d_crtc->get_regs();
	uint32_t addr = regs[12] * 256 + regs[13];
	int cursor_x = -1, cursor_y = -1;
	
	if((regs[8] & 0xc0) != 0xc0) {
		uint8_t bp = regs[10] & 0x60;
		if(bp == 0x00 || (bp == 0x40 && (crtc_blink & 0x08) != 0) || (bp == 0x60 && (crtc_blink & 0x10) != 0)) {
			uint32_t cursor_addr = regs[14] * 256 + regs[15];
			cursor_x = cursor_addr & 31;
			cursor_y = cursor_addr >> 7;
		}
	}
	for(int y = 0; y < 16; y++) {
		for(int x = 0; x < 32; x++) {
			bool cursor_hit = (x == cursor_x && y == cursor_y);
			for(int l = 0; l < 8; l++) {
				uint8_t pat = (cursor_hit && l == 7) ? 0xff : vram[0xfff - (addr & 0xfff)];
				for(int b = 0; b < 8; b++) {
					screen[y * 8 + l][x * 8 + b] = pat & (0x80 >> b);
				}
				addr++;
			}
		}
	}
	for(int y = 0; y < 128; y++) {
		scrntype_t *dest = emu->get_screen_buffer(y);
		for(int x = 0; x < 256; x++) {
			dest[x] = screen[y][x] ? green : 0;
		}
	}
}

uint32_t IO::get_rtc(uint32_t addr)
{
	switch(addr & 0xff0f) {
	case 0x6300:
		return TO_BCD_LO(cur_time.second);
	case 0x6301:
		return TO_BCD_HI(cur_time.second);
	case 0x6302:
		return TO_BCD_LO(cur_time.minute);
	case 0x6303:
		return TO_BCD_HI(cur_time.minute);
	case 0x6304:
		return TO_BCD_LO(cur_time.hour);
	case 0x6305:
		return TO_BCD_HI(cur_time.hour);
	case 0x6306:
		return TO_BCD_LO(cur_time.day);
	case 0x6307:
		return TO_BCD_HI(cur_time.day);
	case 0x6308:
		return TO_BCD_LO(cur_time.month);
	case 0x6309:
		return TO_BCD_HI(cur_time.month);
	case 0x630a:
		return TO_BCD_LO(cur_time.year);
	case 0x630b:
		return TO_BCD_HI(cur_time.year);
	}
	return 0;
}

void IO::set_rtc(uint32_t addr, uint32_t data)
{
	// NOTE: this code does not consider the case rtc is incremented while setting registers
	switch(addr & 0xff0f) {
	case 0x6300:
		cur_time.second = TO_BCD_HI(cur_time.second) * 10 + (data & 0x0f);
		break;
	case 0x6301:
		cur_time.second = TO_BCD_LO(cur_time.second) + (data & 0x0f) * 10;
		break;
	case 0x6302:
		cur_time.minute = TO_BCD_HI(cur_time.minute) * 10 + (data & 0x0f);
		break;
	case 0x6303:
		cur_time.minute = TO_BCD_LO(cur_time.minute) + (data & 0x0f) * 10;
		break;
	case 0x6304:
		cur_time.hour = TO_BCD_HI(cur_time.hour) * 10 + (data & 0x0f);
		break;
	case 0x6305:
		cur_time.hour = TO_BCD_LO(cur_time.hour) + (data & 0x0f) * 10;
		break;
	case 0x6306:
		cur_time.day = TO_BCD_HI(cur_time.day) * 10 + (data & 0x0f);
		break;
	case 0x6307:
		cur_time.day = TO_BCD_LO(cur_time.day) + (data & 0x0f) * 10;
		break;
	case 0x6308:
		cur_time.month = TO_BCD_HI(cur_time.month) * 10 + (data & 0x0f);
		break;
	case 0x6309:
		cur_time.month = TO_BCD_LO(cur_time.month) + (data & 0x0f) * 10;
		break;
	case 0x630a:
		cur_time.year = TO_BCD_HI(cur_time.year) * 10 + (data & 0x0f);
		cur_time.update_year();
		break;
	case 0x630b:
		cur_time.year = TO_BCD_LO(cur_time.year) + (data & 0x0f) * 10;
		cur_time.update_year();
		break;
	}
}

#define STATE_VERSION	1

bool IO::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	if(!key_fifo->process_state((void *)state_fio, loading)) {
		return false;
	}
	if(!cur_time.process_state((void *)state_fio, loading)) {
		return false;
	}
	state_fio->StateValue(key_data);
	state_fio->StateValue(crtc_blink);
	state_fio->StateValue(crtc_disp);
	state_fio->StateValue(cmt_ear);
	state_fio->StateValue(op1_addr);
	state_fio->StateValue(op1_data);
	return true;
}
