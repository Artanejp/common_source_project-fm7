/*
	SONY SMC-70 Emulator 'eSMC-70'
	SONY SMC-777 Emulator 'eSMC-777'

	Author : Takeda.Toshiya
	Date   : 2015.08.13-

	[ memory and i/o bus ]
*/

#include "memory.h"
#include "../datarec.h"
#include "../mb8877.h"
#if defined(_SMC70)
#include "../msm58321.h"
#endif
#include "../pcm1bit.h"

#define EVENT_KEY_REPEAT	0
#define EVENT_TEXT_BLINK	1

#define IRQ_BIT_VSYNC	1
#define IRQ_BIT_KEYIN	2

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

static const uint8_t keytable_base[68][6] = {
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

void MEMORY::initialize()
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
#if defined(_SMC70)
	if(fio->Fopen(create_local_path(_T("SMC70ROMA.DAT")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, 0x4000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("SMC70ROMB.DAT")), FILEIO_READ_BINARY)) {
		fio->Fread(rom + 0x4000, 0x4000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("SMC70ROM.DAT")), FILEIO_READ_BINARY)) {
#else
	if(fio->Fopen(create_local_path(_T("SMCROM.DAT")), FILEIO_READ_BINARY) ||
	   fio->Fopen(create_local_path(_T("SMC777ROM.DAT")), FILEIO_READ_BINARY)) {
#endif
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("KANJIROM.DAT")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji, sizeof(kanji), 1);
		fio->Fclose();
	}
#if defined(_SMC70)
	if(fio->Fopen(create_local_path(_T("BASICROM.DAT")), FILEIO_READ_BINARY)) {
		fio->Fread(basic, sizeof(basic), 1);
		fio->Fclose();
	}
#endif
	delete fio;
	
	// initialize inputs
	initialize_key();
	caps = kana = false;
	key_stat = emu->get_key_buffer();
	joy_stat = emu->get_joy_buffer();
	
	// initialize display
	static const uint8_t color_table[16][3] = {
		{  0,  0,  0}, {  0,  0,255}, {  0,255,  0}, {  0,255,255}, {255,  0,  0}, {255,  0,255}, {255,255,  0}, {255,255,255},
		// from WinSMC
		{ 16, 64, 16}, { 16,112, 32}, {208, 80, 32}, {224,144, 32}, { 16, 80,128}, { 16,144,224}, {240,112,144}, {128,128,128}
	};
	for(int i = 0; i < 16 + 16; i++) {
		palette_pc[i] = RGB_COLOR(color_table[i & 15][0], color_table[i & 15][1], color_table[i & 15][2]);
	}
#if defined(_SMC70)
	palette_bw_pc[0] = RGB_COLOR(  0,   0,   0);
	palette_bw_pc[1] = RGB_COLOR(255, 255, 255);
#endif
	
	vsup = false;
#if defined(_SMC777)
	use_palette_text = use_palette_graph = false;
	memset(pal, 0, sizeof(pal));
#endif
	vsync_irq = false;
	
	// register event
	register_frame_event(this);
	register_vline_event(this);
	register_event(this, EVENT_TEXT_BLINK, 1000000.0 / 8.0, true, NULL); // 2.6Hz-4.4Hz
}

void MEMORY::reset()
{
	SET_BANK(0x0000, sizeof(rom) - 1, wdmy, rom);
	SET_BANK(sizeof(rom), 0xffff, ram + sizeof(rom), ram + sizeof(rom));
	rom_selected = true;
	rom_switch_wait = ram_switch_wait = 0;
	
	key_code = key_status = key_cmd = 0;
	key_repeat_event = -1;
	
	gcw = 0x80;
	vsync = disp = blink = false;
	cblink = 0;
	
	ief_key = ief_vsync = false;//true;
//	vsync_irq = false;
	fdc_irq = fdc_drq = false;
	drec_in = false;
}

void MEMORY::initialize_key()
{
	memset(keytable, 0, sizeof(keytable));
	memset(keytable_shift, 0, sizeof(keytable_shift));
	memset(keytable_ctrl, 0, sizeof(keytable_ctrl));
	memset(keytable_kana, 0, sizeof(keytable_kana));
	memset(keytable_kana_shift, 0, sizeof(keytable_kana_shift));
	
	for(int i = 0; i < 68; i++) {
		uint8_t code = keytable_base[i][0];
		keytable[code] = keytable_base[i][1];
		keytable_shift[code] = keytable_base[i][2];
		keytable_ctrl[code] = keytable_base[i][3];
		keytable_kana[code] = keytable_base[i][4];
		keytable_kana_shift[code] = keytable_base[i][5];
	}
	key_repeat_start = 1000;
	key_repeat_interval = 100;
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	wbank[(addr >> 14) & 3][addr & 0x3fff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	return rbank[(addr >> 14) & 3][addr & 0x3fff];
}

uint32_t MEMORY::fetch_op(uint32_t addr, int *wait)
{
	if(rom_switch_wait) {
		if(--rom_switch_wait == 0) {
			SET_BANK(0x0000, sizeof(rom) - 1, wdmy, rom);
			rom_selected = true;
		}
	} else if(ram_switch_wait) {
		if(--ram_switch_wait == 0) {
			SET_BANK(0x0000, sizeof(rom) - 1, ram, ram);
			rom_selected = false;
		}
	}
	*wait = 0;
	return read_data8(addr);
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
#ifdef _IO_DEBUG_LOG
	this->out_debug_log(_T("%04x\tOUT8\t%04x,%02x\n"), d_cpu->get_pc(), addr, data);
#endif
	uint8_t laddr = addr & 0xff;
	
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
				d_drec->write_signal(SIG_DATAREC_MIC, data, 0x10);
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
#if defined(_SMC70)
		case 0x24:
			d_rtc->write_signal(SIG_MSM58321_CS, data, 0x80);
			d_rtc->write_signal(SIG_MSM58321_ADDR_WRITE, data, 0x10);	// prev data is written
			d_rtc->write_signal(SIG_MSM58321_DATA, data, 0x0f);
			d_rtc->write_signal(SIG_MSM58321_READ, data, 0x20);
			d_rtc->write_signal(SIG_MSM58321_WRITE, data, 0x40);		// current data is written
			break;
#endif
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
#if defined(_SMC777)
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
#endif
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

uint32_t MEMORY::read_io8(uint32_t addr)
#ifdef _IO_DEBUG_LOG
{
	uint32_t val = read_io8_debug(addr);
	this->out_debug_log(_T("%04x\tIN8\t%04x = %02x\n"), d_cpu->get_pc(), addr, val);
	return val;
}

uint32_t MEMORY::read_io8_debug(uint32_t addr)
#endif
{
	uint8_t laddr = addr & 0xff;
	
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
			// bit1,0: AUTO START	1,0:ROM, 0,0:DISK, 1,1:OFF (SMC-70)
#if defined(_SMC70)
			return (warm_start ? 0x80 : 0) | 4 | (config.boot_mode == 0 ? 2 : config.boot_mode == 1 ? 0 : 3);
#else
			return (warm_start ? 0x80 : 0);
#endif
		case 0x1d:
			// bit7: TC IN		input signal from cassette data recorder
			// bit4: PR BUSY	printer busy
			// bit3: PR ACK		printer ack
			// bit2: ID		0 = SMC-777, 1 = SMC-70
			// bit1,0: AUTO START	1,0:ROM, 0,0:DISK, 1,1:OFF (SMC-70)
#if defined(_SMC70)
			return (drec_in ? 0x80 : 0) | 4 | (config.boot_mode == 0 ? 2 : config.boot_mode == 1 ? 0 : 3);
#else
			return (drec_in ? 0x80 : 0);
#endif
		case 0x20:
			// is this okay???
			return gcw;
		case 0x21:
			// is this okay???
			{
				uint32_t value = vsync_irq ? 1 : 0;
				if(!(d_cpu->read_signal(SIG_CPU_IRQ) & IRQ_BIT_VSYNC)) {
					vsync_irq = false;
				}
				return value;
			}
#if defined(_SMC70)
		case 0x25:
			return (rtc_busy ? 0x80 : 0) | (rtc_data & 0x0f);
#endif
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
#if defined(_SMC777)
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
				uint32_t stat = joy_stat[(addr & 0x100) ? 0 : 1];
				if(addr & 0x100) {
					if(key_stat[0x26]) stat |= 0x01; // up
					if(key_stat[0x28]) stat |= 0x02; // down
					if(key_stat[0x25]) stat |= 0x04; // left
					if(key_stat[0x27]) stat |= 0x08; // right
					if(key_stat[0x20]) stat |= 0x10; // space
				}
				return (~stat & 0x1f) | (disp ? 0x80 : 0);
			}
#endif
		case 0x7e: // KANJI ROM data
			// addr bit8-12: l/r and raster
			if(kanji_hi >= 0x21 && kanji_hi <= 0x4f && kanji_lo >= 0x20 && kanji_lo <= 0x7f) {
				int ofs = (kanji_hi - 0x21) * 96 + (kanji_lo - 0x20);
				return kanji[ofs * 32 + ((addr >> 8) & 0x1f)];
			}
			break;
#if defined(_SMC70)
		case 0x7f: // BASIC ROM data
			return basic[(kanji_lo << 8) | ((addr >> 8) & 0xff)];
#endif
		}
		return 0;//0xff;
	} else {
		addr = ((addr & 0xff00) >> 8) | (laddr << 8);
		return gram[addr & 0x7fff];
	}
}


void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMORY_FDC_IRQ) {
		fdc_irq = ((data & mask) != 0);
	} else if(id == SIG_MEMORY_FDC_DRQ) {
		fdc_drq = ((data & mask) != 0);
	} else if(id == SIG_MEMORY_CRTC_DISP) {
		disp = ((data & mask) != 0);
	} else if(id == SIG_MEMORY_CRTC_VSYNC) {
		bool prev = vsync;
		vsync = ((data & mask) != 0);
		if(prev && !vsync && ief_vsync) {
			vsync_irq = true;
			d_cpu->write_signal(SIG_CPU_IRQ, IRQ_BIT_VSYNC, IRQ_BIT_VSYNC);
		}
	} else if(id == SIG_MEMORY_DATAREC_IN) {
		drec_in = ((data & mask) != 0);
#if defined(_SMC70)
	} else if(id == SIG_MEMORY_RTC_DATA) {
		rtc_data = data & mask;
	} else if(id == SIG_MEMORY_RTC_BUSY) {
		rtc_busy = ((data & mask) != 0);
#endif
	}
}

void MEMORY::key_down_up(int code, bool down)
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
				d_cpu->write_signal(SIG_CPU_IRQ, IRQ_BIT_KEYIN, IRQ_BIT_KEYIN);
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

void MEMORY::event_callback(int event_id, int err)
{
	if(event_id == EVENT_KEY_REPEAT) {
		if(ief_key) {
			d_cpu->write_signal(SIG_CPU_IRQ, IRQ_BIT_KEYIN, IRQ_BIT_KEYIN);
		}
		key_status |= 5;
		register_event(this, EVENT_KEY_REPEAT, key_repeat_interval * 1000, false, &key_repeat_event);
	} else if(event_id == EVENT_TEXT_BLINK) {
		blink = !blink;
	}
}

void MEMORY::event_frame()
{
	cblink = (cblink + 1) & 0x1f;
}

void MEMORY::event_vline(int v, int clock)
{
	if(v < 200) {
#if defined(_SMC777)
		scrntype_t *palette_text_pc = &palette_pc[use_palette_text ? 16 : 0];
		scrntype_t *palette_graph_pc = &palette_pc[use_palette_graph ? 16 : 0];
		
		memcpy(palette_line_text_pc[v], palette_text_pc, sizeof(scrntype_t) * 16);
		memcpy(palette_line_graph_pc[v], palette_graph_pc, sizeof(scrntype_t) * 16);
#endif
		
		// render text/graph screens
		if(v == 0) {
			memset(text, 0, sizeof(text));
			memset(graph, 0, sizeof(graph));
		}
		if(vsup) {
			return;
		}
		if(gcw & 0x80) {
			draw_text_40x25(v);
		} else {
			draw_text_80x25(v);
		}
#if defined(_SMC777)
		if(gcw & 0x08) {
			draw_graph_640x200(v);
		} else {
			draw_graph_320x200(v);
		}
#else
		switch(gcw & 0x0c) {
		case 0x00: draw_graph_160x100(v); break;
		case 0x04: draw_graph_320x200(v); break;
		case 0x08: draw_graph_640x200(v); break;
		case 0x0c: draw_graph_640x400(v); break;
		}
#endif
	}
}

void MEMORY::draw_screen()
{
	if(emu->now_waiting_in_debugger) {
		// draw lines
		for(int v = 0; v < 200; v++) {
			event_vline(v, 0);
		}
	}
	
#if defined(_SMC70)
	if((gcw & 0x0c) == 0x0c) {
		emu->screen_skip_line(false);
	} else
#endif
	emu->screen_skip_line(true);
	
	// copy to screen buffer
#if defined(_SMC70)
	#define palette_text_pc  palette_pc
//	#define palette_graph_pc palette_pc
	scrntype_t *palette_graph_pc = ((gcw & 0x0c) == 0x0c) ? palette_bw_pc : palette_pc;
	
	if((gcw & 0x0c) == 0x0c) {
		for(int y = 0; y < 400; y++) {
			scrntype_t* dest = emu->get_screen_buffer(y);
			uint8_t* src_t = text[y >> 1];
			uint8_t* src_g = graph[y];
			
			for(int x = 0; x < 640; x++) {
				uint8_t t = src_t[x];
				dest[x] = t ? palette_text_pc[t & 15] : palette_graph_pc[src_g[x]];
			}
		}
	} else
#endif
	for(int y = 0; y < 200; y++) {
		scrntype_t* dest0 = emu->get_screen_buffer(y * 2);
		scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1);
		uint8_t* src_t = text[y];
		uint8_t* src_g = graph[y];
#if defined(_SMC777)
		scrntype_t *palette_text_pc = palette_line_text_pc[y];
		scrntype_t *palette_graph_pc = palette_line_graph_pc[y];
#endif
		
		for(int x = 0; x < 640; x++) {
			uint8_t t = src_t[x];
			dest0[x] = t ? palette_text_pc[t & 15] : palette_graph_pc[src_g[x]];
		}
		if(config.scan_line) {
			memset(dest1, 0, 640 * sizeof(scrntype_t));
		} else {
			memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
		}
	}
}

void MEMORY::draw_text_80x25(int v)
{
	int hz = crtc_regs[1];
	int vt = crtc_regs[6] & 0x7f;
	int ht = 8;// (crtc_regs[9] & 0x1f) + 1;
	uint8_t bp = crtc_regs[10] & 0x60;
	uint16_t src = (crtc_regs[12] << 8) | crtc_regs[13];
	uint16_t cursor = (crtc_regs[14] << 8) | crtc_regs[15];
	
	src &= 0x7ff;
	cursor &= 0x7ff;
	
	int y = v / ht;
	int l = v % ht;
	
	src += 80 * y;
	src &= 0x7ff;
	
//	for(int y = 0; y < vt && y < 25; y++) {
		for(int x = 0; x < hz && x < 80; x++) {
			uint8_t code = cram[src];
			uint8_t attr = aram[src];
			
			if(attr & 0x80) {
				attr = 7;
			}
			bool blink_tmp = ((attr & 0x40) && blink);
			bool reverse = (((attr & 0x20) != 0) != blink_tmp);
			
			uint8_t front = (attr & 7) | 16, back;
			switch((attr >> 3) & 3) {
			case 0: back =  0; break; // transparent
			case 1: back =  7; break; // white
			case 2: back = 16; break; // black
			case 3: back = ~front; break;
			}
			
			// draw pattern
//			for(int l = 0; l < ht; l++) {
				uint8_t pat = (l < 8) ? pcg[(code << 3) + l] : 0;
				if(reverse) pat = ~pat;
				int yy = y * ht + l;
				if(yy >= 200) {
					break;
				}
				uint8_t* d = &text[yy][x << 3];
				d[0] = (pat & 0x80) ? front : back;
				d[1] = (pat & 0x40) ? front : back;
				d[2] = (pat & 0x20) ? front : back;
				d[3] = (pat & 0x10) ? front : back;
				d[4] = (pat & 0x08) ? front : back;
				d[5] = (pat & 0x04) ? front : back;
				d[6] = (pat & 0x02) ? front : back;
				d[7] = (pat & 0x01) ? front : back;
//			}
			
			// draw cursor
			if(src == cursor) {
				int s = crtc_regs[10] & 0x1f;
				int e = crtc_regs[11] & 0x1f;
				if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
//					for(int l = s; l <= e && l < ht; l++) {
					if(l >= s && l <= e) {
//						int yy = y * ht + l;
						if(yy < 200) {
							memset(&text[yy][x << 3], 7, 8);
						}
					}
				}
			}
			src = (src + 1) & 0x7ff;
		}
//	}
}

void MEMORY::draw_text_40x25(int v)
{
	int hz = crtc_regs[1];
	int vt = crtc_regs[6] & 0x7f;
	int ht = 8;// (crtc_regs[9] & 0x1f) + 1;
	uint8_t bp = crtc_regs[10] & 0x60;
	uint16_t src = (crtc_regs[12] << 8) | crtc_regs[13];
	uint16_t cursor = (crtc_regs[14] << 8) | crtc_regs[15];
	
	int page = (gcw >> 6) & 1;
	src = (src & 0x7fe) | page;
	cursor = (cursor & 0x7fe) | page;
	
	int y = v / ht;
	int l = v % ht;
	
	src += 80 * y;
	src &= 0x7ff;
	
//	for(int y = 0; y < vt && y < 25; y++) {
		for(int x = 0; x < hz && x < 80; x += 2) {
			uint8_t code = cram[src];
			uint8_t attr = aram[src];
			
			if(attr & 0x80) {
				attr = 7;
			}
			bool blink_tmp = ((attr & 0x40) && blink);
			bool reverse = (((attr & 0x20) != 0) != blink_tmp);
			
			uint8_t front = (attr & 7) | 16, back;
			switch((attr >> 3) & 3) {
			case 0: back =  0; break; // transparent
			case 1: back =  7; break; // white
			case 2: back = 16; break; // black
			case 3: back = ~front; break;
			}
			
			// draw pattern
//			for(int l = 0; l < ht; l++) {
				uint8_t pat = (l < 8) ? pcg[(code << 3) + l] : 0;
				if(reverse) pat = ~pat;
				int yy = y * ht + l;
				if(yy >= 200) {
					break;
				}
				uint8_t* d = &text[yy][x << 3];
				d[ 0] = d[ 1] = (pat & 0x80) ? front : back;
				d[ 2] = d[ 3] = (pat & 0x40) ? front : back;
				d[ 4] = d[ 5] = (pat & 0x20) ? front : back;
				d[ 6] = d[ 7] = (pat & 0x10) ? front : back;
				d[ 8] = d[ 9] = (pat & 0x08) ? front : back;
				d[10] = d[11] = (pat & 0x04) ? front : back;
				d[12] = d[13] = (pat & 0x02) ? front : back;
				d[14] = d[15] = (pat & 0x01) ? front : back;
//			}
			
			// draw cursor
			if(src == cursor) {
				int s = crtc_regs[10] & 0x1f;
				int e = crtc_regs[11] & 0x1f;
				if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
//					for(int l = s; l <= e && l < ht; l++) {
					if(l >= s && l <= e) {
//						int yy = y * ht + l;
						if(yy < 200) {
							memset(&text[yy][x << 3], 7, 16);
						}
					}
				}
			}
			src = (src + 2) & 0x7ff;
		}
//	}
}

void MEMORY::draw_graph_640x400(int v)
{
	int hz = crtc_regs[1];
	int vt = crtc_regs[6] & 0x7f;
	int ht = 8;// (crtc_regs[9] & 0x1f) + 1;
	uint16_t src = (crtc_regs[12] << 8) | crtc_regs[13];
	
	int y = v / ht;
	int l = v % ht;
	
	src += 160 * y;
	
//	for(int y = 0; y < vt && y < 25; y++) {
		for(int x = 0; x < hz && x < 80; x++) {
//			for(int l = 0; l < ht; l++) {
				uint8_t pat0 = gram[(src + 0x1000 * l    ) & 0x7fff];
				uint8_t pat1 = gram[(src + 0x1000 * l + 1) & 0x7fff];
				int yy = y * ht + l;
				if(yy >= 200) {
					break;
				}
				uint8_t* d0 = &graph[2 * yy    ][x << 3];
				uint8_t* d1 = &graph[2 * yy + 1][x << 3];
				// FIXME: is this correct?
				d0[0] = (pat0 >> 7)    ;
				d0[1] = (pat0 >> 6) & 1;
				d0[2] = (pat0 >> 5) & 1;
				d0[3] = (pat0 >> 4) & 1;
				d0[4] = (pat0 >> 3) & 1;
				d0[5] = (pat0 >> 2) & 1;
				d0[6] = (pat0 >> 1) & 1;
				d0[7] = (pat0     ) & 1;
				d1[0] = (pat1 >> 7)    ;
				d1[1] = (pat1 >> 6) & 1;
				d1[2] = (pat1 >> 5) & 1;
				d1[3] = (pat1 >> 4) & 1;
				d1[4] = (pat1 >> 3) & 1;
				d1[5] = (pat1 >> 2) & 1;
				d1[6] = (pat1 >> 1) & 1;
				d1[7] = (pat1     ) & 1;
//			}
			src += 2;
		}
//	}
}

void MEMORY::draw_graph_640x200(int v)
{
	static const uint8_t color_table[2][4] = {{0, 4, 2, 1}, {0, 4, 2, 7}};
	static const uint8_t* color_ptr = color_table[(gcw >> 5) & 1];
	
	int hz = crtc_regs[1];
	int vt = crtc_regs[6] & 0x7f;
	int ht = 8;// (crtc_regs[9] & 0x1f) + 1;
	uint16_t src = (crtc_regs[12] << 8) | crtc_regs[13];
	
	int y = v / ht;
	int l = v % ht;
	
	src += 160 * y;
	
//	for(int y = 0; y < vt && y < 25; y++) {
		for(int x = 0; x < hz && x < 80; x++) {
//			for(int l = 0; l < ht; l++) {
				uint8_t pat0 = gram[(src + 0x1000 * l    ) & 0x7fff];
				uint8_t pat1 = gram[(src + 0x1000 * l + 1) & 0x7fff];
				int yy = y * ht + l;
				if(yy >= 200) {
					break;
				}
				uint8_t* d = &graph[yy][x << 3];
				d[0] = color_ptr[(pat0 >> 6)    ];
				d[1] = color_ptr[(pat0 >> 4) & 3];
				d[2] = color_ptr[(pat0 >> 2) & 3];
				d[3] = color_ptr[(pat0     ) & 3];
				d[4] = color_ptr[(pat1 >> 6)    ];
				d[5] = color_ptr[(pat1 >> 4) & 3];
				d[6] = color_ptr[(pat1 >> 2) & 3];
				d[7] = color_ptr[(pat1     ) & 3];
//			}
			src += 2;
		}
//	}
}

void MEMORY::draw_graph_320x200(int v)
{
	int hz = crtc_regs[1];
	int vt = crtc_regs[6] & 0x7f;
	int ht = 8;// (crtc_regs[9] & 0x1f) + 1;
	uint16_t src = (crtc_regs[12] << 8) | crtc_regs[13];
	
	int y = v / ht;
	int l = v % ht;
	
	src += 160 * y;
	
//	for(int y = 0; y < vt && y < 25; y++) {
		for(int x = 0; x < hz && x < 80; x++) {
//			for(int l = 0; l < ht; l++) {
				uint8_t pat0 = gram[(src + 0x1000 * l    ) & 0x7fff];
				uint8_t pat1 = gram[(src + 0x1000 * l + 1) & 0x7fff];
				int yy = y * ht + l;
				if(yy >= 200) {
					break;
				}
				uint8_t* d = &graph[yy][x << 3];
				d[0] = d[1] = pat0 >> 4;
				d[2] = d[3] = pat0 & 15;
				d[4] = d[5] = pat1 >> 4;
				d[6] = d[7] = pat1 & 15;
//			}
			src += 2;
		}
//	}
}

void MEMORY::draw_graph_160x100(int v)
{
	if(v & 1) {
		return;
	}
	int hz = crtc_regs[1];
	int vt = crtc_regs[6] & 0x7f;
	int ht = 8;// (crtc_regs[9] & 0x1f) + 1;
	uint16_t src = (crtc_regs[12] << 8) | crtc_regs[13];
	
	src += 0x1000 * ((gcw >> 1) & 1) + (gcw & 1);
	
	int y = v / ht;
	int l = v % ht;
	
	src += 160 * y;
	
//	for(int y = 0; y < vt && y < 25; y++) {
		for(int x = 0; x < hz && x < 80; x++) {
//			for(int l = 0; l < ht; l += 2) {
				uint8_t pat = gram[(src + 0x1000 * l) & 0x7fff];
				int yy = y * ht + l;
				if(yy >= 200) {
					break;
				}
				uint8_t* d0 = &graph[yy    ][x << 3];
				uint8_t* d1 = &graph[yy + 1][x << 3];
				d0[0] = d0[1] = d0[2] = d0[3] = 
				d1[0] = d1[1] = d1[2] = d1[3] = pat >> 4;
				d0[4] = d0[5] = d0[6] = d0[7] = 
				d1[4] = d1[5] = d1[6] = d1[7] = pat & 15;
//			}
			src += 2;
		}
//	}
}

#define STATE_VERSION	4

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateArray(cram, sizeof(cram), 1);
	state_fio->StateArray(aram, sizeof(aram), 1);
	state_fio->StateArray(pcg, sizeof(pcg), 1);
	state_fio->StateArray(gram, sizeof(gram), 1);
	state_fio->StateValue(rom_selected);
	state_fio->StateValue(rom_switch_wait);
	state_fio->StateValue(ram_switch_wait);
	state_fio->StateArray(keytable, sizeof(keytable), 1);
	state_fio->StateArray(keytable_shift, sizeof(keytable_shift), 1);
	state_fio->StateArray(keytable_ctrl, sizeof(keytable_ctrl), 1);
	state_fio->StateArray(keytable_kana, sizeof(keytable_kana), 1);
	state_fio->StateArray(keytable_kana_shift, sizeof(keytable_kana_shift), 1);
	state_fio->StateValue(key_code);
	state_fio->StateValue(key_status);
	state_fio->StateValue(key_cmd);
	state_fio->StateValue(key_repeat_start);
	state_fio->StateValue(key_repeat_interval);
	state_fio->StateValue(key_repeat_event);
	state_fio->StateValue(funckey_code);
	state_fio->StateValue(funckey_index);
	state_fio->StateValue(caps);
	state_fio->StateValue(kana);
	state_fio->StateValue(gcw);
	state_fio->StateValue(vsup);
	state_fio->StateValue(vsync);
	state_fio->StateValue(disp);
	state_fio->StateValue(cblink);
#if defined(_SMC777)
	state_fio->StateValue(use_palette_text);
	state_fio->StateValue(use_palette_graph);
	for(int i = 0; i < array_length(pal); i++) {
		state_fio->StateValue(pal[i].r);
		state_fio->StateValue(pal[i].g);
		state_fio->StateValue(pal[i].b);
	}
	state_fio->StateArray(palette_pc, sizeof(palette_pc), 1);
#endif
	state_fio->StateValue(kanji_hi);
	state_fio->StateValue(kanji_lo);
	state_fio->StateValue(ief_key);
	state_fio->StateValue(ief_vsync);
	state_fio->StateValue(vsync_irq);
	state_fio->StateValue(fdc_irq);
	state_fio->StateValue(fdc_drq);
	state_fio->StateValue(drec_in);
#if defined(_SMC70)
	state_fio->StateValue(rtc_data);
	state_fio->StateValue(rtc_busy);
#endif
	
	// post process
	if(loading) {
		if(rom_selected) {
			SET_BANK(0x0000, sizeof(rom) - 1, wdmy, rom);
		} else {
			SET_BANK(0x0000, sizeof(rom) - 1, ram, ram);
		}
	}
	return true;
}

