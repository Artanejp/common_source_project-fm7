/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.16-

	[ d88 image handler ]
*/

#ifndef _DISK_H_
#define _DISK_H_

#include "vm.h"
#include "../emu.h"

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

#define SPECIAL_DISK_X1_ALPHA	1
#define SPECIAL_DISK_X1_BATTEN	2

// d88 constant
#define DISK_BUFFER_SIZE	0x380000	// 3.5MB
#define TRACK_BUFFER_SIZE	0x080000	// 0.5MB

// teledisk decoder constant
#define STRING_BUFFER_SIZE	4096
#define LOOKAHEAD_BUFFER_SIZE	60
#define THRESHOLD		2
#define N_CHAR			(256 - THRESHOLD + LOOKAHEAD_BUFFER_SIZE)
#define TABLE_SIZE		(N_CHAR * 2 - 1)
#define ROOT_POSITION		(TABLE_SIZE - 1)
#define MAX_FREQ		0x8000

class FILEIO;

class DISK
{
protected:
	EMU* emu;
private:
	FILEIO* fi;
	uint8 buffer[DISK_BUFFER_SIZE + TRACK_BUFFER_SIZE];
	_TCHAR orig_path[_MAX_PATH];
	_TCHAR dest_path[_MAX_PATH];
	_TCHAR temp_path[_MAX_PATH];
	pair file_size;
	int file_bank;
	uint32 crc32;
	bool trim_required;
	bool temporary;
	uint8 fdi_header[4096];
	
	void set_sector_info(uint8 *t);
	void trim_buffer();
	
	// teledisk image decoder (td0)
	bool teledisk_to_d88();
	int next_word();
	int get_bit();
	int get_byte();
	void start_huff();
	void reconst();
	void update(int c);
	short decode_char();
	short decode_position();
	void init_decode();
	int decode(uint8 *buf, int len);
	
	// imagedisk image decoder (imd)
	bool imagedisk_to_d88();
	
	// cpdread image decoder (dsk)
	bool cpdread_to_d88(int extended);
	
	// standard image decoder (fdi/tfd/2d/sf7)
	bool standard_to_d88(int type, int ncyl, int nside, int nsec, int size);
	
	uint8 text_buf[STRING_BUFFER_SIZE + LOOKAHEAD_BUFFER_SIZE - 1];
	uint16 ptr;
	uint16 bufcnt, bufndx, bufpos;
	uint16 ibufcnt,ibufndx;
	uint8 inbuf[512];
	uint16 freq[TABLE_SIZE + 1];
	short prnt[TABLE_SIZE + N_CHAR];
	short son[TABLE_SIZE];
	uint16 getbuf;
	uint8 getlen;
	
	typedef struct {
		char sig[3];
		uint8 unknown;
		uint8 ver;
		uint8 dens;
		uint8 type;
		uint8 flag;
		uint8 dos;
		uint8 sides;
		uint16 crc;
	} td_hdr_t;
	typedef struct {
		uint16 crc;
		uint16 len;
		uint8 ymd[3];
		uint8 hms[3];
	} td_cmt_t;
	typedef struct {
		uint8 nsec, trk, head;
		uint8 crc;
	} td_trk_t;
	typedef struct {
		uint8 c, h, r, n;
		uint8 ctrl, crc;
	} td_sct_t;
	typedef struct {
		uint8 mode;
		uint8 cyl;
		uint8 head;
		uint8 nsec;
		uint8 size;
	} imd_trk_t;
	typedef struct {
		char title[17];
		uint8 rsrv[9];
		uint8 protect;
		uint8 type;
		uint32 size;
		uint32 trkptr[164];
	} d88_hdr_t;
	typedef struct {
		uint8 c, h, r, n;
		uint16 nsec;
		uint8 dens, del, stat;
		uint8 rsrv[5];
		uint16 size;
	} d88_sct_t;
public:
	DISK(EMU* parent_emu) : emu(parent_emu)
	{
		inserted = ejected = write_protected = changed = false;
		file_size.d = 0;
		sector_size.sd = sector_num.sd = 0;
		sector = NULL;
		drive_type = DRIVE_TYPE_UNK;
		drive_rpm = 0;
		drive_mfm = true;
		static int num = 0;
		drive_num = num++;
	}
	~DISK()
	{
		if(inserted) {
			close();
		}
	}
	
	void open(_TCHAR path[], int bank);
	void close();
	bool get_track(int trk, int side);
	bool make_track(int trk, int side);
	bool get_sector(int trk, int side, int index);
	void set_deleted(bool value);
	void set_crc_error(bool value);
	
	bool format_track(int trk, int side);
	void insert_sector(uint8 c, uint8 h, uint8 r, uint8 n, bool deleted, bool crc_error, uint8 fill_data, int length);
	void sync_buffer();
	
	int get_rpm();
	int get_track_size();
	double get_usec_per_bytes(int bytes);
	bool check_media_type();
	
	bool inserted;
	bool ejected;
	bool write_protected;
	bool changed;
	uint8 media_type;
	bool is_standard_image;
	bool is_fdi_image;
	int is_special_disk;
	
	// track
	uint8 track[TRACK_BUFFER_SIZE];
	pair sector_num;
	bool invalid_format;
	bool no_skew;
	int cur_track, cur_side;
	
	int sync_position[256];
	int id_position[256];
	int data_position[256];
	
	// sector
	uint8* sector;
	pair sector_size;
	uint8 id[6];
	uint8 density;
	bool deleted;
	bool crc_error;
	
	// drive
	uint8 drive_type;
	int drive_rpm;
	bool drive_mfm;
	int drive_num;
	
	// state
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

#endif

