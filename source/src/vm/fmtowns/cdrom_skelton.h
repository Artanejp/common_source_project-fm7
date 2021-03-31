#pragma once

/*!
 * @file cdrom_skelton.h
 * @brief Skelton definitions of new CD-ROM class for eFM-Towns.
 * @author K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * @date 2021-03-04
 * @copyright GPLv2
 */
#include "../../common.h"
#include <string>
class FILEIO;

namespace CDROM_META {  // BEGIN OF NAMESPACE CDROM_META .
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
	TRACKTYPE_MODE1_ISO,  //!< MODE1/ISO (2048 bytes/sector)
	TRACKTYPE_MODE2_2336, //!< MODE2/2336
	TRACKTYPE_MODE2_2352, //!< MODE2/2352
	TRACKTYPE_MODE2_ISO,  //!< MODE2/ISO (2336 bytes/sector)
	TRACKTYPE_2352_ISO,   //!< ISO (2352 bytes/sector)
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
	uint8_t sync[12];	 //!< SYNC BYTES (Mostly 00h * 12?)
	uint8_t addr_m;		 //!< Minute as BCD
	uint8_t addr_s;      //!< Second as BCD
	uint8_t addr_f;      //!< Frame as BCD
	uint8_t sector_type; //!< 1 = MODE1, 2=MODE2
} cdrom_data_head_t;
#pragma pack()

#pragma pack(1)
/*!
 * @struct definition of CD-ROM MODE1 sector struct (excepts ISO image).
 * @note ToDo: Still not implement crc32 and ecc.
 * @note 20201116 K.O
 */
typedef struct {
	cdrom_data_head_t header;	//!< HEADER
	uint8_t data[2048];			//!< DATA field (2048bytes)
	uint8_t crc32[4];			//!< CRC32 checksum.
	uint8_t reserved[8];		//!< Reserved
	uint8_t ecc[276];			//! ERROR CORRECTIOM DATA; by read solomon code.
} cdrom_data_mode1_t;
#pragma pack()

#pragma pack(1)
/*!
 * @struct definition of CD-ROM MODE2 sector struct (excepts ISO image).
 */
typedef struct {
	cdrom_data_head_t header;	//!< HEADER
	uint8_t data[2336];			//!< DATA field 2336 bytes
} cdrom_data_mode2_t;
#pragma pack()

#pragma pack(1)
/*!
 * @struct definition of CD-DA sector struct (excepts ISO image).
 */
typedef struct {
	uint8_t data[2352]; //!< DATA field (without HEADER) 2352 bytes, without ECCs.
} cdrom_audio_sector_t;
#pragma pack()

#pragma pack(1)
/*!
 * @struct definition of CDROM RAW sector struct (excepts ISO image).
 */
typedef struct {
	uint8_t data[2352];  //!< RAW DATA field.
} cdrom_raw_sector_t;
#pragma pack()

#pragma pack(1)
/*!
 * @struct definition of DATA field for ISO.
 * @note ToDo: Add fake header and crc and ecc.
 * @note 20201116 K.O
 */
typedef struct {
	uint8_t data[2048]; //!< AT ISO virtual image, contains only data sector.
} cdrom_iso_data_t;
#pragma pack()

/*!
 * @struct definition of track table.
 * @note 20210311 K.O
 */
typedef struct {
	bool available;	               //!< indicate this track is available.
	uint8_t type;	               //!< track type (enum CDIMAGE_TRACK_TYPE)
	int64_t pregap;                //!< pregap value
	int64_t absolute_lba;          //!< absolute lba position.
	int64_t lba_offset;            //!< LBA offset Within a image.
	int64_t lba_size;              //!< LBA size of track.
	int64_t index0;                //!< INDEX0 (relative)
	int64_t index1;                //!< INDEX1 (relative)
	uint64_t current_bytes_offset; //!< CURRENT BYTES OFFSET INSIDE OF THIS IMAGE.
	uint32_t physical_size;        //!< Physical sector size
	uint32_t real_physical_size;   //!< Real physical sector size
	uint32_t logial_size;          //!< Logical sector size
	_TCHAR filename[_MAX_PATH];    //!< Image file name.
} cdrom_toc_table_t;

/*!
 * Information of PLaying position and a track (BCD Format).
 * This is useful to read TOC and to make SUBQ.
 */
typedef struct {
	//!< Current track information.
	uint8_t trk;      //!< 0-99 : Available / 0xff: Unavailable (BCD Value).
	uint8_t type;     //!< enum CDROM_META::CDIMAGE_TRACK_TYPE
	
	uint8_t pregap_m; //!< Pregap of track ; minutes (BCD Value).
	uint8_t pregap_s; //!< Pregap of track ; seconds (BCD Value).
	uint8_t pregap_f; //!< Pregap of track ; frames  (BCD Value).
	
	uint8_t start_m;  //!< Start of track ; minutes (BCD Value).
	uint8_t start_s;  //!< Start of track ; seconds (BCD Value).
	uint8_t start_f;  //!< Start of track ; frames  (BCD Value).
	
	uint8_t end_m;    //!< End of track ; minutes   (BCD Value).
	uint8_t end_s;    //!< End of track ; seconds   (BCD Value).
	uint8_t end_f;    //!< End of track ; frames    (BCD Value).
	//!< Belows are frame position(s).
	uint8_t abs_m;    //!< Relative minutes from start of track (BCD Value).  
	uint8_t abs_s;    //!< Relative seconds from start of track (BCD Value). 
	uint8_t abs_f;    //!< Relative frames from start of track  (BCD Value).
	
	uint8_t rel_m;    //!< Relative minutes from start of track (BCD Value).  
	uint8_t rel_s;    //!< Relative seconds from start of track (BCD Value). 
	uint8_t rel_f;    //!< Relative frames from start of track  (BCD Value). 
} cdrom_position_bcd_t;

/*!
 * Information of PLaying position and a track (Binary Format).
 * This is useful to read TOC and to make SUBQ.
 */
typedef struct {
	uint8_t  trk;     //!< 0-99 : Available / 0xff: Unavailable (BCD Value).
	uint8_t  type;    //!< enum CDROM_META::CDIMAGE_TRACK_TYPE
	int      pregap;  //!< Pregap value.
	int64_t  start;   //!< Start LBA Position.
	int64_t  end;     //!< End LBA Position. (start + SIZE - 1).
	//!< Belows are frame position(s).
	uint64_t abs_pos; //!< Absolute LBA Position.
	uint64_t rel_pos; //!< Relative LBA Position.
} cdrom_position_binary_t;
	
} // END OF NAMESPACE CDROM_META .

/*! 
 * @class BASIC class of CD Image.
 * @note You must add list of tracks.
 */
class DLL_PREFIX CDIMAGE_SKELTON {
protected:
	FILEIO* current_fio;
	FILEIO* sheet_fio;
	
	uint8_t type;      //!< enum CDIMAGE_TYPE 
	uint8_t tracks;	   //!< 00-99. (Maybe)
	uint8_t tracktype; //!< enum CDIMAGE_TRACK_TYPE 
	uint8_t openmode;  //!< enum CDIMAGE_OPENMODE 
	
	CDROM_META::cdrom_toc_table_t toc_table[102];
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
	 * @note Initialize TOC table when calling.
	 */
	virtual bool parse_sheet();
	/*!
	 * @brief Load / Save state(TOC table part) to VM.
	 * @param state_fio FILE IO for state loading/saving.
	 * @param loading If true loading, false is saving.
	 * @return true if succeeded.
	 */
	virtual bool load_save_toc_table(FILEIO* state_fio, bool loading);
	/*!
	 * @brief Load / Save state(main part) to VM.
	 * @param state_fio FILE IO for state loading/saving.
	 * @param loading If true loading, false is saving.
	 * @return true if succeeded.
	 */
	virtual bool load_save_params(FILEIO* state_fio, bool loading);
	/*!
	 * @brief Initialize TOC table..
	 * @param num track number.
	 */
	virtual void init_toc_table(uint8_t num);
	/*!
	 * @brief Convert BCD value to binary value.
	 * @param n BCD value
	 * @return Binarized value.-1 if error.
	 */
	inline int bcd_to_bin(uint8_t n, bool& __error);
	/*!
	 * @brief Convert POSITIVE BINARY value to BCD.
	 * @param n Binary value.
	 * @param _error Set true if available range (0 to 99) at n.
	 * @return BCDed value.
	 */
	inline uint8_t bin_to_bcd(uint8_t n, bool& __error);

	/*!
	 * @brief Get track position now accessing.
	 * @return track value.-1 if not avaiable image.
	 */
	inline int __get_track() const;
	/*!
	 * @brief Get LBA position of now accessing.
	 * @return LBA position of now accessing.
	 */
	inline int64_t __get_lba() const;
	/*!
	 * @brief Get Relative LBA offset value (at head of this track) of this image.
	 * @return Relative LBA offset value (in image).
	 */
	inline int64_t __get_lba_offset() const;
	/*!
	 * @brief Get number of sectors at this track.
	 * @return Number of sectors at this track.
	 */
	inline int64_t __get_sectors_of_this_track() const;
	/*!
	 * @brief Get current position-offset in this sector.
	 * @return Offset position.
	 */
	inline int __get_offset_of_this_sector() const;
	/*!
	 * @brief Whether this track is available.
	 * @return true if available.
	 */
	inline bool __is_available() const;
	/*!
	 * @brief Get blocks of this virtual CD.
	 * @return Blocks (sectors) of this virtual CD.
	 */
	inline int64_t __get_blocks() const;
	/*!
	 * @brief Get physical block size of this virtual CD.
	 * @return Physical block size of this virtual CD.
	 */
	inline uint32_t __get_physical_block_size() const;
	/*!
	 * @brief Get REAL (in VIRTUAL IMAGE) physical block size of this virtual CD.
	 * @return Physical block size of this virtual CD.
	 */
	inline uint32_t __get_real_physical_block_size() const;
	/*!
	 * @brief Get logical block size of this virtual CD.
	 * @return Logical block size of this virtual CD.
	 */
	inline uint32_t __get_logical_block_size() const;
	/*!
	 * @brief Get image type of this virtual CD.
	 * @return Image type of virtual CD.
	 */
	inline enum CDROM_META::CDIMAGE_TYPE __get_type() const;
	/*!
	 * @brief Get track type of this track.
	 * @return TRACK TYPE.
	 */
	inline enum CDROM_META::CDIMAGE_TRACK_TYPE __get_track_type() const;
	/*!
	 * @brief Get full path of this virtual image.
	 * @param var Returned full path of opened file.Erase if not opened.
	 * @return true if already opened.
	 */
	inline bool __get_track_image_file_name(std::string& var);
	
	
	/*!
	 * @brief Set seek speed.
	 * @param usec Basic transfer time normally 1.0 / 150.0KHz.
	 */
	inline void __set_transfer_time_us(double usec);
	/*!
	 * @brief Get transfer time per byte.
	 * @return transfer time as uSec.
	 */
	inline double __get_transfer_time_us();
	/*!
	 * @brief Get seek multiply rate.
	 * @return Seek rate.
	 */
	inline double __get_seek_speed();
	
	/*!
	 * @brief Seek assigned position in track.
	 * @param lba *Relative* LBA in this track.
	 * @return true if success.
	 */
	virtual bool seek_in_track(int64_t lba);
	/*!
	 * @brief Get image data of track.
	 * @param track track number.
	 * return true if success.
	 */
	virtual bool get_track_image(uint8_t track);
	/*!
	 * @brief reset FILEIOs for sheet and image.
	 */
	virtual void reset_sheet_fio();
public:
	/*!
	 * @brief constructor
	 */
	CDIMAGE_SKELTON();
	/*!
	 * @brief de-constructor
	 * @note Please implement de-allocating tracks list 
	 * in de-constructor if you need.
	 */
	~CDIMAGE_SKELTON();

	/*!
	 * @brief initialize function (blank skelton)
	 */
	virtual void initialize();
	/*!
	 * @brief de-initialize function (blank skelton)
	 */
	virtual void release();
	/*!
	 * @brief reset status function (blank skelton)
	 */
	virtual void reset();
	
	/*!
	 * @brief Get Relative LBA offset value (at head of this track) of this image.
	 * @return Relative LBA offset value (in image).
	 */
	virtual int64_t get_lba_offset() const;
	/*!
	 * @brief Get track type of this track.
	 * @return TRACK TYPE.
	 */
	virtual enum CDROM_META::CDIMAGE_TRACK_TYPE get_track_type() const;
	/*!
	 * @brief Get image type of this virtual CD.
	 * @return Image type of virtual CD.
	 */
	virtual enum CDROM_META::CDIMAGE_TYPE get_type() const;
	/*!
	 * @brief Get full path of this virtual image.
	 * @param var Returned full path of opened file.Erase if not opened.
	 * @return true if already opened.
	 */
	virtual bool get_track_image_file_name(std::string& var);
	
	/*!
	 * @brief Set seek speed.
	 * @param usec Basic transfer time normally 1.0 / 150.0KHz.
	 */
	virtual void set_transfer_time_us(double usec);
	/*!
	 * @brief Get transfer time per byte.
	 * @return transfer time as uSec.
	 */
	virtual double get_transfer_time_us();
	/*!
	 * @brief Get seek multiply rate.
	 * @return Seek rate.
	 */
	virtual double get_seek_speed();
	/*!
	 * @brief Get seek time per block.
	 * @return seek time as uSec.
	 */
	virtual double get_single_seek_time_us();
	/*!
	 * @brief Set seek speed.
	 * @param speed Transfer speed multiply rate, normally 1.0.
	 */
	virtual void set_seek_speed(double speed);
	/*!
	 * @brief Set physical bytes per block (in emulation).
	 * @param bytes bytes per block.
	 */
	virtual void set_physical_bytes_per_block(uint32_t bytes);
	/*!
	 * @brief Set physical bytes per block (in image).
	 * @param bytes bytes per block.
	 */
	virtual void set_real_physical_bytes_per_block(uint32_t bytes);
	/*!
	 * @brief Set logical bytes per block (in emulation).
	 * @param bytes bytes per block.
	 */
	virtual void set_logical_bytes_per_block(uint32_t bytes);
	
	/*!
	 * @brief Convert BCD value to binary value.
	 * @param n BCD value
	 * @return Binarized value.-1 if error.
	 */
	virtual int bcd2bin(uint8_t n, bool& __error);
	/*!
	 * @brief Convert POSITIVE BINARY value to BCD.
	 * @param n Binary value.
	 * @param _error Set true if available range (0 to 99) at n.
	 * @return BCDed value.
	 */
	virtual uint8_t bin2bcd(uint8_t n, bool& __error);
	/*!
	 * @brief Calculate LBA position of M,S,F.
	 * @param m minutes of LBA.
	 * @param s seconds of LBA.
	 * @param f frames  of LBA.
	 * @return LBA position.-1 if error.
	 * @note m,s,f must be encoded by BCD.Not binary.
	 */
	virtual int64_t msf_to_lba(uint8_t m, uint8_t s, uint8_t f) const;
	/*!
	 * @brief Calculate M,S,F from LBA position.
	 * @param lba LBA position. 
	 * @param m minutes of LBA.
	 * @param s seconds of LBA.
	 * @param f frames  of LBA.
	 * @return true if suceeded.
	 * @note m,s,f are encoded by BCD.Not binary.
	 */
	virtual bool lba_to_msf(int64_t lba, uint8_t& m, uint8_t& s, uint8_t& f) const;

	/*
	 * MAIN APIs
	 */
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
	virtual bool open(_TCHAR *filename, enum CDROM_META::CDIMAGE_OPEN_MODE req_type);
	/*!
	 * @brief Close virtual disc image.
	 * @return true if succeeded.
	 */
	virtual bool close();
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
	virtual ssize_t read_mode1(uint8_t *buf, ssize_t buflen, size_t sectors = 1, bool _clear = false);
	/*!
	 * @brief Read image data to buffer as CD-ROM/MODE2 from current LBA position.
	 * @param buf Destination pointer of read buffer.
	 * @param buflen Size of read buffer.
	 * @param sectors Count of sectors (LBAs).
	 * @param _clear true if expect to clear buffer.
	 * @return size of reading.
	 * @note Override and inherit this to implement real method.
	 * @note Stop when reaches END of CURRENT TRACK.
	 */
	virtual ssize_t read_mode2(uint8_t *buf, ssize_t buflen, size_t sectors = 1, bool _clear = false);

	/*!
	 * @brief Read image data to buffer as CD-DA from current LBA position.
	 * @param buf Destination pointer of read buffer.Must be pair16_t[(2352 / 4) * 2].
	 * @param buflen bytes of read buffer.
	 * @param sectors Count of sectors (LBAs).
	 * @param swap_byte true if swap byte order.
	 * @param _clear true if expect to clear buffer.
	 * @return read samples.
	 * @note Override and inherit this to implement real method.
	 */
	virtual ssize_t read_cdda(pair16_t *buf, ssize_t buflen, size_t sectors = 1, bool swap_byte = false, bool _clear = false);
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
	virtual ssize_t read_raw(uint8_t *buf, ssize_t buflen, size_t sectors = 1, bool _clear = false);
	/*!
	 * @brief Try to seek to expected LBA.
	 * @param m minutes of LBA (absolute)
	 * @param s seconds of LBA (absolute)
	 * @param f frames  of LBA (absolute)
	 * @param in_track Set true if within track, set false if seek to another track.
	 * @return true if succeeded.
	 * @note need to implement accross another tracks.
	 */	
	virtual bool seek(uint8_t m, uint8_t s, uint8_t f, bool& in_track);
	/*!
	 * @brief Try to seek to expected LBA.
	 * @param lba Position of LBA (absolute)
	 * @param in_track Set true if within track, set false if seek to another track.
	 * @return true if succeeded.
	 * @note need to implement accross another tracks.
	 */	
	virtual bool seek_absolute_lba(int64_t lba, bool& in_track);
	/*!
	 * @brief Try to seek to expected LBA.
	 * @param lba Position of LBA (relative)
	 * @param in_track Set true if within track, set false if seek to another track.
	 * @return true if succeeded.
	 * @note need to implement accross another tracks.
	 */	
	virtual bool seek_relative_lba(int64_t lba, bool& in_track);

	/*!
	 * @brief Read TOC table by TRACK.
	 * @param trk TRACK NUM (0 to 99).
	 * @param pointer of Destination TOC buffer.
	 *        MUST allocate more than sizeof(CDROM_META::cdrom_toc_table_t).
	 * @return true if success.
	 */
	virtual bool get_toc_table(int trk, CDROM_META::cdrom_toc_table_t* data);
	/*!
	 * @brief Read TOC/Playing position of used track (BCD Format)
	 * @param trk TRACK NUM (0 to 99). If -1 get informations of track now playing.
	 * @param pointer of Destination Playing status table buffer.
	 *        MUST allocate more than sizeof(CDROM_META::cdrom_position_bcd_t).
	 * @return true if available this track.
	 */
	virtual bool get_position_by_bcd(int trk, CDROM_META::cdrom_position_bcd_t* data);
	/*!
	 * @brief Read TOC/Playing position of used track (BINARY Format)
	 * @param trk TRACK NUM (0 to 99). If -1 get informations of track now playing.
	 * @param pointer of Destination Playing status table buffer.
	 *        MUST allocate more than sizeof(CDROM_META::cdrom_position_bcd_t).
	 * @return true if available this track.
	 */
	virtual bool get_position_by_binary(int trk, CDROM_META::cdrom_position_binary_t* data);
	/*!
	 * @brief Get track position now accessing.
	 * @return track value.-1 if not avaiable image.
	 */
	virtual int get_track() const;
	/*!
	 * @brief Get LBA position of now accessing.
	 * @return LBA position of now accessing.
	 */
	virtual int64_t get_lba() const;
	/*!
	 * @brief Get number of sectors at this track.
	 * @return Number of sectors at this track.
	 */
	virtual int64_t get_sectors_of_this_track() const;
	/*!
	 * @brief Get current position-offset in this sector.
	 * @return Offset position.
	 */
	virtual int get_offset_of_this_sector() const;
	/*!
	 * @brief Whether this track is available.
	 * @return true if available.
	 */
	virtual bool is_available() const;
	/*!
	 * @brief Get blocks of this virtual CD.
	 * @return Blocks (sectors) of this virtual CD.
	 */
	virtual int64_t get_blocks() const;
	/*!
	 * @brief Get physical block size of this virtual CD.
	 * @return Physical block size of this virtual CD.
	 */
	virtual uint32_t get_physical_block_size() const;
	/*!
	 * @brief Get REAL (in VIRTUAL IMAGE) physical block size of this virtual CD.
	 * @return Physical block size of this virtual CD.
	 */
	virtual uint32_t get_real_physical_block_size() const;
	/*!
	 * @brief Get logical block size of this virtual CD.
	 * @return Logical block size of this virtual CD.
	 */
	virtual uint32_t get_logical_block_size() const;
	/*!
	 * @brief Calculate seek time to expected LBA.
	 * @param m minutes of LBA (absolute)
	 * @param s seconds of LBA (absolute)
	 * @param f frames  of LBA (absolute)
	 * @return seek time as usec.
	 * If error, return NaN. 
	 */	
	virtual double get_seek_time(uint8_t m, uint8_t s, uint8_t f);
	/*!
	 * @brief Calculate seek time to expected LBA.
	 * @param lba Position of LBA (absolute)
	 * @return seek time as usec.
	 * If error, return NaN. 
	 */	
	virtual double get_seek_time_absolute_lba(int64_t lba);
	/*!
	 * @brief Calculate seek time to expected LBA.
	 * @param lba Position of LBA (relative)
	 * @return seek time as usec.
	 * If error, return NaN. 
	 */	
	virtual double get_seek_time_relative_lba(int64_t lba);
	/*!
	 * @brief Set enable/disable beyond track reading.
	 * @param val enable when setting true.
	 */
	virtual void enable_beyond_track_reading(bool val);
	
	/*!
	 * @brief Check type of virtual disc image by filename.
	 * @param filename Filename of image (absolute path).
	 * @return Type of CD image.
	 */
	static enum CDROM_META::CDIMAGE_TYPE check_type(_TCHAR *filename);

	/*!
	 * @brief Get uint value from BCD string.
	 * @param s source string
	 * @param errorval set true if wrong string value.
	 * @return Value if success, 0 if not.
	 */
	static uint64_t get_val_from_bcdstr(std::string s, bool& errorval);
	/*!
	 * @brief Get uint value from HEXADECIMAL string.
	 * @param s source string
	 * @param errorval set true if wrong string value.
	 * @return Value if success, 0 if not.
	 */
	static uint64_t get_val_from_hexstr(std::string s, bool& errorval);
	/*!
	 * @brief Decode frame value from MSF string.
	 * @param timestr Time string. Encoded by "xx:xx:xx". xx must be BCD value.
	 * @param errorval true if wrong string.
	 * @return value if success, 0 when failed.
	 */
	static uint64_t get_frames_from_msfstr(std::string timestr, bool &errorval);
	/*!
	 * @brief To be Upper characters from string.
	 * @param s source string.
	 * @return ToUpper'ed string.
	 */
	static std::string to_upper(std::string s);

	/*!
	 * @brief Load / Save state to VM.
	 * @param state_fio FILE IO for state loading/saving.
	 * @param loading If true loading, false is saving.
	 * @return true if succeeded.
	 */
	virtual bool process_state(FILEIO* state_fio, bool loading);
};

/*!
 * Inline functions.
 */
/*!
 * @brief Convert BCD value to binary value.
 * @param n BCD value
 * @return Binarized value.-1 if error.
 */
inline int CDROM_SKELTON::bcd_to_bin(uint8_t n, bool& __error)
{
	uint8_t n1 = n >> 4;
	uint8_t n2 = n & 0x0f;
	if(n1 >= 10) {
		__error = true;
		return 0;
	}
	if(n2 >= 10) {
		__error = true;
		return 0;
	}
	__error = false;
	return (((int)(n1 * 10)) + (int)n2);
}
/*!
 * @brief Convert POSITIVE BINARY value to BCD.
 * @param n Binary value.
 * @param _error Set true if available range (0 to 99) at n.
 * @return BCDed value.
 */
inline uint8_t CDROM_SKELTON::bin_to_bcd(uint8_t n, bool& __error)
{
	if((n < 0) || (n >= 100)) {
		__error = true;
		return 0;
	}
	uint8_t n1 = ((uint8_t)(n / 10));
	uint8_t n2 = ((uint8_t)(n % 10));
	__error = false;
	return ((n1 << 8) | n2);
}

/*!
 * @brief Get track position now accessing.
 * @return track value.-1 if not avaiable image.
 */
inline int CDROM_SKELTON::__get_track() const
{
	return now_track;
}

/*!
 * @brief Get LBA position of now accessing.
 * @return LBA position of now accessing.
 */
inline int64_t CDROM_SKELTON::__get_lba() const
{
	return now_lba;
}
/*!
 * @brief Get Relative LBA offset value (at head of this track) of this image.
 * @return Relative LBA offset value (in image).
 */
inline int64_t CDROM_SKELTON::__get_lba_offset() const
{
	return lba_offset_of_this_track;
}
/*!
 * @brief Get number of sectors at this track.
 * @return Number of sectors at this track.
 */
inline int64_t CDROM_SKELTON::__get_sectors_of_this_track() const
{
	return sectors_of_this_track;
}
/*!
 * @brief Get current position-offset in this sector.
 * @return Offset position.
 */
inline int CDROM_SKELTON::__get_offset_of_this_sector() const
{
	return (int)offset_in_sector;
}
/*!
 * @brief Whether this track is available.
 * @return true if available.
 */
inline bool CDROM_SKELTON::__is_available() const
{
	return track_is_available;
}
/*!
 * @brief Get blocks of this virtual CD.
 * @return Blocks (sectors) of this virtual CD.
 */
inline int64_t  CDROM_SKELTON::__get_blocks() const
{
	return max_blocks;
}
/*!
 * @brief Get physical block size of this virtual CD.
 * @return Physical block size of this virtual CD.
 */
inline uint32_t CDROM_SKELTON::__get_physical_block_size() const
{
	return physical_bytes_per_block;
}
/*!
 * @brief Get REAL (in VIRTUAL IMAGE) physical block size of this virtual CD.
 * @return Physical block size of this virtual CD.
 */
inline uint32_t CDROM_SKELTON::__get_real_physical_block_size() const
{
	return real_physical_bytes_per_block;
}
/*!
 * @brief Get logical block size of this virtual CD.
 * @return Logical block size of this virtual CD.
 */
inline uint32_t CDROM_SKELTON::__get_logical_block_size() const
{
	return logical_bytes_per_block;
}

/*!
 * @brief Get transfer time per byte.
 * @return transfer time as uSec.
 */
inline double CDROM_SKELTON::__get_transfer_time_us()
{
	double _speed = seek_speed;
	if(_speed <= 0.0) _speed = 1.0;
		
	return (transfer_time_us / _speed);
}

/*!
 * @brief Get seek multiply rate.
 * @return Seek rate.
 */
inline double CDROM_SKELTON::__get_seek_speed()
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
inline double CDROM_SKELTON::__get_single_seek_time_us()
{
	double bytes = (double)physical_bytes_per_block;
	if(bytes < 1.0) bytes = 1.0;
	double usec = __get_transfer_time_us();

	if(usec < (1.0 / 32.0)) usec = 1.0 / 32.0;
	return usec * bytes;
}

/*!
 * @brief Set seek speed.
 * @param usec Basic transfer time normally 1.0 / 150.0KHz.
 */
inline void CDROM_SKELTON::__set_transfer_time_us(double usec)
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
 * @brief Set seek speed.
 * @param speed Transfer speed multiply rate, normally 1.0.
 */
inline void CDROM_SKELTON::__set_seek_speed(double speed)
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
inline void CDROM_SKELTON::__set_physical_bytes_per_block(uint32_t bytes)
{
	if(bytes == 0) bytes = 2352; // Default value
	physical_bytes_per_block = bytes;
}
/*!
 * @brief Set physical bytes per block (in image).
 * @param bytes bytes per block.
 */
inline void CDROM_SKELTON::__set_real_physical_bytes_per_block(uint32_t bytes)
{
	if(bytes == 0) bytes = 2352; // Default value
	real_physical_bytes_per_block = bytes;
}
/*!
 * @brief Set logical bytes per block (in emulation).
 * @param bytes bytes per block.
 */
inline void CDROM_SKELTON::__set_logical_bytes_per_block(uint32_t bytes)
{
	if(bytes == 0) bytes = 2048; // Default value
	logical_bytes_per_block = bytes;
}

/*!
 * @brief Get track type of this track.
 * @return TRACK TYPE.
 */
inline enum CDROM_META::CDIMAGE_TRACK_TYPE CDROM_SKELTON::__get_track_type() const
{
	return ((CDROM_META::CDIMAGE_TRACK_TYPE)tracktype);
}
/*!
 * @brief Get image type of this virtual CD.
 * @return Image type of virtual CD.
 */
inline enum CDROM_META::CDIMAGE_TYPE CDROM_SKELTON::__get_type() const
{
	return ((CDROM_META::CDIMAGE_TYPE)type);
}
/*!
 * @brief Get full path of this virtual image.
 * @param var Returned full path of opened file.Erase if not opened.
 * @return true if already opened.
 */
inline bool CDROM_SKELTON::__get_track_image_file_name(std::string& var)
{
	if(__filename.empty()) {
		var.erase();
		return false;
	}
	var = __filename;
	return true;
}

