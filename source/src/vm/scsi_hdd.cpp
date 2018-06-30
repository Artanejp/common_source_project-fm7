/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2016.03.01-

	[ SCSI hard disk drive ]
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
	return 512;
}

uint32_t SCSI_HDD::logical_block_size()
{
	HARDDISK *unit = disk[get_logical_unit_number()];
	
	if(unit != NULL && unit->mounted()) {
		return unit->sector_size;
	}
	return 512;
}

uint32_t SCSI_HDD::max_logical_block_addr()
{
	HARDDISK *unit = disk[get_logical_unit_number()];
	
	if(unit != NULL && unit->mounted()) {
		return unit->cylinders * unit->surfaces * unit->sectors;
	}
	return 0;
}

void SCSI_HDD::read_buffer(int length)
{
	HARDDISK *unit = disk[get_logical_unit_number()];
	
	if(unit != NULL && unit->mounted()) {
		while(length > 0) {
			uint8_t tmp_buffer[SCSI_BUFFER_SIZE];
			int tmp_length = min(length, (int)sizeof(tmp_buffer));
			
			unit->read_buffer((long)position, tmp_length, tmp_buffer);
			for(int i = 0; i < tmp_length; i++) {
				buffer->write(tmp_buffer[i]);
			}
			length -= tmp_length;
			position += tmp_length;
		}
	}
}

void SCSI_HDD::write_buffer(int length)
{
	HARDDISK *unit = disk[get_logical_unit_number()];
	
	if(unit != NULL && unit->mounted()) {
		while(length > 0) {
			uint8_t tmp_buffer[SCSI_BUFFER_SIZE];
			int tmp_length = min(length, (int)sizeof(tmp_buffer));
			
			for(int i = 0; i < tmp_length; i++) {
				tmp_buffer[i] = buffer->read();
			}
			unit->write_buffer((long)position, tmp_length, tmp_buffer);
			length -= tmp_length;
			position += tmp_length;
		}
	}
}

#define STATE_VERSION	1

#include "../statesub.h"

void SCSI_HDD::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
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

