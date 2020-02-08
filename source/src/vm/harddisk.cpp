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
#if 0
	printf(_T("OPEN IMAGE: %s DEFAULT SECTOR SIZE=%d\n"), file_path, default_sector_size);
#endif
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
				cylinders = tmp.sd;
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
				cylinders = tmp.sd;
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
				cylinders = tmp.sd;
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
				sectors   = header[144];
				surfaces  = header[145];
				cylinders = header[146];
//				tmp.read_2bytes_le_from(header + 146);
//				int cylinders = tmp.sd;
				tmp.read_4bytes_le_from(header + 148);
				sector_num = tmp.sd;
//				sector_num = cylinders * surfaces * sectors;
			}  else {
				bool is_hx = false;
				for(int i = 0; i < 9; i++) {
					_TCHAR _tmps[6] = {0};
					my_stprintf_s(_tmps, 6, _T("h%d"), i);
					if(check_file_extension(file_path, _tmps)) {
						is_hx = true;
						break;
					}
				}
				if(is_hx) {
					// *.H0 .. *.H9 : Variable cylinders
					header_size = 0;
					surfaces = 8;
					sectors = 63;
					cylinders = (fio->FileLength() / default_sector_size) / (sectors * surfaces);
					sector_size = default_sector_size;
					sector_num = fio->FileLength() / sector_size;
					return;
				}						
				// solid
				header_size = 0;
				// sectors = 33, surfaces = 4, cylinders = 153, sector_size = 256	// 5MB
				// sectors = 33, surfaces = 8, cylinders = 310, sector_size = 256	// 10MB
				surfaces = (default_sector_size == 256 && fio->FileLength() <= 33 * 4 * 310 * 256) ? 4 : 8;
				cylinders = (surfaces == 4) ? 153 : 310;
				sectors = 33;
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

long HARDDISK::get_cur_position()
{
	if(mounted()) {
		long _p = fio->Ftell();
		_p = _p - (long)header_size;
		if(_p >= 0) {
			_p = _p / (long)sector_size;
			if(_p > (long)sector_num) return -1;
			return _p;
		} else {
			return -1;
		}
	}
	return -1;
}

int HARDDISK::get_sector_size()
{
	return sector_size;
}

int HARDDISK::get_cylinders()
{
	return cylinders;
}

int HARDDISK::get_headers()
{
	return surfaces;
}

int HARDDISK::get_drive_num()
{
	return drive_num;
}

int HARDDISK::get_sector_num()
{
	return sector_num;
}

int HARDDISK::get_sectors_per_cylinder()
{
//	if((surfaces <= 0) || (cylinders <= 0)) return -1;
//	int _n = (sector_num / cylinders) / surfaces;
//	return _n;
	return sectors;
}
