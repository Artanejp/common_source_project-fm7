/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.31 -

	[FM-Towns CD-ROM based on SCSI CDROM]
*/


#include "./towns_cdrom.h"
#include "../../fifo.h"
#include "../../fileio.h"
#include "../scsi_host.h"

// SAME AS SCSI_CDROM::
#define CDDA_OFF	0
#define CDDA_PLAYING	1
#define CDDA_PAUSED	2

// 0-99 is reserved for SCSI_DEV class
#define EVENT_CDDA						100
#define EVENT_CDDA_DELAY_PLAY			101
#define EVENT_CDROM_SEEK_SCSI			102
#define EVENT_CDROM_DELAY_INTERRUPT_ON	103
#define EVENT_CDROM_DELAY_INTERRUPT_OFF	104
#define EVENT_CDDA_DELAY_STOP			105

#define _SCSI_DEBUG_LOG
#define _CDROM_DEBUG_LOG

// Event must be larger than 116.

namespace FMTOWNS {
void TOWNS_CDROM::initialize()
{
	subq_buffer = new FIFO(32); // OK?
	subq_overrun = false;
	stat_track = 0;
	SCSI_CDROM::initialize();
}

void TOWNS_CDROM::release()
{
	subq_buffer->release();
	delete subq_buffer;
	SCSI_CDROM::release();
}

void TOWNS_CDROM::reset()
{
	subq_buffer->clear();
	subq_overrun = false;
	stat_track = current_track;
	SCSI_CDROM::reset();
}

void TOWNS_CDROM::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_TOWNS_CDROM_SET_TRACK) {
		if(((data < 100) && (data >= 0)) || (data == 0xaa)) {
			stat_track = data;
		}
		return;
	}
	SCSI_CDROM::write_signal(id, data, mask);
}

uint32_t TOWNS_CDROM::read_signal(int id)
{
	if(id == SIG_TOWNS_CDROM_IS_MEDIA_INSERTED) {
		return ((is_device_ready()) ? 0xffffffff : 0x00000000);
	} else if(id == SIG_TOWNS_CDROM_MAX_TRACK) {
		if(track_num <= 0) {
			return (uint32_t)(TO_BCD(0x00));
		} else {
			return (uint32_t)(TO_BCD(track_num));
		}
	} else if(id == SIG_TOWNS_CDROM_REACHED_MAX_TRACK) {
		if(track_num <= 0) {
			return 0xffffffff;
		} else {
			if(current_track >= track_num) {
				return 0xffffffff;
			} else {
				return 0x00000000;
			}
		}
	} else if(id == SIG_TOWNS_CDROM_CURRENT_TRACK) {
		if(current_track > track_num) {
			return 0x00000000;
		} else {
			return TO_BCD(current_track);
		}
	} else if(id == SIG_TOWNS_CDROM_START_MSF) {
		int trk = stat_track;
		if(trk <= 0) {
			return 0xffffffff;
		}
		if(trk == 0xaa) {
			trk = track_num;
		}
		int index0 = toc_table[trk].index0;
		int index1 = toc_table[trk].index1;
		int pregap = toc_table[trk].pregap;
		uint32_t lba = (uint32_t)index0;
		if(pregap > 0) lba = lba - pregap;
		if(lba < 150) lba = 150;
		uint32_t msf = lba_to_msf(lba); // Q:lba + 150?
		stat_track++;
		return msf;
	} else if(id == SIG_TOWNS_CDROM_START_MSF_AA) {
		int trk = track_num;
		int index0 = toc_table[trk].index0;
		int index1 = toc_table[trk].index1;
		int pregap = toc_table[trk].pregap;
		uint32_t lba = (uint32_t)index0;
		if(pregap > 0) lba = lba - pregap;
		if(lba < 150) lba = 150;
		uint32_t msf = lba_to_msf(lba); // Q:lba + 150?
		return msf;
	} else if(id == SIG_TOWNS_CDROM_RELATIVE_MSF) {
		if(toc_table[current_track].is_audio) {
			if(!(is_device_ready())) {
				return 0;
			}
			if(cdda_playing_frame <= cdda_start_frame) {
				return 0;
			}
			uint32_t msf;
			if(cdda_playing_frame >= cdda_end_frame) {
				if(cdda_repeat) {
					return 0;
				} else {
					msf = lba_to_msf(cdda_end_frame - cdda_start_frame);
					return msf;
				}
			}
			msf = lba_to_msf(cdda_playing_frame - cdda_start_frame);
			return msf;
		} else {
			if(!(is_device_ready())) {
				return 0;
			}
			if(fio_img->IsOpened()) {
				uint32_t cur_position = (uint32_t)fio_img->Ftell();
				cur_position = cur_position / logical_block_size();
				if(cur_position >= max_logical_block) {
					cur_position = max_logical_block;
				}
				uint32_t msf = lba_to_msf(cur_position);
				return msf;
			}
			return 0;
		}
	}  else if(id == SIG_TOWNS_CDROM_ABSOLUTE_MSF) {
		if(toc_table[current_track].is_audio) {
			if(!(is_device_ready())) {
				return 0;
			}
			uint32_t msf;
			msf = lba_to_msf(cdda_playing_frame);
			return msf;
		} else {
			if(!(is_device_ready())) {
				return 0;
			}
			if(fio_img->IsOpened()) {
				uint32_t cur_position = (uint32_t)fio_img->Ftell();
				cur_position = cur_position / logical_block_size();
				if(cur_position >= max_logical_block) {
					cur_position = max_logical_block;
				}
				uint32_t msf = lba_to_msf(cur_position + toc_table[current_track].lba_offset);
				return msf;
			}
			return 0;
		}
	} else if(id == SIG_TOWNS_CDROM_GET_ADR) {
		int trk = stat_track;
		if(!(is_device_ready())) {
			return 0xffffffff; // OK?
		}
		if(trk == 0xaa) {
			return 0x10; // AUDIO SUBQ
		}
		if(trk > track_num) {
			return 0xffffffff; // OK?
		}
		if(toc_table[trk].is_audio) {
			return 0x10;
		}
		return 0x14; // return as data
	} 
	return SCSI_CDROM::read_signal(id);
}

void TOWNS_CDROM::event_callback(int event_id, int err)
{
	SCSI_CDROM::event_callback(event_id, err);
	if(event_id == EVENT_CDDA) {
		// Post process
		if(((cdda_buffer_ptr % 2352) == 0) && (cdda_status == CDDA_PLAYING)) {
			set_subq();
		}
	}
}

int TOWNS_CDROM::get_command_length(int value)
{
	switch(value) {
	case TOWNS_CDROM_CDDA_PLAY:
		return 10;
		break;
	case TOWNS_CDROM_CDDA_PAUSE:
		return 4;
		break;
	case TOWNS_CDROM_CDDA_UNPAUSE:
		return 4;
		break;
	case TOWNS_CDROM_CDDA_STOP:
		return 4;
		break;
	}
		
	return SCSI_CDROM::get_command_length(value);
}

void TOWNS_CDROM::start_command()
{
	touch_sound();
	switch(command[0]) {
	case TOWNS_CDROM_CDDA_PLAY:
		play_cdda_from_cmd();
		break;
	case TOWNS_CDROM_CDDA_PAUSE:
		pause_cdda_from_cmd();
		break;
	case TOWNS_CDROM_CDDA_UNPAUSE:
		unpause_cdda_from_cmd();
		break;
	case TOWNS_CDROM_CDDA_STOP:
		stop_cdda_from_cmd();
		break;
	case SCSI_CMD_TST_U_RDY:
	case SCSI_CMD_INQUIRY:
	case SCSI_CMD_REQ_SENSE:
	case SCSI_CMD_RD_DEFECT:
	case SCSI_CMD_RD_CAPAC:
	case SCSI_CMD_MODE_SEL6: // OK?
	case SCSI_CMD_READ6:
	case SCSI_CMD_READ10:
	case SCSI_CMD_READ12:
		SCSI_CDROM::start_command();
		set_subq(); // First
		break;
	case 0xff:
		// End of List
		set_dat(SCSI_STATUS_CHKCOND);
		break;
	default:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SCSI_DEV:ID=%d] Command: Unknown %02X\n"), scsi_id, command[0]);
		#endif
		set_dat(SCSI_STATUS_GOOD);
		set_phase_delay(SCSI_PHASE_STATUS, 10.0);
	}
}

	
// From MAME 0203's fmtowns.cpp .	
void TOWNS_CDROM::pause_cdda_from_cmd()
{
	if(cdda_status == CDDA_PLAYING) {
		set_cdda_status(CDDA_PAUSED);
		//set_subq();
	}
}

void TOWNS_CDROM::unpause_cdda_from_cmd()
{
	if(cdda_status == CDDA_PAUSED) {
		set_cdda_status(CDDA_PLAYING);
		//set_subq();
	}
}

void TOWNS_CDROM::stop_cdda_from_cmd()
{
	set_cdda_status(CDDA_OFF);
	set_subq();
}

void TOWNS_CDROM::play_cdda_from_cmd()
{
	uint8_t m_start = command[3]; 
	uint8_t s_start = command[4];
	uint8_t f_start = command[5];
	uint8_t m_end   = command[7];
	uint8_t s_end   = command[8];
	uint8_t f_end   = command[9];
	if(is_device_ready()) {
		cdda_start_frame = FROM_BCD(f_start) + (FROM_BCD(s_start) + FROM_BCD(m_start) * 60) * 75;
		cdda_end_frame   = FROM_BCD(f_end)   + (FROM_BCD(s_end)   + FROM_BCD(m_end) * 60) * 75;
		int track = get_track(cdda_start_frame);
		if(cdda_start_frame >= toc_table[track].pregap) {
			cdda_start_frame -= toc_table[track].pregap;
		}
		if(cdda_start_frame < toc_table[track].index0) {
			cdda_start_frame = toc_table[track].index0; // don't play pregap
		} else if(cdda_start_frame > max_logical_block) {
			cdda_start_frame = 0;
		}
		track = current_track;
		cdda_playing_frame = cdda_start_frame;
		if(cdda_end_frame > toc_table[track + 1].index1 && (cdda_end_frame - toc_table[track].pregap) <= toc_table[track + 1].index1) {
			//auto_increment_track = true;
		}
		if(event_cdda_delay_play >= 0) {
			cancel_event(this, event_cdda_delay_play);
			event_cdda_delay_play = -1;
		}
		register_event(this, EVENT_CDDA_DELAY_PLAY, 10.0, false, &event_cdda_delay_play);
		
	}
	set_subq(); // First
}
	
void TOWNS_CDROM::set_subq(void)
{
	if(is_device_ready()) {
		// create track info
		int track = current_track;
		uint32_t frame;
		uint32_t msf_abs;
		uint32_t msf_rel;
		if(toc_table[track].is_audio) { // OK? (or force ERROR) 20181120 K.O
			frame = (cdda_status == CDDA_OFF) ? cdda_start_frame : cdda_playing_frame;
			msf_rel = lba_to_msf_alt(frame - toc_table[track].index0);
		} else { // Data
			if(fio_img->IsOpened()) {
				uint32_t cur_position = (uint32_t)(fio_img->Ftell());
				if(is_cue) {
					frame = (cur_position / physical_block_size()) + toc_table[track].lba_offset;
					msf_rel = lba_to_msf_alt(frame - toc_table[track].lba_offset);
				} else {
					frame = cur_position / physical_block_size();
					if(frame > toc_table[track].lba_offset) {
						msf_rel = lba_to_msf_alt(frame - toc_table[track].lba_offset);
					} else {
						msf_rel = lba_to_msf_alt(0);
					}
				}
			} else {
				frame = toc_table[track].lba_offset;
				msf_rel = 0;
			}
		}
		msf_abs = lba_to_msf_alt(frame);
		subq_overrun = !(subq_buffer->empty());
		subq_buffer->clear();
		// http://www.ecma-international.org/publications/files/ECMA-ST/Ecma-130.pdf
		subq_buffer->write(0x01 | (toc_table[track].is_audio ? 0x00 : 0x40));
		
		subq_buffer->write(TO_BCD(track + 1));		// TNO
		subq_buffer->write(TO_BCD((cdda_status == CDDA_PLAYING) ? 0x00 : ((cdda_status == CDDA_PAUSED) ? 0x00 : 0x01))); // INDEX
		subq_buffer->write(TO_BCD((msf_rel >> 16) & 0xff));	// M (relative)
		subq_buffer->write(TO_BCD((msf_rel >>  8) & 0xff));	// S (relative)
		subq_buffer->write(TO_BCD((msf_rel >>  0) & 0xff));	// F (relative)
		subq_buffer->write(TO_BCD(0x00));	// Zero (relative)
		subq_buffer->write(TO_BCD((msf_abs >> 16) & 0xff));	// M (absolute)
		subq_buffer->write(TO_BCD((msf_abs >>  8) & 0xff));	// S (absolute)
		subq_buffer->write(TO_BCD((msf_abs >>  0) & 0xff));	// F (absolute)
		// transfer length
		remain = subq_buffer->count();
		// set first data
		// change to data in phase
		//set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
	} else {
		//write_signals(&output_subq_overrun, (subq_buffer->empty()) ? 0x00000000 : 0xffffffff); // OK?
		subq_buffer->clear();
		// transfer length
		remain = subq_buffer->count();
		set_dat(is_device_ready() ? SCSI_STATUS_GOOD : SCSI_STATUS_CHKCOND);
		//set_phase_delay(SCSI_PHASE_STATUS, 10.0);
	}
	return;
}

uint8_t TOWNS_CDROM::get_subq_status()
{
	uint8_t val = 0x00;
	val = val | ((subq_buffer->empty()) ? 0x00 : 0x01);
	val = val | ((subq_overrun) ? 0x02 : 0x00);
	return val;
}

uint8_t TOWNS_CDROM::read_subq()
{
	uint8_t val;
//	if(subq_buffer->empty()) {
//		set_subq();
//	}
	val = (uint8_t)(subq_buffer->read() & 0xff);
	return val;
}

#define STATE_VERSION	1

bool TOWNS_CDROM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	if(!(subq_buffer->process_state((void *)state_fio, loading))) {
		return false;
	}
	state_fio->StateValue(subq_overrun);
	state_fio->StateValue(stat_track);
	
	return SCSI_CDROM::process_state(state_fio, loading);
}
	
	
}
