#pragma once

/*!
 * @file cdrom_skelton.h
 * @brief Skelton definitions of new CD-ROM class for eFM-Towns.
 * @author K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * @date 2021-03-04
 * @copyright GPLv2
 */


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
 * @class BASIC class of CD Image.
 * @note You must add list of tracks.
 */
class DLL_PREFIX CDIMAGE_META {
protected:
	enum CDIMAGE_TYPE type;
	enum CDIMAGE_TRACK_TYPE tracktype;
	enum CDIMAGE_OPENMODE openmode;
	uint32_t logical_bytes_per_block;
	uint32_t physical_bytes_per_block;
	uint32_t real_physical_bytes_per_block;
	int64_t max_blocks;
	bool allow_beyond_track; //!< allow over track reading.
	
	/*!
	 * @note belows are status value.
	 * Not table values.
	 */
	bool track_is_available;
	int now_track; //! @note 00 - 99.
	int64_t now_lba;
	uint32_t offset_in_sector;
	int64_t lba_offset_of_this_track;
	int64_t sectors_of_this_track;
	int64_t pregap_of_this_track;
	std::string __filename;

	/*!
	 * @brief Parse CUE/CCD sheet, check track data and construct tracks table.
	 * @param filename filename (absolute full path) of sheet (or ISO data).
	 * @return true if succeeded.
	 */
	virtual bool parse_sheet(_TCHAR *filename)
	{
		return true;
	}
public:
	/*!
	 * @brief constructor
	 */
	CDIMAGE_META()
	{
		max_blocks = 0;
		type = IMAGETYPE_NONE;
		tracktype = TRACKTYPE_NONE;
		openmode = OPENMODE_AUTO;
		logical_bytes_per_block = 2048;
		physical_bytes_per_block = 2352;
		real_physical_bytes_per_block = 2352; //!< 2048 if MODE1/ISO.
		allow_beyond_track = false;

		now_lba = 0;
		now_track = 0;
		offset_in_sector = 0;
		lba_offset_of_this_track = 0;
		sectors_of_this_track = 0;
		pregap_of_this_track = 150;
		track_is_available = false;
		
		__filename.erase();
	}
	/*!
	 * @brief de-constructor
	 * @note Please implement de-allocating tracks list 
	 * in de-constructor if you need.
	 */
	~CDIMAGE_META()
	{
	}
	/*!
	 * @brief Get track position now accessing.
	 * @return track value.-1 if not avaiable image.
	 */
	int track() const
	{
		return now_track;
	}
	/*!
	 * @brief Get LBA position of now accessing.
	 * @return LBA position of now accessing.
	 */
	int64_t lba() const
	{
		return now_lba;
	}
	/*!
	 * @brief Get Relative LBA offset value (at head of this track) of this image.
	 * @return Relative LBA offset value (in image).
	 */
	int64_t lba_offset() const
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
		if(filename == NULL) {
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
		return false;
	}
	/*!
	 * @brief Close virtual disc image.
	 * @return true if succeeded.
	 */
	virtual bool close()
	{
		return true;
	}

	/*!
	 * @brief Read logical image data to buffer from current LBA position.
	 * @param buf Destination pointer of read buffer.
	 * @param buflen Size of read buffer.
	 * @param sectors Count of sectors (LBAs).
	 * @param _clear true if expect to clear buffer.
	 * @return size of reading.
	 * @note Override and inherit this to implement real method.
	 * @note Stop when reaches END of CURRENT TRACK.
	 */
	virtual ssize_t read(uint8_t *buf, ssize_t buflen, size_t sectors = 1, bool _clear = false)
	{
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
		in_track = true;
		return true;
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
		in_track = true;
		return true;
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
		in_track = true;
		return true;
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
		return std::nan();
	}
	/*!
	 * @brief Calculate seek time to expected LBA.
	 * @param lba Position of LBA (absolute)
	 * @return seek time as usec.
	 * If error, return NaN. 
	 */	
	virtual double get_seek_time_absolute_lba(int64_t lba)
	{
		return std::nan();
	}
	/*!
	 * @brief Calculate seek time to expected LBA.
	 * @param lba Position of LBA (relative)
	 * @return seek time as usec.
	 * If error, return NaN. 
	 */	
	virtual double get_seek_time_relative_lba(int64_t lba)
	{
		return std::nan();
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
};
