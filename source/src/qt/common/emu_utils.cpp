
//#include <SDL/SDL.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "common.h"
#include "emu.h"
#include "qt_main.h"


// Belows are Wrappers.
#if !defined(Q_OS_WIN) && !defined(Q_OS_CYGWIN)
extern "C" {
   

   uint32_t timeGetTime(void)
     {
	return SDL_GetTicks();
     }
}
#endif


#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || \
    defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)

void open_disk(int drv, _TCHAR* path, int bank)
{
   
        _TCHAR path_shadow[_MAX_PATH];
        if(path == NULL) return;
        strncpy(path_shadow, path, _MAX_PATH - 1);
	emu->d88_file[drv].bank_num = 0;
	emu->d88_file[drv].cur_bank = -1;
//	emu->d88_file[drv].bank[0].offset = 0;

	if(check_file_extension(path, ".d88") || check_file_extension(path, ".d77")) {
	
		FILEIO *fio = new FILEIO();
		if(fio->Fopen(path, FILEIO_READ_BINARY)) {
			try {
				fio->Fseek(0, FILEIO_SEEK_END);
				int file_size = fio->Ftell(), file_offset = 0;
				while(file_offset + 0x2b0 <= file_size && emu->d88_file[drv].bank_num < MAX_D88_BANKS) {
					fio->Fseek(file_offset, FILEIO_SEEK_SET);
					char tmp[18];
					memset(tmp, 0x00, sizeof(tmp));
					fio->Fread(tmp, 17, 1);
					memset(emu->d88_file[drv].disk_name[emu->d88_file[drv].bank_num], 0x00, 128);
					if(strlen(tmp) > 0) Convert_CP932_to_UTF8(emu->d88_file[drv].disk_name[emu->d88_file[drv].bank_num], tmp, 127, 17);

					fio->Fseek(file_offset + 0x1c, FILEIO_SEEK_SET);
				        file_offset += fio->FgetUint32_LE();
					emu->d88_file[drv].bank_num++;
				}
				strcpy(emu->d88_file[drv].path, path);
			        if(bank >= emu->d88_file[drv].bank_num) bank = emu->d88_file[drv].bank_num - 1;
			        if(bank < 0) bank = 0;
				emu->d88_file[drv].cur_bank = bank;
			}
			catch(...) {
				bank = 0;
				emu->d88_file[drv].bank_num = 0;
			}
		   	fio->Fclose();
		}
	   	delete fio;
	} else {
	   bank = 0;
	}
	emu->open_disk(drv, path, bank);
}

void close_disk(int drv)
{
	emu->close_disk(drv);
	emu->d88_file[drv].bank_num = 0;
	emu->d88_file[drv].cur_bank = -1;
}
#endif

#if defined(USE_QD1) || defined(USE_QD2)
#endif

