/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.31 -

	[Towns CDROM]
*/


#include "../scsi_cdrom.h"
#include "../fifo.h"

#define CDDA_OFF	0
#define CDDA_PLAYING	1
#define CDDA_PAUSED	2

#define _SCSI_DEBUG_LOG
#define _CDROM_DEBUG_LOG

// Event must be larger than 116.

namespace FMTOWNS {
void TOWNS_CDROM::initialize()
{
	SCSI_CDROM::initialize();
}

void TOWNS_CDROM::release()
{
	SCSI_CDROM::release();
}

void TOWNS_CDROM::reset()
{
	SCSI_CDROM::reset();
}

void TOWNS_CDROM::write_signal(int id, uint32_t data, uint32_t mask)
{
	SCSI_CDROM::write_signal(id, data, mask);
}

uint32_t TOWNS_CDROM::read_signal(int id)
{
	return SCSI_CDROM::read_signal(id);
}

void TOWNS_CDROM::event_callback(int event_id, int err)
{
	SCSI_CDROM::event_callback(event_id, err);
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
		return;
		break;
	case TOWNS_CDROM_CDDA_PAUSE:
		pause_cdda_from_cmd();
		return;
		break;
	case TOWNS_CDROM_CDDA_UNPAUSE:
		unpause_cdda_from_cmd();
		return;
		break;
	case TOWNS_CDROM_CDDA_STOP:
		stop_cdda_from_cmd();
		return;
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
		return;
		break;
	case 0xff:
		// End of List
		set_dat(SCSI_STATUS_CHKCOND);
		return;
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
		int track = current_track;
		cdda_playing_frame = cdda_start_frame;
		if(cdda_end_frame > toc_table[track + 1].index1 && (cdda_end_frame - toc_table[track].pregap) <= toc_table[track + 1].index1) {
			auto_increment_track = true;
		}
		if(event_cdda_delay_play >= 0) {
			cancel_event(this, event_cdda_delay_play);
			event_cdda_delay_play = -1;
		}
		register_event(this, EVENT_CDDA_DELAY_PLAY, 10.0, false, &event_cdda_delay_play);
		
	}
}
	
void TOWNS_CDROM::set_subq(void)
{
	if(is_device_ready()) {
		// create track info
		uint32_t frame = (cdda_status == CDDA_OFF) ? cdda_start_frame : cdda_playing_frame;
		uint32_t msf_abs = lba_to_msf_alt(frame);
		int track;
		double delay_time = 10.0;
		track = current_track;
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
		
		write_signals(&output_subq_overrun, (subq_buffer->empty()) ? 0x00000000 : 0xffffffff);
		subq_buffer->clear();
		// http://www.ecma-international.org/publications/files/ECMA-ST/Ecma-130.pdf
		//subq_buffer->write(0x01 | (toc_table[track].is_audio ? 0x00 : 0x40));
		
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
		set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
	} else {
		//write_signals(&output_subq_overrun, (subq_buffer->empty()) ? 0x00000000 : 0xffffffff); // OK?
		subq_buffer->clear();
		// transfer length
		remain = subq_buffer->count();
		set_dat(is_device_ready() ? SCSI_STATUS_GOOD : SCSI_STATUS_CHKCOND);
		set_phase_delay(SCSI_PHASE_STATUS, 10.0);
	}
	return;
}
