/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.16-

	[ d88 handler ]
*/

#include "harddisk.h"
#include "../fileio.h"

void HARDDISK::open(const _TCHAR* file_path, int default_sector_size)
{
	uint8_t header[512];
	pair32_t tmp;
	
	close();
	
	if(FILEIO::IsFileExisting(file_path)) {
		fio = new FILEIO();
		
		if(fio->Fopen(file_path, FILEIO_READ_WRITE_BINARY)) {
			// from NP2 sxsihdd.c
			const char sig_vhd[8] = "VHD1.00";
			const char sig_nhd[15] = "T98HDDIMAGE.R0";
			
			fio->Fread(header, 256, 1);
			
			if(check_file_extension(file_path, _T(".thd"))) {
				// T98
/*
				typedef struct thd_header_s {
					int16_t cylinders;
				} thd_header_s;
*/
				header_size = 256;
				tmp.read_2bytes_le_from(header + 0);
				int cylinders = tmp.sd;
				surfaces = 8;
				sectors = 33;
				sector_size = 256;
				sector_num = cylinders * surfaces * sectors;
			} else if(check_file_extension(file_path, _T(".nhd")) && memcmp(header, sig_nhd, 15) == 0) {
				// T98-Next
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
				int cylinders = tmp.sd;
				tmp.read_2bytes_le_from(header + 280);
				surfaces = tmp.sd;
				tmp.read_2bytes_le_from(header + 282);
				sectors = tmp.sd;
				tmp.read_2bytes_le_from(header + 284);
				sector_size = tmp.sd;
				sector_num = cylinders * surfaces * sectors;
			} else if(check_file_extension(file_path, _T(".hdi"))) {
				// ANEX86
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
				int cylinders = tmp.sd;
				sector_num = cylinders * surfaces * sectors;
			} else if(check_file_extension(file_path, _T(".hdd")) && memcmp(header, sig_vhd, 5) == 0) {
				// Virtual98
/*
				typedef struct {
					char    sig[3];		// +  0
					char    ver[4];		// +  3
					char    delimita;	// +  7
					char    comment[128];	// +  8
					uint8_t pad1[4];	// +136
					int16_t mbsize;		// +140
					int16_t sectorsize;	// +142
					uint8_t sectors;	// +144
					uint8_t surfaces;	// +145
					int16_t cylinders;	// +146
					int32_t totals;		// +148
					uint8_t pad2[0x44];	// +152
				} virtual98_header_t;
*/
				header_size = 288;
				tmp.read_2bytes_le_from(header + 142);
				sector_size = tmp.sd;
				sectors = header[144];
				surfaces = header[145];
//				tmp.read_2bytes_le_from(header + 146);
//				int cylinders = tmp.sd;
				tmp.read_4bytes_le_from(header + 148);
				sector_num = tmp.sd;
//				sector_num = cylinders * surfaces * sectors;
			} else {
				// solid
				header_size = 0;
				// sectors = 33/17, surfaces = 4, cylinders = 153, sector_size = 256/512	// 5MB
				// sectors = 33/17, surfaces = 4, cylinders = 310, sector_size = 256/512	// 10MB
				// sectors = 33/17, surfaces = 4, cylinders = 615, sector_size = 256/512	// 20MB
				// sectors = 33/17, surfaces = 8, cylinders = 615, sector_size = 256/512	// 40MB
#if 0
				surfaces = (fio->FileLength() <= 17 * 4 * 615 * 512) ? 4 : 8;
#else
				surfaces = ((int)(fio->FileLength() / 1024 / 1024) < 24) ? 4 : 8;
#endif
				sectors = (default_sector_size == 1024) ? 8 : (default_sector_size == 512) ? 17 : 33;
				sector_size = default_sector_size;
				sector_num = fio->FileLength() / sector_size;
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

