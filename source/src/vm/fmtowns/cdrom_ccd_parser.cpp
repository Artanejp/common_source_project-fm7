/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.31 -

	[FM-Towns CD-ROM : CLONECD PARSER]
*/

#include "./cdrom.h"
#include "../../fileio.h"

#include <string>
#include <map>

namespace FMTOWNS {

bool TOWNS_CDROM::open_ccd_file(const _TCHAR* file_path, _TCHAR* img_file_path)
{
	my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.img"), get_file_path_without_extensiton(file_path));
	if(!FILEIO::IsFileExisting(img_file_path)) {
		my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.gz"), get_file_path_without_extensiton(file_path));
		if(!FILEIO::IsFileExisting(img_file_path)) {
			my_stprintf_s(img_file_path, _MAX_PATH, _T("%s.img.gz"), get_file_path_without_extensiton(file_path));
		}
		if(!FILEIO::IsFileExisting(img_file_path)) {
			memset(img_file_path, 0x00, _MAX_PATH);
			return false;
		}
	}

	std::string line_buf;
	std::string line_buf_shadow;
	std::string image_tmp_data_path;

	_TCHAR full_path_ccd[_MAX_PATH];
	memset(full_path_ccd, 0x00, sizeof(full_path_ccd));
	image_tmp_data_path.clear();

	get_long_full_path_name(file_path, full_path_ccd, sizeof(full_path_ccd));
	const _TCHAR *parent_dir = get_parent_dir((const _TCHAR *)full_path_ccd);
	std::map<std::string, int> ccd_phase;
	std::map<std::string, int> ccd_enum;

	// Initialize
	ccd_phase.insert(std::make_pair("[ENTRY ", CCD_PHASE_ENTRY));
	ccd_phase.insert(std::make_pair("[SESSION ", CCD_PHASE_SESSION));
	ccd_phase.insert(std::make_pair("[TRACK ", CCD_PHASE_TRACK));
	ccd_phase.insert(std::make_pair("[CLONECDCD] ", CCD_PHASE_CLONECD));
	ccd_phase.insert(std::make_pair("[DISC] ", CCD_PHASE_DISC));
	
	// [Entry foo]
	// ToDo:
	// Support AMin/ASec/AFrame and PMin/PSec/PFrame.
	ccd_enum.insert(std::make_pair("POINT=", CCD_POINT));
	ccd_enum.insert(std::make_pair("CONTROL=", CCD_CONTROL));
	ccd_enum.insert(std::make_pair("PLBA=", CCD_PLBA));
	ccd_enum.insert(std::make_pair("ALBA=", CCD_ALBA));
	// Note: If ("INDEX 0" or "INDEX 1") has set, pregap may calculate from index0 and index1.Prefer than ALBA.
	ccd_enum.insert(std::make_pair("INDEX 0=", CCD_INDEX_0));
	ccd_enum.insert(std::make_pair("INDEX 1=", CCD_INDEX_1));
	ccd_enum.insert(std::make_pair("MODE=", CCD_MODE));

	// [Disc]
	// ToDo: Support DataTracksScrambled. OR, Will not support?
	ccd_enum.insert(std::make_pair("TOCENTRIES=", CCD_TOC_ENTRIES));
	ccd_enum.insert(std::make_pair("CDTEXTLENGTH=", CCD_CDTEXT_LENGTH));

	// [Session foo]
	ccd_enum.insert(std::make_pair("PREGAPMODE=", CCD_PREGAP_MODE));
	ccd_enum.insert(std::make_pair("PREGAPSUBC=", CCD_PREGAP_SUBC));

#if 1
	int entries = 0;
	int phase = CCD_PHASE_NULL;
	int64_t cdtext_length = -1;
	if(fio_img->Fopen(img_file_path, FILEIO_READ_BINARY)) {
		is_cue = false;
		is_iso = false;
		current_track = 0;
		// get image file size
		if((max_logical_block = fio_img->FileLength() / 2352) > 0) {
			// read ccd file
			FILEIO* fio = new FILEIO();
			if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
				line_buf.clear();
				for(int i = 0; i < 100; i++) {
					memset(&(track_data_path[i][0]), 0x00, _MAX_PATH * sizeof(_TCHAR));
					with_filename[i] = false;
				}
				for(int i = 0; i <= 100; i++) {
					toc_table[i].type = MODE_MODE1_2352;
					toc_table[i].index0 = 0;
					toc_table[i].index1 = 0;
					toc_table[i].pregap = 0;
					toc_table[i].physical_size = 2352;
					toc_table[i].logical_size = 2048;
				}
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
					//
					// Check Phase
					;
					if(line_buf[0] == '[') {
						// Change phase
						int tmp_phase = CCD_PHASE_NULL;
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
							tmp_phase = CCD_PHASE_NULL;
						}
						// ToDo: exception handling for CCD_NULL
						if(tmp_phase != CCD_PHASE_NULL) {
							phase = tmp_phase;
						}
						switch(phase) {
						case CCD_PHASE_ENTRY:
							phase_arg = string_to_numeric(arg2);
							if((phase_arg != entry_num) && (phase_arg > 0)) {
								// Recalc track
								entry_num = phase_arg;
								//calc_track_index(_track);
							}
							break;
						case CCD_PHASE_SESSION:
							phase_arg = string_to_numeric(arg2);
							break;
						case CCD_PHASE_TRACK:
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
						int local_type = CCD_TYPE_NULL;
						std::string token = line_buf;
						size_t ptr1 = token.find_first_of("=");
						std::string arg2 = "";
						if(ptr1 != std::string::npos) { // OK
							token.resize(ptr1 + 1);
							try {
								local_type = ccd_enum.at(token);
							} catch (std::out_of_range &e) {
								local_type = CCD_TYPE_NULL;
							}
							try {
								arg2 = line_buf.substr(ptr1 + 1, line_buf.size());
							} catch (std::out_of_range &e) {
								arg2 = "";
							}
						}
						int64_t arg_val = 0;
						if(local_type != CCD_TYPE_NULL) {
							arg_val = string_to_numeric(arg2);
						}
						switch(phase) {
						case CCD_PHASE_SESSION:
							
							// ToDo: Imprement around pregap.
							break;
						case CCD_PHASE_DISC:
							switch(local_type) {
							case CCD_TOC_ENTRIES:
								toc_entries = arg_val;
								break;
							case CCD_CDTEXT_LENGTH:
								cdtext_length = arg_val;
								break;
							}
							break;
						case CCD_PHASE_ENTRY:
							switch(local_type) {
							case CCD_POINT:
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
							case CCD_CONTROL:
								// Track type
								if((_track >= 0) && (_track <= 100)) {
									switch(arg_val) {
									case 0: // Audio
										toc_table[_track].type = MODE_AUDIO;
										toc_table[_track].is_audio = true;
										toc_table[_track].logical_size = 2352;
										break;
									case 4: // DATA
										toc_table[_track].type = MODE_MODE1_2352;
										toc_table[_track].is_audio = false;
										toc_table[_track].logical_size = 2048;
										break;
										// ToDo: another types.
									}
								}
								break;
							case CCD_PLBA:
								// PLBA -> LBA POSITION
								if((_track >= 0) && (_track <= 100)) {
									toc_table[_track].index1 = arg_val;
								}
								break;
							case CCD_ALBA:
								// ALBA -> PREGAP
								if((_track >= 0) && (_track <= 100)) {
									toc_table[_track].pregap = -arg_val;
								}
								break;
							case CCD_INDEX_0:
								// Index0
								if((_track >= 0) && (_track <= 100)) {
									toc_table[_track].index0 = arg_val;
									toc_table[_track].pregap = 0;
								}
								break;
							case CCD_INDEX_1:
								// Index0
								if((_track > 0) && (_track <= 100)) {
									toc_table[_track].index1 = arg_val;
									toc_table[_track].pregap = 0;
								}
								break;
							}
							break;
						case CCD_PHASE_TRACK:
							switch(local_type) {
							case CCD_INDEX_0:
								// Index0
								if((_track >= 0) && (_track <= 100)) {
									toc_table[_track].index0 = arg_val;
									toc_table[_track].pregap = 0;
								}
								break;
							case CCD_INDEX_1:
								// Index0
								if((_track > 0) && (_track <= 100)) {
									toc_table[_track].index1 = arg_val;
									toc_table[_track].pregap = 0;
								}
								break;
							case CCD_MODE:
								// Index0
								if((_track >= 0) && (_track <= 100)) {
									switch(arg_val) {
									case 1: // MODE1/2352
										toc_table[_track].type = MODE_MODE1_2352;
										toc_table[_track].is_audio = false;
										toc_table[_track].logical_size = 2048;
										break;
									case 0: // Audio
										toc_table[_track].type = MODE_AUDIO;
										toc_table[_track].is_audio = true;
										toc_table[_track].logical_size = 2352;
										break;
									}
								}
								break;
							}
							break;
						}
					}
				_n_continue:
					if(is_eof) {
						toc_table[0].lba_offset = 0;
						toc_table[0].lba_size = 0;
						toc_table[0].index0 = toc_table[0].index1 = toc_table[0].pregap = 0;
						// P1: Calc
						int _n = 0;
						int vnptr = 0;
//						max_logical_block = toc_table[track_num - 1].index1; // PLBA
						toc_table[track_num].index0 = toc_table[track_num].index1 = max_logical_block;
						toc_table[track_num].lba_offset = max_logical_block;
						toc_table[track_num].lba_size = 0;
						for(int i = 1; i < track_num; i++) {
							if(toc_table[i].index0 == 0) {
								toc_table[i].index0 = toc_table[i].index1;
							}
							if(toc_table[i].pregap == 0) {
								toc_table[i].pregap = toc_table[i].index1 - toc_table[i].index0;
							}
							if(toc_table[i].pregap <= 150) {
								toc_table[i].pregap = 150; // Default PREGAP must be 2Sec. From OoTake.(Only with PCE? Not with FM-Towns?)
							}
							if(toc_table[i].index0 == toc_table[i].index1) {
								toc_table[i].index0 = toc_table[i].index1 - toc_table[i].pregap;
							}
							if(toc_table[i].index0 <= 0) {
								toc_table[i].index0 = 0;
							}
							toc_table[i].lba_offset = 0;
							toc_table[i].lba_size = toc_table[i + 1].index1 - toc_table[i].index0;
						}
//						if((track_num == 2) && (max_logical_block > 0)) {
//							toc_table[track_num - 1].lba_size -= 1;
//							max_logical_block--;
//						}
						for(int i = 1; i < track_num; i++) {
//							toc_table[i].index0 += toc_table[i].lba_offset;
//							toc_table[i].index1 += toc_table[i].lba_offset;
							out_debug_log(_T("TRACK#%02d TYPE=%s PREGAP=%d INDEX0=%d INDEX1=%d LBA_SIZE=%d LBA_OFFSET=%d PATH=%s\n"),
											i, (toc_table[i].is_audio) ? _T("AUDIO") : _T("MODE1/2352"),
											toc_table[i].pregap, toc_table[i].index0, toc_table[i].index1,
											toc_table[i].lba_size, toc_table[i].lba_offset, track_data_path[i - 1]);
							//#endif
						}

						break;
					}
				}
				fio->Fclose();
			}
			delete fio;
			return true;
		} else {
			fio_img->Fclose();
			return false;
		}
	}
#else
	if(fio_img->Fopen(img_file_path, FILEIO_READ_BINARY)) {
		is_cue = false;
		is_iso = false;
		current_track = 0;
		// get image file size
		if((max_logical_block = fio_img->FileLength() / 2352) > 0) {
			// read ccd file
			FILEIO* fio = new FILEIO();
			if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
				char line[1024] = {0};
				char *ptr;
				int track = -1;
				while(fio->Fgets(line, 1024) != NULL) {
					if(strstr(line, "[Session ") != NULL) {
						track = -1;
					} else if((ptr = strstr(line, "Point=0x")) != NULL) {
						if((track = hexatoi(ptr + 8)) > 0 && track < 0xa0) {
							if((track + 1) > track_num) {
								track_num = track + 1;
							}
						}
					} else if((ptr = strstr(line, "Control=0x")) != NULL) {
						if(track > 0 && track < 0xa0) {
							toc_table[track].is_audio = (hexatoi(ptr + 10) != 4);
						}
					} else if((ptr = strstr(line, "ALBA=-")) != NULL) {
						if(track > 0 && track < 0xa0) {
							toc_table[track].pregap = atoi(ptr + 6);
						}
					} else if((ptr = strstr(line, "PLBA=")) != NULL) {
						if(track > 0 && track < 0xa0) {
							toc_table[track + 1].index0 = atoi(ptr + 5);
						}
					} else if((ptr = strstr(line, "[TRACK ")) != NULL) {
						char* ptr2 = ptr;
						if((ptr2 = strstr(ptr + 7, "]")) != NULL) {
							uintptr_t n = (uintptr_t)ptr2;
							uintptr_t m = (uintptr_t)ptr;
							n = n - m;
							if(n > 2) n = 2;
							char numbuf[3] = {0};
							strncpy(numbuf, ptr + 7, (size_t)n);
							track = atoi(numbuf);
							if((track + 1) > track_num) {
								track_num = track + 1;
							}
						}
					} else if((ptr = strstr(line, "INDEX 0=")) != NULL) {
						if(track > 0 && track < 0xa0) {
							toc_table[track].index0 = atoi(ptr + 8);
						}
					} else if((ptr = strstr(line, "INDEX 1=")) != NULL) {
						if(track > 0 && track < 0xa0) {
							toc_table[track].index1 = atoi(ptr + 8);
						}
					} else if((ptr = strstr(line, "MODE=")) != NULL) {
						if(track > 0 && track < 0xa0) {
							int mode;
							mode = atoi(ptr + 5);
							switch(mode) {
							case 0:
								toc_table[track].type = MODE_AUDIO;
								toc_table[track].is_audio = true;
								toc_table[track].logical_size = 2352;
								break;
							case 1:
								toc_table[track].type = MODE_MODE1_2352;
								toc_table[track].is_audio = false;
								toc_table[track].logical_size = 2048;
								break;
							case 2:
								toc_table[track].type = MODE_MODE2_2336;
								toc_table[track].is_audio = false;
								toc_table[track].logical_size = 2336;
								break;
							default: // ???
								toc_table[track].type = MODE_AUDIO;
								toc_table[track].is_audio = true;
								toc_table[track].logical_size = 2352;
								break;
							}
						}
					}
				}
				toc_table[0].lba_offset = 0;
				toc_table[0].lba_size = 0;
				toc_table[0].index0 = toc_table[0].index1 = toc_table[0].pregap = 0;
				toc_table[0].physical_size = 0;
				toc_table[0].logical_size = 0;
				if(track_num > 0) {
					for(int i = 1; i < track_num; i++) {
						// ToDo: Some types.
						toc_table[i].physical_size = 2352;

						if(toc_table[i].index0 == 0) {
							toc_table[i].index0 = toc_table[i].index1;
						}
						if(toc_table[i].pregap == 0) {
							toc_table[i].pregap = toc_table[i].index1 - toc_table[i].index0;
						}
						if(toc_table[i].pregap <= 150) toc_table[i].pregap = 150;
					}
					toc_table[track_num].index0 = toc_table[track_num].index1 = max_logical_block;
					for(int i = 1; i < track_num; i++) {
						toc_table[i].lba_offset = toc_table[i].index1;
						toc_table[i].lba_size = toc_table[i + 1].index1 - toc_table[i].index1;
						if(toc_table[i].lba_size > 0) toc_table[i].lba_size -= 1;
					}
					toc_table[track_num].lba_size = 0;
					toc_table[track_num].physical_size = 0;
					toc_table[track_num].logical_size = 0;
				} else {
					fio_img->Fclose();
					fio->Fclose();
					delete fio;
					return false;
				}
				fio->Fclose();
			}
			delete fio;
			return true;
		}
	}
#endif
	return false;
}

}
