#pragma once

/*!
 * @file cdrom_cue.h
 * @brief CUE CDROM virtual image definitions at new CD-ROM class for eFM-Towns.
 * @author K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * @date 2021-03-11
 * @copyright GPLv2
 */
#include "./cdrom_skelton.h"

class DLL_PREFIX CDIMAGE_CUE : public CDIMAGE_SKELTON {
protected:
	/*!
	 * @brief Parse CUE/CCD sheet, check track data and construct tracks table.
	 * @param filename filename (absolute full path) of sheet (or ISO data).
	 * @return true if succeeded.
	 */
	virtual bool parse_sheet();
	
public:
	CDIMAGE_CUE();
	~CDIMAGE_CUE();	
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
	 * @note Changing size of data by type of Virtual image.
	 */
	virtual ssize_t read_mode2(uint8_t *buf, ssize_t buflen, size_t sectors = 1, bool _clear = false);
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
	virtual ssize_t read_cdda(uint8_t *buf, ssize_t buflen, size_t sectors = 1, bool _clear = false);
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
	 * @brief Load / Save state to VM.
	 * @param state_fio FILE IO for state loading/saving.
	 * @param loading If true loading, false is saving.
	 * @return true if succeeded.
	 */
	virtual bool process_state(FILEIO* state_fio, bool loading);
};










	


