/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.31 -

	[FM-Towns CD-ROM : CUE PARSER]
*/

#include "../cdrom.h"
#include "../../../fileio.h"

#include <map>

namespace FMTOWNS {

enum {
	CUE_NONE = 0,
	CUE_REM,
	CUE_FILE,
	CUE_TRACK,
	CUE_INDEX,
	CUE_PREGAP,
};

bool TOWNS_CDROM::check_toc_and_open_image_file(CDROM_TOC_TABLE_t *pt, uint64_t& image_length, uint64_t& image_offset, bool& is_image_changed, std::string& recent_path, std::string current_data_path)
{
	if(pt == NULL) return false;
	is_image_changed = false;
	bool is_valid_image = false;
	

	bool need_to_reopen = false;
	if(recent_path.empty()) need_to_reopen = true;
	if(recent_path.compare(current_data_path) != 0) need_to_reopen |= true;
	
	if(need_to_reopen) {
		if(current_data_path.empty()) return false;
		if(!(FILEIO::IsFileExisting((_TCHAR *)(current_data_path.c_str())))) return false;
		// Ok, Try to Open File.
		FILEIO tmp_fio;
		if(!(tmp_fio.Fopen((_TCHAR *)(current_data_path.c_str()), FILEIO_READ_BINARY))) {
			return false;
		}
		size_t tmp_len = tmp_fio.FileLength();
		if(tmp_len <= 0) {
			return false;
		}
		// OK, Open new file.
		tmp_fio.Fclose();
		
		if(fio_img->IsOpened()) {
			fio_img->Fclose();
		}
		if(!(fio_img->Fopen((_TCHAR *)(current_data_path.c_str()), FILEIO_READ_BINARY))) {
			return false;
		}
		// OK!
		fio_img->Fseek(0, FILEIO_SEEK_SET);
		
		tmp_len = fio_img->FileLength();
		if(tmp_len <= 0) {
			return false;
		}
		image_length = (uint64_t)tmp_len;
		is_image_changed = true;
		
		image_offset = 0;
		recent_path = current_data_path;
		return true;
	}
	if(!(fio_img->IsOpened())) {
		return false;
	}
	return true;
}

/* New open_cue_file() */
std::string TOWNS_CDROM::read_line_from_text(FILEIO* fio, bool& is_eof)
{
	std::string retstr;
	is_eof = false;
	retstr.clear();
	if(fio == NULL) {
		return retstr;
	}
	if(!(fio->IsOpened())) {
		return retstr;
	}
	bool is_eoln = false;
	do {
		int _c = fio->Fgetc();
		switch(_c) {
		case EOF:
			is_eof = true; 
			break;
		case '\0': /* OK? */
			is_eof = true;
			break;
		case '\n': /* LF */
			is_eoln = true;
			break;
		case '\r': /* CR */
			// ToDo: Process for Older MAC string, terminated by only CR .
			break;
		default:
			retstr.push_back((char)_c);
			break;
		}
	} while(!(is_eof) && !(is_eoln));
	return retstr;
}

std::string TOWNS_CDROM::to_upper(std::string source)
{
	std::transform(source.begin(), source.end(), source.begin(),
				   [](unsigned char c) -> unsigned char{ return std::toupper(c); });
	return source;
}

std::string TOWNS_CDROM::sanitize_eoln(std::string source)
{
	std::string retstr;
	retstr.clear();
	if(source.empty()) {
		return retstr;
	}
	const std::regex _ts("[ \t\n\r]+$");
	std::smatch _sm;
	if(std::regex_search(source, _sm, _ts)) {
		retstr = _sm.prefix();
	} else {
		retstr = source;
	}
	return retstr;
}

std::string TOWNS_CDROM::sanitize_prev_space_tab(std::string source)
{
	std::string retstr;
	retstr.clear();
	if(source.empty()) {
		return retstr;
	}
	const std::regex _ts("^[ \t]*");
	std::smatch _sm;
	if(std::regex_search(source, _sm, _ts)) {
		retstr = _sm.suffix();
	} else {
		retstr = source;
	}
	return retstr;
}
	
	
std::vector<std::string> TOWNS_CDROM::tokenize_a_line(std::string source, size_t& tokens)
{
	std::vector<std::string> tmplist;
	
	tmplist.clear();
	tokens = 0;
	
	if(!(source.empty())) {
		std::string line1;
		// Split header.
		line1 = sanitize_prev_space_tab(source);
		if(line1.empty()) { // Assume empty line.
			return tmplist;
		}
		
		// Check Quote
		std::string _tmps;
		bool within_quote = false;
		bool need_break = false;
		for(auto _c = line1.begin(); _c != line1.end(); ++_c) {
			switch(*(_c)) {
			case '\"': // Quote
				if(within_quote) { // END OF QUOTE
					tmplist.push_back(_tmps);
					_tmps.clear();
					within_quote = false;
				} else {
					within_quote = true;
				}
				break;
			case '\0': // EOLN?
				need_break = true;
				break;
			case '\r': // CR : Ignore?
				break;
			case '\n':
				need_break = true;
				break;
			case '\t':
			case ' ': // Split token
				if(within_quote) {
					_tmps.push_back((*_c));
				} else {
					if(!(_tmps.empty())) {
						tmplist.push_back(_tmps);
						_tmps.clear();
					}
				}
				break;
			default:
				_tmps.push_back((*_c));
				break;
			}
			if(need_break) {
				break;
			}
		}
		if(!(_tmps.empty())) {
			tmplist.push_back(_tmps);
			_tmps.clear();
		}
		if(tmplist.empty()) { // Fallback
			tmplist.push_back(line1);
		}
		tokens = (size_t)(tmplist.size());
	}
	return tmplist;
}
bool TOWNS_CDROM::open_cue_file(const _TCHAR* file_path)
{
	std::string line_buf;
	std::string image_tmp_data_path;
	std::string image_tmp_data_type;

	_TCHAR full_path_cue[_MAX_PATH];
	size_t ptr;
	int line_count = 0;
	int slen;
	int nr_current_track = 0;
	bool is_valid_image = false;
	FILEIO* fio = new FILEIO();
	if(fio == NULL) return false;

	memset(full_path_cue, 0x00, sizeof(full_path_cue));

	get_long_full_path_name(file_path, full_path_cue, sizeof(full_path_cue));

	const _TCHAR *parent_dir = get_parent_dir((const _TCHAR *)full_path_cue);

	std::string tmp_track_data_path[101];
	std::string tmp_track_data_type[101];
	
	std::map<std::string, int> cue_enum;

	// Initialize
	cue_enum.insert(std::make_pair("REM", CUE_REM));
	cue_enum.insert(std::make_pair("FILE", CUE_FILE));
	cue_enum.insert(std::make_pair("TRACK", CUE_TRACK));
	cue_enum.insert(std::make_pair("INDEX", CUE_INDEX));
	cue_enum.insert(std::make_pair("PREGAP", CUE_PREGAP));

	CDROM_TOC_TABLE_t toc_table_tmp[101];
	for(int i = 0; i < 101; i++) {
		initialize_toc_table(&(toc_table_tmp[i]));
	}

	int tmp_track_num = 0;
	if(fio->Fopen(full_path_cue, FILEIO_READ_ASCII)) { // ToDo: Support not ASCII cue file (i.e. SJIS/UTF8).20181118 K.O
		bool is_eof = false;
		int sptr = 0;
		bool have_filename = false;
		std::vector<std::string> _tokens;
		size_t tokens_count;
		// Pass 1: Parse from cue file.
		while(1) {
			line_buf = read_line_from_text(fio, is_eof);
			tokens_count = 0;
			_tokens.clear();
			if(!(line_buf.empty())) {
				_tokens = tokenize_a_line(line_buf, tokens_count);
			}
			//for(auto p = _tokens.begin(); p != _tokens.end(); ++p) {
			//	out_debug_log(_T("TOKEN: %s"), (*p).c_str());
			//}
			if(tokens_count > 0) {
				// Extract command
				std::string cmdstr = sanitize_eoln(_tokens[0]);
				std::string arg1str;
				std::string arg2str;
				int tmp_track;
				cmdstr = to_upper(cmdstr);
				int typeval;
				try {
					typeval = cue_enum.at(cmdstr);
				} catch (std::out_of_range &e) {
					typeval = CUE_NONE;
				}
				arg1str.clear();
				arg2str.clear();
				if(tokens_count > 1) {
					arg1str = _tokens[1]; // Still don't last spaces, because may include them at filename.
				}
				if(tokens_count > 2) {
					arg2str = _tokens[2]; // Still don't last spaces, because may include them at filename.
				}
				switch(typeval) {
				case CUE_REM:
					//out_debug_log(_T("TRACK#%02d REM:%s\n"),  nr_current_track, line_buf.c_str());
					break;
				case CUE_FILE:
					if(!(arg1str.empty())) {
						std::string __prefix;
						if(parent_dir != NULL) {
							__prefix = (const char*)parent_dir;
						}
						image_tmp_data_path = __prefix;
						image_tmp_data_path.append(arg1str);
						image_tmp_data_type = arg2str;
						out_debug_log(_T("TRACK#%02d FILE:\"%s\" TYPE=%s\n"),  nr_current_track, image_tmp_data_path.c_str(), image_tmp_data_type.c_str());
					}
					break;
				case CUE_INDEX:
					// By https://totalsonic.net/cuesheetsyntax.htm :
					// " All index numbers must be between 0 and 99 inclusive.
					//   The first index must be 0 or 1 with
					//   all other indexes being sequential to the first one.
					//   The first index of a file must start at 00:00:00.
					//   INDEX 0 Specifies the starting time of the track “pregap”.
					//   INDEX 1 Specifies the starting time of the track data.
					//           This is the only index that is stored in the disc’s
					//           table-of-contents.
					//   INDEX > 1 Specifies a subindex within a track. "
					// - 20250223 K.O

					{
						uint32_t msf = 0;
						int _index = parse_cue_index(arg1str, arg2str, nr_current_track, msf);
						switch(_index) {
						case 0:
							toc_table_tmp[nr_current_track].index0 = msf;
							break;
						case 1:
							toc_table_tmp[nr_current_track].index1 = msf;
							break;
						default:
							// ToDo: index > 1 
							break;
						}
						out_debug_log(_T("TRACK#%02d INDEX=%02d MSF=%s\n"),  nr_current_track, _index, arg2str.c_str());
					}
					break;
				case CUE_TRACK:
					tmp_track = nr_current_track;
					if(parse_cue_track(arg1str, arg2str, &(toc_table_tmp[0]), tmp_track, tmp_track_num)) {
						if(tmp_track > 0) {
							nr_current_track = tmp_track;
							tmp_track_data_path[nr_current_track] = image_tmp_data_path;
							tmp_track_data_type[nr_current_track] = image_tmp_data_type;
						}
					}
					out_debug_log(_T("TRACK#%02d TRACK:%s TYPE:%s\n"),  nr_current_track, arg1str.c_str(), arg2str.c_str());
					break;
				case CUE_PREGAP:
					if((nr_current_track > 0) && (nr_current_track < 100) && !(arg1str.empty())) {
						toc_table_tmp[nr_current_track].pregap = get_frames_from_msf(arg1str.c_str());
					}
					out_debug_log(_T("TRACK#%02d PREGAP:%s\n"),  nr_current_track, arg1str.c_str());
					break;
				default:
					//out_debug_log(_T("TRACK#%02d ???:%s\n"),  nr_current_track, line_buf.c_str());
					break;
				}
			}
			if(is_eof) break;
		}
		fio->Fclose();
	} else {
		delete fio;
		return false; // Failed to open.
	}
	delete fio;
	fio = NULL;
	
	if((tmp_track_num <= 1) || (tmp_track_num > 100)) {
		return false; // No means at this file or too much tracks.
	}
	// Pass2 : Index.
	out_debug_log(_T("Pass 2: Index."));
	max_logical_block = 0;
	uint32_t lba_ptr = 0;
	//toc_table[0].lba_offset = 0;
	toc_table_tmp[0].lba_size = 0;
	toc_table_tmp[0].index0 = toc_table_tmp[0].index1 = toc_table_tmp[0].pregap = 0;
	toc_table_tmp[0].bytes_offset = 0;
	toc_table_tmp[0].physical_size = 2352;
	toc_table_tmp[0].logical_size = 2048;
	
	is_valid_image = true;
	
	for(int i = 1; (i < tmp_track_num) && (i < 100) ; i++) {
		CDROM_TOC_TABLE_t *pt = &(toc_table_tmp[i]);
		if(pt == NULL) {
			is_valid_image = false;
			break; // NG.
		}
		if(pt->type >= MODE_NONE) {
			is_valid_image = false; // Illegal?
			break;
		}
		int tmp_index0 = pt->index0;
		int tmp_index1 = pt->index1;
		int tmp_pregap = pt->pregap;
		// From https://totalsonic.net/cuesheetsyntax.htm :
		// PREGAP [mm:ss:ff]
		// Parameters: mm:ss:ff – Specifies the pregap length
		//             in minutes, seconds, and frames.
		// Example: PREGAP 00:02:00
		// Rules: The PREGAP command must appear after a TRACK command,
		// but before any INDEX commands.
		// Only one PREGAP command is allowed per track.
		// - 20250223 K.O
		// So, I decide below rule:
		const int tmp_2sec = get_frames_from_msf(_T("00:02:00"));
		if(tmp_index0 < 0) {
			tmp_index0 = 0;
		}
		if(tmp_index1 < 0) {
			tmp_index1 = 0;
		}
		if(tmp_pregap <= 0) {
			tmp_pregap = tmp_2sec;
			if((tmp_index1 > tmp_index0) && (tmp_index0 > 0)) {
				tmp_pregap = tmp_index1 - tmp_index0;
			}
		}
		// ToDo: if pregap < 2Sec.
		if((tmp_index1 == 0) /*&& (tmp_index0 > 0) */) {
			tmp_index1 = tmp_index0 + tmp_pregap;
		}
		if(tmp_index0 == 0) { // Maybe tmp_index1 != 0
			tmp_index0 = tmp_index1 - tmp_pregap;
			if(tmp_index0 < 0) {
				tmp_index0 = 0;
			}
		}
		pt->index0 = tmp_index0;
		pt->index1 = tmp_index1;
		pt->pregap = tmp_pregap;
	}
	//
	if(!(is_valid_image)) {
		return false;
	}
	// Pass 3: Trim image.
	out_debug_log(_T("Pass 3: Trim image."));
	// Debug
	for(int i = 0; (i <= tmp_track_num) && (i <= 100) ; i++) {
		cdrom_debug_log(_T("TRACK %03d TYPE=%d INDEX0=%06x INDEX1=%06x PREGAP=%06x PHYS_SIZE=%d LOGI_SIZE=%d LBA_SIZE=%d BYTES_OFFSET=%d DATA=%s"), i, toc_table_tmp[i].type, lba_to_msf(toc_table_tmp[i].index0),  lba_to_msf(toc_table_tmp[i].index1),  lba_to_msf(toc_table_tmp[i].pregap),  toc_table_tmp[i].physical_size, toc_table_tmp[i].logical_size, toc_table_tmp[i].lba_size, toc_table_tmp[i].bytes_offset, tmp_track_data_path[i].c_str());
	}
	uint64_t __offset_bytes = 0;
	uint64_t __image_size   = 0;
	bool is_image_changed = false;
	uint32_t lba_begin = 0;
	uint64_t base_sectors = 0;
	uint64_t total_bytes = 0;
	uint64_t total_sectors = 0;
	int latest_changed = 1;
	std::string __recent_path;

	is_valid_image = true;
	for(int i = 1; (i < tmp_track_num) && (i < 100) ; i++) {
		uint64_t old_image_size = __image_size;
		uint64_t old_offset = __offset_bytes;
		if(!(check_toc_and_open_image_file(&(toc_table_tmp[i]), __image_size, __offset_bytes, is_image_changed, __recent_path, tmp_track_data_path[i]))) {
			tmp_track_data_path[i].clear();
			is_valid_image = false;
			cdrom_debug_log(_T("INVALID IMAGE DETECTED at TRACK#%02d , LINE %d ."), i, __LINE__); 
			break;
		}
		int64_t tmp_sector_size = get_sector_size_from_mode(toc_table_tmp[i].type);
		if(tmp_sector_size <= 0) {
			is_valid_image = false;
			cdrom_debug_log(_T("INVALID IMAGE DETECTED at TRACK#%02d , LINE %d ."), i, __LINE__); 
			break;
		}
		toc_table_tmp[i].physical_size = tmp_sector_size;
		toc_table_tmp[i].logical_size = get_logical_size_from_mode(toc_table_tmp[i].type);

		if(i == 1) {
//						toc_table_tmp[i].lba_size = (__image_size - __offset_bytes) / tmp_sector_size; // TMP
//						toc_table_tmp[i].bytes_offset = 0;
		} else {
			int64_t old_sector_size = toc_table_tmp[i - 1].physical_size;
			if(old_sector_size < 0) {
				is_valid_image = false;
				cdrom_debug_log(_T("INVALID IMAGE DETECTED at TRACK#%02d , LINE %d ."), i, __LINE__); 
				break;
			}
			uint64_t __sectors = 0;
			if(is_image_changed) {
				// Trim index0 and index1
				base_sectors = total_sectors;
				for(int j = latest_changed; j < i; j++) {
					//toc_table_tmp[j].lba_offset = base_sectors;
					toc_table_tmp[j].index0 += base_sectors;
					toc_table_tmp[j].index1 += base_sectors;
				}
				latest_changed = i;
				if(old_image_size >= old_offset) {
					__sectors = (old_image_size - old_offset) / old_sector_size;
					toc_table_tmp[i - 1].lba_size = __sectors;
					toc_table_tmp[i].bytes_offset = 0;
				} else {
					is_valid_image = false;
					cdrom_debug_log(_T("INVALID IMAGE DETECTED at TRACK#%02d , LINE %d ."), i, __LINE__); 
					break;
				}
				total_bytes += old_image_size;
			} else { // Continue
				uint64_t old_index0 = toc_table_tmp[i - 1].index0;
				uint64_t old_index1 = toc_table_tmp[i - 1].index1;
				uint64_t new_index0 = toc_table_tmp[i].index0;
				uint64_t new_index1 = toc_table_tmp[i].index1;
				if((new_index0 >= old_index0) && (new_index1 >= old_index1)) {
					__sectors = new_index0 - old_index0;
					uint64_t __size = __sectors * tmp_sector_size;
					//if(__sectors == 0) {
					// Eject?
					//	is_valid_image = false;
					//	break;
					//}
					//if(__image_size < (__size + __offset_bytes)) {
					// Eject?
					//	is_valid_image = false;
					//	break;
					//}
					__offset_bytes += __size;
					toc_table_tmp[i].bytes_offset = __offset_bytes;
				} else {
					__sectors = 0;
				}
			}
			toc_table_tmp[i - 1].lba_size = __sectors;
			total_sectors += __sectors;
		}
	}
	if(!(is_valid_image)) {
		return false;
	}
	// Trim remain INDEX 00 and INDEX 01 .
	out_debug_log(_T("Pass 4: Trim remain INDEX 00 and INDEX 01."));
	if((latest_changed < tmp_track_num) && (latest_changed > 1)) {
		for(int j = latest_changed; j < tmp_track_num; j++) {
			//toc_table_tmp[j].lba_offset = base_sectors;
			toc_table_tmp[j].index0 += base_sectors;
			toc_table_tmp[j].index1 += base_sectors;
		}
	}
	// TRIM TRACK 00 .
	toc_table_tmp[0].index0       = 0;
	toc_table_tmp[0].index1       = 0;
	toc_table_tmp[0].pregap       = 0;
	toc_table_tmp[0].lba_size     = 0;
	toc_table_tmp[0].physical_size = 2352;
	toc_table_tmp[0].logical_size = 2048;
	toc_table_tmp[0].bytes_offset = 0;
	tmp_track_data_path[0] = std::string("");
	tmp_track_data_type[0] = std::string("");
				
	// TRIM TRACN [track_num]
	if((__image_size > 0) && (toc_table_tmp[tmp_track_num - 1].physical_size > 0)) {
		if(__image_size > toc_table[tmp_track_num - 1].bytes_offset) {
			uint64_t __last_sectors = (__image_size - toc_table[tmp_track_num - 1].bytes_offset) / toc_table_tmp[tmp_track_num - 1].physical_size;
			toc_table_tmp[tmp_track_num - 1].lba_size = __last_sectors;
			total_sectors += __last_sectors;
		} else {
			//	is_valid_image = false;
		}
	}
	toc_table_tmp[tmp_track_num].index0       = total_sectors;
	toc_table_tmp[tmp_track_num].index1		  = total_sectors;
	toc_table_tmp[tmp_track_num].pregap       = 0;
	toc_table_tmp[tmp_track_num].lba_size     = 0;
	toc_table_tmp[tmp_track_num].bytes_offset = total_bytes;
	toc_table_tmp[tmp_track_num].physical_size = 2352;
	toc_table_tmp[tmp_track_num].logical_size = 2048;
	tmp_track_data_path[tmp_track_num] = std::string("");
	tmp_track_data_type[tmp_track_num] = std::string("");

	if(is_valid_image) {
		out_debug_log(_T("Pass 5: Check Image Exists and seekable."));
		// Check image exists and seekable.
		for(int i = 1; (i < tmp_track_num) && (i < 100) ; i++) {
			if(tmp_track_data_path[i].empty()) {
				is_valid_image = false;
				break;
			}
			if(fio_img->IsOpened()) { // OK?
				fio_img->Fclose();
			}
			if(fio_img->Fopen(tmp_track_data_path[i].c_str(), FILEIO_READ_BINARY)) {
				if(fio_img->Fseek((long)(toc_table_tmp[i].bytes_offset), FILEIO_SEEK_SET) != 0) {
					is_valid_image = false;
				}
				fio_img->Fclose();
			} else {
				is_valid_image = false;
			}
			if(!(is_valid_image)) {
				break;
			}
		}
	}
	if(fio_img->IsOpened()) { // OK?
		fio_img->Fclose();
	}
	if(!(is_valid_image)) {
		return false;
	}
	// Valid track.
	track_num = tmp_track_num;
	max_logical_block = total_sectors;
	// ToDo : Copy toc_table.
						
	for(int i = 0; i < 101; i++) {
		initialize_toc_table(&(toc_table[i]));
	}
	for(int i = 0; i <= track_num; i++) {
		copy_toc_table_to_main(i, &(toc_table_tmp[i]), tmp_track_data_path[i], tmp_track_data_type[i]);
	}
	return true;
}
	

// ToDo: Implement Image type.
#if 1
bool TOWNS_CDROM::parse_cue_track(std::string track_num_str, std::string track_type_str, CDROM_TOC_TABLE_t* toc_table_tmp, int& nr_current_track, int& max_track_num)
{
	if(toc_table_tmp == NULL) return false;
	bool result = false;

	std::string _tmp_track_num = sanitize_eoln(track_num_str);
	_tmp_track_num = sanitize_prev_space_tab(_tmp_track_num);
	
	std::string _tmp_track_type = sanitize_eoln(track_type_str);
	_tmp_track_type = sanitize_prev_space_tab(_tmp_track_type);

	if((_tmp_track_num.empty()) || (_tmp_track_type.empty())) {
		return result;
	}
	int _nr_num = atoi(_tmp_track_num.c_str());
	_tmp_track_type = to_upper(_tmp_track_type);
	
	// Set image file
	if((_nr_num > 0) && (_nr_num < 100)) {
		std::map<std::string, CDROM_MODE_t> cue_type;
		cue_type.insert(std::make_pair("AUDIO", MODE_AUDIO));
		cue_type.insert(std::make_pair("MODE1/2048", MODE1_2048));
		cue_type.insert(std::make_pair("MODE1/2352", MODE1_2352));
		cue_type.insert(std::make_pair("MODE2/2336", MODE2_2336));
		cue_type.insert(std::make_pair("MODE2/2352", MODE2_2352));
		cue_type.insert(std::make_pair("CDI/2336", CDI_2336));
		cue_type.insert(std::make_pair("CDI/2352", CDI_2352));
		cue_type.insert(std::make_pair("CDG", CD_G));

		nr_current_track = _nr_num;
		
		toc_table_tmp[nr_current_track].type = MODE_NONE;
		toc_table_tmp[nr_current_track].is_audio = false;
		toc_table_tmp[nr_current_track].index0 = 0;
		toc_table_tmp[nr_current_track].index1 = 0;
		toc_table_tmp[nr_current_track].pregap = 0;
		toc_table_tmp[nr_current_track].physical_size = 2352;
		toc_table_tmp[nr_current_track].logical_size = 2048;
		CDROM_MODE_t track_type;
		try {
			track_type = cue_type.at(_tmp_track_type);
		} catch (std::out_of_range &e) {
			track_type = MODE_NONE;
		}
		int64_t phys_size = get_sector_size_from_mode(track_type);
		if(phys_size > 0) {
			toc_table_tmp[nr_current_track].type = track_type;
			toc_table_tmp[nr_current_track].physical_size = phys_size;
			toc_table_tmp[nr_current_track].logical_size = get_logical_size_from_mode(track_type);
			if(track_type == MODE_AUDIO) {
				toc_table_tmp[nr_current_track].is_audio = true;
			}
			result = true;
		}
		if((result) && (max_track_num < (nr_current_track + 1))) {
			max_track_num = nr_current_track + 1;
		}
	} else {
		// ToDo: 20181118 K.Ohta
		nr_current_track = 0;
	}
	return result;
}

int TOWNS_CDROM::parse_cue_index(std::string index_type, std::string time_msf, int nr_current_track, uint32_t& msf)
{
	int index = -1;
	msf = 0;
	if((index_type.empty()) || (time_msf.empty())) {
		return index;
	}
	if((nr_current_track > 0) && (nr_current_track < 100)) {
		// Get 1st ARG: INDEX NUM .
		// Get 2nd ARG: MSF .
		std::string _tmpindex = sanitize_eoln(index_type);
		_tmpindex = sanitize_prev_space_tab(_tmpindex);
		
		std::string _tmpmsf = sanitize_eoln(time_msf);
		_tmpmsf = sanitize_prev_space_tab(_tmpmsf);
		
		if(!(_tmpindex.empty())) {
			index = atoi(_tmpindex.c_str());
			if((index >= 0) && !(_tmpmsf.empty())) {
				msf = get_frames_from_msf((const char*)(_tmpmsf.c_str()));
			} else {
				index = -1;
			}
		}
	}
	return index;
}
#else
bool TOWNS_CDROM::parse_cue_track(std::string &_arg2, CDROM_TOC_TABLE_t* toc_table_tmp, int& nr_current_track)
{
	if(toc_table_tmp == NULL) return false;
	size_t _arg2_ptr_s;
	size_t _arg2_ptr;
	_arg2_ptr_s = _arg2.find_first_of((const char *)" \t");

	std::string _arg3 = _arg2.substr(_arg2_ptr_s);
	std::string _arg4;
	_arg2 = _arg2.substr(0, _arg2_ptr_s);
	size_t _arg3_ptr = _arg3.find_first_not_of((const char *)" \t");
	size_t _arg3_ptr_s;
	int _nr_num = atoi(_arg2.c_str());
	bool result = false;
	
	// Set image file
	if((_nr_num > 0) && (_nr_num < 100) && (_arg3_ptr != std::string::npos)) {
		std::map<std::string, CDROM_MODE_t> cue_type;
		cue_type.insert(std::make_pair("AUDIO", MODE_AUDIO));
		cue_type.insert(std::make_pair("MODE1/2048", MODE1_2048));
		cue_type.insert(std::make_pair("MODE1/2352", MODE1_2352));
		cue_type.insert(std::make_pair("MODE2/2336", MODE2_2336));
		cue_type.insert(std::make_pair("MODE2/2352", MODE2_2352));
		cue_type.insert(std::make_pair("CDI/2336", CDI_2336));
		cue_type.insert(std::make_pair("CDI/2352", CDI_2352));
		cue_type.insert(std::make_pair("CDG", CD_G));

		nr_current_track = _nr_num;
		_arg3 = _arg3.substr(_arg3_ptr);

		
		_arg3_ptr_s = _arg3.find_first_of((const char *)" \t\n");
		_arg4 = _arg3.substr(0, _arg3_ptr_s);

		std::transform(_arg3.begin(), _arg3.end(), _arg3.begin(),
					   [](unsigned char c) -> unsigned char{ return std::toupper(c); });

		toc_table_tmp[nr_current_track].type = MODE_NONE;
		toc_table_tmp[nr_current_track].is_audio = false;
		toc_table_tmp[nr_current_track].index0 = 0;
		toc_table_tmp[nr_current_track].index1 = 0;
		toc_table_tmp[nr_current_track].pregap = 0;
		toc_table_tmp[nr_current_track].physical_size = 2352;
		toc_table_tmp[nr_current_track].logical_size = 2048;
		CDROM_MODE_t track_type;
		try {
			track_type = cue_type.at(_arg3);
		} catch (std::out_of_range &e) {
			track_type = MODE_NONE;
		}
		int64_t phys_size = get_sector_size_from_mode(track_type);
		if(phys_size > 0) {
			toc_table_tmp[nr_current_track].type = track_type;
			toc_table_tmp[nr_current_track].physical_size = phys_size;
			toc_table_tmp[nr_current_track].logical_size = get_logical_size_from_mode(track_type);
			if(track_type == MODE_AUDIO) {
				toc_table_tmp[nr_current_track].is_audio = true;
			}
			result = true;
		}
		if(track_num < (_nr_num + 1)) track_num = _nr_num + 1;
	} else {
		// ToDo: 20181118 K.Ohta
		nr_current_track = 0;
	}
	return result;
}

int TOWNS_CDROM::parse_cue_index(std::string &_arg2, int nr_current_track, int& value)
{
	int index = -1;
	value = 0;
	const std::regex _ts1("^[ \t]*");
	const std::regex _ts2("[ \t\n\r]+$");
	const std::regex _ts3("[ \t\n\r]+");									  
	
	std::string _arg3;
	std::string _arg4;
	std::smatch _sm3a, _sm3b, _sm4, _sm5;
	
	if((nr_current_track > 0) && (nr_current_track < 100)) {
		// Split header.
		if(std::regex_search(_arg2, _sm3a, _ts1)) {
			_arg3 = _sm3a.suffix();
		} else {
			return -1;
		}
		// Split footer.
		if(std::regex_search(_arg3, _sm3b, _ts2)) {
			_arg3 = _sm3b.suffix();
		}
		// Get 1st ARG: INDEX NUM .
		// Get 2nd ARG: MSF .
		if(std::regex_search(_arg3, _sm4, _ts3)) {
			_arg3 = _sm4.prefix();
			_arg4 = _sm4.suffix();
			if(std::regex_search(_arg3, _sm5, _ts1)) {
				_arg3 = _sm5.suffix();
			}
		} else {
			return -1;
		}
		index = atoi(_arg3.c_str());

		switch(index) {
		case 0:
			value = get_frames_from_msf(_arg4.c_str());
			break;
		case 1:
			value = get_frames_from_msf(_arg4.c_str());
			break;
		default:
			index = -1;
			break;
		}
	}
	return index;
}
#endif
}
