/*
	SONY SMC-777 Emulator 'eSMC-777'

	Author : Takeda.Toshiya
	Date   : 2015.08.13-

	[ i/o and memory bus ]
*/

#include "io.h"
#include "../datarec.h"
#include "../mb8877.h"
#include "../pcm1bit.h"

#define EVENT_KEY_REPEAT	0

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 14, eb = (e) >> 14; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x4000 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x4000 * (i - sb); \
		} \
	} \
}

static const uint8 keytable_base[68][6] = {
	{0x70, 0x01, 0x15, 0x1a, 0x01, 0x15},	// F1
	{0x71, 0x02, 0x18, 0x10, 0x02, 0x18},	// F2
	{0x72, 0x04, 0x12, 0x13, 0x04, 0x12},	// F3
	{0x73, 0x06, 0x05, 0x07, 0x06, 0x05},	// F4
	{0x74, 0x0b, 0x03, 0x0c, 0x0b, 0x03},	// F5
	{0x23, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e},	// CLR -> END
	{0x2e, 0x11, 0x11, 0x11, 0x11, 0x11},	// DEL
	{0x2d, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f},	// INS
	{0x24, 0x14, 0x14, 0x14, 0x14, 0x14},	// HOME
	{0x25, 0x16, 0x16, 0x16, 0x16, 0x16},	// LEFT
	{0x26, 0x17, 0x17, 0x17, 0x17, 0x17},	// UP
	{0x28, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c},	// DOWN
	{0x27, 0x19, 0x19, 0x19, 0x19, 0x19},	// RIGHT
	{0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b},	// ESC
	{0x31, 0x31, 0x21, 0x00, 0xb1, 0xa7},	// '1'
	{0x32, 0x32, 0x40, 0x00, 0xb2, 0xa8},	// '2'
	{0x33, 0x33, 0x23, 0x00, 0xb3, 0xa9},	// '3'
	{0x34, 0x34, 0x24, 0x00, 0xb4, 0xaa},	// '4'
	{0x35, 0x35, 0x25, 0x00, 0xb5, 0xab},	// '5'
	{0x36, 0x36, 0x5e, 0x00, 0xc5, 0xc5},	// '6'
	{0x37, 0x37, 0x26, 0x00, 0xc6, 0xc6},	// '7'
	{0x38, 0x38, 0x2a, 0x00, 0xc7, 0xc7},	// '8'
	{0x39, 0x39, 0x28, 0x00, 0xc8, 0xc8},	// '9'
	{0x30, 0x30, 0x29, 0x00, 0xc9, 0xc9},	// '0'
	{0xbd, 0x2d, 0x5f, 0x00, 0xd7, 0xd7},	// '-'
	{0xde, 0x3d, 0x2b, 0x00, 0xd8, 0xd8},	// '='
	{0xdc, 0x7f, 0x7f, 0x7f, 0xd9, 0xd9},	// RUB OUT -> '\'
	{0x08, 0x08, 0x08, 0x08, 0x08, 0x08},	// BS
	{0x51, 0x71, 0x51, 0x11, 0xb6, 0xb6},	// 'Q'
	{0x57, 0x77, 0x57, 0x17, 0xb7, 0xb7},	// 'W'
	{0x45, 0x65, 0x45, 0x05, 0xb8, 0xb8},	// 'E'
	{0x52, 0x72, 0x52, 0x12, 0xb9, 0xb9},	// 'R'
	{0x54, 0x74, 0x54, 0x14, 0xba, 0xba},	// 'T'
	{0x59, 0x79, 0x59, 0x19, 0xca, 0xca},	// 'Y'
	{0x55, 0x75, 0x55, 0x15, 0xcb, 0xcb},	// 'U'
	{0x49, 0x69, 0x49, 0x09, 0xcc, 0xcc},	// 'I'
	{0x4f, 0x6f, 0x4f, 0x0f, 0xcd, 0xcd},	// 'O'
	{0x50, 0x70, 0x50, 0x10, 0xce, 0xce},	// 'P'
	{0xc0, 0x5b, 0x7b, 0x1b, 0xda, 0xda},	// '['
	{0xdb, 0x5d, 0x7d, 0x1d, 0xdb, 0xb0},	// ']'
	{0x7b, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a},	// LF -> F12
	{0x41, 0x61, 0x41, 0x01, 0xbb, 0xbb},	// 'A'
	{0x53, 0x73, 0x53, 0x13, 0xbc, 0xbc},	// 'S'
	{0x44, 0x64, 0x44, 0x04, 0xbd, 0xbd},	// 'D'
	{0x46, 0x66, 0x46, 0x06, 0xbe, 0xbe},	// 'F'
	{0x47, 0x67, 0x47, 0x07, 0xbf, 0xbf},	// 'G'
	{0x48, 0x68, 0x48, 0x08, 0xcf, 0xcf},	// 'H'
	{0x4a, 0x6a, 0x4a, 0x0a, 0xd0, 0xd0},	// 'J'
	{0x4b, 0x6b, 0x4b, 0x0b, 0xd1, 0xd1},	// 'K'
	{0x4c, 0x6c, 0x4c, 0x0c, 0xd2, 0xd2},	// 'L'
	{0xbb, 0x3b, 0x3a, 0x00, 0xd3, 0xd3},	// ';'
	{0xba, 0x27, 0x22, 0x00, 0xde, 0xa2},	// ','
	{0xdd, 0x60, 0x7e, 0x00, 0xdf, 0xa3},	// '`'
	{0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d},	// RETURN
	{0x5a, 0x7a, 0x5a, 0x1a, 0xc0, 0xc0},	// 'Z'
	{0x58, 0x78, 0x58, 0x18, 0xc1, 0xc1},	// 'X'
	{0x43, 0x63, 0x43, 0x03, 0xc2, 0xaf},	// 'C'
	{0x56, 0x76, 0x56, 0x16, 0xc3, 0xc3},	// 'V'
	{0x42, 0x62, 0x42, 0x02, 0xc4, 0xc4},	// 'B'
	{0x4e, 0x6e, 0x4e, 0x0e, 0xd4, 0xac},	// 'N'
	{0x4d, 0x6d, 0x4d, 0x0d, 0xd5, 0xad},	// 'M'
	{0xbc, 0x2c, 0x3c, 0x00, 0xd6, 0xae},	// ','
	{0xbe, 0x2e, 0x3e, 0x1e, 0xdc, 0xa4},	// '.'
	{0xbf, 0x2f, 0x3f, 0x1f, 0xa6, 0xa1},	// '/'
	{0xe2, 0x5c, 0x7c, 0x1c, 0xdd, 0xa5},	// '\'
	{0x09, 0x09, 0x09, 0x09, 0x09, 0x09},	// TAB
	{0x20, 0x20, 0x20, 0x20, 0x20, 0x20},	// SPACE
	{0x7a, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d}	// H -> F11
};

void IO::initialize()
{
	// initialize memory
	memset(ram, 0, sizeof(ram));
	memset(rom, 0xff, sizeof(rom));
	memset(cram, 0, sizeof(cram));
	memset(aram, 0, sizeof(aram));
	memset(pcg, 0, sizeof(pcg));
	memset(gram, 0, sizeof(gram));
	memset(rdmy, 0xff, sizeof(rdmy));
	memset(kanji, 0xff, sizeof(kanji));
	
	// load WinSMC rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("SMCROM.DAT")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("KANJIROM.DAT")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji, sizeof(kanji), 1);
		fio->Fclose();
	}
	delete fio;
	
	// initialize inputs
	initialize_key();
	caps = kana = false;
	key_stat = emu->key_buffer();
	joy_stat = emu->joy_buffer();
	
	// initialize display
	static const uint8 color_table[16][3] = {
		{  0,  0,  0}, {  0,  0,255}, {  0,255,  0}, {  0,255,255}, {255,  0,  0}, {255,  0,255}, {255,255,  0}, {255,255,255},
		// from WinSMC
		{ 16, 64, 16}, { 16,112, 32}, {208, 80, 32}, {224,144, 32}, { 16, 80,128}, { 16,144,224}, {240,112,144}, {128,128,128}
	};
	for(int i = 0; i < 16 + 16; i++) {
		palette_pc[i] = RGB_COLOR(color_table[i & 15][0], color_table[i & 15][1], color_table[i & 15][2]);
	}
	vsup = false;
	use_palette_text = use_palette_graph = false;
	memset(pal, 0, sizeof(pal));
	
	// register event
	register_frame_event(this);
}

void IO::reset()
{
	SET_BANK(0x0000, 0x3fff, wdmy, rom);
	SET_BANK(0x4000, 0xffff, ram + 0x4000, ram + 0x4000);
	rom_selected = true;
	rom_switch_wait = ram_switch_wait = 0;
	
	key_code = key_status = key_cmd = 0;
	key_repeat_event = -1;
	
	gcw = 0x80;
	vsync = disp = false;
	cblink = 0;
	
	ief_key = ief_vsync = false;//true;
	fdc_irq = fdc_drq = false;
	drec_in = false;
}

void IO::initialize_key()
{
	memset(keytable, 0, sizeof(keytable));
	memset(keytable_shift, 0, sizeof(keytable_shift));
	memset(keytable_ctrl, 0, sizeof(keytable_ctrl));
	memset(keytable_kana, 0, sizeof(keytable_kana));
	memset(keytable_kana_shift, 0, sizeof(keytable_kana_shift));
	
	for(int i = 0; i < 68; i++) {
		uint8 code = keytable_base[i][0];
		keytable[code] = keytable_base[i][1];
		keytable_shift[code] = keytable_base[i][2];
		keytable_ctrl[code] = keytable_base[i][3];
		keytable_kana[code] = keytable_base[i][4];
		keytable_kana_shift[code] = keytable_base[i][5];
	}
	key_repeat_start = 1000;
	key_repeat_interval = 100;
}

void IO::write_data8(uint32 addr, uint32 data)
{
	wbank[(addr >> 14) & 3][addr & 0x3fff] = data;
}

uint32 IO::read_data8(uint32 addr)
{
	return rbank[(addr >> 14) & 3][addr & 0x3fff];
}

uint32 IO::fetch_op(uint32 addr, int *wait)
{
	if(rom_switch_wait) {
		if(--rom_switch_wait == 0) {
			SET_BANK(0x0000, 0x3fff, wdmy, rom);
			rom_selected = true;
		}
	} else if(ram_switch_wait) {
		if(--ram_switch_wait == 0) {
			SET_BANK(0x0000, 0x3fff, ram, ram);
			rom_selected = false;
		}
	}
	*wait = 0;
	return read_data8(addr);
}

void IO::write_io8(uint32 addr, uint32 data)
{
#ifdef _IO_DEBUG_LOG
	emu->out_debug_log(_T("%6x\tOUT8\t%04x,%02x\n"), d_cpu->get_pc(), addr, data);
#endif
	uint8 laddr = addr & 0xff;
	
	if(laddr < 0x08) {
		addr = ((addr & 0xff00) >> 8) | (laddr << 8);
		cram[addr & 0x7ff] = data;
	} else if(laddr < 0x10) {
		addr = ((addr & 0xff00) >> 8) | (laddr << 8);
		aram[addr & 0x7ff] = data;
	} else if(laddr < 0x18) {
		addr = ((addr & 0xff00) >> 8) | (laddr << 8);
		pcg[addr & 0x7ff] = data;
	} else if(laddr < 0x80) {
		switch(laddr) {
		case 0x18: // HD46505S-1 register number
		case 0x19: // HD46505S-1 register
			d_crtc->write_io8(addr, data);
			break;
		case 0x1a: // 8041 key encoder data
			if(key_cmd == 0) {
				// is this okay???
				if(data & 0x80) {
					// clear keyboard irq
				}
				if(data & 0x10) {
					// go to setting mode
					key_cmd = -1;
				}
				ief_key = ((data & 1) != 0);
			} else if(key_cmd == 0xa0) {
				// function key code
				int index = funckey_index++;
				if(index == 0) {
					// key code
					keytable[funckey_code] = keytable_kana[funckey_code] = data;
				} else if(index == 1) {
					// key  code with shift key
					keytable_shift[funckey_code] = keytable_kana_shift[funckey_code] = data;
				} else if(index == 2) {
					// key code with ctrl key
					keytable_ctrl[funckey_code] = data;
				} else {
					key_cmd = -1;
				}
			}
			break;
		case 0x1b: // 8041 key encoder control
			if(key_cmd == 0) {
				// bit7: ICF		1 = clear keyboard irq
				// bit4: FEF		1 = go to setting mode, 0 = scan mode
				// bit0: IEF		1 = enable keyboard irq
				if(data & 0x80) {
					// clear keyboard irq
				}
				if(data & 0x10) {
					// go to setting mode
					key_cmd = -1;
				}
				ief_key = ((data & 1) != 0);
			} else {
				// bit6-7: command type
				key_cmd = data & 0xe0;
				switch(data & 0xc0) {
				case 0xc0: // initialize key setting and exit setting mode
				case 0xe0:
					initialize_key();
				case 0x00: // exit setting mode
				case 0x20:
					key_cmd = 0;
					break;
				case 0x40: // set key repeat interval time
					// bit0-3: time
					key_repeat_interval = 20 + 20 * (data & 0x0f);
					break;
				case 0x60: // set key repeat start time
					key_repeat_start = 500 + 100 * (data & 0x0f);
					break;
				case 0x80: // read function key code
				case 0xa0: // write function key code
					// bit5: ~R/W
					// bit0-3: key address flag
					switch(data & 0x0f) {
					case 1:  funckey_code = 0x70; break; // F1
					case 2:  funckey_code = 0x71; break; // F2
					case 3:  funckey_code = 0x72; break; // F3
					case 4:  funckey_code = 0x73; break; // F4
					case 5:  funckey_code = 0x74; break; // F5
					case 6:  funckey_code = 0x7a; break; // H -> F11
					default: funckey_code = 0x00; break;
					}
					funckey_index = 0;
					break;
				}
			}
			break;
		case 0x1c:
		case 0x1d:
			// bit4: OD		output data
			// bit0-2: PA		pin selection
			switch(data & 7) {
			case 0:
				// RAM/~ROM	ram/rom switching will be done after next instruction
				if(data & 0x10) {
					ram_switch_wait = 2;
				} else {
					rom_switch_wait = 2;
				}
				break;
			case 1:
				// ~V.SUP	1 = screen is black
				vsup = ((data & 0x10) != 0);
				break;
			case 2:
				// ~525/625
				break;
			case 3:
				// RGB/~COMP
				break;
			case 4:
				// MOTOR ON/OFF
				d_drec->write_signal(SIG_DATAREC_REMOTE, data, 0x10);
				break;
			case 5:
				// SOUND OUT
				d_pcm->write_signal(SIG_PCM1BIT_SIGNAL, data, 0x10);
				break;
			case 6:
				// ~PRINTER STRB
				break;
			case 7:
				// CASSETE OUT
				d_drec->write_signal(SIG_DATAREC_OUT, data, 0x10);
				break;
			}
			break;
		case 0x20:
			// bit7: CM		0 = 80x25, 1 = 40x25
			// bit6: P		0 = 40x25 page 0 (even addr), 1 = page 1 (odd addr)
			// bit5: T		0 = 640x200 type 1, 1 = 640x200 type 2
			// bit3: GM		0 = 320x200, 1 = 640x200
			gcw = data;
			break;
		case 0x21:
			// bit0: IEF		1 = enable 60hz (vsync?) irq
			ief_vsync = ((data & 1) != 0);
			break;
		case 0x23:
			// bit0-3: border color
//			border = data & 0x0f;
			break;
		case 0x30: // MB8877 command register
		case 0x31: // MB8877 track register
		case 0x32: // MB8877 sector register
		case 0x33: // MB8877 data register
			d_fdc->write_io8(addr, data);
			break;
		case 0x34:
			// bit0: drive num	0 = drive A, 1 = drive B
			d_fdc->write_signal(SIG_MB8877_DRIVEREG, data, 1);
			break;
		case 0x51:
			// bit4: OD		output data
			// bit0-2: PA		pin selection
			switch(data & 7) {
			case 6: // character screen
				// bit4: 0 = color generator, 1 = color palette board
				use_palette_text = ((data & 0x10) != 0);
				break;
			case 7: // graphic screen
				// bit4: 0 = color generator, 1 = color palette board
				use_palette_graph = ((data & 0x10) != 0);
				break;
			}
			break;
		case 0x52:
			// addr bit12-13: AD	0 = red, 1 = green, 2 = blue
			// addr bit8-11: BD	color code
			// bit4-7: CD		0 = dark, 7 = blight
			{
				int n = (addr >> 8) & 15;
				switch((addr >> 12) & 3) {
				case 0: pal[n].r = data >> 4; break;
				case 1: pal[n].g = data >> 4; break;
				case 2: pal[n].b = data >> 4; break;
				}
				palette_pc[n + 16] = RGB_COLOR(pal[n].r * 255 / 15, pal[n].g * 255 / 15, pal[n].b * 255 / 15);
			}
			break;
		case 0x53: // SN76489AN
			d_psg->write_io8(addr, data);
			break;
		case 0x7e: // KANJI ROM jis code (hi)
			kanji_hi = data & 0x7f;
			break;
		case 0x7f: // KANJI ROM jis code (lo)
			kanji_lo = data & 0x7f;
			break;
		}
	} else {
		addr = ((addr & 0xff00) >> 8) | (laddr << 8);
		gram[addr & 0x7fff] = data;
	}
}

uint32 IO::read_io8(uint32 addr)
#ifdef _IO_DEBUG_LOG
{
	uint32 val = read_io8_debug(addr);
	emu->out_debug_log(_T("%06x\tIN8\t%04x = %02x\n"), d_cpu->get_pc(), addr, val);
	return val;
}

uint32 IO::read_io8_debug(uint32 addr)
#endif
{
	uint8 laddr = addr & 0xff;
	
	if(laddr < 0x08) {
		addr = ((addr & 0xff00) >> 8) | (laddr << 8);
		return cram[addr & 0x7ff];
	} else if(laddr < 0x10) {
		addr = ((addr & 0xff00) >> 8) | (laddr << 8);
		return aram[addr & 0x7ff];
	} else if(laddr < 0x18) {
		addr = ((addr & 0xff00) >> 8) | (laddr << 8);
		return pcg[addr & 0x7ff];
	} else if(laddr < 0x80) {
		switch(laddr) {
		case 0x18: // HD46505S-1 register number
		case 0x19: // HD46505S-1 register
			return d_crtc->read_io8(addr);
		case 0x1a: // 8041 key encoder data
			if(key_cmd == 0) {
				// key code
				key_status &= ~1;
				return key_code;
			} else if(key_cmd == 0x80) {
				// function key code
				int index = funckey_index++;
				if(index == 0) {
					// key code
					return keytable[funckey_code];
				} else if(index == 1) {
					// key  code with shift key
					return keytable_shift[funckey_code];
				} else if(index == 2) {
					// key  code with ctrl key
					return keytable_ctrl[funckey_code];
				} else {
					key_cmd = -1;
				}
			}
			break;
		case 0x1b: // 8041 key encoder status
			if(key_cmd == 0) {
				// bit7: CF		1 = ctrl key is pressed
				// bit6: SF		1 = shift key is pressed
				// bit2: ASF		1 = key is pressed pressed now
				// bit0: BSF		1 = key was pressed (will be cleared when key code is read)
				return key_status;
			} else {
				// bit2: ~BUSY		0 = command processing, 1 = command done
				// bit1: ~CS		0 = command accepted, 1 = command not accepted
				// bit0: DR		0 = data not ready, 1 = data ready
				if(key_cmd == 0x80) {
					return 5;
				} else if(key_cmd != -1) {
					return 4;
				}
				return 4;//6;
			}
			break;
		case 0x1c:
			// bit7: RES		0 = cold start (power-on), 1 = warm start (reset)
			// bit4: ~CP		0 = color palette board is attached
			// bit2: ID		0 = SMC-777, 1 = SMC-70
#if defined(_SMC70)
			return (warm_start ? 0x80 : 0) | 4;
#else
			return (warm_start ? 0x80 : 0);
#endif
		case 0x1d:
			// bit7: TC IN		input signal from cassette data recorder
			// bit4: PR BUSY	printer busy
			// bit3: PR ACK		printer ack
			// bit2: ID		0 = SMC-777, 1 = SMC-70
#if defined(_SMC70)
			return (drec_in ? 0x80 : 0) | 4;
#else
			return (drec_in ? 0x80 : 0);
#endif
		case 0x20:
			// is this okay???
			return gcw;
		case 0x21:
			// is this okay???
			return ief_vsync ? 1 : 0;
		case 0x30: // MB8877 status register
		case 0x31: // MB8877 track register
		case 0x32: // MB8877 sector register
		case 0x33: // MB8877 data register
			return d_fdc->read_io8(addr);
		case 0x34:
			// bit7: IRQ
			// bit6: ~DRQ
			// bit0-5: 0
			return (fdc_irq ? 0x80: 0) | (fdc_drq ? 0 : 0x40);
		case 0x51:
			// addr bit8:		0 = joystick #2, 1 = joystick #1
			// bit7: ~BL		0 = h/v blanking, 1 = not blanking
			// bit6: ~CS		0 = joystick #2 is enabled
			// bit4: ~T		0 = joystick trigger on
			// bit3: ~R		0 = joystick right on
			// bit2: ~L		0 = joystick left on
			// bit1: ~B		0 = joystick back on
			// bit0: ~F		0 = joystick forward on
			{
				uint32 stat = joy_stat[(addr & 0x100) ? 0 : 1];
				return (~stat & 0x0f) | ((stat & 0x30) ? 0 : 0x10) | (disp ? 0x80 : 0);
			}
		case 0x7e: // KANJI ROM data
			// addr bit8-12: l/r and raster
			if(kanji_hi >= 0x21 && kanji_hi <= 0x4f && kanji_lo >= 0x20 && kanji_lo <= 0x7f) {
				int ofs = (kanji_hi - 0x21) * 96 + (kanji_lo - 0x20);
				return kanji[ofs * 32 + ((addr >> 8) & 0x1f)];
			}
			break;
		}
		return 0;//0xff;
	} else {
		addr = ((addr & 0xff00) >> 8) | (laddr << 8);
		return gram[addr & 0x7fff];
	}
}


void IO::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_IO_FDC_IRQ) {
		fdc_irq = ((data & mask) != 0);
	} else if(id == SIG_IO_FDC_DRQ) {
		fdc_drq = ((data & mask) != 0);
	} else if(id == SIG_IO_CRTC_DISP) {
		disp = ((data & mask) != 0);
	} else if(id == SIG_IO_CRTC_VSYNC) {
		vsync = ((data & mask) != 0);
		if((data & mask) && ief_vsync) {
			d_cpu->write_signal(SIG_CPU_IRQ, 1, 1);
		}
	} else if(id == SIG_IO_DATAREC_IN) {
		drec_in = ((data & mask) != 0);
	}
}

void IO::key_down_up(int code, bool down)
{
	if(code == 0x14 && down) {
		caps = !caps;
		return;
	} else if(code == 0x15 && down) {
		kana = !kana;
		return;
	}
	bool shift = (key_stat[0x10] != 0);
	bool ctrl = (key_stat[0x11] != 0);
	bool kana_tmp = kana;
	
	if(code >= 0x60 && code <= 0x69) { // numpad 0-9
		code = code - 0x60 + 0x30;
		shift = ctrl = kana_tmp = false;
	} else if(code == 0x6a) { // numpad *
		code = 0x38;
		shift = true; ctrl = kana_tmp = false;
	} else if(code == 0x6b) { // numpad +
		code = 0xde;
		shift = true; ctrl = kana_tmp = false;
	} else if(code == 0x6c) { // numpad ,
		code = 0xbc;
		shift = ctrl = kana_tmp = false;
	} else if(code == 0x6d) { // numpad -
		code = 0xbd;
		shift = ctrl = kana_tmp = false;
	} else if(code == 0x6e) { // numpad .
		code = 0xbe;
		shift = ctrl = kana_tmp = false;
	} else if(code == 0x6f) { // numpad /
		code = 0xbf;
		shift = ctrl = kana_tmp = false;
	} else if(code >= 0x75 && code <= 0x79) { // F6-F10 -> Shift + F1-F5
		code = code - 0x75 + 0x70;
		shift = true;
	}
	if(ctrl) {
		code = keytable_ctrl[code];
	} else if(kana_tmp && shift) {
		code = keytable_kana_shift[code];
	} else if(kana_tmp) {
		code = keytable_kana[code];
	} else if(shift) {
		code = keytable_shift[code];
	} else {
		code = keytable[code];
	}
	if(code != 0) {
		if(caps && ((code >= 'a' && code <= 'z') || (code >= 'A' && code <= 'Z'))) {
			code ^= 0x20;
		}
		if(down && key_code != code) {
			if(ief_key) {
				d_cpu->write_signal(SIG_CPU_IRQ, 1, 1);
			}
			if(key_repeat_event != -1) {
				cancel_event(this, key_repeat_event);
			}
			key_code = code;
			key_status = ctrl ? 0x85 : shift ? 0x45 : 5;
			register_event(this, EVENT_KEY_REPEAT, key_repeat_start * 1000, false, &key_repeat_event);
		} else if(!down && key_code == code) {
			if(key_repeat_event != -1) {
				cancel_event(this, key_repeat_event);
				key_repeat_event = -1;
			}
			key_code = 0;
			key_status &= ~4;
		}
	}
}

void IO::event_callback(int event_id, int err)
{
	if(event_id == EVENT_KEY_REPEAT) {
		if(ief_key) {
			d_cpu->write_signal(SIG_CPU_IRQ, 1, 1);
		}
		key_status |= 5;
		register_event(this, EVENT_KEY_REPEAT, key_repeat_interval * 1000, false, &key_repeat_event);
	}
}

void IO::event_frame()
{
	cblink = (cblink + 1) & 0x1f;
}

void IO::draw_screen()
{
	emu->screen_skip_line = true;
	
	if(vsup) {
		for(int y = 0; y < 400; y++) {
			scrntype* dest = emu->screen_buffer(y);
			memset(dest, 0, 640 * sizeof(scrntype));
		}
		return;
	}
	
	// render text/graph screens
	memset(text, 0, sizeof(text));
	memset(graph, 0, sizeof(graph));
	
	if(gcw & 0x80) {
		draw_text_40x25();
	} else {
		draw_text_80x25();
	}
	if(gcw & 0x08) {
		draw_graph_640x200();
	} else {
		draw_graph_320x200();
	}
	
	// copy to screen buffer
	scrntype *palette_pc_text = &palette_pc[use_palette_text ? 16 : 0];
	scrntype *palette_pc_graph = &palette_pc[use_palette_graph ? 16 : 0];
	
	for(int y = 0; y < 200; y++) {
		scrntype* dest0 = emu->screen_buffer(y * 2);
		scrntype* dest1 = emu->screen_buffer(y * 2 + 1);
		uint8* src_t = text[y];
		uint8* src_g = graph[y];
		
		for(int x = 0; x < 640; x++) {
			uint8 t = src_t[x];
			dest0[x] = t ? palette_pc_text[t & 15] : palette_pc_graph[src_g[x]];
		}
		if(config.scan_line) {
			memset(dest1, 0, 640 * sizeof(scrntype));
		} else {
			memcpy(dest1, dest0, 640 * sizeof(scrntype));
		}
	}
}

void IO::draw_text_80x25()
{
	int hz = crtc_regs[1];
	int vt = crtc_regs[6] & 0x7f;
	int ht = (crtc_regs[9] & 0x1f) + 1;
	uint8 bp = crtc_regs[10] & 0x60;
	uint16 src = (crtc_regs[12] << 8) | crtc_regs[13];
	uint16 cursor = (crtc_regs[14] << 8) | crtc_regs[15];
	
	src &= 0x7ff;
	cursor &= 0x7ff;
	
	for(int y = 0; y < vt && y < 25; y++) {
		for(int x = 0; x < hz && x < 80; x++) {
			uint8 code = cram[src];
			uint8 attr = aram[src];
			
			if(attr & 0x80) {
				attr = 7;
			}
			bool blink = ((attr & 0x40) && (cblink & 0x20));
			bool reverse = (((attr & 0x20) != 0) != blink);
			
			uint8 front = (attr & 7) | 16, back;
			switch((attr >> 3) & 3) {
			case 0: back =  0; break; // transparent
			case 1: back =  7; break; // white
			case 2: back = 16; break; // black
			case 3: back = ~front; break;
			}
			
			// draw pattern
			for(int l = 0; l < ht; l++) {
				uint8 pat = (l < 8) ? pcg[(code << 3) + l] : 0;
				if(reverse) pat = ~pat;
				int yy = y * ht + l;
				if(yy >= 200) {
					break;
				}
				uint8* d = &text[yy][x << 3];
				d[0] = (pat & 0x80) ? front : back;
				d[1] = (pat & 0x40) ? front : back;
				d[2] = (pat & 0x20) ? front : back;
				d[3] = (pat & 0x10) ? front : back;
				d[4] = (pat & 0x08) ? front : back;
				d[5] = (pat & 0x04) ? front : back;
				d[6] = (pat & 0x02) ? front : back;
				d[7] = (pat & 0x01) ? front : back;
			
			}
			
			// draw cursor
			if(src == cursor) {
				int s = crtc_regs[10] & 0x1f;
				int e = crtc_regs[11] & 0x1f;
				if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int l = s; l <= e && l < ht; l++) {
						int yy = y * ht + l;
						if(yy < 200) {
							memset(&text[yy][x << 3], 7, 8);
						}
					}
				}
			}
			src = (src + 1) & 0x7ff;
		}
	}
}

void IO::draw_text_40x25()
{
	int hz = crtc_regs[1];
	int vt = crtc_regs[6] & 0x7f;
	int ht = (crtc_regs[9] & 0x1f) + 1;
	uint8 bp = crtc_regs[10] & 0x60;
	uint16 src = (crtc_regs[12] << 8) | crtc_regs[13];
	uint16 cursor = (crtc_regs[14] << 8) | crtc_regs[15];
	
	int page = (gcw >> 6) & 1;
	src = (src & 0x7fe) | page;
	cursor = (cursor & 0x7fe) | page;
	
	for(int y = 0; y < vt && y < 25; y++) {
		for(int x = 0; x < hz && x < 80; x += 2) {
			uint8 code = cram[src];
			uint8 attr = aram[src];
			
			if(attr & 0x80) {
				attr = 7;
			}
			bool blink = ((attr & 0x40) && (cblink & 0x20));
			bool reverse = (((attr & 0x20) != 0) != blink);
			
			uint8 front = (attr & 7) | 16, back;
			switch((attr >> 3) & 3) {
			case 0: back =  0; break; // transparent
			case 1: back =  7; break; // white
			case 2: back = 16; break; // black
			case 3: back = ~front; break;
			}
			
			// draw pattern
			for(int l = 0; l < ht; l++) {
				uint8 pat = (l < 8) ? pcg[(code << 3) + l] : 0;
				if(reverse) pat = ~pat;
				int yy = y * ht + l;
				if(yy >= 200) {
					break;
				}
				uint8* d = &text[yy][x << 3];
				d[ 0] = d[ 1] = (pat & 0x80) ? front : back;
				d[ 2] = d[ 3] = (pat & 0x40) ? front : back;
				d[ 4] = d[ 5] = (pat & 0x20) ? front : back;
				d[ 6] = d[ 7] = (pat & 0x10) ? front : back;
				d[ 8] = d[ 9] = (pat & 0x08) ? front : back;
				d[10] = d[11] = (pat & 0x04) ? front : back;
				d[12] = d[13] = (pat & 0x02) ? front : back;
				d[14] = d[15] = (pat & 0x01) ? front : back;
			
			}
			
			// draw cursor
			if(src == cursor) {
				int s = crtc_regs[10] & 0x1f;
				int e = crtc_regs[11] & 0x1f;
				if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int l = s; l <= e && l < ht; l++) {
						int yy = y * ht + l;
						if(yy < 200) {
							memset(&text[yy][x << 3], 7, 16);
						}
					}
				}
			}
			src = (src + 2) & 0x7ff;
		}
	}
}

void IO::draw_graph_640x200()
{
	static const uint8 color_table[2][4] = {{0, 4, 2, 1}, {0, 4, 2, 7}};
	static const uint8* color_ptr = color_table[(gcw >> 5) & 1];
	
	int hz = crtc_regs[1];
	int vt = crtc_regs[6] & 0x7f;
	int ht = (crtc_regs[9] & 0x1f) + 1;
	uint16 src = (crtc_regs[12] << 8) | crtc_regs[13];
	
	for(int y = 0; y < vt && y < 25; y++) {
		for(int x = 0; x < hz && x < 80; x++) {
			for(int l = 0; l < ht; l++) {
				uint8 pat0 = gram[(src + 0x1000 * l    ) & 0x7fff];
				uint8 pat1 = gram[(src + 0x1000 * l + 1) & 0x7fff];
				int yy = y * ht + l;
				if(yy >= 200) {
					break;
				}
				uint8* d = &graph[yy][x << 3];
				d[0] = color_ptr[(pat0 >> 6)    ];
				d[1] = color_ptr[(pat0 >> 4) & 3];
				d[2] = color_ptr[(pat0 >> 2) & 3];
				d[3] = color_ptr[(pat0     ) & 3];
				d[4] = color_ptr[(pat1 >> 6)    ];
				d[5] = color_ptr[(pat1 >> 4) & 3];
				d[6] = color_ptr[(pat1 >> 2) & 3];
				d[7] = color_ptr[(pat1     ) & 3];
			}
			src += 2;
		}
	}
}

void IO::draw_graph_320x200()
{
	int hz = crtc_regs[1];
	int vt = crtc_regs[6] & 0x7f;
	int ht = (crtc_regs[9] & 0x1f) + 1;
	uint16 src = (crtc_regs[12] << 8) | crtc_regs[13];
	
	for(int y = 0; y < vt && y < 25; y++) {
		for(int x = 0; x < hz && x < 80; x++) {
			for(int l = 0; l < ht; l++) {
				uint8 pat0 = gram[(src + 0x1000 * l    ) & 0x7fff];
				uint8 pat1 = gram[(src + 0x1000 * l + 1) & 0x7fff];
				int yy = y * ht + l;
				if(yy >= 200) {
					break;
				}
				uint8* d = &graph[yy][x << 3];
				d[0] = d[1] = pat0 >> 4;
				d[2] = d[3] = pat0 & 15;
				d[4] = d[5] = pat1 >> 4;
				d[6] = d[7] = pat1 & 15;
			}
			src += 2;
		}
	}
}

#define STATE_VERSION	1

void IO::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(ram, sizeof(ram), 1);
	state_fio->Fwrite(cram, sizeof(cram), 1);
	state_fio->Fwrite(aram, sizeof(aram), 1);
	state_fio->Fwrite(pcg, sizeof(pcg), 1);
	state_fio->Fwrite(gram, sizeof(gram), 1);
	state_fio->FputBool(rom_selected);
	state_fio->FputInt32(rom_switch_wait);
	state_fio->FputInt32(ram_switch_wait);
	state_fio->Fwrite(keytable, sizeof(keytable), 1);
	state_fio->Fwrite(keytable_shift, sizeof(keytable_shift), 1);
	state_fio->Fwrite(keytable_ctrl, sizeof(keytable_ctrl), 1);
	state_fio->Fwrite(keytable_kana, sizeof(keytable_kana), 1);
	state_fio->Fwrite(keytable_kana_shift, sizeof(keytable_kana_shift), 1);
	state_fio->FputUint8(key_code);
	state_fio->FputUint8(key_status);
	state_fio->FputUint8(key_cmd);
	state_fio->FputInt32(key_repeat_start);
	state_fio->FputInt32(key_repeat_interval);
	state_fio->FputInt32(key_repeat_event);
	state_fio->FputUint8(funckey_code);
	state_fio->FputInt32(funckey_index);
	state_fio->FputBool(caps);
	state_fio->FputBool(kana);
	state_fio->FputUint8(gcw);
	state_fio->FputBool(vsup);
	state_fio->FputBool(vsync);
	state_fio->FputBool(disp);
	state_fio->FputInt32(cblink);
	state_fio->FputBool(use_palette_text);
	state_fio->FputBool(use_palette_graph);
	state_fio->Fwrite(pal, sizeof(pal), 1);
	state_fio->Fwrite(palette_pc, sizeof(palette_pc), 1);
	state_fio->FputInt32(kanji_hi);
	state_fio->FputInt32(kanji_lo);
	state_fio->FputBool(ief_key);
	state_fio->FputBool(ief_vsync);
	state_fio->FputBool(fdc_irq);
	state_fio->FputBool(fdc_drq);
	state_fio->FputBool(drec_in);
}

bool IO::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(ram, sizeof(ram), 1);
	state_fio->Fread(cram, sizeof(cram), 1);
	state_fio->Fread(aram, sizeof(aram), 1);
	state_fio->Fread(pcg, sizeof(pcg), 1);
	state_fio->Fread(gram, sizeof(gram), 1);
	rom_selected = state_fio->FgetBool();
	rom_switch_wait = state_fio->FgetInt32();
	ram_switch_wait = state_fio->FgetInt32();
	state_fio->Fread(keytable, sizeof(keytable), 1);
	state_fio->Fread(keytable_shift, sizeof(keytable_shift), 1);
	state_fio->Fread(keytable_ctrl, sizeof(keytable_ctrl), 1);
	state_fio->Fread(keytable_kana, sizeof(keytable_kana), 1);
	state_fio->Fread(keytable_kana_shift, sizeof(keytable_kana_shift), 1);
	key_code = state_fio->FgetUint8();
	key_status = state_fio->FgetUint8();
	key_cmd = state_fio->FgetUint8();
	key_repeat_start = state_fio->FgetInt32();
	key_repeat_interval = state_fio->FgetInt32();
	key_repeat_event = state_fio->FgetInt32();
	funckey_code = state_fio->FgetUint8();
	funckey_index = state_fio->FgetInt32();
	caps = state_fio->FgetBool();
	kana = state_fio->FgetBool();
	gcw = state_fio->FgetUint8();
	vsup = state_fio->FgetBool();
	vsync = state_fio->FgetBool();
	disp = state_fio->FgetBool();
	cblink = state_fio->FgetInt32();
	use_palette_text = state_fio->FgetBool();
	use_palette_graph = state_fio->FgetBool();
	state_fio->Fread(pal, sizeof(pal), 1);
	state_fio->Fread(palette_pc, sizeof(palette_pc), 1);
	kanji_hi = state_fio->FgetInt32();
	kanji_lo = state_fio->FgetInt32();
	ief_key = state_fio->FgetBool();
	ief_vsync = state_fio->FgetBool();
	fdc_irq = state_fio->FgetBool();
	fdc_drq = state_fio->FgetBool();
	drec_in = state_fio->FgetBool();
	
	// post process
	if(rom_selected) {
		SET_BANK(0x0000, 0x3fff, wdmy, rom);
	} else {
		SET_BANK(0x0000, 0x3fff, ram, ram);
	}
	return true;
}

