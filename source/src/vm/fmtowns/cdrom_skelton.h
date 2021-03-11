#pragma once

/*!
 * @file cdrom_skelton.h
 * @brief Skelton definitions of new CD-ROM class for eFM-Towns.
 * @author K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * @date 2021-03-04
 * @copyright GPLv2
 */
#include <stdlib.h>
#include "../../fileio.h"

/*!
 * @enum Image type of current virtual CD image.
 */
typedef enum CDIMAGE_TYPE {
	IMAGETYPE_NONE = 0,
	IMAGETYPE_ISO,
	IMAGETYPE_CUE,
	IMAGETYPE_CCD
}
/*!
 * @enum Track type of opening virtual CD image.
 */
typedef enum CDIMAGE_TRACK_TYPE {
	TRACKTYPE_NONE = 0,   //!< NONE OPENING TYPE
	TRACKTYPE_AUDIO,      //!< Open as audio
	TRACKTYPE_MODE1_2048, //!< MODE1/2048
	TRACKTYPE_MODE1_2352, //!< MODE1/2352
	TRACKTYPE_MODE1_ISO,  //!< MODE1/ISO
	TRACKTYPE_MODE2_2336, //!< MODE2/2336
	TRACKTYPE_MODE2_2352, //!< MODE2/2352
	TRACKTYPE_CDI_2336,   //!< CD-I/2336
	TRACKTYPE_CDI_2352,   //!< CD-I/2352
	TRACKTYPE_CDG,        //!< CD/G
}

/*!
 * @enum Opening modes of virtual CD image.
 */
typedef enum CDIMAGE_OPEN_MODE {
	OPENMODE_AUDIO, //!< AUDIO
	OPENMODE_MODE1, //!< MODE1
	OPENMODE_MODE2, //!< MODE2
	OPENMODE_CDI,   //!< CD-I
	OPENMODE_CDG    //!< CD/G
}


/*!
 * @note Belows are CD-ROM sector structuer.
 * @note See https://en.wikipedia.org/wiki/CD-ROM#Sector_structure .
 */

#pragma pack(1)
/*!
 * @struct definition of SUBC field of CD-ROM.
 */
typedef union {
	struct {
		uint8_t P:1;
		uint8_t Q:1;
		uint8_t R:1;
		uint8_t S:1;
		uint8_t T:1;
		uint8_t U:1;
		uint8_t V:1;
		uint8_t W:1;
	} bit;
	uint8_t byte;
} cdrom_SUBC_t;
#pragma pack()

#pragma pack(1)
/*!
 * @struct definition of CD-ROM data header (excepts audio track).
 */
typedef struct {
	uint8_t sync[12];
	uint8_t addr_m;
	uint8_t addr_s;
	uint8_t addr_f;
	uint8_t sector_type; //! 1 = MODE1, 2=MODE2
} cdrom_data_head_t;
#pragma pack()

#pragma pack(1)
/*!
 * @struct definition of CD-ROM MODE1 sector struct (excepts ISO image).
 * @note ToDo: Still not implement crc32 and ecc.
 * @note 20201116 K.O
 */
typedef struct {
	cdrom_data_head_t header;
	uint8_t data[2048];
	uint8_t crc32[4]; //! CRC32 checksum.
	uint8_t reserved[8];
	uint8_t ecc[276]; //! ERROR CORRECTIOM DATA; by read solomon code.
} cdrom_data_mode1_t;
#pragma pack()

#pragma pack(1)
/*!
 * @struct definition of CD-ROM MODE2 sector struct (excepts ISO image).
 */
typedef struct {
	cd_data_head_t header;
	uint8_t data[2336];
} cdrom_data_mode2_t;
#pragma pack()

#pragma pack(1)
/*!
 * @struct definition of CD-DA sector struct (excepts ISO image).
 */
typedef struct {
	uint8_t data[2352];
} cdrom_audio_sector_t;
#pragma pack()

#pragma pack(1)
/*!
 * @struct definition of DATA field for ISO.
 * @note ToDo: Add fake header and crc and ecc.
 * @note 20201116 K.O
 */
typedef struct {
	uint8_t data[2048];
} cdrom_iso_data_t;
#pragma pack()

#include <cmath>
#include <string>

/*!
 * @struct definition of track table.
 * @note 20210311 K.O
 */
typedef struct {
	bool available;	             //!< indicate this track is available.
	uint8_t type;	             //!< track type (enum CDIMAGE_TRACK_TYPE)
	int64_t pregap;              //!< pregap value
	int64_t absolute_lba;        //!< absolute lba position.
	int64_t lba_offset;          //!< LBA offset Within a image.
	int64_t lba_size;            //!< LBA size of track.
	int64_t index0;              //!< INDEX0 (relative)
	int64_t index1;              //!< INDEX1 (relative)
	uint32_t physical_size;      //!< Physical sector size
	uint32_t real_physical_size; //!< Real physical sector size
	uint32_t logial_size;        //!< Logical sector size
	_TCHAR filename[_MAX_PATH];  //!< Image file name.
} cdrom_toc_table_t;

/*! 
 * @class BASIC class of CD Image.
 * @note You must add list of tracks.
 */
#define CDIMAGE_META_STATE_VERSION 1

class DLL_PREFIX CDIMAGE_META {
protected:
	FILEIO* current_fio;
	FILEIO* sheet_fio;
	
	uint8_t type; //!< enum CDIMAGE_TYPE 
	uint8_t tracks;	
	uint8_t tracktype; //!< enum CDIMAGE_TRACK_TYPE 
	uint8_t openmode; //!< enum CDIMAGE_OPENMODE 
	
	cdrom_toc_table_t toc_table[102];
	uint32_t logical_bytes_per_block;
	uint32_t physical_bytes_per_block;
	uint32_t real_physical_bytes_per_block;
	int64_t max_blocks;
	bool allow_beyond_track; //!< allow over track reading.

	double transfer_time_us;
	double seek_speed;
	/*!
	 * @note belows are status value.
	 * Not table values.
	 */
	bool track_is_available;
	uint8_t now_track; //! @note 00 - 99.
	int64_t now_lba;
	uint32_t offset_in_sector;
	int64_t bytes_position;
	int64_t lba_offset_of_this_track;
	int64_t sectors_of_this_track;
	int64_t pregap_of_this_track;
	std::string __filename;

	/*!
	 * @brief Parse CUE/CCD sheet, check track data and construct tracks table.
	 * @return true if succeeded.
	 * @note Must open sheet file before using.
	 */
	virtual bool parse_sheet()
	{
		if(sheet_fio == nullptr) return false;
		if(!(sheet_fio->IsOpened())) {
			return false;
		}
		if(current_fio != nullptr) {
			if(current_fio->IsOpened()) {
				current_fio->Fclose();
			}
			delete current_fio;
			current_fio = NULL;
		}
		return true;
	}
	/*!
	 * @brief Load / Save state(TOC table part) to VM.
	 * @param state_fio FILE IO for state loading/saving.
	 * @param loading If true loading, false is saving.
	 * @return true if succeeded.
	 */
	virtual bool load_save_toc_table(FILEIO* state_fio, bool loading)
	{
		uint8_t ntracks = 0;
		if(loading) {
			ntracks = state_fio->FgetUint8();
			if(ntracks >= 101) return false;
		} else {
			ntracks = tracks;
			if(ntracks >= 101) return false;
			state_fio->FputUint8(tracks);
		}
		uint8_t n = 0;
		for(int i = 1; i <= ntracks; i++) {
			if(loading) {
				n = state_fio->FgetUint8(); //!< Get track number
				if(n != i) return false; //!< Error when wrong filename.
			} else {
				state_fio->FputUint8((uint8_t)i);
			}
			state_fio->StateValue(toc_table[i].available);
			state_fio->StateValue(toc_table[i].type);
			state_fio->StateValue(toc_table[i].pregap);
			state_fio->StateValue(toc_table[i].absolute_lba);
			state_fio->StateValue(toc_table[i].lba_offset);
			state_fio->StateValue(toc_table[i].lba_size);
			state_fio->StateValue(toc_table[i].index0);
			state_fio->StateValue(toc_table[i].index1);
			state_fio->StateValue(toc_table[i].physical_size);
			state_fio->StateValue(toc_table[i].real_physical_size);
			state_fio->StateValue(toc_table[i].logical_size);
			state_fio->StateArray(toc_table[i].filename, _MAX_PATH, 1);
		}
		if(loading) {
			tracks = ntracks;
			/*
			 * clear TRACK 00.
			 */
			init_toc_table(0);
			if(tracks > 1) {
				toc_table[0].available = true;
			} else {
				toc_table[0].available = false;
			}
		}
		return true;
	}
	
	/*!
	 * @brief Load / Save state(main part) to VM.
	 * @param state_fio FILE IO for state loading/saving.
	 * @param loading If true loading, false is saving.
	 * @return true if succeeded.
	 */
	virtual bool load_save_params(FILEIO* state_fio, bool loading)
	{
		if(!state_fio->StateCheckUint32(CDIMAGE_META_STATE_VERSION)) {
			return false;
		}
		// device id is not exists.
		state_fio->StateValue(type);
		state_fio->StateValue(tracks);
		state_fio->StateValue(tracktype);
		state_fio->StateValue(openmode);
		state_fio->StateValue(logical_bytes_per_block);
		state_fio->StateValue(physical_bytes_per_block);
		state_fio->StateValue(real_physical_bytes_per_block);
		state_fio->StateValue(max_blocks);

		state_fio->StateValue(transfer_time_us);
		state_fio->StateValue(seek_speed);

		state_fio->StateValue(allow_beyond_track);

		state_fio->StateValue(track_is_available);
		state_fio->StateValue(now_track);
		state_fio->StateValue(now_lba);
		state_fio->StateValue(bytes_position);
		state_fio->StateValue(offset_in_sector);
		state_fio->StateValue(lba_offset_of_this_track);
		state_fio->StateValue(sectors_of_this_track);
		state_fio->StateValue(pregap_of_this_track);

		int strsize = 0;
		_TCHAR _l[_MAX_PATH] = {0};
		if(loading) {
			strsize = state_fio->FgetInt32_LE();
			if(strsize < 0) return false;
			
			__filename.clear();
			state_fio->StateArray(_l, _MAX_PATH, 1);
			__filename.assign(_l, _MAX_PATH - 1);
		} else {
			strsize = __filename.length();
			if(strsize < 0) strsize = 0;
			state_fio->FputInt32_LE(strsize);
			__filename.copy(_l, _MAX_PATH - 1);
			state_fio->StateArray(_l, _MAX_PATH, 1);
		}
		return true;
	}
			
	/*!
	 * @brief Initialize TOC table..
	 * @param num track number.
	 */
	void init_toc_table(uint8_t num)
	{
		if(num > 101) return;
		
		toc_table[num].available = false;
		toc_table[num].type = TRACKTYPE_NONE;
		toc_table[num].pregap = 0;
		toc_table[num].absolute_lba = 0;
		toc_table[num].lba_offset = 0;
		toc_table[num].lba_size = 0;
		toc_table[num].index0 = 0;
		toc_table[num].index1 = 0;
		toc_table[num].physical_size = 0;
		memset(toc_table[num].filename, 0x00, sizeof(_TCHAR) * _MAX_PATH);
	}
	/*!
	 * @brief Seek assigned position in track.
	 * @param lba *Relative* LBA in this track.
	 * @return true if success.
	 */
	virtual bool seek_in_track(int64_t lba)
	{
		if(lba < 0) return false;
		if((now_track < 0) || (now_track >= 100)) return false;
		
		int64_t _lba_bak = lba + lba_offset_of_this_track;
		if(current_fio == nullptr) return true;
		
		int64_t lba_max = sectors_of_this_track + lba_offset_of_this_track;
		int64_t lba_min = lba_offset_of_this_track;
		if(lba_min < 0) lba_min = 0;
		if(lba_max < 0) lba_max = 0;
		lba = lba + lba_offset_of_this_track;
		
		if(lba >= lba_max) return false;
		if(lba < lba_min) return false;
		if(pregap_of_this_track < 0) return false; // Illegal PREGAP
		
		lba = lba - pregap_of_this_track;
		if(lba < 0) lba = 0;
		
		int64_t offset = lba * get_real_physical_block_size();
		
		if(offset >= LONG_MAX) {
			bytes_position = 0;
			offset_in_sector = 0;
			return false;
		}
		if(current_fio->Fseek(offset, FILEIO_SEEK_SET) != 0) {
			bytes_position = 0;
			offset_in_sector = 0;
			return false;
		}
		now_lba = _lba_bak;
		bytes_position = offset;
		offset_in_sector = 0;
		return true;
	}
	/*!
	 * @brief Get image data of track.
	 * @param track track number.
	 * return true if success.
	 */
	virtual bool get_track_image(uint8_t track)
	{
		bool result = false;
		if(track >= tracks) track = tracks;
		/*
		 * Set default values
		 */
		tracktype = TRACKTYPE_NONE;
		logical_bytes_per_block = 2048;
		physical_bytes_per_block = 2352;
		real_physical_bytes_per_block = 2352;
		track_is_available = false;
		now_lba = 0;
		offset_in_sector = 0;
		bytes_position = 0;
		lba_offset_of_this_track = 0;

		sectors_of_this_track = 0;
		pregap_of_this_track = 150;
		
		__filename.erase();
		
		if(current_fio != nullptr) {
			if(current_fio->IsOpened()) {
				current_fio->Fclose();
			}
			delete curent_fio;
			current_fio = NULL;
		}
		if(track >= 100) {
			return false;
		}
		
		if(now_track != track) {
			now_track = track;
		}
		if(!(toc_table[track].available)) {
			return false;
		}
		if(strlen(toc_table[track].filename) > 0) {
			current_fio = new FILEIO();
			if(current_fio != nullptr) {
				result = current_fio->Fopen(toc_table[track].filename, FILEIO_READ_BINARY);
			}
			if(result) {
				__filename.assign(toc_table[track].filename, _MAX_PATH);
			}
		}
		if(result) {
			tracktype = toc_table[now_track].type;
			logical_bytes_per_block = toc_table[now_track].logical_size;
			physical_bytes_per_block = toc_table[now_track].physical_size;
			real_physical_bytes_per_block = toc_table[now_track].real_physical_size;
			track_is_available = toc_table[now_track].available;
			now_lba = toc_table[now_track].lba_offset;
			offset_in_sector = 0;
			bytes_position = 0;
			lba_offset_of_this_track = toc_table[now_track].lba_offset;
			sectors_of_this_track = toc_table[now_track].lba_size;
			pregap_of_this_track = toc_table[now_track].pregap;
		}
		return false;
	}
	/*!
	 * @brief reset FILEIOs for sheet and image.
	 */
	virtual void reset_sheet_fio()
	{
		if(current_fio != nullptr) {
			if(current_fio->IsOpened()) {
				current_fio->Fclose();
			}
			delete current_fio;
			current_fio = NULL;
		}
		if(sheet_fio != nullptr) {
			if(sheet_fio->IsOpened()) {
				sheet_fio->Fclose();
			}
			delete sheet_fio;
			sheet_fio = NULL;
		}
	}
public:
	/*!
	 * @brief constructor
	 */
	CDIMAGE_META()
	{
		max_blocks = 0;
		tracks = 0;
		type = IMAGETYPE_NONE;
		tracktype = TRACKTYPE_NONE;
		openmode = OPENMODE_AUTO;
		logical_bytes_per_block = 2048;
		physical_bytes_per_block = 2352;
		real_physical_bytes_per_block = 2352; //!< 2048 if MODE1/ISO.
		allow_beyond_track = false;
		transfer_rate_us = 1.0e6 / 150.0e3; //!< 1.0x 150s

		seek_speed = 1.0;
		
		now_lba = 0;
		now_track = 0;
		offset_in_sector = 0;
		lba_offset_of_this_track = 0;
		sectors_of_this_track = 0;
		pregap_of_this_track = 150;
		track_is_available = false;
		bytes_position = 0;
		__filename.erase();
		for(int t = 0; t < 102; i++) {
			init_toc_table(t);
		}
		
		current_fio = NULL;
		sheet_fio = NULL;
	}
	/*!
	 * @brief de-constructor
	 * @note Please implement de-allocating tracks list 
	 * in de-constructor if you need.
	 */
	~CDIMAGE_META()
	{
		reset_sheet_fio();
	}
	/*!
	 * @brief Get track position now accessing.
	 * @return track value.-1 if not avaiable image.
	 */
	int get_track() const
	{
		return now_track;
	}
	/*!
	 * @brief Get LBA position of now accessing.
	 * @return LBA position of now accessing.
	 */
	int64_t get_lba() const
	{
		return now_lba;
	}
	/*!
	 * @brief Get Relative LBA offset value (at head of this track) of this image.
	 * @return Relative LBA offset value (in image).
	 */
	int64_t get_lba_offset() const
	{
		return lba_offset_of_this_track;
	}
	/*!
	 * @brief Get number of sectors at this track.
	 * @return Number of sectors at this track.
	 */
	int64_t get_sectors_of_this_track() const
	{
		return sectors_of_this_track;
	}
	/*!
	 * @brief Get current position-offset in this sector.
	 * @return Offset position.
	 */
	int offset_of_this_sector() const
	{
		return (int)offset_in_sector;
	}
	/*!
	 * @brief Whether this track is available.
	 * @return true if available.
	 */
	bool is_available() const
	{
		return track_is_available;
	}
	/*!
	 * @brief Get blocks of this virtual CD.
	 * @return Blocks (sectors) of this virtual CD.
	 */
	int64_t  get_blocks() const
	{
		return max_blocks;
	}
	/*!
	 * @brief Get physical block size of this virtual CD.
	 * @return Physical block size of this virtual CD.
	 */
	uint32_t  physical_block_size() const
	{
		return physical_bytes_per_block;
	}
	/*!
	 * @brief Get REAL (in VIRTUAL IMAGE) physical block size of this virtual CD.
	 * @return Physical block size of this virtual CD.
	 */
	uint32_t  real_physical_block_size() const
	{
		return real_physical_bytes_per_block;
	}
	/*!
	 * @brief Get logical block size of this virtual CD.
	 * @return Logical block size of this virtual CD.
	 */
	uint32_t logical_block_size() const
	{
		return logical_bytes_per_block;
	}
	/*!
	 * @brief Get image type of this virtual CD.
	 * @return Image type of virtual CD.
	 */
	enum CDIMAGE_TYPE get_type() const
	{
		return type;
	}
	/*!
	 * @brief Set enable/disable beyond track reading.
	 * @param val enable when setting true.
	 */
	void enable_beyond_track_reading(bool val)
	{
		allow_beyond_track = val;
	}
	/*!
	 * @brief Get full path of this virtual image.
	 * @param var Returned full path of opened file.Erase if not opened.
	 * @return true if already opened.
	 */
	bool get_file_name(std::string& var)
	{
		if(__filename.empty()) {
			var.erase();
			return false;
		}
		var = __filename;
		return true;
	}
	/*!
	 * @brief Check type of virtual disc image by filename.
	 * @param filename Filename of image (absolute path).
	 * @return Type of CD image.
	 */
	static enum CDIMAGE_TYPE check_type(_TCHAR *filename)
	{
		if(filename == nullptr) {
			return IMAGETYPE_NONE;
		}
		if(!(FILEIO::IsFileExisting(filename))) {
			return IMAGETYPE_NONE;
		}
		if(check_file_extension(filename, _T(".gz"))) {
			if(check_file_extension(filename, _T(".iso.gz"))) {
				return IMAGETYPE_ISO;
			}
			if(check_file_extension(filename, _T(".cue.gz"))) {
				return IMAGETYPE_CUE;
			}
			if(check_file_extension(filename, _T(".ccd.gz"))) {
				return IMAGETYPE_CCD;
			}
		} else {
			if(check_file_extension(filename, _T(".iso"))) {
				return IMAGETYPE_ISO;
			}
			if(check_file_extension(filename, _T(".cue"))) {
				return IMAGETYPE_CUE;
			}
			if(check_file_extension(filename, _T(".ccd"))) {
				return IMAGETYPE_CCD;
			}
		}
		return IMAGETYPE_NONE; // Write Unique routines to next.
	}
	
	/*!
	 * @brief Open virtual disc image.
	 * @param filename Filename of image (absolute path).
	 * @param req_type Opening mode.
	 * @return true if succeeded.
	 *
	 * @note Must set disc parameters list only at this function (or sub-functions).
	 * @see parse_sheet
	 * @see check_type
	 */
	virtual bool open(_TCHAR *filename, enum CDIMAGE_OPEN_MODE req_type)
	{
		close();
		if(FILEIO::IsFileExisting(filename)) {
			sheet_fio = new FILEIO();
			if(sheet_fio != nullptr) {
				if(sheet_fio->Fopen(filename, FILEIO_READ_BINARY)) {
					return parse_sheet();
				}
			}
		}
		return false;
	}
	/*!
	 * @brief Close virtual disc image.
	 * @return true if succeeded.
	 */
	virtual bool close()
	{
		reset_sheet_fio();
		for(uint8_t num = 0; num < 102; num++) {
			init_toc_table(num);
		}
		
		type = IMAGETYPE_NODE;
		tracktype = TRACKTYPE_NONE;
		logical_bytes_per_block = 2048;
		physical_bytes_per_block = 2352;
		real_physical_bytes_per_block = 2352;
		track_is_available = false;
		now_lba = 0;
		offset_in_sector = 0;
		bytes_position = 0;
		lba_offset_of_this_track = 0;

		sectors_of_this_track = 0;
		pregap_of_this_track = 150;
		__filename.erase();

		return true;
	}

	/*!
	 * @brief Read image data to buffer as CD-ROM/MODE1 from current LBA position.
	 * @param buf Destination pointer of read buffer.
	 * @param buflen Size of read buffer.
	 * @param sectors Count of sectors (LBAs).
	 * @param _clear true if expect to clear buffer.
	 * @return size of reading.
	 * @note Override and inherit this to implement real method.
	 * @note Stop when reaches END of CURRENT TRACK.
	 */
	virtual ssize_t read_mode1(uint8_t *buf, ssize_t buflen, size_t sectors = 1, bool _clear = false)
	{
		if((now_track == 0) || (now_track >= 100)) return -1;
		if(now_track >= tracks) return -1;
		
		if(buf == nullptr) return -1;
		if(buflen <= 0) return -1;
		if(sectors <= 0) return -1;

		int64_t logical_size = (int64_t)(sectors * logical_bytes_per_block);
		if(logical_size >= buflen) logical_size = buflen;

		if(logical_size <= 0) return -1;
		if(_clear) {
			memset(buf, 0x00, logical_size);
		}

		return logical_size;
	}
	/*!
	 * @brief Read image data to buffer as CD-ROM/MODE2 from current LBA position.
	 * @param buf Destination pointer of read buffer.
	 * @param buflen Size of read buffer.
	 * @param sectors Count of sectors (LBAs).
	 * @param _clear true if expect to clear buffer.
	 * @return size of reading.
	 * @note Override and inherit this to implement real method.
	 * @note Stop when reaches END of CURRENT TRACK.
	 * @note Changing size of data by type of Virtual image.
	 */
	virtual ssize_t read_mode2(uint8_t *buf, ssize_t buflen, size_t sectors = 1, bool _clear = false)
	{
		if((now_track == 0) || (now_track >= 100)) return -1;
		if(now_track >= tracks) return -1;
		
		if(buf == nullptr) return -1;
		if(buflen <= 0) return -1;
		if(sectors <= 0) return -1;

		int64_t logical_size = (int64_t)(sectors * logical_bytes_per_block);
		if(logiacal_size >= buflen) logical_size = buflen;

		if(physical_size <= 0) return -1;
		if(_clear) {
			memset(buf, 0x00, logical_size);
		}

		return logical_size;
	}
	/*!
	 * @brief Read image data to buffer as CD-DA from current LBA position.
	 * @param buf Destination pointer of read buffer.
	 * @param buflen Size of read buffer.
	 * @param sectors Count of sectors (LBAs).
	 * @param _clear true if expect to clear buffer.
	 * @return size of reading.
	 * @note Override and inherit this to implement real method.
	 * @note Stop when reaches END of CURRENT TRACK.
	 * @note Changing size of data by type of Virtual image.
	 */
	virtual ssize_t read_cdda(uint8_t *buf, ssize_t buflen, size_t sectors = 1, bool _clear = false)
	{
		if((now_track == 0) || (now_track >= 100)) return -1;
		if(now_track >= tracks) return -1;
		
		if(buf == nullptr) return -1;
		if(buflen <= 0) return -1;
		if(sectors <= 0) return -1;

		int64_t physical_size = (int64_t)(sectors * physical_bytes_per_block);
		if(physiacal_size >= buflen) physical_size = buflen;

		if(physical_size <= 0) return -1;
		if(_clear) {
			memset(buf, 0x00, physical_size);
		}

		return physical_size;
	}
	
	/*!
	 * @brief Read raw image data to buffer from current LBA position.
	 * @param buf Destination pointer of read buffer.
	 * @param buflen Size of read buffer.
	 * @param sectors Count of sectors (LBAs).
	 * @param _clear true if expect to clear buffer.
	 * @return size of reading.
	 * @note Override and inherit this to implement real method.
	 * @note Stop when reaches END of CURRENT TRACK.
	 * @note Changing size of data by type of Virtual image.
	 */
	virtual ssize_t read_raw(uint8_t *buf, ssize_t buflen, size_t sectors = 1, bool _clear = false)
	{
		if((now_track == 0) || (now_track >= 100)) return -1;
		if(now_track >= tracks) return -1;
		
		if(buf == nullptr) return -1;
		if(buflen <= 0) return -1;
		if(sectors <= 0) return -1;

		int64_t physical_size = (int64_t)(sectors * physical_bytes_per_block);
		if(physical_size >= buflen) physical_size = buflen;

		if(physical_size <= 0) return -1;
		if(_clear) {
			memset(buf, 0x00, physical_size);
		}

		return physical_size;
	}
	/*!
	 * @brief Try to seek to expected LBA.
	 * @param m minutes of LBA (absolute)
	 * @param s seconds of LBA (absolute)
	 * @param f frames  of LBA (absolute)
	 * @param in_track Set true if within track, set false if seek to another track.
	 * @return true if succeeded.
	 * @note need to implement accross another tracks.
	 */	
	virtual bool seek(uint8_t m, uint8_t s, uint8_t f, bool& in_track)
	{
		int64_t lba = msf_to_lba(m, s, f);
		return seek_absolute_lba(lba);
	}
	/*!
	 * @brief Try to seek to expected LBA.
	 * @param lba Position of LBA (absolute)
	 * @param in_track Set true if within track, set false if seek to another track.
	 * @return true if succeeded.
	 * @note need to implement accross another tracks.
	 */	
	virtual bool seek_absolute_lba(int64_t lba, bool& in_track)
	{
		if(lba >= max_blocks) {
			lba = max_blocks;
		}
		if(lba < 0) {
			lba = 0;
		}
		uint8_t trk = 0;
		for(int i = 0; i < tracks; i++) {
			if(toc_table[trk + 1].absolute_lba > lba) break;
			trk++;
		}
		if(trk >= tracks) {
			trk = (tracks > 0) ? (tracks - 1) : 0;
		}
		if((trk == 0) || (!(toc_table[trk].available))) {
			if(now_track != 0) {
				in_track = false;
				if(!(allow_beyond_track)) {
					return false;
				}
			}
			get_track_image(0);
			return seek_in_track(0);
		}
		if(trk != now_track) {
			in_track = false;
			now_track = trk;
			if(allow_beyond_track) {
				if(!(get_track_image(now_trk))) {  // Seek inner.
					seek_in_track(0);
					return false;
				}
			} else {
				get_track_image(0);
				seek_in_track(0);
				return false;
			}
		} else {
			in_track = true;
		}
		lba = lba - toc_table[now_track].lba_offset;
		if(lba <= 0) {
			lba = 0;
		}
		return seek_in_track(lba);
	}
	/*!
	 * @brief Try to seek to expected LBA.
	 * @param lba Position of LBA (relative)
	 * @param in_track Set true if within track, set false if seek to another track.
	 * @return true if succeeded.
	 * @note need to implement accross another tracks.
	 */	
	virtual bool seek_relative_lba(int64_t lba, bool& in_track)
	{
		int64_t __lba = lba + now_lba;
		return seek_absolute_lba(__lba, in_track);
	}
	/*!
	 * @brief Set seek speed.
	 * @param usec Basic transfer time normally 1.0 / 150.0KHz.
	 */
	virtual void set_transfer_time_us(double usec)
	{
		if(usec <= 0.0) {
			usec = 1.0e6 / 150.0e3;
		}
		transfer_time_us = usec;
		if(transfer_time_us <= (1.0 / 32.0)) {
			transfer_time_us = 1.0 / 32.0;
		}
	}
	/*!
	 * @brief Get transfer time per byte.
	 * @return transfer time as uSec.
	 */
	inline double get_transfer_time_us()
	{
		double _speed = seek_speed;
		if(_speed <= 0.0) _speed = 1.0;
		
		return (transfer_time_us / _speed);
	}
	/*!
	 * @brief Get seek multiply rate.
	 * @return Seek rate.
	 */
	inline double get_seek_speed()
	{
		double _speed = seek_speed;
		if(_speed <= 0.0) {
			_speed = 1.0;
		}
		return _speed;
	}
	/*!
	 * @brief Get seek time per block.
	 * @return seek time as uSec.
	 */
	inline double get_single_seek_time_us()
	{
		double bytes = (double)physical_bytes_per_block;
		if(bytes < 1.0) bytes = 1.0;
		double usec = get_transfer_time_us();

		if(usec < (1.0 / 32.0)) usec = 1.0 / 32.0;
		return usec * bytes;
	}
	/*!
	 * @brief Set seek speed.
	 * @param speed Transfer speed multiply rate, normally 1.0.
	 */
	virtual void set_seek_speed(double speed)
	{
		if(speed <= 0.0) {
			speed = 1.0;
		}
		seek_speed = speed;
	}
	/*!
	 * @brief Set physical bytes per block (in emulation).
	 * @param bytes bytes per block.
	 */
	virtual void set_physical_bytes_per_block(uint32_t bytes)
	{
		if(bytes == 0) bytes = 2352; // Default value
		physical_bytes_per_block = bytes;
	}
	/*!
	 * @brief Set physical bytes per block (in image).
	 * @param bytes bytes per block.
	 */
	virtual void set_real_physical_bytes_per_block(uint32_t bytes)
	{
		if(bytes == 0) bytes = 2352; // Default value
		real_physical_bytes_per_block = bytes;
	}
	/*!
	 * @brief Set logical bytes per block (in emulation).
	 * @param bytes bytes per block.
	 */
	virtual void set_logical_bytes_per_block(uint32_t bytes)
	{
		if(bytes == 0) bytes = 2048; // Default value
		logical_bytes_per_block = bytes;
	}
	/*!
	 * @brief Calculate seek time to expected LBA.
	 * @param m minutes of LBA (absolute)
	 * @param s seconds of LBA (absolute)
	 * @param f frames  of LBA (absolute)
	 * @return seek time as usec.
	 * If error, return NaN. 
	 */	
	virtual double get_seek_time(uint8_t m, uint8_t s, uint8_t f)
	{
		int64_t lba = msf_to_lba(m, s, f);
		return get_seek_time_absolute_lba(lba);
	}
	/*!
	 * @brief Calculate seek time to expected LBA.
	 * @param lba Position of LBA (absolute)
	 * @return seek time as usec.
	 * If error, return NaN. 
	 */	
	virtual double get_seek_time_absolute_lba(int64_t lba)
	{
		if(lba < 0) {
			return std::nan();
		} else {
			int64_t step = (int64_t)(llabs(lba - now_lba));
			if(step > max_blocks) step = max_blocks;
			
			double _t = (get_single_seek_time_us() * ((double)step));
			return _t;
		}
	}
	/*!
	 * @brief Calculate seek time to expected LBA.
	 * @param lba Position of LBA (relative)
	 * @return seek time as usec.
	 * If error, return NaN. 
	 */	
	virtual double get_seek_time_relative_lba(int64_t lba)
	{
		int64_t lba2 = llabs(lba);
		if(lba2 > max_blocks) lba2 = max_blocks;

		double _t = (get_single_seek_time_us() * ((double)lba2));
		return _t;
	}
	/*!
	 * @brief Convert BCD value to binary value.
	 * @param n BCD value
	 * @return Binarized value.-1 if error.
	 */
	inline int bcd_to_bin(uint8_t n)
	{
		uint8_t n1 = n >> 8;
		uint8_t n2 = n & 0x0f;
		if(n1 >= 10) return -1;
		if(n2 >= 10) return -1;
		return (((int)(n1 * 10)) + (int)n2);
	}
	/*!
	 * @brief Convert POSITIVE BINARY value to BCD.
	 * @param n Binary value.
	 * @param _error Set true if available range (0 to 99) at n.
	 * @return BCDed value.
	 */
	inline uint8_t bin_to_bcd(int n, bool& _error)
	{
		if((n < 0) || (n >= 100)) {
			_error = true;
			return 0;
		}
		uint8_t n1 = ((uint8_t)(n / 10));
		uint8_t n2 = ((uint8_t)(n % 10));
		_error = false;
		return ((n1 << 8) | n2);
	}
	/*!
	 * @brief Calculate LBA position of M,S,F.
	 * @param m minutes of LBA.
	 * @param s seconds of LBA.
	 * @param f frames  of LBA.
	 * @return LBA position.-1 if error.
	 * @note m,s,f must be encoded by BCD.Not binary.
	 */
	virtual int64_t msf_to_lba(uint8_t m, uint8_t s, uint8_t f) const
	{
		int64_t mm = bcd_to_bin(m);
		int64_t ss = bcd_to_bin(s);
		int64_t ff = bcd_to_bin(f);
		if((mm < 0) || (ss < 0) || (ff < 0)) return -1; // Illegal value.
		return ((mm * 60 * 75) + (ss * 75) + ff);
	}
	/*!
	 * @brief Calculate M,S,F from LBA position.
	 * @param lba LBA position. 
	 * @param m minutes of LBA.
	 * @param s seconds of LBA.
	 * @param f frames  of LBA.
	 * @return true if suceeded.
	 * @note m,s,f are encoded by BCD.Not binary.
	 */
	virtual bool lba_to_msf(int64_t lba, uint8_t& m, uint8_t& s, uint8_t& f) const
	{
		if(lba < 0) return false;
		int64_t mm = lba / (60 * 75);
		int64_t ss;
		int64_t ff;
		bool _error = false;
		
		ss = (lba - (mm * (60 * 75))) / 75;
		ff =  lba - ((mm * (60 * 75)) + ss * 75);
		if((mm >= 100) || (mm < 0)) return false; 
		if((ss >= 60) || (ss < 0)) return false; 
		if((ff >= 75) || (ff < 0)) return false; 
		m = bin_to_bcd(mm, _error);
		if(_error) return false;
		s = bin_to_bcd(ss, _error);
		if(_error) return false;
		f = bin_to_bcd(ff, _error);
		if(_error) return false;
		return true;
	}

	/*!
	 * @brief Load / Save state to VM.
	 * @param state_fio FILE IO for state loading/saving.
	 * @param loading If true loading, false is saving.
	 * @return true if succeeded.
	 */
	virtual bool process_state(FILEIO* state_fio, bool loading)
	{
		/*!
		 * @note Must place checking STATE_VERSION and MAGIC for unique image type..
		 */
		if(!(load_save_params(state_fio, loading))) {
			return false;
		}
		if(!(load_save_toc_table(state_fio, loading))) {
			return false;
		}
		/*
		 * please place state procesing for unuque values below.
		 */
		return true;
	}
};

