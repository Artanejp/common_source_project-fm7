/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.16-

	[ d88 handler ]
*/

#include "harddisk.h"
#include "../fileio.h"

void HARDDISK::open(const _TCHAR* file_path)
{
	FILEIO *fio = new FILEIO();
	pair_t tmp;
	
	if(buffer != NULL) {
		close();
	}
	buffer = NULL;
	
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		my_tcscpy_s(image_path, _MAX_PATH, file_path);
		
		if((buffer_size = fio->FileLength()) > 0) {
			buffer = (uint8_t *)malloc(buffer_size);
			fio->Fread(buffer, buffer_size, 1);
			
			// from NP2 sxsihdd.c
			if(check_file_extension(file_path, _T(".thd"))) {
				// T98
				buffer = (uint8_t *)malloc(buffer_size);
				fio->Fread(buffer, buffer_size, 1);
				
				header_size = 256;
				tmp.read_2bytes_le_from(buffer + 0);
				cylinders = tmp.sd;
				surfaces = 8;
				sectors = 33;
				sector_size = 256;

			} else if(check_file_extension(file_path, _T(".nhd"))) {
				// T98Next
				buffer = (uint8_t *)malloc(buffer_size);
				fio->Fread(buffer, buffer_size, 1);
				
/*
				typedef struct nhd_header_s {
					char sig[16];
					char comment[256];
					int32_t header_size;	// +272
					int32_t cylinders;	// +276
					int16_t surfaces;	// +280
					int16_t sectors;	// +282
					int16_t sector_size;	// +284
					uint8_t reserved[0xe2];
				} nhd_header_t;
*/
				tmp.read_4bytes_le_from(buffer + 272);
				header_size = tmp.sd;
				tmp.read_4bytes_le_from(buffer + 276);
				cylinders = tmp.sd;
				tmp.read_2bytes_le_from(buffer + 280);
				surfaces = tmp.sd;
				tmp.read_2bytes_le_from(buffer + 282);
				sectors = tmp.sd;
				tmp.read_2bytes_le_from(buffer + 284);
				sector_size = tmp.sd;
			} else if(check_file_extension(file_path, _T(".hdi"))) {
				// ANEX86
				buffer = (uint8_t *)malloc(buffer_size);
				fio->Fread(buffer, buffer_size, 1);
				
/*
				typedef struct hdi_header_s {
					int32_t dummy;		// + 0
					int32_t hdd_type;	// + 4
					int32_t header_size;	// + 8
					int32_t hdd_size;	// +12
					int32_t sector_size;	// +16
					int32_t sectors;	// +20
					int32_t surfaces;	// +24
					int32_t cylinders;	// +28
				} hdi_header_t;
*/
				tmp.read_4bytes_le_from(buffer + 8);
				header_size = tmp.sd;
				tmp.read_4bytes_le_from(buffer + 16);
				sector_size = tmp.sd;
				tmp.read_4bytes_le_from(buffer + 20);
				sectors = tmp.sd;
				tmp.read_4bytes_le_from(buffer + 24);
				surfaces = tmp.sd;
				tmp.read_4bytes_le_from(buffer + 28);
				cylinders = tmp.sd;
			} else {
				// solid
				switch(buffer_size) {
				case  5 * 1024 * 1024: buffer_size = 33 * 4 * 153 * 256; break;		// 5MB
				case 10 * 1024 * 1024: buffer_size = 33 * 4 * 310 * 256; break;		// 10MB
				case 15 * 1024 * 1024: buffer_size = 33 * 6 * 310 * 256; break;		// 15MB
				case 20 * 1024 * 1024: buffer_size = 33 * 8 * 310 * 256; break;		// 20MB
//				case 20 * 1024 * 1024: buffer_size = 33 * 4 * 615 * 256; break;		// 20MB (not used!)
				case 30 * 1024 * 1024: buffer_size = 33 * 6 * 615 * 256; break;		// 30MB
				case 40 * 1024 * 1024: buffer_size = 33 * 8 * 615 * 256; break;		// 40MB
				}
				buffer = (uint8_t *)malloc(buffer_size);
				fio->Fread(buffer, buffer_size, 1);
				
				header_size = 0;
				cylinders = (int)(buffer_size / (8 * 33 * 256));
				surfaces = 8;
				sectors = 33;
				sector_size = 256;
				
				typedef struct {
					int sectors;
					int surfaces;
					int cylinders;
				} format_t;
				static const format_t formats[] = {
					{33, 4, 153},		// 5MB
					{33, 4, 310},		// 10MB
					{33, 6, 310},		// 15MB
					{33, 8, 310},		// 20MB
					{33, 4, 615},		// 20MB (not used!)
					{33, 6, 615},		// 30MB
					{33, 8, 615},		// 40MB
				};
				for(int i = 0; i < array_length(formats); i++) {
					if(buffer_size == (formats[i].sectors * formats[i].surfaces * formats[i].cylinders * 256)) {
						cylinders = formats[i].cylinders;
						surfaces = formats[i].surfaces;
						sectors = formats[i].sectors;
						break;
					}
				}
			}
		}
		fio->Fclose();
	}
	delete fio;
}

void HARDDISK::close()
{
	// write disk image
	if(buffer != NULL) {
		FILEIO* fio = new FILEIO();
		
		if(fio->Fopen(image_path, FILEIO_WRITE_BINARY)) {
			fio->Fwrite(buffer, buffer_size, 1);
			fio->Fclose();
		}
		free(buffer);
		buffer = NULL;
	}
}

bool HARDDISK::read_buffer(int position, int length, uint8_t *buf)
{
	if(header_size + position + length <= buffer_size) {
		memcpy(buf, buffer + header_size + position + length, length);
		return true;
	}
	return false;
}

bool HARDDISK::write_buffer(int position, int length, uint8_t *buf)
{
	if(header_size + position + length <= buffer_size) {
		memcpy(buffer + header_size + position + length, buf, length);
		return true;
	}
	return false;
}

#define STATE_VERSION	1

void HARDDISK::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	
	state_fio->Fwrite(image_path, sizeof(image_path), 1);
	if(buffer) {
		state_fio->FputInt32(buffer_size);
		state_fio->Fwrite(buffer, buffer_size, 1);
	} else {
		state_fio->FputInt32(0);
	}
	state_fio->FputInt32(header_size);
	state_fio->FputInt32(cylinders);
	state_fio->FputInt32(surfaces);
	state_fio->FputInt32(sectors);
	state_fio->FputInt32(sector_size);
}

bool HARDDISK::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	state_fio->Fread(image_path, sizeof(image_path), 1);
	buffer_size = state_fio->FgetInt32();
	if(buffer) {
		free(buffer);
		buffer = NULL;
	}
	if(buffer_size) {
		buffer = (uint8_t *)malloc(buffer_size);
		state_fio->Fread(buffer, buffer_size, 1);
	}
	header_size = state_fio->FgetInt32();
	cylinders = state_fio->FgetInt32();
	surfaces = state_fio->FgetInt32();
	sectors = state_fio->FgetInt32();
	sector_size = state_fio->FgetInt32();
	return true;
}

