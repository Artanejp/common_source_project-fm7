/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2016.03.06-

	[ SCSI CD-ROM drive ]
*/

#include "scsi_cdrom.h"
#include "../types/util_sound.h"

#include "../fifo.h"

#include <algorithm>
#include <cctype> 

#define CDDA_OFF	0
#define CDDA_PLAYING	1
#define CDDA_PAUSED	2

#define MODE_REPEAT	1
#define MODE_INTERRUPT	2
#define MODE_NO_REPEAT	3

// 0-99 is reserved for SCSI_DEV class
#define EVENT_CDDA						100
#define EVENT_CDDA_DELAY_PLAY			101
#define EVENT_CDROM_SEEK_SCSI			102
#define EVENT_CDROM_DELAY_INTERRUPT_ON	103
#define EVENT_CDROM_DELAY_INTERRUPT_OFF	104
#define EVENT_CDDA_DELAY_STOP			105

void SCSI_CDROM::initialize()
{
	SCSI_DEV::initialize();
	fio_img = new FILEIO();
	__CDROM_DEBUG_LOG = osd->check_feature(_T("_CDROM_DEBUG_LOG"));
	
	if(44100 % emu->get_sound_rate() == 0) {
		mix_loop_num = 44100 / emu->get_sound_rate();
	} else {
		mix_loop_num = 0;
	}
	event_cdda = -1;
	event_cdda_delay_play = -1;
	event_delay_interrupt = -1;
	cdda_start_frame = cdda_start_pregap = 0;
	cdda_end_frame = 0;
	cdda_playing_frame = 0;
	cdda_status = CDDA_OFF;
	cdda_play_mode = 0;
	
	is_cue = false;
	current_track = 0;
	read_sectors = 0;
	for(int i = 0; i < 99; i++) {
		memset(track_data_path[i], 0x00, _MAX_PATH * sizeof(_TCHAR));
	}
}

void SCSI_CDROM::out_debug_log(const _TCHAR *format, ...)
{
	if(!(__CDROM_DEBUG_LOG) && !(_OUT_DEBUG_LOG)) return;
	va_list args;
	_TCHAR _tmps[4096] = {0};
	_TCHAR _domain[256] = {0};
	my_sprintf_s(_domain, sizeof(_domain) / sizeof(_TCHAR), _T("[SCSI_CDROM:ID=%d]"), scsi_id);
	va_start(args, format);
	vsnprintf(_tmps, sizeof(_tmps) / sizeof(_TCHAR), format, args);
	va_end(args);
	DEVICE::out_debug_log(_T("%s %s"), _domain, _tmps);
}

void SCSI_CDROM::release()
{
	if(fio_img->IsOpened()) {
		fio_img->Fclose();
	}
	delete fio_img;
	SCSI_DEV::release();
}

void SCSI_CDROM::reset()
{
	touch_sound();
	if(event_delay_interrupt != -1) cancel_event(this, event_delay_interrupt);
	if(event_cdda_delay_play != -1) cancel_event(this, event_cdda_delay_play);
	if(event_cdda != -1) cancel_event(this, event_cdda);
	event_cdda = -1;
	event_cdda_delay_play = -1;
	event_delay_interrupt = -1;
	SCSI_DEV::reset();
	read_mode = false;
	set_cdda_status(CDDA_OFF);
	read_sectors = 0;
	// Q: Does not seek to track 0? 20181118 K.O
	//current_track = 0;

}

void SCSI_CDROM::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool _b = ((data & mask) != 0);
	switch(id) {
	case SIG_SCSI_CDROM_CDDA_STOP:
		if(cdda_status != CDDA_OFF) {
			if(_b) set_cdda_status(CDDA_OFF);
		}
		break;
	case SIG_SCSI_CDROM_CDDA_PLAY:
		if(cdda_status != CDDA_PLAYING) {
			if(_b) set_cdda_status(CDDA_PLAYING);
		}
		break;
	case SIG_SCSI_CDROM_CDDA_PAUSE:
		if(cdda_status != CDDA_PAUSED) {
			if(_b) set_cdda_status(CDDA_PAUSED);
		}
		break;
	default:
		SCSI_DEV::write_signal(id, data, mask);
		break;
	}
}

uint32_t SCSI_CDROM::read_signal(int id)
{
	switch(id) {
	case SIG_SCSI_CDROM_PLAYING:
		return (cdda_status == CDDA_PLAYING && cdda_play_mode == MODE_INTERRUPT) ? 0xffffffff : 0;
		
	case SIG_SCSI_CDROM_SAMPLE_L:
		return (uint32_t)abs(cdda_sample_l);
		
	case SIG_SCSI_CDROM_SAMPLE_R:
		return (uint32_t)abs(cdda_sample_r);
	}
	return SCSI_DEV::read_signal(id);
}

void SCSI_CDROM::event_callback(int event_id, int err)
{
	switch (event_id) {
	case EVENT_CDROM_DELAY_INTERRUPT_ON:
		write_signals(&outputs_done, 0xffffffff);
		event_delay_interrupt = -1;
		break;
	case EVENT_CDROM_DELAY_INTERRUPT_OFF:
		write_signals(&outputs_done, 0x00000000);
		event_delay_interrupt = -1;
		break;
	case EVENT_CDDA_DELAY_PLAY:
		if(cdda_status != CDDA_PLAYING) {
			set_cdda_status(CDDA_PLAYING);
		}
		event_cdda_delay_play = -1;
		break;
	case EVENT_CDROM_SEEK_SCSI:
		seek_time = 10.0;
		event_cdda_delay_play = -1;
		SCSI_DEV::start_command();
		break;
	case EVENT_CDDA:
		if(event_cdda_delay_play > -1) return; // WAIT for SEEK COMPLETED
		// read 16bit 2ch samples in the cd-da buffer, called 44100 times/sec
		
//		cdda_sample_l = (int)(int16_t)(cdda_buffer[cdda_buffer_ptr + 0] + cdda_buffer[cdda_buffer_ptr + 1] * 0x100);
//		cdda_sample_r = (int)(int16_t)(cdda_buffer[cdda_buffer_ptr + 2] + cdda_buffer[cdda_buffer_ptr + 3] * 0x100);
//		if(cdda_buffer_ptr < max_logical_block * 2532) {
			pair16_t tmp_l, tmp_r;
			tmp_l.read_2bytes_le_from(&cdda_buffer[cdda_buffer_ptr + 0]);
			tmp_r.read_2bytes_le_from(&cdda_buffer[cdda_buffer_ptr + 2]);
			cdda_sample_l = tmp_l.sw;
			cdda_sample_r = tmp_r.sw;
//		} else {
//			cdda_sample_l = cdda_sample_r = 0;
//		}
		// ToDo: CLEAR IRQ Line (for PCE)
		if((cdda_buffer_ptr += 4) % 2352 == 0) {
			// one frame finished
			cdda_playing_frame++;
			if((is_cue) && (cdda_playing_frame != cdda_end_frame)) {
				if((cdda_playing_frame >= toc_table[current_track + 1].index0) && (track_num > (current_track + 1))) {
					get_track_by_track_num(current_track + 1); // Note: Increment current track
					if(fio_img->IsOpened()) {
						//fio_img->Fseek(0, FILEIO_SEEK_SET);
						read_sectors = fio_img->Fread(cdda_buffer, 2352 * sizeof(uint8_t), array_length(cdda_buffer) / 2352);
					} else {
						// Seek error (maybe end of disc)
						read_sectors = 0;
						memset(cdda_buffer, 0x00, sizeof(cdda_buffer));
					}
					cdda_buffer_ptr = 0;
					access = false;
				}
			}
			if(cdda_playing_frame >= min(cdda_end_frame, max_logical_block)) {
				out_debug_log(_T("Reaches to the end of track.(FRAME %d). START_FRAME=%d END_FRAME=%d REPEAT=%s INTERRUPT=%s\n"),
							  cdda_playing_frame, cdda_start_frame, cdda_end_frame, 
							  (cdda_play_mode == MODE_REPEAT) ? _T("YES") : _T("NO"),
							  (cdda_play_mode == MODE_INTERRUPT) ? _T("YES") : _T("NO"));
				// reached to end frame
				if(cdda_play_mode == MODE_REPEAT) {
					// reload buffer
					// Restart.
					if(cdda_start_frame < max_logical_block) {
						if(is_cue) {
							int trk = get_track(cdda_start_frame);
							if(fio_img->IsOpened()) {
								fio_img->Fseek((cdda_start_frame - toc_table[trk].lba_offset) * 2352, FILEIO_SEEK_SET);
							}
//						fio_img->Fclose();
							//current_track = 0;
							//int trk = get_track(cdda_start_frame);
//						int trk = current_track;
//						fio_img->Fseek((cdda_start_frame - toc_table[trk].lba_offset) * 2352, FILEIO_SEEK_SET);
						} else {
							fio_img->Fseek(cdda_start_frame * 2352, FILEIO_SEEK_SET);
						}
						if(fio_img->IsOpened()) {
							read_sectors = fio_img->Fread(cdda_buffer, 2352 * sizeof(uint8_t) , array_length(cdda_buffer) / 2352);
						} else {
							read_sectors = 0;
							memset(cdda_buffer, 0x00, sizeof(cdda_buffer));
						}
					} else {
						read_sectors = 0;
						memset(cdda_buffer, 0x00, sizeof(cdda_buffer));
					}
					cdda_buffer_ptr = 0;
					cdda_playing_frame = cdda_start_frame;
					access = true;
				} else {
					// Stop
					if(event_cdda_delay_play >= 0) cancel_event(this, event_cdda_delay_play);
					memset(cdda_buffer, 0x00, sizeof(cdda_buffer));
					register_event(this, EVENT_CDDA_DELAY_STOP, 1000.0, false, &event_cdda_delay_play);

					//set_cdda_status(CDDA_OFF);
					//register_event(this, EVENT_CDDA_DELAY_STOP, 1000.0, false, &event_cdda_delay_play);
					access = false;
				}
			} else if((cdda_buffer_ptr % 2352) == 0) {
				// refresh buffer
				read_sectors--;
				if(read_sectors <= 0) {
					if(fio_img->IsOpened()) {
						read_sectors = fio_img->Fread(cdda_buffer, 2352 * sizeof(uint8_t), array_length(cdda_buffer) / 2352);
					} else {
						read_sectors = 0;
						memset(cdda_buffer, 0x00, sizeof(cdda_buffer));
					}
//					read_sectors = fio_img->Fread(cdda_buffer, 2352 * sizeof(uint8_t), array_length(cdda_buffer) / 2352);
					cdda_buffer_ptr = 0;
					access = false;
				} else {
				}
			}
		}
		break;
	case EVENT_CDDA_DELAY_STOP:
		if(cdda_play_mode == MODE_INTERRUPT) {
			write_signals(&outputs_done, 0xffffffff);
		}
		set_cdda_status(CDDA_OFF);
		event_cdda_delay_play = -1;
		break;			
	default:
		SCSI_DEV::event_callback(event_id, err);
		break;
	}
}

void SCSI_CDROM::set_cdda_status(uint8_t status)
{
	if(status == CDDA_PLAYING) {
		if(mix_loop_num == 0) {
			if(event_cdda == -1) {
				register_event(this, EVENT_CDDA, 1000000.0 / 44100.0, true, &event_cdda);
			}
		}
		if(cdda_status != CDDA_PLAYING) {
			//// Notify to release bus.
			write_signals(&outputs_done, 0x00000000);
			if(cdda_status == CDDA_OFF) {
				//get_track_by_track_num(current_track); // Re-Play
				//memset(cdda_buffer, 0x00, sizeof(cdda_buffer));
				cdda_playing_frame = cdda_start_frame;
				get_track(cdda_playing_frame);
				cdda_buffer_ptr = 0;
				access = false;
			} else if(cdda_status == CDDA_PAUSED) {
				// Unpause
				access = true;
			}
			touch_sound();
			set_realtime_render(this, true);
			out_debug_log(_T("Play CDDA from %s.\n"), (cdda_status == CDDA_PAUSED) ? _T("PAUSED") : _T("STOPPED"));
		}
	} else {
		if(event_cdda != -1) {
			cancel_event(this, event_cdda);
			event_cdda = -1;
		}
		if(cdda_status == CDDA_PLAYING) {
			// Notify to release bus.
			write_signals(&outputs_done, 0x00000000);
			//if(event_delay_interrupt >= 0) cancel_event(this, event_delay_interrupt);
			//register_event(this, EVENT_CDROM_DELAY_INTERRUPT_OFF, 1.0e6 / (44100.0 * 2352), false, &event_delay_interrupt);
			if(status == CDDA_OFF) {
				memset(cdda_buffer, 0x00, sizeof(cdda_buffer));
				cdda_buffer_ptr = 0;
				read_sectors = 0;
				//cdda_repeat = false; // OK?
				//if(is_cue) {
				//	if(fio_img->IsOpened()) fio_img->Fclose();
				//}
				//current_track = 0;
			}
			touch_sound();
			set_realtime_render(this, false);
			out_debug_log(_T("%s playing CDDA.\n"), (status == CDDA_PAUSED) ? _T("PAUSE") : _T("STOP"));
		}
	}
	cdda_status = status;
}

void SCSI_CDROM::reset_device()
{
	set_cdda_status(CDDA_OFF);
	SCSI_DEV::reset_device();
}

bool SCSI_CDROM::is_device_ready()
{
	return mounted();
}

int SCSI_CDROM::get_command_length(int value)
{
	switch(value) {
	case 0xd8:
	case 0xd9:
	case 0xda:
	case 0xdd:
	case 0xde:
		return 10;
	}
	return SCSI_DEV::get_command_length(value);
}

void SCSI_CDROM::get_track_by_track_num(int track)
{
	if((track <= 0) || (track >= track_num)) {
		if(is_cue) {
			if(fio_img->IsOpened()) fio_img->Fclose();
		}
		//if(track <= 0) current_track = 0;
		//if(track >= track_num)current_track = track_num;
		current_track = 0;
		return;
	}
	if(is_cue) {
		// ToDo: Apply audio with some codecs.
		if((current_track != track) || !(fio_img->IsOpened())){
			if(fio_img->IsOpened()) {
				fio_img->Fclose();
			}
			out_debug_log(_T("LOAD TRK #%02d from %s\n"), track, track_data_path[track - 1]);
			
			if((track > 0) && (track < 100) && (track < track_num)) {
				if((strlen(track_data_path[track - 1]) <= 0) ||
				   !(fio_img->Fopen(track_data_path[track - 1], FILEIO_READ_BINARY))) {
					track = 0;
				}	
			} else {
				track = 0;
			}
		}
	}
	current_track = track;
}

// Detect only track num.
int SCSI_CDROM::get_track_noop(uint32_t lba)
{
	int track = 0;
	for(int i = 0; i < track_num; i++) {
		if(lba >= toc_table[i].index0) {
			track = i;
		} else {
			break;
		}
	}
	return track;
}	

int SCSI_CDROM::get_track(uint32_t lba)
{
	int track = 0;
	track = get_track_noop(lba);
	get_track_by_track_num(track);
	return track;
}

uint32_t SCSI_CDROM::get_image_cur_position()
{
	uint32_t frame = 0;
	int track = current_track;
	if(is_device_ready()) {
		if(fio_img->IsOpened()) {
			uint32_t cur_position = (uint32_t)(fio_img->Ftell());
			frame = cur_position / physical_block_size();
			if(is_cue) {
				frame = frame + toc_table[track].lba_offset;
			}
		} else {
			frame = 0;
		}
	}
	return frame;
}

double SCSI_CDROM::get_seek_time(uint64_t new_position, uint64_t length)
{
	// Backport from FM TOWNS.
	// 20240910 K.O
	uint32_t lba = (uint32_t)new_position; // OK?
	uint32_t cur_lba = get_image_cur_position();
	int distance;
	if((track_num <= 0) || !(mounted())) {
		return 400000; // 400msec
	}		

	if(lba > max_logical_block) lba = max_logical_block;
	if(lba > cur_lba) {
		distance = lba - cur_lba;
	} else {
		distance = cur_lba - lba;
	}
	if((cur_lba < 150) && (lba < 150)) {
		// Seek not effective.
		distance = 0;
	}
	
	double ratio = ((double)distance / 333000.0) / (double)physical_block_size(); // 333000: sectors in media
	return std::max(10.0, 400000.0 * 2.0 * ratio);
}

uint32_t SCSI_CDROM::lba_to_msf(uint32_t lba)
{
	uint8_t m = lba / (60 * 75);
	lba -= m * (60 * 75);
	uint8_t s = lba / 75;
	uint8_t f = lba % 75;

	return ((m / 10) << 20) | ((m % 10) << 16) | ((s / 10) << 12) | ((s % 10) << 8) | ((f / 10) << 4) | ((f % 10) << 0);
}

uint32_t SCSI_CDROM::lba_to_msf_alt(uint32_t lba)
{
	uint32_t ret = 0;
	ret |= ((lba / (60 * 75)) & 0xff) << 16;
	ret |= (((lba / 75) % 60) & 0xff) <<  8;
	ret |= ((lba % 75)        & 0xff) <<  0;
	return ret;
}

void SCSI_CDROM::start_command()
{
	touch_sound();
	//out_debug_log(_T("Command: #%02x %02x %02x %02x %02x %02x\n"), command[0], command[1], command[2], command[3], command[4], command[5]);
	switch(command[0]) {
	case SCSI_CMD_TST_U_RDY:
		read_mode = false;
		break;
	case SCSI_CMD_READ6:
		seek_time = get_seek_time((command[1] & 0x1f) * 0x10000 + command[2] * 0x100 + command[3]);
		set_cdda_status(CDDA_OFF);
		if(seek_time > 10.0) {
			if(event_cdda_delay_play >= 0) {
				cancel_event(this, event_cdda_delay_play);
				event_cdda_delay_play = -1;
			}
			register_event(this, EVENT_CDROM_SEEK_SCSI, seek_time, false, &event_cdda_delay_play);
			return;
		} else {
			seek_time = 10.0;
		}
		break;
	case SCSI_CMD_READ10:
	case SCSI_CMD_READ12:
		seek_time = get_seek_time(command[2] * 0x1000000 + command[3] * 0x10000 + command[4] * 0x100 + command[5]);
		set_cdda_status(CDDA_OFF);
		if(seek_time > 10.0) {
			if(event_cdda_delay_play >= 0) {
				cancel_event(this, event_cdda_delay_play);
				event_cdda_delay_play = -1;
			}
			register_event(this, EVENT_CDROM_SEEK_SCSI, seek_time - 10.0, false, &event_cdda_delay_play);
			return;
		} else {
			seek_time = 10.0;
		}
		break;
	case SCSI_CMD_MODE_SEL6:
		out_debug_log(_T("Command: NEC Read Mode Select 6-byte\n"), scsi_id);
		// start position
		set_cdda_status(CDDA_OFF);
//		position = (command[1] & 0x1f) * 0x10000 + command[2] * 0x100 + command[3];
//		position *= physical_block_size();
		position = 0;
		// transfer length
//		remain = command[4];// * logical_block_size();
		remain = 11;
		if(remain != 0) {
			// clear data buffer
			buffer->clear();
			// change to data in phase
			set_phase_delay(SCSI_PHASE_DATA_OUT, seek_time);
		} else {
			// transfer length is zero, change to status phase
			set_dat(SCSI_STATUS_GOOD);
			set_sense_code(SCSI_SENSE_NOSENSE);
			set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		}
		return;
		
	case 0xd8:
		out_debug_log(_T("Command: NEC Set Audio Playback Start Position CMD=%02x ARG=%02x %02x %02x %02x %02x\n"), command[9], command[1], command[2], command[3], command[4], command[5]);
		if(is_device_ready()) {
			bool req_play = false;
			if(command[2] == 0 && command[3] == 0 && command[4] == 0) {
				// stop cd-da if all params are zero
				cdda_start_frame = cdda_start_pregap = 0;
				cdda_end_frame = toc_table[track_num].index0; // end of disc
				double seek_time = get_seek_time(cdda_end_frame);
				req_play = false;
				if(event_cdda_delay_play >= 0) cancel_event(this, event_cdda_delay_play);
				register_event(this, EVENT_CDDA_DELAY_STOP, (seek_time > 10.0) ? seek_time : 10.0, false, &event_cdda_delay_play);
			} else {
				uint32_t seek_offset = 0;
				switch(command[9] & 0xc0) {
				case 0x00:
					cdda_start_frame = (command[2] << 16) | (command[3] << 8) | command[4];
					break;
				case 0x40:
					{
						if(__SCSI_DEBUG_LOG) {
							this->out_debug_log(_T("[SCSI_DEV:ID=%d] M:S:F=%02x:%02x:%02x\n"), scsi_id,
								command[2], command[3], command[4]);
						}
						uint8_t m = FROM_BCD(command[2]);
						uint8_t s = FROM_BCD(command[3]);
						uint8_t f = FROM_BCD(command[4]);
						cdda_start_frame = f + 75 * (s + m * 60);
					}
					break;
				case 0x80:
					{
						int8_t track = FROM_BCD(command[2] - 1);
						cdda_start_frame = toc_table[track].index1;
					}
					break;
				default:
					cdda_start_frame = 0;
					break;
				}
				{
					// PCE tries to be clever here and set (start of track + track pregap size) to skip the pregap
					// (I guess it wants the TOC to have the real start sector for data tracks and the start of the pregap for audio?)
					int track_bak = current_track;
					int track = get_track(cdda_start_frame);
#if 1					
					if(is_cue) {
						if(cdda_start_frame >= (toc_table[track].pregap + toc_table[track].index0)) {
							cdda_start_frame -= toc_table[track].pregap;
						}
					} else {
						if(cdda_start_frame >= toc_table[track].pregap) {
							cdda_start_frame -= toc_table[track].pregap;
						}
					}
#endif
					req_play = true;
#if 1
					if(cdda_start_frame < toc_table[track].index0) {
						cdda_start_frame = toc_table[track].index0; // don't play pregap
					} else if(cdda_start_frame > max_logical_block) {
						cdda_start_frame = 0;
						req_play = false;
					}
#endif
				}
				double delay_time = get_seek_time(cdda_start_frame);

				if(cdda_end_frame <= toc_table[current_track].index0) {
					cdda_end_frame = toc_table[current_track + 1].index0; // don't play pregap
				} else if(cdda_end_frame > toc_table[current_track + 1].index0) {
					cdda_end_frame = toc_table[current_track + 1].index0; // don't play pregap
				}
#if 1
				if(cdda_start_frame < max_logical_block) {
					if(is_cue) {
						fio_img->Fseek((cdda_start_frame - toc_table[current_track].index0) * 2352, FILEIO_SEEK_SET);
					} else {
						fio_img->Fseek(cdda_start_frame * 2352, FILEIO_SEEK_SET);
					}
				}
#endif
				if(cdda_end_frame <= toc_table[current_track].index0) {
					cdda_end_frame = toc_table[current_track + 1].index0; // don't play pregap
				} else if(cdda_end_frame > toc_table[current_track + 1].index0) {
					cdda_end_frame = toc_table[current_track + 1].index0; // don't play pregap
				}
				if(cdda_start_frame < max_logical_block) {
					read_sectors = fio_img->Fread(cdda_buffer, 2352 * sizeof(uint8_t), array_length(cdda_buffer) / 2352);
				} else {
					memset(cdda_buffer, 0, sizeof(cdda_buffer));
					read_sectors = 0;
				}
				if((command[1] & 3) != 0) {
					cdda_play_mode = ((command[1] & 3) == 1) ? MODE_REPEAT : MODE_NO_REPEAT;
//					cdda_repeat = ((command[1] & 3) == 1);
					// read buffer
					if(req_play) {
						cdda_buffer_ptr = 0;
						cdda_playing_frame = cdda_start_frame;
						access = true;
						if(event_cdda_delay_play >= 0) {
							cancel_event(this, event_cdda_delay_play);
							event_cdda_delay_play = -1;
						}
						register_event(this, EVENT_CDDA_DELAY_PLAY, delay_time, false, &event_cdda_delay_play);
						//set_cdda_status(CDDA_PLAYING);
						cdda_play_mode = (command[1] & 2) ? MODE_INTERRUPT : MODE_NO_REPEAT;
					} else {
						set_cdda_status(CDDA_OFF);
						cdda_play_mode = MODE_NO_REPEAT;
					}
				}else {
					//cdda_repeat = false;
					cdda_play_mode = MODE_NO_REPEAT;
					if(cdda_status == CDDA_PAUSED) {
						cdda_end_frame = toc_table[track_num].index0; // end of disc
						cdda_start_frame = toc_table[track_num].index0; // end of disc
						cdda_playing_frame = 0;
						set_cdda_status(CDDA_OFF);
					} else{
						set_cdda_status(CDDA_PAUSED);
					}
				}
//				cdda_interrupt = ((command[1] & 3) == 2);
				
				// change to status phase
				set_dat(SCSI_STATUS_GOOD);
				// From Mame 0.203
				if(event_delay_interrupt >= 0) cancel_event(this, event_delay_interrupt);
				register_event(this, EVENT_CDROM_DELAY_INTERRUPT_ON, delay_time, false, &event_delay_interrupt);
				//set_phase_delay(SCSI_PHASE_STATUS, delay_time + 10.0);
				//write_signals(&outputs_done, 0xffffffff);
				set_phase_delay(SCSI_PHASE_STATUS, delay_time);
				if(__SCSI_DEBUG_LOG) {
					uint32_t s_msf = lba_to_msf(cdda_start_frame);
					uint32_t e_msf = lba_to_msf(cdda_end_frame);
					this->out_debug_log(_T("[SCSI_DEV:ID=%d] Start=%02x:%02x:%02x End=%02x:%02x:%02x Mode=%d\n"), scsi_id,
										(s_msf >> 16) & 0xff, (s_msf >> 8) & 0xff, s_msf & 0xff,
										(e_msf >> 16) & 0xff, (e_msf >> 8) & 0xff, e_msf & 0xff, cdda_play_mode);
				}
				return;
			}
		}
		// change to status phase
		set_dat(is_device_ready() ? SCSI_STATUS_GOOD : SCSI_STATUS_CHKCOND);
		// From Mame 0.203
		//if(is_device_ready()) write_signals(&outputs_done, 0xffffffff);
		set_phase_delay(SCSI_PHASE_STATUS, 10.0);

		return;
		
	case 0xd9:
		out_debug_log(_T("Command: NEC Set Audio Playback End Position CMD=%02x ARG=%02x %02x %02x %02x %02x\n"), scsi_id, command[9], command[1], command[2], command[3], command[4], command[5]);
		if(is_device_ready()) {
			
			switch(command[9] & 0xc0) {
			case 0x00:
				cdda_end_frame = (command[3] << 16) | (command[4] << 8) | command[5];
				break;
			case 0x40:
				{
					if(__SCSI_DEBUG_LOG) {
						this->out_debug_log(_T("[SCSI_DEV:ID=%d] M:S:F=%02x:%02x:%02x\n"), scsi_id,
							command[2], command[3], command[4]);
					}
					uint8_t m = FROM_BCD(command[2]);
					uint8_t s = FROM_BCD(command[3]);
					uint8_t f = FROM_BCD(command[4]);
					cdda_end_frame = f + 75 * (s + m * 60);
					
					// PCE tries to be clever here and set (start of track + track pregap size) to skip the pregap
					// (I guess it wants the TOC to have the real start sector for data tracks and the start of the pregap for audio?)
					
#if 0					
					int track = current_track;
					cdda_playing_frame = cdda_start_frame;
					if(cdda_end_frame > toc_table[track + 1].index1 && (cdda_end_frame - toc_table[track].pregap) <= toc_table[track + 1].index1) {
						cdda_end_frame = toc_table[track + 1].index1;
					}
					cdda_buffer_ptr = 0;
#else
					cdda_end_frame -= min(cdda_start_pregap, cdda_end_frame);
#endif
				}
				break;
			case 0x80:
				{
					if(__SCSI_DEBUG_LOG) {
						this->out_debug_log(_T("[SCSI_DEV:ID=%d] Track=%02x\n"), scsi_id, command[2]);
					}
					int _track = FROM_BCD(command[2]) - 1;
					if((_track >= 0) && ((_track + 1) <= track_num)) {
						//cdda_end_frame = toc_table[_track].index0;
						if(is_cue) {
							if(current_track != _track) {
								get_track_by_track_num(_track);
								cdda_start_frame = toc_table[_track].index1;
//								cdda_start_frame = toc_table[_track].index0;
								cdda_end_frame = toc_table[_track].lba_size + toc_table[_track].lba_offset;
								cdda_playing_frame = cdda_start_frame;
								if(is_cue) {
									fio_img->Fseek((cdda_start_frame - toc_table[current_track].lba_offset) * 2352, FILEIO_SEEK_SET);
								} else {
									fio_img->Fseek(cdda_start_frame * 2352, FILEIO_SEEK_SET);
								}
								read_sectors = fio_img->Fread(cdda_buffer, 2352 * sizeof(uint8_t), array_length(cdda_buffer) / 2352);
								cdda_buffer_ptr = 0;
							}
						}
					} else {
//						cdda_end_frame = toc_table[track_num].index0;
						cdda_start_frame = toc_table[track_num].index0;
					}
				}
				break;
			default:
				cdda_end_frame = toc_table[track_num].index0;
//				cdda_start_frame = 0;
				break; // ToDo
			}
			cdda_start_frame = min(cdda_start_frame, max_logical_block);
				
			if((command[1] & 3) != 0) {
//				cdda_play_mode = ((command[1] & 3) == 2) ? MODE_INTERRUPT : (((command[1] & 3) == 1) ? MODE_NO_REPEAT : MODE_NO_REPEAT);
//				cdda_repeat = ((command[1] & 3) == 1);
//				cdda_interrupt = ((command[1] & 3) == 2);
				cdda_end_frame = min(cdda_end_frame, max_logical_block);
				cdda_play_mode = command[1] & 3;
				if(cdda_play_mode != 0) {
					if(event_cdda_delay_play >= 0) cancel_event(this, event_cdda_delay_play);
					register_event(this, EVENT_CDDA_DELAY_PLAY, 10.0, false, &event_cdda_delay_play);
				} else {
					set_cdda_status(CDDA_OFF);
				}
			}/* else {
					cdda_end_frame = toc_table[min(get_track(cdda_start_frame) + 1, track_num)].index0; // end of this track
 					set_cdda_status(CDDA_PAUSED);
					cdda_play_mode = MODE_NO_REPEAT;
			}*/
			if(__SCSI_DEBUG_LOG) {
				uint32_t s_msf = lba_to_msf(cdda_start_frame);
				uint32_t e_msf = lba_to_msf(cdda_end_frame);
				this->out_debug_log(_T("[SCSI_DEV:ID=%d] Start=%02x:%02x:%02x End=%02x:%02x:%02x Mode=%d\n"), scsi_id,
					(s_msf >> 16) & 0xff, (s_msf >> 8) & 0xff, s_msf & 0xff,
					(e_msf >> 16) & 0xff, (e_msf >> 8) & 0xff, e_msf & 0xff, cdda_play_mode);
			}
		}
		// change to status phase
		set_dat(is_device_ready() ? SCSI_STATUS_GOOD : SCSI_STATUS_CHKCOND);
		if(is_device_ready()) {
			write_signals(&outputs_done, 0xffffffff);
			//if(event_delay_interrupt >= 0) cancel_event(this, event_delay_interrupt);
			//register_event(this, EVENT_CDROM_DELAY_INTERRUPT_ON, 10.0, false, &event_delay_interrupt);

		}
		set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		return;
		
	case 0xda:
		out_debug_log(_T("Command: NEC Pause\n"));
		if(is_device_ready()) {
			if(cdda_status == CDDA_PLAYING) {
				set_cdda_status(CDDA_PAUSED);
			}
		}
		// change to status phase
		set_dat(is_device_ready() ? SCSI_STATUS_GOOD : SCSI_STATUS_CHKCOND);
		if(is_device_ready()) write_signals(&outputs_done, 0xffffffff);
		set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		return;
		
	case 0xdd:
		out_debug_log(_T("Command: NEC Read Sub Channel Q\n"));
		if(is_device_ready()) {
			// create track info
			uint32_t frame = (cdda_status == CDDA_OFF) ? cdda_start_frame : cdda_playing_frame;
			uint32_t msf_abs = lba_to_msf_alt(frame);
			int track = min(get_track(frame), track_num - 1);
			double delay_time = 10.0;
			if((cdda_status == CDDA_OFF) && (toc_table[track].is_audio)) { // OK? (or force ERROR) 20181120 K.O
				//set_cdda_status(CDDA_PLAYING);
				delay_time = get_seek_time(frame);
				//delay_time = 10.0;
				if(event_cdda_delay_play >= 0) {
					cancel_event(this, event_cdda_delay_play);
					event_cdda_delay_play = -1;
				}
				register_event(this, EVENT_CDDA_DELAY_PLAY, delay_time, false, &event_cdda_delay_play);
			}
			uint32_t msf_rel = lba_to_msf_alt(frame - toc_table[track].index0);
			buffer->clear();
			buffer->write((cdda_status == CDDA_PLAYING) ? 0x00 : (cdda_status == CDDA_PAUSED) ? 0x02 : 0x03);
			buffer->write(0x01 | (toc_table[track].is_audio ? 0x00 : 0x40));
			buffer->write(TO_BCD(track + 1));		// Track
			buffer->write(0x01);				// Index
			buffer->write(TO_BCD((msf_rel >> 16) & 0xff));	// M (relative)
			buffer->write(TO_BCD((msf_rel >>  8) & 0xff));	// S (relative)
			buffer->write(TO_BCD((msf_rel >>  0) & 0xff));	// F (relative)
			buffer->write(TO_BCD((msf_abs >> 16) & 0xff));	// M (absolute)
			buffer->write(TO_BCD((msf_abs >>  8) & 0xff));	// S (absolute)
			buffer->write(TO_BCD((msf_abs >>  0) & 0xff));	// F (absolute)
			// transfer length
			remain = buffer->count();
			// set first data
			set_dat(buffer->read());
			// change to data in phase
			set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
		} else {
			// change to status phase
			set_dat(is_device_ready() ? SCSI_STATUS_GOOD : SCSI_STATUS_CHKCOND);
			set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		}
		return;
		
	case 0xde:
		out_debug_log(_T("Command: NEC Get Dir Info\n"));
		if(is_device_ready()) {
			buffer->clear();
			out_debug_log(_T("CMD=%02x ARG=%02x \n"), command[1], command[2]);
			switch(command[1]) {
			case 0x00:      /* Get first and last track numbers */
				buffer->write(TO_BCD(1));
				buffer->write(TO_BCD(track_num));
				// PC-8801 CD BIOS invites 4 bytes ?
				buffer->write(0);
				buffer->write(0);
				break;
			case 0x01:      /* Get total disk size in MSF format */
				{
					uint32_t msf = lba_to_msf(toc_table[track_num].index0 + 150);
					buffer->write((msf >> 16) & 0xff);
					buffer->write((msf >>  8) & 0xff);
					buffer->write((msf >>  0) & 0xff);
					// PC-8801 CD BIOS invites 4 bytes ?
					buffer->write(0);
				}
				break;
			case 0x02:      /* Get track information */
				if(command[2] == 0xaa) {
					uint32_t msf = lba_to_msf(toc_table[track_num].index0 + 150);
					buffer->write((msf >> 16) & 0xff);
					buffer->write((msf >>  8) & 0xff);
					buffer->write((msf >>  0) & 0xff);
					buffer->write(0x04); // correct ?
				} else {
					int track = max(FROM_BCD(command[2]), 1);
					uint32_t frame = toc_table[track].index0;
					// PCE wants the start sector for data tracks to *not* include the pregap
					if(!toc_table[track].is_audio) {
						frame += toc_table[track].pregap;
					}
					get_track_by_track_num(track);
					
//					uint32_t msf = lba_to_msf(toc_table[track].index1 + 150);
					uint32_t msf = lba_to_msf(frame + 150);
					buffer->write((msf >> 16) & 0xff); // M
					buffer->write((msf >>  8) & 0xff); // S
					buffer->write((msf >>  0) & 0xff); // F
					buffer->write(toc_table[track].is_audio ? 0x00 : 0x04);
				}
				break;
			}
			// transfer length
			remain = buffer->count();
			// set first data
			set_dat(buffer->read());
			// change to data in phase
			set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
		} else {
			// change to status phase
			set_dat(is_device_ready() ? SCSI_STATUS_GOOD : SCSI_STATUS_CHKCOND);
			set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		}
		return;
	case 0xff:
		// End of List
		set_dat(SCSI_STATUS_CHKCOND);
		return;
	}
	
	// start standard command
	SCSI_DEV::start_command();
}

bool SCSI_CDROM::read_buffer(int length)
{
	if(!(mounted()) || (track_num <= 0)) {
		set_sense_code(SCSI_SENSE_NOTREADY);
		return false;
	}
	uint32_t offset = (uint32_t)(position % physical_block_size());
	long relative_position = (long)position;
	// ToDo: Mixed physical block size. Will re-implement.
	if(is_cue) { // OK?
		uint32_t target_lba = (uint32_t)(position / physical_block_size());
		out_debug_log(_T("Seek to LBA %d LENGTH=%d\n"), target_lba, length);
		int track_tmp = get_track_noop(target_lba);
		if((track_tmp != current_track) || !(fio_img->IsOpened())) {
			get_track_by_track_num(track_tmp);
		}
		if(!(fio_img->IsOpened())) {
			set_sense_code(SCSI_SENSE_NOTREADY);
			return false;
		}
		if(current_track == 0) {
			set_sense_code(SCSI_SENSE_ILLGLBLKADDR); //SCSI_SENSE_SEEKERR
			out_debug_log(_T("Error on reading (ILLGLBLKADDR) at line %d\n"), __LINE__);
			return false;
		}
		relative_position = (long)position - (long)(toc_table[current_track].lba_offset * physical_block_size());
		offset = (uint32_t)(relative_position % physical_block_size());
		if(relative_position < 0) {
			set_sense_code(SCSI_SENSE_ILLGLBLKADDR); //SCSI_SENSE_SEEKERR
			out_debug_log(_T("Error on reading (ILLGLBLKADDR) at line %d\n"), __LINE__);
			return false;
		}
	}
	if(fio_img->Fseek(relative_position, FILEIO_SEEK_SET) != 0) {
		set_sense_code(SCSI_SENSE_ILLGLBLKADDR); //SCSI_SENSE_SEEKERR
		out_debug_log(_T("Error on reading (ILLGLBLKADDR) at line %d\n"), __LINE__);
		return false;
	}
	while(length > 0) {
		uint8_t tmp_buffer[2352];
//		int tmp_length = min(length, (int)sizeof(tmp_buffer));
		int tmp_length = 2352 - offset;
		
		if(fio_img->Fread(tmp_buffer, tmp_length, 1) != 1) {
			out_debug_log(_T("Error on reading (ILLGLBLKADDR) at line %d\n"), __LINE__);
			set_sense_code(SCSI_SENSE_ILLGLBLKADDR); //SCSI_SENSE_NORECORDFND
			return false;
		}
		for(int i = 0; i < tmp_length; i++) {
			if(offset >= 16 && offset < 16 + logical_block_size()) {
				int value = tmp_buffer[i];
				buffer->write(value);
				length--;
			}
			position++;
			offset = (offset + 1) % 2352;
//			if(offset == 0) {
//				if(length > 0) {
//					write_signals(&outputs_next_sector, 0xffffffff);
//				}					
//			}
		}
		access = true;
	}
//	write_signals(&outputs_completed, 0xffffffff);
	// Is This right? 20181120 K.O
	//write_signals(&outputs_done, 0xffffffff); // Maybe don't need "DONE SIGNAL" with reading DATA TRACK. 20181207 K.O
	set_sense_code(SCSI_SENSE_NOSENSE);
	return true;
}

bool SCSI_CDROM::write_buffer(int length)
{
	for(int i = 0; i < length; i++) {
		int value = buffer->read();
		if(command[0] == SCSI_CMD_MODE_SEL6) {
			if(i == 4) {
				out_debug_log(_T("NEC Read Mode = %02X\n"), value);
				read_mode = (value != 0);
			} else if(i == 10) {
				out_debug_log(_T("NEC Retry Count = %02X\n"), value);
			}
		}
		position++;
	}
	set_sense_code(SCSI_SENSE_NOSENSE);
	return true;
}

int get_frames_from_msf(const char *string)
{
	const char *ptr = string;
	int frames[3] = {0};
	int index = 0;
	
	while(1) {
		if(*ptr >= '0' && *ptr <= '9') {
			frames[index] = frames[index] * 10 + (*ptr - '0');
		} else if(*ptr == ':') {
			if(++index == 3) {
				// abnormal data
				break;
			}
		} else if(*ptr == '\r' || *ptr == '\n' || *ptr == '\0') {
			// end of line
			break;
		}
		ptr++;
	}
	return (frames[0] * 60 + frames[1]) * 75 + frames[2]; // 75frames/sec
}

int hexatoi(const char *string)
{
	const char *ptr = string;
	int value = 0;
	
	while(1) {
		if(*ptr >= '0' && *ptr <= '9') {
			value = value * 16 + (*ptr - '0');
		} else if(*ptr >= 'a' && *ptr <= 'f') {
			value = value * 16 + (*ptr - 'a' + 10);
		} else if(*ptr >= 'A' && *ptr <= 'F') {
			value = value * 16 + (*ptr - 'A' + 10);
		} else if(*ptr == '\r' || *ptr == '\n' || *ptr == '\0') {
			break;
		}
		ptr++;
	}
	return value;
}

#include <string>

bool SCSI_CDROM::open_cue_file(const _TCHAR* file_path)
{
	std::string line_buf;
	std::string line_buf_shadow;
	std::string image_tmp_data_path;

	_TCHAR full_path_cue[_MAX_PATH];
	size_t ptr;
	int line_count = 0;
	int slen;
	int nr_current_track = 0;
	FILEIO* fio = new FILEIO();
	if(fio == NULL) return false;

	memset(full_path_cue, 0x00, sizeof(full_path_cue));
	image_tmp_data_path.clear();

	get_long_full_path_name(file_path, full_path_cue, sizeof(full_path_cue));
	
	const _TCHAR *parent_dir = get_parent_dir((const _TCHAR *)full_path_cue);

	size_t _arg1_ptr;
	size_t _arg2_ptr;
	size_t _arg2_ptr_s;
	size_t _arg3_ptr;
	size_t _arg3_ptr_s;

	std::string _arg1;
	std::string _arg2;
	std::string _arg3;
	
	if(fio->Fopen(file_path, FILEIO_READ_ASCII)) { // ToDo: Support not ASCII cue file (i.e. SJIS/UTF8).20181118 K.O
		line_buf.clear();
		for(int i = 0; i < 100; i++) {
			memset(&(track_data_path[i][0]), 0x00, _MAX_PATH * sizeof(_TCHAR));
		}
		int _c;
		bool is_eof = false;
		int sptr = 0;
		while(1) {
			line_buf.clear();
			int _np = 0;
			_c = EOF;
			do {
				_c = fio->Fgetc();
				if((_c == '\0') || (_c == '\n') || (_c == EOF)) break;;
				if(_c != '\r') line_buf.push_back((char)_c);
			} while(1);
			if(_c == EOF) is_eof = true;
			slen = (int)line_buf.length();
			if(slen <= 0) goto _n_continue;
			// Trim head of Space or TAB
			ptr = 0;
			sptr = 0;
			// Tokenize
			_arg1.clear();
			_arg2.clear();
			_arg3.clear();
			
			ptr = line_buf.find_first_not_of((const char*)" \t");
			if(ptr == std::string::npos) {
				goto _n_continue;
			}
			// Token1
			line_buf_shadow = line_buf.substr(ptr);

			_arg1_ptr = line_buf_shadow.find_first_of((const char *)" \t");
			_arg1 = line_buf_shadow.substr(0, _arg1_ptr);
			_arg2 = line_buf_shadow.substr(_arg1_ptr);
			std::transform(_arg1.begin(), _arg1.end(), _arg1.begin(),
						   [](unsigned char c) -> unsigned char{ return std::toupper(c); });

			_arg2_ptr = _arg2.find_first_not_of((const char *)" \t");

			if(_arg2_ptr != std::string::npos) {
				_arg2 = _arg2.substr(_arg2_ptr);
			}

			if(_arg1 == "REM") {
				// REM
				goto _n_continue;
			} else if(_arg1 == "FILE") {
				_arg2_ptr = _arg2.find_first_of((const char *)"\"") + 1;
				if(_arg2_ptr == std::string::npos) goto _n_continue;
				
				_arg2 = _arg2.substr(_arg2_ptr);
				_arg3_ptr = _arg2.find_first_of((const char *)"\"");
				if(_arg3_ptr == std::string::npos) goto _n_continue;
				_arg2 = _arg2.substr(0, _arg3_ptr);
				
				image_tmp_data_path.clear();
				image_tmp_data_path = std::string(parent_dir);
				image_tmp_data_path.append(_arg2);
				
				out_debug_log(_T("**FILE %s\n"), image_tmp_data_path.c_str());
				goto _n_continue; // ToDo: Check ARG2 (BINARY etc..) 20181118 K.O
			} else if(_arg1 == "TRACK") {
				_arg2_ptr_s = _arg2.find_first_of((const char *)" \t");
				
				_arg3 = _arg2.substr(_arg2_ptr_s);
				_arg2 = _arg2.substr(0, _arg2_ptr_s);
				_arg3_ptr = _arg3.find_first_not_of((const char *)" \t");
				int _nr_num = atoi(_arg2.c_str());
				
				// Set image file
				if((_nr_num > 0) && (_nr_num < 100) && (_arg3_ptr != std::string::npos)) {
					nr_current_track = _nr_num;
					_arg3 = _arg3.substr(_arg3_ptr);
					
					memset(track_data_path[_nr_num - 1], 0x00, sizeof(_TCHAR) * _MAX_PATH);
					strncpy((char *)(track_data_path[_nr_num - 1]), image_tmp_data_path.c_str(), _MAX_PATH);
					
					_arg3_ptr_s = _arg3.find_first_of((const char *)" \t\n");
					_arg3.substr(0, _arg3_ptr_s);
					
					std::transform(_arg3.begin(), _arg3.end(), _arg3.begin(),
								   [](unsigned char c) -> unsigned char{ return std::toupper(c); });
					
					toc_table[nr_current_track].is_audio = false;
					toc_table[nr_current_track].index0 = 0;
					toc_table[nr_current_track].index1 = 0;
					toc_table[nr_current_track].pregap = 0;
						
					if(_arg3.compare("AUDIO") == 0) {
						toc_table[nr_current_track].is_audio = true;
					} else if(_arg3.compare("MODE1/2352") == 0) {
						toc_table[nr_current_track].is_audio = false;
					} else {
						// ToDo:  another type
					}
					if(track_num < (_nr_num + 1)) track_num = _nr_num + 1;
				} else {
					// ToDo: 20181118 K.Ohta
					nr_current_track = 0;
				}
				goto _n_continue;
			} else if(_arg1 == "INDEX") {
				
				if((nr_current_track > 0) && (nr_current_track < 100)) {
					_arg2_ptr_s = _arg2.find_first_of((const char *)" \t");
					if(_arg2_ptr_s == std::string::npos) goto _n_continue;
					
					_arg3 = _arg2.substr(_arg2_ptr_s);
					_arg2 = _arg2.substr(0, _arg2_ptr_s);
					_arg3_ptr = _arg3.find_first_not_of((const char *)" \t");
					if(_arg3_ptr == std::string::npos) goto _n_continue;

					_arg3 = _arg3.substr(_arg3_ptr);
					_arg3_ptr_s = _arg3.find_first_of((const char *)" \t");
					_arg3.substr(0, _arg3_ptr_s);
					int index_type = atoi(_arg2.c_str());

					switch(index_type) {
					case 0:
						toc_table[nr_current_track].index0 = get_frames_from_msf(_arg3.c_str());
						break;
					case 1:
						toc_table[nr_current_track].index1 = get_frames_from_msf(_arg3.c_str());
						break;
					default:
						break;
					}
					goto _n_continue;
				} else {
					goto _n_continue;
				}
			} else if(_arg1 == "PREGAP") {
				if((nr_current_track > 0) && (nr_current_track < 100)) {
					_arg2_ptr_s = _arg2.find_first_of((const char *)" \t");
					_arg2 = _arg2.substr(0, _arg2_ptr_s - 1);
					
					toc_table[nr_current_track].pregap = get_frames_from_msf(_arg2.c_str());
					goto _n_continue;
				} else {
					goto _n_continue;
				}
			}
		_n_continue:
			if(is_eof) break;
			line_buf.clear();
			continue;
		}
		// Finish
		max_logical_block = 0;
		if(track_num > 0) {
			if(toc_table[0].is_audio) {
				toc_table[0].index0 = 0;
				toc_table[0].index1 = toc_table[0].pregap;
			} else{
				toc_table[0].index0 = toc_table[0].index1 = toc_table[0].pregap = 0;
			}
//			toc_table[0].lba_offset = 0;
//			toc_table[0].lba_size = 0;
//			toc_table[0].index0 = toc_table[0].index1 = toc_table[0].pregap = 0;
			// P1: Calc
			int _n = 0;
			for(int i = 1; i < track_num; i++) {

				if(fio_img->IsOpened()) {
					fio_img->Fclose();
				}
				// Even...
				//if(toc_table[i].pregap <= 0) {
				//	toc_table[i].pregap = 150; // Default PREGAP must be 2Sec. From OoTake.(Only with PCE? Not with FM-Towns?)
				//}
				if(toc_table[i].index1 != 0) {
					toc_table[i].index0 = toc_table[i].index0 + max_logical_block;
					toc_table[i].index1 = toc_table[i].index1 + max_logical_block;
					if(toc_table[i].pregap == 0) {
						toc_table[i].pregap = toc_table[i].index1 - toc_table[i].index0;
					}
				} else { // index1 == 0
					if(toc_table[i].pregap == 0) {
						toc_table[i].pregap = toc_table[i].index0;
					}
					toc_table[i].index1 = toc_table[i].index0 + max_logical_block;
					toc_table[i].index0 = max_logical_block;
				}
				// Even...
				if(toc_table[i].pregap <= 150) {
					toc_table[i].pregap = 150; // Default PREGAP must be 2Sec. From OoTake.(Only with PCE? Not with FM-Towns?)
				}

				_n = 0;
				if(strlen(track_data_path[i - 1]) > 0) {
					if(fio_img->Fopen(track_data_path[i - 1], FILEIO_READ_BINARY)) {
						if((_n = fio_img->FileLength() / 2352) > 0) {
							max_logical_block += _n;
						} else {
							_n = 0;
						}
						fio_img->Fclose();
					}
				}
				toc_table[i].lba_size = _n;
				toc_table[i].lba_offset = max_logical_block - _n;

				
				//#ifdef _CDROM_DEBUG_LOG
				out_debug_log(_T("TRACK#%02d TYPE=%s PREGAP=%d INDEX0=%d INDEX1=%d LBA_SIZE=%d LBA_OFFSET=%d PATH=%s\n"),
									i, (toc_table[i].is_audio) ? _T("AUDIO") : _T("MODE1/2352"),
									toc_table[i].pregap, toc_table[i].index0, toc_table[i].index1,
									toc_table[i].lba_size, toc_table[i].lba_offset, track_data_path[i - 1]);
				//#endif
			}
			
		}
		fio->Fclose();
	}
	delete fio;

	is_cue = false;
	if(track_num > 0) is_cue = true;
	// Not Cue FILE.
	return is_cue;
}

void SCSI_CDROM::open(const _TCHAR* file_path)
{
	_TCHAR img_file_path[_MAX_PATH];
	
	close();
	access = false;
	
	if(check_file_extension(file_path, _T(".cue"))) {
		is_cue = false;
		current_track = 0;
		open_cue_file(file_path);
	} else if(check_file_extension(file_path, _T(".ccd"))) {
		// get image file name
		my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.img"), get_file_path_without_extensiton(file_path));
		if(!FILEIO::IsFileExisting(img_file_path)) {
			my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.gz"), get_file_path_without_extensiton(file_path));
			if(!FILEIO::IsFileExisting(img_file_path)) {
				my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.img.gz"), get_file_path_without_extensiton(file_path));
			}
		}
		if(fio_img->Fopen(img_file_path, FILEIO_READ_BINARY)) {
			is_cue = false;
			current_track = 0;
			// get image file size
			if((max_logical_block = fio_img->FileLength() / 2352) > 0) {
				// read cue file
				FILEIO* fio = new FILEIO();
				if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
					char line[1024], *ptr;
					int track = -1;
					while(fio->Fgets(line, 1024) != NULL) {
						if(strstr(line, "[Session ") != NULL) {
							track = -1;
						} else if((ptr = strstr(line, "Point=0x")) != NULL) {
							if((track = hexatoi(ptr + 8)) > 0 && track < 0xa0) {
								if(track > track_num) {
									track_num = track;
								}
							}
						} else if((ptr = strstr(line, "Control=0x")) != NULL) {
							if(track > 0 && track < 0xa0) {
								toc_table[track - 1].is_audio = (hexatoi(ptr + 10) != 4);
							}
						} else if((ptr = strstr(line, "ALBA=-")) != NULL) {
							if(track > 0 && track < 0xa0) {
								toc_table[track - 1].pregap = atoi(ptr + 6);
							}
						} else if((ptr = strstr(line, "PLBA=")) != NULL) {
							if(track > 0 && track < 0xa0) {
								toc_table[track - 1].index1 = atoi(ptr + 5);
							}
						}
					}
					if(track_num != 0) {
#if 0
						toc_table[0].lba_offset = 0;
						toc_table[0].pregap = 0;
						for(int i = 1; i < track_num; i++) {
							toc_table[i].index0 = toc_table[i].index1 - toc_table[i].pregap;
							toc_table[i].lba_offset = toc_table[i].pregap;
							toc_table[i - 1].lba_size = toc_table[i].pregap - toc_table[i - 1].pregap;
						}
						toc_table[0].index0 = toc_table[0].index1 = toc_table[0].pregap = 0;
						toc_table[track_num].index0 = toc_table[track_num].index1 = max_logical_block;
						if(track_num > 0) {
							toc_table[track_num].lba_size = max_logical_block - toc_table[track_num - 1].lba_offset;
						} else {
							toc_table[track_num].lba_size = 0;
						}
#endif
					} else {
						fio_img->Fclose();
					}
					fio->Fclose();
				}
				delete fio;
			}
		}
	}
 
 	if(mounted()) {
		if(!is_cue) {
			if(toc_table[0].is_audio) {
				toc_table[0].index0 = 0;
				toc_table[0].index1 = toc_table[0].pregap;
			} else{
				toc_table[0].index0 = toc_table[0].index1 = toc_table[0].pregap = 0;
			}
			for(int i = 1; i < track_num; i++) {
				if(toc_table[i].index0 == 0) {
					toc_table[i].index0 = toc_table[i].index1 - toc_table[i].pregap;
				} else if(toc_table[i].pregap == 0) {
					toc_table[i].pregap = toc_table[i].index1 - toc_table[i].index0;
				}
			}
			toc_table[track_num].index0 = toc_table[track_num].index1 = max_logical_block;
			toc_table[track_num].pregap = 0;
		}
	}
	if(mounted() && (__SCSI_DEBUG_LOG)) {
		for(int i = 0; i < track_num + 1; i++) {
			uint32_t idx0_msf = lba_to_msf(toc_table[i].index0);
			uint32_t idx1_msf = lba_to_msf(toc_table[i].index1);
			uint32_t pgap_msf = lba_to_msf(toc_table[i].pregap);
			this->out_debug_log(_T("Track%02d: Index0=%02x:%02x:%02x Index1=%02x:%02x:%02x PreGap=%02x:%02x:%02x Audio=%d\n"), i + 1,
			(idx0_msf >> 16) & 0xff, (idx0_msf >> 8) & 0xff, idx0_msf & 0xff,
			(idx1_msf >> 16) & 0xff, (idx1_msf >> 8) & 0xff, idx1_msf & 0xff,
			(pgap_msf >> 16) & 0xff, (pgap_msf >> 8) & 0xff, pgap_msf & 0xff,
			toc_table[i].is_audio);
		}
	}
}

void SCSI_CDROM::close()
{
	if(fio_img->IsOpened()) {
		fio_img->Fclose();
	}
	memset(toc_table, 0, sizeof(toc_table));
	track_num = 0;
	is_cue = false;
	current_track = 0;
	set_cdda_status(CDDA_OFF);
}

bool SCSI_CDROM::mounted()
{
	if(is_cue) return true;
	return fio_img->IsOpened();
}

bool SCSI_CDROM::accessed()
{
	bool value = access;
	access = false;
	return value;
}

void SCSI_CDROM::mix(int32_t* buffer, int cnt)
{
	if(cdda_status == CDDA_PLAYING) {
		if(mix_loop_num != 0) {
			int tmp_l = 0, tmp_r = 0;
			for(int i = 0; i < mix_loop_num; i++) {
				event_callback(EVENT_CDDA, 0);
				tmp_l += cdda_sample_l;
				tmp_r += cdda_sample_r;
			}
			cdda_sample_l = tmp_l / mix_loop_num;
			cdda_sample_r = tmp_r / mix_loop_num;
		}
		int32_t val_l = apply_volume(apply_volume(cdda_sample_l, volume_m), volume_l);
		int32_t val_r = apply_volume(apply_volume(cdda_sample_r, volume_m), volume_r);
		
		for(int i = 0; i < cnt; i++) {
			*buffer++ += val_l; // L
			*buffer++ += val_r; // R
		}
	}
}

void SCSI_CDROM::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l, 3);
	volume_r = decibel_to_volume(decibel_r, 3);
}

void SCSI_CDROM::set_volume(int volume)
{
	volume_m = (int)(1024.0 * (max(0, min(100, volume)) / 100.0));
}

#define STATE_VERSION	5

// Q: If loading state when using another (saved) image? 20181013 K.O
//    May need close() and open() (or ...?).
bool SCSI_CDROM::process_state(FILEIO* state_fio, bool loading)
{
	uint32_t offset = 0;
 	
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateValue(cdda_start_frame);
	state_fio->StateValue(cdda_start_pregap);
	state_fio->StateValue(cdda_end_frame);
	state_fio->StateValue(cdda_playing_frame);
	state_fio->StateValue(cdda_status);
	state_fio->StateValue(cdda_play_mode);
	state_fio->StateArray(cdda_buffer, sizeof(cdda_buffer), 1);
	state_fio->StateValue(cdda_buffer_ptr);
	state_fio->StateValue(cdda_sample_l);
	state_fio->StateValue(cdda_sample_r);
	state_fio->StateValue(event_cdda);
	state_fio->StateValue(event_cdda_delay_play);
	state_fio->StateValue(event_delay_interrupt);
	state_fio->StateValue(read_sectors);
//	state_fio->StateValue(mix_loop_num);
	state_fio->StateValue(read_mode);
	state_fio->StateValue(volume_m);
	if(loading) {
		offset = state_fio->FgetUint32_LE();
	} else {
		if(fio_img->IsOpened()) {
			offset = fio_img->Ftell();
		}
		state_fio->FputUint32_LE(offset);
	}
 	
	state_fio->StateValue(is_cue);
	state_fio->StateValue(current_track);
	// ToDo: Re-Open Image.20181118 K.O
 	// post process
	if(loading && fio_img->IsOpened()) {
 		fio_img->Fseek(offset, FILEIO_SEEK_SET);
 	}
	return SCSI_DEV::process_state(state_fio, loading);
}
