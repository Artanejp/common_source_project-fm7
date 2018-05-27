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
	open(file_path, 256);
}

void HARDDISK::open(const _TCHAR* file_path, int default_sector_size)
{
	uint8_t header[512];
	pair_t tmp;
	
	close();
	
	if(FILEIO::IsFileExisting(file_path)) {
		fio = new FILEIO();
		
		if(fio->Fopen(file_path, FILEIO_READ_WRITE_BINARY)) {
			// from NP2 sxsihdd.c
			if(check_file_extension(file_path, _T(".thd"))) {
				// T98
				fio->Fread(header, 256, 1);
/*
				typedef struct thd_header_s {
					int16_t cylinders;
				} thd_header_s;
*/
				header_size = 256;
				tmp.read_2bytes_le_from(header + 0);
				cylinders = tmp.sd;
				surfaces = 8;
				sectors = 33;
				sector_size = 256;

			} else if(check_file_extension(file_path, _T(".nhd"))) {
				// T98Next
				fio->Fread(header, 256, 1);
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
				tmp.read_4bytes_le_from(header + 272);
				header_size = tmp.sd;
				tmp.read_4bytes_le_from(header + 276);
				cylinders = tmp.sd;
				tmp.read_2bytes_le_from(header + 280);
				surfaces = tmp.sd;
				tmp.read_2bytes_le_from(header + 282);
				sectors = tmp.sd;
				tmp.read_2bytes_le_from(header + 284);
				sector_size = tmp.sd;
			} else if(check_file_extension(file_path, _T(".hdi"))) {
				// ANEX86
				fio->Fread(header, 32, 1);
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
				tmp.read_4bytes_le_from(header + 8);
				header_size = tmp.sd;
				tmp.read_4bytes_le_from(header + 16);
				sector_size = tmp.sd;
				tmp.read_4bytes_le_from(header + 20);
				sectors = tmp.sd;
				tmp.read_4bytes_le_from(header + 24);
				surfaces = tmp.sd;
				tmp.read_4bytes_le_from(header + 28);
				cylinders = tmp.sd;
			} else {
				// solid
				typedef struct {
					int sectors, surfaces, cylinders, sector_size;
				} hd_format_t;
				long length = fio->FileLength();
				
				header_size = 0;
				cylinders = (int)(length / (8 * 33 * default_sector_size));
				surfaces = 8;
				sectors = 33;
				sector_size = default_sector_size;
				
				if(default_sector_size == 256) {
					static const hd_format_t hd_formats[] = {
						{33, 4, 153, 256},	// 5MB
						{33, 4, 310, 256},	// 10MB
						{33, 6, 310, 256},	// 15MB
						{33, 8, 310, 256},	// 20MB
						{33, 4, 615, 256},	// 20MB (not used!)
						{33, 6, 615, 256},	// 30MB
						{33, 8, 615, 256},	// 40MB
					};
					for(int i = 0; i < array_length(hd_formats); i++) {
						const hd_format_t *p = &hd_formats[i];
						if(length == p->sectors * p->surfaces * p->cylinders * p->sector_size) {
							cylinders = p->cylinders;
							surfaces = p->surfaces;
							sectors = p->sectors;
							sector_size = p->sector_size;
							break;
						}
					}
				} else if(default_sector_size == 512) {
					
				}
			}
		}
	}
}

void HARDDISK::close()
{
	// write disk image
	if(fio != NULL) {
		if(fio->IsOpened()) {
			fio->Fclose();
		}
		delete fio;
		fio = NULL;
	}
}

bool HARDDISK::mounted()
{
	return (fio != NULL && fio->IsOpened());
}

bool HARDDISK::accessed()
{
	bool value = access;
	access = false;
	return value;
}

bool HARDDISK::read_buffer(long position, int length, uint8_t *buffer)
{
	if(mounted()) {
		if(fio->Fseek(header_size + position, FILEIO_SEEK_SET) == 0) {
			access = true;
			return (fio->Fread(buffer, length, 1) == 1);
		}
	}
	return false;
}

bool HARDDISK::write_buffer(long position, int length, uint8_t *buffer)
{
	if(mounted()) {
		if(fio->Fseek(header_size + position, FILEIO_SEEK_SET) == 0) {
			access = true;
			return (fio->Fwrite(buffer, length, 1) == 1);
		}
	}
	return false;
}

/*
#define STATE_VERSION	1

void HARDDISK::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	
	state_fio->Fwrite(image_path, sizeof(image_path), 1);
	if(mounted()) {
		uint8_t *buffer = (uint8_t *)malloc(0x100000); // 1MB
		long remain = fio->FileLength();
		state_fio->FputInt32(remain);
		fio->Fseek(0, FILEIO_SEEK_SET);
		while(remain > 0) {
			long size = min(remain, 0x100000);
			fio->Fread(buffer, size, 1);
			state_fio->Fwrite(buffer, size, 1);
			remain -= size;
		}
		free(buffer);
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
	close();
	
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	state_fio->Fread(image_path, sizeof(image_path), 1);
	long remain = state_fio->FgetInt32();
	if(remain > 0) {
		fio = new FILEIO();
		if(!fio->Fopen(image_path, FILEIO_READ_WRITE_NEW_BINARY)) {
			if(check_file_extension(image_path, _T(".thd"))) {
				create_local_path(image_path, _MAX_PATH, _T("temporary_saved_hard_disk_#%d.thd"), drive_num);
			} else if(check_file_extension(image_path, _T(".nhd"))) {
				create_local_path(image_path, _MAX_PATH, _T("temporary_saved_hard_disk_#%d.nhd"), drive_num);
			} else if(check_file_extension(image_path, _T(".hdi"))) {
				create_local_path(image_path, _MAX_PATH, _T("temporary_saved_hard_disk_#%d.hdi"), drive_num);
			} else {
				create_local_path(image_path, _MAX_PATH, _T("temporary_saved_hard_disk_#%d.dat"), drive_num);
			}
			fio->Fopen(image_path, FILEIO_READ_WRITE_NEW_BINARY);
		}
		if(fio->IsOpened()) {
			uint8_t *buffer = (uint8_t *)malloc(0x100000); // 1MB
			fio->Fseek(0, FILEIO_SEEK_SET);
			while(remain > 0) {
				long size = min(remain, 0x100000);
				state_fio->Fread(buffer, size, 1);
				fio->Fwrite(buffer, size, 1);
				remain -= size;
			}
			free(buffer);
		} else {
			state_fio->Fseek(remain, FILEIO_SEEK_CUR);
		}
	}
	header_size = state_fio->FgetInt32();
	cylinders = state_fio->FgetInt32();
	surfaces = state_fio->FgetInt32();
	sectors = state_fio->FgetInt32();
	sector_size = state_fio->FgetInt32();
	return true;
}
*/
