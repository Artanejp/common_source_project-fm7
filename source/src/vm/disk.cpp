/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.16-

	[ d88 handler ]
*/

#include "disk.h"
//#include "../fileio.h"
#if defined(_USE_AGAR) || defined(_USE_SDL) || defined(_USE_QT)
#include "agar_logger.h"
#endif

// crc table
static const uint16 crc_table[256] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823, 0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

// teledisk decoder table
static const uint8 d_code[256] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
	0x0c, 0x0c, 0x0c, 0x0c, 0x0d, 0x0d, 0x0d, 0x0d, 0x0e, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f,
	0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11, 0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13,
	0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15, 0x16, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x17,
	0x18, 0x18, 0x19, 0x19, 0x1a, 0x1a, 0x1b, 0x1b, 0x1c, 0x1c, 0x1d, 0x1d, 0x1e, 0x1e, 0x1f, 0x1f,
	0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23, 0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x27,
	0x28, 0x28, 0x29, 0x29, 0x2a, 0x2a, 0x2b, 0x2b, 0x2c, 0x2c, 0x2d, 0x2d, 0x2e, 0x2e, 0x2f, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};
static const uint8 d_len[256] = {
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};
static const int secsize[8] = {
	128, 256, 512, 1024, 2048, 4096, 8192, 16384
};

static uint8 tmp_buffer[DISK_BUFFER_SIZE];

typedef struct {
	int type;
	int ncyl, nside, nsec, size;
} fd_format_t;

static const fd_format_t fd_formats[] = {
	{ MEDIA_TYPE_2D,  40, 1, 16,  256 },	// 1D   160KB
	{ MEDIA_TYPE_2D , 40, 2, 16,  256 },	// 2D   320KB
#if defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200) || defined(_MZ2500)
	{ MEDIA_TYPE_2DD, 80, 2, 16,  256 },	// 2DD  640KB (MZ-2500)
#else
	{ MEDIA_TYPE_2DD, 80, 2,  8,  512 },	// 2DD  640KB
#endif
	{ MEDIA_TYPE_2DD, 80, 2,  9,  512 },	// 2DD  720KB
	{ MEDIA_TYPE_2HD, 80, 2, 15,  512 },	// 2HC 1.20MB
	{ MEDIA_TYPE_2HD, 77, 2,  8, 1024 },	// 2HD 1.25MB
	{ MEDIA_TYPE_144, 80, 2, 18,  512 },	// 2HD 1.44MB
	{ MEDIA_TYPE_144, 80, 2, 36,  512 },	// 2ED 2.88MB
	{ -1, 0, 0, 0, 0 },
};

#define IS_VALID_TRACK(offset) ((offset) >= 0x20 && (offset) < sizeof(buffer))

void DISK::open(_TCHAR path[], int bank)
{
	// check current disk image
	if(inserted) {
		if(_tcsicmp(orig_path, path) == 0 && file_bank == bank) {
			return;
		}
		close();
	}
	memset(buffer, 0, sizeof(buffer));
	media_type = MEDIA_TYPE_UNK;
	is_standard_image = is_fdi_image = false;
	trim_required = false;
	
	// open disk image
	fi = new FILEIO();
	if(fi->Fopen(path, FILEIO_READ_BINARY)) {
		bool converted = false;
		
		_tcscpy_s(orig_path, _MAX_PATH, path);
		_tcscpy_s(dest_path, _MAX_PATH, path);
		_stprintf_s(temp_path, _MAX_PATH, _T("%s.$$$"), path);
		temporary = false;
		
		// check if file protected
		write_protected = fi->IsProtected(path);
		
		// is this d88 format ?
		if(check_file_extension(path, _T(".d88")) || check_file_extension(path, _T(".d77"))) {
			uint32 offset = 0;
			for(int i = 0; i < bank; i++) {
				fi->Fseek(offset + 0x1c, SEEK_SET);
				offset += fi->FgetUint32_LE();
			}
			fi->Fseek(offset + 0x1c, FILEIO_SEEK_SET);
			file_size.d = fi->FgetUint32_LE();
			fi->Fseek(offset, FILEIO_SEEK_SET);
			fi->Fread(buffer, file_size.d, 1);
			file_bank = bank;
			inserted = changed = true;
//			trim_required = true;
	
			// fix sector number from big endian to little endian
			for(int trkside = 0; trkside < 164; trkside++) {
				pair offset;
				offset.read_4bytes_le_from(buffer + 0x20 + trkside * 4);
				
				if(!IS_VALID_TRACK(offset.d)) {
					break;
				}
				uint8* t = buffer + offset.d;
				pair sector_num, data_size;
				sector_num.read_2bytes_le_from(t + 4);
				bool is_be = (sector_num.b.l == 0 && sector_num.b.h >= 4);
				if(is_be) {
					sector_num.read_2bytes_be_from(t + 4);
					sector_num.write_2bytes_le_to(t + 4);
				}
				for(int i = 0; i < sector_num.sd; i++) {
					if(is_be) {
						sector_num.write_2bytes_le_to(t + 4);
					}
					data_size.read_2bytes_le_from(t + 14);
					t += data_size.sd + 0x10;
				}
			}
			goto file_loaded;
		}
		
		fi->Fseek(0, FILEIO_SEEK_END);
		file_size.d = fi->Ftell();
		fi->Fseek(0, FILEIO_SEEK_SET);
		file_bank = 0;
		
#if defined(_X1) || defined(_X1TWIN) || defined(_X1TURBO) || defined(_X1TURBOZ)
		// is this 2d format ?
		if(check_file_extension(path, _T(".2d"))) {
			if(standard_to_d88(MEDIA_TYPE_2D, 40, 2, 16, 256)) {
				inserted = changed = is_standard_image = true;
				goto file_loaded;
			}
			fi->Fseek(0, FILEIO_SEEK_SET);
		}
#endif
		
		// check image file format
		for(int i = 0;; i++) {
			const fd_format_t *p = &fd_formats[i];
			if(p->type == -1) {
				break;
			}
			int len = p->ncyl * p->nside * p->nsec * p->size;
			// 4096 bytes: FDI header ???
			if(file_size.d == len || (file_size.d == (len + 4096) && (len == 655360 || len == 1261568))) {
				if(file_size.d == len + 4096) {
					is_fdi_image = true;
					fi->Fread(fdi_header, 4096, 1);
				}
				if(standard_to_d88(p->type, p->ncyl, p->nside, p->nsec, p->size)) {
					inserted = changed = is_standard_image = true;
					goto file_loaded;
				}
			}
		}
		if(0 < file_size.d && file_size.d <= DISK_BUFFER_SIZE) {
			memset(buffer, 0, sizeof(buffer));
			fi->Fread(buffer, file_size.d, 1);
			
			// check d88 format (temporary)
			if(file_size.b.l == buffer[0x1c] && file_size.b.h == buffer[0x1d] && file_size.b.h2 == buffer[0x1e] && file_size.b.h3 == buffer[0x1f]) {
				inserted = changed = true;
				goto file_loaded;
			}
			_stprintf_s(dest_path, _MAX_PATH, _T("%s.D88"), path);
			
			// check file header
			try {
				if(memcmp(buffer, "TD", 2) == 0 || memcmp(buffer, "td", 2) == 0) {
					// teledisk image file
					inserted = changed = converted = teledisk_to_d88();
				} else if(memcmp(buffer, "IMD", 3) == 0) {
					// imagedisk image file
					inserted = changed = converted = imagedisk_to_d88();
				} else if(memcmp(buffer, "MV - CPC", 8) == 0) {
					// standard cpdread image file
					inserted = changed = converted = cpdread_to_d88(0);
				} else if(memcmp(buffer, "EXTENDED", 8) == 0) {
					// extended cpdread image file
					inserted = changed = converted = cpdread_to_d88(1);
				}
			} catch(...) {
				// failed to convert the disk image
			}
		}
file_loaded:
		if(fi->IsOpened()) {
			fi->Fclose();
		}
		if(temporary) {
			fi->Remove(temp_path);
		}
		if(inserted) {
#if 0
			if(converted) {
				// write image
				FILEIO* fio = new FILEIO();
				if(fio->Fopen(dest_path, FILEIO_WRITE_BINARY)) {
					fio->Fwrite(buffer, file_size.d, 1);
					fio->Fclose();
				}
				delete fio;
			}
#endif
			crc32 = getcrc32(buffer, file_size.d);
		}
		if(buffer[0x1a] != 0) {
			write_protected = true;
		}
		if(media_type == MEDIA_TYPE_UNK) {
			if((media_type = buffer[0x1b]) == MEDIA_TYPE_2HD) {
				for(int trkside = 0; trkside < 164; trkside++) {
					pair offset;
					offset.read_4bytes_le_from(buffer + 0x20 + trkside * 4);
					
					if(!IS_VALID_TRACK(offset.d)) {
						continue;
					}
					// track found
					uint8 *t = buffer + offset.d;
					pair sector_num, data_size;
					sector_num.read_2bytes_le_from(t + 4);
					data_size.read_2bytes_le_from(t + 14);
					
					if(sector_num.sd >= 18 && data_size.sd == 512) {
						media_type = MEDIA_TYPE_144;
					}
					break;
				}
			}
		}
		// FIXME: ugly patch for X1turbo ALPHA and Batten Tanuki
		is_special_disk = 0;
#if defined(_X1) || defined(_X1TWIN) || defined(_X1TURBO) || defined(_X1TURBOZ)
		if(media_type == MEDIA_TYPE_2D) {
			// check first sector
			pair offset;
			offset.read_4bytes_le_from(buffer + 0x20);
			if(IS_VALID_TRACK(offset.d)) {
				static const uint8 batten[] = {0xca, 0xde, 0xaf, 0xc3, 0xdd, 0x20, 0xc0, 0xc7, 0xb7};
				uint8 *t = buffer + offset.d;
				if(strncmp((char *)(t + 0x11), "turbo ALPHA", 11) == 0) {
					is_special_disk = SPECIAL_DISK_X1_ALPHA;
				} else if(memcmp((void *)(t + 0x11), batten, sizeof(batten)) == 0) {
					is_special_disk = SPECIAL_DISK_X1_BATTEN;
				}
			}
		}
#endif
	}
	delete fi;
}

void DISK::close()
{
	// write disk image
	if(inserted) {
		if(trim_required) {
			trim_buffer();
			trim_required = false;
		}
		if(!write_protected && file_size.d && getcrc32(buffer, file_size.d) != crc32) {
			// write image
			FILEIO* fio = new FILEIO();
			int pre_size = 0, post_size = 0;
			uint8 *pre_buffer = NULL, *post_buffer = NULL;
			
			// is this d88 format ?
			if(check_file_extension(dest_path, _T(".d88")) || check_file_extension(dest_path, _T(".d77"))) {
				if(fio->Fopen(dest_path, FILEIO_READ_BINARY)) {
					fio->Fseek(0, FILEIO_SEEK_END);
					uint32 total_size = fio->Ftell(), offset = 0;
					for(int i = 0; i < file_bank; i++) {
						fio->Fseek(offset + 0x1c, SEEK_SET);
						offset += fio->FgetUint32_LE();
					}
					if((pre_size = offset) > 0) {
						pre_buffer = (uint8 *)malloc(pre_size);
						fio->Fseek(0, FILEIO_SEEK_SET);
						fio->Fread(pre_buffer, pre_size, 1);
					}
					fio->Fseek(offset + 0x1c, SEEK_SET);
					offset += fio->FgetUint32_LE();
					if((post_size = total_size - offset) > 0) {
						post_buffer = (uint8 *)malloc(post_size);
						fio->Fseek(offset, FILEIO_SEEK_SET);
						fio->Fread(post_buffer, post_size, 1);
					}
					fio->Fclose();
				}
			}
			if(!fio->Fopen(dest_path, FILEIO_WRITE_BINARY)) {
				_TCHAR tmp_path[_MAX_PATH];
				_stprintf_s(tmp_path, _MAX_PATH, _T("temporary_saved_floppy_disk_#%d.d88"), drive_num);
				fio->Fopen(emu->bios_path(tmp_path), FILEIO_WRITE_BINARY);
			}
			if(fio->IsOpened()) {
				if(pre_buffer) {
					fio->Fwrite(pre_buffer, pre_size, 1);
				}
				if(is_standard_image) {
					if(is_fdi_image) {
						fio->Fwrite(fdi_header, 4096, 1);
					}
					for(int trkside = 0; trkside < 164; trkside++) {
						pair offset;
						offset.read_4bytes_le_from(buffer + 0x20 + trkside * 4);
						
						if(!IS_VALID_TRACK(offset.d)) {
							break;
						}
						uint8* t = buffer + offset.d;
						pair sector_num, data_size;
						sector_num.read_2bytes_le_from(t + 4);
						
						for(int i = 0; i < sector_num.sd; i++) {
							data_size.read_2bytes_le_from(t + 14);
							fio->Fwrite(t + 0x10, data_size.sd, 1);
							t += data_size.sd + 0x10;
						}
					}
				} else {
					fio->Fwrite(buffer, file_size.d, 1);
				}
				if(post_buffer) {
					fio->Fwrite(post_buffer, post_size, 1);
				}
				fio->Fclose();
			}
			if(pre_buffer) {
				free(pre_buffer);
			}
			if(post_buffer) {
				free(post_buffer);
			}
			delete fio;
		}
		ejected = true;
	}
	inserted = write_protected = false;
	file_size.d = 0;
	sector_size.sd = sector_num.sd = 0;
	sector = NULL;
}

bool DISK::get_track(int trk, int side)
{
	sector_size.sd = sector_num.sd = 0;
	invalid_format = false;
	no_skew = true;
	
	// disk not inserted or invalid media type
	if(!(inserted && check_media_type())) {
		return false;
	}
	
	// search track
	int trkside = trk * 2 + (side & 1);
	if(!(0 <= trkside && trkside < 164)) {
		return false;
	}
	pair offset;
	offset.read_4bytes_le_from(buffer + 0x20 + trkside * 4);
	
	if(!IS_VALID_TRACK(offset.d)) {
		return false;
	}
	
	// track found
	sector = buffer + offset.d;
	sector_num.read_2bytes_le_from(sector + 4);
	pair data_size;
	data_size.read_2bytes_le_from(sector + 14);
	
	// create each sector position in track
	int sync_size  = drive_mfm ? 12 : 6;
	int am_size = drive_mfm ? 3 : 0;
	int gap0_size = drive_mfm ? 80 : 40;
	int gap1_size = drive_mfm ? 50 : 26;
	int gap2_size = drive_mfm ? 22 : 11;
	int gap3_size = 0, gap4_size;
	
	if(media_type == MEDIA_TYPE_144 || media_type == MEDIA_TYPE_2HD) {
		if(drive_mfm) {
			if(data_size.sd ==  256 && sector_num.sd == 26) gap3_size =  54;
			if(data_size.sd ==  512 && sector_num.sd == 15) gap3_size =  84;
			if(data_size.sd == 1024 && sector_num.sd ==  8) gap3_size = 116;
		} else {
			if(data_size.sd ==  128 && sector_num.sd == 26) gap3_size =  27;
			if(data_size.sd ==  256 && sector_num.sd == 15) gap3_size =  42;
			if(data_size.sd ==  512 && sector_num.sd ==  8) gap3_size =  58;
		}
	} else {
		if(drive_mfm) {
			if(data_size.sd ==  256 && sector_num.sd == 16) gap3_size =  51;
			if(data_size.sd ==  512 && sector_num.sd ==  9) gap3_size =  80;
			if(data_size.sd == 1024 && sector_num.sd ==  5) gap3_size = 116;
		} else {
			if(data_size.sd ==  128 && sector_num.sd == 16) gap3_size =  27;
			if(data_size.sd ==  256 && sector_num.sd ==  9) gap3_size =  42;
			if(data_size.sd ==  512 && sector_num.sd ==  5) gap3_size =  58;
		}
	}
	
	uint8* t = sector;
	int total = sync_size + (am_size + 1);
	
	for(int i = 0; i < sector_num.sd; i++) {
		data_size.read_2bytes_le_from(t + 14);
		total += sync_size + (am_size + 1) + (4 + 2) + gap2_size + sync_size + (am_size + 1);
		total += data_size.sd + 2;
		if(t[2] != i + 1) {
			no_skew = false;
		}
		t += data_size.sd + 0x10;
	}
	if(gap3_size == 0) {
		gap3_size = (get_track_size() - total - gap0_size - gap1_size) / (sector_num.sd + 1);
	}
	gap4_size = get_track_size() - total - gap0_size - gap1_size - gap3_size * sector_num.sd;
	
	if(gap3_size < 8 || gap4_size < 8) {
		gap0_size = gap1_size = gap3_size = (get_track_size() - total) / (2 + sector_num.sd + 1);
		gap4_size = get_track_size() - total - gap0_size - gap1_size - gap3_size * sector_num.sd;
	}
	if(gap3_size < 8 || gap4_size < 8) {
		gap0_size = gap1_size = gap3_size = gap4_size = 32;
		invalid_format = true;
	}
	int preamble_size = gap0_size + sync_size + (am_size + 1) + gap1_size;
	
	t = sector;
	total = preamble_size;
	sync_position[array_length(sync_position) - 1] = gap0_size; // sync position in preamble
	
	for(int i = 0; i < sector_num.sd; i++) {
		data_size.read_2bytes_le_from(t + 14);
		if(invalid_format) {
			total = preamble_size + (get_track_size() - preamble_size - gap4_size) * i / sector_num.sd;
		}
		sync_position[i] = total;
		total += sync_size + (am_size + 1);
		id_position[i] = total;
		total += (4 + 2) + gap2_size + sync_size + (am_size + 1);
		data_position[i] = total;
		total += data_size.sd + 2 + gap3_size;
		t += data_size.sd + 0x10;
	}
	return true;
}

bool DISK::make_track(int trk, int side)
{
	int track_size = get_track_size();
	
	if(!get_track(trk, side)) {
		// create a dummy track
		for(int i = 0; i < track_size; i++) {
			track[i] = rand();
		}
		return false;
	}
	
	// make track image
	int sync_size  = drive_mfm ? 12 : 6;
	int am_size = drive_mfm ? 3 : 0;
	int gap2_size = drive_mfm ? 22 : 11;
	uint8 gap_data = drive_mfm ? 0x4e : 0xff;
	
	// preamble
	memset(track, gap_data, track_size);
	int q = sync_position[array_length(sync_position) - 1];
	
	// sync
	for(int i = 0; i < sync_size; i++) {
		track[q++] = 0x00;
	}
	// index mark
	for(int i = 0; i < am_size; i++) {
		track[q++] = 0xc2;
	}
	track[q++] = 0xfc;
	
	// sectors
	uint8 *t = sector;
	
	for(int i = 0; i < sector_num.sd; i++) {
		pair data_size;
		data_size.read_2bytes_le_from(t + 14);
		int p = sync_position[i];
		
		// sync
		for(int j = 0; j < sync_size; j++) {
			if(p < track_size) track[p++] = 0x00;
		}
		// am1
		for(int j = 0; j < am_size; j++) {
			if(p < track_size) track[p++] = 0xa1;
		}
		if(p < track_size) track[p++] = 0xfe;
		// id
		if(p < track_size) track[p++] = t[0];
		if(p < track_size) track[p++] = t[1];
		if(p < track_size) track[p++] = t[2];
		if(p < track_size) track[p++] = t[3];
		uint16 crc = 0;
		crc = (uint16)((crc << 8) ^ crc_table[(uint8)(crc >> 8) ^ t[0]]);
		crc = (uint16)((crc << 8) ^ crc_table[(uint8)(crc >> 8) ^ t[1]]);
		crc = (uint16)((crc << 8) ^ crc_table[(uint8)(crc >> 8) ^ t[2]]);
		crc = (uint16)((crc << 8) ^ crc_table[(uint8)(crc >> 8) ^ t[3]]);
		if(p < track_size) track[p++] = (crc >> 8) & 0xff;
		if(p < track_size) track[p++] = (crc >> 0) & 0xff;
		// gap2
		for(int j = 0; j < gap2_size; j++) {
			if(p < track_size) track[p++] = gap_data;
		}
		// sync
		for(int j = 0; j < sync_size; j++) {
			if(p < track_size) track[p++] = 0x00;
		}
		// am2
		for(int j = 0; j < am_size; j++) {
			if(p < track_size) track[p++] = 0xa1;
		}
		if(p < track_size) track[p++] = (t[7] != 0) ? 0xf8 : 0xfb;
		// data
		crc = 0;
		for(int j = 0; j < data_size.sd; j++) {
			if(p < track_size) track[p++] = t[0x10 + j];
			crc = (uint16)((crc << 8) ^ crc_table[(uint8)(crc >> 8) ^ t[0x10 + j]]);
		}
		if(p < track_size) track[p++] = (crc >> 8) & 0xff;
		if(p < track_size) track[p++] = (crc >> 0) & 0xff;
		
		t += data_size.sd + 0x10;
	}
	return true;
}

bool DISK::get_sector(int trk, int side, int index)
{
	sector_size.sd = sector_num.sd = 0;
	sector = NULL;
	
	// disk not inserted or invalid media type
	if(!(inserted && check_media_type())) {
		return false;
	}
	
	// search track
	int trkside = trk * 2 + (side & 1);
	if(!(0 <= trkside && trkside < 164)) {
		return false;
	}
	pair offset;
	offset.read_4bytes_le_from(buffer + 0x20 + trkside * 4);
	
	if(!IS_VALID_TRACK(offset.d)) {
		return false;
	}
	
	// track found
	uint8* t = buffer + offset.d;
	sector_num.read_2bytes_le_from(t + 4);
	
	if(index >= sector_num.sd) {
		return false;
	}
	
	// skip sector
	for(int i = 0; i < index; i++) {
		pair data_size;
		data_size.read_2bytes_le_from(t + 14);
		t += data_size.sd + 0x10;
	}
	set_sector_info(t);
	return true;
}

void DISK::set_sector_info(uint8 *t)
{
	// header info
	id[0] = t[0];
	id[1] = t[1];
	id[2] = t[2];
	id[3] = t[3];
	uint16 crc = 0;
	crc = (uint16)((crc << 8) ^ crc_table[(uint8)(crc >> 8) ^ t[0]]);
	crc = (uint16)((crc << 8) ^ crc_table[(uint8)(crc >> 8) ^ t[1]]);
	crc = (uint16)((crc << 8) ^ crc_table[(uint8)(crc >> 8) ^ t[2]]);
	crc = (uint16)((crc << 8) ^ crc_table[(uint8)(crc >> 8) ^ t[3]]);
	id[4] = (crc >> 8) & 0xff;
	id[5] = (crc >> 0) & 0xff;
	density = t[6];
	deleted = (t[7] != 0);
	//crc_error = (t[8] != 0x00 && t[8] != 0x10);
	crc_error = false; // Assume NOT CRC ERROR for any disk image.
	sector = t + 0x10;
	sector_size.read_2bytes_le_from(t + 14);
}

void DISK::set_deleted(bool value)
{
	if(sector != NULL) {
		uint8 *t = sector - 0x10;
		t[7] = value ? 0x10 : 0;
		if(t[8] == 0x00 || t[8] == 0x10) {
			t[8] = t[7];
		}
	}
	deleted = value;
}

void DISK::set_crc_error(bool value)
{
	if(sector != NULL) {
		uint8 *t = sector - 0x10;
		t[8] = value ? 0xb0 : t[7];
	}
	crc_error = value;
}

bool DISK::format_track(int trk, int side)
{
	// disk not inserted or invalid media type
	if(!(inserted && check_media_type())) {
		return false;
	}
	
	// search track
	int trkside = trk * 2 + (side & 1);
	if(!(0 <= trkside && trkside < 164)) {
		return false;
	}
	
	// create new empty track
	if(trim_required) {
		trim_buffer();
		trim_required = false;
	}
	memset(buffer + DISK_BUFFER_SIZE, 0, sizeof(buffer) - DISK_BUFFER_SIZE);
	pair offset;
	offset.d = DISK_BUFFER_SIZE;
	offset.write_4bytes_le_to(buffer + 0x20 + trkside * 4);
	
	trim_required = true;
	sector_num.sd = 0;
	return true;
}

void DISK::insert_sector(uint8 c, uint8 h, uint8 r, uint8 n, bool deleted, bool crc_error, uint8 fill_data, int length)
{
	uint8* t = buffer + DISK_BUFFER_SIZE;
	
	sector_num.sd++;
	for(int i = 0; i < (sector_num.sd - 1); i++) {
		t[4] = sector_num.b.l;
		t[5] = sector_num.b.h;
		pair data_size;
		data_size.read_2bytes_le_from(t + 14);
		t += data_size.sd + 0x10;
	}
	t[0] = c;
	t[1] = h;
	t[2] = r;
	t[3] = n;
	t[4] = sector_num.b.l;
	t[5] = sector_num.b.h;
	t[6] = drive_mfm ? 0 : 0x40;
	t[7] = deleted ? 0x10 : 0;
	t[8] = crc_error ? 0xb0 : t[7];
	t[14] = (length >> 0) & 0xff;
	t[15] = (length >> 8) & 0xff;
	memset(t + 16, fill_data, length);
	
	set_sector_info(t);
}

void DISK::trim_buffer()
{
	int max_tracks = 164;
	uint32 dest_offset = 0x2b0;
	
	// copy header
	memset(tmp_buffer, 0, sizeof(tmp_buffer));
	memcpy(tmp_buffer, buffer, 0x20);
	
	// check max tracks
	for(int trkside = 0; trkside < 164; trkside++) {
		pair src_trk_offset;
		src_trk_offset.read_4bytes_le_from(buffer + 0x20 + trkside * 4);
		if(src_trk_offset.d != 0) {
#if 1
			if(src_trk_offset.d < 0x2b0) {
				max_tracks = (src_trk_offset.d - 0x20) >> 2;
			}
#else
			if(src_trk_offset.d != 0x2b0) {
				max_tracks = (src_trk_offset.d - 0x20) >> 2;
				if(max_tracks > 164) {
					dest_offset = 0x20 + max_tracks * 4);
				}
			}
#endif
			break;
		}
	}
	
	// copy tracks
	for(int trkside = 0; trkside < max_tracks; trkside++) {
		pair src_trk_offset;
		src_trk_offset.read_4bytes_le_from(buffer + 0x20 + trkside * 4);
		
		pair dest_trk_offset;
		dest_trk_offset.d = 0;
		
		if(IS_VALID_TRACK(src_trk_offset.d)) {
			uint8* t = buffer + src_trk_offset.d;
			pair sector_num, data_size;
			sector_num.read_2bytes_le_from(t + 4);
			if(sector_num.sd != 0) {
				dest_trk_offset.d = dest_offset;
				for(int i = 0; i < sector_num.sd; i++) {
					data_size.read_2bytes_le_from(t + 14);
					memcpy(tmp_buffer + dest_offset, t, data_size.sd + 0x10);
					dest_offset += data_size.sd + 0x10;
					t += data_size.sd + 0x10;
				}
			}
		}
		dest_trk_offset.write_4bytes_le_to(tmp_buffer + 0x20 + trkside * 4);
	}
	
	// update file size
	file_size.d = dest_offset;
	file_size.write_4bytes_le_to(tmp_buffer + 0x1c);
	
	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, tmp_buffer, file_size.d);
}

int DISK::get_rpm()
{
	if(drive_rpm != 0) {
		return drive_rpm;
	} else if(inserted) {
		return (media_type == MEDIA_TYPE_2HD) ? 360 : 300;
	} else {
		return (drive_type == DRIVE_TYPE_2HD) ? 360 : 300;
	}
}

int DISK::get_track_size()
{
	if(inserted) {
		return media_type == MEDIA_TYPE_144 ? 12500 : media_type == MEDIA_TYPE_2HD ? 10410 : drive_mfm ? 6250 : 3100;
	} else {
		return drive_type == DRIVE_TYPE_144 ? 12500 : drive_type == DRIVE_TYPE_2HD ? 10410 : drive_mfm ? 6250 : 3100;
	}
}

double DISK::get_usec_per_bytes(int bytes)
{
	return 1000000.0 / (get_track_size() * (get_rpm() / 60.0)) * bytes;
}

bool DISK::check_media_type()
{
	switch(drive_type) {
	case DRIVE_TYPE_2D:
		return (media_type == MEDIA_TYPE_2D);
	case DRIVE_TYPE_2DD:
		return (media_type == MEDIA_TYPE_2D || media_type == MEDIA_TYPE_2DD);
	case DRIVE_TYPE_2HD:
		return (media_type == MEDIA_TYPE_2HD);
	case DRIVE_TYPE_144:
		return (media_type == MEDIA_TYPE_144);
	case DRIVE_TYPE_UNK:
		return true; // always okay
	}
	return false;
}

// teledisk image decoder

/*
	this teledisk image decoder is based on:
	
		LZHUF.C English version 1.0 based on Japanese version 29-NOV-1988
		LZSS coded by Haruhiko OKUMURA
		Adaptive Huffman Coding coded by Haruyasu YOSHIZAKI
		Edited and translated to English by Kenji RIKITAKE
		TDLZHUF.C by WTK
*/

#define COPYBUFFER(src, size) { \
	if(file_size.d + (size) > DISK_BUFFER_SIZE) { \
		return false; \
	} \
	memcpy(buffer + file_size.d, (src), (size)); \
	file_size.d += (size); \
}

bool DISK::teledisk_to_d88()
{
	td_hdr_t hdr;
	td_cmt_t cmt;
	td_trk_t trk;
	td_sct_t sct;
	d88_hdr_t d88_hdr;
	d88_sct_t d88_sct;
	uint8 obuf[512];
	
	// check teledisk header
	fi->Fseek(0, FILEIO_SEEK_SET);
	fi->Fread(&hdr, sizeof(td_hdr_t), 1);
	if(hdr.sig[0] == 't' && hdr.sig[1] == 'd') {
		// decompress to the temporary file
		FILEIO* fo = new FILEIO();
		if(!fo->Fopen(temp_path, FILEIO_WRITE_BINARY)) {
			delete fo;
			return false;
		}
		int rd = 1;
		init_decode();
		do {
			if((rd = decode(obuf, 512)) > 0) {
				fo->Fwrite(obuf, rd, 1);
			}
		}
		while(rd > 0);
		fo->Fclose();
		delete fo;
		temporary = true;
		
		// reopen the temporary file
		fi->Fclose();
		if(!fi->Fopen(temp_path, FILEIO_READ_BINARY)) {
			return false;
		}
	}
	if(hdr.flag & 0x80) {
		// skip comment
		fi->Fread(&cmt, sizeof(td_cmt_t), 1);
		fi->Fseek(cmt.len, FILEIO_SEEK_CUR);
	}
	
	// create d88 image
	file_size.d = 0;
	
	// create d88 header
	memset(&d88_hdr, 0, sizeof(d88_hdr_t));
	_strcpy_s(d88_hdr.title, sizeof(d88_hdr.title), "TELEDISK");
	d88_hdr.protect = 0; // non-protected
	COPYBUFFER(&d88_hdr, sizeof(d88_hdr_t));
	
	// create tracks
	int trkcnt = 0, trkptr = sizeof(d88_hdr_t);
	fi->Fread(&trk, sizeof(td_trk_t), 1);
	while(trk.nsec != 0xff) {
		d88_hdr.trkptr[trkcnt++] = trkptr;
		if(hdr.sides == 1) {
			d88_hdr.trkptr[trkcnt++] = trkptr;
		}
		
		// read sectors in this track
		for(int i = 0; i < trk.nsec; i++) {
			uint8 buf[2048], dst[2048];
			memset(buf, 0, sizeof(buf));
			memset(dst, 0, sizeof(dst));
			
			// read sector header
			fi->Fread(&sct, sizeof(td_sct_t), 1);
			
			// create d88 sector header
			memset(&d88_sct, 0, sizeof(d88_sct_t));
			d88_sct.c = sct.c;
			d88_sct.h = sct.h;
			d88_sct.r = sct.r;
			d88_sct.n = sct.n;
			d88_sct.nsec = trk.nsec;
			d88_sct.dens = (hdr.dens & 0x80) ? 0x40 : 0;
			d88_sct.del = (sct.ctrl & 4) ? 0x10 : 0;
			d88_sct.stat = (sct.ctrl & 2) ? 0xb0 : d88_sct.del;
			d88_sct.size = secsize[sct.n & 3];
			
			// create sector image
			if(sct.ctrl != 0x10) {
				// read sector source
				int len = fi->Fgetc();
				len += fi->Fgetc() * 256 - 1;
				int flag = fi->Fgetc(), d = 0;
				fi->Fread(buf, len, 1);
				
				// convert
				if(flag == 0) {
					memcpy(dst, buf, len);
				} else if(flag == 1) {
					pair len2;
					len2.read_2bytes_le_from(buf);
					while(len2.sd--) {
						dst[d++] = buf[2];
						dst[d++] = buf[3];
					}
				} else if(flag == 2) {
					for(int s = 0; s < len;) {
						int type = buf[s++];
						int len2 = buf[s++];
						if(type == 0) {
							while(len2--) {
								dst[d++] = buf[s++];
							}
						} else if(type < 5) {
							uint8 pat[256];
							int n = 2;
							while(type-- > 1) {
								n *= 2;
							}
							for(int j = 0; j < n; j++) {
								pat[j] = buf[s++];
							}
							while(len2--) {
								for(int j = 0; j < n; j++) {
									dst[d++] = pat[j];
								}
							}
						} else {
							break; // unknown type
						}
					}
				} else {
					break; // unknown flag
				}
			} else {
				d88_sct.size = 0;
			}
			
			// copy to d88
			COPYBUFFER(&d88_sct, sizeof(d88_sct_t));
			COPYBUFFER(dst, d88_sct.size);
			trkptr += sizeof(d88_sct_t) + d88_sct.size;
		}
		// read next track
		fi->Fread(&trk, sizeof(td_trk_t), 1);
	}
	d88_hdr.type = ((hdr.dens & 3) == 2) ? MEDIA_TYPE_2HD : ((trkcnt >> 1) > 60) ? MEDIA_TYPE_2DD : MEDIA_TYPE_2D;
	d88_hdr.size = trkptr;
	memcpy(buffer, &d88_hdr, sizeof(d88_hdr_t));
	return true;
}

int DISK::next_word()
{
	if(ibufndx >= ibufcnt) {
		ibufndx = ibufcnt = 0;
		memset(inbuf, 0, 512);
		for(int i = 0; i < 512; i++) {
			int d = fi->Fgetc();
			if(d == EOF) {
				if(i) {
					break;
				}
				return(-1);
			}
			inbuf[i] = d;
			ibufcnt = i + 1;
		}
	}
	while(getlen <= 8) {
		getbuf |= inbuf[ibufndx++] << (8 - getlen);
		getlen += 8;
	}
	return 0;
}

int DISK::get_bit()
{
	if(next_word() < 0) {
		return -1;
	}
	short i = getbuf;
	getbuf <<= 1;
	getlen--;
	return (i < 0) ? 1 : 0;
}

int DISK::get_byte()
{
	if(next_word() != 0) {
		return -1;
	}
	uint16 i = getbuf;
	getbuf <<= 8;
	getlen -= 8;
	i >>= 8;
	return (int)i;
}

void DISK::start_huff()
{
	int i, j;
	for(i = 0; i < N_CHAR; i++) {
		freq[i] = 1;
		son[i] = i + TABLE_SIZE;
		prnt[i + TABLE_SIZE] = i;
	}
	i = 0; j = N_CHAR;
	while(j <= ROOT_POSITION) {
		freq[j] = freq[i] + freq[i + 1];
		son[j] = i;
		prnt[i] = prnt[i + 1] = j;
		i += 2; j++;
	}
	freq[TABLE_SIZE] = 0xffff;
	prnt[ROOT_POSITION] = 0;
}

void DISK::reconst()
{
	short i, j = 0, k;
	uint16 f, l;
	for(i = 0; i < TABLE_SIZE; i++) {
		if(son[i] >= TABLE_SIZE) {
			freq[j] = (freq[i] + 1) / 2;
			son[j] = son[i];
			j++;
		}
	}
	for(i = 0, j = N_CHAR; j < TABLE_SIZE; i += 2, j++) {
		k = i + 1;
		f = freq[j] = freq[i] + freq[k];
		for(k = j - 1; f < freq[k]; k--);
		k++;
		l = (j - k) * 2;
		memmove(&freq[k + 1], &freq[k], l);
		freq[k] = f;
		memmove(&son[k + 1], &son[k], l);
		son[k] = i;
	}
	for(i = 0; i < TABLE_SIZE; i++) {
		if((k = son[i]) >= TABLE_SIZE) {
			prnt[k] = i;
		} else {
			prnt[k] = prnt[k + 1] = i;
		}
	}
}

void DISK::update(int c)
{
	int i, j, k, l;
	if(freq[ROOT_POSITION] == MAX_FREQ) {
		reconst();
	}
	c = prnt[c + TABLE_SIZE];
	do {
		k = ++freq[c];
		if(k > freq[l = c + 1]) {
			while(k > freq[++l]);
			l--;
			freq[c] = freq[l];
			freq[l] = k;
			i = son[c];
			prnt[i] = l;
			if(i < TABLE_SIZE) {
				prnt[i + 1] = l;
			}
			j = son[l];
			son[l] = i;
			prnt[j] = c;
			if(j < TABLE_SIZE) {
				prnt[j + 1] = c;
			}
			son[c] = j;
			c = l;
		}
	}
	while((c = prnt[c]) != 0);
}

short DISK::decode_char()
{
	int ret;
	uint16 c = son[ROOT_POSITION];
	while(c < TABLE_SIZE) {
		if((ret = get_bit()) < 0) {
			return -1;
		}
		c += (unsigned)ret;
		c = son[c];
	}
	c -= TABLE_SIZE;
	update(c);
	return c;
}

short DISK::decode_position()
{
	short bit;
	uint16 i, j, c;
	if((bit = get_byte()) < 0) {
		return -1;
	}
	i = (uint16)bit;
	c = (uint16)d_code[i] << 6;
	j = d_len[i] - 2;
	while(j--) {
		if((bit = get_bit()) < 0) {
			 return -1;
		}
		i = (i << 1) + bit;
	}
	return (c | i & 0x3f);
}

void DISK::init_decode()
{
	ibufcnt= ibufndx = bufcnt = getbuf = 0;
	getlen = 0;
	start_huff();
	for(int i = 0; i < STRING_BUFFER_SIZE - LOOKAHEAD_BUFFER_SIZE; i++) {
		text_buf[i] = ' ';
	}
	ptr = STRING_BUFFER_SIZE - LOOKAHEAD_BUFFER_SIZE;
}

int DISK::decode(uint8 *buf, int len)
{
	short c, pos;
	int  count;
	for(count = 0; count < len;) {
		if(bufcnt == 0) {
			if((c = decode_char()) < 0) {
				return count;
			}
			if(c < 256) {
				*(buf++) = (uint8)c;
				text_buf[ptr++] = (uint8)c;
				ptr &= (STRING_BUFFER_SIZE - 1);
				count++;
			} else {
				if((pos = decode_position()) < 0) {
					return count;
				}
				bufpos = (ptr - pos - 1) & (STRING_BUFFER_SIZE - 1);
				bufcnt = c - 255 + THRESHOLD;
				bufndx = 0;
			}
		} else {
			while(bufndx < bufcnt && count < len) {
				c = text_buf[(bufpos + bufndx) & (STRING_BUFFER_SIZE - 1)];
				*(buf++) = (uint8)c;
				bufndx++;
				text_buf[ptr++] = (uint8)c;
				ptr &= (STRING_BUFFER_SIZE - 1);
				count++;
			}
			if(bufndx >= bufcnt) {
				bufndx = bufcnt = 0;
			}
		}
	}
	return count;
}

// imagedisk image decoder

bool DISK::imagedisk_to_d88()
{
	imd_trk_t trk;
	d88_hdr_t d88_hdr;
	d88_sct_t d88_sct;
	
	// skip comment
	fi->Fseek(0, FILEIO_SEEK_SET);
	int tmp;
	while((tmp = fi->Fgetc()) != 0x1a) {
		if(tmp == EOF) {
			return false;
		}
	}
	
	// create d88 image
	file_size.d = 0;
	
	// create d88 header
	memset(&d88_hdr, 0, sizeof(d88_hdr_t));
	_strcpy_s(d88_hdr.title, sizeof(d88_hdr.title), "IMAGEDISK");
	d88_hdr.protect = 0; // non-protected
	COPYBUFFER(&d88_hdr, sizeof(d88_hdr_t));
	
	// create tracks
	int trkptr = sizeof(d88_hdr_t);
	int trkcnt = 0, mode;
	
	for(int t = 0; t < 164; t++) {
		// check end of file
		if(fi->Fread(&trk, sizeof(imd_trk_t), 1) != 1) {
			break;
		}
		trkcnt = t;
		
		// check track header
		if(t == 0) {
			mode = trk.mode % 3; // 0=500kbps, 1=300kbps, 2=250kbps
		}
		if(!trk.nsec) {
			continue;
		}
		d88_hdr.trkptr[t] = trkptr;
		
		// setup sector id
		uint8 c[64], h[64], r[64];
		fi->Fread(r, trk.nsec, 1);
		if(trk.head & 0x80) {
			fi->Fread(c, trk.nsec, 1);
		} else {
			memset(c, trk.cyl, sizeof(c));
		}
		if(trk.head & 0x40) {
			fi->Fread(h, trk.nsec, 1);
		} else {
			memset(h, trk.head & 1, sizeof(h));
		}
		
		// read sectors in this track
		for(int i = 0; i < trk.nsec; i++) {
			// create d88 sector header
			int sectype = fi->Fgetc();
			if(sectype > 8) {
				return false;
			}
			memset(&d88_sct, 0, sizeof(d88_sct_t));
			d88_sct.c = c[i];
			d88_sct.h = h[i];
			d88_sct.r = r[i];
			d88_sct.n = trk.size;
			d88_sct.nsec = trk.nsec;
			d88_sct.dens = (trk.mode < 3) ? 0x40 : 0;
			d88_sct.del = (sectype == 3 || sectype == 4 || sectype == 7 || sectype == 8) ? 0x10 : 0;
			d88_sct.stat = (sectype == 5 || sectype == 6 || sectype == 7 || sectype == 8) ? 0xb0 : d88_sct.del;
			d88_sct.size = secsize[trk.size & 7];
			
			// create sector image
			uint8 dst[8192];
			if(sectype == 1 || sectype == 3 || sectype == 5 || sectype == 7) {
				// uncompressed
				fi->Fread(dst, d88_sct.size, 1);
			} else if(sectype == 2 || sectype == 4 || sectype == 6 || sectype == 8) {
				// compressed
				int tmp = fi->Fgetc();
				memset(dst, tmp, d88_sct.size);
			} else {
				d88_sct.size = 0;
			}
			
			// copy to d88
			COPYBUFFER(&d88_sct, sizeof(d88_sct_t));
			COPYBUFFER(dst, d88_sct.size);
			trkptr += sizeof(d88_sct_t) + d88_sct.size;
		}
	}
	d88_hdr.type = (mode == 0) ? MEDIA_TYPE_2HD : ((trkcnt >> 1) > 60) ? MEDIA_TYPE_2DD : MEDIA_TYPE_2D;
	d88_hdr.size = trkptr;
	memcpy(buffer, &d88_hdr, sizeof(d88_hdr_t));
	return true;
}

// cpdread image decoder (from MESS formats/dsk_dsk.c)

bool DISK::cpdread_to_d88(int extended)
{
	d88_hdr_t d88_hdr;
	d88_sct_t d88_sct;
	int total = 0;
	
	// get cylinder number and side number
	memcpy(tmp_buffer, buffer, file_size.d);
	int ncyl = tmp_buffer[0x30];
	int nside = tmp_buffer[0x31];
	
	// create d88 image
	file_size.d = 0;
	
	// create d88 header
	memset(&d88_hdr, 0, sizeof(d88_hdr_t));
	_strcpy_s(d88_hdr.title, sizeof(d88_hdr.title), "CPDRead");
	d88_hdr.protect = 0; // non-protected
	COPYBUFFER(&d88_hdr, sizeof(d88_hdr_t));
	
	// create tracks
	int trkofs = 0x100, trkofs_ptr = 0x34;
	int trkptr = sizeof(d88_hdr_t);
	
	for(int c = 0; c < ncyl; c++) {
		for(int h = 0; h < nside; h++) {
			// read sectors in this track
			uint8 *track_info = tmp_buffer + trkofs;
			int cyl = track_info[0x10];
			int side = track_info[0x11];
			int nsec = track_info[0x15];
			int size = 1 << (track_info[0x14] + 7); // standard
			int sctofs = trkofs + 0x100;
			
			if(nside == 1) {
				// double side
				d88_hdr.trkptr[2 * cyl] = d88_hdr.trkptr[2 * cyl + 1] = trkptr;
			} else {
				d88_hdr.trkptr[2 * cyl + side] = trkptr;
			}
			for(int s = 0; s < nsec; s++) {
				// get sector size
				uint8 *sector_info = tmp_buffer + trkofs + 0x18 + s * 8;
				if(extended) {
					size = sector_info[6] + sector_info[7] * 256;
				}
				
				// create d88 sector header
				memset(&d88_sct, 0, sizeof(d88_sct_t));
				d88_sct.c = sector_info[0];
				d88_sct.h = sector_info[1];
				d88_sct.r = sector_info[2];
				d88_sct.n = sector_info[3];
				d88_sct.nsec = nsec;
				d88_sct.dens = 0;
				d88_sct.del = (sector_info[5] == 0xb2) ? 0x10 : 0;
				d88_sct.stat = (sector_info[5] == 0xb5) ? 0xb0 : d88_sct.del;
				d88_sct.size = size;
				
				// copy to d88
				COPYBUFFER(&d88_sct, sizeof(d88_sct_t));
				COPYBUFFER(tmp_buffer + sctofs, size);
				trkptr += sizeof(d88_sct_t) + size;
				sctofs += size;
				total += size;
			}
			
			if(extended) {
				trkofs += tmp_buffer[trkofs_ptr++] * 256;
			} else {
				trkofs += tmp_buffer[0x32] + tmp_buffer[0x33] * 256;
			}
		}
	}
	d88_hdr.type = (total < (368640 + 655360) / 2) ? MEDIA_TYPE_2D : (total < (737280 + 1228800) / 2) ? MEDIA_TYPE_2DD : MEDIA_TYPE_2HD;
	d88_hdr.size = trkptr;
	memcpy(buffer, &d88_hdr, sizeof(d88_hdr_t));
	return true;
}

// standard image decoder

bool DISK::standard_to_d88(int type, int ncyl, int nside, int nsec, int size)
{
	d88_hdr_t d88_hdr;
	d88_sct_t d88_sct;
	int n = 0, t = 0;
	
	file_size.d = 0;
	
	// create d88 header
	memset(&d88_hdr, 0, sizeof(d88_hdr_t));
	_strcpy_s(d88_hdr.title, sizeof(d88_hdr.title), "STANDARD");
	d88_hdr.protect = 0; // non-protected
	d88_hdr.type = (type == MEDIA_TYPE_144) ? MEDIA_TYPE_2HD : type;
	media_type = type;
	COPYBUFFER(&d88_hdr, sizeof(d88_hdr_t));
	
	// sector length
	for(int i = 0; i < 8; i++) {
		if(size == (128 << i)) {
			n = i;
			break;
		}
	}
	
	// create tracks
	int trkptr = sizeof(d88_hdr_t);
	for(int c = 0; c < ncyl; c++) {
		for(int h = 0; h < nside; h++) {
			d88_hdr.trkptr[t++] = trkptr;
			if(nside == 1) {
				// double side
				d88_hdr.trkptr[t++] = trkptr;
			}
			
			// read sectors in this track
			for(int s = 0; s < nsec; s++) {
				// create d88 sector header
				memset(&d88_sct, 0, sizeof(d88_sct_t));
				d88_sct.c = c;
				d88_sct.h = h;
				d88_sct.r = s + 1;
				d88_sct.n = n;
				d88_sct.nsec = nsec;
				d88_sct.dens = 0;
				d88_sct.del = 0;
				d88_sct.stat = 0;
				d88_sct.size = size;
				
				// create sector image
				uint8 dst[16384];
				memset(dst, 0xe5, sizeof(dst));
				fi->Fread(dst, size, 1);
				
				// copy to d88
				COPYBUFFER(&d88_sct, sizeof(d88_sct_t));
				COPYBUFFER(dst, size);
				trkptr += sizeof(d88_sct_t) + size;
			}
		}
	}
	d88_hdr.size = trkptr;
	memcpy(buffer, &d88_hdr, sizeof(d88_hdr_t));
	return true;
}

#define STATE_VERSION	4

void DISK::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	
	state_fio->Fwrite(buffer, sizeof(buffer), 1);
	state_fio->Fwrite(orig_path, sizeof(orig_path), 1);
	state_fio->Fwrite(dest_path, sizeof(dest_path), 1);
	state_fio->FputUint32(file_size.d);
	state_fio->FputInt32(file_bank);
	state_fio->FputUint32(crc32);
	state_fio->Fwrite(fdi_header, sizeof(fdi_header), 1);
	state_fio->FputBool(inserted);
	state_fio->FputBool(ejected);
	state_fio->FputBool(write_protected);
	state_fio->FputBool(changed);
	state_fio->FputUint8(media_type);
	state_fio->FputBool(is_standard_image);
	state_fio->FputBool(is_fdi_image);
	state_fio->FputInt32(is_special_disk);
	state_fio->Fwrite(track, sizeof(track), 1);
	state_fio->FputInt32(sector_num.sd);
	state_fio->FputBool(invalid_format);
	state_fio->FputBool(no_skew);
	state_fio->Fwrite(sync_position, sizeof(sync_position), 1);
	state_fio->Fwrite(id_position, sizeof(id_position), 1);
	state_fio->Fwrite(data_position, sizeof(data_position), 1);
	state_fio->FputInt32(sector ? (int)(sector - buffer) : -1);
	state_fio->FputInt32(sector_size.sd);
	state_fio->Fwrite(id, sizeof(id), 1);
	state_fio->FputUint8(density);
	state_fio->FputBool(deleted);
	state_fio->FputBool(crc_error);
	state_fio->FputUint8(drive_type);
	state_fio->FputInt32(drive_rpm);
	state_fio->FputBool(drive_mfm);
}

bool DISK::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	state_fio->Fread(buffer, sizeof(buffer), 1);
	state_fio->Fread(orig_path, sizeof(orig_path), 1);
	state_fio->Fread(dest_path, sizeof(dest_path), 1);
	file_size.d = state_fio->FgetUint32();
	file_bank = state_fio->FgetInt32();
	crc32 = state_fio->FgetUint32();
	state_fio->Fread(fdi_header, sizeof(fdi_header), 1);
	inserted = state_fio->FgetBool();
	ejected = state_fio->FgetBool();
	write_protected = state_fio->FgetBool();
	changed = state_fio->FgetBool();
	media_type = state_fio->FgetUint8();
	is_standard_image = state_fio->FgetBool();
	is_fdi_image = state_fio->FgetBool();
	is_special_disk = state_fio->FgetInt32();
	state_fio->Fread(track, sizeof(track), 1);
	sector_num.sd = state_fio->FgetInt32();
	invalid_format = state_fio->FgetBool();
	no_skew = state_fio->FgetBool();
	state_fio->Fread(sync_position, sizeof(sync_position), 1);
	state_fio->Fread(id_position, sizeof(id_position), 1);
	state_fio->Fread(data_position, sizeof(data_position), 1);
	int offset = state_fio->FgetInt32();
	sector = (offset != -1) ? buffer + offset : NULL;
	sector_size.sd = state_fio->FgetInt32();
	state_fio->Fread(id, sizeof(id), 1);
	density = state_fio->FgetUint8();
	deleted = state_fio->FgetBool();
	crc_error = state_fio->FgetBool();
	drive_type = state_fio->FgetUint8();
	drive_rpm = state_fio->FgetInt32();
	drive_mfm = state_fio->FgetBool();
	return true;
}

