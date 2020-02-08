/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2016.03.01-

	[ SCSI/SASI hard disk drive ]
*/

#include "scsi_hdd.h"
#include "harddisk.h"
#include "../fifo.h"

void SCSI_HDD::release()
{
	for(int i = 0; i < 8; i++) {
		if(disk[i] != NULL) {
			disk[i]->close();
			delete disk[i];
		}
	}
	SCSI_DEV::release();
}

void SCSI_HDD::start_command()
{
	switch(command[0]) {
	case SCSI_CMD_SEND_DIAG:
		{
			pair16_t len;
			len.b.h = command[3];
			len.b.l = command[4];
//			remain = len.w;
//			remain *= logical_block_size();
			// ToDo: Implement dummy write
		}
//		return;
		break;
	case SCSI_CMD_RCV_DIAG:
		{
			// ToDo: Implement dummy read
		}
		break;
	case SCSI_CMD_START_STP: // Start/Stop
		buffer->clear();
		out_debug_log(_T("[SCSI_HDD:ID=%d] START/STOP Unit \n"), scsi_id);
		set_dat(SCSI_STATUS_GOOD);
		set_sense_code(SCSI_SENSE_NOSENSE);
		set_phase_delay(SCSI_PHASE_STATUS, 10.0);
		return;
		break;
	case SCSI_CMD_REZERO: // Recaliblate
		{
			double usec = 10.0;
			for(int drv = 0; drv < 8; drv++) {
				long pos = 0;
				if(disk[drv] != NULL) {
					pos = disk[drv]->get_cur_position();
					long distance = pos / (disk[drv]->get_headers() * disk[drv]->get_sectors_per_cylinder());
					if(distance > 0) {
						usec += ((double)seek_time * (double)distance);
					} else {
						usec += 10.0;
					}
				}
			}
			buffer->clear();
			if(usec < (double)seek_time) usec = (double)seek_time;
			out_debug_log(_T("[SCSI_HDD:ID=%d] RECALIBRATE Total Seek time=%fus\n"), scsi_id, usec);
			set_dat(SCSI_STATUS_GOOD);
			set_sense_code(SCSI_SENSE_NOSENSE);
			set_phase_delay(SCSI_PHASE_STATUS, usec);
		}
		return;
		break;
	}
	SCSI_DEV::start_command();
}

void SCSI_HDD::reset()
{
	if(!is_hot_swappable) {
		for(int drv = 0; drv < 8; drv++) {
			if(disk[drv] != NULL) {
				if(image_path[drv][0] != _T('\0') && FILEIO::IsFileExisting(image_path[drv])) {
					disk[drv]->open(image_path[drv], sector_size[drv]);
				} else {
					disk[drv]->close();
				}
			}
		}
	}
	SCSI_DEV::reset();
}

void SCSI_HDD::open(int drv, const _TCHAR* file_path, int default_sector_size)
{
	if(drv < 8 && disk[drv] != NULL) {
		if(!is_hot_swappable) {
			my_tcscpy_s(image_path[drv], _MAX_PATH, file_path);
			sector_size[drv] = default_sector_size;
		} else {
			disk[drv]->open(file_path, default_sector_size);
		}
	}
}

void SCSI_HDD::close(int drv)
{
	if(drv < 8 && disk[drv] != NULL) {
		if(!is_hot_swappable) {
			image_path[drv][0] = _T('\0');
		} else {
			disk[drv]->close();
		}
	}
}

bool SCSI_HDD::mounted(int drv)
{
	if(drv < 8 && disk[drv] != NULL) {
		if(!is_hot_swappable) {
			return (image_path[drv][0] != _T('\0'));
		} else {
			return disk[drv]->mounted();
		}
	}
	return false;
}

bool SCSI_HDD::accessed(int drv)
{
	if(drv < 8 && disk[drv] != NULL) {
		return disk[drv]->accessed();
	}
	return false;
}

bool SCSI_HDD::is_device_existing()
{
	for(int i = 0; i < 8; i++) {
		if(disk[i] != NULL && disk[i]->mounted()) {
			return true;
		}
	}
	return false;
}

uint32_t SCSI_HDD::physical_block_size()
{
	HARDDISK *unit = disk[get_logical_unit_number()];
	
	if(unit != NULL && unit->mounted()) {
		return unit->sector_size;
	}
	return 0;// 512;
}

uint32_t SCSI_HDD::logical_block_size()
{
	HARDDISK *unit = disk[get_logical_unit_number()];
	
	if(unit != NULL && unit->mounted()) {
		return unit->sector_size;
	}
	return 0;// 512;
}

uint32_t SCSI_HDD::max_logical_block_addr()
{
	HARDDISK *unit = disk[get_logical_unit_number()];
	
	if(unit != NULL && unit->mounted() && unit->sector_num > 0) {
		return unit->sector_num - 1;
	}
	return 0;
}

bool SCSI_HDD::read_buffer(int length)
{
	if(!(command[0] == SCSI_CMD_READ6 || command[0] == SCSI_CMD_READ10 || command[0] == SCSI_CMD_READ12)) {
		for(int i = 0; i < length; i++) {
			buffer->write(0);
			position++;
		}
		set_sense_code(SCSI_SENSE_NOSENSE);
		return true;
	}
	HARDDISK *unit = disk[get_logical_unit_number()];
	
	if(!(unit != NULL && unit->mounted())) {
		set_sense_code(SCSI_SENSE_NOTREADY);
		return false;
	}
	while(length > 0) {
		uint8_t tmp_buffer[SCSI_BUFFER_SIZE];
		int tmp_length = min(length, (int)sizeof(tmp_buffer));
		
		if(!unit->read_buffer((long)position, tmp_length, tmp_buffer)) {
			set_sense_code(SCSI_SENSE_ILLGLBLKADDR); //SCSI_SENSE_NORECORDFND
			return false;
		}
		for(int i = 0; i < tmp_length; i++) {
			buffer->write(tmp_buffer[i]);
 		}
		length -= tmp_length;
		position += tmp_length;
 	}
	set_sense_code(SCSI_SENSE_NOSENSE);
	return true;
}

bool SCSI_HDD::write_buffer(int length)
{
	if(!(command[0] == SCSI_CMD_WRITE6 || command[0] == SCSI_CMD_WRITE10 || command[0] == SCSI_CMD_WRITE12)) {
		for(int i = 0; i < length; i++) {
			buffer->read();
			position++;
		}
		set_sense_code(SCSI_SENSE_NOSENSE);
		return true;
	}
	HARDDISK *unit = disk[get_logical_unit_number()];
	
	if(!(unit != NULL && unit->mounted())) {
		set_sense_code(SCSI_SENSE_NOTREADY);
		return false;
	}
	while(length > 0) {
		uint8_t tmp_buffer[SCSI_BUFFER_SIZE];
		int tmp_length = min(length, (int)sizeof(tmp_buffer));
		
		for(int i = 0; i < tmp_length; i++) {
			tmp_buffer[i] = buffer->read();
		}
		if(!unit->write_buffer((long)position, tmp_length, tmp_buffer)) {
			set_sense_code(SCSI_SENSE_ILLGLBLKADDR); //SCSI_SENSE_NORECORDFND
			return false;
		}
		length -= tmp_length;
		position += tmp_length;
 	}
	set_sense_code(SCSI_SENSE_NOSENSE);
	return true;
}

#define STATE_VERSION	3

bool SCSI_HDD::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	/*
 	for(int drv = 0; drv < 8; drv++) {
 		if(disk[drv] != NULL) {
			if(!disk[drv]->process_state(state_fio, loading)) {
 				return false;
 			}
 		}
 	}
	*/
	state_fio->StateArray(&image_path[0][0], sizeof(image_path), 1);
	state_fio->StateArray(sector_size, sizeof(sector_size), 1);
	return SCSI_DEV::process_state(state_fio, loading);
}

// SASI hard disk drive

void SASI_HDD::start_command()
{
	switch(command[0]) {
	case SCSI_CMD_REQ_SENSE:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI_HDD:ID=%d] Command: Request Sense\n"), scsi_id);
		#endif
		// start position
		position = (command[1] & 0x1f) * 0x10000 + command[2] * 0x100 + command[3];
		position *= physical_block_size();
		// transfer length
		remain = 4;
		// create sense data table
		buffer->clear();
		buffer->write(get_sense_code());
		buffer->write(((max_logical_block_addr() >> 16) & 0x1f) | (get_logical_unit_number() << 5));
		buffer->write(((max_logical_block_addr() >>  8) & 0xff));
		buffer->write(((max_logical_block_addr() >>  0) & 0xff));
		// change to data in phase
		set_dat(buffer->read());
		set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
		set_sense_code(SCSI_SENSE_NOSENSE);
		return;
		
	case 0xc2:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[SASI_HDD:ID=%d] Command: SASI Command 0xC2\n"), scsi_id);
		#endif
		// transfer length
		remain = 10; // DTCŒn (ƒgƒ‰ƒ“ƒWƒXƒ^‹ZpSPECIAL No.27, P.88)
		// clear data buffer
		buffer->clear();
		// change to data in phase
		set_phase_delay(SCSI_PHASE_DATA_OUT, 1.0);
		return;
	}
	// start standard command
	SCSI_HDD::start_command();
}
