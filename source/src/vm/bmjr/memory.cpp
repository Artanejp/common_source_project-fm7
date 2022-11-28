/*
	HITACH BASIC Master Jr Emulator 'eBASICMasterJr'

	Author : Takeda.Toshiya
	Date   : 2015.08.28-

	[ memory bus ]
*/

#include "memory.h"
#include "../datarec.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 11, eb = (e) >> 11; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x800 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x800 * (i - sb); \
		} \
	} \
}

#define SOUND_VOLUME	8000

static const int key_table[13][4] = {
	{0x5a, 0x41, 0x51, 0x31},	//	'Z'	'A'	'Q'	'1'
	{0x58, 0x53, 0x57, 0x32},	//	'X'	'S'	'W'	'2'
	{0x43, 0x44, 0x45, 0x33},	//	'C'	'D'	'E'	'3'
	{0x56, 0x46, 0x52, 0x34},	//	'V'	'F'	'R'	'4'
	{0x42, 0x47, 0x54, 0x35},	//	'B'	'G'	'T'	'5'
	{0x4e, 0x48, 0x59, 0x36},	//	'N'	'H'	'Y'	'6'
	{0x4d, 0x4a, 0x55, 0x37},	//	'M'	'J'	'U'	'7'
	{0xbc, 0x4b, 0x49, 0x38},	//	','	'K'	'I'	'8'
	{0xbe, 0x4c, 0x4f, 0x39},	//	'.'	'L'	'O'	'9'
	{0xbf, 0xbb, 0x50, 0x30},	//	'/'	';'	'P'	'0'
	{0xe2, 0xba, 0xc0, 0xbd},	//	'_'	':'	'@'	'-'
	{0x20, 0xdd, 0xdb, 0xde},	//	SPACE	']'	'['	'^'
	{0x00, 0x0d, 0x2e, 0xdc},	//		RETURN	DEL	'\'
};

void MEMORY::initialize()
{
	// initialize memory
	memset(ram, 0, sizeof(ram));
	memset(basic, 0xff, sizeof(basic));
	memset(printer, 0xff, sizeof(printer));
	memset(monitor, 0xff, sizeof(monitor));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(basic, sizeof(basic), 1);
		fio->Fclose();
	} else if(fio->Fopen(create_local_path(_T("BAS.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(basic, sizeof(basic), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("PRINTER.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(printer, sizeof(printer), 1);
		fio->Fclose();
	} else if(fio->Fopen(create_local_path(_T("PRT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(printer, sizeof(printer), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("MONITOR.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(monitor, sizeof(monitor), 1);
		fio->Fclose();
	} else if(fio->Fopen(create_local_path(_T("MON.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(monitor, sizeof(monitor), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	}
	delete fio;
	
	SET_BANK(0x0000, 0xafff, ram,  ram);
	memory_bank = 0;
	update_bank();
	
	// initialize inputs
	key_stat = emu->get_key_buffer();
	
	// initialize display
	for(int i = 0; i < 8; i++) {
		palette_pc[i] = RGB_COLOR((i & 2) ? 0xff : 0, (i & 4) ? 0xff : 0, (i & 1) ? 0xff : 0);
	}
	
	// register event
	register_frame_event(this);
}

void MEMORY::reset()
{
	touch_sound();
	memory_bank = 0;
	update_bank();
	
	memset(color_table, 7, sizeof(color_table));
	char_color = 7;
	back_color = mp1710_enb = 0;
	
	screen_mode = 0;
	screen_reversed = false;
	
	drec_bit = drec_in = false;
	
	key_column = 0;
	nmi_enb = break_pressed = false;
	
	sound_sample = 0;
	sound_accum = 0;
	sound_clock = sound_mix_clock = get_current_clock();
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	if((addr & 0xfe00) == 0xee00) {
		// EE00h - EFFFh
		switch(addr & 0xffff) {
		case 0xee40:
			screen_reversed = ((data & 0x80) != 0);
			break;
		case 0xee80:
			if(sound_sample != ((data >> 1) & 0x1f)) {
				touch_sound();
				sound_accum += (double)sound_sample * get_passed_usec(sound_clock);
				sound_clock = get_current_clock();
				sound_sample = (data >> 1) & 0x1f;
			}
			d_drec->write_signal(SIG_DATAREC_MIC, ~data, 0x01);
			break;
		case 0xeec0:
			key_column = data & 0x0f;
			nmi_enb = ((data & 0x80) != 0);
			event_frame(); // update keyboard
			break;
		case 0xefd0:
			// bit4: unknown (timer on/off)
			memory_bank = data;
			update_bank();
			break;
		case 0xefe0:
			screen_mode = data;
			break;
		}
		return;
	}
	if((addr & 0xf800) == 0xe800 && !(memory_bank & 2)) {
		// E800h - EFFFh
		switch(addr & 0xffff) {
		case 0xe800:
		case 0xe801:
		case 0xe802:
		case 0xe803:
			d_pia->write_io8(addr, data);
			break;
		case 0xe890:
			// bit0-3: fore color
			// bit4-7: back color
			char_color = data;
			break;
		case 0xe891:
			back_color = data;
			break;
		case 0xe892:
			mp1710_enb = data;
			break;
		}
		return;
	}
	if(addr >= 0x100 && addr < 0x400) {
		color_table[addr - 0x100] = char_color;
	}
	wbank[(addr >> 11) & 0x1f][addr & 0x7ff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	if((addr & 0xfe00) == 0xee00) {
		// EE00h - EFFFh
		switch(addr & 0xffff) {
		case 0xee00:
		case 0xee20:
			d_drec->write_signal(SIG_DATAREC_REMOTE, addr, 0x20);
			return 0x01;
		case 0xee80:
			return drec_bit ? 0x80 : 0;
		case 0xeec0:
			return key_data;
		case 0xef00:
			// unknown (timer)
			break;
		case 0xef80:
			if(break_pressed) {
				break_pressed = false;
				return 0x80;
			}
			return 0x00;
		case 0xefd0:
			return memory_bank;
		}
		return 0xff;
	}
	if((addr & 0xf800) == 0xe800 && !(memory_bank & 2)) {
		// E800h - EFFFh
		switch(addr & 0xffff) {
		case 0xe800:
		case 0xe801:
		case 0xe802:
		case 0xe803:
			return d_pia->read_io8(addr);
		case 0xe890:
			return char_color;
		case 0xe891:
			return back_color;
		case 0xe892:
			return mp1710_enb;
		}
		return 0xff;
	}
	return rbank[(addr >> 11) & 0x1f][addr & 0x7ff];
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMORY_DATAREC_EAR) {
		bool new_in = ((data & mask) != 0);
		if(!drec_in && new_in) {
			// L -> H
			drec_clock = get_current_clock();
		} else if(drec_in && !new_in) {
			// H -> L
			int usec = (int)get_passed_usec(drec_clock);
			if(usec > 417 - 42 && usec < 417 + 42) {
				drec_bit = false;	// 1200Hz
			} else if(usec > 208 - 21 && usec < 208 + 21) {
				drec_bit = true;	// 2400Hz
			}
		}
		drec_in = new_in;
	}
}

void MEMORY::event_frame()
{
	key_data = 0xff;
	if(key_column < 13) {
		if(key_stat[key_table[key_column][0]]) key_data &= ~0x01;
		if(key_stat[key_table[key_column][1]]) key_data &= ~0x02;
		if(key_stat[key_table[key_column][2]]) key_data &= ~0x04;
		if(key_stat[key_table[key_column][3]]) key_data &= ~0x08;
	}
#if defined(_USE_QT)
	// If same as bm2, not effect below keys at Qt version.
	if(key_stat[VK_LCONTROL]) key_data &= ~0x10; // 英数     -> LCTRL
	if(key_stat[VK_LSHIFT  ]) key_data &= ~0x20; // 英記号   -> L-SHIFT
	if(key_stat[VK_RWIN    ]) key_data &= ~0x40; // カナ記号 -> R-Win
	if(key_stat[VK_KANA    ]) key_data &= ~0x80; // カナ     -> カタカナひらがな
#else
	// this is same as "日立ベーシックマスターJr.(MB-6885)エミュレータ bm2"
	if(key_stat[0xa2]) key_data &= ~0x10; // 英数     -> L-CTRL
	if(key_stat[0xa0]) key_data &= ~0x20; // 英記号   -> L-SHIFT
	if(key_stat[0xa1]) key_data &= ~0x40; // カナ記号 -> R-SHIFT
	if(key_stat[0xa3]) key_data &= ~0x80; // カナ     -> R-CTRL
#endif
}

void MEMORY::key_down(int code)
{
	// pause -> break
	if(code == 0x13) {
		if(nmi_enb) {
			d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
		}
		break_pressed = true;
	}
}

void MEMORY::update_bank()
{
	if(memory_bank & 1) {
		SET_BANK(0xb000, 0xdfff, ram + 0xb000, ram + 0xb000);
	} else {
		SET_BANK(0xb000, 0xdfff, wdmy, basic);
	}
	if(memory_bank & 2) {
		SET_BANK(0xe000, 0xefff, ram + 0xe000, ram + 0xe000);
	} else {
		SET_BANK(0xe000, 0xe7ff, wdmy, printer);
		SET_BANK(0xe800, 0xefff, wdmy, rdmy);	// memory mapped i/o
	}
	if(memory_bank & 4) {
		SET_BANK(0xf000, 0xffff, ram + 0xf000, ram + 0xf000);
	} else {
		SET_BANK(0xf000, 0xffff, wdmy, monitor);
	}
}

void MEMORY::mix(int32_t* buffer, int cnt)
{
	int32_t volume = 0;
	if(get_passed_clock(sound_mix_clock) != 0) {
		sound_accum += (double)sound_sample * get_passed_usec(sound_clock);
		volume = (int32_t)(SOUND_VOLUME * sound_accum / (31.0 * get_passed_usec(sound_mix_clock)));
	}
	sound_accum = 0;
	sound_clock = sound_mix_clock = get_current_clock();
	
	int32_t vol_l = apply_volume(volume, volume_l);
	int32_t vol_r = apply_volume(volume, volume_r);
	
	for(int i = 0; i < cnt; i++) {
		*buffer++ += vol_l; // L
		*buffer++ += vol_r; // R
	}
}

void MEMORY::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
}

void MEMORY::draw_screen()
{
	if(!(screen_mode & 0x80)) {
		// text
		scrntype_t fore = palette_pc[screen_reversed ? 0 : 7];
		scrntype_t back = palette_pc[screen_reversed ? 7 : 0];
		int taddr = 0x100;
		
		for(int y = 0, yy = 0; y < 24; y++, yy += 8) {
			for(int x = 0, xx = 0; x < 32; x++, xx += 8) {
				if(mp1710_enb & 1) {
					uint8_t color = color_table[taddr];
					if(screen_reversed) {
						color = (color >> 4) | (color << 4);
					}
					fore = palette_pc[(color     ) & 7];
					back = palette_pc[(color >> 4) & 7];
				}
				int code = ram[taddr] << 3;
				for(int l = 0; l < 8; l++) {
					scrntype_t* dest = emu->get_screen_buffer(yy + l) + xx;
					uint8_t pat = font[code + l];
					dest[0] = (pat & 0x80) ? fore : back;
					dest[1] = (pat & 0x40) ? fore : back;
					dest[2] = (pat & 0x20) ? fore : back;
					dest[3] = (pat & 0x10) ? fore : back;
					dest[4] = (pat & 0x08) ? fore : back;
					dest[5] = (pat & 0x04) ? fore : back;
					dest[6] = (pat & 0x02) ? fore : back;
					dest[7] = (pat & 0x01) ? fore : back;
				}
				taddr++;
			}
		}
	} else {
		// graph
		scrntype_t fore = palette_pc[screen_reversed ? 0 : 7];
		scrntype_t back = palette_pc[screen_reversed ? 7 : 0];
		int taddr = 0x100;
		int gaddr = 0x900 + ((screen_mode & 0x0f) << 9);
		
		for(int y = 0, yy = 0; y < 24; y++, yy += 8) {
			for(int x = 0, xx = 0; x < 32; x++, xx += 8) {
				if(mp1710_enb & 1) {
					uint8_t color = color_table[taddr];
					if(screen_reversed) {
						color = (color >> 4) | (color << 4);
					}
					fore = palette_pc[(color     ) & 7];
					back = palette_pc[(color >> 4) & 7];
				}
				for(int l = 0, ll = 0; l < 8; l++, ll += 32) {
					scrntype_t* dest = emu->get_screen_buffer(yy + l) + xx;
					uint8_t pat = ram[gaddr + ll];
					dest[0] = (pat & 0x80) ? fore : back;
					dest[1] = (pat & 0x40) ? fore : back;
					dest[2] = (pat & 0x20) ? fore : back;
					dest[3] = (pat & 0x10) ? fore : back;
					dest[4] = (pat & 0x08) ? fore : back;
					dest[5] = (pat & 0x04) ? fore : back;
					dest[6] = (pat & 0x02) ? fore : back;
					dest[7] = (pat & 0x01) ? fore : back;
				}
				taddr++;
				gaddr++;
			}
			gaddr += 32 * 7;
		}
	}
//	emu->screen_skip_line(false);
}

#define STATE_VERSION	2

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateValue(memory_bank);
	state_fio->StateArray(color_table, sizeof(color_table), 1);
	state_fio->StateValue(char_color);
	state_fio->StateValue(back_color);
	state_fio->StateValue(mp1710_enb);
	state_fio->StateValue(screen_mode);
	state_fio->StateValue(screen_reversed);
	state_fio->StateValue(drec_bit);
	state_fio->StateValue(drec_in);
	state_fio->StateValue(drec_clock);
	state_fio->StateValue(key_column);
	state_fio->StateValue(key_data);
	state_fio->StateValue(nmi_enb);
	state_fio->StateValue(break_pressed);
	state_fio->StateValue(sound_sample);
	state_fio->StateValue(sound_accum);
	state_fio->StateValue(sound_clock);
	state_fio->StateValue(sound_mix_clock);
	
	// post process
	if(loading) {
		update_bank();
	}
	return true;
}

