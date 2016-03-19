/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2016.03.01-

	[ SCSI hard disk drive ]
*/

#include "scsi_hdd.h"
#include "../fifo.h"

void SCSI_HDD::read_buffer(int length)
{
	// make sure drive size is not zero
	initialize_max_logical_block_addr();
	
	// read blocks
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("SCSI%d.DAT"), scsi_id), FILEIO_READ_BINARY)) {
		// FIXME: consider the case that disk size is bigger than MAX_LONG
		fio->Fseek((long)position, FILEIO_SEEK_SET);
		while(length > 0) {
			uint8_t tmp_buffer[SCSI_BUFFER_SIZE];
			int tmp_length = min(length, (int)sizeof(tmp_buffer));
			
			fio->Fread(tmp_buffer, tmp_length, 1);
			for(int i = 0; i < tmp_length; i++) {
				buffer->write(tmp_buffer[i]);
			}
			length -= tmp_length;
			position += tmp_length;
		}
		fio->Fclose();
	}
	delete fio;
}

void SCSI_HDD::write_buffer(int length)
{
	// make sure drive size is not zero
	initialize_max_logical_block_addr();
	
	// write blocks
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("SCSI%d.DAT"), scsi_id), FILEIO_READ_WRITE_BINARY)) {
		// FIXME: consider the case that disk size is bigger than MAX_LONG
		fio->Fseek((long)position, FILEIO_SEEK_SET);
		while(length > 0) {
			uint8_t tmp_buffer[SCSI_BUFFER_SIZE];
			int tmp_length = min(length, (int)sizeof(tmp_buffer));
			
			for(int i = 0; i < tmp_length; i++) {
				tmp_buffer[i] = buffer->read();
			}
			fio->Fwrite(tmp_buffer, tmp_length, 1);
			length -= tmp_length;
			position += tmp_length;
		}
		fio->Fclose();
	}
	delete fio;
}

void SCSI_HDD::initialize_max_logical_block_addr()
{
	if(max_logical_block_addr == 0) {
		FILEIO* fio = new FILEIO();
		uint32_t file_size = 0;
		
		if(FILEIO::IsFileExisting(create_local_path(_T("SCSI%d.DAT"), scsi_id))) {
			// if scsi hard disk drive image exists, get the file size
			if(fio->Fopen(create_local_path(_T("SCSI%d.DAT"), scsi_id), FILEIO_READ_WRITE_BINARY)) {
				if((file_size = fio->FileLength()) == 0) {
					uint32_t remain = (file_size = default_drive_size);
					void *tmp = calloc(1, SCSI_BUFFER_SIZE);
					while(remain > 0) {
						uint32_t length = min(remain, (uint32_t)SCSI_BUFFER_SIZE);
						fio->Fwrite(tmp, length, 1);
						remain -= length;
					}
					free(tmp);
				}
				fio->Fclose();
			}
		} else {
			// if scsi hard disk drive image does not exist, create the image with default size
			if(fio->Fopen(create_local_path(_T("SCSI%d.DAT"), scsi_id), FILEIO_WRITE_BINARY)) {
				uint32_t remain = (file_size = default_drive_size);
				void *tmp = calloc(1, SCSI_BUFFER_SIZE);
				while(remain > 0) {
					uint32_t length = min(remain, (uint32_t)SCSI_BUFFER_SIZE);
					fio->Fwrite(tmp, length, 1);
					remain -= length;
				}
				free(tmp);
				fio->Fclose();
			}
		}
		if(file_size != 0) {
			max_logical_block_addr = (file_size / logical_block_size) - 1;
		}
		delete fio;
	}
}

#define STATE_VERSION	1

void SCSI_HDD::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	SCSI_DEV::save_state(state_fio);
}

bool SCSI_HDD::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	return SCSI_DEV::load_state(state_fio);
}

