/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.31 -

	[FM-Towns CD-ROM : CLONECD PARSER]
*/

#include "../cdrom.h"
#include "../../../fileio.h"

#include <string>
#include <regex>
#include <map>

namespace FMTOWNS {

	namespace CCD_PARSER {
		typedef enum {
			PHASE_NULL = 0,
			PHASE_ENTRY = 1,
			PHASE_SESSION,
			PHASE_CLONECD,
			PHASE_DISC,
			PHASE_TRACK
		} CCD_PHASE_t;
		typedef enum {
			TYPE_NULL = 0,
			POINT = 1,
			CONTROL,
			PLBA,
			ALBA,
			INDEX0,
			INDEX1,
			MODE,

			TOC_ENTRIES = 0x101,
			CDTEXT_LENGTH,

			PREGAP_MODE = 0x201,
			PREGAP_SUBC
		} CCD_MODE_t;
	}

using CCD_PARSER::PHASE_NULL;
using CCD_PARSER::PHASE_ENTRY;
using CCD_PARSER::PHASE_SESSION;
using CCD_PARSER::PHASE_CLONECD;
using CCD_PARSER::PHASE_DISC;
using CCD_PARSER::PHASE_TRACK;

using CCD_PARSER::TYPE_NULL;
using CCD_PARSER::POINT;
using CCD_PARSER::CONTROL;
using CCD_PARSER::PLBA;
using CCD_PARSER::ALBA;
using CCD_PARSER::INDEX0;
using CCD_PARSER::INDEX1;
using CCD_PARSER::MODE;
using CCD_PARSER::TOC_ENTRIES;
using CCD_PARSER::CDTEXT_LENGTH;
using CCD_PARSER::PREGAP_MODE;
using CCD_PARSER::PREGAP_SUBC;

bool TOWNS_CDROM::open_ccd_file(const _TCHAR* file_path)
{

	_TCHAR img_file_path[_MAX_PATH + 1] = {0};
	_TCHAR full_path_ccd[_MAX_PATH] = {0};
	_TCHAR full_path_img[_MAX_PATH + 1] = {0};
	
	get_long_full_path_name(file_path, full_path_ccd, sizeof(full_path_ccd));
	if(!FILEIO::IsFileExisting(full_path_ccd)) {
		return false;
	}
	
	my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.img"), get_file_path_without_extensiton(file_path));
	get_long_full_path_name(img_file_path, full_path_img, sizeof(full_path_img));
	
	if(!FILEIO::IsFileExisting(full_path_img)) {
		my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.gz"), get_file_path_without_extensiton(file_path));
		memset(full_path_img, 0x00, sizeof(full_path_img));
		get_long_full_path_name(img_file_path, full_path_img, sizeof(full_path_img));
		
		if(!FILEIO::IsFileExisting(full_path_img)) {
			my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.img.gz"), get_file_path_without_extensiton(file_path));
			memset(full_path_img, 0x00, sizeof(full_path_img));
			get_long_full_path_name(img_file_path, full_path_img, sizeof(full_path_img));
			if(!FILEIO::IsFileExisting(full_path_img)) {
				return false;
			}
		}
	}
	if(strlen(full_path_img) <= 0) {
		return false;
	}
	std::string line_buf;
	std::string line_buf_shadow;
	std::string tmp_img_path = std::string(full_path_img);

	CDROM_TOC_TABLE_t toc_table_tmp[101];
	std::string tmp_track_data_path[101];	
	std::string tmp_track_data_type[101];	
	for(int i = 0; i < 101; i++) {
		initialize_toc_table(&(toc_table_tmp[i]));
	}

//	out_debug_log(_T("open_ccd_file(): file_path = %s  / full_path_ccd = %s"), file_path, full_path_ccd);
	
	const _TCHAR *parent_dir = get_parent_dir((const _TCHAR *)full_path_ccd);
	std::map<std::string, CCD_PARSER::CCD_PHASE_t> ccd_phase;
	std::map<std::string, CCD_PARSER::CCD_MODE_t>  ccd_enum;

	// Initialize
	ccd_phase.insert(std::make_pair("[ENTRY ", PHASE_ENTRY));
	ccd_phase.insert(std::make_pair("[SESSION ", PHASE_SESSION));
	ccd_phase.insert(std::make_pair("[TRACK ", PHASE_TRACK));
	ccd_phase.insert(std::make_pair("[CLONECD] ", PHASE_CLONECD));
	ccd_phase.insert(std::make_pair("[DISC] ", PHASE_DISC));

	// [Entry foo]
	// ToDo:
	// Support AMin/ASec/AFrame and PMin/PSec/PFrame.
	ccd_enum.insert(std::make_pair("POINT=", POINT));
	ccd_enum.insert(std::make_pair("CONTROL=", CONTROL));
	ccd_enum.insert(std::make_pair("PLBA=", PLBA));
	ccd_enum.insert(std::make_pair("ALBA=", ALBA));
	// Note: If ("INDEX 0" or "INDEX 1") has set, pregap may calculate from index0 and index1.Prefer than ALBA.
	ccd_enum.insert(std::make_pair("INDEX 0=", INDEX0));
	ccd_enum.insert(std::make_pair("INDEX 1=", INDEX1));
	ccd_enum.insert(std::make_pair("MODE=", MODE));

	// [Disc]
	// ToDo: Support DataTracksScrambled. OR, Will not support?
	ccd_enum.insert(std::make_pair("TOCENTRIES=", TOC_ENTRIES));
	ccd_enum.insert(std::make_pair("CDTEXTLENGTH=", CDTEXT_LENGTH));

	// [Session foo]
	ccd_enum.insert(std::make_pair("PREGAPMODE=", PREGAP_MODE));
	ccd_enum.insert(std::make_pair("PREGAPSUBC=", PREGAP_SUBC));
	
	int entries = 0;
	CCD_PARSER::CCD_PHASE_t phase = PHASE_NULL;
	int64_t cdtext_length = -1;
	if(fio_img->Fopen(img_file_path, FILEIO_READ_BINARY)) {
		current_track = 0;
		// get image file size
		if((max_logical_block = fio_img->FileLength() / 2352) > 0) {
			// read ccd file
			FILEIO* fio = new FILEIO();
			if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
				line_buf.clear();
				// ToDo: Will implement copying data_path and data_type.
				// - 20250218 K.O
				int _c;
				bool is_eof = false;
				int sptr = 0;
				bool have_filename = false;
//		int _nr_num = 0;
				int64_t phase_arg = 0;
				int _track = -1;
				int entry_num = -1;
				int toc_entries = -1;
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
					int slen = (int)line_buf.length();
					if(slen <= 0) goto _n_continue;
					// Convert String to upper.
					std::transform(line_buf.begin(), line_buf.end(), line_buf.begin(),
					   [](unsigned char c) -> unsigned char{ return std::toupper(c); });
//					out_debug_log(_T("FILE_PATH=\"%s\"  line=\"%s\""), file_path, line_buf.c_str());
					//
					// Check Phase
					;
					if(line_buf[0] == '[') {
						// Change phase
						CCD_PARSER::CCD_PHASE_t tmp_phase = PHASE_NULL;
						std::string token = line_buf;
						size_t ptr1 = line_buf.find_first_of(" ");
						size_t ptr2 = line_buf.find_first_of("]");
						std::string arg2;
						if(ptr1 != std::string::npos) {
							size_t ptr3 = line_buf.find_last_of("]");
							if(ptr3 != std::string::npos) {
								token.resize(ptr1 + 1);
								try {
									arg2 = line_buf.substr(ptr1 + 1, (ptr3 - (ptr1 + 1)));
								} catch (std::out_of_range &e) {
									arg2 = "";
								}

							}
						} else if(ptr2 != std::string::npos) {
							token.resize(ptr2 + 1);
						}

						try {
							tmp_phase = ccd_phase.at(token);
						} catch (std::out_of_range &e) {
							tmp_phase = PHASE_NULL;
						}
						// ToDo: exception handling for CCD_NULL
						if(tmp_phase != PHASE_NULL) {
							phase = tmp_phase;
						}
//						out_debug_log(_T("open_ccd_file(): file_path = %s  / full_path_ccd = %s"), file_path, full_path_ccd);
						switch(phase) {
						case PHASE_ENTRY:
							phase_arg = string_to_numeric(arg2);
							if((phase_arg != entry_num) && (phase_arg > 0)) {
								// Recalc track
								entry_num = phase_arg;
								//calc_track_index(_track);
							}
							break;
						case PHASE_SESSION:
							phase_arg = string_to_numeric(arg2);
							break;
						case PHASE_TRACK:
							phase_arg = string_to_numeric(arg2);
							if((phase_arg > 0) && (phase_arg <= 100)) {
								if(_track != phase_arg) {
									_track = phase_arg;
									if(track_num <= _track) {
										track_num = _track + 1;
									}
								}
							}
							break;
						default:
							phase_arg = 0;
							break;
						}
					} else {
						// Per phase.
						CCD_PARSER::CCD_MODE_t local_type = TYPE_NULL;
						std::string token = line_buf;
						size_t ptr1 = token.find_first_of("=");
						std::string arg2 = "";
						if(ptr1 != std::string::npos) { // OK
							token.resize(ptr1 + 1);
							try {
								local_type = ccd_enum.at(token);
							} catch (std::out_of_range &e) {
								local_type = TYPE_NULL;
							}
							try {
								arg2 = line_buf.substr(ptr1 + 1, line_buf.size());
							} catch (std::out_of_range &e) {
								arg2 = "";
							}
						}
						int64_t arg_val = 0;
						if(local_type != TYPE_NULL) {
							arg_val = string_to_numeric(arg2);
						}
						switch(phase) {
						case PHASE_SESSION:

							// ToDo: Imprement around pregap.
							break;
						case PHASE_DISC:
							switch(local_type) {
							case TOC_ENTRIES:
								toc_entries = arg_val;
								break;
							case CDTEXT_LENGTH:
								cdtext_length = arg_val;
								break;
							default:
								break;
							}
							break;
						case PHASE_ENTRY:
							switch(local_type) {
							case POINT:
								// Point -> Track num
								switch(arg_val) {
								case 0xa0:
									// ToDo
									break;
								case 0xa1:
									// ToDo
									break;
								case 0xa2:
									// ToDo
									break;
								case 0xa3:
									// ToDo
									break;
								default:
									if((arg_val >= 0) && (arg_val <= 100)) {
										if(arg_val != _track) {
											//recalc_track_index(_track);
											_track = arg_val;
											if(track_num <= _track) {
												track_num = _track + 1;
											}
										}
									}
									break;
								}
								break;
							case CONTROL:
								// Track type
								if((_track >= 0) && (_track <= 100)) {
									switch(arg_val) {
									case 0: // Audio
										toc_table_tmp[_track].type = MODE_AUDIO;
										toc_table_tmp[_track].is_audio = true;
										toc_table_tmp[_track].logical_size = 2352;
										toc_table_tmp[_track].physical_size = 2352;
										tmp_track_data_path[_track] = tmp_img_path;
										tmp_track_data_type[_track] = std::string(_T("BINARY"));
										break;
									case 4: // DATA
										toc_table_tmp[_track].type = MODE1_2352;
										toc_table_tmp[_track].is_audio = false;
										toc_table_tmp[_track].logical_size = 2048;
										toc_table_tmp[_track].physical_size = 2352;
										tmp_track_data_path[_track] = tmp_img_path;
										tmp_track_data_type[_track] = std::string(_T("BINARY"));
										break;
										// ToDo: another types.
									}
								}
								break;
							case PLBA:
								// PLBA -> LBA POSITION
								if((_track >= 0) && (_track <= 100)) {
									toc_table_tmp[_track].index1 = arg_val;
								}
								break;
							case ALBA:
								// ALBA -> PREGAP
								if((_track >= 0) && (_track <= 100)) {
									toc_table_tmp[_track].pregap = -arg_val;
								}
								break;
							case INDEX0:
								// Index0
								if((_track >= 0) && (_track <= 100)) {
									toc_table_tmp[_track].index0 = arg_val;
									toc_table_tmp[_track].pregap = 0;
								}
								break;
							case INDEX1:
								// Index0
								if((_track > 0) && (_track <= 100)) {
									toc_table_tmp[_track].index1 = arg_val;
									toc_table_tmp[_track].pregap = 0;
								}
								break;
							default:
								break;
							}
							break;
						case PHASE_TRACK:
							switch(local_type) {
							case INDEX0:
								// Index0
								if((_track >= 0) && (_track <= 100)) {
									toc_table_tmp[_track].index0 = arg_val;
									toc_table_tmp[_track].pregap = 0;
								}
								break;
							case INDEX1:
								// Index0
								if((_track > 0) && (_track <= 100)) {
									toc_table_tmp[_track].index1 = arg_val;
									toc_table_tmp[_track].pregap = 0;
								}
								break;
							case MODE:
								// Index0
								if((_track >= 0) && (_track <= 100)) {
									switch(arg_val) {
									case 1: // MODE1/2352
										toc_table_tmp[_track].type = MODE1_2352;
										toc_table_tmp[_track].is_audio = false;
										toc_table_tmp[_track].logical_size = 2048;
										toc_table_tmp[_track].physical_size = 2352;
										tmp_track_data_path[_track] = tmp_img_path;
										tmp_track_data_type[_track] = std::string(_T("BINARY"));
										break;
									case 0: // Audio
										toc_table_tmp[_track].type = MODE_AUDIO;
										toc_table_tmp[_track].is_audio = true;
										toc_table_tmp[_track].logical_size = 2352;
										toc_table_tmp[_track].physical_size = 2352;
										tmp_track_data_path[_track] = tmp_img_path;
										tmp_track_data_type[_track] = std::string(_T("BINARY"));
										break;
									default:
										break;
									}
								}
								break;
							default:
								break;
							}
							break;
						default:
							break;
						}
					}
				_n_continue:
					if(is_eof) {
						toc_table_tmp[0].bytes_offset = 0;
						toc_table_tmp[0].lba_size = 0;
						toc_table_tmp[0].index0 = toc_table_tmp[0].index1 = toc_table_tmp[0].pregap = 0;
						// P1: Calc
						int _n = 0;
						int vnptr = 0;
//						max_logical_block = toc_table_tmp[track_num - 1].index1; // PLBA
						toc_table_tmp[track_num].index0 = toc_table_tmp[track_num].index1 = max_logical_block;
						toc_table_tmp[track_num].lba_size = 0;
						uint64_t bytes_offset = 0;
						for(int i = 1; i < track_num; i++) {
							if(toc_table_tmp[i].index0 == 0) {
								toc_table_tmp[i].index0 = toc_table_tmp[i].index1;
							}
							if(toc_table_tmp[i].pregap == 0) {
								toc_table_tmp[i].pregap = toc_table_tmp[i].index1 - toc_table_tmp[i].index0;
							}
							if(toc_table_tmp[i].pregap <= 150) {
								toc_table_tmp[i].pregap = 150; // Default PREGAP must be 2Sec. From OoTake.(Only with PCE? Not with FM-Towns?)
							}
							if(toc_table_tmp[i].index0 == toc_table_tmp[i].index1) {
								toc_table_tmp[i].index0 = toc_table_tmp[i].index1 - toc_table_tmp[i].pregap;
							}
							if(toc_table_tmp[i].index0 <= 0) {
								toc_table_tmp[i].index0 = 0;
							}
							toc_table_tmp[i].bytes_offset = bytes_offset;
							toc_table_tmp[i].lba_size = toc_table_tmp[i + 1].index0 - toc_table_tmp[i].index0;
							bytes_offset += (toc_table_tmp[i].lba_size * toc_table_tmp[i].physical_size);
						}
						toc_table_tmp[track_num].bytes_offset = bytes_offset;
//						if((track_num == 2) && (max_logical_block > 0)) {
//							toc_table_tmp[track_num - 1].lba_size -= 1;
//							max_logical_block--;
//						}

						// ToDo: Will fix brakes insize of *file_ptr . 20230217 K.O
#if 1
						for(int i = 1; i < track_num; i++) {
//							toc_table_tmp[i].index0 += toc_table_tmp[i].lba_offset;
//							toc_table_tmp[i].index1 += toc_table_tmp[i].lba_offset;

							out_debug_log(_T("TRACK#%d TYPE=%s PREGAP=%d INDEX0=%d INDEX1=%d LBA_SIZE=%d BYTES_OFFSET=%d"),

										  i, ((toc_table_tmp[i].is_audio) ? _T("AUDIO") : _T("MODE1/2352")),
										  toc_table_tmp[i].pregap, toc_table_tmp[i].index0, toc_table_tmp[i].index1,
										  toc_table_tmp[i].lba_size, toc_table_tmp[i].bytes_offset);
							//#endif
						}
#endif
//						out_debug_log(_T("open_ccd_file(): track_num = %d file_path = %s  / full_path_ccd = %s"), track_num, file_path, full_path_ccd);
						break;
					}
				}
				fio->Fclose();
				for(int i = 0; i <= 100; i++) {
					copy_toc_table_to_main(i, &(toc_table_tmp[i]), tmp_track_data_path[i], tmp_track_data_type[i]);
				}
				delete fio;
				return true;
			}
			delete fio;
			return false;
		} else {
			fio_img->Fclose();
			return false;
		}
	}
	return false;
}

}
