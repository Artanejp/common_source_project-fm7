/*!
 * @file cdrom_skelton.cpp
 * @brief Skelton methods of new CD-ROM class for eFM-Towns.
 * @author K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * @date 2021-03-12
 * @copyright GPLv2
 */
#include <cmath>
#include <stdlib.h>

#include "../../fileio.h"
#include "./cdrom_skelton.h"

/*!
 * @brief Calculate LBA position of M,S,F.
 * @param m minutes of LBA.
 * @param s seconds of LBA.
 * @param f frames  of LBA.
 * @return LBA position.-1 if error.
 * @note m,s,f must be encoded by BCD.Not binary.
 */
int64_t CDROM_SKELTON::msf_to_lba(uint8_t m, uint8_t s, uint8_t f) const
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
bool  CDROM_SKELTON::lba_to_msf(int64_t lba, uint8_t& m, uint8_t& s, uint8_t& f) const
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
 * @brief Open virtual disc image.
 * @param filename Filename of image (absolute path).
 * @param req_type Opening mode.
 * @return true if succeeded.
 *
 * @note Must set disc parameters list only at this function (or sub-functions).
 * @see parse_sheet
 * @see check_type
 */
bool CDROM_SKELTON::open(_TCHAR *filename, enum CDROM_META::CDIMAGE_OPEN_MODE req_type)
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
bool CDROM_SKELTON::close()
{
	reset_sheet_fio();
	for(uint8_t num = 0; num < 102; num++) {
		init_toc_table(num);
	}
		
	type = CDROM_META::IMAGETYPE_NODE;
	tracktype = CDROM_META::TRACKTYPE_NONE;
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
ssize_t CDROM_SKELTON::read_mode1(uint8_t *buf, ssize_t buflen, size_t sectors, bool _clear)
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
ssize_t CDROM_SKELTON::read_mode2(uint8_t *buf, ssize_t buflen, size_t sectors, bool _clear)
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
ssize_t CDROM_SKELTON::read_cdda(uint8_t *buf, ssize_t buflen, size_t sectors, bool _clear)
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
ssize_t CDROM_SKELTON::read_raw(uint8_t *buf, ssize_t buflen, size_t sectors, bool _clear)
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
bool CDROM_SKELTON::seek(uint8_t m, uint8_t s, uint8_t f, bool& in_track)
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
bool CDROM_SKELTON::seek_absolute_lba(int64_t lba, bool& in_track)
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
bool CDROM_SKELTON::seek_relative_lba(int64_t lba, bool& in_track)
{
	int64_t __lba = lba + now_lba;
	return seek_absolute_lba(__lba, in_track);
}

/*!
 * @brief Set seek speed.
 * @param usec Basic transfer time normally 1.0 / 150.0KHz.
 */
void CDROM_SKELTON::set_transfer_time_us(double usec)
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
void CDROM_SKELTON::set_seek_speed(double speed)
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
void CDROM_SKELTON::set_physical_bytes_per_block(uint32_t bytes)
{
	if(bytes == 0) bytes = 2352; // Default value
	physical_bytes_per_block = bytes;
}
/*!
 * @brief Set physical bytes per block (in image).
 * @param bytes bytes per block.
 */
void CDROM_SKELTON::set_real_physical_bytes_per_block(uint32_t bytes)
{
	if(bytes == 0) bytes = 2352; // Default value
	real_physical_bytes_per_block = bytes;
}
/*!
 * @brief Set logical bytes per block (in emulation).
 * @param bytes bytes per block.
 */
void CDROM_SKELTON::set_logical_bytes_per_block(uint32_t bytes)
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
double CDROM_SKELTON::get_seek_time(uint8_t m, uint8_t s, uint8_t f)
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
double CDROM_SKELTON::get_seek_time_absolute_lba(int64_t lba)
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
double CDROM_SKELTON::get_seek_time_relative_lba(int64_t lba)
{
	int64_t lba2 = llabs(lba);
	if(lba2 > max_blocks) lba2 = max_blocks;

	double _t = (get_single_seek_time_us() * ((double)lba2));
	return _t;
}
/*!
 * @brief Check type of virtual disc image by filename.
 * @param filename Filename of image (absolute path).
 * @return Type of CD image.
 */
static enum CDROM_META::CDIMAGE_TYPE CDROM_SKELTON::check_type(_TCHAR *filename)
{
	if(filename == nullptr) {
		return CDROM_META::IMAGETYPE_NONE;
	}
	if(!(FILEIO::IsFileExisting(filename))) {
		return CDROM_META::IMAGETYPE_NONE;
	}
	if(check_file_extension(filename, _T(".gz"))) {
		if(check_file_extension(filename, _T(".iso.gz"))) {
			return CDROM_META::IMAGETYPE_ISO;
		}
		if(check_file_extension(filename, _T(".cue.gz"))) {
			return CDROM_META::IMAGETYPE_CUE;
		}
		if(check_file_extension(filename, _T(".ccd.gz"))) {
			return CDROM_META::IMAGETYPE_CCD;
		}
	} else {
		if(check_file_extension(filename, _T(".iso"))) {
			return CDROM_META::IMAGETYPE_ISO;
		}
		if(check_file_extension(filename, _T(".cue"))) {
			return CDROM_META::IMAGETYPE_CUE;
		}
		if(check_file_extension(filename, _T(".ccd"))) {
			return CDROM_META::IMAGETYPE_CCD;
		}
	}
	return CDROM_META::IMAGETYPE_NONE; // Write Unique routines to next.
}


/*!
 * @brief Get image type of this virtual CD.
 * @return Image type of virtual CD.
 */
enum CDROM_META::CDIMAGE_TYPE CDROM_SKELTON::get_type() const
{
	return ((CDROM_META::CDIMAGE_TYPE)type);
}
/*!
 * @brief Get full path of this virtual image.
 * @param var Returned full path of opened file.Erase if not opened.
 * @return true if already opened.
 */
bool CDROM_SKELTON::get_track_image_file_name(std::string& var)
{
	if(__filename.empty()) {
		var.erase();
		return false;
	}
	var = __filename;
	return true;
}

/*!
 * @brief Set enable/disable beyond track reading.
 * @param val enable when setting true.
 */
void CDROM_SKELTON::enable_beyond_track_reading(bool val)
{
	allow_beyond_track = val;
}


/*!
 * @brief Parse CUE/CCD sheet, check track data and construct tracks table.
 * @return true if succeeded.
 * @note Must open sheet file before using.
 */
bool CDROM_SKELTON::parse_sheet()
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
bool CDROM_SKELTON::load_save_toc_table(FILEIO* state_fio, bool loading)
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
	
#define STATE_VERSION 1
/*!
 * @brief Load / Save state(main part) to VM.
 * @param state_fio FILE IO for state loading/saving.
 * @param loading If true loading, false is saving.
 * @return true if succeeded.
 */
bool CDROM_SKELTON::load_save_params(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
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
void CDROM_SKELTON::init_toc_table(uint8_t num)
{
	if(num > 101) return;
		
	toc_table[num].available = false;
	toc_table[num].type = CDROM_META::TRACKTYPE_NONE;
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
bool CDROM_SKELTON::seek_in_track(int64_t lba)
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
bool CDROM_SKELTON::get_track_image(uint8_t track)
{
	bool result = false;
	if(track >= tracks) track = tracks;
	/*
	 * Set default values
	 */
	tracktype = CDROM_META::TRACKTYPE_NONE;
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
void CDROM_SKELTON::reset_sheet_fio()
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

/*!
 * @brief Load / Save state to VM.
 * @param state_fio FILE IO for state loading/saving.
 * @param loading If true loading, false is saving.
 * @return true if succeeded.
 */
bool CDROM_SKELTON::process_state(FILEIO* state_fio, bool loading)
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
