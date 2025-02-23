/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.31 -

	[FM-Towns CD-ROM : ISO PARSER]
*/

#include "../cdrom.h"
#include "../../../fileio.h"

#include <string>
#include <map>

namespace FMTOWNS {

bool TOWNS_CDROM::open_iso_file(const _TCHAR* file_path)
{
	_TCHAR full_path_iso[_MAX_PATH] = {0};

	int nr_current_track = 0;
	FILEIO* fio = new FILEIO();
	if(fio == NULL) return false;

	CDROM_TOC_TABLE_t toc_table_tmp[101];
	for(int i = 0; i < 101; i++) {
		initialize_toc_table(&(toc_table_tmp[i]));
	}
	
	get_long_full_path_name(file_path, full_path_iso, sizeof(full_path_iso));

	bool result = false;
	if(fio->Fopen(full_path_iso, FILEIO_READ_BINARY)) { //
		uint64_t total_size = (uint64_t)fio->FileLength();
		uint64_t sectors = total_size / 2048; //! @note Support only MODE1/2352.
		CDROM_MODE_t track_type = MODE1_ISO; //! @note Support only MODE1/2352.
		if(sectors >= 1) {
			track_num = 2;
			max_logical_block = sectors;
			toc_table_tmp[1].type = track_type;
			toc_table_tmp[1].bytes_offset = 0;
			toc_table_tmp[1].lba_size = sectors;
			toc_table_tmp[1].is_audio = false;
			toc_table_tmp[1].index0 = 0;
			toc_table_tmp[1].index1 = get_frames_from_msf(_T("00:02:00"));
			toc_table_tmp[1].pregap = get_frames_from_msf(_T("00:02:00"));
			toc_table_tmp[1].physical_size = 2352;
			toc_table_tmp[1].logical_size = 2048;
												  
			toc_table_tmp[2].type = MODE_NONE;
			toc_table_tmp[2].bytes_offset = total_size;
			toc_table_tmp[2].lba_size = 0;
			toc_table_tmp[2].is_audio = false;
			toc_table_tmp[2].index0 = sectors + 1;
			toc_table_tmp[2].index1 = sectors + 1;
			toc_table_tmp[2].pregap = 150;
			toc_table_tmp[2].physical_size = 0;
			toc_table_tmp[2].logical_size = 0;
												  
			copy_toc_table_to_main(0, &(toc_table_tmp[0]), std::string(""), std::string(""));
			copy_toc_table_to_main(1, &(toc_table_tmp[1]), std::string(full_path_iso), std::string(_T("BINARY")));
			for(int i = 2; i <= 100; i++) {
				copy_toc_table_to_main(i, &(toc_table_tmp[i]), std::string(""), std::string(""));
			}
			result = true;
		} else {
			track_num = 0;
			max_logical_block = 0;
		}
		fio->Fclose();
	}
	delete fio;
	return result;
}

}
