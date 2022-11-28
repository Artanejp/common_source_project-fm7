/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ data recorder ]
*/

#include "datarec.h"
#include "event.h"
#include "noise.h"

#define EVENT_SIGNAL	0
#define EVENT_SOUND	1

#ifndef DATAREC_FAST_FWD_SPEED
#define DATAREC_FAST_FWD_SPEED	10
#endif
#ifndef DATAREC_FAST_REW_SPEED
#define DATAREC_FAST_REW_SPEED	10
#endif

void DATAREC::initialize()
{
	play_fio = new FILEIO();
	rec_fio = new FILEIO();
	
	memset(rec_file_path, sizeof(rec_file_path), 1);
	play = rec = remote = trigger = false;
	ff_rew = 0;
	in_signal = out_signal = false;
	register_id = -1;
	realtime = false;
	
	buffer = buffer_bak = NULL;
#ifdef DATAREC_SOUND
	sound_buffer = NULL;
#endif
	apss_buffer = NULL;
	buffer_ptr = buffer_length = 0;
	is_wav = is_tap = is_t77 = false;
	ave_hi_freq = 0;
	
	pcm_changed = 0;
	pcm_last_vol_l = pcm_last_vol_r = 0;
#ifdef DATAREC_SOUND
	sound_last_vol_l = sound_last_vol_r = 0;
#endif
	
	// initialize noise
	if(d_noise_play != NULL) {
		d_noise_play->set_device_name(_T("Noise Player (CMT Play)"));
		d_noise_play->load_wav_file(_T("RELAY_ON.WAV"));
		d_noise_play->set_mute(!config.sound_noise_cmt);
	}
	if(d_noise_stop != NULL) {
		d_noise_stop->set_device_name(_T("Noise Player (CMT Stop)"));
		d_noise_stop->load_wav_file(_T("RELAYOFF.WAV"));
		d_noise_stop->set_mute(!config.sound_noise_cmt);
	}
	if(d_noise_fast != NULL) {
		d_noise_fast->set_device_name(_T("Noise Player (CMT Fast)"));
		d_noise_fast->load_wav_file(_T("FAST_FWD.WAV"));
		d_noise_fast->set_loop(true);
		d_noise_fast->set_mute(!config.sound_noise_cmt);
	}
	
	// skip frames
	signal_changed = 0;
	register_frame_event(this);
}

void DATAREC::reset()
{
	touch_sound();
	close_tape();
	pcm_prev_clock = get_current_clock();
	pcm_positive_clocks = pcm_negative_clocks = 0;
}

void DATAREC::release()
{
	close_file();
	delete play_fio;
	delete rec_fio;
}

void DATAREC::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool signal = ((data & mask) != 0);
	
	if(id == SIG_DATAREC_MIC) {
		if(out_signal != signal) {
			touch_sound();
			if(rec && remote) {
				if(out_signal) {
					pcm_positive_clocks += get_passed_clock(pcm_prev_clock);
				} else {
					pcm_negative_clocks += get_passed_clock(pcm_prev_clock);
				}
				pcm_prev_clock = get_current_clock();
				pcm_changed = 2;
				signal_changed++;
			}
			if(prev_clock != 0) {
				if(out_signal) {
					positive_clocks += get_passed_clock(prev_clock);
				} else {
					negative_clocks += get_passed_clock(prev_clock);
				}
				prev_clock = get_current_clock();
			}
			out_signal = signal;
		}
	} else if(id == SIG_DATAREC_REMOTE) {
		touch_sound();
		set_remote(signal);
	} else if(id == SIG_DATAREC_TRIG) {
		// L->H: remote signal is switched
		if(signal && !trigger) {
			touch_sound();
			set_remote(!remote);
		}
		trigger = signal;
	}
}

void DATAREC::event_frame()
{
	if(pcm_changed) {
		pcm_changed--;
	}
#ifdef DATAREC_SOUND
	if(remote && (play || rec) && ff_rew == 0 && signal_changed > 10 && !config.sound_tape_signal && sound_sample == 0) {
#else
	if(remote && (play || rec) && ff_rew == 0 && signal_changed > 10 && !config.sound_tape_signal) {
#endif
		request_skip_frames();
	}
	signal_changed = 0;
}

void DATAREC::event_callback(int event_id, int err)
{
	if(event_id == EVENT_SIGNAL) {
		if(play) {
			if(ff_rew > 0) {
				my_stprintf_s(message, 1024, _T("Fast Forward (%d %%)"), get_tape_position());
			} else if(ff_rew < 0) {
				my_stprintf_s(message, 1024, _T("Fast Rewind (%d %%)"), get_tape_position());
			} else {
				my_stprintf_s(message, 1024, _T("Play (%d %%)"), get_tape_position());
			}
			bool signal = in_signal;
			if(is_wav) {
				if(buffer_ptr >= 0 && buffer_ptr < buffer_length) {
					if(buffer != NULL) {
						signal = ((buffer[buffer_ptr] & 0x80) != 0);
					} else {
						signal = false;
					}
#ifdef DATAREC_SOUND
					if(sound_buffer != NULL && ff_rew == 0) {
						sound_sample = sound_buffer[buffer_ptr];
					} else {
						sound_sample = 0;
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
					if((buffer_bak != NULL) && (buffer != NULL)) {
						my_memcpy(buffer, buffer_bak, buffer_length);
					}
					buffer_ptr = 0;
					set_remote(false);	// top of tape
				} else {
					if(buffer != NULL) {
						while(buffer_ptr < buffer_length) {
							if((buffer[buffer_ptr] & 0x7f) == 0) {
								if(++buffer_ptr == buffer_length) {
									set_remote(false);	// end of tape
									signal = false;
									break;
								}
								signal = ((buffer[buffer_ptr] & 0x80) != 0);
							} else {
								signal = ((buffer[buffer_ptr] & 0x80) != 0);
								uint8_t tmp = buffer[buffer_ptr];
								buffer[buffer_ptr] = (tmp & 0x80) | ((tmp & 0x7f) - 1);
								break;
							}
						}
					} else {
						set_remote(false);	// end of tape
						signal = false;
					}
				}
			}
			// notify the signal is changed
			if(signal != in_signal) {
				if(in_signal) {
					pcm_positive_clocks += get_passed_clock(pcm_prev_clock);
				} else {
					pcm_negative_clocks += get_passed_clock(pcm_prev_clock);
				}
				pcm_prev_clock = get_current_clock();
				pcm_changed = 2;
				in_signal = signal;
				signal_changed++;
				touch_sound();
				write_signals(&outputs_ear, in_signal ? 0xffffffff : 0);
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
		} else if(rec && buffer != NULL) {
			if(out_signal) {
				positive_clocks += get_passed_clock(prev_clock);
			} else {
				negative_clocks += get_passed_clock(prev_clock);
			}
			if(is_wav) {
				if(positive_clocks != 0 || negative_clocks != 0) {
					buffer[buffer_ptr] = (255 * positive_clocks) / (positive_clocks + negative_clocks);
				} else {
					buffer[buffer_ptr] = 0;
				}
				if(++buffer_ptr >= buffer_length) {
					if(is_tap) {
						for(int i = 0; i < buffer_length; i += 8) {
							uint8_t val = 0;
							for(int j = 0, bit = 0x80; j < 8; j++, bit >>= 1) {
								if(i + j < buffer_length && buffer[i + j] >= 0x80) {
									val |= bit;
								}
							}
							rec_fio->FputUint8(val);
						}
					} else {
						rec_fio->Fwrite(buffer, buffer_length, 1);
					}
					buffer_ptr = 0;
				}
			} else if(is_t77) {
				bool signal = (positive_clocks > negative_clocks);
				uint16_t data = (buffer[buffer_ptr] << 8) | buffer[buffer_ptr + 1];
				if(signal != ((data & 0x8000) != 0)) {
					if((buffer_ptr += 2) >= buffer_length) {
						rec_fio->Fwrite(buffer, buffer_length, 1);
						buffer_ptr = 0;
					}
					data = signal ? 0x8001 : 0x0001;
				} else if((data & 0x7fff) < 0x7fff) {
					data++;
				}
				buffer[buffer_ptr + 0] = data >> 8;
				buffer[buffer_ptr + 1] = data & 0xff;
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
			prev_clock = get_current_clock();
			positive_clocks = negative_clocks = 0;
		}
	}
}

void DATAREC::set_remote(bool value)
{
	if(remote != value) {
		if(value) {
			if(d_noise_play != NULL) {
				d_noise_play->play();
			}
			if(d_noise_fast != NULL && ff_rew != 0) {
				d_noise_fast->play();
			}
		} else {
			if(d_noise_stop != NULL) {
				d_noise_stop->play();
			}
			if(d_noise_fast != NULL) {
				d_noise_fast->stop();
			}
		}
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
		if(value != 0) {
			if(d_noise_fast != NULL && remote) {
				d_noise_fast->play();
			}
		} else {
			if(d_noise_fast != NULL) {
				d_noise_fast->stop();
			}
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
		if(play) {
			my_stprintf_s(message, 1024, _T("APSS Forward (%d %%)"), get_tape_position());
		} else {
			my_stprintf_s(message, 1024, _T("APSS Forward"));
		}
	} else {
		if(play) {
			my_stprintf_s(message, 1024, _T("APSS Rewind (%d %%)"), get_tape_position());
		} else {
			my_stprintf_s(message, 1024, _T("APSS Rewind"));
		}
	}
	return result;
}

void DATAREC::update_event()
{
	if(remote && (play || rec)) {
		if(register_id == -1) {
			if(ff_rew > 0) {
				register_event(this, EVENT_SIGNAL, sample_usec / DATAREC_FAST_FWD_SPEED, true, &register_id);
			} else if(ff_rew < 0) {
				register_event(this, EVENT_SIGNAL, sample_usec / DATAREC_FAST_REW_SPEED, true, &register_id);
			} else {
				if(rec) {
					my_stprintf_s(message, 1024, _T("Record"));
				}
				register_event(this, EVENT_SIGNAL, sample_usec, true, &register_id);
			}
			prev_clock = get_current_clock();
			positive_clocks = negative_clocks = 0;
		}
	} else {
		if(register_id != -1) {
			cancel_event(this, register_id);
			register_id = -1;
			if(play) {
				if(buffer_ptr >= buffer_length) {
					my_stprintf_s(message, 1024, _T("Stop (End-of-Tape)"));
				} else if(buffer_ptr <= 0) {
					my_stprintf_s(message, 1024, _T("Stop (Beginning-of-Tape)"));
				} else {
					my_stprintf_s(message, 1024, _T("Stop (%d %%)"), get_tape_position());
				}
			} else {
				my_stprintf_s(message, 1024, _T("Stop"));
			}
		}
		prev_clock = 0;
	}
	
	// update signals
#ifdef DATAREC_SOUND
	if(!(play && remote)) {
		sound_sample = 0;
	}
#endif
	write_signals(&outputs_remote, remote ? 0xffffffff : 0);
	write_signals(&outputs_rotate, (register_id != -1) ? 0xffffffff : 0);
	write_signals(&outputs_end, (buffer_ptr == buffer_length) ? 0xffffffff : 0);
	write_signals(&outputs_top, (buffer_ptr == 0) ? 0xffffffff : 0);
	
	update_realtime_render();
}

void DATAREC::update_realtime_render()
{
	bool value = (remote && (play || rec) && ff_rew == 0 && (config.sound_tape_signal || config.sound_tape_voice));
	
	if(realtime != value) {
		set_realtime_render(this, value);
		realtime = value;
	}
}

bool DATAREC::play_tape(const _TCHAR* file_path)
{
	close_tape();
	
	if(play_fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		if(check_file_extension(play_fio->FilePath(), _T(".wav")) || check_file_extension(play_fio->FilePath(), _T(".mti"))) {
			// standard PCM wave file
			if((buffer_length = load_wav_image(0)) != 0) {
				play = is_wav = true;
			}
		} else if(check_file_extension(play_fio->FilePath(), _T(".t77"))) {
			// FUJITSU FM-7 series tape image
			if((buffer_length = load_t77_image()) != 0) {
				buffer = (uint8_t *)malloc(buffer_length);
				load_t77_image();
				play = is_wav = true;
			}
		} else if(check_file_extension(play_fio->FilePath(), _T(".tap"))) {
			// SHARP X1 series tape image
			if((buffer_length = load_tap_image()) != 0) {
				buffer = (uint8_t *)malloc(buffer_length);
				load_tap_image();
				play = is_wav = true;
			}
		} else if(check_file_extension(play_fio->FilePath(), _T(".mzt")) || check_file_extension(play_fio->FilePath(), _T(".mzf")) || check_file_extension(play_fio->FilePath(), _T(".m12"))) {
			// SHARP MZ series tape image
			if((buffer_length = load_mzt_image()) != 0) {
				buffer = (uint8_t *)malloc(buffer_length);
				load_mzt_image();
				play = is_wav = true;
			}
		} else if(check_file_extension(play_fio->FilePath(), _T(".mtw"))) {
			// skip mzt image
			uint8_t header[128];
			play_fio->Fread(header, sizeof(header), 1);
			uint16_t size = header[0x12] | (header[0x13] << 8);
			// load standard PCM wave file
			if((buffer_length = load_wav_image(sizeof(header) + size)) != 0) {
				play = is_wav = true;
			}
		} else if(check_file_extension(play_fio->FilePath(), _T(".p6"))) {
			// NEC PC-6001/6601 series tape image
			if((buffer_length = load_p6_image(false)) != 0) {
				buffer = (uint8_t *)malloc(buffer_length);
				load_p6_image(false);
				play = is_wav = true;
			}
		} else if(check_file_extension(play_fio->FilePath(), _T(".p6t"))) {
			// NEC PC-6001/6601 series tape image
			if((buffer_length = load_p6_image(true)) != 0) {
				buffer = (uint8_t *)malloc(buffer_length);
				load_p6_image(true);
				play = is_wav = true;
			}
		} else if(check_file_extension(play_fio->FilePath(), _T(".bin"))) {
			// HITACH BASIC Master Jr tape image (bm2)
			if((buffer_length = load_bmjr_image()) != 0) {
				buffer = (uint8_t *)malloc(buffer_length);
				load_bmjr_image();
				play = is_wav = true;
			}
		} else if(check_file_extension(play_fio->FilePath(), _T(".cas"))) {
			// standard cas image for my emulator
			if((buffer_length = load_cas_image()) != 0) {
				buffer = (uint8_t *)malloc(buffer_length);
				load_cas_image();
				play = is_wav = true;
			}
		}
		play_fio->Fclose();
	}
	if(play) {
		if(!is_wav && buffer_length != 0) {
			buffer_bak = (uint8_t *)malloc(buffer_length);
			my_memcpy(buffer_bak, buffer, buffer_length);
		}
		
		// get the first signal
		bool signal = ((buffer[0] & 0x80) != 0);
		if(signal != in_signal) {
			touch_sound();
			write_signals(&outputs_ear, signal ? 0xffffffff : 0);
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

bool DATAREC::rec_tape(const _TCHAR* file_path)
{
	close_tape();
	
	if(rec_fio->Fopen(file_path, FILEIO_READ_WRITE_NEW_BINARY)) {
		my_tcscpy_s(rec_file_path, _MAX_PATH, file_path);
		sample_rate = 48000;
		sample_usec = 1000000. / sample_rate;
		buffer_length = 1024 * 1024;
		buffer = (uint8_t *)calloc(buffer_length, 1);
		
		if(check_file_extension(file_path, _T(".wav"))) {
			// write wave header
			uint8_t dummy[sizeof(wav_header_t) + sizeof(wav_chunk_t)];
			memset(dummy, 0, sizeof(dummy));
			rec_fio->Fwrite(dummy, sizeof(dummy), 1);
			is_wav = true;
		} else if(check_file_extension(file_path, _T(".tap"))) {
			// write frequency
			rec_fio->FputUint32((uint32_t)sample_rate);
			is_wav = is_tap = true;
		} else if(check_file_extension(file_path, _T(".t77"))) {
			// 9us/sample
			sample_usec = 9;
			sample_rate = (int)(1000000.0 / sample_usec + 0.5);
			// write header
			rec_fio->Fwrite("XM7 TAPE IMAGE 0", 16, 1);
			rec_fio->FputUint16(0); // marker
			is_t77 = true;
			// initialize buffer
			buffer[0] = out_signal ? 0x80 : 0;
			buffer[1] = 0;
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
	touch_sound();
	close_file();
	set_remote(false);
	
	play = rec = is_wav = is_tap = is_t77 = false;
	buffer_ptr = buffer_length = 0;
	ave_hi_freq = 0;
	
	update_event();
	
	// no sounds
	write_signals(&outputs_ear, 0);
	in_signal = false;
}

void DATAREC::close_file()
{
	if(play_fio->IsOpened()) {
		play_fio->Fclose();
	}
	if(rec_fio->IsOpened()) {
		if(rec) {
			if(is_tap) {
				for(int i = 0; i < buffer_ptr; i += 8) {
					uint8_t val = 0;
					for(int j = 0, bit = 0x80; j < 8; j++, bit >>= 1) {
						if(i + j < buffer_ptr && buffer[i + j] >= 0x80) {
							val |= bit;
						}
					}
					rec_fio->FputUint8(val);
				}
			} else if(is_t77) {
				rec_fio->Fwrite(buffer, buffer_ptr + 2, 1);
			} else if(is_wav) {
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
	if(sound_buffer != NULL) {
		free(sound_buffer);
		sound_buffer = NULL;
	}
#endif
	if(apss_buffer != NULL) {
		free(apss_buffer);
		apss_buffer = NULL;
	}
}

// standard PCM wave file

void adjust_zero_position(int16_t *wav_buffer, int samples, int sample_rate)
{
	int16_t *zero_buffer = (int16_t *)malloc(samples * sizeof(int16_t));
	int width = (int)((double)sample_rate / 1000.0 + 0.5);
	
	for(int i = width; i < samples - width; i++) {
		int max_sample = -65536, min_sample = 65536;
		for(int j = -width; j < width; j++) {
			if(max_sample < (int)wav_buffer[i + j]) max_sample = (int)wav_buffer[i + j];
			if(min_sample > (int)wav_buffer[i + j]) min_sample = (int)wav_buffer[i + j];
		}
		if(max_sample - min_sample > 4096) {
			zero_buffer[i] = (int16_t)((max_sample + min_sample) / 2);
		} else {
			zero_buffer[i] = wav_buffer[i];
		}
	}
	for(int i = 0; i < samples; i++) {
		wav_buffer[i] -= zero_buffer[(i < width) ? width : (i < samples - width) ? i : (samples - width - 1)];
	}
	free(zero_buffer);
}

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
	sample_usec = 1000000. / sample_rate;
	
	// load samples
	if(samples > 0) {
		#define TMP_LENGTH (0x10000 * header.channels)
		
		uint8_t *tmp_buffer = (uint8_t *)malloc(TMP_LENGTH);
		play_fio->Fread(tmp_buffer, TMP_LENGTH, 1);
		
		#define GET_SAMPLE { \
			for(int ch = 0; ch < header.channels; ch++) { \
				if(header.sample_bits == 16) { \
					union { \
						int16_t s16; \
						struct { \
							uint8_t l, h; \
						} b; \
					} pair; \
					pair.b.l = tmp_buffer[tmp_ptr++]; \
					pair.b.h = tmp_buffer[tmp_ptr++]; \
					sample[ch] = pair.s16; \
				} else { \
					sample[ch] = ((int)tmp_buffer[tmp_ptr++] - 128) * 256; \
				} \
			} \
			if(tmp_ptr == TMP_LENGTH) { \
				play_fio->Fread(tmp_buffer, TMP_LENGTH, 1); \
				tmp_ptr = 0; \
			} \
		}
		
#ifdef DATAREC_SOUND
		if(!config.wave_shaper[drive_num] || header.channels > 1) {
#else
		if(!config.wave_shaper[drive_num]) {
#endif
			// load samples
#ifdef DATAREC_SOUND
			if(header.channels > 1) {
				sound_buffer_length = samples * sizeof(int16_t);
				sound_buffer = (int16_t *)malloc(sound_buffer_length);
			}
#endif
			int16_t *wav_buffer = (int16_t *)malloc(samples * sizeof(int16_t));
			for(int i = 0, tmp_ptr = 0; i < samples; i++) {
				int16_t sample[16];
				GET_SAMPLE
				int16_t sample_signal = sample[0];
#ifdef DATAREC_SOUND
				if(header.channels > 1) {
#ifdef DATAREC_SOUND_LEFT
					sample_signal = sample[1];
					sound_buffer[i] = sample[0];
#else
					sound_buffer[i] = sample[1];
#endif
				}
#endif
				wav_buffer[i] = sample_signal;
			}
			adjust_zero_position(wav_buffer, samples, header.sample_rate);
			
			// copy to dest buffer
			buffer = (uint8_t *)malloc(samples);
			bool prev_signal = false;
			int top_index = 0;
			int16_t max_sample = 0, min_sample = 0;
			
			for(int i = 0, tmp_ptr = 0; i < samples; i++) {
				int16_t sample_signal = wav_buffer[i];
				bool signal = (sample_signal > 0);
				
				if(!prev_signal && signal) {
					if(!(min_sample < -2048)) {
						for(int j = top_index; j < i; j++) {
							buffer[j] = 0;
						}
					}
				} else if(prev_signal && !signal) {
					if(!(max_sample > 2048)) {
						for(int j = top_index; j < i; j++) {
							buffer[j] = 0;
						}
					}
				}
				if(prev_signal != signal) {
					top_index = i;
					max_sample = min_sample = 0;
				}
				if(signal) {
					if(max_sample < sample_signal) {
						max_sample = sample_signal;
					}
				} else {
					if(min_sample > sample_signal) {
						min_sample = sample_signal;
					}
				}
				buffer[i] = (signal ? 0xff : 0);
				prev_signal = signal;
			}
			if(!prev_signal) {
				if(!(min_sample < -2048)) {
					for(int j = top_index; j < samples; j++) {
						buffer[j] = 0;
					}
				}
			} else {
				if(!(max_sample > 2048)) {
					for(int j = top_index; j < samples; j++) {
						buffer[j] = 0;
					}
				}
			}
			free(wav_buffer);
			
			loaded_samples = samples;
		} else {
			// load samples
			int16_t *wav_buffer = (int16_t *)malloc(samples * sizeof(int16_t));
			for(int i = 0, tmp_ptr = 0; i < samples; i++) {
				int16_t sample[16];
				GET_SAMPLE
				wav_buffer[i] = sample[0];
			}
			adjust_zero_position(wav_buffer, samples, header.sample_rate);
			
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
					buffer = (uint8_t *)malloc(loaded_samples);
#ifdef DATAREC_SOUND
					if(header.channels > 1) {
						sound_buffer_length = loaded_samples * sizeof(int16_t);
						sound_buffer = (int16_t *)malloc(sound_buffer_length);
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
	uint32_t length = rec_fio->Ftell();
	
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

// FUJITSU FM-7 series tape image

#define T77_PUT_SIGNAL(signal, len) { \
	int remain = len; \
	while(remain > 0) { \
		if(buffer != NULL) { \
			buffer[ptr++] = (signal) ? 0xff : 0x00; \
		} else { \
			ptr++; \
		} \
		remain--; \
	} \
}

int DATAREC::load_t77_image()
{
	sample_usec = 9;
	sample_rate = (int)(1000000.0 / sample_usec + 0.5);
	
	// load t77 file
	uint8_t tmpbuf[17];
	int ptr = 0;
	int file_size = (int)play_fio->FileLength();
	
	if(file_size <= 0) {
		return 0; // over 2GB
	}
	play_fio->Fseek(0, FILEIO_SEEK_SET);
	play_fio->Fread(tmpbuf, 16, 1);
	tmpbuf[16] = '\0';
	if(strcmp((char *)tmpbuf, "XM7 TAPE IMAGE 0") != 0) {
		return 0;
	}
	file_size -= 16;
	
	while(file_size > 0) {
		uint16_t h = play_fio->FgetUint8();
		uint16_t l = play_fio->FgetUint8();
		uint16_t v = h * 256 + l;
		
		if((file_size -= 2) < 0) {
			break;
		}
		if(v & 0x7fff) {
			T77_PUT_SIGNAL((h & 0x80) != 0, v & 0x7fff);
		}
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
	uint8_t header[4];
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
		sample_usec = 1000000. / sample_rate;
		// data length
		play_fio->Fread(header, 4, 1);
		// play position
		play_fio->Fread(header, 4, 1);
	} else {
		// sample rate
		sample_rate = header[0] | (header[1] << 8) | (header[2] << 16) | (header[3] << 24);
		sample_usec = 1000000. / sample_rate;
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

//#define MZT_PUT_SIGNAL(signal, len) { \
//	int remain = len; \
//	while(remain > 0) { \
//		if(buffer != NULL) { \
//			buffer[ptr++] = ((signal != 0) ? 0x80 : 0) | min(remain, 0x7f); \
//		} else { \
//			ptr++; \
//		} \
//		remain -= min(remain, 0x7f); \
//	} \
//}

#define MZT_PUT_SIGNAL(signal, len) { \
	int remain = len; \
	while(remain > 0) { \
		if(buffer != NULL) { \
			buffer[ptr++] = (signal != 0) ? 0xff : 0; \
		} else { \
			ptr++; \
		} \
		remain--; \
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
	uint8_t hi = (count >> 8) & 0xff; \
	uint8_t lo = (count >> 0) & 0xff; \
	MZT_PUT_BYTE(hi); \
	MZT_PUT_BYTE(lo); \
}

int DATAREC::load_mzt_image()
{
	sample_rate = 48000;
	sample_usec = 1000000. / sample_rate;
	
	// get file size
	play_fio->Fseek(0, FILEIO_SEEK_END);
	int file_size = play_fio->Ftell();
	play_fio->Fseek(0, FILEIO_SEEK_SET);
	
	// load mzt file
	int ptr = 0;
	while(file_size > 128) {
		// load header
		uint8_t header[128], ram[0x20000];
		play_fio->Fread(header, sizeof(header), 1);
		file_size -= sizeof(header);
		
		uint16_t size = header[0x12] | (header[0x13] << 8);
		uint16_t offs = header[0x14] | (header[0x15] << 8);
		memset(ram, 0, sizeof(ram));
		play_fio->Fread(ram + offs, size, 1);
		file_size -= size;
//#if defined(_MZ80K) || defined(_MZ700) || defined(_MZ1200) || defined(_MZ1500)
#if 0
		// apply mz700win patch
		if(header[0x40] == 'P' && header[0x41] == 'A' && header[0x42] == 'T' && header[0x43] == ':') {
			int patch_ofs = 0x44;
			for(; patch_ofs < 0x80; ) {
				uint16_t patch_addr = header[patch_ofs] | (header[patch_ofs + 1] << 8);
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

// NEC PC-6001/6601 series tape image

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

int DATAREC::load_p6_image(bool is_p6t)
{
	sample_rate = 48000;
	sample_usec = 1000000. / sample_rate;
	
	int ptr = 0, remain = 0x10000, data;
	if(is_p6t) {
		// get info block offset
		play_fio->Fseek(-4, FILEIO_SEEK_END);
		int length = play_fio->FgetInt32();
		// check info block
		play_fio->Fseek(length, FILEIO_SEEK_SET);
		char id_p = play_fio->Fgetc();
		char id_6 = play_fio->Fgetc();
		uint8_t ver = play_fio->FgetUint8();
		if(id_p == 'P' && id_6 == '6' && ver == 2) {
			uint8_t blocks = play_fio->FgetUint8();
			if(blocks >= 1) {
				play_fio->FgetUint8();
				play_fio->FgetUint8();
				play_fio->FgetUint8();
				uint16_t cmd = play_fio->FgetUint16();
				play_fio->Fseek(cmd, FILEIO_SEEK_CUR);
				uint16_t exp = play_fio->FgetUint16();
				play_fio->Fseek(exp, FILEIO_SEEK_CUR);
				// check 1st data block
				char id_t = play_fio->Fgetc();
				char id_i = play_fio->Fgetc();
				if(id_t == 'T' && id_i == 'I') {
					play_fio->FgetUint8();
					play_fio->Fseek(16, FILEIO_SEEK_CUR);
					uint16_t baud = play_fio->FgetUint16();	// 600 or 1200
					sample_rate = sample_rate * baud / 1200;
					sample_usec = 1000000. / sample_rate;
				}
			}
			remain = min(length, 0x10000);
		}
	}
	play_fio->Fseek(0, FILEIO_SEEK_SET);
	
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
		remain--;
	}
//	for(int i = 0; i < 1280; i++) {
	for(int i = 0; i < 2400; i++) {
		P6_PUT_2400HZ();
	}
	while((data = play_fio->Fgetc()) != EOF && remain > 0) {
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
		remain--;
	}
#if 1
	for(int i = 0; i < 16; i++) {
		P6_PUT_1200HZ();
		for(int j = 0; j < 8; j++) {
			P6_PUT_1200HZ();
		}
		P6_PUT_2400HZ_X2();
		P6_PUT_2400HZ_X2();
		P6_PUT_2400HZ_X2();
	}
#endif
	return ptr;
}

// HITACH BASIC Master Jr tape image (bm2)

#define BMJR_PUT_1200HZ_X4() { \
	if(buffer != NULL) { \
		for(int k = 0; k < 4; k++) { \
			for(int p = 0; p < 20; p++) buffer[ptr++] = 0xff; \
			for(int p = 0; p < 20; p++) buffer[ptr++] = 0x00; \
		} \
	} else { \
		ptr += 40 * 4; \
	} \
}

#define BMJR_PUT_2400HZ_X8() { \
	if(buffer != NULL) { \
		for(int k = 0; k < 8; k++) { \
			for(int p = 0; p < 10; p++) buffer[ptr++] = 0xff; \
			for(int p = 0; p < 10; p++) buffer[ptr++] = 0x00; \
		} \
	} else { \
		ptr += 20 * 8; \
	} \
}

int DATAREC::load_bmjr_image()
{
	sample_rate = 48000;
	sample_usec = 1000000. / sample_rate;
	
	play_fio->Fseek(0, FILEIO_SEEK_SET);
	
	int ptr = 0, data;
	while((data = play_fio->Fgetc()) != EOF) {
		// start bit = 0
		BMJR_PUT_1200HZ_X4();
		// data bits
		for(int j = 0; j < 8; j++) {
			if(data & (1 << j)) {
				BMJR_PUT_2400HZ_X8();
			} else {
				BMJR_PUT_1200HZ_X4();
			}
		}
		// stop bits = 1,1
		BMJR_PUT_2400HZ_X8();
		BMJR_PUT_2400HZ_X8();
	}
	return ptr;
}

// standard cas image for my emulator

static const uint8_t msx_cas_header[8] = {0x1f, 0xa6, 0xde, 0xba, 0xcc, 0x13, 0x7d, 0x74};

int DATAREC::load_cas_image()
{
	sample_rate = 48000;
	sample_usec = 1000000. / sample_rate;
	
	// SORD m5 or NEC PC-6001 series cas image ?
	static const uint8_t momomomomomo[6] = {0xd3, 0xd3, 0xd3, 0xd3, 0xd3, 0xd3};
	uint8_t tmp_header[8];
	play_fio->Fseek(0, FILEIO_SEEK_SET);
	play_fio->Fread(tmp_header, sizeof(tmp_header), 1);
	
	if(memcmp(tmp_header, "SORDM5", 6) == 0) {
		return load_m5_cas_image();
	} else if(memcmp(tmp_header, msx_cas_header, 8) == 0) {
		return load_msx_cas_image();
	} else if(memcmp(tmp_header, momomomomomo, 6) == 0) {
		return load_p6_image(false);
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
			uint8_t data = play_fio->Fgetc();
			M5_PUT_BYTE(data);
		}
		M5_PUT_BIT(0xff, 8);
		M5_PUT_BIT(0x00, 7);
	}
	M5_PUT_BIT(0x00, 1);
	return ptr;
}

// ASCII MSX tape image (fMSX)
// MAME/MESS /src/lib/formats/fmsx_cas.c by Mr.Sean Young

#define CAS_PERIOD		(16)
#define CAS_HEADER_PERIODS	(4000)
#define CAS_EMPTY_PERIODS	(1000)

int DATAREC::load_msx_cas_image()
{
	sample_rate = 22050;
	sample_usec = 1000000. / sample_rate;
	
	play_fio->Fseek(0, FILEIO_SEEK_END);
	int cas_size = play_fio->Ftell();
	uint8_t *bytes = (uint8_t *)malloc(cas_size);
	play_fio->Fseek(0, FILEIO_SEEK_SET);
	play_fio->Fread(bytes, cas_size, 1);
	
	int cas_pos, bit, state = 1, samples_pos, size, n, i, p;
	
	cas_pos = 0;
	samples_pos = 0;
	
	while(/*samples_pos < sample_count && */cas_pos < cas_size) {
		/* Check if we need to output a header */
		if(cas_pos + 8 < cas_size) {
			if(!memcmp( bytes + cas_pos, msx_cas_header, 8)) {
				/* Write CAS_EMPTY_PERIODS of silence */
				n = CAS_EMPTY_PERIODS * CAS_PERIOD;
				while(n--) {
					if(buffer != NULL) {
						buffer[samples_pos] = 0;
					}
					samples_pos++;
				}
				/* Write CAS_HEADER_PERIODS of header (high frequency) */
				for(i = 0; i < CAS_HEADER_PERIODS * 4 ; i++) {
					for(n = 0; n < CAS_PERIOD / 4; n++) {
						if(buffer != NULL) {
							buffer[samples_pos + n] = (state ? 0xff : 0);
						}
					}
					samples_pos += CAS_PERIOD / 4 ;
					state = !state;
				}
				cas_pos += 8;
			}
		}
		
		for(i = 0; i <= 11; i++) {
			if(i == 0) {
				bit = 0;
			} else if(i < 9) {
				bit = (bytes[cas_pos] & (1 << (i - 1) ) );
			} else {
				bit = 1;
			}
			
			/* write this one bit */
			for(n = 0; n < (bit ? 4 : 2); n++) {
				size = (bit ? CAS_PERIOD / 4 : CAS_PERIOD / 2);
				for(p = 0; p < size; p++) {
					if(buffer != NULL) {
						buffer[samples_pos + p] = (state ? 0xff : 0);
					}
				}
				state = !state;
				samples_pos += size;
			}
		}
		cas_pos++;
	}
	free(bytes);
	return samples_pos;
}

void DATAREC::mix(int32_t* buffer, int cnt)
{
	if(config.sound_tape_signal && pcm_changed && remote && (play || rec) && ff_rew == 0) {
		bool signal = ((play && in_signal) || (rec && out_signal));
		if(signal) {
			pcm_positive_clocks += get_passed_clock(pcm_prev_clock);
		} else {
			pcm_negative_clocks += get_passed_clock(pcm_prev_clock);
		}
		int clocks = pcm_positive_clocks + pcm_negative_clocks;
		int sample = clocks ? (pcm_max_vol * pcm_positive_clocks - pcm_max_vol * pcm_negative_clocks) / clocks : signal ? pcm_max_vol : -pcm_max_vol;
		
		pcm_last_vol_l = apply_volume(sample, pcm_volume_l);
		pcm_last_vol_r = apply_volume(sample, pcm_volume_r);
		
		for(int i = 0; i < cnt; i++) {
			buffer[i] += pcm_last_vol_l; // L
			buffer[i] += pcm_last_vol_r; // R
		}
	} else if(pcm_last_vol_l || pcm_last_vol_r) {
		// suppress petite noise when go to mute
		for(int i = 0; i < cnt; i++) {
			buffer[i] += pcm_last_vol_l; // L
			buffer[i] += pcm_last_vol_r; // R
			
			if(pcm_last_vol_l > 0) {
				pcm_last_vol_l--;
			} else if(pcm_last_vol_l < 0) {
				pcm_last_vol_l++;
			}
			if(pcm_last_vol_r > 0) {
				pcm_last_vol_r--;
			} else if(pcm_last_vol_r < 0) {
				pcm_last_vol_r++;
			}
		}
	}
	pcm_prev_clock = get_current_clock();
	pcm_positive_clocks = pcm_negative_clocks = 0;
	
#ifdef DATAREC_SOUND
	if(config.sound_tape_voice && remote && play && ff_rew == 0) {
		sound_last_vol_l = apply_volume(sound_sample, sound_volume_l);
		sound_last_vol_r = apply_volume(sound_sample, sound_volume_r);
		for(int i = 0; i < cnt; i++) {
			buffer[i] += sound_last_vol_l; // L
			buffer[i] += sound_last_vol_r; // R
		}
	} else if(sound_last_vol_l || sound_last_vol_r) {
		// suppress petite noise when go to mute
		for(int i = 0; i < cnt; i++) {
			buffer[i] += sound_last_vol_l; // L
			buffer[i] += sound_last_vol_r; // R
			
			if(sound_last_vol_l > 0) {
				sound_last_vol_l--;
			} else if(sound_last_vol_l < 0) {
				sound_last_vol_l++;
			}
			if(sound_last_vol_r > 0) {
				sound_last_vol_r--;
			} else if(sound_last_vol_r < 0) {
				sound_last_vol_r++;
			}
		}
	}
#endif
}

void DATAREC::set_volume(int ch, int decibel_l, int decibel_r)
{
	if(ch == 0) {
		pcm_volume_l = decibel_to_volume(decibel_l);
		pcm_volume_r = decibel_to_volume(decibel_r);
#ifdef DATAREC_SOUND
	} else if(ch == 1) {
		sound_volume_l = decibel_to_volume(decibel_l);
		sound_volume_r = decibel_to_volume(decibel_r);
#endif
	}
}

double DATAREC::get_ave_hi_freq()
{
	if(ave_hi_freq == 0 && play && is_wav) {
		bool prev_signal = false;
		int positive = 0, negative = 0, pulse_count = 0;
		double sum = 0;
		double base_usec = 1000000.0 / (double)sample_rate;
		
		for(int i=0; i < buffer_length; i++) {
			bool next_signal = ((buffer[i] & 0x80) != 0);
			if(!prev_signal && next_signal) {
				double usec = base_usec * (positive + negative);
				if(316.667 <= usec && usec < 516.667) {
					sum += usec;
					pulse_count += 1;
				} else if(633.333 <= usec && usec < 1033.333) {
					sum += usec;
					pulse_count += 2;
				}
				positive = negative = 0;
			}
			if(next_signal) {
				positive++;
			} else {
				negative++;
			}
			prev_signal = next_signal;
		}
		if(sum != 0 && pulse_count != 0) {
			double average = sum / pulse_count;
			ave_hi_freq = 1000000.0 / average;
		}
	}
	if(ave_hi_freq == 0) {
		ave_hi_freq = 2400;
	}
	return ave_hi_freq;
}

void DATAREC::update_config()
{
	if(d_noise_play != NULL) {
		d_noise_play->set_mute(!config.sound_noise_cmt);
	}
	if(d_noise_stop != NULL) {
		d_noise_stop->set_mute(!config.sound_noise_cmt);
	}
	if(d_noise_fast != NULL) {
		d_noise_fast->set_mute(!config.sound_noise_cmt);
	}
	update_realtime_render();
}

#define STATE_VERSION	8

bool DATAREC::process_state(FILEIO* state_fio, bool loading)
{
	int length_tmp;
	
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	// pre process
	if(loading) {
		close_file();
	}
	state_fio->StateValue(play);
	state_fio->StateValue(rec);
	state_fio->StateValue(remote);
	state_fio->StateValue(trigger);
	state_fio->StateArray(rec_file_path, sizeof(rec_file_path), 1);
	if(loading) {
		length_tmp = state_fio->FgetInt32_LE();
		if(rec) {
			rec_fio->Fopen(rec_file_path, FILEIO_READ_WRITE_NEW_BINARY);
			while(length_tmp != 0) {
				uint8_t buffer_tmp[1024];
				int length_rw = min(length_tmp, (int)sizeof(buffer_tmp));
				state_fio->Fread(buffer_tmp, length_rw, 1);
				if(rec_fio->IsOpened()) {
					rec_fio->Fwrite(buffer_tmp, length_rw, 1);
				}
				length_tmp -= length_rw;
			}
		}
	} else {
		if(rec && rec_fio->IsOpened()) {
			length_tmp = (int)rec_fio->Ftell();
			rec_fio->Fseek(0, FILEIO_SEEK_SET);
			state_fio->FputInt32_LE(length_tmp);
			while(length_tmp != 0) {
				uint8_t buffer_tmp[1024];
				int length_rw = min(length_tmp, (int)sizeof(buffer_tmp));
				rec_fio->Fread(buffer_tmp, length_rw, 1);
				state_fio->Fwrite(buffer_tmp, length_rw, 1);
				length_tmp -= length_rw;
			}
		} else {
			state_fio->FputInt32_LE(0);
		}
	}
	state_fio->StateValue(ff_rew);
	state_fio->StateValue(in_signal);
	state_fio->StateValue(out_signal);
	state_fio->StateValue(prev_clock);
	state_fio->StateValue(positive_clocks);
	state_fio->StateValue(negative_clocks);
	state_fio->StateValue(signal_changed);
	state_fio->StateValue(register_id);
	state_fio->StateValue(realtime);
	state_fio->StateValue(sample_rate);
	state_fio->StateValue(sample_usec);
	state_fio->StateValue(buffer_ptr);
	if(loading) {
		if((buffer_length = state_fio->FgetInt32_LE()) != 0) {
			buffer = (uint8_t *)malloc(buffer_length);
			state_fio->Fread(buffer, buffer_length, 1);
		}
		if((length_tmp = state_fio->FgetInt32_LE()) != 0) {
			buffer_bak = (uint8_t *)malloc(length_tmp);
			state_fio->Fread(buffer_bak, length_tmp, 1);
		}
#ifdef DATAREC_SOUND
		if((sound_buffer_length = state_fio->FgetInt32_LE()) != 0) {
			sound_buffer = (int16_t *)malloc(sound_buffer_length);
			state_fio->Fread(sound_buffer, sound_buffer_length, 1);
		}
#endif
	} else {
		if(buffer) {
			state_fio->FputInt32_LE(buffer_length);
			state_fio->Fwrite(buffer, buffer_length, 1);
		} else {
			state_fio->FputInt32_LE(0);
		}
		if(buffer_bak) {
			state_fio->FputInt32_LE(buffer_length);
			state_fio->Fwrite(buffer_bak, buffer_length, 1);
		} else {
			state_fio->FputInt32_LE(0);
		}
#ifdef DATAREC_SOUND
		if(sound_buffer) {
			state_fio->FputInt32_LE(sound_buffer_length);
			state_fio->Fwrite(sound_buffer, sound_buffer_length, 1);
		} else {
			state_fio->FputInt32_LE(0);
		}
#endif
	}
#ifdef DATAREC_SOUND
	state_fio->StateValue(sound_sample);
#endif
	state_fio->StateValue(is_wav);
	state_fio->StateValue(is_tap);
	state_fio->StateValue(is_t77);
	if(loading) {
		if((apss_buffer_length = state_fio->FgetInt32_LE()) != 0) {
			apss_buffer = (bool *)malloc(apss_buffer_length);
			state_fio->Fread(apss_buffer, apss_buffer_length, 1);
		}
	} else {
		if(apss_buffer) {
			state_fio->FputInt32_LE(apss_buffer_length);
			state_fio->Fwrite(apss_buffer, apss_buffer_length, 1);
		} else {
			state_fio->FputInt32_LE(0);
		}
	}
	state_fio->StateValue(apss_ptr);
	state_fio->StateValue(apss_count);
	state_fio->StateValue(apss_remain);
	state_fio->StateValue(apss_signals);
	state_fio->StateValue(pcm_changed);
	state_fio->StateValue(pcm_prev_clock);
	state_fio->StateValue(pcm_positive_clocks);
	state_fio->StateValue(pcm_negative_clocks);
	
	// post process
	if(loading) {
		pcm_last_vol_l = pcm_last_vol_r = 0;
#ifdef DATAREC_SOUND
		sound_last_vol_l = sound_last_vol_r = 0;
#endif
	}
	return true;
}

