/*
	NEC PC-6001 Emulator 'yaPC-6001'
	NEC PC-6001mkII Emulator 'yaPC-6201'
	NEC PC-6001mkIISR Emulator 'yaPC-6401'
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : Takeda.Toshiya
	Date   : 2014.05.24-

	[ sub cpu ]
*/

#include "sub.h"
#include "timer.h"
#include "../datarec.h"
#include "../i8255.h"
#include "../mcs48.h"

#define EVENT_PLAY	0
#define EVENT_STOP	1

static const uint8_t key_matrix[16][8] = {
	{0x00,	0x11,	0x10,	0x12,	0x00,	0x00,	0x00,	0x00},
	{0x31,	0x51,	0x41,	0x5a,	0x4b,	0x49,	0x38,	0xbc},
	{0x32,	0x57,	0x53,	0x58,	0x4c,	0x4f,	0x39,	0xbe},
	{0x33,	0x45,	0x44,	0x43,	0xbb,	0x50,	0x70,	0xbf},
	{0x34,	0x52,	0x46,	0x56,	0xba,	0xc0,	0x71,	0xe2},
	{0x35,	0x54,	0x47,	0x42,	0xdd,	0xdb,	0x72,	0x20},
	{0x36,	0x59,	0x48,	0x4e,	0xbd,	0xde,	0x73,	0x30},
	{0x37,	0x55,	0x4a,	0x4d,	0x00,	0xdc,	0x74,	0x00},
	{0x0d,	0x13,	0x26,	0x28,	0x27,	0x25,	0x09,	0x1b},
	{0x15,	0x2d,	0x2e,	0x23,	0x24,	0x00,	0x00,	0x00},	// ROT=END
	{0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00},
	{0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00},
	{0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00},
	{0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00},
	{0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00},
	{0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00},
};

void SUB::initialize()
{
	fio = new FILEIO();
	rec = false;
	
	key_stat = emu->get_key_buffer();
	p1_out = 0x10;
	p2_in = 0xff;
	drec_in = rxrdy_in = false;
	skip = false;
	update_key = true;
	register_frame_event(this);
}

void SUB::release()
{
	close_tape();
	delete fio;
}

void SUB::reset()
{
	close_tape();
	prev_command = 0;
}

#define SET_WR(v) d_pio->write_signal(SIG_I8255_PORT_C, (v) ? 0xff : 0, 0x10)
#define SET_RD(v) d_pio->write_signal(SIG_I8255_PORT_C, (v) ? 0xff : 0, 0x40)

void SUB::write_io8(uint32_t addr, uint32_t data)
{
	// FIXME: this function is referred from both 80c48 and z80
	if((addr & 0xf0) == 0x90) {
		// Z80
		if((addr & 0xff) == 0x90) {
			if(prev_command == 0x38) {
				if(rec && index < sizeof(buffer)) {
					buffer[index++] = data;
					skip = true;
				}
				prev_command = 0;
			} else {
				switch(data) {
				case 0x19:
//					d_drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
					register_event(this, EVENT_PLAY, 1000000.0, false, NULL);
					break;
				case 0x1a:
//					d_drec->write_signal(SIG_DATAREC_REMOTE, 0, 0);
					register_event(this, EVENT_STOP, 1000000.0, false, NULL);
					break;
				case 0x1e:
				case 0x3e:
					baud = 1200;
					break;
				case 0x1d:
				case 0x3d:
					baud = 600;
					break;
				}
				prev_command = data;
			}
		}
		d_pio->write_io8(addr, data);
	} else {
		// 8048
		switch(addr) {
		case MCS48_PORT_P1:
			if((p1_out & 0x0f) != (data & 0x0f)) {
				update_key = true;
			}
			p1_out = data;
//			d_drec->write_signal(SIG_DATAREC_MIC, data, 0x20);
			d_timer->write_signal(SIG_TIMER_IRQ_SUB_CPU, ~data, 0x80);
			break;
		default:
			SET_WR(1);
			SET_WR(0);
			SET_WR(1);
			d_pio->write_signal(SIG_I8255_PORT_A, data, 0xff);
			break;
		}
	}
}

uint32_t SUB::read_io8(uint32_t addr)
{
	// FIXME: this function is referred from both 80c48 and z80
	if((addr & 0xf0) == 0x90) {
		// Z80
		if((addr & 0xff) == 0x90) {
			return d_pio->read_io8(addr);
		} else {
			return 0xff;
		}
	} else {
		// 8048
		switch(addr) {
		case MCS48_PORT_P2:
			if(update_key) {
				int column = p1_out & 0x0f;
				int val = 0;
				for(int i = 0; i < 8; i++) {
					if(key_stat[key_matrix[column][i]]) {
						val |= 1 << i;
					}
				}
				p2_in = (~val) & 0xff;
				update_key = false;
			}
			if(p1_out & 0x10) {
				return p2_in;
			} else {
				// bit3: signal from datarec
				// bit1: rxrdy from rs-232c
				return (rxrdy_in ? 2 : 0) | (drec_in ? 8 : 0) | (p2_in & 0xf0);
			}
		case MCS48_PORT_T0:
			// PC5 -> T0
			return (d_pio->read_signal(SIG_I8255_PORT_C) >> 5) & 1;
		default:
			SET_RD(1);
			SET_RD(0);
			SET_RD(1);
			return d_pio->read_signal(SIG_I8255_PORT_A);
		}
	}
	return 0xff;
}

uint32_t SUB::get_intr_ack()
{
	return d_pio->read_io8(0);
}

void SUB::event_frame()
{
	if(skip) {
		request_skip_frames();
		skip = false;
	}
	update_key = true;
}

void SUB::event_callback(int event_id, int err)
{
	if(event_id == EVENT_PLAY) {
		d_drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
	} else if(event_id == EVENT_STOP) {
		d_drec->write_signal(SIG_DATAREC_REMOTE, 0, 0);
	}
}

void SUB::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_SUB_DATAREC) {
		drec_in = ((data & mask) != 0);
	} else if(id == SIG_SUB_RXRDY) {
		rxrdy_in = ((data & mask) != 0);
	}
}

bool SUB::rec_tape(const _TCHAR* file_path)
{
	close_tape();
	
	if(fio->Fopen(file_path, FILEIO_READ_WRITE_NEW_BINARY)) {
		my_tcscpy_s(rec_file_path, _MAX_PATH, file_path);
		index = 0;
		rec = true;
		is_wav = check_file_extension(file_path, _T(".wav"));
		is_p6t = check_file_extension(file_path, _T(".p6t"));
	}
	return rec;
}

static const uint8_t pulse_1200hz[40] = {
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const uint8_t pulse_2400hz[20] = {
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const uint8_t pulse_2400hz_x2[40] = {
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

void SUB::close_tape()
{
	if(fio->IsOpened()) {
		if(rec) {
			if(is_wav) {
				wav_header_t wav_header;
				wav_chunk_t wav_chunk;
				int sample_rate = (baud == 600) ? 24000 : 48000;
				
				fio->Fwrite(&wav_header, sizeof(wav_header), 1);
				fio->Fwrite(&wav_chunk, sizeof(wav_chunk), 1);
				
				for(int i = 0; i < 9600; i++) {
					fio->Fwrite((void *)pulse_2400hz, sizeof(pulse_2400hz), 1);
				}
				for(int i = 0; i < 16; i++) {
					fio->Fwrite((void *)pulse_1200hz, sizeof(pulse_1200hz), 1);
					for(int j = 0; j < 8; j++) {
						if(buffer[i] & (1 << j)) {
							fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
						} else {
							fio->Fwrite((void *)pulse_1200hz, sizeof(pulse_1200hz), 1);
						}
					}
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
				}
	//			for(int i = 0; i < 1280; i++) {
				for(int i = 0; i < 2400; i++) {
					fio->Fwrite((void *)pulse_2400hz, sizeof(pulse_2400hz), 1);
				}
				for(int i = 16; i < index; i++) {
					fio->Fwrite((void *)pulse_1200hz, sizeof(pulse_1200hz), 1);
					for(int j = 0; j < 8; j++) {
						if(buffer[i] & (1 << j)) {
							fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
						} else {
							fio->Fwrite((void *)pulse_1200hz, sizeof(pulse_1200hz), 1);
						}
					}
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
				}
#if 1
				for(int i = 0; i < 16; i++) {
					fio->Fwrite((void *)pulse_1200hz, sizeof(pulse_1200hz), 1);
					for(int j = 0; j < 8; j++) {
						fio->Fwrite((void *)pulse_1200hz, sizeof(pulse_1200hz), 1);
					}
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
					fio->Fwrite((void *)pulse_2400hz_x2, sizeof(pulse_2400hz_x2), 1);
				}
#endif
				uint32_t length = fio->Ftell();
				
				memcpy(wav_header.riff_chunk.id, "RIFF", 4);
				wav_header.riff_chunk.size = length - 8;
				memcpy(wav_header.wave, "WAVE", 4);
				memcpy(wav_header.fmt_chunk.id, "fmt ", 4);
				wav_header.fmt_chunk.size = 16;
				wav_header.format_id = 1;
				wav_header.channels = 1;
				wav_header.sample_rate = sample_rate;
				wav_header.data_speed = sample_rate;
				wav_header.block_size = 1;
				wav_header.sample_bits = 8;
				
				memcpy(wav_chunk.id, "data", 4);
				wav_chunk.size = length - sizeof(wav_header) - sizeof(wav_chunk);
				
				fio->Fseek(0, FILEIO_SEEK_SET);
				fio->Fwrite(&wav_header, sizeof(wav_header), 1);
				fio->Fwrite(&wav_chunk, sizeof(wav_chunk), 1);
			} else {
				fio->Fwrite(buffer, index, 1);
				if(is_p6t) {
					fio->Fputc('P');
					fio->Fputc('6');
					fio->FputUint8(2);
					fio->FputUint8(2);
					fio->FputUint8(0);
					fio->FputUint8(0);
					fio->FputUint8(0);
					fio->FputUint16(0);
					fio->FputUint16(0);
					
					fio->Fputc('T');
					fio->Fputc('I');
					fio->FputUint8(0);
					for(int i = 0; i < 6; i++) {
						fio->FputUint8(buffer[10 + i]);
					}
					for(int i = 6; i < 16; i++) {
						fio->FputUint8(0);
					}
					fio->FputUint16(baud);
					fio->FputUint16(3000);
					fio->FputUint16(4000);
					fio->FputUint32(0);
					fio->FputUint32(16);
					
					fio->Fputc('T');
					fio->Fputc('I');
					fio->FputUint8(0);
					for(int i = 0; i < 16; i++) {
						fio->FputUint8(0);
					}
					fio->FputUint16(baud);
					fio->FputUint16(0);
					fio->FputUint16(1000);
					fio->FputUint32(16);
					fio->FputUint32(index - 16);
					
					fio->FputUint32(index);
				}
			}
		}
		fio->Fclose();
	}
	rec = false;
}

#define STATE_VERSION	1

bool SUB::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(p1_out);
	state_fio->StateValue(p2_in);
	state_fio->StateValue(drec_in);
	state_fio->StateValue(rxrdy_in);
	state_fio->StateValue(update_key);
	state_fio->StateValue(rec);
	state_fio->StateValue(is_wav);
	state_fio->StateValue(is_p6t);
	state_fio->StateArray(rec_file_path, sizeof(rec_file_path), 1);
	if(loading) {
		int length_tmp = state_fio->FgetInt32_LE();
		if(rec) {
			fio->Fopen(rec_file_path, FILEIO_READ_WRITE_NEW_BINARY);
			while(length_tmp != 0) {
				uint8_t buffer_tmp[1024];
				int length_rw = min(length_tmp, (int)sizeof(buffer_tmp));
				state_fio->Fread(buffer_tmp, length_rw, 1);
				if(fio->IsOpened()) {
					fio->Fwrite(buffer_tmp, length_rw, 1);
				}
				length_tmp -= length_rw;
			}
		}
	} else {
		if(rec && fio->IsOpened()) {
			int length_tmp = (int)fio->Ftell();
			fio->Fseek(0, FILEIO_SEEK_SET);
			state_fio->FputInt32_LE(length_tmp);
			while(length_tmp != 0) {
				uint8_t buffer_tmp[1024];
				int length_rw = min(length_tmp, (int)sizeof(buffer_tmp));
				fio->Fread(buffer_tmp, length_rw, 1);
				state_fio->Fwrite(buffer_tmp, length_rw, 1);
				length_tmp -= length_rw;
			}
		} else {
			state_fio->FputInt32_LE(0);
		}
	}
	state_fio->StateValue(prev_command);
	state_fio->StateValue(baud);
	state_fio->StateValue(index);
	state_fio->StateValue(skip);
	state_fio->StateArray(buffer, sizeof(buffer), 1);
	return true;
}

