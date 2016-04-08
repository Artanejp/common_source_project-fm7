/*
	Yuasa Kyouiku System YALKY Emulator 'eYALKY'

	Author : Takeda.Toshiya
	Date   : 2016.03.28-

	[ i/o ]
*/

#include "io.h"
#include "../i8155.h"

void IO::initialize()
{
	// load font rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	}
	delete fio;
	
	key_stat = emu->get_key_buffer();
	pb = pc = 0xff;
	counter = 0;
	register_frame_event(this);
}

/*
PA0 <- KEYBOARD DATA
PA1 <- KEYBOARD DATA
PA2 <- KEYBOARD DATA
PA3 <- KEYBOARD DATA
PA4 <- KEYBOARD DATA
PA5 <- DATA RECORDER
PA6 <- 4520 QB2
PA7 <- +5V

PB0 -> 7948 Pin #6
PB1 -> FONT ROM (2364) A12
PB2 -> 7945 Pin #1
PB3 -> LED
PB4 -> 4520 ENABLE B
PB5 -> DATA RECORDER
PB6 -> DATA RECORDER
PB7 -> DATA RECORDER

PC0 -> KEYBOARD COLUMN
PC1 -> KEYBOARD COLUMN
PC2 -> KEYBOARD COLUMN
PC3
PC4

	PA0	PA1	PA2	PA3	PA4
PC0	0	1	2	3	4
PC1	5	6	7	8	9
PC2	POINT	CLEAR	MARK	BACK	SET
*/

void IO::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
		d_pio->write_io8(addr, data);
		break;
	}
}

uint32_t IO::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
		return d_pio->read_io8(addr);
	}
	return 0xff;
}

void IO::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_IO_PORT_B:
		pb &= ~mask;
		pb |= data & mask;
		break;
		
	case SIG_IO_PORT_C:
		pc &= ~mask;
		pc |= data & mask;
		update_key();
		break;
		
	case SIG_IO_DREC_ROT:
		d_pio->write_signal(SIG_I8155_PORT_A, !(data & mask) ? 0xffffffff : 0, 0x20);
		break;
	}
}

void IO::event_frame()
{
//	if(!(pb & 0x10)) {
		counter++;
//	}
	d_pio->write_signal(SIG_I8155_PORT_A, (counter & 4) ? 0xffffffff : 0, 0x40);
	update_key();
}

void IO::update_key()
{
	uint8_t value = 0xff;
	
	if(!(pc & 0x01)) {
		if(key_stat[0x30] || key_stat[0x60]) value &= ~0x01;	// 0
		if(key_stat[0x31] || key_stat[0x61]) value &= ~0x02;	// 1
		if(key_stat[0x32] || key_stat[0x62]) value &= ~0x04;	// 2
		if(key_stat[0x33] || key_stat[0x63]) value &= ~0x08;	// 3
		if(key_stat[0x34] || key_stat[0x64]) value &= ~0x10;	// 4
	}
	if(!(pc & 0x02)) {
		if(key_stat[0x35] || key_stat[0x65]) value &= ~0x01;	// 5
		if(key_stat[0x36] || key_stat[0x66]) value &= ~0x02;	// 6
		if(key_stat[0x37] || key_stat[0x67]) value &= ~0x04;	// 7
		if(key_stat[0x38] || key_stat[0x68]) value &= ~0x08;	// 8
		if(key_stat[0x39] || key_stat[0x69]) value &= ~0x10;	// 9
	}
	if(!(pc & 0x04)) {
		if(key_stat[0xbe] || key_stat[0x6e]) value &= ~0x01;	// DECIMAL POINT
		if(key_stat[0x43] || key_stat[0x1b]) value &= ~0x02;	// CLEAR -> C or ESC
		if(key_stat[0x4d] || key_stat[0x20]) value &= ~0x04;	// MARK  -> M or SPACE
		if(key_stat[0x42] || key_stat[0x08]) value &= ~0x08;	// BACK  -> B or BACK SPACE
		if(key_stat[0x53] || key_stat[0x0d]) value &= ~0x10;	// SET   -> S or ENTER
	}
	d_pio->write_signal(SIG_I8155_PORT_A, value, 0x1f);
}

void IO::draw_screen()
{
	scrntype_t cd = RGB_COLOR(0, 255, 0);
	scrntype_t cb = RGB_COLOR(0, 0, 0);
	
	for(int y = 0; y < 16; y++) {
		for(int x = 0; x < 32; x++) {
			uint8_t code = vram[x + y * 32];
			uint8_t attr = vram[x + y * 32 + 512];
			
			for(int l = 0; l < 16; l++) {
				scrntype_t* dst = emu->get_screen_buffer(y * 16 + l) + x * 8;
				uint8_t pattern = font[code * 16 + l];
				
				dst[0] = (pattern & 0x80) ? cd : cb;
				dst[1] = (pattern & 0x40) ? cd : cb;
				dst[2] = (pattern & 0x20) ? cd : cb;
				dst[3] = (pattern & 0x10) ? cd : cb;
				dst[4] = (pattern & 0x08) ? cd : cb;
				dst[5] = (pattern & 0x04) ? cd : cb;
				dst[6] = (pattern & 0x02) ? cd : cb;
				dst[7] = (pattern & 0x01) ? cd : cb;
			}
		}
	}
}

#define STATE_VERSION	1

void IO::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint8(pb);
	state_fio->FputUint8(pc);
	state_fio->FputUint8(counter);
}

bool IO::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	pb = state_fio->FgetUint8();
	pc = state_fio->FgetUint8();
	counter = state_fio->FgetUint8();
	return true;
}

