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
 * @brief constructor
 */
CDROM_SKELTON::CDIMAGE_SKELTON()
{
	max_blocks = 0;
	tracks = 0;
	type = CDROM_META::IMAGETYPE_NONE;
	tracktype = CDROM_META::TRACKTYPE_NONE;
	openmode = CDROM_META::OPENMODE_AUTO;
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
CDROM_SKELTON::~CDIMAGE_SKELTON()
{
	reset_sheet_fio();
}

/*!
 * @brief initialize function (blank skelton)
 */
void CDROM_SKELTON::initialize()
{
}
/*!
 * @brief de-initialize function (blank skelton)
 */
void CDROM_SKELTON::release()
{
}

/*!
 * @brief reset status function (blank skelton)
 */
void CDROM_SKELTON::reset()
{
}

/*!
 * @brief Convert BCD value to binary value.
 * @param n BCD value
 * @return Binarized value.-1 if error.
 */
int CDROM_SKELTON::bcd2bin(uint8_t n, bool& __error)
{
	return bcd_to_bin(n, __error);
}
/*!
 * @brief Convert POSITIVE BINARY value to BCD.
 * @param n Binary value.
 * @param _error Set true if available range (0 to 99) at n.
 * @return BCDed value.
 */
uint8_t CDROM_SKELTON::bin2bcd(uint8_t n, bool& __error)
{
	return bin_to_bcd(n, __error);
}

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
	bool _err1, _err2, _err3;
	int64_t mm = bcd_to_bin(m, _err1);
	int64_t ss = bcd_to_bin(s, _err2);
	int64_t ff = bcd_to_bin(f, _err3);

	if((_err1) || (_err2) || (_err3)) return -1;
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
		
//	ss = (lba - (mm * (60 * 75))) / 75;
//	ff =  lba - ((mm * (60 * 75)) + ss * 75);
	ss = (lba / 75) % 60;
	ff = lba % 75;
	if((mm >= 100) || (mm < 0)) return false; 
	if((ss >= 60) || (ss < 0)) return false; 
	if((ff >= 75) || (ff < 0)) return false; 
	m = bin_to_bcd((uint8_t)mm, _error);
	if(_error) return false;
	s = bin_to_bcd((uint8_t)ss, _error);
	if(_error) return false;
	f = bin_to_bcd((uint8_t)ff, _error);
	if(_error) return false;
	return true;
}

/*!
 * @brief Read TOC/Playing position of used track (BCD Format)
 * @param trk TRACK NUM (0 to 99). If -1 get informations of track now playing.
 * @param pointer of Destination Playing status table buffer.
 *        MUST allocate more than sizeof(CDROM_META::cdrom_position_bcd_t).
 * @return true if available this track.
 */
bool CDROM_SKELTON::get_position_by_bcd(int trk, CDROM_META::cdrom_position_bcd_t* data)
{
	if(data == nullptr) return false;
	if(trk == -1) {
		if(now_track > 99) return -false;
		if(now_track < 1)  return false;
		trk = now_track;
	}
	if((trk < 0) || (trk > 99)) return false;
	if(trk >= tracks) return false;
	if(!(toc_table[trk].available)) return false;
	CDROM_META::cdrom_position_binary_t __data;

	memset(((uint8_t*)data), 0x00, sizeof(cdrom_position_bcd_t));
	if(get_position_by_bynary(trk, &__data)) {
		uint8_t m, s, f;
		
		data->trk = __data.trk;
		data->type = __data.type;
		
//		if(!(lba_to_msf(__data.pregap, m, s, f))) {
//			return false;
//		}
		data->pregap_m = m;
		data->pregap_s = s;
		data->pregap_f = f;
		
//		if(!(lba_to_msf(__data.start, m, s, f))) {
//			return false;
//		}
		data->start_m = m;
		data->start_s = s;
		data->start_f = f;

//		if(!(lba_to_msf(__data.end, m, s, f))) {
//			return false;
//		}
		data->end_m = m;
		data->end_s = s;
		data->end_f = f;
		
//		if(!(lba_to_msf(__data.abs_pos, m, s, f))) {
//			return false;
//		}
		data->abs_m = m;
		data->abs_s = s;
		data->abs_f = f;

//		if(!(lba_to_msf(__data.rel_pos, m, s, f))) {
//			return false;
//		}
		data->rel_m = m;
		data->rel_s = s;
		data->rel_f = f;
		return true;
	}
	return false;
}

/*!
 * @brief Read TOC/Playing position of used track (BINARY Format)
 * @param trk TRACK NUM (0 to 99). If -1 get informations of track now playing.
 * @param pointer of Destination Playing status table buffer.
 *        MUST allocate more than sizeof(CDROM_META::cdrom_position_bcd_t).
 * @return true if available this track.
 */
bool CDROM_SKELTON::get_position_by_binary(int trk, CDROM_META::cdrom_position_binary_t* data)
{
	if(data == nullptr) return false;
	if(trk == -1) {
		if(now_track > 99) return -false;
		if(now_track < 1)  return false;
		trk = now_track;
	}
	if((trk < 0) || (trk > 99)) return false;
	if(trk >= tracks) return false;
	if(!(toc_table[trk].available)) return false;
	
	memset(((uint8_t*)data), 0x00, sizeof(cdrom_position_binary_t));

	int64_t _pregap = toc_table[trk].pregap;
	
	if(_pregap < 0) _pregap = 0; // OK?
	data->trk = trk;
	data->type = toc_table[trk].type;
	data->pregap = _pregap;
	data->start = toc_table[trk].absolute_lba;

	int64_t n_size = toc_table[trk].lba_size;
	if(n_size < 1) n_size = 1;

	data->end = toc_table[trk].absolute_lba + n_size - 1;

	data->abs_pos = 0;
	data->rel_pos = 0;
	
	int64_t rel_lba = now_lba;
	if(rel_lba > 0) {
		if(trk == now_track) {
			data->abs_pos = toc_table[trk].absolute_lba + rel_lba;
			data->rel_pos = rel_lba;
		}
	}
	return true;
}
/*!
 * @brief Get TOC table by TRACK (RAW format).
 * @param trk TRACK NUM (0 to 99).
 * @param pointer of Destination TOC buffer.
 *        MUST allocate more than sizeof(CDROM_META::cdrom_toc_table_t).
 * @return true if success.
 */
bool CDROM_SKELTON::get_toc_table(int trk, CDROM_META::cdrom_toc_table_t* data)
{
	if(data == nullptr) return false;
	if((trk < 0) || (track > 99)) return false;
	if(trk >= tracks) return false;
		
	data->available            = toc_table[trk].available;
	data->type                 = toc_table[trk].type;
	data->absolute_lba         = toc_table[trk].absolute_lba;
	data->lba_offset           = toc_table[trk].lba_offset;
	data->lba_size             = toc_table[trk].lba_size;
	data->index0               = toc_table[trk].index0;
	data->index1               = toc_table[trk].index1;
	data->current_bytes_offset = toc_table[trk].current_bytes_offset;
	data->pregap               = toc_table[trk].pregap;
	data->physical_size        = toc_table[trk].physical_size;
	data->real_physical_size   = toc_table[trk].real_physical_size;
	data->logical_size         = toc_table[trk].logical_size;
		
	memset(&(data->filename[0]), 0x00, sizeof(_TCHAR) * _MAX_PATH);
	my_tcscpy_s(&(data->filename[0]), _MAX_PATH - 1, &(toc_table[trk].filename[0]));
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
		bool stat = false;
		sheet_fio = new FILEIO();
		if(sheet_fio != nullptr) {
			if(sheet_fio->Fopen(filename, FILEIO_READ_BINARY)) {
				parse_sheet();
			}
			delete sheet_fio;
			sheet_fio = NULL;
		}
		return stat;
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
	if(buf == nullptr) return -1;
	if(buflen <= 0) return -1;
	if(_clear) {
		memset(buf, 0x00, buflen);
	}
	
	if((now_track == 0) || (now_track >= 100)) return -1;
	if(now_track >= tracks) return -1;
		
	if(sectors <= 0) return -1;
	if(current_fio == nullptr) return -1;
	if(!(curent_fio->IsOpened())) return -1;
	
	int64_t logical_size = (int64_t)(sectors * logical_bytes_per_block);
	if(logical_size >= buflen) logical_size = buflen;
	if(logical_size <= 0) return -1;

	int xptr = 0;
	/*!
	 * @note standard reading function.
	 */
	switch(tracktype) {
	case TRACKTYPE_2352_ISO:
	case TRACKTYPE_MODE1_2352:
		{
			CDROM_META::cdrom_data_mode1_t secbuf;
			for(int i = 0; i < sectors; i++) {
				if(xptr >= logical_size) break;
				if(now_lba >= sectors_of_this_track) {
					// SEEK?
					return xptr;
				}
				size_t n = current_fio->Fread((uint8_t*)(&secbuf), sizeof(secbuf), 1);
				now_lba++;
				if(n != sizeof(secbuf)) {
					return xptr;
				}
				// Data OK
				memcpy(&(buf[xptr]), &(secbuf.data[0]), 2048);
				xptr += 2048;
			}
		}
		break;
	case TRACKTYPE_MODE1_2048:
	case TRACKTYPE_MODE1_ISO:
		{
			uint8_t secbuf[2048];
			for(int i = 0; i < sectors; i++) {
				if(xptr >= logical_size) break;
				if(now_lba >= sectors_of_this_track) {
					// SEEK?
					return xptr;
				}
				size_t n = current_fio->Fread((uint8_t*)(secbuf), sizeof(secbuf), 1);
				now_lba++;
				if(n != sizeof(secbuf)) {
					return xptr;
				}
				// Data OK
				memcpy(&(buf[xptr]), secbuf, 2048);
				xptr += 2048;
			}
		}
		break;
		/*
		 * If another Image type, return -1.
		 */
	default:
		return -1;
		break;
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
 */
ssize_t CDROM_SKELTON::read_mode2(uint8_t *buf, ssize_t buflen, size_t sectors, bool _clear)
{
	if(buf == nullptr) return -1;
	if(buflen <= 0) return -1;
	if(_clear) {
		memset(buf, 0x00, buflen);
	}
	
	if((now_track == 0) || (now_track >= 100)) return -1;
	if(now_track >= tracks) return -1;
		
	if(sectors <= 0) return -1;
	if(current_fio == nullptr) return -1;
	if(!(curent_fio->IsOpened())) return -1;

	int64_t logical_size = (int64_t)(sectors * logical_bytes_per_block);
	if(logical_size >= buflen) logical_size = buflen;
	if(logical_size <= 0) return -1;

	int xptr = 0;
	/*!
	 * @note standard reading function.
	 */
	switch(tracktype) {
	case TRACKTYPE_MODE2_2352:
	case TRACKTYPE_2352_ISO:
		{
			CDROM_META::cdrom_data_mode2_t secbuf;
			for(int i = 0; i < sectors; i++) {
				if(xptr >= logical_size) break;
				if(now_lba >= sectors_of_this_track) {
					// SEEK?
					return xptr;
				}
				size_t n = current_fio->Fread((uint8_t*)(&secbuf), sizeof(secbuf), 1);
				 
				now_lba++;
				if(n != sizeof(secbuf)) {
					return xptr;
				}
				// Data OK
				memcpy(&(buf[xptr]), &(secbuf.data[0]), 2336);
				xptr += 2336;
			}
		}
		break;
	case TRACKTYPE_MODE2_ISO:
	case TRACKTYPE_MODE2_2336:
		{
			uint8_t secbuf[2336];
			for(int i = 0; i < sectors; i++) {
				if(xptr >= logical_size) break;
				if(now_lba >= sectors_of_this_track) {
					// SEEK?
					return xptr;
				}
				size_t n = current_fio->Fread((uint8_t*)(secbuf), sizeof(secbuf), 1);
				now_lba++;
				if(n != sizeof(secbuf)) {
					return xptr;
				}
				// Data OK
				memcpy(&(buf[xptr]), secbuf, 2336);
				xptr += 2336;
			}
		}
		break;
		/*
		 * If another Image type, return -1.
		 */
	default:
		return -1;
		break;
	}
	return logical_size;
}
/*!
 * @brief Read image data to buffer as CD-DA from current LBA position.
 * @param buf Destination pointer of read buffer.Must be pair16_t[(2352 / 4) * 2].
 * @param buflen bytes of read buffer.
 * @param sectors Count of sectors (LBAs).
 * @param swap_byte true if swap byte order.
 * @param _clear true if expect to clear buffer.
 * @return read samples.
 * @note Override and inherit this to implement real method.
 * @note Stop when reaches END of CURRENT TRACK.
 */
ssize_t CDROM_SKELTON::read_cdda(pair16_t *buf, ssize_t buflen, size_t sectors, bool swap_byte, bool _clear)
{
	if(buf == nullptr) return -1;
	if(buflen <= 0) return -1;
	if(_clear) {
		memset(buf, 0x00, buflen);
	}
	
	if((now_track == 0) || (now_track >= 100)) return -1;
	if(now_track >= tracks) return -1;
		
	if(sectors <= 0) return -1;
	if(current_fio == nullptr) return -1;
	if(!(curent_fio->IsOpened())) return -1;

	int64_t physical_size = ((int64_t)sectors) * (2352 / 4);
	buflen >>= 2;
	if(physical_size >= buflen) physical_size = buflen;
	if(physical_size <= 0) return -1;
	
	int xptr = 0;
	int64_t xbptr = 0;
	switch(tracktype) {
	case TRACKTYPE_AUDIO:
	case TRACKTYPE_MODE1_2352:
	case TRACKTYPE_MODE2_2352:
	case TRACKTYPE_CDI_2352:
	case TRACKTYPE_2352_ISO:
		{
			__DECL_ALIGNED(8) uint8_t secbuf[2352];
			for(int i = 0; i < sectors; i++) {
				if(xbptr >= physical_size) break;
				if(now_lba >= sectors_of_this_track) {
					// SEEK?
					if(!(allow_beyond_track)) {
						return xbptr;
					} else {
						if(now_track >= 99) return xbptr;
						if(!(get_track_image(now_track + 1))) {
							return xbptr;
						}
						// Refleshed.
						if(!(seek_in_track(pregap_of_this_track))) {
							return xbptr;
						}
					}
				}
				size_t n = current_fio->Fread((uint8_t*)(secbuf), sizeof(secbuf), 1);
				now_lba++;
				if(n != sizeof(secbuf)) {
					return xbptr;
				}
				// Data OK
				xptr = 0;
				if(swap_byte) {
__DECL_VECTORIZED_LOOP					
					for(int j = 0; j < (2352 / 2); j++) {
						buf[j + (xbptr << 1)].b.h = secbuf[xptr];
						buf[j + (xbptr << 1)].b.l = secbuf[xptr + 1];
						xptr += 2;
					}
				} else {
__DECL_VECTORIZED_LOOP
					for(int j = 0; j < (2352 / 2); j++) {
						buf[j + (xbptr << 1)].b.l = secbuf[xptr];
						buf[j + (xbptr << 1)].b.h = secbuf[xptr + 1];
						xptr += 2;
					}
				}
				xbptr += (2352 / 4);
			}
		}
		break;
		/*!
		 * @todo Implement sector size = 2048 or 2336.
		 */
	default:
		return -1;
	}
	return xbptr;
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
	if(buf == nullptr) return -1;
	if(buflen <= 0) return -1;
	if(_clear) {
		memset(buf, 0x00, buflen);
	}
	
	if((now_track == 0) || (now_track >= 100)) return -1;
	if(now_track >= tracks) return -1;
		
	if(sectors <= 0) return -1;
	if(current_fio == nullptr) return -1;
	if(!(curent_fio->IsOpened())) return -1;

	int64_t physical_size = (int64_t)(sectors * physical_bytes_per_block);
	if(physical_size >= buflen) physical_size = buflen;
	if(physical_size <= 0) return -1;
	int xptr = 0;
	uint8_t secbuf[2352];
	int ps = physical_bytes_per_block;
	if(ps >= 2352) ps = 2352;
	if(ps <= 0) {
		return -1;
	}
	

	for(int i = 0; i < sectors; i++) {
		if(xptr >= physical_size) break;
		if(now_lba >= sectors_of_this_track) {
			// SEEK?
			if(!(allow_beyond_track)) {
				return xptr;
			} else {
				if(now_track >= 99) return xptr;
				if(!(get_track_image(now_track + 1))) {
					return xptr;
				}
				// Refleshed.
				if(!(seek_in_track(pregap_of_this_track))) {
					return xptr;
				}
			}
		}
		size_t n = current_fio->Fread((uint8_t*)(secbuf), ps, 1);
		now_lba++;
		if(n != ps) {
			return xptr;
		}
		memcpy(&(buf[xptr]), secbuf, ps);
		xptr += ps;
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
 * @brief Get Relative LBA offset value (at head of this track) of this image.
 * @return Relative LBA offset value (in image).
 */
int64_t CDROM_SKELTON::get_lba_offset() const
{
	return __get_lba_offset();
}
/*!
 * @brief Get image type of this virtual CD.
 * @return Image type of virtual CD.
 */
enum CDROM_META::CDIMAGE_TYPE CDROM_SKELTON::get_type() const
{
	return __get_type();
}
/*!
 * @brief Get full path of this virtual image.
 * @param var Returned full path of opened file.Erase if not opened.
 * @return true if already opened.
 */
bool CDROM_SKELTON::get_track_image_file_name(std::string& var)
{
	return __get_track_image_file_name(var);
}
	
/*!
 * @brief Set seek speed.
 * @param usec Basic transfer time normally 1.0 / 150.0KHz.
 */
void CDROM_SKELTON::set_transfer_time_us(double usec)
{
	__set_transfer_time_us(double usec);
}
/*!
 * @brief Get transfer time per byte.
 * @return transfer time as uSec.
 */
double CDROM_SKELTON::get_transfer_time_us()
{
	return __get_transfer_time_us();
}
/*!
 * @brief Get seek multiply rate.
 * @return Seek rate.
 */
double CDROM_SKELTON::get_seek_speed()
{
	return __get_seek_speed();
}
/*!
 * @brief Get seek time per block.
 * @return seek time as uSec.
 */
double CDROM_SKELTON::get_single_seek_time_us()
{
	return __get_single_seek_time_us();
}
/*!
 * @brief Set seek speed.
 * @param speed Transfer speed multiply rate, normally 1.0.
 */
void CDROM_SKELTON::set_seek_speed(double speed)
{
	__set_seek_speed(speed);
}
/*!
 * @brief Set physical bytes per block (in emulation).
 * @param bytes bytes per block.
 */
void CDROM_SKELTON::set_physical_bytes_per_block(uint32_t bytes)
{
	__set_physical_bytes_per_block(bytes);
}
/*!
 * @brief Set physical bytes per block (in image).
 * @param bytes bytes per block.
 */
void CDROM_SKELTON::set_real_physical_bytes_per_block(uint32_t bytes)
{
	__set_real_physical_bytes_per_block(bytes);
}
/*!
 * @brief Set logical bytes per block (in emulation).
 * @param bytes bytes per block.
 */
void CDROM_SKELTON::set_logical_bytes_per_block(uint32_t bytes)
{
	__set_logical_bytes_per_block(bytes);
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
 * @brief Get track position now accessing.
 * @return track value.-1 if not avaiable image.
 */
int CDROM_SKELTON::get_track() const
{
	return __get_track();
}
/*!
 * @brief Get LBA position of now accessing.
 * @return LBA position of now accessing.
 */
int64_t CDROM_SKELTON::get_lba() const
{
	return __get_lba();
}
/*!
 * @brief Get number of sectors at this track.
 * @return Number of sectors at this track.
 */
int64_t CDROM_SKELTON::get_sectors_of_this_track() const
{
	return __get_sectors_of_this_track();
}
/*!
 * @brief Get current position-offset in this sector.
 * @return Offset position.
 */
int CDROM_SKELTON::get_offset_of_this_sector() const
{
	return __get_offset_of_this_sector();
}
/*!
 * @brief Whether this track is available.
 * @return true if available.
 */
bool CDROM_SKELTON::is_available() const
{
	return __is_available();
}
/*!
 * @brief Get blocks of this virtual CD.
 * @return Blocks (sectors) of this virtual CD.
 */
int64_t CDROM_SKELTON::get_blocks() const
{
	return __get_blocks();
}
/*!
 * @brief Get physical block size of this virtual CD.
 * @return Physical block size of this virtual CD.
 */
uint32_t CDROM_SKELTON::get_physical_block_size() const
{
	return __get_physical_block_size();
}
/*!
 * @brief Get REAL (in VIRTUAL IMAGE) physical block size of this virtual CD.
 * @return Physical block size of this virtual CD.
 */
uint32_t CDROM_SKELTON::get_real_physical_block_size() const
{
	return __get_real_physical_block_size();
}
/*!
 * @brief Get logical block size of this virtual CD.
 * @return Logical block size of this virtual CD.
 */
uint32_t CDROM_SKELTON::get_logical_block_size() const
{
	return __get_logical_block_size();
}

/*!
 * @brief Parse CUE/CCD sheet, check track data and construct tracks table.
 * @return true if succeeded.
 * @note Must open sheet file before using.
 */
bool CDROM_SKELTON::parse_sheet()
{
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
 * @brief To be Upper characters from string.
 * @param s source string.
 * @return ToUpper'ed string.
 */
static std::string CDROM_SKELTON::to_upper(std::string s)
{
	std::string rets;

	rets.clear();
	if(!(s.empty())) {
		for(auto c = s.begin(); c != s.end(); ++c) {
			_TCHAR n = std::toupper(*c);
			rets.push_back(n);
		}
	}
	return rets;
}
/*!
 * @brief Get uint value from BCD string.
 * @param s source string
 * @param errorval set true if wrong string value.
 * @return Value if success, 0 if not.
 */
static uint64_t CDROM_SKELTON::get_val_from_bcdstr(std::string s, bool& errorval)
{
	int pos = 0;
	uint64_t rval = 0;
	int pval;
	for(auto c = s.begin(); c != s.end() ; ++c) {
		char cc = *c;
		if((cc < '0') || (cc > '9')) {
			break;
		}
		rval = rval * 10;
		pval = cc - '0';
		rval = rval + pval;
		pos++;
	}
	if(pos < 1) {
		errorval = true;
		return 0;
	}
	errorval = false;
	return rval;
}

/*!
 * @brief Get uint value from HEXADECIMAL string.
 * @param s source string
 * @param errorval set true if wrong string value.
 * @return Value if success, 0 if not.
 */
static uint64_t CDROM_SKELTON::get_val_from_hexstr(std::string s, bool& errorval)
{
	int pos = 0;
	uint64_t rval = 0;
	int pval;
	for(auto c = s.begin(); c != s.end() ; ++c) {
		char cc = *c;
		bool is_val = false;
		if((cc >= '0') && (cc <= '9')) {
			is_val = true;
			pval = cc - '0';
		} else if((cc >= 'A') && (cc <= 'F')) {
			is_val = true;
			pval = cc - 'A';
		} else if((cc >= 'a') && (cc <= 'f')) {
			is_val = true;
			pval = cc - 'a';
		}
		if(!(is_val)) {
			break;
		}
		rval = rval << 4;
		rval = rval + pval;
		pos++;
	}
	if(pos < 1) {
		errorval = true;
		return 0;
	}
	errorval = false;
	return rval;
}

/*!
 * @brief Decode frame value from MSF string.
 * @param timestr Time string. Encoded by "xx:xx:xx". xx must be BCD value.
 * @param errorval true if wrong string.
 * @return value if success, 0 when failed.
 */
static uint64_t CDROM_SKELTON::get_frames_from_msfstr(std::string timestr, bool &errorval)
{
	if(timestr.size() < 8) { //xx:xx:xx
		errorval = true;
		return 0;
	}
	std::string s_m = timnestr.substr(0, 2);
	std::string _d1 = timnestr.substr(2, 1);
	std::string s_s = timnestr.substr(3, 2);
	std::string _d2 = timnestr.substr(5, 1);
	std::string s_f = timnestr.substr(6, 2);

	if((_d1 != ":") || (_d2 != ":")) {
		// delimiter(s) not found
		errorval = true;
		return 0;
	}
	bool error1, error2, error3;
	error1 = false;
	error2 = false;
	error3 = false;
	uint64_t mm = get_val_from_bcdstr(s_m, error1);
	uint64_t ss = get_val_from_bcdstr(s_s, error2);
	uint64_t ff = get_val_from_bcdstr(s_f, error3);
	if((error1) || (error2) || (error3)) {
		errorval = true;
		return 0;
	}
	if(mm > 99) {
		errorval = true;
		return 0;
	}
	if(ss > 59) {
		errorval = true;
		return 0;
	}
	if(ff > 74) {
		errorval = true;
		return 0;
	}
	uint64_t nt = (mm * 60 * 75) + (ss * 75) + ff;
	errorval = false;
	return nt;
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
	toc_table[num].physical_size = 2352;
	toc_table[num].logical_size = 2048;
	toc_table[num].real_physical_size = 2352;
	toc_table[num].current_bytes_offset = 0;
	
	memset(toc_table[num].filename, 0x00, sizeof(_TCHAR) * _MAX_PATH);
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
