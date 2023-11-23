/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.31 -

	[FM-Towns CD-ROM : CUE PARSER]
*/

#include "../cdrom.h"
#include "../../../fileio.h"

#include <string>
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


bool TOWNS_CDROM::open_cue_file(const _TCHAR* file_path)
{
	std::string line_buf;
	std::string line_buf_shadow;
	std::string image_tmp_data_path;

	_TCHAR full_path_cue[_MAX_PATH];
	size_t ptr;
	int line_count = 0;
	int slen;
	int nr_current_track = 0;
	FILEIO* fio = new FILEIO();
	if(fio == NULL) return false;

	memset(full_path_cue, 0x00, sizeof(full_path_cue));
	image_tmp_data_path.clear();

	get_long_full_path_name(file_path, full_path_cue, sizeof(full_path_cue));

	const _TCHAR *parent_dir = get_parent_dir((const _TCHAR *)full_path_cue);

	size_t _arg1_ptr;
	size_t _arg2_ptr;
	size_t _arg2_ptr_s;
	size_t _arg3_ptr;
	size_t _arg3_ptr_s;

	std::string _arg1;
	std::string _arg2;
	std::string _arg3;

	std::map<std::string, int> cue_enum;

	// Initialize
	cue_enum.insert(std::make_pair("REM", CUE_REM));
	cue_enum.insert(std::make_pair("FILE", CUE_FILE));
	cue_enum.insert(std::make_pair("TRACK", CUE_TRACK));
	cue_enum.insert(std::make_pair("INDEX", CUE_INDEX));
	cue_enum.insert(std::make_pair("PREGAP", CUE_PREGAP));


	if(fio->Fopen(file_path, FILEIO_READ_ASCII)) { // ToDo: Support not ASCII cue file (i.e. SJIS/UTF8).20181118 K.O
		line_buf.clear();
		for(int i = 0; i < 100; i++) {
			memset(&(track_data_path[i][0]), 0x00, _MAX_PATH * sizeof(_TCHAR));
			with_filename[i] = false;
		}
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
			if(slen <= 0) goto _n_continue;
			// Trim head of Space or TAB
			ptr = 0;
			sptr = 0;
			// Tokenize
			_arg1.clear();
			_arg2.clear();
			_arg3.clear();

			ptr = line_buf.find_first_not_of((const char*)" \t");
			if(ptr == std::string::npos) {
				goto _n_continue;
			}
			// Token1
			line_buf_shadow = line_buf.substr(ptr);

			_arg1_ptr = line_buf_shadow.find_first_of((const char *)" \t");
			_arg1 = line_buf_shadow.substr(0, _arg1_ptr);
			_arg2 = line_buf_shadow.substr(_arg1_ptr);
			std::transform(_arg1.begin(), _arg1.end(), _arg1.begin(),
						   [](unsigned char c) -> unsigned char{ return std::toupper(c); });

			_arg2_ptr = _arg2.find_first_not_of((const char *)" \t");

			if(_arg2_ptr != std::string::npos) {
				_arg2 = _arg2.substr(_arg2_ptr);
			}
			int typeval;
			try {
				typeval = cue_enum.at(_arg1);
			} catch (std::out_of_range &e) {
				typeval = CUE_NONE;
			}
			switch(typeval) {
			case CUE_REM:
				break;
			case CUE_FILE:
				{
					if(!(parse_cue_file_args(_arg2, parent_dir, image_tmp_data_path))) break;
					with_filename[nr_current_track + 1] = true;
				}
				break;
			case CUE_TRACK:
				{
					parse_cue_track(_arg2, nr_current_track, image_tmp_data_path);
				}
				break;
			case CUE_INDEX:
				parse_cue_index(_arg2, nr_current_track);
				break;
			case CUE_PREGAP:
				if((nr_current_track > 0) && (nr_current_track < 100)) {
					_arg2_ptr_s = _arg2.find_first_of((const char *)" \t");
					_arg2 = _arg2.substr(0, _arg2_ptr_s - 1);

					toc_table[nr_current_track].pregap = get_frames_from_msf(_arg2.c_str());
				}
				break;
			}
		_n_continue:
			if(is_eof) break;
			line_buf.clear();
			continue;
		}
		// Finish
		max_logical_block = 0;
		uint32_t pt_lba_ptr = 0;
		if(track_num > 0) {
			toc_table[0].lba_offset = 0;
			toc_table[0].lba_size = 0;
			toc_table[0].index0 = toc_table[0].index1 = toc_table[0].pregap = 0;
			// P1: Calc
			int _n = 0;
			int vnptr = 0;
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
				toc_table[i].lba_offset = max_logical_block - _n;
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
		   	if((track_num == 2) && (max_logical_block > 0)) {
				toc_table[track_num - 1].lba_size -= 1;
				max_logical_block--;
			}
			for(int i = 1; i < track_num; i++) {
				toc_table[i].index0 += toc_table[i].lba_offset;
				toc_table[i].index1 += toc_table[i].lba_offset;
#if 1
				out_debug_log(_T("TRACK#%02d TYPE=%s PREGAP=%d INDEX0=%d INDEX1=%d LBA_SIZE=%d LBA_OFFSET=%d PATH=%s\n"),
									i, (toc_table[i].is_audio) ? _T("AUDIO") : _T("MODE1/2352"),
									toc_table[i].pregap, toc_table[i].index0, toc_table[i].index1,
									toc_table[i].lba_size, toc_table[i].lba_offset, track_data_path[i - 1]);
#endif
			}
			toc_table[0].index0 = toc_table[0].index1 = toc_table[0].pregap = 0;
			toc_table[0].physical_size = 2352;
			toc_table[0].logical_size = 2048;
			toc_table[track_num].index0 = toc_table[track_num].index1 = max_logical_block;
			toc_table[track_num].lba_offset = max_logical_block;
			toc_table[track_num].lba_size = 0;
		}
		fio->Fclose();
	}
	delete fio;

	is_cue = false;
	if(track_num > 0) is_cue = true;
	// Not Cue FILE.
	return is_cue;
}

bool TOWNS_CDROM::parse_cue_file_args(std::string& _arg2, const _TCHAR *parent_dir, std::string& imgpath)
{
	size_t _arg2_ptr;
	size_t _arg3_ptr;
	std::string _arg3;

	_arg2_ptr = _arg2.find_first_of((const char *)"\"") + 1;
	if(_arg2_ptr == std::string::npos) return false;

	_arg2 = _arg2.substr(_arg2_ptr);
	_arg3_ptr = _arg2.find_first_of((const char *)"\"");
	if(_arg3_ptr == std::string::npos) return false;
	_arg2 = _arg2.substr(0, _arg3_ptr);

	imgpath.clear();
	imgpath = std::string(parent_dir);
	imgpath.append(_arg2);

//	cdrom_debug_log(_T("**FILE %s\n"), imgpath.c_str());

	return true;
}

void TOWNS_CDROM::parse_cue_track(std::string &_arg2, int& nr_current_track, std::string imgpath)
{
	size_t _arg2_ptr_s;
	size_t _arg2_ptr;
	_arg2_ptr_s = _arg2.find_first_of((const char *)" \t");

	std::string _arg3 = _arg2.substr(_arg2_ptr_s);
	_arg2 = _arg2.substr(0, _arg2_ptr_s);
	size_t _arg3_ptr = _arg3.find_first_not_of((const char *)" \t");
	size_t _arg3_ptr_s;
	int _nr_num = atoi(_arg2.c_str());

	// Set image file
	if((_nr_num > 0) && (_nr_num < 100) && (_arg3_ptr != std::string::npos)) {
		std::map<std::string, int> cue_type;
		cue_type.insert(std::make_pair("AUDIO", MODE_AUDIO));
		cue_type.insert(std::make_pair("MODE1/2048", MODE_MODE1_2048));
		cue_type.insert(std::make_pair("MODE1/2352", MODE_MODE1_2352));
		cue_type.insert(std::make_pair("MODE2/2336", MODE_MODE2_2336));
		cue_type.insert(std::make_pair("MODE2/2352", MODE_MODE2_2352));
		cue_type.insert(std::make_pair("CDI/2336", MODE_CDI_2336));
		cue_type.insert(std::make_pair("CDI/2352", MODE_CDI_2352));
		cue_type.insert(std::make_pair("CDG", MODE_CD_G));

		nr_current_track = _nr_num;
		_arg3 = _arg3.substr(_arg3_ptr);

		memset(track_data_path[_nr_num - 1], 0x00, sizeof(_TCHAR) * _MAX_PATH);
		strncpy((char *)(track_data_path[_nr_num - 1]), imgpath.c_str(), _MAX_PATH - 1);
//		image_tmp_data_path.clear();
//		with_filename[_nr_num - 1] = have_filename;
//		have_filename = false;
		_arg3_ptr_s = _arg3.find_first_of((const char *)" \t\n");
		_arg3.substr(0, _arg3_ptr_s);

		std::transform(_arg3.begin(), _arg3.end(), _arg3.begin(),
					   [](unsigned char c) -> unsigned char{ return std::toupper(c); });

		toc_table[nr_current_track].is_audio = false;
		toc_table[nr_current_track].index0 = 0;
		toc_table[nr_current_track].index1 = 0;
		toc_table[nr_current_track].pregap = 0;
		toc_table[nr_current_track].physical_size = 2352;
		toc_table[nr_current_track].logical_size = 2048;
		int track_type;
		try {
			track_type = cue_type.at(_arg3);
		} catch (std::out_of_range &e) {
			track_type = MODE_NONE;
		}
		toc_table[nr_current_track].type = track_type;

		switch(track_type) {
		case MODE_AUDIO:
			toc_table[nr_current_track].is_audio = true;
			toc_table[nr_current_track].logical_size = 2352;
			break;
		case MODE_MODE1_2048:
			toc_table[nr_current_track].logical_size = 2048;
			toc_table[nr_current_track].physical_size = 2048;
			break;
		case MODE_MODE1_2352:
			toc_table[nr_current_track].logical_size = 2048;
			break;
		case MODE_MODE2_2336:
			toc_table[nr_current_track].logical_size = 2336;
			toc_table[nr_current_track].physical_size = 2336;
			break;
		case MODE_MODE2_2352:
			toc_table[nr_current_track].logical_size = 2336;
			break;
		case MODE_CDI_2336:
			toc_table[nr_current_track].logical_size = 2336;
			toc_table[nr_current_track].physical_size = 2336;
			break;
		case MODE_CDI_2352:
			toc_table[nr_current_track].logical_size = 2336;
			break;
		case MODE_CD_G:
			toc_table[nr_current_track].logical_size = 2448;
			toc_table[nr_current_track].physical_size = 2448;
			break;
			// ToDo: Set data size.
		}
		if(track_num < (_nr_num + 1)) track_num = _nr_num + 1;
	} else {
		// ToDo: 20181118 K.Ohta
		nr_current_track = 0;
	}

}

int TOWNS_CDROM::parse_cue_index(std::string &_arg2, int nr_current_track)
{
	int index = -1;
	std::string _arg3;
	size_t _arg2_ptr_s;
	size_t _arg3_ptr_s;
	size_t _arg3_ptr;
	if((nr_current_track > 0) && (nr_current_track < 100)) {
		_arg2_ptr_s = _arg2.find_first_of((const char *)" \t");
		if(_arg2_ptr_s == std::string::npos) return -1;;

		_arg3 = _arg2.substr(_arg2_ptr_s);
		_arg2 = _arg2.substr(0, _arg2_ptr_s);
		_arg3_ptr = _arg3.find_first_not_of((const char *)" \t");
		if(_arg3_ptr == std::string::npos) return -1;

		_arg3 = _arg3.substr(_arg3_ptr);
		_arg3_ptr_s = _arg3.find_first_of((const char *)" \t");
		_arg3.substr(0, _arg3_ptr_s);
		index = atoi(_arg2.c_str());

		switch(index) {
		case 0:
			toc_table[nr_current_track].index0 = get_frames_from_msf(_arg3.c_str());
			break;
		case 1:
			toc_table[nr_current_track].index1 = get_frames_from_msf(_arg3.c_str());
			break;
		default:
			index = -1;
			break;
		}
	}
	return index;
}

}
