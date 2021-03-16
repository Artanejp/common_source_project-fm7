/*!
 * @file cdrom_cue.cpp
 * @brief Cue methods of new CD-ROM class for eFM-Towns.
 * @author K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * @date 2021-03-17
 * @copyright GPLv2
 */
#include <cmath>
#include <stdlib.h>
#include <cctype>

#include "../../fileio.h"
#include "./cdrom_cue.h"

/*!
 * @brief constructor
 */
CDROM_CUE::CDIMAGE_CUE()
{
}

/*!
 * @brief de-constructor
 * @note Please implement de-allocating tracks list 
 * in de-constructor if you need.
 */
CDROM_CUE::~CDIMAGE_CUE()
{
}

/*!
 * @brief Parse CUE sheet, check track data and construct tracks table.
 * @return true if succeeded.
 * @note Must open sheet file before using.
 * @todo Implement re-calculate multiple tracks inside of ONE IMAGE.
 */
bool CDROM_CUE::parse_sheet()
{
	if(!(CDROM_SKELTON::parse_sheet())) return false; // Failed to prepare.

	if(sheet_fio->Fseek(0, FILEIO_SEEK_SET) != 0) {
		return false; // Failed to rewined TOP.
	}
	_TCHAR linebuf[1024] = {0};
	int linecount = 0;
	parsed_track = 0;
	current_lba_offset = 0;
	current_lba_size = 0;
	absolute_lba = 0;
	current_bytes_offset = 0;
	current_filename.clear();
	do {
		memset(linebuf, 0x00, sizeof(linebuf));
		if(sheet_fio->Fgets(linebuf, (sizeof(linebuf) / sizeof(_TCHAR)) - 1) == NULL) {
			break; // EOT
		}
		// Parse linebuf
		std::string pstr = linebuf;
		if(pstr.empty()) continue;

		std::string ustr = to_upper(pstr); // ToUpper'ed string
		// Tokenize
		bool first = true;
		if(ustr.find(_T("REM")) != basic_string::npos) {
			linecount++;
			continue;
		} else if(ustr.find(_T("FILE")) != basic_string::npos) {
			// Get a file name
			std::string arg1;
			std::string filename;
			std::string s_type;
			int args = parse_args_filename(pstr, arg1, filename, s_type);
			if(args < 2) {
				linecount++;
				continue;
			}
			if(!(filename.empty())) {
				recalc_index_table(parsed_track);
				current_lba_offset = 0;
				current_lba_size = 0;
				current_bytes_offset = 0;
				current_filename = filename;
				first = true;
			}
		} else if(ustr.find(_T("TRACK")) != basic_string::npos) {
			std::string arg1str;
			std::string trackstr;
			std::string typestr;
			if(parse_args_std(ustr, arg1str, trackstr, typestr) < 2) {
				linecount++;
				continue;
			}
			if(!(trackstr.empty())) {
				// Check Track num
				int n = atoi(trackstr.c_str());
				if((n > 0) && (n < 100)) {
					// compare arg3
					bool enabled = false;
					if(typestr.compare(_T("AUDIO")) == 0) {
						enabled = true;
						toc_table[n].available = true;
						toc_table[n].type = TRACKTYPE_AUDIO;
						toc_table[n].physical_size = 2352;
						toc_table[n].logical_size = 2352;
						toc_table[n].real_physical_size = 2352;
					} else if(typestr.compare(_T("MODE1/2048")) == 0) {
						enabled = true;
						toc_table[n].available = true;
						toc_table[n].type = TRACKTYPE_MODE1_2048;
						toc_table[n].physical_size = 2352;
						toc_table[n].logical_size = 2048;
						toc_table[n].real_physical_size = 2048;
					} else if(typestr.compare(_T("MODE1/2352")) == 0) {
						enabled = true;
						toc_table[n].available = true;
						toc_table[n].type = TRACKTYPE_MODE1_2352;
						toc_table[n].physical_size = 2352;
						toc_table[n].logical_size = 2048;
						toc_table[n].real_physical_size = 2352;
					} else if(typestr.compare(_T("MODE2/2336")) == 0) {
						enabled = true;
						toc_table[n].available = true;
						toc_table[n].type = TRACKTYPE_MODE2_2336;
						toc_table[n].physical_size = 2352;
						toc_table[n].logical_size = 2336;
						toc_table[n].real_physical_size = 2336;
					} else if(typestr.compare(_T("MODE2/2352")) == 0) {
						enabled = true;
						toc_table[n].available = true;
						toc_table[n].type = TRACKTYPE_MODE2_2352;
						toc_table[n].physical_size = 2352;
						toc_table[n].logical_size = 2336;
						toc_table[n].real_physical_size = 2352;
					} else if(typestr.compare(_T("CDI/2336")) == 0) {
						enabled = true;
						toc_table[n].available = true;
						toc_table[n].type = TRACKTYPE_CDI_2336;
						toc_table[n].physical_size = 2352;
						toc_table[n].logical_size = 2336;
						toc_table[n].real_physical_size = 2336;
					} else if(typestr.compare(_T("CDI/2352")) == 0) {
						enabled = true;
						toc_table[n].available = true;
						toc_table[n].type = TRACKTYPE_CDI_2352;
						toc_table[n].physical_size = 2352;
						toc_table[n].logical_size = 2336;
						toc_table[n].real_physical_size = 2352;
					} else if(typestr.compare(_T("CDG")) == 0) { // OK?
						enabled = true;
						toc_table[n].available = true;
						toc_table[n].type = TRACKTYPE_CDI_2352;
						toc_table[n].physical_size = 2352;
						toc_table[n].logical_size = 2352;
						toc_table[n].real_physical_size = 2352;
					}
					if((enabled) /*|| (first)*/) {
						recalc_index_table(parsed_track);
						absolute_lba += current_lba_size;
						if(!(first)) {
							current_lba_offset += current_lba_size;
							current_bytes_offset += (current_lba_size * toc_table[parsed_track].real_physical_size);
						}
						first = false;
						parsed_track = n;	
					}
				}
			}
		} else if(ustr.find(_T("INDEX")) != basic_string::npos) {
			if((parsed_track > 0) && (parsed_track < 100)) {
				std::string arg1str;
				std::string indexstr;
				std::string timestr;
				bool errorval = false;
				if(parse_args_std(ustr, arg1str, indexstr, timestr) < 3) {
					linecount++;
					continue;
				}
				if(indexstr.size() != 2) { //xx
					linecount++;
					continue;
				}
				if((indexstr.empty()) || (timestr.empty())) {
					linecount++;
					continue;
				}
				int indexnum = atoi(indexstr.c_str());
				int64_t framepos = get_frames_from_msfstr(timestr, errorval);
				if(errorval) {
					linecount++;
					continue;
				}
				switch(indexnum) {
				case 0:
					toc_table[parsed_track].index0 = framepos;
					break;
				case 1:
					toc_table[parsed_track].index1 = framepos;
					break;
				}
//				recalc_index_table(parsed_track);
			}
		} else if(ustr.find(_T("PREGAP")) != basic_string::npos) {
			if((parsed_track > 0) && (parsed_track < 100)) {
				std::string arg1str;
				std::string timestr;
				std::string arg3str;
				bool errorval = false;
				if(parse_args_std(ustr, arg1str, timestr, arg3str) < 2) {
					linecount++;
					continue;
				}
				int64_t framepos = get_frames_from_msfstr(timestr, errorval);
				if(errorval) {
					linecount++;
					continue;
				}
				toc_table[parsed_track].pregap = framepos;
				//recalc_index_table(parsed_track);
			}
		}
		linecount++;
	} while(1);
	if(linecount <= 0) return false;
	// Re-Calc last track
	recalc_index_table(parsed_track);
	// Calc tail.
	absolute_lba += current_lba_size;
	current_lba_offset += current_lba_size;
	current_bytes_offset += (current_lba_size * toc_table[parsed_track].real_physical_size);

	recalc_index_table(parsed_track + 1);
	
	return true;
}

/*!
 * @brief RE-Calcurate TOC table.
 * @param num track number.
 */
void CDROM_CUE::recalc_index_table(int num)
{
	if((num < 0) || (num > 100)) return;
	int64_t index0 = toc_table[num].index0;
	int64_t index1 = toc_table[num].index1;
	int64_t pregap = toc_table[num].pregap;
	int64_t lbasize = 0;

	if(pregap < 150) {
		pregap = 150;
	}
	if(index0 < index1) {
		lbasize = index1 - index0;
	} else if(index0 > index1) {
		lbasize = index0 - index1;
	}
	if(lbasize < pregap) {
		lbasize = 0;
	} else {
		lbasize -= pregap;
	}
	
	if(lbasize < 0) {
		lbasize = 0;
	}
	current_lba_size = lbasize;
		
	toc_table[num].absolute_lba = absolute_lba;
	toc_table[num].lba_offset = current_lba_offset;
	toc_table[num].bytes_offset = current_bytes_offset;
	toc_table[num].lba_size = lbasize;

	toc_table[num].pregap = pregap;
	toc_table[num].index0 = index0;
	toc_table[num].index1 = index1;
	
	memset(toc_table[num].filename, 0x00, sizeof(_TCHAR) * _MAX_PATH);
	if(!(current_filename.empty())) {
		current_filename.copy(toc_table[num].filename, _MAX_PATH - 1);
	}
}
/*!
 * @brief Parse arguments for filename.
 * @param line source line.
 * @param arg1 answer arg1.
 * @param arg_filename answer arg2 (filename).
 * @param arg_type answer arg3 (type).
 * @return Numbers of arguments.
 * @note arg_filename don't remove delimiters this is for filename contains some space characters.
 */
int CDROM_CUE::parse_args_filename(std::string line, std::string& arg1, std::string& arg_filename, std::string& arg_type)
{
	if(line.empty()) return 0;

	size_type pos1l = line.find_first_of(_T(" \t\n\r"));
	if(pos1l == basic_string::npos) return 0;
	
	size_type pos2b = line.find_first_not_of(_T(" \t\n\r"), pos1l);
	if(pos2b == basic_string::npos) return 0;

	size_type pos2l = line.find_first_of(_T(" \t\n\r"), pos2b);
	if(pos2l == basic_string::npos) {
		pos2l = line.size();
	}
	arg1.substr(line, 0, pos1l);
	if(pos2b >= pos2l) return 1;
	arg2.substr(line, pos2b, pos2l);
	return 2;
}

/*!
 * @brief Parse arguments.
 * @param line source line.
 * @param arg1 answer arg1.
 * @param arg2 answer arg2.
 * @param arg2 answer arg3.
 * @return Numbers of arguments.
 * @note arguments should be less-equal than 3.
 */
int CDROM_CUE::parse_args_std(std::string line, std::string& arg1, std::string arg2, std::string arg3)
{
	if(line.empty()) return 0;

	size_type pos1l = line.find_first_of(_T(" \t\n\r"));
	if(pos1l == basic_string::npos) return 0;
	
	size_type pos2b = line.find_first_not_of(_T(" \t\n\r"), pos1l);
	if(pos2b == basic_string::npos) return 0;

	size_type pos2l = line.find_first_of(_T(" \t\n\r"), pos2b);
	if(pos2l == basic_string::npos) {
		pos2l = line.size();
	}

	arg1.substr(line, 0, pos1l);
	if(pos2b >= pos2l) return 1;
	
	arg2.substr(line, pos2b, pos2l);
	
	size_type pos3b = line.find_first_not_of(_T(" \t\n\r"), pos2l);
	if(pos3b == basic_string::npos){
		return 2;
	}

	size_type pos3l = line.find_first_of(_T(" \t\n\r"), pos3b);
	if(pos3l == basic_string::npos) {
		pos3l = line.size();
	}
	
	arg3.substr(line, pos3b, pos3l);
	return 3;
}

