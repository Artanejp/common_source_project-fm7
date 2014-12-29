/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ data recorder ]
*/

#include "datarec.h"
#include "event.h"
#include "../fileio.h"

#define EVENT_SIGNAL	0
#define EVENT_SOUND	1

#ifndef DATAREC_FF_REW_SPEED
#define DATAREC_FF_REW_SPEED	10
#endif

#pragma pack(1)
typedef struct {
	char id[4];
	uint32 size;
} wav_chunk_t;
#pragma pack()

#pragma pack(1)
typedef struct {
	wav_chunk_t riff_chunk;
	char wave[4];
	wav_chunk_t fmt_chunk;
	uint16 format_id;
	uint16 channels;
	uint32 sample_rate;
	uint32 data_speed;
	uint16 block_size;
	uint16 sample_bits;
} wav_header_t;
#pragma pack()

void DATAREC::initialize()
{
	play_fio = new FILEIO();
	rec_fio = new FILEIO();
	
	memset(rec_file_path, sizeof(rec_file_path), 1);
	play = rec = remote = trigger = false;
	ff_rew = 0;
	in_signal = out_signal = false;
	register_id = -1;
	
	buffer = buffer_bak = NULL;
#ifdef DATAREC_SOUND
	snd_buffer = NULL;
#endif
	apss_buffer = NULL;
	buffer_ptr = buffer_length = 0;
	is_wav = false;
	
#ifdef DATAREC_SOUND
	mix_buffer = NULL;
	mix_buffer_ptr = mix_buffer_length = 0;
#endif
	// skip frames
	changed = 0;
	prev_skip = false;
	register_frame_event(this);
}

void DATAREC::reset()
{
	close_tape();
}

void DATAREC::release()
{
#ifdef DATAREC_SOUND
	if(mix_buffer != NULL) {
		free(mix_buffer);
	}
#endif
	close_file();
	delete play_fio;
	delete rec_fio;
}

void DATAREC::write_signal(int id, uint32 data, uint32 mask)
{
	bool signal = ((data & mask) != 0);
	
	if(id == SIG_DATAREC_OUT) {
		if(out_signal != signal) {
			if(rec && remote) {
				changed++;
			}
			if(prev_clock != 0) {
				if(out_signal) {
					positive_clocks += passed_clock(prev_clock);
				} else {
					negative_clocks += passed_clock(prev_clock);
				}
				prev_clock = current_clock();
			}
			out_signal = signal;
		}
	} else if(id == SIG_DATAREC_REMOTE) {
		set_remote(signal);
	} else if(id == SIG_DATAREC_TRIG) {
		// L->H: remote signal is switched
		if(signal && !trigger) {
			set_remote(!remote);
		}
		trigger = signal;
	}
}

void DATAREC::event_frame()
{
	bool next_skip = (changed > 10) && (ff_rew == 0);
	if(prev_skip != next_skip) {
		set_skip_frames(next_skip);
		prev_skip = next_skip;
	}
	changed = 0;
}

void DATAREC::event_callback(int event_id, int err)
{
	if(event_id == EVENT_SIGNAL) {
		if(play) {
			if(buffer_ptr < buffer_length && ff_rew == 0) {
				emu->out_message(_T("CMT: Play (%d %%)"), 100 * buffer_ptr / buffer_length);
			}
			bool signal = in_signal;
			if(is_wav) {
				if(buffer_ptr >= 0 && buffer_ptr < buffer_length) {
					signal = ((buffer[buffer_ptr] & 0x80) != 0);
#ifdef DATAREC_SOUND
					if(snd_buffer != NULL && ff_rew == 0) {
						snd_sample = snd_buffer[buffer_ptr];
					} else {
						snd_sample = 0;
					}
#endif
				}
				if(ff_rew < 0) {
					if((buffer_ptr = max(buffer_ptr - 1, 0)) == 0) {
						set_remote(false);	// top of tape
						signal = false;
					}
				} else {
					if((buffer_ptr = min(buffer_ptr + 1, buffer_length)) == buffer_length) {
						set_remote(false);	// end of tape
						signal = false;
					}
				}
				update_event();
			} else {
				if(ff_rew < 0) {
					if(buffer_bak != NULL) {
						memcpy(buffer, buffer_bak, buffer_length);
					}
					buffer_ptr = 0;
					set_remote(false);	// top of tape
				} else {
					while(buffer_ptr < buffer_length) {
						if((buffer[buffer_ptr] & 0x7f) == 0) {
							if(++buffer_ptr == buffer_length) {
								set_remote(false);	// end of tape
								signal = false;
								break;
							}
						} else {
							signal = ((buffer[buffer_ptr] & 0x80) != 0);
							uint8 tmp = buffer[buffer_ptr];
							buffer[buffer_ptr] = (tmp & 0x80) | ((tmp & 0x7f) - 1);
							break;
						}
					}
				}
			}
			// notify the signal is changed
			if(signal != in_signal) {
				in_signal = signal;
				changed++;
				write_signals(&outputs_out, in_signal ? 0xffffffff : 0);
			}
			// chek apss state
			if(apss_buffer != NULL) {
				int ptr = (apss_ptr++) % (sample_rate * 2);
				if(apss_buffer[ptr]) {
					apss_count--;
				}
				if(in_signal) {
					apss_count++;
				}
				apss_buffer[ptr] = in_signal;
				
				if(apss_ptr >= sample_rate * 2) {
					double rate = (double)apss_count / (double)(sample_rate * 2);
					if(rate > 0.9 || rate < 0.1) {
						if(apss_signals) {
							if(apss_remain > 0) {
								apss_remain--;
							} else if(apss_remain < 0) {
								apss_remain++;
							}
							write_signals(&outputs_apss, 0xffffffff);
							apss_signals = false;
						}
					} else {
						if(!apss_signals) {
							write_signals(&outputs_apss, 0);
							apss_signals = true;
						}
					}
				}
			}
		} else if(rec) {
			if(out_signal) {
				positive_clocks += passed_clock(prev_clock);
			} else {
				negative_clocks += passed_clock(prev_clock);
			}
			if(is_wav) {
				if(positive_clocks != 0 || negative_clocks != 0) {
					buffer[buffer_ptr] = (255 * positive_clocks) / (positive_clocks + negative_clocks);
				} else {
					buffer[buffer_ptr] = 0;
				}
				if(++buffer_ptr >= buffer_length) {
					rec_fio->Fwrite(buffer, buffer_length, 1);
					buffer_ptr = 0;
				}
			} else {
				bool prev_signal = ((buffer[buffer_ptr] & 0x80) != 0);
				bool cur_signal = (positive_clocks > negative_clocks);
				if(prev_signal != cur_signal || (buffer[buffer_ptr] & 0x7f) == 0x7f) {
					if(++buffer_ptr >= buffer_length) {
						rec_fio->Fwrite(buffer, buffer_length, 1);
						buffer_ptr = 0;
					}
					buffer[buffer_ptr] = cur_signal ? 0x80 : 0;
				}
				buffer[buffer_ptr]++;
			}
			prev_clock = current_clock();
			positive_clocks = negative_clocks = 0;
		}
#ifdef DATAREC_SOUND
	} else if(event_id == EVENT_SOUND) {
		if(mix_buffer_ptr < mix_buffer_length) {
			mix_buffer[mix_buffer_ptr++] = snd_sample;
		}
#endif
	}
}

void DATAREC::set_remote(bool value)
{
	if(remote != value) {
		remote = value;
		update_event();
	}
}

void DATAREC::set_ff_rew(int value)
{
	if(ff_rew != value) {
		if(register_id != -1) {
			cancel_event(this, register_id);
			register_id = -1;
		}
		ff_rew = value;
		apss_signals = false;
		update_event();
	}
}

bool DATAREC::do_apss(int value)
{
	bool result = false;
	
	if(play) {
		set_ff_rew(0);
		set_remote(true);
		set_ff_rew(value > 0 ? 1 : -1);
		apss_remain = value;
		
		while(apss_remain != 0 && remote) {
			event_callback(EVENT_SIGNAL, 0);
		}
		result = (apss_remain == 0);
	}
	
	// stop cmt
	set_remote(false);
	set_ff_rew(0);
	
	if(value > 0) {
		emu->out_message(_T("CMT: APSS (+%d)"), value);
	} else {
		emu->out_message(_T("CMT: APSS (%d)"), value);
	}
	return result;
}

void DATAREC::update_event()
{
	if(remote && (play || rec)) {
		if(register_id == -1) {
			if(ff_rew != 0) {
				register_event(this, EVENT_SIGNAL, 1000000. / sample_rate / DATAREC_FF_REW_SPEED, true, &register_id);
				if(ff_rew > 0) {
					emu->out_message(_T("CMT: Fast Forward"));
				} else {
					emu->out_message(_T("CMT: Fast Rewind"));
				}
			} else {
				register_event(this, EVENT_SIGNAL, 1000000. / sample_rate, true, &register_id);
				if(play) {
					if(buffer_ptr < buffer_length) {
						emu->out_message(_T("CMT: Play (%d %%)"), 100 * buffer_ptr / buffer_length);
					} else {
						emu->out_message(_T("CMT: Play"));
					}
				} else {
					emu->out_message(_T("CMT: Record"));
				}
			}
			prev_clock = current_clock();
			positive_clocks = negative_clocks = 0;
		}
	} else {
		if(register_id != -1) {
			cancel_event(this, register_id);
			register_id = -1;
			if(buffer_ptr == buffer_length) {
				emu->out_message(_T("CMT: Stop (End-of-Tape)"));
			} else if(buffer_ptr == 0) {
				emu->out_message(_T("CMT: Stop (Beginning-of-Tape)"));
			} else {
				emu->out_message(_T("CMT: Stop"));
			}
		}
		prev_clock = 0;
	}
	
	// update signals
#ifdef DATAREC_SOUND
	if(!(play && remote)) {
		snd_sample = 0;
	}
#endif
	write_signals(&outputs_remote, remote ? 0xffffffff : 0);
	write_signals(&outputs_rotate, (register_id != -1) ? 0xffffffff : 0);
	write_signals(&outputs_end, (buffer_ptr == buffer_length) ? 0xffffffff : 0);
	write_signals(&outputs_top, (buffer_ptr == 0) ? 0xffffffff : 0);
}

bool DATAREC::play_tape(_TCHAR* file_path)
{
	close_tape();
	
	if(play_fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		if(check_file_extension(file_path, _T(".wav")) || check_file_extension(file_path, _T(".mti"))) {
			// standard PCM wave file
			if((buffer_length = load_wav_image(0)) != 0) {
				play = is_wav = true;
			}
		} else if(check_file_extension(file_path, _T(".tap"))) {
			// SHARP X1 series tape image
			if((buffer_length = load_tap_image()) != 0) {
				buffer = (uint8 *)malloc(buffer_length);
				load_tap_image();
				play = is_wav = true;
			}
		} else if(check_file_extension(file_path, _T(".mzt")) || check_file_extension(file_path, _T(".m12"))) {
			// SHARP MZ series tape image
			if((buffer_length = load_mzt_image()) != 0) {
				buffer = (uint8 *)malloc(buffer_length);
				load_mzt_image();
				play = true;
			}
		} else if(check_file_extension(file_path, _T(".mtw"))) {
			// skip mzt image
			uint8 header[128];
			play_fio->Fread(header, sizeof(header), 1);
			uint16 size = header[0x12] | (header[0x13] << 8);
			// load standard PCM wave file
			if((buffer_length = load_wav_image(sizeof(header) + size)) != 0) {
				play = is_wav = true;
			}
		} else if(check_file_extension(file_path, _T(".p6"))) {
			// NEC PC-6001 series tape image
			if((buffer_length = load_p6_image()) != 0) {
				buffer = (uint8 *)malloc(buffer_length);
				load_p6_image();
				play = is_wav = true;
			}
		} else if(check_file_extension(file_path, _T(".cas"))) {
			// standard cas image for my emulator
			if((buffer_length = load_cas_image()) != 0) {
				buffer = (uint8 *)malloc(buffer_length);
				load_cas_image();
				play = is_wav = true;
			}
		}
		play_fio->Fclose();
	}
	if(play) {
		if(!is_wav && buffer_length != 0) {
			buffer_bak = (uint8 *)malloc(buffer_length);
			memcpy(buffer_bak, buffer, buffer_length);
		}
		
		// get the first signal
		bool signal = ((buffer[0] & 0x80) != 0);
		if(signal != in_signal) {
			write_signals(&outputs_out, signal ? 0xffffffff : 0);
			in_signal = signal;
		}
		
		// initialize apss
		apss_buffer_length = sample_rate * 2;
		apss_buffer = (bool *)calloc(apss_buffer_length, 1);
		apss_ptr = apss_count = 0;
		apss_signals = false;
		write_signals(&outputs_apss, 0);
		
		update_event();
	}
	return play;
}

bool DATAREC::rec_tape(_TCHAR* file_path)
{
	close_tape();
	
	if(rec_fio->Fopen(file_path, FILEIO_READ_WRITE_NEW_BINARY)) {
		_tcscpy(rec_file_path, file_path);
		sample_rate = 48000;
		buffer_length = 1024 * 1024;
		buffer = (uint8 *)malloc(buffer_length);
		
		if(check_file_extension(file_path, _T(".wav"))) {
			// write wave header
			uint8 dummy[sizeof(wav_header_t) + sizeof(wav_chunk_t)];
			memset(dummy, 0, sizeof(dummy));
			rec_fio->Fwrite(dummy, sizeof(dummy), 1);
			is_wav = true;
		} else {
			// initialize buffer
			buffer[0] = out_signal ? 0x80 : 0;
		}
		rec = true;
		update_event();
	}
	return rec;
}

void DATAREC::close_tape()
{
	close_file();
	
	play = rec = is_wav = false;
	buffer_ptr = buffer_length = 0;
	update_event();
	
	// no sounds
	write_signals(&outputs_out, 0);
	in_signal = false;
}

void DATAREC::close_file()
{
	if(play_fio->IsOpened()) {
		play_fio->Fclose();
	}
	if(rec_fio->IsOpened()) {
		if(rec) {
			if(is_wav) {
				save_wav_image();
			} else {
				rec_fio->Fwrite(buffer, buffer_ptr + 1, 1);
			}
		}
		rec_fio->Fclose();
	}
	if(buffer != NULL) {
		free(buffer);
		buffer = NULL;
	}
	if(buffer_bak != NULL) {
		free(buffer_bak);
		buffer_bak = NULL;
	}
#ifdef DATAREC_SOUND
	if(snd_buffer != NULL) {
		free(snd_buffer);
		snd_buffer = NULL;
	}
#endif
	if(apss_buffer != NULL) {
		free(apss_buffer);
		apss_buffer = NULL;
	}
}

// standard cas image for my emulator

int DATAREC::load_cas_image()
{
	sample_rate = 48000;
	
	// SORD m5 or NEC PC-6001 series cas image ?
	static const uint8 momomomomomo[6] = {0xd3, 0xd3, 0xd3, 0xd3, 0xd3, 0xd3};
	uint8 tmp_header[16];
	play_fio->Fseek(0, FILEIO_SEEK_SET);
	play_fio->Fread(tmp_header, sizeof(tmp_header), 1);
	
	if(memcmp(tmp_header, "SORDM5", 6) == 0) {
		return load_m5_cas_image();
	} else if(memcmp(tmp_header, momomomomomo, 6) == 0) {
		return load_p6_image();
	}
	
	// this is the standard cas image for my emulator
	play_fio->Fseek(0, FILEIO_SEEK_SET);
	int ptr = 0, data;
	while((data = play_fio->Fgetc()) != EOF) {
		for(int i = 0; i < (data & 0x7f); i++) {
			if(buffer != NULL) {
				buffer[ptr] = (data & 0x80) ? 255 : 0;
			}
			ptr++;
		}
	}
	return ptr;
}

// standard PCM wave file

int DATAREC::load_wav_image(int offset)
{
	// check wave header
	wav_header_t header;
	wav_chunk_t chunk;
	
	play_fio->Fseek(offset, FILEIO_SEEK_SET);
	play_fio->Fread(&header, sizeof(header), 1);
	if(header.format_id != 1 || !(header.sample_bits == 8 || header.sample_bits == 16)) {
		return 0;
	}
	play_fio->Fseek(header.fmt_chunk.size - 16, FILEIO_SEEK_CUR);
	while(1) {
		play_fio->Fread(&chunk, sizeof(chunk), 1);
		if(strncmp(chunk.id, "data", 4) == 0) {
			break;
		}
		play_fio->Fseek(chunk.size, FILEIO_SEEK_CUR);
	}
	
	int samples = chunk.size / header.channels, loaded_samples = 0;
	if(header.sample_bits == 16) {
		samples /= 2;
	}
	sample_rate = header.sample_rate;
	
	// load samples
	if(samples > 0) {
		#define TMP_LENGTH (0x10000 * header.channels)
		
		uint8 *tmp_buffer = (uint8 *)malloc(TMP_LENGTH);
		play_fio->Fread(tmp_buffer, TMP_LENGTH, 1);
		
		#define GET_SAMPLE { \
			for(int ch = 0; ch < header.channels; ch++) { \
				if(header.sample_bits == 16) { \
					union { \
						int16 s16; \
						struct { \
							uint8 l, h; \
						} b; \
					} pair; \
					pair.b.l = tmp_buffer[tmp_ptr++]; \
					pair.b.h = tmp_buffer[tmp_ptr++]; \
					sample[ch] = pair.s16; \
				} else { \
					sample[ch] = (tmp_buffer[tmp_ptr++] - 128) * 256; \
				} \
			} \
			if(tmp_ptr == TMP_LENGTH) { \
				play_fio->Fread(tmp_buffer, TMP_LENGTH, 1); \
				tmp_ptr = 0; \
			} \
		}
		
#ifdef DATAREC_SOUND
		if(!config.wave_shaper || header.channels > 1) {
#else
		if(!config.wave_shaper) {
#endif
			buffer = (uint8 *)malloc(samples);
#ifdef DATAREC_SOUND
			if(header.channels > 1) {
				snd_buffer_length = samples * sizeof(int16);
				snd_buffer = (int16 *)malloc(snd_buffer_length);
			}
#endif
			bool prev_signal = false;
			for(int i = 0, tmp_ptr = 0; i < samples; i++) {
				int16 sample[16];
				GET_SAMPLE
				bool signal = (sample[0] > (prev_signal ? -1024 : 1024));
				buffer[i] = (signal ? 0xff : 0);
#ifdef DATAREC_SOUND
				if(header.channels > 1) {
					snd_buffer[i] = sample[1];
				}
#endif
				prev_signal = signal;
			}
			loaded_samples = samples;
		} else {
			// load samples
			int16 *wav_buffer = (int16 *)malloc(samples * sizeof(int16));
			for(int i = 0, tmp_ptr = 0; i < samples; i++) {
				int16 sample[16];
				GET_SAMPLE
				wav_buffer[i] = sample[0];
			}
			
			// adjust zero position
			int16 *zero_buffer = (int16 *)malloc(samples * sizeof(int16));
			int width = (int)(header.sample_rate / 1000.0 + 0.5);
			for(int i = width; i < samples - width; i++) {
				int max_sample = -65536, min_sample = 65536;
				for(int j = -width; j < width; j++) {
					if(max_sample < wav_buffer[i + j]) max_sample = wav_buffer[i + j];
					if(min_sample > wav_buffer[i + j]) min_sample = wav_buffer[i + j];
				}
				if(max_sample - min_sample > 4096) {
					zero_buffer[i] = (max_sample + min_sample) / 2;
				} else {
					zero_buffer[i] = wav_buffer[i];
				}
			}
			for(int i = 0; i < samples; i++) {
				wav_buffer[i] -= zero_buffer[(i < width) ?  width : (i < samples - width) ? i : (samples - width - 1)];
			}
			free(zero_buffer);
			
			// t=0 : get thresholds
			// t=1 : get number of samples
			// t=2 : load samples
			#define FREQ_SCALE 16
			int min_threshold = (int)(header.sample_rate * FREQ_SCALE / 2400.0 / 2.0 / 3.0 + 0.5);
			int max_threshold = (int)(header.sample_rate * FREQ_SCALE / 1200.0 / 2.0 * 3.0 + 0.5);
			int half_threshold, hi_count, lo_count;
			int *counts = (int *)calloc(max_threshold, sizeof(int));
			
			for(int t = 0; t < 3; t++) {
				int count_positive = 0, count_negative = 0;
				bool prev_signal = false;
				
				for(int i = 0; i < samples - 1; i++) {
					int prev = wav_buffer[i], next = wav_buffer[i + 1];
					double diff = (double)(next - prev) / FREQ_SCALE;
					for(int j = 0; j < FREQ_SCALE; j++) {
						int sample = prev + (int)(diff * j + 0.5);
						bool signal = (sample > 0);
						
						if(!prev_signal && signal) {
							if(t == 0) {
								if(count_positive < max_threshold && count_positive > min_threshold && count_negative > min_threshold) {
									counts[count_positive]++;
								}
							} else {
								int count_p = count_positive / FREQ_SCALE;
								int count_n = count_negative / FREQ_SCALE;
								if(count_positive < max_threshold && count_positive > min_threshold && count_negative > min_threshold) {
									count_p = (count_positive > half_threshold) ? hi_count : lo_count;
									if(count_negative < max_threshold) {
										count_n = count_p;
									}
								}
								if(buffer != NULL) {

									for(int j = 0; j < count_p; j++) buffer[loaded_samples++] = 0xff;
									for(int j = 0; j < count_n; j++) buffer[loaded_samples++] = 0x00;
								} else {
									loaded_samples += count_p + count_n;
								}
							}
							count_positive = count_negative = 0;
						}
						if(signal) {
							count_positive++;
						} else {
							count_negative++;
						}
						prev_signal = signal;
					}
				}
				if(t == 0) {
					long sum_value = 0, sum_count = 0, half_tmp;
					for(int i = 0; i < max_threshold; i++) {
						sum_value += i * counts[i];
						sum_count += counts[i];
					}
					// 1920 = 2400 * 0.6 + 1200 * 0.4
					if(sum_count > 60 * 1920) {
						half_tmp = (int)((double)sum_value / (double)sum_count + 0.5);
					} else {
						half_tmp = (int)(header.sample_rate * FREQ_SCALE / 1920.0 / 2.0 + 0.5);
					}
					
					sum_value = sum_count = 0;
					for(int i = 0; i < half_tmp; i++) {
						sum_value += i * counts[i];
						sum_count += counts[i];
					}
					double lo_tmp = (double)sum_value / (double)sum_count;
					
					sum_value = sum_count = 0;
					for(int i = half_tmp; i < half_tmp * 2; i++) {
						sum_value += i * counts[i];
						sum_count += counts[i];
					}
					double hi_tmp = (double)sum_value / (double)sum_count;
					
					half_threshold = (int)((lo_tmp + hi_tmp) / 2 + 0.5);
					min_threshold = (int)(2 * lo_tmp - half_threshold + 0.5);
					max_threshold = (int)(2 * hi_tmp - half_threshold + 0.5);
					lo_count = (int)(lo_tmp / FREQ_SCALE + 0.5);
					hi_count = (int)(hi_tmp / FREQ_SCALE + 0.5);
				} else {
					int count_p = count_positive / FREQ_SCALE;
					int count_n = count_negative / FREQ_SCALE;
					if(count_positive < max_threshold && count_positive > min_threshold && count_negative > min_threshold) {
						count_p = (count_positive > half_threshold) ? hi_count : lo_count;
						if(count_negative < max_threshold) {
							count_n = count_p;
						}
					}
					if(buffer != NULL) {
						for(int j = 0; j < count_p; j++) buffer[loaded_samples++] = 0xff;
						for(int j = 0; j < count_n; j++) buffer[loaded_samples++] = 0x00;
					} else {
						loaded_samples += count_p + count_n;
					}
				}
				if(t == 1) {
					buffer = (uint8 *)malloc(loaded_samples);
#ifdef DATAREC_SOUND
					if(header.channels > 1) {
						snd_buffer_length = loaded_samples * sizeof(int16);
						snd_buffer = (int16 *)malloc(snd_buffer_length);
					}
#endif
					loaded_samples = 0;
				}
			}
			free(counts);
			free(wav_buffer);
		}
		free(tmp_buffer);
	}
	return loaded_samples;
}

void DATAREC::save_wav_image()
{
	// write samples remained in buffer
	if(buffer_ptr > 0) {
		rec_fio->Fwrite(buffer, buffer_ptr, 1);
	}
	uint32 length = rec_fio->Ftell();
	
	wav_header_t wav_header;
	wav_chunk_t wav_chunk;
	
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
	
	rec_fio->Fseek(0, FILEIO_SEEK_SET);
	rec_fio->Fwrite(&wav_header, sizeof(wav_header), 1);
	rec_fio->Fwrite(&wav_chunk, sizeof(wav_chunk), 1);
}

// SORD M5 tape image

#define M5_PUT_BIT(val, len) { \
	int remain = len; \
	while(remain > 0) { \
		if(buffer != NULL) { \
			buffer[ptr] = val ? 0 : 0xff; \
		} \
		ptr++; \
		remain--; \
	} \
}

#define M5_PUT_BYTE(data) { \
	for(int j = 0; j < 10; j++) { \
		int bit = (j == 0) ? 1 : (j == 1) ? 0 : ((data >> (j - 2)) & 1); \
		if(bit) { \
			M5_PUT_BIT(0xff, 8); \
			M5_PUT_BIT(0x00, 7); \
		} else { \
			M5_PUT_BIT(0xff, 16); \
			M5_PUT_BIT(0x00, 14); \
		} \
	} \
}

int DATAREC::load_m5_cas_image()
{
	play_fio->Fseek(16, FILEIO_SEEK_SET);
	int ptr = 0, block_type;
	
	while((block_type = play_fio->Fgetc()) != EOF) {
		if(block_type != 'H' && block_type != 'D') {
			return 0;
		}
		int block_size = play_fio->Fgetc();
		
		if(block_type == 'H') {
			M5_PUT_BIT(0x00, 1);
		}
		for(int i = 0; i < (block_type == 'H' ? 945 : 59); i++) {
			M5_PUT_BIT(0xff, 8);
			M5_PUT_BIT(0x00, 7);
		}
		M5_PUT_BYTE(block_type);
		M5_PUT_BYTE(block_size);
		
		for(int i = 0; i < ((block_size == 0) ? 0x101 : (block_size + 1)); i++) {
			uint8 data = play_fio->Fgetc();
			M5_PUT_BYTE(data);
		}
		M5_PUT_BIT(0xff, 8);
		M5_PUT_BIT(0x00, 7);
	}
	M5_PUT_BIT(0x00, 1);
	return ptr;
}

#define P6_PUT_1200HZ() { \
	if(buffer != NULL) { \
		for(int p = 0; p < 20; p++) buffer[ptr++] = 0xff; \
		for(int p = 0; p < 20; p++) buffer[ptr++] = 0x00; \
	} else { \
		ptr += 40; \
	} \
}

#define P6_PUT_2400HZ() { \
	if(buffer != NULL) { \
		for(int p = 0; p < 10; p++) buffer[ptr++] = 0xff; \
		for(int p = 0; p < 10; p++) buffer[ptr++] = 0x00; \
	} else { \
		ptr += 20; \
	} \
}

#define P6_PUT_2400HZ_X2() { \
	if(buffer != NULL) { \
		for(int p = 0; p < 10; p++) buffer[ptr++] = 0xff; \
		for(int p = 0; p < 10; p++) buffer[ptr++] = 0x00; \
		for(int p = 0; p < 10; p++) buffer[ptr++] = 0xff; \
		for(int p = 0; p < 10; p++) buffer[ptr++] = 0x00; \
	} else { \
		ptr += 40; \
	} \
}

int DATAREC::load_p6_image()
{
	sample_rate = 48000;
	
	play_fio->Fseek(0, FILEIO_SEEK_SET);
	int ptr = 0, data;
	
	for(int i = 0; i < 9600; i++) {
		P6_PUT_2400HZ();
	}
	for(int i = 0; i < 16; i++) {
		data = play_fio->Fgetc();
		P6_PUT_1200HZ();
		for(int j = 0; j < 8; j++) {
			if(data & (1 << j)) {
				P6_PUT_2400HZ_X2();
			} else {
				P6_PUT_1200HZ();
			}
		}
		P6_PUT_2400HZ_X2();
		P6_PUT_2400HZ_X2();
		P6_PUT_2400HZ_X2();
	}
	for(int i = 0; i < 1280; i++) {
		P6_PUT_2400HZ();
	}
	while((data = play_fio->Fgetc()) != EOF) {
		P6_PUT_1200HZ();
		for(int j = 0; j < 8; j++) {
			if(data & (1 << j)) {
				P6_PUT_2400HZ_X2();
			} else {
				P6_PUT_1200HZ();
			}
		}
		P6_PUT_2400HZ_X2();
		P6_PUT_2400HZ_X2();
		P6_PUT_2400HZ_X2();
	}
	return ptr;
}

// SHARP X1 series tape image

/*
	new tape file format for t-tune (from tape_fmt.txt)

	offset:size :
	00H   :  4  : 識別インデックス "TAPE"
	04H   : 17  : テープの名前(asciiz)
	15H   :  5  : リザーブ
	1AH   :  1  : ライトプロテクトノッチ(00H=書き込み可、10H=書き込み禁止）
	1BH   :  1  : 記録フォーマットの種類(01H=定速サンプリング方法）
	1CH   :  4  : サンプリング周波数(Ｈｚ単位）
	20H   :  4  : テープデータのサイズ（ビット単位）
	24H   :  4  : テープの位置（ビット単位）
	28H   :  ?  : テープのデータ
*/

int DATAREC::load_tap_image()
{
	// get file size
	play_fio->Fseek(0, FILEIO_SEEK_END);
	int file_size = play_fio->Ftell();
	play_fio->Fseek(0, FILEIO_SEEK_SET);
	
	// check header
	uint8 header[4];
	play_fio->Fread(header, 4, 1);
	
	if(header[0] == 'T' && header[1] == 'A' && header[2] == 'P' && header[3] == 'E') {
		// skip name, reserved, write protect notch
		play_fio->Fseek(17 + 5 + 1, FILEIO_SEEK_CUR);
		// format
		if(play_fio->Fgetc() != 0x01) {
			// unknown data format
			return 0;
		}
		// sample rate
		play_fio->Fread(header, 4, 1);
		sample_rate = header[0] | (header[1] << 8) | (header[2] << 16) | (header[3] << 24);
		// data length
		play_fio->Fread(header, 4, 1);
		// play position
		play_fio->Fread(header, 4, 1);
	} else {
		// sample rate
		sample_rate = header[0] | (header[1] << 8) | (header[2] << 16) | (header[3] << 24);
	}
	
	// load samples
	int ptr = 0, data;
	while((data = play_fio->Fgetc()) != EOF) {
		for(int i = 0, bit = 0x80; i < 8; i++, bit >>= 1) {
			if(buffer != NULL) {
				buffer[ptr] = ((data & bit) != 0) ? 255 : 0;
			}
			ptr++;
		}
	}
	return ptr;
}

// SHARP MZ series tape image

#define MZT_PUT_SIGNAL(signal, len) { \
	int remain = len; \
	while(remain > 0) { \
		if(buffer != NULL) { \
			buffer[ptr++] = ((signal != 0) ? 0x80 : 0) | min(remain, 0x7f); \
		} else { \
			ptr++; \
		} \
		remain -= min(remain, 0x7f); \
	} \
}

#if defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
#define MZT_PUT_BIT(bit, len) { \
	for(int l = 0; l < (len); l++) { \
		if(bit) { \
			MZT_PUT_SIGNAL(1, (int)(120.0 / 16.0 * sample_rate / 22050.0 + 0.5)); \
			MZT_PUT_SIGNAL(0, (int)(120.0 / 16.0 * sample_rate / 22050.0 + 0.5)); \
		} else { \
			MZT_PUT_SIGNAL(1, (int)(60.0 / 16.0 * sample_rate / 22050.0 + 0.5)); \
			MZT_PUT_SIGNAL(0, (int)(60.0 / 16.0 * sample_rate / 22050.0 + 0.5)); \
		} \
	} \
}
#else
#define MZT_PUT_BIT(bit, len) { \
	for(int l = 0; l < (len); l++) { \
		if(bit) { \
			MZT_PUT_SIGNAL(1, (int)(24.0 * sample_rate / 48000.0 + 0.5)); \
			MZT_PUT_SIGNAL(0, (int)(29.0 * sample_rate / 48000.0 + 0.5)); \
		} else { \
			MZT_PUT_SIGNAL(1, (int)(11.0 * sample_rate / 48000.0 + 0.5)); \
			MZT_PUT_SIGNAL(0, (int)(15.0 * sample_rate / 48000.0 + 0.5)); \
		} \
	} \
}
#endif

#define MZT_PUT_BYTE(byte) { \
	MZT_PUT_BIT(1, 1); \
	for(int j = 0; j < 8; j++) { \
		if((byte) & (0x80 >> j)) { \
			MZT_PUT_BIT(1, 1); \
			count++; \
		} else { \
			MZT_PUT_BIT(0, 1); \
		} \
	} \
}

#define MZT_PUT_BLOCK(buf, len) { \
	int count = 0; \
	for(int i = 0; i < (len); i++) { \
		MZT_PUT_BYTE((buf)[i]); \
	} \
	uint8 hi = (count >> 8) & 0xff; \
	uint8 lo = (count >> 0) & 0xff; \
	MZT_PUT_BYTE(hi); \
	MZT_PUT_BYTE(lo); \
}

int DATAREC::load_mzt_image()
{
	sample_rate = 48000;
	
	// get file size
	play_fio->Fseek(0, FILEIO_SEEK_END);
	int file_size = play_fio->Ftell();
	play_fio->Fseek(0, FILEIO_SEEK_SET);
	
	// load mzt file
	int ptr = 0;
	while(file_size > 128) {
		// load header
		uint8 header[128], ram[0x20000];
		play_fio->Fread(header, sizeof(header), 1);
		file_size -= sizeof(header);
		
		uint16 size = header[0x12] | (header[0x13] << 8);
		uint16 offs = header[0x14] | (header[0x15] << 8);
		memset(ram, 0, sizeof(ram));
		play_fio->Fread(ram + offs, size, 1);
		file_size -= size;
//#if defined(_MZ80K) || defined(_MZ700) || defined(_MZ1200) || defined(_MZ1500)
#if 0
		// apply mz700win patch
		if(header[0x40] == 'P' && header[0x41] == 'A' && header[0x42] == 'T' && header[0x43] == ':') {
			int patch_ofs = 0x44;
			for(; patch_ofs < 0x80; ) {
				uint16 patch_addr = header[patch_ofs] | (header[patch_ofs + 1] << 8);
				patch_ofs += 2;
				if(patch_addr == 0xffff) {
					break;
				}
				int patch_len = header[patch_ofs++];
				for(int i = 0; i < patch_len; i++) {
					ram[patch_addr + i] = header[patch_ofs++];
				}
			}
			for(int i = 0x40; i < patch_ofs; i++) {
				header[i] = 0;
			}
		}
#endif
		// output to buffer
		MZT_PUT_SIGNAL(0, sample_rate);
#if defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
		// Bin2Wav Ver 0.03
		MZT_PUT_BIT(0, 22000);
		MZT_PUT_BIT(1, 40);
		MZT_PUT_BIT(0, 41);
		MZT_PUT_BLOCK(header, 128);
		MZT_PUT_BIT(1, 1);
		MZT_PUT_SIGNAL(1, (int)(22.0 * sample_rate / 22050.0 + 0.5));
		MZT_PUT_SIGNAL(0, (int)(22.0 * sample_rate / 22050.0 + 0.5));
		MZT_PUT_SIGNAL(0, sample_rate);
		MZT_PUT_BIT(0, 11000);
		MZT_PUT_BIT(1, 20);
		MZT_PUT_BIT(0, 21);
		MZT_PUT_BLOCK(ram + offs, size);
		MZT_PUT_BIT(1, 1);
#else
		// format info written in 試験に出るX1
		MZT_PUT_BIT(0, 10000);
		MZT_PUT_BIT(1, 40);
		MZT_PUT_BIT(0, 40);
		MZT_PUT_BIT(1, 1);
		MZT_PUT_BLOCK(header, 128);
		MZT_PUT_BIT(1, 1);
		MZT_PUT_BIT(0, 256);
		MZT_PUT_BLOCK(header, 128);
		MZT_PUT_BIT(1, 1);
		MZT_PUT_SIGNAL(0, sample_rate);
		MZT_PUT_BIT(0, 10000);
		MZT_PUT_BIT(1, 20);
		MZT_PUT_BIT(0, 20);
		MZT_PUT_BIT(1, 1);
		MZT_PUT_BLOCK(ram + offs, size);
		MZT_PUT_BIT(1, 1);
#endif
	}
	return ptr;
}

#ifdef DATAREC_SOUND
void DATAREC::initialize_sound(int rate, int samples)
{
	mix_buffer = (int16 *)malloc(samples * 2 * sizeof(int16));
	mix_buffer_length = samples * 2;
	register_event(this, EVENT_SOUND, 1000000. / (double)rate, true, NULL);
}

void DATAREC::mix(int32* buffer, int cnt)
{
	int16 sample = 0;
	for(int i = 0; i < cnt; i++) {
		if(i < mix_buffer_ptr) {
			sample = mix_buffer[i];
		}
		*buffer += sample;
		*buffer += sample;
	}
	if(cnt < mix_buffer_ptr) {
		memmove(mix_buffer, mix_buffer + cnt, (mix_buffer_ptr - cnt) * sizeof(int16));
		mix_buffer_ptr -= cnt;
	} else {
		mix_buffer_ptr = 0;
	}
}
#endif

#define STATE_VERSION	1

void DATAREC::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(rec_file_path, sizeof(rec_file_path), 1);
	if(rec && rec_fio->IsOpened()) {
		int length_tmp = (int)rec_fio->Ftell();
		rec_fio->Fseek(0, FILEIO_SEEK_SET);
		state_fio->FputInt32(length_tmp);
		while(length_tmp != 0) {
			uint8 buffer[1024];
			int length_rw = min(length_tmp, sizeof(buffer));
			rec_fio->Fread(buffer, length_rw, 1);
			state_fio->Fwrite(buffer, length_rw, 1);
			length_tmp -= length_rw;
		}
	} else {
		state_fio->FputInt32(0);
	}
	state_fio->FputBool(play);
	state_fio->FputBool(rec);
	state_fio->FputBool(remote);
	state_fio->FputBool(trigger);
	state_fio->FputInt32(ff_rew);
	state_fio->FputBool(in_signal);
	state_fio->FputBool(out_signal);
	state_fio->FputUint32(prev_clock);
	state_fio->FputInt32(positive_clocks);
	state_fio->FputInt32(negative_clocks);
	state_fio->FputInt32(changed);
	state_fio->FputBool(prev_skip);
	state_fio->FputInt32(register_id);
	state_fio->FputInt32(sample_rate);
	state_fio->FputInt32(buffer_ptr);
	if(buffer) {
		state_fio->FputInt32(buffer_length);
		state_fio->Fwrite(buffer, buffer_length, 1);
	} else {
		state_fio->FputInt32(0);
	}
	if(buffer_bak) {
		state_fio->FputInt32(buffer_length);
		state_fio->Fwrite(buffer_bak, buffer_length, 1);
	} else {
		state_fio->FputInt32(0);
	}
#ifdef DATAREC_SOUND
	if(snd_buffer) {
		state_fio->FputInt32(snd_buffer_length);
		state_fio->Fwrite(snd_buffer, snd_buffer_length, 1);
	} else {
		state_fio->FputInt32(0);
	}
	state_fio->FputInt16(snd_sample);
#endif
	state_fio->FputBool(is_wav);
	if(apss_buffer) {
		state_fio->FputInt32(apss_buffer_length);
		state_fio->Fwrite(apss_buffer, apss_buffer_length, 1);
	} else {
		state_fio->FputInt32(0);
	}
	state_fio->FputInt32(apss_ptr);
	state_fio->FputInt32(apss_count);
	state_fio->FputInt32(apss_remain);
	state_fio->FputBool(apss_signals);
}

bool DATAREC::load_state(FILEIO* state_fio)
{
	int length_tmp;
	
	close_file();
	
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(rec_file_path, sizeof(rec_file_path), 1);
	if((length_tmp = state_fio->FgetInt32()) != 0) {
		rec_fio->Fopen(rec_file_path, FILEIO_READ_WRITE_NEW_BINARY);
		while(length_tmp != 0) {
			uint8 buffer[1024];
			int length_rw = min(length_tmp, sizeof(buffer));
			state_fio->Fread(buffer, length_rw, 1);
			if(rec_fio->IsOpened()) {
				rec_fio->Fwrite(buffer, length_rw, 1);
			}
			length_tmp -= length_rw;
		}
	}
	play = state_fio->FgetBool();
	rec = state_fio->FgetBool();
	remote = state_fio->FgetBool();
	trigger = state_fio->FgetBool();
	ff_rew = state_fio->FgetInt32();
	in_signal = state_fio->FgetBool();
	out_signal = state_fio->FgetBool();
	prev_clock = state_fio->FgetUint32();
	positive_clocks = state_fio->FgetInt32();
	negative_clocks = state_fio->FgetInt32();
	changed = state_fio->FgetInt32();
	prev_skip = state_fio->FgetBool();
	register_id = state_fio->FgetInt32();
	sample_rate = state_fio->FgetInt32();
	buffer_ptr = state_fio->FgetInt32();
	if((buffer_length = state_fio->FgetInt32()) != 0) {
		buffer = (uint8 *)malloc(buffer_length);
		state_fio->Fread(buffer, buffer_length, 1);
	}
	if((length_tmp = state_fio->FgetInt32()) != 0) {
		buffer_bak = (uint8 *)malloc(length_tmp);
		state_fio->Fread(buffer_bak, length_tmp, 1);
	}
#ifdef DATAREC_SOUND
	if((snd_buffer_length = state_fio->FgetInt32()) != 0) {
		snd_buffer = (int16 *)malloc(snd_buffer_length);
		state_fio->Fread(snd_buffer, snd_buffer_length, 1);
	}
	snd_sample = state_fio->FgetInt16();
#endif
	is_wav = state_fio->FgetBool();
	if((apss_buffer_length = state_fio->FgetInt32()) != 0) {
		apss_buffer = (bool *)malloc(apss_buffer_length);
		state_fio->Fread(apss_buffer, apss_buffer_length, 1);
	}
	apss_ptr = state_fio->FgetInt32();
	apss_count = state_fio->FgetInt32();
	apss_remain = state_fio->FgetInt32();
	apss_signals = state_fio->FgetBool();
	
#ifdef DATAREC_SOUND
	// clear mix buffer
	mix_buffer_ptr = 0;
#endif
	return true;
}

