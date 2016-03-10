/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2016.03.01-

	[ SCSI hard disk drive ]
*/

#include "scsi_hdd.h"
#include "../fifo.h"

void SCSI_HDD::initialize_max_logical_block_addr()
{
	if(max_logical_block_addr == 0) {
		FILEIO* fio = new FILEIO();
		uint32_t file_size = 0;
		
		if(default_drive_size == 0) {
			default_drive_size = 0x2800000; // 40MB
		}
		if(FILEIO::IsFileExisting(create_local_path(_T("SCSI%d.DAT"), scsi_id))) {
			if(fio->Fopen(create_local_path(_T("SCSI%d.DAT"), scsi_id), FILEIO_READ_WRITE_BINARY)) {
				if((file_size = fio->FileLength()) == 0) {
					uint32_t remain = (file_size = default_drive_size);
					void *tmp = calloc(1, SCSI_BUFFER_SIZE);
					while(remain > 0) {
						uint32_t length = min(remain, SCSI_BUFFER_SIZE);
						fio->Fwrite(tmp, length, 1);
						remain -= length;
					}
					free(tmp);
				}
				fio->Fclose();
			}
		} else {
			if(fio->Fopen(create_local_path(_T("SCSI%d.DAT"), scsi_id), FILEIO_WRITE_BINARY)) {
				uint32_t remain = (file_size = default_drive_size);
				void *tmp = calloc(1, SCSI_BUFFER_SIZE);
				while(remain > 0) {
					uint32_t length = min(remain, SCSI_BUFFER_SIZE);
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

void SCSI_HDD::start_command()
{
	switch(command[0]) {
	case SCSI_CMD_INQUIRY:
		#ifdef _SCSI_DEBUG_LOG
			emu->out_debug_log(_T("[SCSI_HDD:ID=%d] Command: Inquiry\n"), scsi_id);
		#endif
		// start position
		position = ((command[1] & 0x1f) * 0x10000 + command[2] * 0x100 + command[3]) * logical_block_size;
		// transfer length
		remain = 32;
		// create sense data table
		buffer->clear();
		buffer->write(0x00);
		buffer->write(0x00);
		buffer->write(0x02); // ANSI SCSI2
		buffer->write(0x01); // ANSI-CCS
		buffer->write(0x1c);
		buffer->write(0x00);
		buffer->write(0x00);
		buffer->write(0x18);
		for(int i = 0; i < (int)strlen(vendor_id) && i < 8; i++) {
			buffer->write(vendor_id[i]);
		}
		for(int i = strlen(vendor_id); i < 8; i++) {
			buffer->write(0x20);
		}
		for(int i = 0; i < (int)strlen(product_id) && i < 16; i++) {
			buffer->write(vendor_id[i]);
		}
		for(int i = strlen(product_id); i < 16; i++) {
			buffer->write(0x20);
		}
		// set first data
		set_dat(buffer->read());
		// change to data in phase
		set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
		break;
		
	case SCSI_CMD_RD_CAPAC:
		#ifdef _SCSI_DEBUG_LOG
			emu->out_debug_log(_T("[SCSI_HDD:ID=%d] Command: Read Capacity\n"), scsi_id);
		#endif
		// initialize max logical block address
		initialize_max_logical_block_addr();
		// start position
		position = (command[2] * 0x1000000 + command[3] * 0x10000 + command[4] * 0x100 + command[5]) * logical_block_size;
		// transfer length
		remain = 8;
		// create capacity data table
		buffer->clear();
		buffer->write((max_logical_block_addr >> 24) & 0xff);
		buffer->write((max_logical_block_addr >> 16) & 0xff);
		buffer->write((max_logical_block_addr >>  8) & 0xff);
		buffer->write((max_logical_block_addr >>  0) & 0xff);
		buffer->write((    logical_block_size >> 24) & 0xff);
		buffer->write((    logical_block_size >> 16) & 0xff);
		buffer->write((    logical_block_size >>  8) & 0xff);
		buffer->write((    logical_block_size >>  0) & 0xff);
		// set first data
		set_dat(buffer->read());
		// change to data in phase
		set_phase_delay(SCSI_PHASE_DATA_IN, 10.0);
		break;
		
	default:
		SCSI_DEV::start_command();
		break;
	}
}

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
			int tmp_length = min(length, sizeof(tmp_buffer));
			
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
			int tmp_length = min(length, sizeof(tmp_buffer));
			
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

