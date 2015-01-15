/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2014.02.12-

	[ Pioneer LD-700 ]
*/

#define EVENT_ACK		0
#define EVENT_SOUND		1
#define EVENT_MIX		2

#define PHASE_IDLE		0
#define PHASE_HEADER_PULSE	1
#define PHASE_HEADER_SPACE	2
#define PHASE_BITS_PULSE	3
#define PHASE_BITS_SPACE	4

#define STATUS_EJECT		0
#define STATUS_STOP		1
#define STATUS_PLAY		2
#define STATUS_PAUSE		3

#define SEEK_CHAPTER		0x40
#define SEEK_FRAME		0x41
#define SEEK_WAIT		0x5f

#include "ld700.h"
#include "../fifo.h"
#include "../fileio.h"

void LD700::initialize()
{
	prev_remote_signal = false;
	prev_remote_time = 0;
	command = num_bits = 0;
	
	status = STATUS_EJECT;
	phase = PHASE_IDLE;
	seek_mode = seek_num = 0;
	accepted = false;
	cur_frame_raw = 0;
	wait_frame_raw = 0;
	
	prev_sound_signal = false;
	sound_buffer_l = new FIFO(48000 * 4);
	sound_buffer_r = new FIFO(48000 * 4);
	signal_buffer = new FIFO(48000 * 4);
	signal_buffer_ok = false;
	sound_event_id = -1;
	sound_sample_l = sound_sample_r = 0;
	
	mix_buffer_l = mix_buffer_r = NULL;
	mix_buffer_ptr = mix_buffer_length = 0;
	mix_buffer_ptr = mix_buffer_length = 0;
	
	register_frame_event(this);
}

void LD700::release()
{
	if(mix_buffer_l != NULL) {
		free(mix_buffer_l);
	}
	if(mix_buffer_r != NULL) {
		free(mix_buffer_r);
	}
	sound_buffer_l->release();
	delete sound_buffer_l;
	sound_buffer_r->release();
	delete sound_buffer_r;
	signal_buffer->release();
	delete signal_buffer;
}

void LD700::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_LD700_REMOTE) {
		bool signal = ((data & mask) != 0);
		if(prev_remote_signal != signal) {
			int usec = (int)passed_usec(prev_remote_time);
			prev_remote_time = current_clock();
			prev_remote_signal = signal;
			
			// from openmsx-0.10.0/src/laserdisc/
			switch(phase) {
			case PHASE_IDLE:
				if(signal) {
					command = num_bits = 0;
					phase = PHASE_HEADER_PULSE;
				}
				break;
			case PHASE_HEADER_PULSE:
				if(5800 <= usec && usec < 11200) {
					phase = PHASE_HEADER_SPACE;
				} else {
					phase = PHASE_IDLE;
				}
				break;
			case PHASE_HEADER_SPACE:
				if(3400 <= usec && usec < 6200) {
					phase = PHASE_BITS_PULSE;
				} else {
					phase = PHASE_IDLE;
				}
				break;
			case PHASE_BITS_PULSE:
				if(usec >= 380 && usec < 1070) {
					phase = PHASE_BITS_SPACE;
				} else {
					phase = PHASE_IDLE;
				}
				break;
			case PHASE_BITS_SPACE:
				if(1260 <= usec && usec < 4720) {
					// bit 1
					command |= 1 << num_bits;
				} else if(usec < 300 || usec >= 1065) {
					// error
					phase = PHASE_IDLE;
					break;
				}
				if(++num_bits == 32) {
					uint8 custom      = ( command >>  0) & 0xff;
					uint8 custom_comp = (~command >>  8) & 0xff;
					uint8 code        = ( command >> 16) & 0xff;
					uint8 code_comp   = (~command >> 24) & 0xff;
					if(custom == custom_comp && custom == 0xa8 && code == code_comp) {
						// command accepted
						accepted = true;
					}
					phase = PHASE_IDLE;
				} else {
					phase = PHASE_BITS_PULSE;
				}
				break;
			}
		}
	} else if(id == SIG_LD700_MUTE_L) {
		sound_mute_l = ((data & mask) != 0);
	} else if(id == SIG_LD700_MUTE_R) {
		sound_mute_r = ((data & mask) != 0);
	}
}

void LD700::event_frame()
{
	int prev_frame_raw = cur_frame_raw;
	bool seek_done = false;
	
	cur_frame_raw = get_cur_frame_raw();
	
	if(accepted) {
		command = (command >> 16) & 0xff;
		emu->out_debug_log("---\n",command);
		emu->out_debug_log("LD700: COMMAND=%02x\n",command);
		switch(command) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
			if(status != STATUS_EJECT /*&& status != STATUS_STOP*/) {
				seek_num = seek_num * 10 + command;
				emu->out_debug_log("LD700: SEEK NUMBER=%d\n", seek_num);
			}
			break;
		case 0x16:
			if(status != STATUS_EJECT) {
				if(status == STATUS_STOP) {
					//emu->close_laser_disc();
				} else {
					emu->stop_movie();
					emu->set_cur_movie_frame(0, false);
					set_status(STATUS_STOP);
					emu->out_debug_log("LD700: STOP\n");
				}
			}
			break;
		case 0x17:
			if(status != STATUS_EJECT && status != STATUS_PLAY) {
				emu->mute_direct_show_dev(true, true);
				emu->play_movie();
				set_status(STATUS_PLAY);
				emu->out_debug_log("LD700: PLAY\n");
			}
			break;
		case 0x18:
			if(status != STATUS_EJECT /*&& status != STATUS_STOP*/) {
				emu->pause_movie();
				set_status(STATUS_PAUSE);
				emu->out_debug_log("LD700: PAUSE\n");
			}
			break;
		case 0x40:	// SEEK_CHAPTER
		case 0x41:	// SEEK_FRAME
		case 0x5f:	// SEEK_WAIT
			if(status != STATUS_EJECT /*&& status != STATUS_STOP*/) {
				seek_mode = command;
				seek_num = 0;
			}
			break;
		case 0x42:
			if(status != STATUS_EJECT /*&& status != STATUS_STOP*/) {
				int tmp = seek_num, num[3];
				bool flag = true;
				memset(num, 0, sizeof(num));
				
				for(int i = 0; i < 3; i++) {
					int n0 = tmp % 10;
					tmp /= 10;
					int n1 = tmp % 10;
					tmp /= 10;
					int n2 = tmp % 10;
					tmp /= 10;
					if(n0 == n1 && n0 == n2) {
						num[i] = n0;
					} else {
						flag = false;
						break;
					}
				}
				if(flag && (num[1] != 0 || num[2] != 0)) {
					seek_num = num[0] + num[1] * 10 + num[2] * 100;
				}
				if(seek_mode == SEEK_WAIT) {
					if(seek_num >= 101 && seek_num < 200) {
						wait_frame_raw = track_frame_raw[seek_num - 100];
					} else {
						wait_frame_raw = (int)((double)seek_num / 29.97 * emu->get_movie_frame_rate() + 0.5);
					}
					emu->out_debug_log("LD700: WAIT FRAME=%d\n", seek_num);
				} else {
					if(seek_mode == SEEK_CHAPTER) {
						emu->out_debug_log("LD700: SEEK TRACK=%d\n", seek_num);
						set_cur_track(seek_num);
					} else if(seek_mode == SEEK_FRAME) {
						emu->out_debug_log("LD700: SEEK FRAME=%d\n", seek_num);
						set_cur_frame(seek_num, false);
					}
					if(status == STATUS_PAUSE) {
						emu->mute_direct_show_dev(true, true);
						emu->play_movie();
						set_status(STATUS_PLAY);
						emu->out_debug_log("LD700: PLAY\n");
					}
					seek_done = true;
				}
				seek_mode = 0;
			}
			break;
		case 0x45:
			if(status != STATUS_EJECT /*&& status != STATUS_STOP*/) {
				seek_num = 0;
			}
			break;
		default:
			emu->out_debug_log(_T("LaserDisc: Unknown Command %02X\n"), command);
		}
		accepted = false;
		set_ack(true);
	}
	
	if(!seek_done && status == STATUS_PLAY) {
		if(wait_frame_raw != 0 && prev_frame_raw < wait_frame_raw && cur_frame_raw >= wait_frame_raw) {
			emu->out_debug_log("LD700: WAIT RAW FRAME=%d (%d)\n", wait_frame_raw, cur_frame_raw);
			set_ack(true);
			wait_frame_raw = 0;
		}
		for(int i = 0; i < num_pauses; i++) {
			if(prev_frame_raw < pause_frame_raw[i] && cur_frame_raw >= pause_frame_raw[i]) {
				emu->pause_movie();
				set_status(STATUS_PAUSE);
				emu->out_debug_log("LD700: PAUSE RAW FRAME=%d (%d->%d)\n", pause_frame_raw[i], prev_frame_raw, cur_frame_raw);
				break;
			}
		}
	}
}

void LD700::event_callback(int event_id, int err)
{
	if(event_id == EVENT_ACK) {
		set_ack(false);
	} else if(event_id == EVENT_SOUND) {
		if(signal_buffer_ok) {
			int sample = signal_buffer->read();
			bool signal = sample > 100 ? true : sample < -100 ? false : prev_sound_signal;
			prev_sound_signal = signal;
			write_signals(&outputs_sound, signal ? 0xffffffff : 0);
		}
		sound_sample_l = sound_buffer_l->read();
		sound_sample_r = sound_buffer_r->read();
	} else if(event_id == EVENT_MIX) {
		if(mix_buffer_ptr < mix_buffer_length) {
			mix_buffer_l[mix_buffer_ptr] = sound_mute_l ? 0 : sound_sample_l;
			mix_buffer_r[mix_buffer_ptr] = sound_mute_r ? 0 : sound_sample_r;
			mix_buffer_ptr++;
		}
	}
}

void LD700::set_status(int value)
{
	if(status != value) {
		if(value == STATUS_PLAY) {
			if(sound_event_id == -1) {
				register_event(this, EVENT_SOUND, 1000000.0 / emu->get_movie_sound_rate(), true, &sound_event_id);
			}
			sound_buffer_l->clear();
			sound_buffer_r->clear();
			signal_buffer->clear();
			signal_buffer_ok = false;
		} else {
			if(sound_event_id != -1) {
				cancel_event(this, sound_event_id);
				sound_event_id = -1;
				sound_sample_l = sound_sample_r = 0;
			}
		}
		write_signals(&outputs_exv, !(value == STATUS_EJECT || value == STATUS_STOP) ? 0xffffffff : 0);
		status = value;
	}
}

void LD700::set_ack(bool value)
{
	if(value) {
		register_event(this, EVENT_ACK, 46000, false, NULL);
	}
	write_signals(&outputs_ack, value ? 0xffffffff : 0);
}

void LD700::set_cur_frame(int frame, bool relative)
{
	if(relative) {
		if(frame == 0) {
			return;
		}
	} else {
		if(frame < 0) {
			return;
		}
	}
	bool sign = (frame >= 0);
	frame = (int)((double)abs(frame) / 29.97 * emu->get_movie_frame_rate() + 0.5);
	if(relative && frame == 0) {
		frame = 1;
	}
	emu->set_cur_movie_frame(sign ? frame : -frame, relative);
	emu->out_debug_log("LD700: SEEK RAW FRAME=%d RELATIVE=%d\n", sign ? frame : -frame, relative);
}

int LD700::get_cur_frame_raw()
{
	return emu->get_cur_movie_frame();
}

void LD700::set_cur_track(int track)
{
	if(track >= 0 && track <= num_tracks) {
		emu->set_cur_movie_frame(track_frame_raw[track], false);
	}
}

void LD700::open_disc(_TCHAR* file_path)
{
	if(emu->open_movie_file(file_path)) {
		emu->out_debug_log("LD700: OPEN MOVIE PATH=%s\n", file_path);
		
		// read LOCATION information
		num_tracks = -1;
		memset(track_frame_raw, 0, sizeof(track_frame_raw));
		num_pauses = 0;
		memset(pause_frame_raw, 0, sizeof(pause_frame_raw));
		
		if(check_file_extension(file_path, _T(".ogv"))) {
			FILEIO* fio = new FILEIO();
			if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
				uint8 buffer[0x1000+1];
				fio->Fread(buffer, sizeof(buffer), 1);
				fio->Fclose();
				buffer[0x1000] = 0;
				
				for(int i = 0; i < 0x1000; i++) {
					char *top = (char *)(buffer + i), tmp[128];
					if(_strnicmp(top, "chapter:", 8) == 0) {
						top += 8;
						for(int j = 0;;) {
							char c = *top++;
							if(c >= '0' && c <= '9') {
								tmp[j++] = c;
							} else if(c != ' ') {
								tmp[j] = '\0';
								break;
							}
						}
						int track = atoi(tmp);
						for(int j = 0;;) {
							char c = *top++;
							if(c >= '0' && c <= '9') {
								tmp[j++] = c;
							} else if(c != ' ') {
								tmp[j] = '\0';
								break;
							}
						}
						if(track >= 0 && track <= MAX_TRACKS) {
							if(track > num_tracks) {
								num_tracks = track;
							}
							track_frame_raw[track] = atoi(tmp);
							emu->out_debug_log("LD700: TRACK %d: %d\n", track, track_frame_raw[track]);
						}
					} else if(_strnicmp(top, "stop:", 5) == 0) {
						top += 5;
						for(int j = 0;;) {
							char c = *top++;
							if(c >= '0' && c <= '9') {
								tmp[j++] = c;
							} else if(c != ' ') {
								tmp[j] = '\0';
								break;
							}
						}
						if(num_pauses < MAX_PAUSES) {
							pause_frame_raw[num_pauses] = atoi(tmp) > 300 ? atoi(tmp) : 285;
							emu->out_debug_log("LD700: PAUSE %d\n", pause_frame_raw[num_pauses]);
							num_pauses++;
						}
					} else if(_strnicmp(top, "ENCODER=", 8) == 0) {
						break;
					}
				}
			}
			delete fio;
		} else {
			_TCHAR ini_path[_MAX_PATH];
			_stprintf_s(ini_path, _MAX_PATH, _T("%s.ini"), get_file_path_without_extensiton(file_path));
			emu->out_debug_log("LD700: OPEN INI PATH=%s\n", ini_path);
			
			for(int i = 0; i <= MAX_TRACKS; i++) {
				_TCHAR name[64];
				_stprintf_s(name, 64, _T("chapter%d"), i);
				int value = GetPrivateProfileInt(_T("Location"), name, -1, ini_path);
				if(value < 0) {
					break;
				} else {
					track_frame_raw[i] = value;
					num_tracks = i;
				}
			}
			for(int i = 0; i < MAX_PAUSES; i++) {
				_TCHAR name[64];
				_stprintf_s(name, 64, _T("stop%d"), i);
				int value = GetPrivateProfileInt(_T("Location"), name, -1, ini_path);
				if(value < 0) {
					break;
				} else {
					pause_frame_raw[num_pauses++] = value;
				}
			}
		}
		for(int i = 1; i < num_tracks; i++) {
			if(track_frame_raw[i] == 0) {
				track_frame_raw[i] = track_frame_raw[i - 1];
			}
		}
		set_status(STATUS_STOP);
	} else {
		close_disc();
	}
}

void LD700::close_disc()
{
	emu->close_movie_file();
	num_tracks = -1;
	set_status(STATUS_EJECT);
}

bool LD700::disc_inserted()
{
	return (status != STATUS_EJECT);
}

void LD700::initialize_sound(int rate, int samples)
{
	mix_buffer_l = (int16 *)malloc(samples * 2 * sizeof(int16));
	mix_buffer_r = (int16 *)malloc(samples * 2 * sizeof(int16));
	mix_buffer_length = samples * 2;
	register_event(this, EVENT_MIX, 1000000. / (double)rate, true, NULL);
}

void LD700::mix(int32* buffer, int cnt)
{
	int16 sample_l = 0, sample_r = 0;
	for(int i = 0; i < cnt; i++) {
		if(i < mix_buffer_ptr) {
			sample_l = mix_buffer_l[i];
			sample_r = mix_buffer_r[i];
		}
		*buffer += sample_l;
		*buffer += sample_r;
	}
	if(cnt < mix_buffer_ptr) {
		memmove(mix_buffer_l, mix_buffer_l + cnt, (mix_buffer_ptr - cnt) * sizeof(int16));
		memmove(mix_buffer_r, mix_buffer_r + cnt, (mix_buffer_ptr - cnt) * sizeof(int16));
		mix_buffer_ptr -= cnt;
	} else {
		mix_buffer_ptr = 0;
	}
}

void LD700::movie_sound_callback(uint8 *buffer, long size)
{
	if(status == STATUS_PLAY) {
		int16 *buffer16 = (int16 *)buffer;
		size /= 2;
		for(int i = 0; i < size; i += 2) {
			sound_buffer_l->write(buffer16[i]);
			sound_buffer_r->write(buffer16[i + 1]);
			signal_buffer->write(buffer16[i + 1]);
		}
		if(signal_buffer->count() >= emu->get_movie_sound_rate() / 2) {
			signal_buffer_ok = true;
		}
	}
}
