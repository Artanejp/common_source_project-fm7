/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.16-

	[ d88 image handler ]
*/

#ifndef _DISK_H_
#define _DISK_H_

#ifndef _ANY2D88
#include "vm.h"
#include "../emu.h"
#else
#include "../common.h"
#endif

// d88 media type
#define MEDIA_TYPE_2D	0x00
#define MEDIA_TYPE_2DD	0x10
#define MEDIA_TYPE_2HD	0x20
#define MEDIA_TYPE_144	0x30
#define MEDIA_TYPE_UNK	0xff

#define DRIVE_TYPE_2D	MEDIA_TYPE_2D
#define DRIVE_TYPE_2DD	MEDIA_TYPE_2DD
#define DRIVE_TYPE_2HD	MEDIA_TYPE_2HD
#define DRIVE_TYPE_144	MEDIA_TYPE_144
#define DRIVE_TYPE_UNK	MEDIA_TYPE_UNK

// this value will be stored to the state file,
// so don't change these definitions
#define SPECIAL_DISK_X1TURBO_ALPHA	 1
#define SPECIAL_DISK_X1_BATTEN		 2
#define SPECIAL_DISK_FM7_GAMBLER	11
#define SPECIAL_DISK_FM7_DEATHFORCE	12
#define SPECIAL_DISK_FM77AV_PSYOBLADE	13
#define SPECIAL_DISK_FM7_TAIYOU1	14
#define SPECIAL_DISK_FM7_TAIYOU2	15
#define SPECIAL_DISK_FM7_XANADU2_D	16
#define SPECIAL_DISK_FM7_RIGLAS		17
#define SPECIAL_DISK_FM7_FLEX		18

// d88 constant
#define DISK_BUFFER_SIZE	0x380000	// 3.5MB
#define TRACK_BUFFER_SIZE	0x080000	// 0.5MB

class FILEIO;

class DISK
{
#ifndef _ANY2D88
protected:
	EMU* emu;
#endif
private:
	uint8_t buffer[DISK_BUFFER_SIZE + TRACK_BUFFER_SIZE];
	_TCHAR orig_path[_MAX_PATH];
	_TCHAR dest_path[_MAX_PATH];
	pair32_t file_size;
	int file_bank;
	uint32_t orig_file_size;
	uint32_t orig_crc32;
	bool trim_required;
	
	bool is_d8e_image;
	bool is_1dd_image;
	bool is_solid_image;
	bool is_fdi_image;
	uint8_t fdi_header[4096];
	int solid_ncyl, solid_nside, solid_nsec, solid_size;
	bool solid_mfm;
	int solid_nsec2, solid_size2;
	bool solid_mfm2;
	
	void set_sector_info(uint8_t *t);
	void trim_buffer();
	
	// teledisk image decoder (td0)
	bool teledisk_to_d88(FILEIO *fio);
	
	// imagedisk image decoder (imd)
	bool imagedisk_to_d88(FILEIO *fio);
	
	// cpdread image decoder (dsk)
	bool cpdread_to_d88(FILEIO *fio);
	
	// nfd r0/r1 image decoder (nfd)
	bool nfdr0_to_d88(FILEIO *fio);
	bool nfdr1_to_d88(FILEIO *fio);
	
	// solid image decoder (fdi/hdm/xdf/2d/img/sf7/tfd)
	bool solid_to_d88(FILEIO *fio, int type, int ncyl, int nside, int nsec, int size, bool mfm);
	bool solid_to_d88(FILEIO *fio, int type, int ncyl, int nside, int nsec, int size, bool mfm, int nsec2, int size2, bool mfm2);
	
	// internal routines for track
	bool get_track_tmp(int trk, int side);
	bool make_track_tmp(int trk, int side);
	bool get_sector_tmp(int trk, int side, int index);
	int get_track_num(uint8_t *t);
	uint8_t *get_unstable_sector(uint8_t *t, int index);
	bool get_sector_info_tmp(int trk, int side, int index, uint8_t *c, uint8_t *h, uint8_t *r, uint8_t *n, bool *mfm, int *length);
	bool format_track_tmp(int trk, int side);
	
public:
#ifndef _ANY2D88
	DISK(EMU* parent_emu) : emu(parent_emu)
#else
	DISK()
#endif
	{
		inserted = ejected = write_protected = changed = false;
		is_special_disk = 0;
		file_size.d = 0;
		sector_size.sd = sector_num.sd = 0;
		sector = unstable = NULL;
		drive_type = DRIVE_TYPE_UNK;
		drive_rpm = 0;
		drive_mfm = true;
		track_size = 0;
		static int num = 0;
		drive_num = num++;
		set_device_name(_T("Floppy Disk Drive #%d"), drive_num + 1);
	}
	~DISK()
	{
#ifndef _ANY2D88
		if(inserted) {
			close();
		}
#endif
	}
	
	void open(const _TCHAR* file_path, int bank);
	void close();
#ifdef _ANY2D88
	bool is_d8e()
	{
		return is_d8e_image;
	}
	bool open_as_1dd;
	bool open_as_256;
	void save_as_d88(const _TCHAR* file_path);
#endif
	bool get_track(int trk, int side);
	bool make_track(int trk, int side);
	bool get_sector(int trk, int side, int index);
	bool get_sector_info(int trk, int side, int index, uint8_t *c, uint8_t *h, uint8_t *r, uint8_t *n, bool *mfm, int *length);
	void set_deleted(bool value);
	void set_data_crc_error(bool value);
	void set_data_mark_missing();
	
	bool format_track(int trk, int side);
	void insert_sector(uint8_t c, uint8_t h, uint8_t r, uint8_t n, bool deleted, bool data_crc_error, uint8_t fill_data, int length);
	void sync_buffer();
	
	int get_max_tracks();
	int get_rpm();
	int get_track_size();
	double get_usec_per_track();
	double get_usec_per_bytes(int bytes);
	int get_bytes_per_usec(double usec);
	bool check_media_type();
	
	bool inserted;
	bool ejected;
	bool write_protected;
	bool changed;
	uint8_t media_type;
	bool two_side;
	int is_special_disk;
	
	// track
	uint8_t track[TRACK_BUFFER_SIZE];
	pair32_t sector_num;
	bool track_mfm;
	bool invalid_format;
//	bool no_skew;
	int cur_track, cur_side;
	
	int sync_position[512];
	int am1_position[512];
	int id_position[512];
	int data_position[512];
//	int gap3_size;
	
	// sector
	uint8_t* sector;
	uint8_t* unstable;
	pair32_t sector_size;
	uint8_t id[6];
	bool sector_mfm;
	bool deleted;
	bool addr_crc_error;
	bool data_crc_error;
	
	// drive
	uint8_t drive_type;
	int drive_rpm;
	bool drive_mfm;
	int track_size; // hack for YIS :-(
	int drive_num;
	bool correct_timing()
	{
#ifndef _ANY2D88
#if defined(_FM7) || defined(_FM8) || defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
		if((is_special_disk == SPECIAL_DISK_FM7_TAIYOU1) || (is_special_disk == SPECIAL_DISK_FM7_TAIYOU2)) {
			return true;
		}
#endif
		if(drive_num < (int)array_length(config.correct_disk_timing)) {
			return config.correct_disk_timing[drive_num];
		}
#endif
		return false;
	}
	bool ignore_crc()
	{
#ifndef _ANY2D88
		if(drive_num < (int)array_length(config.ignore_disk_crc)) {
			return config.ignore_disk_crc[drive_num];
		}
#endif
		return false;
	}
	
	// state
	bool process_state(FILEIO* state_fio, bool loading);
	
	// device name
	void set_device_name(const _TCHAR* format, ...)
	{
		if(format != NULL) {
			va_list ap;
			_TCHAR buffer[1024];
			
			va_start(ap, format);
			my_vstprintf_s(buffer, 1024, format, ap);
			va_end(ap);
			
			my_tcscpy_s(this_device_name, 128, buffer);
#ifdef _USE_QT
//			emu->get_osd()->set_vm_node(this_device_id, buffer);
#endif
		}
	}
	const _TCHAR *get_device_name()
	{
		return (const _TCHAR *)this_device_name;
	}
	_TCHAR this_device_name[128];
};

#endif

