/*
	National JR-100 Emulator 'eJR-100'

	Author : Takeda.Toshiya
	Date   : 2015.08.27-

	[ memory bus ]
*/

#include "memory.h"
#include "../sy6522.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 13, eb = (e) >> 13; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x2000 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x2000 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	// initialize memory
	memset(ram, 0, sizeof(ram));
	memset(vram, 0, sizeof(vram));
	memset(rom, 0xff, sizeof(rom));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	} else if(fio->Fopen(create_local_path(_T("JR100.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;
	
	SET_BANK(0x0000, 0x7fff, ram,  ram );
	SET_BANK(0x8000, 0xdfff, wdmy, rdmy);
	SET_BANK(0xe000, 0xffff, wdmy, rom );
	
	// initialize inputs
	key_stat = emu->get_key_buffer();
	joy_stat = emu->get_joy_buffer();
	
	// initialize display
	palette_pc[0] = RGB_COLOR(0, 0, 0);
	palette_pc[1] = RGB_COLOR(255, 255, 255);
	
	key_column = 0;
	cmode = false;
	
	// register event
	register_frame_event(this);
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	if((addr & 0xf000) == 0xc000) {
		switch(addr & 0xfc00) {
		case 0xc000:
			vram[addr & 0x3ff] = data;
			break;
		case 0xc800:
			d_via->write_io8(addr, data);
			break;
		}
		return;
	}
	wbank[(addr >> 13) & 7][addr & 0x1fff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	if((addr & 0xf000) == 0xc000) {
		switch(addr & 0xfc00) {
		case 0xc000:
			return vram[addr & 0x3ff];
		case 0xc800:
			return d_via->read_io8(addr);
		case 0xcc00:
			if((addr & 0xffff) == 0xcc02) {
				return ((joy_stat[0] & 0x08) >> 3) |	// bit0: right
				       ((joy_stat[0] & 0x04) >> 1) |	// bit1: left
				       ((joy_stat[0] & 0x01) << 2) |	// bit2: up
				       ((joy_stat[0] & 0x02) << 2) |	// bit3: down
				       ((joy_stat[0] & 0x10)     ) |	// bit4: switch
				       ((joy_stat[0] & 0x20) >> 1);
			}
			break;
		}
		return 0xff;
	}
	return rbank[(addr >> 13) & 7][addr & 0x1fff];
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMORY_VIA_PORT_A) {
		key_column = data & 0x0f;
		event_frame();
	} else if(id == SIG_MEMORY_VIA_PORT_B) {
		cmode = ((data & 0x20) != 0);
	}
}

static const int key_table[9][5] = {
	{0x11, 0x10, 0x5a, 0x58, 0x43},	//	CTRL	SHIFT	Z	X	C
	{0x41, 0x53, 0x44, 0x46, 0x47},	//	A	S	D	F	G
	{0x51, 0x57, 0x45, 0x52, 0x54},	//	Q	W	E	R	T
	{0x31, 0x32, 0x33, 0x34, 0x35},	//	1	2	3	4	5
	{0x36, 0x37, 0x38, 0x39, 0x30},	//	6	7	8	9	0
	{0x59, 0x55, 0x49, 0x4F, 0x50},	//	Y	U	I	O	P
	{0x48, 0x4a, 0x4b, 0x4c, 0xbb},	//	H	J	K	L	;
	{0x56, 0x42, 0x4e, 0x4d, 0xbc},	//	V	B	N	M	,
	{0xbe, 0x20, 0xba, 0x0d, 0xbd},	//	.	SPACE	:	RET	-
};

void MEMORY::event_frame()
{
	uint32_t data = 0;
	if(key_column < 9) {
		if(key_stat[key_table[key_column][0]]) data |= 0x01;
		if(key_stat[key_table[key_column][1]]) data |= 0x02;
		if(key_stat[key_table[key_column][2]]) data |= 0x04;
		if(key_stat[key_table[key_column][3]]) data |= 0x08;
		if(key_stat[key_table[key_column][4]]) data |= 0x10;
	}
	d_via->write_signal(SIG_SY6522_PORT_B, ~data, 0x1f);
}

void MEMORY::draw_screen()
{
	int src = 0x100;
	for(int y = 0, yy = 0; y < 24; y++, yy += 8) {
		for(int x = 0, xx = 0; x < 32; x++, xx += 8) {
			int code = (vram[src  ] & 0x7f) << 3;
			bool attr = ((vram[src++] & 0x80) != 0);
			
			for(int l = 0; l < 8; l++) {
				scrntype_t* dest = emu->get_screen_buffer(yy + l) + xx;
				uint8_t pat = (cmode && attr) ? vram[(code + l) & 0x3ff] : attr ? ~rom[code + l] : rom[code + l];
				dest[0] = palette_pc[(pat >> 7) & 1];
				dest[1] = palette_pc[(pat >> 6) & 1];
				dest[2] = palette_pc[(pat >> 5) & 1];
				dest[3] = palette_pc[(pat >> 4) & 1];
				dest[4] = palette_pc[(pat >> 3) & 1];
				dest[5] = palette_pc[(pat >> 2) & 1];
				dest[6] = palette_pc[(pat >> 1) & 1];
				dest[7] = palette_pc[(pat     ) & 1];
			}
		}
	}
//	emu->screen_skip_line(false);
}

#define STATE_VERSION	1

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateArray(vram, sizeof(vram), 1);
	state_fio->StateValue(key_column);
	state_fio->StateValue(cmode);
	return true;
}

