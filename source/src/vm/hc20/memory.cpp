/*
	EPSON HC-20 Emulator 'eHC-20'

	Author : Takeda.Toshiya
	Date   : 2011.05.23-

	[ memory ]
*/

#include "memory.h"
#include "../beep.h"
#include "../mc6800.h"
#include "../z80sio.h"
#include "../../fifo.h"

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

#define INT_KEYBOARD	1
#define INT_CLOCK	2
#define INT_POWER	4

#define EVENT_SOUND	0

static int key_table[8][10] = {
	// PAUSE=F6, MENU=F7, BREAK=F8 NUM=F9 CLR=F10 SCRN=F11 PRINT=PgUp PAPER=PgDn
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x70, 0x00,
	0x38, 0x39, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0x71, 0x00,
	0xc0, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x72, 0x00,
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x73, 0x00,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x74, 0x00,
	0x58, 0x59, 0x5a, 0xdb, 0xdd, 0xdc, 0x27, 0x25, 0x22, 0x10,
	0x0d, 0x20, 0x09, 0x00, 0x00, 0x78, 0x00, 0x34, 0x00, 0x11,
	0x79, 0x7a, 0x77, 0x75, 0x2e, 0x76, 0x00, 0x00, 0x00, 0x21
};

void MEMORY::initialize()
{
	// initialize memory
	memset(ram, 0, sizeof(ram));
	memset(rom, 0, sizeof(rom));
	memset(ext, 0, sizeof(ext));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load backuped ram / rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("DRAM.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(ram, sizeof(ram), 1);
		fio->Fclose();
	} else if(fio->Fopen(create_local_path(_T("BACKUP.BIN")), FILEIO_READ_BINARY)) {
		// for compatibility
		fio->Fread(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("EXT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ext, sizeof(ext), 1);
		fio->Fclose();
	}
	delete fio;
	
//	SET_BANK(0x0000, 0x3fff, ram, ram);
//	SET_BANK(0x4000, 0x7fff, wdmy, rdmy);
	SET_BANK(0x0000, 0x7fff, ram, ram);
	SET_BANK(0x8000, 0xffff, wdmy, rom);
	
	// init command buffer
	cmd_buf = new FIFO(512);
	memset(slave_mem, 0, sizeof(slave_mem));
	
	// init sound
	double tone_tmp[13];
	tone_tmp[9] = 110.0;
	for(int i = 8; i >= 0; i--) {
		tone_tmp[i] = tone_tmp[i + 1] / 1.05946;
	}
	for(int i = 10; i < 13; i++) {
		tone_tmp[i] = tone_tmp[i - 1] * 1.05946;
	}
	static const int tone_index[7] = {0, 2, 4, 5, 7, 9, 11};
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 7; j++) {
			tone_table[i * 7 + j +  1] = tone_tmp[tone_index[j]    ] * (2 << i);
			tone_table[i * 7 + j + 29] = tone_tmp[tone_index[j] + 1] * (2 << i);
		}
	}
	tone_table[0] = 0;
	
	// init keyboard
	memset(key_stat, 0, sizeof(key_stat));
	memset(key_flag, 0, sizeof(key_flag));
	
	for(int i = 0; i < 8; i++) {
		for(int j = 0; j < 10; j++) {
			key_flag[key_table[i][j]] = 1;
		}
	}
	key_flag[0] = key_flag[0x10] = key_flag[0x11] = key_flag[0x12] = 0;
	
	// init cmt
	cmt_count = 0;
	cmt_play = cmt_rec = false;
	cmt_fio = new FILEIO();
	
	// init lcd
	pd = RGB_COLOR(48, 56, 16);
	pb = RGB_COLOR(160, 168, 160);
	memset(lcd, 0, sizeof(lcd));
	
	// register event
	register_event_by_clock(this, EVENT_SOUND, 256, true, NULL);
}

void MEMORY::release()
{
	// save battery backuped ram
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("DRAM.BIN")), FILEIO_WRITE_BINARY)) {
		fio->Fwrite(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	delete fio;
	
	// release cmt
	close_tape();
	delete cmt_fio;
	
	// release command buffer
	cmd_buf->release();
	delete cmd_buf;
}

void MEMORY::reset()
{
	// select internal rom
//	SET_BANK(0x4000, 0x7fff, wdmy, rdmy);
	SET_BANK(0x8000, 0xbfff, wdmy, rom);
	
	cmd_buf->clear();
	sio_select = true;
	special_cmd_masked = true;
	
	sound_ptr = sound_count = 0;
	sound_freq = 0;
	
	key_strobe = 0xff;
	key_data = 0x3ff;
	key_intmask = 0;
	
	close_tape();
	
	lcd_select = 0;
	lcd_clock = 0;
	
	int_status = 0;
	int_mask = 0;
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	if(addr < 0x40) {
		switch(addr) {
		case 0x20:
			if(key_strobe != data) {
				key_strobe = data;
				update_keyboard();
			}
			break;
		case 0x26:
			lcd_select = data & 0x0f;
			key_intmask = data & 0x10;
			// interrupt mask reset in sleep mode
			if(int_mask) {
				int_mask = 0;
//				update_intr();
			}
			break;
		case 0x2a:
			lcd_data = data;
			lcd_clock = 8;
			break;
		case 0x2c:
			// used for interrupt mask setting in sleep mode
			if(!int_mask) {
				int_mask = 1;
//				update_intr();
			}
			break;
		case 0x30:
//			SET_BANK(0x4000, 0x7fff, ram + 0x4000, ram + 0x4000);
			SET_BANK(0x8000, 0xbfff, wdmy, ext);
			break;
		case 0x32:
		case 0x33:
//			SET_BANK(0x4000, 0x7fff, wdmy, rdmy);
			SET_BANK(0x8000, 0xbfff, wdmy, rom);
			break;
		}
		ram[addr] = data;
	} else if(addr < 0x80) {
		d_rtc->write_io8(1, addr & 0x3f);
		d_rtc->write_io8(0, data);
	} else {
		wbank[(addr >> 13) & 7][addr & 0x1fff] = data;
	}
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	if(addr < 0x40) {
		switch(addr) {
		case 0x20:
			return key_strobe;
		case 0x22:
			return key_data & 0xff;
		case 0x26:
			// interrupt mask reset in sleep mode
			if(int_mask) {
				int_mask = 0;
//				update_intr();
			}
			break;
		case 0x28:
			// bit6: power switch interrupt flag (0=active)
			// bit7: busy signal of lcd controller (0=busy)
			return ((key_data >> 8) & 3) | ((int_status & INT_POWER) ? 0 : 0x40) | 0xa8;
		case 0x2a:
		case 0x2b:
			if(lcd_clock > 0 && --lcd_clock <= 0) {
				int c = lcd_select & 7;
				if(c >= 1 && c <= 6) {
					lcd_t *block = &lcd[c - 1];
					if(lcd_select & 8) {
						block->bank = lcd_data & 0x40 ? 40 : 0;
						block->addr = lcd_data & 0x3f;
					} else if(block->addr < 40) {
						block->buffer[block->bank + block->addr] = lcd_data;
						block->addr++;
					}
				}
			}
			break;
		case 0x2c:
			// used for interrupt mask setting in sleep mode
			if(!int_mask) {
				int_mask = 1;
//				update_intr();
			}
			break;
		case 0x30:
//			SET_BANK(0x4000, 0x7fff, ram + 0x4000, ram + 0x4000);
			SET_BANK(0x8000, 0xbfff, ext, rom);
			break;
		case 0x32:
		case 0x33:
//			SET_BANK(0x4000, 0x7fff, wdmy, rdmy);
			SET_BANK(0x8000, 0xbfff, wdmy, rom);
			break;
		}
//		return ram[addr];
		return addr;
	} else if(addr < 0x80) {
		d_rtc->write_io8(1, addr & 0x3f);
		return d_rtc->read_io8(0);
	}
	return rbank[(addr >> 13) & 7][addr & 0x1fff];
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMORY_PORT_2) {
		sio_select = ((data & 0x04) != 0);
	} else if(id == SIG_MEMORY_SIO_MAIN) {
		if(!sio_select) {
			d_sio_tf20->write_signal(SIG_Z80SIO_RECV_CH0, data, 0xff);
		} else {
			send_to_slave(data & mask);
		}
	} else if(id == SIG_MEMORY_SIO_TF20) {
		if(!sio_select) {
			send_to_main(data & mask);
		}
	} else if(id == SIG_MEMORY_RTC_IRQ) {
		if(data & mask) {
			if(!(int_status & INT_CLOCK)) {
				int_status |= INT_CLOCK;
				update_intr();
			}
		} else {
			if((int_status & INT_CLOCK) && (int_status &= ~INT_CLOCK) == 0) {
				update_intr();
			}
		}
	}
}

void MEMORY::event_callback(int event_id, int err)
{
	if(event_id == EVENT_SOUND) {
		update_sound();
	}
}

void MEMORY::update_sound()
{
	if(sound_ptr < sound_count) {
		if(sound[sound_ptr].remain-- == 0) {
			if(++sound_ptr == sound_count) {
				d_beep->write_signal(SIG_BEEP_ON, 0, 0);
				send_to_main(sound_reply);
				return;
			}
			sound[sound_ptr].remain = sound[sound_ptr].period;
		}
		if(sound_freq != sound[sound_ptr].freq) {
			sound_freq = sound[sound_ptr].freq;
			if(sound_freq != 0) {
				d_beep->set_frequency(sound_freq);
				d_beep->write_signal(SIG_BEEP_ON, 1, 1);
			} else {
				d_beep->write_signal(SIG_BEEP_ON, 0, 0);
			}
		}
	}
}

void MEMORY::update_keyboard()
{
	key_data = 0x3ff;
	
	if(key_strobe == 0) {
		// clear key interrupt
		if((int_status & INT_KEYBOARD) && (int_status &= ~INT_KEYBOARD) == 0) {
			update_intr();
		}
		d_cpu->write_signal(SIG_MC6801_PORT_1, 0x20, 0x20);
		
		// clear key buffer except shift/ctrl/alt keys
		uint8_t key_stat_10 = key_stat[0x10];
		uint8_t key_stat_11 = key_stat[0x11];
		uint8_t key_stat_12 = key_stat[0x12];
		memset(key_stat, 0, sizeof(key_stat));
		key_stat[0x10] = key_stat_10;
		key_stat[0x11] = key_stat_11;
		key_stat[0x12] = key_stat_12;
	}
	for(int i = 0; i < 8; i++) {
		if(key_strobe & (1 << i)) {
			continue;
		}
		for(int j = 0; j < 10; j++) {
			if(key_stat[key_table[i][j]]) {
				key_data &= ~(1 << j);
			}
		}
		// dip-switch
		if(i < 4 && (config.dipswitch & (1 << i))) {
			key_data &= ~0x200;
		}
	}
}

void MEMORY::notify_power_off()
{
	int_status |= INT_POWER;
	update_intr();
}

void MEMORY::key_down(int code)
{
	key_stat[code] = 1;
	
	if(key_flag[code]) {
		// raise key interrupt
		if(!(int_status & INT_KEYBOARD)) {
			int_status |= INT_KEYBOARD;
			update_intr();
		}
		d_cpu->write_signal(SIG_MC6801_PORT_1, 0, 0x20);
	}
}

void MEMORY::key_up(int code)
{
	key_stat[code] = 0;
}

void MEMORY::update_intr()
{
//	d_cpu->write_signal(SIG_CPU_IRQ, (int_status && !int_mask) ? 1 : 0, 1);
	d_cpu->write_signal(SIG_CPU_IRQ, int_status ? 1 : 0, 1);
}

void MEMORY::send_to_slave(uint8_t val)
{
	cmd_buf->write(val);
	uint8_t cmd = cmd_buf->read_not_remove(0);
	
//	this->out_debug_log(_T("Command = %2x"), cmd);
//	for(int i = 1; i < cmd_buf->count(); i++) {
//		this->out_debug_log(_T(" %2x"), cmd_buf->read_not_remove(i));
//	}
//	this->out_debug_log(_T("\n"));
	
	switch(cmd) {
	case 0x00: // slave mcpu ready check
	case 0x01: // sets the constants required by slave mcu
	case 0x02: // initialization
		cmd_buf->read();
		send_to_main(0x01);
		break;
	case 0x03: // opens masks for special commands
		if(cmd_buf->count() == 2) {
			cmd_buf->read();
			special_cmd_masked = (cmd_buf->read() != 0xaa);
		}
		send_to_main(0x01);
		break;
	case 0x04: // closes masks for special commands
		special_cmd_masked = true;
		cmd_buf->read();
		send_to_main(0x01);
		break;
	case 0x05: // reads slave mcu memory
		if(special_cmd_masked) {
			cmd_buf->read();
			send_to_main(0x0f);
			break;
		}
		if(cmd_buf->count() == 3) {
			cmd_buf->read();
			int ofs = cmd_buf->read() << 8;
			ofs |= cmd_buf->read();
			send_to_main(slave_mem[ofs]);
			break;
		}
		send_to_main(0x01);
		break;
	case 0x06: // stores slave mcu memory
	case 0x07: // logical or operation
	case 0x08: // logical and operation
		if(special_cmd_masked) {
			cmd_buf->read();
			send_to_main(0x0f);
			break;
		}
		if(cmd_buf->count() == 4) {
			cmd_buf->read();
			int ofs = cmd_buf->read() << 8;
			ofs |= cmd_buf->read();
			if(cmd == 6) {
				slave_mem[ofs] = cmd_buf->read();
			} else if(cmd == 7) {
				slave_mem[ofs] |= cmd_buf->read();
			} else if(cmd == 8) {
				slave_mem[ofs] &= cmd_buf->read();
			}
		}
		send_to_main(0x01);
		break;
	case 0x09: // bar-code reader power on
	case 0x0a: // bar-code reader power off
		cmd_buf->read();
		send_to_main(0x01);
		break;
	case 0x0b: // sets the program counter to a specified value
		if(special_cmd_masked) {
			cmd_buf->read();
			send_to_main(0x0f);
			break;
		}
		if(cmd_buf->count() == 3) {
			cmd_buf->read();
			int ofs = cmd_buf->read() << 8;
			ofs |= cmd_buf->read();
			// todo: implements known routines
		}
		send_to_main(0x01);
		break;
	case 0x0c: // terminate process
		cmd_buf->read();
		send_to_main(0x02);
		// stop sound
		d_beep->write_signal(SIG_BEEP_ON, 0, 0);
		sound_ptr = sound_count;
		break;
	case 0x0d: // cuts off power supply
		if(cmd_buf->count() == 2) {
			cmd_buf->read();
			if(cmd_buf->read() == 0xaa) {
				emu->power_off();
				break;
			}
		}
		send_to_main(0x01);
		break;
	case 0x10: // prints out 6-dot data (bit0-5) to the built-in printer
	case 0x11: // feeds the specified number of dot lines to the built-in printer
		if(cmd_buf->count() == 2) {
			cmd_buf->clear();
		}
		send_to_main(0x01);
		break;
	case 0x12: // paper feed operation (1.2sec)
		cmd_buf->read();
		send_to_main(0x01);
		break;
	case 0x20: // executes external cassette ready check
		send_to_main(0x21);
		cmd_buf->read();
		break;
	case 0x21: // sets constants for the external cassette
		if(cmd_buf->count() == 1) {
			send_to_main(0x01);
			break;
		}
		if(cmd_buf->count() == 9) {
			cmd_buf->clear();
		}
		send_to_main(0x21);
		break;
	case 0x22: // turns the external cassette rem terminal on
	case 0x23: // turns the external cassette rem terminal off
		cmd_buf->read();
		send_to_main(0x01);
		break;
	case 0x24: // writes 1 block of data in EPSON format
		if(cmd_buf->count() == 1) {
			send_to_main(0x01);
			break;
		}
		if(cmd_buf->count() >= 5 && cmd_buf->count() == cmd_buf->read_not_remove(3) * 256 + cmd_buf->read_not_remove(4) + 5) {
			if(cmt_rec) {
				for(int i = 0; i < 5; i++) {
					cmd_buf->read();
				}
				while(!cmd_buf->empty()) {
					cmt_buffer[cmt_count++] = cmd_buf->read();
					if(cmt_count >= CMT_BUFFER_SIZE) {
						cmt_fio->Fwrite(cmt_buffer, cmt_count, 1);
						cmt_count = 0;
					}
				}
			} else {
				cmd_buf->clear();
			}
		}
		send_to_main(0x21);
		break;
	case 0x25: // outputs number of ff patterns
		if(cmd_buf->count() == 1) {
			send_to_main(0x01);
			break;
		}
		if(cmd_buf->count() == 3) {
			cmd_buf->clear();
		}
		send_to_main(0x21);
		break;
	case 0x26: // inputs files from the external cassette
	case 0x27: // inputs files from the external cassette
	case 0x28: // inputs files from the external cassette
		if(cmd_buf->count() == 1) {
			send_to_main(0x01);
			break;
		}
		if(cmd_buf->count() == 5) {
			int len = cmd_buf->read_not_remove(3) * 256 + cmd_buf->read_not_remove(4);
			cmd_buf->clear();
			send_to_main(0x21);
			for(int i = 0; i < len; i++) {
				send_to_main(cmt_buffer[cmt_count++]);
			}
			// ???
			send_to_main(0x01);
			break;
		}
		send_to_main(0x21);
		break;
	case 0x2b: // specifies the input signal for the external cassette
		if(cmd_buf->count() == 1) {
			send_to_main(0x01);
			break;
		}
		if(cmd_buf->count() == 2) {
			cmd_buf->clear();
		}
		send_to_main(0x21);
		break;
	case 0x30: // specifies the tone and duration and sounds the piezo speaker
		if(cmd_buf->count() == 1) {
			send_to_main(0x01);
			break;
		}
		if(cmd_buf->count() == 3) {
			cmd_buf->read();
			int tone = cmd_buf->read();
			int period = cmd_buf->read();
			if(tone >= 0 && tone <= 56 && period != 0) {
				sound[0].freq = tone_table[tone];
				sound[0].period = CPU_CLOCKS * period / 256 / 10;
				sound[0].remain = sound[0].period;
				sound_ptr = 0;
				sound_count = 1;
				sound_reply = 0x31;
				break;
			}
		}
		send_to_main(0x31);
		break;
	case 0x31: // specifies the frequency and duration and sounds the piezo speaker
		if(cmd_buf->count() == 1) {
			send_to_main(0x01);
			break;
		}
		if(cmd_buf->count() == 5) {
			cmd_buf->read();
			int freq = cmd_buf->read() << 8;
			freq |= cmd_buf->read();
			int period = cmd_buf->read() << 8;
			period |= cmd_buf->read();
			if(freq != 0 && period != 0) {
				sound[0].freq = CPU_CLOCKS / freq / 2.0;
				sound[0].period = period;
				sound[0].remain = sound[0].period;
				sound_ptr = 0;
				sound_count = 1;
				sound_reply = 0x31;
				break;
			}
		}
		send_to_main(0x31);
		break;
	case 0x32: // sounds the speaker for 0.03 sec at tone 6
	case 0x33: // sounds the speaker for 1 sec at tone 20
		cmd_buf->read();
		if(cmd == 0x32) {
			sound[0].freq = tone_table[6];
			sound[0].period = CPU_CLOCKS * 3 / 256 / 100;
		} else {
			sound[0].freq = tone_table[20];
			sound[0].period = CPU_CLOCKS / 256;
		}
		sound[0].remain = sound[0].period;
		sound_ptr = 0;
		sound_count = 1;
		sound_reply = 0x01;
		break;
	case 0x34: // sets melody data in the slave mcu
		if(cmd_buf->count() == 1) {
			send_to_main(0x01);
			break;
		}
		if(val == 0xff) {
			cmd_buf->read();
			sound_count = 0;
			while(!cmd_buf->empty()) {
				int tone = cmd_buf->read();
				int period = cmd_buf->read();
				if(tone >= 0 && tone <= 56 && period != 0) {
					sound[sound_count].freq = tone_table[tone];
					sound[sound_count].period = CPU_CLOCKS * period / 256 / 10;
					sound_count++;
				}
			}
			sound_ptr = sound_count;
		}
		send_to_main(0x31);
		break;
	case 0x35: // sounds the melody data specified in command 34
		if(sound_count) {
			sound[0].remain = sound[0].period;
			sound_ptr = 0;
			sound_reply = 0x01;
			break;
		}
		send_to_main(0x01);
		break;
	case 0x40: // turns the serial driver on
	case 0x41: // turns the serial driver off
		cmd_buf->read();
		send_to_main(0x01);
		break;
	case 0x48: // sets the polynomial expression used for CRC check
		if(cmd_buf->count() == 1) {
			send_to_main(0x01);
			break;
		}
		if(cmd_buf->count() == 3) {
			cmd_buf->clear();
		}
		send_to_main(0x41);
		break;
	case 0x50: // identifies the plug-in option
		cmd_buf->read();
		send_to_main(0x02);
		break;
	case 0x51: // turns power of plug-in rom cartridge on
	case 0x52: // turns power of plug-in rom cartridge off
		cmd_buf->read();
		send_to_main(0x01);
		break;
	case 0x60: // executes micro cassette ready check (no respose)
		cmd_buf->read();
		break;
	default:
		// unknown command
		this->out_debug_log(_T("Unknown Command = %2x\n"), cmd);
		send_to_main(0x0f);
		break;
	}
}

void MEMORY::send_to_main(uint8_t val)
{
	// send to main cpu
	d_cpu->write_signal(SIG_MC6801_SIO_RECV, val, 0xff);
}

void MEMORY::play_tape(const _TCHAR* file_path)
{
	close_tape();
	
	if(cmt_fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		memset(cmt_buffer, 0, sizeof(cmt_buffer));
		cmt_fio->Fread(cmt_buffer, sizeof(cmt_buffer), 1);
		cmt_fio->Fclose();
		cmt_count = 0;
		cmt_play = true;
	}
}

void MEMORY::rec_tape(const _TCHAR* file_path)
{
	close_tape();
	
	if(cmt_fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
		my_tcscpy_s(cmt_file_path, _MAX_PATH, file_path);
		cmt_count = 0;
		cmt_rec = true;
	}
}

void MEMORY::close_tape()
{
	if(cmt_fio->IsOpened()) {
		if(cmt_rec && cmt_count) {
			cmt_fio->Fwrite(cmt_buffer, cmt_count, 1);
		}
		cmt_fio->Fclose();
	}
	cmt_count = 0;
	cmt_play = cmt_rec = false;
}

void MEMORY::draw_screen()
{
	static const int xtop[12] = {0, 0, 40, 40, 80, 80, 0, 0, 40, 40, 80, 80};
	static const int ytop[12] = {0, 8, 0, 8, 0, 8, 16, 24, 16, 24, 16, 24};
	
	for(int c = 0; c < 12; c++) {
		int x = xtop[c];
		int y = ytop[c];
		int ofs = (c & 1) ? 40 : 0;
		
		for(int i = 0; i < 40; i++) {
			uint8_t pat = lcd[c >> 1].buffer[ofs + i];
			lcd_render[y + 0][x + i] = (pat & 0x01) ? pd : pb;
			lcd_render[y + 1][x + i] = (pat & 0x02) ? pd : pb;
			lcd_render[y + 2][x + i] = (pat & 0x04) ? pd : pb;
			lcd_render[y + 3][x + i] = (pat & 0x08) ? pd : pb;
			lcd_render[y + 4][x + i] = (pat & 0x10) ? pd : pb;
			lcd_render[y + 5][x + i] = (pat & 0x20) ? pd : pb;
			lcd_render[y + 6][x + i] = (pat & 0x40) ? pd : pb;
			lcd_render[y + 7][x + i] = (pat & 0x80) ? pd : pb;
		}
	}
	for(int y = 0; y < 32; y++) {
		scrntype_t* dest = emu->get_screen_buffer(y);
		my_memcpy(dest, lcd_render[y], sizeof(scrntype_t) * 120);
	}
}

#define STATE_VERSION	2

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	bool wr = false;
	bool rd = false;
	
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	if(loading) {
		wr = state_fio->FgetBool();
		rd = state_fio->FgetBool();
	} else {
		state_fio->FputBool(wbank[0x8000 >> 13] == ext);
		state_fio->FputBool(rbank[0x8000 >> 13] == ext);
	}
	state_fio->StateArray(rom, sizeof(rom), 1);
	state_fio->StateArray(ext, sizeof(ext), 1);
	if(!cmd_buf->process_state((void *)state_fio, loading)) {
		return false;
	}
	state_fio->StateValue(sio_select);
	state_fio->StateValue(special_cmd_masked);
	state_fio->StateArray(slave_mem, sizeof(slave_mem), 1);
	for(int i = 0; i < array_length(sound); i++) {
		state_fio->StateValue(sound[i].freq);
		state_fio->StateValue(sound[i].period);
		state_fio->StateValue(sound[i].remain);
	}
	state_fio->StateValue(sound_ptr);
	state_fio->StateValue(sound_count);
	state_fio->StateValue(sound_reply);
	state_fio->StateValue(sound_freq);
	state_fio->StateArray(key_stat, sizeof(key_stat), 1);
	state_fio->StateArray(key_flag, sizeof(key_flag), 1);
	state_fio->StateValue(key_data);
	state_fio->StateValue(key_strobe);
	state_fio->StateValue(key_intmask);
	state_fio->StateValue(cmt_play);
	state_fio->StateValue(cmt_rec);
	state_fio->StateArray(cmt_file_path, sizeof(cmt_file_path), 1);
	if(loading) {
		int length_tmp = state_fio->FgetInt32_LE();
		if(cmt_rec) {
			cmt_fio->Fopen(cmt_file_path, FILEIO_READ_WRITE_NEW_BINARY);
			while(length_tmp != 0) {
				uint8_t buffer_tmp[1024];
				int length_rw = min(length_tmp, (int)sizeof(buffer_tmp));
				state_fio->Fread(buffer_tmp, length_rw, 1);
				if(cmt_fio->IsOpened()) {
					cmt_fio->Fwrite(buffer_tmp, length_rw, 1);
				}
				length_tmp -= length_rw;
			}
		}
	} else {
		if(cmt_rec && cmt_fio->IsOpened()) {
			int length_tmp = (int)cmt_fio->Ftell();
			cmt_fio->Fseek(0, FILEIO_SEEK_SET);
			state_fio->FputInt32_LE(length_tmp);
			while(length_tmp != 0) {
				uint8_t buffer_tmp[1024];
				int length_rw = min(length_tmp, (int)sizeof(buffer_tmp));
				cmt_fio->Fread(buffer_tmp, length_rw, 1);
				state_fio->Fwrite(buffer_tmp, length_rw, 1);
				length_tmp -= length_rw;
			}
		} else {
			state_fio->FputInt32_LE(0);
		}
	}
	state_fio->StateValue(cmt_count);
	state_fio->StateArray(cmt_buffer, sizeof(cmt_buffer), 1);
	for(int i = 0; i < array_length(lcd); i++) {
		state_fio->StateArray(lcd[i].buffer, sizeof(lcd[i].buffer), 1);
		state_fio->StateValue(lcd[i].bank);
		state_fio->StateValue(lcd[i].addr);
	}
	state_fio->StateValue(lcd_select);
	state_fio->StateValue(lcd_data);
	state_fio->StateValue(lcd_clock);
	state_fio->StateValue(int_status);
	state_fio->StateValue(int_mask);
	
	// post process
	if(loading) {
		SET_BANK(0x8000, 0xbfff, wr ? ext : wdmy, rd ? ext : rom);
	}
	return true;
}

