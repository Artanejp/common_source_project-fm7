/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2016.03.06-

	[ SCSI CD-ROM drive ]
*/

#include "scsi_cdrom.h"
#include "../fifo.h"

#define CDDA_OFF	0
#define CDDA_PLAYING	1
#define CDDA_PAUSED	2

#define MODE_REPEAT	1
#define MODE_INTERRUPT	2
#define MODE_NO_REPEAT	3

// 0-99 is reserved for SCSI_DEV class
#define EVENT_CDDA	100

void SCSI_CDROM::initialize()
{
	SCSI_DEV::initialize();
	fio_img = new FILEIO();
	
	if(44100 % emu->get_sound_rate() == 0) {
		mix_loop_num = 44100 / emu->get_sound_rate();
	} else {
		mix_loop_num = 0;
	}
	event_cdda = -1;
	cdda_start_frame = cdda_start_pregap = 0;
	cdda_end_frame = 0;
	cdda_playing_frame = 0;
	cdda_status = CDDA_OFF;
	cdda_play_mode = 0;
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
	SCSI_DEV::reset();
	read_mode = false;
	set_cdda_status(CDDA_OFF);
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
	case EVENT_CDDA:
		// read 16bit 2ch samples in the cd-da buffer, called 44100 times/sec
//		if(cdda_buffer_ptr < max_logical_block * 2532) {
			pair16_t tmp_l, tmp_r;
			tmp_l.read_2bytes_le_from(&cdda_buffer[cdda_buffer_ptr + 0]);
			tmp_r.read_2bytes_le_from(&cdda_buffer[cdda_buffer_ptr + 2]);
			cdda_sample_l = tmp_l.sw;
			cdda_sample_r = tmp_r.sw;
//		} else {
//			cdda_sample_l = cdda_sample_r = 0;
//		}
		if((cdda_buffer_ptr += 4) % 2352 == 0) {
			// one frame finished
			if(++cdda_playing_frame >= min(cdda_end_frame, max_logical_block)) {
				// reached to end frame
				if(cdda_play_mode == MODE_REPEAT) {
					// reload buffer
					if(cdda_start_frame < max_logical_block) {
						fio_img->Fseek(cdda_start_frame * 2352, FILEIO_SEEK_SET);
						fio_img->Fread(cdda_buffer, sizeof(cdda_buffer), 1);
					} else {
						memset(cdda_buffer, 0, sizeof(cdda_buffer));
					}
					cdda_buffer_ptr = 0;
					cdda_playing_frame = cdda_start_frame;
					access = true;
				} else {
					// stop
					if(cdda_play_mode == MODE_INTERRUPT) {
						write_signals(&outputs_done, 0xffffffff);
					}
					set_cdda_status(CDDA_OFF);
				}
			} else if(cdda_buffer_ptr == array_length(cdda_buffer)) {
				// refresh buffer
				fio_img->Fread(cdda_buffer, sizeof(cdda_buffer), 1);
				cdda_buffer_ptr = 0;
				access = true;
			}
		}
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
			touch_sound();
			set_realtime_render(this, true);
		}
	} else {
		if(event_cdda != -1) {
			cancel_event(this, event_cdda);
			event_cdda = -1;
		}
		if(cdda_status == CDDA_PLAYING) {
			touch_sound();
			set_realtime_render(this, false);
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

int SCSI_CDROM::get_track(uint32_t lba)
{
	int track = 0;
	
	for(int i = 0; i <= track_num; i++) {
		if(lba >= toc_table[i].index0) {
			track = i;
		} else {
			break;
		}
	}
	return track;
}

double SCSI_CDROM::get_seek_time(uint32_t lba)
{
	if(fio_img->IsOpened()) {
		uint32_t cur_position = (int)fio_img->Ftell();
		int distance = abs((int)(lba * physical_block_size()) - (int)cur_position);
		double ratio = (double)distance / 333000 / physical_block_size(); // 333000: sectors in media
		return max(10, (int)(400000 * 2 * ratio));
	} else {
		return 400000; // 400msec
	}
}

uint32_t lba_to_msf(uint32_t lba)
{
	uint8_t m = lba / (60 * 75);
	lba -= m * (60 * 75);
	uint8_t s = lba / 75;
	uint8_t f = lba % 75;

	return ((m / 10) << 20) | ((m % 10) << 16) | ((s / 10) << 12) | ((s % 10) << 8) | ((f / 10) << 4) | ((f % 10) << 0);
}

uint32_t lba_to_msf_alt(uint32_t lba)
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
	
	switch(command[0]) {
	case SCSI_CMD_TST_U_RDY:
		read_mode = 0;
		break;
		
	case SCSI_CMD_READ6:
		seek_time = 10;//get_seek_time((command[1] & 0x1f) * 0x10000 + command[2] * 0x100 + command[3]);
		set_cdda_status(CDDA_OFF);
		break;
		
	case SCSI_CMD_READ10:
	case SCSI_CMD_READ12:
		seek_time = 10;//get_seek_time(command[2] * 0x1000000 + command[3] * 0x10000 + command[4] * 0x100 + command[5]);
		set_cdda_status(CDDA_OFF);
		break;
		
	case SCSI_CMD_MODE_SEL6:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: NEC Read Mode Select 6-byte\n"), scsi_id);
		#endif
		// start position
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
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: NEC Set Audio Playback Start Position\n"), scsi_id);
		#endif
		if(is_device_ready()) {
			if(command[2] == 0 && command[3] == 0 && command[4] == 0) {
				// stop cd-da if all params are zero
				cdda_start_frame = cdda_start_pregap = 0;
				cdda_end_frame = toc_table[track_num].index0; // end of disc
				set_cdda_status(CDDA_OFF);
			} else {
				switch(command[9] & 0xc0) {
				case 0x00:
					cdda_start_frame = (command[2] << 16) | (command[3] << 8) | command[4];
					break;
				case 0x40:
					{
						#ifdef _SCSI_DEBUG_LOG
							this->out_debug_log(_T("[SCSI_DEV:ID=%d] M:S:F=%02x:%02x:%02x\n"), scsi_id,
								command[2], command[3], command[4]);
						#endif
						uint8_t m = FROM_BCD(command[2]);
						uint8_t s = FROM_BCD(command[3]);
						uint8_t f = FROM_BCD(command[4]);
						cdda_start_frame = f + 75 * (s + m * 60);
						
						// PCE tries to be clever here and set (start of track + track pregap size) to skip the pregap
						// (I guess it wants the TOC to have the real start sector for data tracks and the start of the pregap for audio?)
						int track = get_track(cdda_start_frame);
						if(track < track_num) {
							cdda_start_frame -= min(toc_table[track].pregap, cdda_start_frame);
							cdda_start_pregap = toc_table[track].pregap;
						} else {
							cdda_start_frame = toc_table[track_num].index0;
							cdda_start_pregap = 0;
						}
					}
					break;
				case 0x80:
					#ifdef _SCSI_DEBUG_LOG
						this->out_debug_log(_T("[SCSI_DEV:ID=%d] Track=%02x\n"), scsi_id, command[2]);
					#endif
					if(FROM_BCD(command[2]) <= track_num) {
						cdda_start_frame = toc_table[max(FROM_BCD(command[2]), 1) - 1].index1;
					} else {
						cdda_start_frame = toc_table[track_num].index0;
					}
					break;
				default:
					cdda_start_frame = 0;
					break;
				}
				cdda_start_frame = min(cdda_start_frame, max_logical_block);
				
//				if(cdda_status == CDDA_PAUSED) {
//					cdda_end_frame = toc_table[track_num].index0; // end of disc
//					set_cdda_status(CDDA_OFF);
//				} else
				if((command[1] & 3) != 0) {
					cdda_end_frame = toc_table[track_num].index0; // end of disc
					set_cdda_status(CDDA_PLAYING);
					cdda_play_mode = (command[1] & 2) ? MODE_INTERRUPT : MODE_NO_REPEAT;
				} else {
					cdda_end_frame = toc_table[min(get_track(cdda_start_frame) + 1, track_num)].index0; // end of this track
					set_cdda_status(CDDA_PAUSED);
					cdda_play_mode = MODE_NO_REPEAT;
				}
				
				// read buffer
				double seek_time = get_seek_time(cdda_start_frame);
				if(cdda_start_frame < max_logical_block) {
					fio_img->Fseek(cdda_start_frame * 2352, FILEIO_SEEK_SET);
					fio_img->Fread(cdda_buffer, sizeof(cdda_buffer), 1);
				} else {
					memset(cdda_buffer, 0, sizeof(buffer));
				}
				cdda_buffer_ptr = 0;
				cdda_playing_frame = cdda_start_frame;
				access = true;
				
				// change to status phase
				set_dat(SCSI_STATUS_GOOD);
				set_phase_delay(SCSI_PHASE_STATUS, seek_time);
				#ifdef _SCSI_DEBUG_LOG
					uint32_t s_msf = lba_to_msf(cdda_start_frame);
					uint32_t e_msf = lba_to_msf(cdda_end_frame);
					this->out_debug_log(_T("[SCSI_DEV:ID=%d] Start=%02x:%02x:%02x End=%02x:%02x:%02x Mode=%d\n"), scsi_id,
						(s_msf >> 16) & 0xff, (s_msf >> 8) & 0xff, s_msf & 0xff,
						(e_msf >> 16) & 0xff, (e_msf >> 8) & 0xff, e_msf & 0xff, cdda_play_mode);
				#endif
				return;
			}
		}
		// change to status phase
		set_dat(is_device_ready() ? SCSI_STATUS_GOOD : SCSI_STATUS_CHKCOND);
		set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		return;
		
	case 0xd9:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: NEC Set Audio Playback End Position\n"), scsi_id);
		#endif
		if(is_device_ready()) {
			switch(command[9] & 0xc0) {
			case 0x00:
				cdda_end_frame = (command[3] << 16) | (command[4] << 8) | command[5];
				break;
			case 0x40:
				{
					#ifdef _SCSI_DEBUG_LOG
						this->out_debug_log(_T("[SCSI_DEV:ID=%d] M:S:F=%02x:%02x:%02x\n"), scsi_id,
							command[2], command[3], command[4]);
					#endif
					uint8_t m = FROM_BCD(command[2]);
					uint8_t s = FROM_BCD(command[3]);
					uint8_t f = FROM_BCD(command[4]);
					cdda_end_frame = f + 75 * (s + m * 60);
					
					// PCE tries to be clever here and set (start of track + track pregap size) to skip the pregap
					// (I guess it wants the TOC to have the real start sector for data tracks and the start of the pregap for audio?)
					cdda_end_frame -= min(cdda_start_pregap, cdda_end_frame);
				}
				break;
			case 0x80:
				#ifdef _SCSI_DEBUG_LOG
					this->out_debug_log(_T("[SCSI_DEV:ID=%d] Track=%02x\n"), scsi_id, command[2]);
				#endif
				if(FROM_BCD(command[2]) <= track_num) {
					cdda_end_frame = toc_table[max(FROM_BCD(command[2]), 1) - 1].index0;
				} else {
					cdda_end_frame = toc_table[track_num].index0;
				}
				break;
			default:
				cdda_end_frame = toc_table[track_num].index0;
				break;
			}
			cdda_end_frame = min(cdda_end_frame, max_logical_block);
			cdda_play_mode = command[1] & 3;
			
			if(cdda_play_mode != 0) {
				set_cdda_status(CDDA_PLAYING);
			} else {
				set_cdda_status(CDDA_OFF);
			}
			#ifdef _SCSI_DEBUG_LOG
				uint32_t s_msf = lba_to_msf(cdda_start_frame);
				uint32_t e_msf = lba_to_msf(cdda_end_frame);
				this->out_debug_log(_T("[SCSI_DEV:ID=%d] Start=%02x:%02x:%02x End=%02x:%02x:%02x Mode=%d\n"), scsi_id,
					(s_msf >> 16) & 0xff, (s_msf >> 8) & 0xff, s_msf & 0xff,
					(e_msf >> 16) & 0xff, (e_msf >> 8) & 0xff, e_msf & 0xff, cdda_play_mode);
			#endif
		}
		// change to status phase
		set_dat(is_device_ready() ? SCSI_STATUS_GOOD : SCSI_STATUS_CHKCOND);
		set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		return;
		
	case 0xda:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: NEC Pause\n"), scsi_id);
		#endif
		if(is_device_ready()) {
			if(cdda_status == CDDA_PLAYING) {
				set_cdda_status(CDDA_OFF);
			}
		}
		// change to status phase
		set_dat(is_device_ready() ? SCSI_STATUS_GOOD : SCSI_STATUS_CHKCOND);
		set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		return;
		
	case 0xdd:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: NEC Read Sub Channel Q\n"), scsi_id);
		#endif
		if(is_device_ready()) {
			// create track info
			uint32_t frame = (cdda_status == CDDA_OFF) ? cdda_start_frame : cdda_playing_frame;
			uint32_t msf_abs = lba_to_msf_alt(frame);
			int track = min(get_track(frame), track_num - 1);
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
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: NEC Get Dir Info\n"), scsi_id);
		#endif
		if(is_device_ready()) {
			buffer->clear();
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
					uint32_t frame = toc_table[track - 1].index0;
					// PCE wants the start sector for data tracks to *not* include the pregap
					if(!toc_table[track - 1].is_audio) {
						frame += toc_table[track - 1].pregap;
					}
//					uint32_t msf = lba_to_msf(toc_table[track - 1].index1 + 150);
					uint32_t msf = lba_to_msf(frame + 150);
					buffer->write((msf >> 16) & 0xff); // M
					buffer->write((msf >>  8) & 0xff); // S
					buffer->write((msf >>  0) & 0xff); // F
					buffer->write(toc_table[track - 1].is_audio ? 0x00 : 0x04);
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
	}
	
	// start standard command
	SCSI_DEV::start_command();
}

bool SCSI_CDROM::read_buffer(int length)
{
	if(!fio_img->IsOpened()) {
		set_sense_code(SCSI_SENSE_NOTREADY);
		return false;
	}
	uint32_t offset = (uint32_t)(position % 2352);
	
	if(fio_img->Fseek((long)position, FILEIO_SEEK_SET) != 0) {
		set_sense_code(SCSI_SENSE_ILLGLBLKADDR); //SCSI_SENSE_SEEKERR
		return false;
	}
	while(length > 0) {
		uint8_t tmp_buffer[2352];
//		int tmp_length = min(length, (int)sizeof(tmp_buffer));
		int tmp_length = 2352 - offset;
		
		if(fio_img->Fread(tmp_buffer, tmp_length, 1) != 1) {
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
		}
		access = true;
	}
	set_sense_code(SCSI_SENSE_NOSENSE);
	return true;
}

bool SCSI_CDROM::write_buffer(int length)
{
	for(int i = 0; i < length; i++) {
		int value = buffer->read();
		if(command[0] == SCSI_CMD_MODE_SEL6) {
			if(i == 4) {
				#ifdef _SCSI_DEBUG_LOG
					this->out_debug_log(_T("[SCSI_DEV:ID=%d] NEC Read Mode = %02X\n"), scsi_id, value);
				#endif
				read_mode = (value != 0);
			} else if(i == 10) {
				#ifdef _SCSI_DEBUG_LOG
					this->out_debug_log(_T("[SCSI_DEV:ID=%d] NEC Retry Count = %02X\n"), scsi_id, value);
				#endif
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

void SCSI_CDROM::open(const _TCHAR* file_path)
{
	_TCHAR ccd_file_path[_MAX_PATH];
	_TCHAR cue_file_path[_MAX_PATH];
	_TCHAR img_file_path[_MAX_PATH];
	
	close();
	
	my_stprintf_s(ccd_file_path, _MAX_PATH, _T("%s.ccd"), get_file_path_without_extensiton(file_path));
	my_stprintf_s(cue_file_path, _MAX_PATH, _T("%s.cue"), get_file_path_without_extensiton(file_path));
	
	if(FILEIO::IsFileExisting(ccd_file_path)) {
		// get image file name
		my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.img"), get_file_path_without_extensiton(file_path));
		if(!FILEIO::IsFileExisting(img_file_path)) {
			my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.gz"), get_file_path_without_extensiton(file_path));
			if(!FILEIO::IsFileExisting(img_file_path)) {
				my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.img.gz"), get_file_path_without_extensiton(file_path));
			}
		}
		if(fio_img->Fopen(img_file_path, FILEIO_READ_BINARY)) {
			// get image file size
			if((max_logical_block = fio_img->FileLength() / 2352) > 0) {
				// read cue file
				FILEIO* fio = new FILEIO();
				if(fio->Fopen(ccd_file_path, FILEIO_READ_BINARY)) {
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
					if(track_num == 0) {
						fio_img->Fclose();
					}
					fio->Fclose();
				}
				delete fio;
			}
		}
	} else if(FILEIO::IsFileExisting(cue_file_path)) {
		// get image file name
		my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.img"), get_file_path_without_extensiton(file_path));
		if(!FILEIO::IsFileExisting(img_file_path)) {
			my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.bin"), get_file_path_without_extensiton(file_path));
			if(!FILEIO::IsFileExisting(img_file_path)) {
				my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.gz"), get_file_path_without_extensiton(file_path));
				if(!FILEIO::IsFileExisting(img_file_path)) {
					my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.img.gz"), get_file_path_without_extensiton(file_path));
					if(!FILEIO::IsFileExisting(img_file_path)) {
						my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.bin.gz"), get_file_path_without_extensiton(file_path));
					}
				}
			}
		}
		if(fio_img->Fopen(img_file_path, FILEIO_READ_BINARY)) {
			// get image file size
			if((max_logical_block = fio_img->FileLength() / 2352) > 0) {
				// read cue file
				FILEIO* fio = new FILEIO();
				if(fio->Fopen(cue_file_path, FILEIO_READ_BINARY)) {
					char line[1024], *ptr;
					int track = -1;
					while(fio->Fgets(line, 1024) != NULL) {
						if(strstr(line, "FILE") != NULL) {
							// do nothing
						} else if((ptr = strstr(line, "TRACK")) != NULL) {
							// "TRACK 01 AUDIO"
							// "TRACK 02 MODE1/2352"
							ptr += 6;
							while(*ptr == ' ' || *ptr == 0x09) {
								ptr++;
							}
							if((track = atoi(ptr)) > 0) {
								if(track > track_num) {
									track_num = track;
								}
								toc_table[track - 1].is_audio = (strstr(line, "AUDIO") != NULL);
							}
						} else if((ptr = strstr(line, "PREGAP")) != NULL) {
							// "PREGAP 00:02:00"
							if(track > 0) {
								toc_table[track - 1].pregap = get_frames_from_msf(ptr + 7);
							}
						} else if((ptr = strstr(line, "INDEX")) != NULL) {
							// "INDEX 01 00:00:00"
							if(track > 0) {
								ptr += 6;
								while(*ptr == ' ' || *ptr == 0x09) {
									ptr++;
								}
								int num = atoi(ptr);
								while(*ptr >= '0' && *ptr <= '9') {
									ptr++;
								}
								if(num == 0) {
									toc_table[track - 1].index0 = get_frames_from_msf(ptr);
								} else if(num == 1) {
									toc_table[track - 1].index1 = get_frames_from_msf(ptr);
								}
							}
						}
					}
					if(track_num == 0) {
						fio_img->Fclose();
					}
					fio->Fclose();
				}
				delete fio;
			}
		}
	}
	if(mounted()) {
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
#ifdef _SCSI_DEBUG_LOG
		for(int i = 0; i < track_num + 1; i++) {
			uint32_t idx0_msf = lba_to_msf(toc_table[i].index0);
			uint32_t idx1_msf = lba_to_msf(toc_table[i].index1);
			uint32_t pgap_msf = lba_to_msf(toc_table[i].pregap);
			this->out_debug_log(_T("Track%02d: Index0=%02x:%02x:%02x Index1=%02x:%02x:%02x PreGpap=%02x:%02x:%02x Audio=%d\n"), i + 1,
			(idx0_msf >> 16) & 0xff, (idx0_msf >> 8) & 0xff, idx0_msf & 0xff,
			(idx1_msf >> 16) & 0xff, (idx1_msf >> 8) & 0xff, idx1_msf & 0xff,
			(pgap_msf >> 16) & 0xff, (pgap_msf >> 8) & 0xff, pgap_msf & 0xff,
			toc_table[i].is_audio);
		}
#endif
	}
}

void SCSI_CDROM::close()
{
	if(fio_img->IsOpened()) {
		fio_img->Fclose();
	}
	memset(toc_table, 0, sizeof(toc_table));
	track_num = 0;
	max_logical_block = 0;
	set_cdda_status(CDDA_OFF);
}

bool SCSI_CDROM::mounted()
{
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
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
}

void SCSI_CDROM::set_volume(int volume)
{
	volume_m = (int)(1024.0 * (max(0, min(100, volume)) / 100.0));
}

#define STATE_VERSION	4

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
	
	// post process
	if(loading && fio_img->IsOpened()) {
		fio_img->Fseek(offset, FILEIO_SEEK_SET);
	}
	return SCSI_DEV::process_state(state_fio, loading);
}

