/*
	SHARP SM-B-80TE Emulator 'eSM-B-80TE'

	Author : Takeda.Toshiya
	Date   : 2016.12.29-

	[ memory ]
*/

#include "memory.h"
#include "../z80pio.h"

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

void MEMORY::initialize()
{
	// initialize memory
	memset(ram, 0, sizeof(ram));
	memset(mon, 0xff, sizeof(mon));
	memset(user, 0xff, sizeof(user));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("MON.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(mon, sizeof(mon), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("USER.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(user, sizeof(user), 1);
		fio->Fclose();
	}
	delete fio;
	
	SET_BANK(0x0000, 0x0fff, ram,  ram );
	SET_BANK(0x1000, 0x7fff, wdmy, rdmy);
	SET_BANK(0x8000, 0x87ff, wdmy, mon );
	SET_BANK(0x8800, 0x8fff, wdmy, user);
	SET_BANK(0x9000, 0xffff, wdmy, rdmy);
	
	pio1_pa = pio1_pb = 0;
	
	register_vline_event(this);
}

void MEMORY::reset()
{
	if(config.dipswitch & 1) {
		a15_mask = 0x8000;
	} else {
		a15_mask = 0x0000;
	}
	shift_reg = 0;
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	addr |= a15_mask;
	wbank[(addr >> 11) & 0x1f][addr & 0x7ff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	addr |= a15_mask;
	return rbank[(addr >> 11) & 0x1f][addr & 0x7ff];
}

uint32_t MEMORY::fetch_op(uint32_t addr, int *wait)
{
	switch(shift_reg <<= 1) {
	case 0x08: // QD
		d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
		break;
	case 0x10: // QE
		a15_mask = 0x8000;
		break;
	}
	*wait = 0;
	return read_data8(addr);
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xd8:
		// ARES
		a15_mask = 0x0000;
		break;
	case 0xd9:
		// STEP
		shift_reg = 0x01;
		break;
	case 0xda:
		// LED on
		led = true;
		break;
	case 0xdb:
		// LED off
		led = false;
		break;
	}
}

uint32_t MEMORY::read_io8(uint32_t addr)
{
	return 0xff;
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_MEMORY_PIO1_PA:
		pio1_pa = data & mask;
		break;
	case SIG_MEMORY_PIO1_PB:
		pio1_pb = data & mask;
		update_kb();
		break;
	}
}

void MEMORY::event_vline(int v, int clock)
{
	if(!v) {
		update_kb();
		memset(seg, 0, sizeof(seg));
	}
	for(int i = 0; i < 8; i++) {
		if(((pio1_pb >> 3) & 7) == i) {
			for(int j = 0; j < 8; j++) {
				if(!(pio1_pa & (1 << j))) {
					seg[i][j]++;
				}
			}
		}
	}
}

void MEMORY::update_kb()
{
	const uint8_t* key_stat = emu->get_key_buffer();
	uint8_t val = 0xff;
	
	switch((pio1_pb >> 3) & 7) {
	case 0:
		if(key_stat[0x80] || key_stat[0x30] || key_stat[0x60]) val &= ~0x01;	// 0
		if(key_stat[0x81] || key_stat[0x31] || key_stat[0x61]) val &= ~0x02;	// 1
		if(key_stat[0x82] || key_stat[0x32] || key_stat[0x62]) val &= ~0x04;	// 2
		break;
	case 1:
		if(key_stat[0x83] || key_stat[0x33] || key_stat[0x63]) val &= ~0x01;	// 3
		if(key_stat[0x84] || key_stat[0x34] || key_stat[0x64]) val &= ~0x02;	// 4
		if(key_stat[0x85] || key_stat[0x35] || key_stat[0x65]) val &= ~0x04;	// 5
		break;
	case 2:
		if(key_stat[0x86] || key_stat[0x36] || key_stat[0x66]) val &= ~0x01;	// 6
		if(key_stat[0x87] || key_stat[0x37] || key_stat[0x67]) val &= ~0x02;	// 7
		if(key_stat[0x88] || key_stat[0x38] || key_stat[0x68]) val &= ~0x04;	// 8
		break;
	case 3:
		if(key_stat[0x89] || key_stat[0x39] || key_stat[0x69]) val &= ~0x01;	// 9
		if(key_stat[0x8a] || key_stat[0x41]                  ) val &= ~0x02;	// A
		if(key_stat[0x8b] || key_stat[0x42]                  ) val &= ~0x04;	// B
		break;
	case 4:
		if(key_stat[0x8c] || key_stat[0x43]                  ) val &= ~0x01;	// C
		if(key_stat[0x8d] || key_stat[0x44]                  ) val &= ~0x02;	// D
		if(key_stat[0x8e] || key_stat[0x45]                  ) val &= ~0x04;	// E
		break;
	case 5:
		if(key_stat[0x8f] || key_stat[0x46]                  ) val &= ~0x01;	// F
		if(key_stat[0x9d] || key_stat[0x75]                  ) val &= ~0x02;	// REG'/REG ... F6
		if(key_stat[0x9a] || key_stat[0x72] || key_stat[0x21]) val &= ~0x04;	// LD/INC   ... F3 or PgUp
		break;
	case 6:
		if(key_stat[0x9b] || key_stat[0x73] || key_stat[0x22]) val &= ~0x01;	// STOR/DEC ... F4 or PgDn
		if(key_stat[0x98] || key_stat[0x70]                  ) val &= ~0x02;	// RUN      ... F1
		if(key_stat[0x99] || key_stat[0x71]                  ) val &= ~0x04;	// STEP     ... F2
		break;
	case 7:
		if(key_stat[0x9f] || key_stat[0x77] || key_stat[0x0d]) val &= ~0x01;	// WR       ... F8 or Enter
		if(key_stat[0x9e] || key_stat[0x76]                  ) val &= ~0x02;	// ADRS     ... F7
		if(key_stat[0x9c] || key_stat[0x74] || key_stat[0x10]) val &= ~0x04;	// SHIFT    ... F5 or Shift
		break;
	}
	d_pio1->write_signal(SIG_Z80PIO_PORT_B, val, 0x07);




}

static const int led_pattern[LED_HEIGHT][LED_WIDTH] = {
	{0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,2,0,0},
	{0,0,6,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,2,2,0,0},
	{0,0,6,6,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,2,2,2,0,0},
	{0,0,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0},
	{0,0,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0},
	{0,0,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0},
	{0,0,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0},
	{0,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0,0},
	{0,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0,0},
	{0,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,0,0,0},
	{0,0,6,0,7,7,7,7,7,7,7,7,7,7,7,7,7,7,0,2,0,0,0,0},
	{0,0,0,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,0,0,0,0,0},
	{0,0,5,0,7,7,7,7,7,7,7,7,7,7,7,7,7,7,0,3,0,0,0,0},
	{0,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,0,0,0},
	{0,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,0,0,0},
	{0,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,0,0,0},
	{5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,0,0,0,0},
	{5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,0,0,0,0},
	{5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,0,0,0,0},
	{5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,0,0,0,0},
	{5,5,0,4,4,4,4,4,4,4,4,4,4,4,4,4,0,3,3,3,0,0,0,0},
	{5,0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,3,3,0,8,8,8},
	{0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,3,0,8,8,8},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8,8},
};

void MEMORY::draw_screen()
{
	scrntype_t color_on = RGB_COLOR(255, 8, 72);
	scrntype_t color_off = RGB_COLOR(56, 0, 0);
	scrntype_t color_led[9];
	uint8_t* base = ram + (~(config.dipswitch >> 2) & 3) * 0x200;
	color_led[0] = RGB_COLOR(37, 21, 22);
	
	for(int i = 0; i < 8; i++) {
		for(int b = 0; b < 8; b++) {
			color_led[b + 1] = (seg[i][b] > 8) ? color_on : color_off;
		}
		for(int y = 0; y < LED_HEIGHT; y++) {
			scrntype_t* dest = emu->get_screen_buffer(vm_ranges[i].y + y) + vm_ranges[i].x;
			for(int x = 0; x < LED_WIDTH; x++) {
				dest[x] = color_led[led_pattern[y][x]];
			}
		}
	}
}

void MEMORY::load_ram(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	delete fio;
}

void MEMORY::save_ram(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	
	if(fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
		fio->Fwrite(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	delete fio;
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
	state_fio->StateValue(pio1_pa);
	state_fio->StateValue(pio1_pb);
	state_fio->StateValue(shift_reg);
	state_fio->StateValue(a15_mask);
	state_fio->StateValue(led);
	return true;
}

