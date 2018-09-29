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

#include "../statesub.h"

void SCSI_HDD::decl_state()
{
	enter_decl_state(STATE_VERSION);

	for(int i = 0; i < 8; i++) {
		DECL_STATE_ENTRY_STRING_MEMBER(&(image_path[i][0]), MAX_PATH, i);
	}
	DECL_STATE_ENTRY_1D_ARRAY(sector_size, sizeof(sector_size) / sizeof(int));  
	leave_decl_state();

	SCSI_DEV::decl_state();
}

void SCSI_HDD::save_state(FILEIO* state_fio)
{
	uint32_t crc_value = 0xffffffff;
	if(state_entry != NULL) {
		state_entry->save_state(state_fio, &crc_value);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
/*
	for(int i = 0; i < 8; i++) {
		if(disk[i] != NULL) {
			disk[i]->save_state(state_fio);
		}
	}
*/
//	SCSI_DEV::save_state(state_fio);
}

bool SCSI_HDD::load_state(FILEIO* state_fio)
{
	uint32_t crc_value = 0xffffffff;
	bool stat = false;
	bool mb = false;
	if(state_entry != NULL) {
	   mb = state_entry->load_state(state_fio, &crc_value);
	}
	if(!mb) return false;
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
/*
	for(int i = 0; i < 8; i++) {
		if(disk[i] != NULL) {
			if(!disk[i]->load_state(state_fio)) {
				return false;
			}
		}
	}
*/
//	return SCSI_DEV::load_state(state_fio);
	return true;
}

