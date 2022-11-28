/*
	Yuasa Kyouiku System YALKY Emulator 'eYALKY'

	Author : Takeda.Toshiya
	Date   : 2016.03.28-

	[ i/o ]
*/

#include "io.h"
#include "../datarec.h"
#include "../i8080.h"
#include "../i8155.h"

#define EVENT_COUNTER	0

/*
SID 	<- DATA RECORDER SIGNAL (NEGATIVE, L H L H L L H H ...)
RST5	<- DATA RECORDER SIGNAL (POSITIVE, H L H L H H L L ...)
RST7	<- I8156 TIMER OUT (3MHz ???, OSC = 6MHz)

PA0	<- KEYBOARD DATA
PA1	<- KEYBOARD DATA
PA2	<- KEYBOARD DATA
PA3	<- KEYBOARD DATA
PA4	<- KEYBOARD DATA
PA5	<- DATA RECORDER SIGNAL (0 = NO SIGNAL)
PA6	<- COUNTER (4520) QB2
PA7	<- MODEL TYPE ??? (H = PM-1001)

PB0	-> 7948 Pin #6
PB1	-> FONT ROM (2364) A12, FONT BANK SWITCH
PB2	-> 7945 Pin #1
PB3	-> LED
PB4	-> COUNTER (4520) RESET B
PB5	-> DATA RECORDER H = FAST FORWARD or REC ???
PB6	-> DATA RECORDER H = PLAY, L = STOP
PB7	-> DATA RECORDER H = FAST REWIND

PC0	-> KEYBOARD COLUMN
PC1	-> KEYBOARD COLUMN
PC2	-> KEYBOARD COLUMN
PC3	-> ???
PC4	-> ???

	PA0	PA1	PA2	PA3	PA4
PC0	0	1	2	3	4
PC1	5	6	7	8	9
PC2	POINT	CLEAR	MARK	BACK	SET
*/

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
	prev_clock = 0;
	
	register_frame_event(this);
	register_id = -1;
	adjust_event_timing();
}

void IO::reset()
{
	div_counter = counter = 0;
	posi_counter = nega_counter = 0;
	drec_in = drec_toggle = false;
}

void IO::adjust_event_timing()
{
	if(register_id != -1) {
		cancel_event(this, register_id);
	}
	double freq = d_drec->get_ave_hi_freq() * 1.4;
	register_event(this, EVENT_COUNTER, 1000000.0 / freq, true, &register_id);
}

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
		{
			uint8_t prev = pb;
			pb &= ~mask;
			pb |= data & mask;
			
			if((prev & 0x10) && !(pb & 0x10)) {
				adjust_event_timing();
				counter = 1;//0;
				update_counter();
			}
			if(!(prev & 0x80) && (pb & 0x80)) {
				d_drec->set_ff_rew(-1);
			} else if((prev & 0x80) && !(pb & 0x80)) {
				d_drec->set_ff_rew(0);
			}
			if(!(prev & 0x40) && (pb & 0x40)) {
				d_drec->set_remote(true);
			} else if((prev & 0x40) && !(pb & 0x40)) {
				d_drec->set_remote(false);
			}
		}
		break;
		
	case SIG_IO_PORT_C:
		{
			uint8_t prev = pc;
			pc &= ~mask;
			pc |= data & mask;
			update_key();
		}
		break;
		
	case SIG_IO_DREC_EAR:
		{
			bool prev_in = drec_in;
			bool prev_toggle = drec_toggle;
			
			drec_in = ((data & mask) != 0);
			
			if(drec_in) {
				posi_counter++;
			} else {
				nega_counter++;
			}
			if(!prev_in && drec_in) {
				if(prev_clock == 0 || get_passed_usec(prev_clock) > 1000000.0 / d_drec->get_ave_hi_freq() * 6) {
					div_counter = 0;
					drec_toggle = false;
				}
				prev_clock = get_current_clock();
				
				if(++div_counter & 1) {
					drec_toggle = !drec_toggle;
				}
			}
			if(!prev_toggle && drec_toggle) {
				if(counter >= 8) {
					adjust_event_timing();
					counter = 1;
					update_counter();
				}
			}
			d_cpu->write_signal(SIG_I8085_SID, !drec_toggle ? 0xffffffff : 0, 1);	// negative
			d_cpu->write_signal(SIG_I8085_RST5, drec_toggle ? 0xffffffff : 0, 1);	// positive
		}
		break;
	}
}

void IO::event_callback(int event_id, int err)
{
	update_counter();
	counter++;
}

void IO::update_counter()
{
	d_pio->write_signal(SIG_I8155_PORT_A, (counter >= 4) ? 0xffffffff : 0, 0x40);
}

void IO::event_frame()
{
	if((posi_counter + nega_counter) > ((pb & 0x80) ? DATAREC_FAST_REW_SPEED : 1) * 10) {
		int ratio = (100 * posi_counter) / (posi_counter + nega_counter);
		posi_counter = nega_counter = 0;
		d_pio->write_signal(SIG_I8155_PORT_A, (ratio > 40 && ratio < 60) ? 0xffffffff : 0, 0x20);
	} else {
		d_pio->write_signal(SIG_I8155_PORT_A, 0, 0x20);
	}
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
		if(key_stat[0x43] || key_stat[0x08]) value &= ~0x02;	// CLEAR -> C or BACK SPACE
		if(key_stat[0x4d] || key_stat[0x20]) value &= ~0x04;	// MARK  -> M or SPACE
		if(key_stat[0x42] || key_stat[0x1b]) value &= ~0x08;	// BACK  -> B or ESC
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
//			uint8_t attr = vram[x + y * 32 + 512];
			
			for(int l = 0; l < 16; l++) {
				scrntype_t* dst = emu->get_screen_buffer(y * 16 + l) + x * 8;
				uint8_t pattern = font[(code * 16 + l) | ((pb & 2) ? 0x1000 : 0)];
				
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

#define STATE_VERSION	2

bool IO::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(pb);
	state_fio->StateValue(pc);
	state_fio->StateValue(div_counter);
	state_fio->StateValue(counter);
	state_fio->StateValue(posi_counter);
	state_fio->StateValue(nega_counter);
	state_fio->StateValue(drec_in);
	state_fio->StateValue(drec_toggle);
	state_fio->StateValue(prev_clock);
	state_fio->StateValue(register_id);
	return true;
}

