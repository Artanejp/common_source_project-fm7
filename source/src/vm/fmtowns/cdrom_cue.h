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
	int parsed_track;
	uint64_t current_lba_offset;
	uint64_t current_lba_size;
	uint64_t absolute_lba;
	uint64_t current_bytes_offset;
	std::string current_file_name;
	/*!
	 * @brief Parse CUE sheet, check track data and construct tracks table.
	 * @param filename filename (absolute full path) of sheet (or ISO data).
	 * @return true if succeeded.
	 * @todo Implement re-calculate multiple tracks inside of ONE IMAGE.
	 */
	virtual bool parse_sheet();
	/*!
	 * @brief Parse arguments for filename.
	 * @param line source line.
	 * @param arg1 answer arg1.
	 * @param arg_filename answer arg2 (filename).
	 * @param arg_type answer arg3 (type).
	 * @return Numbers of arguments.
	 * @note arg_filename don't remove delimiters this is for filename contains some space characters.
	 */
	virtual int parse_args_filename(std::string line, std::string& arg1, std::string& arg_filename, std::string& arg_type);
	/*!
	 * @brief Parse arguments.
	 * @param line source line.
	 * @param arg1 answer arg1.
	 * @param arg2 answer arg2.
	 * @param arg2 answer arg3.
	 * @return Numbers of arguments.
	 * @note arguments should be less-equal than 3.
	 */
	virtual int parse_args_std(std::string line, std::string& arg1, std::string arg2, std::string arg3);
	/*!
	 * @brief RE-Calcurate TOC table.
	 * @param num track number.
	 */
	virtual void recalc_index_table(int num);
	
public:
	/*!
	 * @brief constructor
	 */
	CDIMAGE_CUE();
	/*!
	 * @brief de-constructor
	 * @note Please implement de-allocating tracks list 
	 * in de-constructor if you need.
	 */
	~CDIMAGE_CUE();	
};










	


