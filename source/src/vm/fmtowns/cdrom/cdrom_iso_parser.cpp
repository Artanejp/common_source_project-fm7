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

	get_long_full_path_name(file_path, full_path_iso, sizeof(full_path_iso));
	const _TCHAR *parent_dir = get_parent_dir((const _TCHAR *)full_path_iso);
	memset(track_data_path[0], 0x00, _MAX_PATH * sizeof(_TCHAR));

	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) { //
		uint64_t total_size = (uint64_t)fio->FileLength();
		uint64_t sectors = total_size / 2048; //! @note Support only MODE1/2352.
		CDROM_MODE_t track_type = MODE1_2048; //! @note Support only MODE1/2352.
		toc_table[0].type = MODE_NONE;
		toc_table[0].lba_offset = 0;
		toc_table[0].lba_size = 0;
		toc_table[0].index0 = toc_table[0].index1 = toc_table[0].pregap = 0;
		if(sectors > 1) {
			track_num = 2;
			max_logical_block = sectors - 1;
			toc_table[1].type = track_type;
			toc_table[1].lba_offset = 0;
			toc_table[1].lba_size = sectors - 1;
			toc_table[1].is_audio = false;
			toc_table[1].index0 = 0;
			toc_table[1].index1 = 0;
			toc_table[1].pregap = 150;
			toc_table[1].physical_size = 2352;
			toc_table[1].logical_size = 2048;


			toc_table[2].type = MODE_NONE;
			toc_table[2].lba_size = 0;
			toc_table[2].is_audio = false;
			toc_table[2].index0 = sectors;
			toc_table[2].index1 = sectors;
			toc_table[2].pregap = 150;
			toc_table[2].physical_size = 0;
			toc_table[2].logical_size = 0;
			with_filename[1] = true;
			strncpy(track_data_path[0], full_path_iso, _MAX_PATH - 1);
		} else {
			track_num = 0;
			max_logical_block = 0;
			with_filename[1] = false;
		}
		fio->Fclose();
	} else {
		delete fio;
		return false;
	}
	delete fio;
	return true;
}

}
