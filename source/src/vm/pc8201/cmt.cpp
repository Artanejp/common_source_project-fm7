/*
	NEC PC-8201 Emulator 'ePC-8201'

	Author : Takeda.Toshiya
	Date   : 2013.04.22-

	[ cmt (record only) ]
*/

#include "cmt.h"

#define SAMPLE_RATE 48000

void CMT::initialize()
{
	fio = new FILEIO();
	rec = remote = false;
}

void CMT::release()
{
	close_tape();
	delete fio;
}

void CMT::reset()
{
	close_tape();
	rec = remote = false;
}

void CMT::write_buffer(uint8_t value, int samples)
{
	if(is_wav) {
		for(int i = 0; i < samples; i++) {
			buffer[bufcnt++] = value;
			if(bufcnt == sizeof(buffer)) {
				fio->Fwrite(buffer, sizeof(buffer), 1);
				bufcnt = 0;
			}
		}
	} else {
		value = (value > 128) ? 0x80 : 0;
		while(samples > 0) {
			int s = min(samples, 0x7f);
			samples -= s;
			buffer[bufcnt++] = value | s;
			if(bufcnt == sizeof(buffer)) {
				fio->Fwrite(buffer, sizeof(buffer), 1);
				bufcnt = 0;
			}
		}
	}
}

void CMT::put_signal()
{
	if(rec && remote) {
		uint32_t clock = get_passed_clock(prev_clock);
		if(prev_signal == 1) {
			// 2400Hz
			int count = (int)(1200.0 * (double)clock / (double)CPU_CLOCKS + 0.5) * 2;
			for(int i = 0; i < count; i++) {
				write_buffer(0xff, SAMPLE_RATE / 2400 / 2);
				write_buffer(0x00, SAMPLE_RATE / 2400 / 2);
			}
		} else if(prev_signal == -1) {
			// 1200Hz
			int count = (int)(1200.0 * (double)clock / (double)CPU_CLOCKS + 0.5);
			for(int i = 0; i < count; i++) {
				write_buffer(0xff, SAMPLE_RATE / 1200 / 2);
				write_buffer(0x00, SAMPLE_RATE / 1200 / 2);
			}
		} else {
			write_buffer(0x80, SAMPLE_RATE * clock / CPU_CLOCKS);
		}
	}
}

void CMT::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool next = ((data & mask) != 0);
	
	if(id == SIG_CMT_REMOTE) {
		if(!remote && next) {
			// start
			prev_signal = 0;
			prev_clock = get_current_clock();
		} else if(remote && !next) {
			// stop
			put_signal();
		}
		remote = next;
	} else if(id == SIG_CMT_SOD) {
		if(remote) {
			request_skip_frames();
			put_signal();
		}
		prev_signal = next ? 1 : -1;
		prev_clock = get_current_clock();
	}
}

void CMT::rec_tape(const _TCHAR* file_path)
{
	close_tape();
	
	if(fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
		my_tcscpy_s(rec_file_path, _MAX_PATH, file_path);
		if(check_file_extension(file_path, _T(".wav"))) {
			uint8_t dummy[sizeof(wav_header_t) + sizeof(wav_chunk_t)];
			memset(dummy, 0, sizeof(dummy));
			fio->Fwrite(dummy, sizeof(dummy), 1);
			is_wav = true;
		}
		bufcnt = 0;
		rec = true;
	}
}

void CMT::close_tape()
{
	// close file
	if(rec) {
		if(bufcnt) {
			fio->Fwrite(buffer, bufcnt, 1);
		}
		if(is_wav) {
			uint32_t length = fio->Ftell();
			
			wav_header_t wav_header;
			wav_chunk_t wav_chunk;
			
			memcpy(wav_header.riff_chunk.id, "RIFF", 4);
			wav_header.riff_chunk.size = length - 8;
			memcpy(wav_header.wave, "WAVE", 4);
			memcpy(wav_header.fmt_chunk.id, "fmt ", 4);
			wav_header.fmt_chunk.size = 16;
			wav_header.format_id = 1;
			wav_header.channels = 1;
			wav_header.sample_rate = SAMPLE_RATE;
			wav_header.data_speed = SAMPLE_RATE;
			wav_header.block_size = 1;
			wav_header.sample_bits = 8;
			
			memcpy(wav_chunk.id, "data", 4);
			wav_chunk.size = length - sizeof(wav_header) - sizeof(wav_chunk);
			
			fio->Fseek(0, FILEIO_SEEK_SET);
			fio->Fwrite(&wav_header, sizeof(wav_header), 1);
			fio->Fwrite(&wav_chunk, sizeof(wav_chunk), 1);
		}
		fio->Fclose();
	}
	is_wav = rec = false;
}

#define STATE_VERSION	1

bool CMT::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(is_wav);
	state_fio->StateValue(rec);
	state_fio->StateValue(remote);
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
	state_fio->StateValue(bufcnt);
	state_fio->StateArray(buffer, sizeof(buffer), 1);
	state_fio->StateValue(prev_signal);
	state_fio->StateValue(prev_clock);
	return true;
}

