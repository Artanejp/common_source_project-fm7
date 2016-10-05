/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.09.03-

	[ T3444A / T3444M ]
*/

#include "t3444a.h"
#include "disk.h"

// TODO: check status in data lost
#define FDC_STA_FDC_READY	0x08	// 1=Ready, 0=Busy
#define FDC_STA_CMD_ERROR	0x06	// Execute command other than Seek to Zero when the drive is not ready, or execute write command when the disk is write protected
#define FDC_STA_ID_MISSING	0x02	// ID field is missing
#define FDC_STA_ID_ERROR	0x05	// Track address in ID field is 0xff (defective track)
#define FDC_STA_SEEK_ERROR	0x07	// Seek error
#define FDC_STA_DATA_ERROR	0x01	// Find error in data field
#define FDC_STA_LAST_SECTOR	0x03	// Terminate read/write command at the last sector in the track when tnd signal is active (low)
#define FDC_STA_SUCCESS		0x00	// Command is successfully executed
#define FDC_STA_SUCCESS_DDM	0x04	// Command is successfully executed and detect the deleted data address mark

#define FDC_CMD_SEEK_ZERO	0x00	// Seek to Zero
#define FDC_CMD_SEEK		0x03	// Seek
#define FDC_CMD_WRITE_ID	0x08	// Write Index/ID
#define FDC_CMD_SEEK_WRITE_ID	0x0a	// Seek and Write Index/ID
#define FDC_CMD_READ		0x05	// Read Data
#define FDC_CMD_SEEK_READ	0x07	// Seek and Read Data
#define FDC_CMD_WRITE		0x09	// Write Data
#define FDC_CMD_WRITE_DDM	0x0d	// Write Data with Deleted Data Address Mark
#define FDC_CMD_SEEK_WRITE	0x0b	// Seek and Write Data
#define FDC_CMD_SEEK_WRITE_DDM	0x0f	// Seek and Write Data with Deleted Data Address Mark
#define FDC_CMD_SENCE_DRV_STAT	0x01	// Sence Drive Status

#define EVENT_SEEK		0
#define EVENT_SEARCH		1
#define EVENT_RQM		2
#define EVENT_LOST		3
#define EVENT_TND		4

void T3444A::cancel_my_event(int event)
{
	if(register_id[event] != -1) {
		cancel_event(this, register_id[event]);
		register_id[event] = -1;
	}
}

void T3444A::register_my_event(int event, double usec)
{
	cancel_my_event(event);
	register_event(this, event, usec, false, &register_id[event]);
}

void T3444A::register_seek_event()
{
	cancel_my_event(EVENT_SEEK);
	if(fdc[drvreg].track == seektrk) {
		register_event(this, EVENT_SEEK, 1, false, &register_id[EVENT_SEEK]);
	} else {
		register_event(this, EVENT_SEEK, timerflag ? 40000 : 25000, false, &register_id[EVENT_SEEK]);
	}
}

void T3444A::register_rqm_event(int bytes)
{
	double usec = disk[drvreg]->get_usec_per_bytes(bytes) - get_passed_usec(prev_rqm_clock);
	if(usec < 4) {
		usec = 4;
	}
	cancel_my_event(EVENT_RQM);
	register_event(this, EVENT_RQM, usec, false, &register_id[EVENT_RQM]);
}

void T3444A::register_lost_event(int bytes)
{
	cancel_my_event(EVENT_LOST);
	register_event(this, EVENT_LOST, disk[drvreg]->get_usec_per_bytes(bytes), false, &register_id[EVENT_LOST]);
}

void T3444A::initialize()
{
	// initialize d88 handler
	for(int i = 0; i < 4; i++) {
		disk[i] = new DISK(emu);
	}
	
	// initialize timing
	memset(fdc, 0, sizeof(fdc));
	
	// initialize fdc
	seektrk = 0;
	status = FDC_STA_FDC_READY | FDC_STA_SUCCESS;
	cmdreg = trkreg = secreg = datareg = 0;
	drvreg = sidereg = 0;
	timerflag = false;
	prev_rqm_clock = 0;
}

void T3444A::release()
{
	// release d88 handler
	for(int i = 0; i < 4; i++) {
		if(disk[i]) {
			disk[i]->close();
			delete disk[i];
		}
	}
}

void T3444A::reset()
{
	for(int i = 0; i < 4; i++) {
		fdc[i].track = 0;
		fdc[i].index = 0;
		fdc[i].access = false;
	}
	for(int i = 0; i < array_length(register_id); i++) {
		register_id[i] = -1;
	}
	now_search = false;
	set_rqm(false);
}

void T3444A::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 3) {
	case 0:
		// command reg
		cmdreg = data & 0x0f;
		process_cmd();
		break;
	case 1:
		// track reg
		trkreg = data & 0x7f;
		timerflag = ((data & 0x80) != 0);
		break;
	case 2:
		// sector reg
		secreg = data & 0x7f;
#ifdef HAS_T3444M
		sidereg = (data >> 7) & 1;
#endif
		break;
	case 3:
		// data reg
		datareg = data;
		if(motor_on && rqm && !now_search) {
			if(cmdreg == FDC_CMD_WRITE || cmdreg == FDC_CMD_WRITE_DDM) {
				// write data
				if(fdc[drvreg].index < disk[drvreg]->sector_size.sd) {
					disk[drvreg]->sector[fdc[drvreg].index++] = datareg;
					disk[drvreg]->set_deleted(cmdreg == FDC_CMD_WRITE_DDM);
					fdc[drvreg].access = true;
				}
				set_rqm(false);
				cancel_my_event(EVENT_LOST);
				
				if(fdc[drvreg].index >= disk[drvreg]->sector_size.sd) {
#ifdef _FDC_DEBUG_LOG
					this->out_debug_log(_T("FDC\tWRITE DATA FINISHED\n"));
#endif
					// 2S: 300rpm, 3100bytes/track -> 0.0155bytes/us
					register_my_event(EVENT_TND, 100); // 0.0155bytes/us * 100us = 1.55bytes < GAP3
				} else {
					if(fdc[drvreg].index == 1) {
						register_rqm_event(fdc[drvreg].bytes_before_2nd_rqm);
					} else {
						register_rqm_event(1);
					}
				}
			} else if(cmdreg == FDC_CMD_WRITE_ID) {
				// write index/id
				if(fdc[drvreg].index < SECTORS_IN_TRACK * 4) {
					sector_id[fdc[drvreg].index++] = datareg;
				}
				set_rqm(false);
				cancel_my_event(EVENT_LOST);
				
				if(fdc[drvreg].index >= SECTORS_IN_TRACK * 4) {
					// format in single-density
					bool drive_mfm = disk[drvreg]->drive_mfm;
					disk[drvreg]->drive_mfm = false;
					disk[drvreg]->format_track(fdc[drvreg].track, sidereg);
					disk[drvreg]->drive_mfm = drive_mfm;
					for(int i = 0; i < SECTORS_IN_TRACK; i++) {
						disk[drvreg]->insert_sector(sector_id[i * 4], sector_id[i * 4 + 1], sector_id[i * 4 + 2], sector_id[i * 4 + 3], false, false, 0xff, 128);
					}
					status |= FDC_STA_FDC_READY;
				} else {
					register_rqm_event(1);
				}
			}
		}
		break;
	}
}

uint32_t T3444A::read_io8(uint32_t addr)
{
	switch(addr & 3) {
	case 0:
		// status reg
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC\tSTATUS=%02x\n"),status);
#endif
		return status;
	case 3:
		// data reg
		if(motor_on && rqm && !now_search) {
			if(cmdreg == FDC_CMD_READ) {
				// read data
				if(fdc[drvreg].index < disk[drvreg]->sector_size.sd) {
					datareg = disk[drvreg]->sector[fdc[drvreg].index++];
					fdc[drvreg].access = true;
				}
				set_rqm(false);
				cancel_my_event(EVENT_LOST);
				
				if(fdc[drvreg].index >= disk[drvreg]->sector_size.sd) {
#ifdef _FDC_DEBUG_LOG
					this->out_debug_log(_T("FDC\tREAD DATA FINISHED\n"));
#endif
//					if(status == FDC_STA_DATA_ERROR) {
//						status |= FDC_STA_FDC_READY;
//					} else {
						register_my_event(EVENT_TND, 100);
//					}
				} else {
					register_rqm_event(1);
				}
			}
		}
		return datareg;
	}
	return 0xff;
}

void T3444A::write_dma_io8(uint32_t addr, uint32_t data)
{
	write_io8(3, data);
}

uint32_t T3444A::read_dma_io8(uint32_t addr)
{
	return read_io8(3);
}

void T3444A::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_T3444A_DRIVE) {
		if(data & 4) {
			drvreg = data & 3;
		}
	} else if(id == SIG_T3444A_TND) {
		tnd = ((data & mask) != 0);
	} else if(id == SIG_T3444A_MOTOR) {
		motor_on = ((data & mask) != 0);
	}
#if defined(USE_SOUND_FILES)
	else if((id >= SIG_SOUNDER_MUTE) && (id < (SIG_SOUNDER_MUTE + 2))) {
		snd_mute = ((data & mask) != 0);
	} else if((id >= SIG_SOUNDER_RELOAD) && (id < (SIG_SOUNDER_RELOAD + 2))) {
		reload_sound_data(id - SIG_SOUNDER_RELOAD);
	}
#endif
}

uint32_t T3444A::read_signal(int ch)
{
	if(ch == SIG_T3444A_DRDY) {
		return (disk[drvreg]->inserted && motor_on) ? 1 : 0;
	} else if(ch == SIG_T3444A_CRDY) {
		return (status & FDC_STA_FDC_READY) ? 1 : 0;
	} else if(ch == SIG_T3444A_RQM) {
		// this is negative signal
		return rqm ? 1 : 0;
	}
	
	// get access status
	uint32_t stat = 0;
	for(int i = 0; i < 4; i++) {
		if(fdc[i].access) {
			stat |= 1 << i;
		}
		fdc[i].access = false;
	}
	if(now_search) {
		stat |= 1 << drvreg;
	}
	return stat;
}

void T3444A::event_callback(int event_id, int err)
{
	register_id[event_id] = -1;
	
	switch(event_id) {
	case EVENT_SEEK:
		if(seektrk > fdc[drvreg].track) {
			fdc[drvreg].track++;
#if defined(USE_SOUND_FILES)
			add_sound(T3444A_SND_TYPE_SEEK);
#endif			
		} else if(seektrk < fdc[drvreg].track) {
			fdc[drvreg].track--;
#if defined(USE_SOUND_FILES)
			add_sound(T3444A_SND_TYPE_SEEK);
#endif			
		}
		if(seektrk != fdc[drvreg].track) {
			register_seek_event();
			break;
		}
		if(cmdreg == FDC_CMD_SEEK_READ) {
			cmdreg = FDC_CMD_READ;
			process_cmd();
		} else if(cmdreg == FDC_CMD_SEEK_WRITE) {
			cmdreg = FDC_CMD_WRITE;
			process_cmd();
		} else if(cmdreg == FDC_CMD_SEEK_WRITE_DDM) {
			cmdreg = FDC_CMD_WRITE_DDM;
			process_cmd();
		} else {
			status |= FDC_STA_FDC_READY;
		}
		break;
	case EVENT_SEARCH:
		if(status == FDC_STA_SUCCESS || status == FDC_STA_SUCCESS_DDM || status == FDC_STA_DATA_ERROR) {
			if(cmdreg == FDC_CMD_WRITE || cmdreg == FDC_CMD_WRITE_DDM) {
				register_lost_event(8);
			} else if(cmdreg == FDC_CMD_SEEK_WRITE_ID) {
				register_lost_event(1); // is this okay ???
			} else {
				register_lost_event(1);
			}
			fdc[drvreg].cur_position = fdc[drvreg].next_trans_position;
			fdc[drvreg].prev_clock = prev_rqm_clock = get_current_clock();
			set_rqm(true);
		} else {
			status |= FDC_STA_FDC_READY;
		}
		now_search = false;
		break;
	case EVENT_RQM:
		if(!(status & FDC_STA_FDC_READY)) {
			if((cmdreg == FDC_CMD_WRITE || cmdreg == FDC_CMD_WRITE_DDM) && fdc[drvreg].index == 0) {
				fdc[drvreg].cur_position = (fdc[drvreg].cur_position + fdc[drvreg].bytes_before_2nd_rqm) % disk[drvreg]->get_track_size();
			} else {
				fdc[drvreg].cur_position = (fdc[drvreg].cur_position + 1) % disk[drvreg]->get_track_size();
			}
			fdc[drvreg].prev_clock = prev_rqm_clock = get_current_clock();
			set_rqm(true);
			if(cmdreg == FDC_CMD_SEEK_WRITE_ID) {
				register_lost_event(1); // is this okay ???
			} else {
				register_lost_event(1);
			}
		}
		break;
	case EVENT_LOST:
		status |= FDC_STA_FDC_READY;
		break;
	case EVENT_TND:
		if(!tnd) {
			if(secreg < SECTORS_IN_TRACK) {
				secreg++;
#ifdef _FDC_DEBUG_LOG
				this->out_debug_log(_T("FDC\tTND AND CONTINUE SEC=%d\n"), secreg);
#endif
				cmd_read_write();
			} else {
//				secreg = 1;
#ifdef _FDC_DEBUG_LOG
				this->out_debug_log(_T("FDC\tTND BUT TERMINATED SEC=%d\n"), secreg);
#endif
				status = FDC_STA_FDC_READY | FDC_STA_LAST_SECTOR;
			}
		} else {
			status |= FDC_STA_FDC_READY;
		}
		break;
	}
}

// ----------------------------------------------------------------------------
// command
// ----------------------------------------------------------------------------

void T3444A::process_cmd()
{
#ifdef _FDC_DEBUG_LOG
	static const _TCHAR *cmdstr[0x10] = {
		_T("Seek to Zero"),
		_T("Sence Drive Status"),
		_T("Unknown"),
		_T("Seek"),
		_T("Unknown"),
		_T("Read Data"),
		_T("Unknown"),
		_T("Seek and Read Data"),
		_T("Write Index/ID"),
		_T("Write Data"),
		_T("Seek and Write Index/ID"),
		_T("Seek and Write Data"),
		_T("Unknown"),
		_T("Write Data with Deleted Data Address Mark"),
		_T("Unknown"),
		_T("Seek and Write Data with Deleted Data Address Mark"),
	};
	if(cmdreg == cmdreg) {
		this->out_debug_log(_T("FDC\tCMD=%2xh (%s) DATA=%2xh DRV=%d TRK=%3d SIDE=%d SEC=%2d\n"), cmdreg, cmdstr[cmdreg], datareg, drvreg, trkreg, sidereg, secreg);
	}
#endif
	status = 0; // FDC is busy
	
	switch(cmdreg) {
	case FDC_CMD_SEEK_ZERO:
		cmd_seek_zero();
		break;
	case FDC_CMD_SEEK:
	case FDC_CMD_SEEK_READ:
	case FDC_CMD_SEEK_WRITE:
	case FDC_CMD_SEEK_WRITE_DDM:
	case FDC_CMD_SEEK_WRITE_ID:
		cmd_seek();
		break;
	case FDC_CMD_READ:
	case FDC_CMD_WRITE:
	case FDC_CMD_WRITE_DDM:
		cmd_read_write();
		break;
	case FDC_CMD_WRITE_ID:
		cmd_write_id();
		break;
	case FDC_CMD_SENCE_DRV_STAT:
		cmd_sence();
		break;
	default:
		status = FDC_STA_FDC_READY | FDC_STA_CMD_ERROR; // is this okay ???
		break;
	}
}

void T3444A::cmd_seek_zero()
{
	if(!disk[drvreg]->inserted || !motor_on) {
//		status = FDC_STA_FDC_READY | FDC_STA_SEEK_ERROR;
		status = FDC_STA_FDC_READY | FDC_STA_SUCCESS;
		seektrk = trkreg = 0;
	} else {
		status = FDC_STA_SUCCESS;
		seektrk = trkreg = 0;
		register_seek_event();
	}
}

void T3444A::cmd_seek()
{
	if(!disk[drvreg]->inserted || !motor_on) {
		status = FDC_STA_FDC_READY | FDC_STA_CMD_ERROR;
	} else if(trkreg > 34) {
		status = FDC_STA_FDC_READY | FDC_STA_SEEK_ERROR;
	} else {
		status = FDC_STA_SUCCESS;
		seektrk = trkreg;
		register_seek_event();
	}
}

void T3444A::cmd_read_write()
{
	if(!((status = search_sector()) & FDC_STA_FDC_READY)) {
		double time;
		if(status == FDC_STA_SUCCESS || status == FDC_STA_SUCCESS_DDM || status == FDC_STA_DATA_ERROR) {
			time = get_usec_to_start_trans();
		} else {
			time = get_usec_to_detect_index_hole(3);
		}
		now_search = true;
		register_my_event(EVENT_SEARCH, time);
		cancel_my_event(EVENT_LOST);
	}
}

void T3444A::cmd_write_id()
{
	if(!disk[drvreg]->inserted || !motor_on || disk[drvreg]->write_protected) {
		status =  FDC_STA_FDC_READY | FDC_STA_CMD_ERROR;
	} else {
		// raise first rqm soon
		fdc[drvreg].next_trans_position = (get_cur_position() + 1) % disk[drvreg]->get_track_size();
		fdc[drvreg].index = 0;
		status = FDC_STA_SUCCESS;
		now_search = true;
		register_my_event(EVENT_SEARCH, disk[drvreg]->get_usec_per_bytes(1));
		cancel_my_event(EVENT_LOST);
	}
}

void T3444A::cmd_sence()
{
	if(!disk[drvreg]->inserted || !motor_on) {
		status = FDC_STA_FDC_READY | FDC_STA_CMD_ERROR;
	} else {
		trkreg = fdc[drvreg].track;
		status = FDC_STA_FDC_READY | FDC_STA_SUCCESS;
	}
}

// ----------------------------------------------------------------------------
// media handler
// ----------------------------------------------------------------------------

uint8_t T3444A::search_sector()
{
	// drive not ready
	if(!disk[drvreg]->inserted || !motor_on) {
		return FDC_STA_FDC_READY | FDC_STA_CMD_ERROR;
	}
	
	// write protect
	if(cmdreg == FDC_CMD_WRITE || cmdreg == FDC_CMD_WRITE_DDM || cmdreg == FDC_CMD_WRITE_ID) {
		if(disk[drvreg]->write_protected) {
			return FDC_STA_FDC_READY | FDC_STA_CMD_ERROR;
		}
	}
	
	// get track
	if(!disk[drvreg]->get_track(fdc[drvreg].track, sidereg)) {
		return FDC_STA_ID_MISSING;
	}
	
	// get current position
	int sector_num = disk[drvreg]->sector_num.sd;
	int position = get_cur_position();
	
	if(position > disk[drvreg]->sync_position[sector_num - 1]) {
		position -= disk[drvreg]->get_track_size();
	}
	
	// first scanned sector
	int first_sector = 0;
	for(int i = 0; i < sector_num; i++) {
		if(position < disk[drvreg]->sync_position[i]) {
			first_sector = i;
			break;
		}
	}
	
	// scan sectors
	for(int i = 0; i < sector_num; i++) {
		// get sector
		int index = (first_sector + i) % sector_num;
		disk[drvreg]->get_sector(-1, -1, index);
		
		// check id
		if(disk[drvreg]->id[0] == 0xff) {
			return FDC_STA_ID_ERROR;
		}
		if(disk[drvreg]->id[0] != trkreg) {
			trkreg = disk[drvreg]->id[0];
			return FDC_STA_SEEK_ERROR;
		}
		if(disk[drvreg]->id[2] != secreg) {
			continue;
		}
		if(disk[drvreg]->sector_size.sd == 0) {
			continue;
		}
		if(disk[drvreg]->addr_crc_error && !disk[drvreg]->ignore_crc()) {
			// id crc error
			disk[drvreg]->sector_size.sd = 0;
			return FDC_STA_ID_MISSING; // is this okay ???
		}
		
		// sector found
		if(cmdreg == FDC_CMD_WRITE || cmdreg == FDC_CMD_WRITE_DDM) {
			fdc[drvreg].next_trans_position = disk[drvreg]->id_position[index] + 4 + 2;
			fdc[drvreg].bytes_before_2nd_rqm = disk[drvreg]->data_position[index] - fdc[drvreg].next_trans_position;
		} else {
			fdc[drvreg].next_trans_position = disk[drvreg]->data_position[index] + 1;
		}
		fdc[drvreg].next_sync_position = disk[drvreg]->sync_position[index];
		fdc[drvreg].index = 0;
#ifdef _FDC_DEBUG_LOG
		this->out_debug_log(_T("FDC\tSECTOR FOUND SIZE=$%04x ID=%02x %02x %02x %02x CRC=%02x %02x CRC_ERROR=%d\n"),
			disk[drvreg]->sector_size.sd,
			disk[drvreg]->id[0], disk[drvreg]->id[1], disk[drvreg]->id[2], disk[drvreg]->id[3],
			disk[drvreg]->id[4], disk[drvreg]->id[5],
			disk[drvreg]->data_crc_error ? 1 : 0);
#endif
		if(disk[drvreg]->data_crc_error && !disk[drvreg]->ignore_crc()) {
			return FDC_STA_DATA_ERROR;
		} else if(disk[drvreg]->deleted || cmdreg == FDC_CMD_WRITE_DDM) {
			return FDC_STA_SUCCESS_DDM;
		} else {
			return FDC_STA_SUCCESS;
		}
	}
	
	// sector not found
	disk[drvreg]->sector_size.sd = 0;
	return FDC_STA_ID_MISSING;
}

// ----------------------------------------------------------------------------
// timing
// ----------------------------------------------------------------------------

int T3444A::get_cur_position()
{
	return (fdc[drvreg].cur_position + disk[drvreg]->get_bytes_per_usec(get_passed_usec(fdc[drvreg].prev_clock))) % disk[drvreg]->get_track_size();
}

double T3444A::get_usec_to_start_trans()
{
	// get time from current position
	double time = get_usec_to_next_trans_pos();
	return time;
}

double T3444A::get_usec_to_next_trans_pos()
{
	int position = get_cur_position();
	
	if(disk[drvreg]->invalid_format) {
		// XXX: this track is invalid format and the calculated sector position may be incorrect.
		// so use the constant period
		return 50000;
	} else if(/*disk[drvreg]->no_skew &&*/ !disk[drvreg]->correct_timing()) {
		// XXX: this image may be a standard image or coverted from a standard image and skew may be incorrect,
		// so use the period to search the next sector from the current position
		int sector_num = disk[drvreg]->sector_num.sd;
		int bytes = -1;
		
		if(position > disk[drvreg]->sync_position[sector_num - 1]) {
			position -= disk[drvreg]->get_track_size();
		}
		for(int i = 0; i < sector_num; i++) {
			if(position < disk[drvreg]->sync_position[i]) {
				if(cmdreg == FDC_CMD_WRITE || cmdreg == FDC_CMD_WRITE_DDM) {
					bytes = (disk[drvreg]->id_position[i] + 4 + 2) - position;
				} else {
					bytes = (disk[drvreg]->data_position[i] + 1) - position;
				}
				if(bytes < 0) {
					bytes += disk[drvreg]->get_track_size(); // to make sure
				}
				break;
			}
		}
		if(bytes > 0) {
			return disk[drvreg]->get_usec_per_bytes(bytes);
		}
		return 50000;
	}
	int bytes = fdc[drvreg].next_trans_position - position;
	if(fdc[drvreg].next_sync_position < position || bytes < 0) {
		bytes += disk[drvreg]->get_track_size();
	}
	return disk[drvreg]->get_usec_per_bytes(bytes);
}

double T3444A::get_usec_to_detect_index_hole(int count)
{
	int position = get_cur_position();
	int bytes = disk[drvreg]->get_track_size() * count - position;
	if(bytes < 0) {
		bytes += disk[drvreg]->get_track_size();
	}
	return disk[drvreg]->get_usec_per_bytes(bytes);
}

// ----------------------------------------------------------------------------
// rqm
// ----------------------------------------------------------------------------

void T3444A::set_rqm(bool val)
{
	write_signals(&outputs_rqm, (rqm = val) ? 0xffffffff : 0);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void T3444A::open_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < 4 && drv < MAX_DRIVE) {
		disk[drv]->open(file_path, bank);
	}
}

void T3444A::close_disk(int drv)
{
	if(drv < 4 && drv < MAX_DRIVE) {
		disk[drv]->close();
	}
}

bool T3444A::is_disk_inserted(int drv)
{
	if(drv < 4 && drv < MAX_DRIVE) {
		return disk[drv]->inserted;
	}
	return false;
}

void T3444A::is_disk_protected(int drv, bool value)
{
	if(drv < 4 && drv < MAX_DRIVE) {
		disk[drv]->write_protected = value;
	}
}

bool T3444A::is_disk_protected(int drv)
{
	if(drv < 4 && drv < MAX_DRIVE) {
		return disk[drv]->write_protected;
	}
	return false;
}

void T3444A::set_drive_type(int drv, uint8_t type)
{
	if(drv < 4 && drv < MAX_DRIVE) {
		disk[drv]->drive_type = type;
	}
}

uint8_t T3444A::get_drive_type(int drv)
{
	if(drv < 4 && drv < MAX_DRIVE) {
		return disk[drv]->drive_type;
	}
	return DRIVE_TYPE_UNK;
}

void T3444A::set_drive_rpm(int drv, int rpm)
{
	if(drv < 4 && drv < MAX_DRIVE) {
		disk[drv]->drive_rpm = rpm;
	}
}

void T3444A::set_drive_mfm(int drv, bool mfm)
{
	if(drv < 4 && drv < MAX_DRIVE) {
		disk[drv]->drive_mfm = mfm;
	}
}
// Set sound data.
// TYPE=
//     0: FDD SEEK
//     1: HEAD ENGAGE (Optional?)
#if defined(USE_SOUND_FILES)
void T3444A::add_sound(int type)
{
	int *p;
	if(type == T3444A_SND_TYPE_SEEK) {
		p = snd_seek_mix_tbl;
	} else if(type == T3444A_SND_TYPE_HEAD) {
		p = snd_head_mix_tbl;
	} else {
		return;
	}
	for(int i = 0; i < T3444A_SND_TBL_MAX; i++) {
		if(p[i] < 0) {
			p[i] = 0;
			break;
		}
	}
}

bool T3444A::load_sound_data(int type, const _TCHAR *pathname)
{
	if((type < 0) || (type > 1)) return false;
	int16_t *data = NULL;
	int dst_size = 0;
	int id = (this_device_id << 8) + type;
	const _TCHAR *sp;
	sp = create_local_path(pathname);
	emu->load_sound_file(id, sp, &data, &dst_size);
	if((dst_size <= 0) || (data == NULL)) { // Failed
		this->out_debug_log("ID=%d : Failed to load SOUND FILE for %s:%s", id, (type == 0) ? _T("SEEK") : _T("HEAD") ,pathname);
		return false;
	} else {
		int utl_size = dst_size * 2 * sizeof(int16_t);
		int alloc_size = utl_size + 64;
		switch(type) {
		case T3444A_SND_TYPE_SEEK: // SEEK
			snd_seek_data = (int16_t *)malloc(alloc_size);
			memcpy(snd_seek_data, data, utl_size);
			strncpy(snd_seek_name, pathname, 511);
			snd_seek_samples_size = dst_size;
			break;
		case T3444A_SND_TYPE_HEAD: // HEAD
			snd_seek_data = (int16_t *)malloc(alloc_size);
			memcpy(snd_head_data, data, utl_size);
			strncpy(snd_head_name, pathname, 511);
			snd_head_samples_size = dst_size;
			break;
		default:
			this->out_debug_log("ID=%d : Illegal type (%d): 0 (SEEK SOUND) or 1 (HEAD SOUND) is available.",
								id, type);
			return false;
		}
		this->out_debug_log("ID=%d : Success to load SOUND FILE for %s:%s",
							id, (type == 0) ? _T("SEEK") : _T("HEAD") ,
							pathname);
	}
	return true;
}

void T3444A::release_sound_data(int type)
{
	switch(type) {
	case T3444A_SND_TYPE_SEEK: // SEEK
		if(snd_seek_data != NULL) free(snd_seek_data);
		memset(snd_seek_name, 0x00, sizeof(snd_seek_name));
		snd_seek_data = NULL;
		break;
	case T3444A_SND_TYPE_HEAD: // HEAD
		if(snd_head_data != NULL) free(snd_head_data);
		memset(snd_head_name, 0x00, sizeof(snd_head_name));
		snd_head_data = NULL;
			break;
	default:
		break;
	}
}

bool T3444A::reload_sound_data(int type)
{
	switch(type) {
	case T3444A_SND_TYPE_SEEK: // SEEK
		if(snd_seek_data != NULL) free(snd_seek_data);
		break;
	case T3444A_SND_TYPE_HEAD:
		if(snd_seek_data != NULL) free(snd_seek_data);
		break;
	default:
		return false;
		break;
	}
	_TCHAR *p = (type == T3444A_SND_TYPE_SEEK) ? snd_seek_name : snd_head_name;
    _TCHAR tmps[512];
	strncpy(tmps, p, 511);
	return load_sound_data(type, tmps);
}

void T3444A::mix_main(int32_t *dst, int count, int16_t *src, int *table, int samples)
{
	int ptr, pp;
	int i, j, k;
	int32_t data[2];
	int32_t *dst_tmp;
	for(i=0; i < T3444A_SND_TBL_MAX; i++) {
		ptr = table[i];
		if(ptr >= 0) {
			if(ptr < samples) {
				if(!snd_mute) {
					pp = ptr << 1;
					dst_tmp = dst;
					k = 0;
					for(j = 0; j < count; j++) {
						if(ptr >= samples) {
							break;
						}
						data[0] = (int32_t)src[pp + 0];
						data[1] = (int32_t)src[pp + 1];
						dst_tmp[k + 0] += apply_volume((int32_t)data[0], snd_level_l);
						dst_tmp[k + 1] += apply_volume((int32_t)data[1], snd_level_r);
						k += 2;
						pp += 2;
						ptr++;
					}
				} else {
					ptr += count;
				}
			}
			if(ptr >= samples) {
				table[i] = -1;
			} else {
				table[i] = ptr;
			}
		}
	}
}

void T3444A::mix(int32_t *buffer, int cnt)
{
	if(snd_seek_data != NULL) mix_main(buffer, cnt, snd_seek_data, snd_seek_mix_tbl, snd_seek_samples_size);
	if(snd_head_data != NULL) mix_main(buffer, cnt, snd_head_data, snd_head_mix_tbl, snd_head_samples_size);
}

void T3444A::set_volume(int ch, int decibel_l, int decibel_r)
{
	snd_level_l = decibel_to_volume(decibel_l);
	snd_level_r = decibel_to_volume(decibel_r);
}
#endif

#define STATE_VERSION	2

void T3444A::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(fdc, sizeof(fdc), 1);
	for(int i = 0; i < MAX_DRIVE; i++) {
		disk[i]->save_state(state_fio);
	}
	state_fio->FputUint8(status);
	state_fio->FputUint8(cmdreg);
	state_fio->FputUint8(trkreg);
	state_fio->FputUint8(secreg);
	state_fio->FputUint8(datareg);
	state_fio->FputUint8(drvreg);
	state_fio->FputUint8(sidereg);
	state_fio->FputBool(timerflag);
	state_fio->Fwrite(sector_id, sizeof(sector_id), 1);
	state_fio->Fwrite(register_id, sizeof(register_id), 1);
	state_fio->FputBool(now_search);
	state_fio->FputInt32(seektrk);
	state_fio->FputBool(rqm);
	state_fio->FputBool(tnd);
	state_fio->FputBool(motor_on);
	state_fio->FputUint32(prev_rqm_clock);
#if defined(USE_SOUND_FILES)
	state_fio->Fwrite(snd_seek_name, sizeof(snd_seek_name), 1);
	state_fio->Fwrite(snd_head_name, sizeof(snd_head_name), 1);
	for(int i = 0; i < T3444A_SND_TBL_MAX; i++) {
		state_fio->FputInt32(snd_seek_mix_tbl[i]);
	}
	for(int i = 0; i < T3444A_SND_TBL_MAX; i++) {
		state_fio->FputInt32(snd_head_mix_tbl[i]);
	}
	state_fio->FputBool(snd_mute);
	state_fio->FputInt32(snd_level_l);
	state_fio->FputInt32(snd_level_r);
#endif
}

bool T3444A::load_state(FILEIO* state_fio)
{
	bool pending = false;
	uint32_t s_version = state_fio->FgetUint32();
	uint32_t desired_version = STATE_VERSION;
	if(s_version != STATE_VERSION) {
		if(s_version == 1) {
			pending = true;
		} else {
			return false;
		}
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(fdc, sizeof(fdc), 1);
	for(int i = 0; i < MAX_DRIVE; i++) {
		if(!disk[i]->load_state(state_fio)) {
			return false;
		}
	}
	status = state_fio->FgetUint8();
	cmdreg = state_fio->FgetUint8();
	trkreg = state_fio->FgetUint8();
	secreg = state_fio->FgetUint8();
	datareg = state_fio->FgetUint8();
	drvreg = state_fio->FgetUint8();
	sidereg = state_fio->FgetUint8();
	timerflag = state_fio->FgetBool();
	state_fio->Fread(sector_id, sizeof(sector_id), 1);
	state_fio->Fread(register_id, sizeof(register_id), 1);
	now_search = state_fio->FgetBool();
	seektrk = state_fio->FgetInt32();
	rqm = state_fio->FgetBool();
	tnd = state_fio->FgetBool();
	motor_on = state_fio->FgetBool();
	prev_rqm_clock = state_fio->FgetUint32();
#if defined(USE_SOUND_FILES)
	if(!pending) {
		state_fio->Fread(snd_seek_name, sizeof(snd_seek_name), 1);
		state_fio->Fread(snd_head_name, sizeof(snd_head_name), 1);
		for(int i = 0; i < T3444A_SND_TBL_MAX; i++) {
			snd_seek_mix_tbl[i] = state_fio->FgetInt32();
		}
		for(int i = 0; i < T3444A_SND_TBL_MAX; i++) {
			snd_head_mix_tbl[i] = state_fio->FgetInt32();
		}
		snd_mute = state_fio->FgetBool();
		snd_level_l = state_fio->FgetInt32();
		snd_level_r = state_fio->FgetInt32();
		if(snd_seek_data != NULL) free(snd_seek_data);
		if(snd_head_data != NULL) free(snd_head_data);
		if(strlen(snd_seek_name) > 0) {
			_TCHAR tmps[512];
			strncpy(tmps, snd_seek_name, 511);
			load_sound_data(T3444A_SND_TYPE_SEEK, (const _TCHAR *)tmps);
		}
		if(strlen(snd_head_name) > 0) {
			_TCHAR tmps[512];
			strncpy(tmps, snd_head_name, 511);
			load_sound_data(T3444A_SND_TYPE_HEAD, (const _TCHAR *)tmps);
		}
	}
#endif
	return true;
}

