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
	
	if(fio->Fopen(full_path_cue, FILEIO_READ_ASCII)) { // ToDo: Support not ASCII cue file (i.e. SJIS/UTF8).20181118 K.O
		int _c;
		bool is_eof = false;
		int sptr = 0;
		bool have_filename = false;
//		int _nr_num = 0;
		while(1) {
			line_buf.clear();
			int _np = 0;
			_c = EOF;
			do {
				_c = fio->Fgetc();
				if((_c == '\0') || (_c == '\n') || (_c == EOF)) break;;
				if(_c != '\r') line_buf.push_back((char)_c);
			} while(1);
			if(_c == EOF) is_eof = true;
			slen = (int)line_buf.length();
			if(slen > 0) {
				const std::regex _ts1("^[ \t]*");
				const std::regex _ts2("[ \t\n\r]+$");
				const std::regex _ts3("[ \t\n\r]+");									  
				std::smatch _sm1;
				std::string line1;
				// Split header.
				if(std::regex_search(line_buf, _sm1, _ts1)) {
					line1 = _sm1.suffix();
				} else {
					line1 = line_buf;
				}
				// Split footer.
				std::string argstr;	
				std::smatch _sm2;
				if(std::regex_search(line1, _sm2, _ts2)) {
					argstr = _sm2.prefix();
				} else {
					argstr = line1;
				}
				std::string cmdstr;
				std::smatch _sm3;
				if(std::regex_search(argstr, _sm3, _ts3)) {
					cmdstr = _sm3.prefix();
					argstr = _sm3.suffix();
					std::smatch _sm4;
					if(std::regex_search(argstr, _sm4, _ts1)) {
						argstr = _sm4.suffix();
					}
				} else {
					cmdstr = argstr;
					argstr = std::string("");
				}
				std::transform(cmdstr.begin(), cmdstr.end(), cmdstr.begin(),
							   [](unsigned char c) -> unsigned char{ return std::toupper(c); });
				//out_debug_log(_T("%s : %s : %s"), line1.c_str(), cmdstr.c_str(), argstr.c_str());
				int typeval;
				try {
					typeval = cue_enum.at(cmdstr);
				} catch (std::out_of_range &e) {
					typeval = CUE_NONE;
				}
				switch(typeval) {
				case CUE_REM:
					//out_debug_log(_T("TRACK#%02d REM:%s\n"),  nr_current_track, line_buf.c_str());
					break;
				case CUE_FILE:
					if(!(argstr.empty())) {
						std::string _fname = argstr;
						if(parse_cue_file_args(_fname, parent_dir, image_tmp_data_path, image_tmp_data_type)) {
							
							
						}
						//out_debug_log(_T("TRACK#%02d FILE:%s\n"),  nr_current_track, _fname.c_str());
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
					if(!(argstr.empty())) {
						int _value = 0;
						int _index = parse_cue_index(argstr, nr_current_track, _value);
						switch(_index) {
						case 0:
							toc_table_tmp[nr_current_track].index0 = _value;
							break;
						case 1:
							toc_table_tmp[nr_current_track].index1 = _value;
							break;
						default:
							break;
						}
					}
					out_debug_log(_T("TRACK#%02d INDEX:%s\n"),  nr_current_track, argstr.c_str());
					break;
				case CUE_TRACK:					
					if(!(argstr.empty())) {
						if(parse_cue_track(argstr, &(toc_table_tmp[0]), nr_current_track)) {
							tmp_track_data_path[nr_current_track] = image_tmp_data_path;
							tmp_track_data_type[nr_current_track] = image_tmp_data_type;
						}
					}
					out_debug_log(_T("TRACK#%02d TRACK:%s\n"),  nr_current_track, argstr.c_str());
					break;
				case CUE_PREGAP:
					if((nr_current_track > 0) && (nr_current_track < 100) && !(argstr.empty())) {
						toc_table_tmp[nr_current_track].pregap = get_frames_from_msf(argstr.c_str());
					}
					out_debug_log(_T("TRACK#%02d PREGAP:%s\n"),  nr_current_track, argstr.c_str());
					break;
				default:
					//out_debug_log(_T("TRACK#%02d ???:%s\n"),  nr_current_track, line_buf.c_str());
					break;
				}
			}
			if(is_eof) break;
		}
		// Finish
		max_logical_block = 0;
		uint32_t pt_lba_ptr = 0;
		if(track_num > 0) {
			//toc_table[0].lba_offset = 0;
			toc_table[0].lba_size = 0;
			toc_table[0].index0 = toc_table[0].index1 = toc_table[0].pregap = 0;
			// P1: Calc
			int _n = 0;
			int vnptr = 0;
			#if 1
			is_valid_image = ((track_num > 1) && (track_num <= 100)) ? true : false;
			for(int i = 1; (i < track_num) && (i < 100) ; i++) {
				CDROM_TOC_TABLE_t *pt = &(toc_table_tmp[i]);
				if(pt == NULL) {
					is_valid_image = false;
					break; // NG.
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
			// Pass2
			if(is_valid_image) {
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
				// 1St. Trim index0, index1, pregap.
				
				for(int i = 1; (i < track_num) && (i < 100) ; i++) {
					uint64_t old_image_size = __image_size;
					uint64_t old_offset = __offset_bytes;
					if(!(check_toc_and_open_image_file(&(toc_table_tmp[i]), __image_size, __offset_bytes, is_image_changed, __recent_path, tmp_track_data_path[i]))) {
						tmp_track_data_path[i].clear();
						is_valid_image = false;
						break;
					}
					int64_t tmp_sector_size = get_sector_size_from_mode(toc_table_tmp[i].type);
					if(tmp_sector_size <= 0) {
						is_valid_image = false;
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
				if(is_valid_image) {
					// Trim remain INDEX 00 and INDEX 01 .
					if((latest_changed < track_num) && (latest_changed > 1)) {
						for(int j = latest_changed; j < track_num; j++) {
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
					if((__image_size > 0) && (toc_table_tmp[track_num - 1].physical_size > 0)) {
						if(__image_size > toc_table[track_num - 1].bytes_offset) {
							uint64_t __last_sectors = (__image_size - toc_table[track_num - 1].bytes_offset) / toc_table_tmp[track_num - 1].physical_size;
							toc_table_tmp[track_num - 1].lba_size = __last_sectors;
							total_sectors += __last_sectors;
						} else {
							//	is_valid_image = false;
						}
					}
					toc_table_tmp[track_num].index0       = total_sectors;
					toc_table_tmp[track_num].index1		  = total_sectors;
					toc_table_tmp[track_num].pregap       = get_frames_from_msf(_T("00:02:00"));
					toc_table_tmp[track_num].lba_size     = 0;
					toc_table_tmp[track_num].bytes_offset = total_bytes;
					toc_table_tmp[track_num].physical_size = 2352;
					toc_table_tmp[track_num].logical_size = 2048;
					tmp_track_data_path[track_num] = std::string("");
					tmp_track_data_type[track_num] = std::string("");

					if(is_valid_image) {
						// ToDo : Copy toc_table.
						for(int i = 0; i <= 100; i++) {
							copy_toc_table_to_main(i, &(toc_table_tmp[i]), tmp_track_data_path[i], tmp_track_data_type[i]);
						}
						
						if(fio_img->IsOpened()) { // OK?
							fio_img->Fclose();
						}
						max_logical_block = 0;
						is_valid_image = false;
						if(!(tmp_track_data_path[1].empty())) {
							if(fio_img->Fopen(tmp_track_data_path[1].c_str(), FILEIO_READ_BINARY)) {
								if(fio_img->Fseek((long)(toc_table[1].bytes_offset), FILEIO_SEEK_SET) == 0) {
									max_logical_block = total_sectors;
									is_valid_image = true;
								}
							}
						}
					}
				}
			}
			#else
			for(int i = 1; i < track_num; i++) {

				if(fio_img->IsOpened()) {
					fio_img->Fclose();
				}
				// Even...
				//if(toc_table[i].pregap <= 0) {
				//	toc_table[i].pregap = 150; // Default PREGAP must be 2Sec. From OoTake.(Only with PCE? Not with FM-Towns?)
				//}
				if((strlen(track_data_path[i - 1]) > 0) && (with_filename[i])) {
					if(toc_table[i].physical_size > 0) {
					if(fio_img->Fopen(track_data_path[i - 1], FILEIO_READ_BINARY)) {
						if((_n = fio_img->FileLength() / toc_table[i].physical_size) > 0) {
							max_logical_block += _n;
						} else {
							_n = 0;
						}
						fio_img->Fclose();
					}
					}
					toc_table[i].lba_size = _n;
				}
				//toc_table[i].lba_offset = max_logical_block - _n;
				if(!(with_filename[i + 1]) && (toc_table[i + 1].index1 > toc_table[i].index1)) {
					toc_table[i].lba_size = toc_table[i + 1].index1 - toc_table[i].index0;
				}
				if(toc_table[i].index0 == 0) {
					toc_table[i].index0 = toc_table[i].index1;
				}
				if(toc_table[i].pregap == 0) {
					toc_table[i].pregap = toc_table[i].index1 - toc_table[i].index0;
				}
				// Even...
				if(toc_table[i].pregap <= 150) {
					toc_table[i].pregap = 150; // Default PREGAP must be 2Sec. From OoTake.(Only with PCE? Not with FM-Towns?)
				}
			}
			#endif
		   	//if((track_num == 2) && (max_logical_block > 0)) {
			//	toc_table[track_num - 1].lba_size -= 1;
			//	max_logical_block--;
			//}
			for(int i = 1; i < track_num; i++) {
				//toc_table[i].index0 += toc_table[i].lba_offset;
				//toc_table[i].index1 += toc_table[i].lba_offset;
				#if 1
				out_debug_log(_T("TRACK#%02d TYPE=%s PREGAP=%d INDEX0=%d INDEX1=%d LBA_SIZE=%d BYTES_OFFSET=%d TYPE=\"%s\" PATH=\"%s\"\n"),
							  i, (toc_table_tmp[i].is_audio) ? _T("AUDIO") : _T("MODE1/2352"),
							  toc_table_tmp[i].pregap, toc_table_tmp[i].index0, toc_table_tmp[i].index1,
							  toc_table_tmp[i].lba_size, toc_table_tmp[i].bytes_offset,
							  tmp_track_data_type[i].c_str(), tmp_track_data_path[i].c_str());
				#endif
			}
			#if 0
			toc_table[0].index0 = toc_table[0].index1 = toc_table[0].pregap = 0;
			toc_table[0].physical_size = 2352;
			toc_table[0].logical_size = 2048;
			toc_table[track_num].index0 = toc_table[track_num].index1 = max_logical_block;
			toc_table[track_num].lba_offset = max_logical_block;
			toc_table[track_num].lba_size = 0;
			#endif
		}

		fio->Fclose();
	}
	delete fio;

	// Not Cue FILE.
	return is_valid_image;
}

// ToDo: Implement Image type.	
bool TOWNS_CDROM::parse_cue_file_args(std::string& _arg2, const _TCHAR *parent_dir, std::string& imgpath, std::string& imgtype)
{
	size_t _arg2_ptr;
	size_t _arg3_ptr;
	std::string _arg3;
	std::string _arg4;
	_arg2_ptr = _arg2.find_first_of((const char *)"\"") + 1;
	if(_arg2_ptr == std::string::npos) return false;

	_arg2 = _arg2.substr(_arg2_ptr);
	_arg3_ptr = _arg2.find_first_of((const char *)"\"");
	if(_arg3_ptr == std::string::npos) return false;

	try {
		_arg3 = _arg2.substr(0, _arg3_ptr);
	} catch (std::out_of_range &e) {
		return false;
	} 
	imgpath = std::string(parent_dir);
	imgpath.append(_arg3);

	try {
		_arg4 = _arg2.substr(_arg3_ptr + 1);
	} catch (std::out_of_range &e) {
		if(imgtype.empty()) {
			_arg4 = std::string("BINARY");
		} else {
			_arg4 = imgtype;
		}
	}
	const std::regex _ts1("^[ \t]*");
	const std::regex _ts2("[ \t\n\r]+$");
	std::smatch _sm1, _sm2;
	if(_arg4.empty()) {
		return true; // OK?
	}
	// Split header.
	if(std::regex_search(_arg4, _sm1, _ts1)) {
		_arg4 = _sm1.suffix();
	}
	// Split Footer
	if(std::regex_search(_arg4, _sm2, _ts2)) {
		_arg4 = _sm2.prefix();
	}
	if(!(_arg4.empty())) {
		imgtype = _arg4;
	}
//	cdrom_debug_log(_T("**FILE %s\n"), imgpath.c_str());

	return true;
}

// ToDo: Implement Image type.	
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

}
