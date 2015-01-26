
//#include <SDL/SDL.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "common.h"
#include "emu.h"
#include "qt_main.h"


// Belows are Wrappers.
extern "C" {
   
   void Sleep(uint32_t tick) 
     {
	SDL_Delay(tick);
     }

   uint32_t timeGetTime(void)
     {
	return SDL_GetTicks();
     }
}



#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || \
    defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)

void open_disk(int drv, _TCHAR* path, int bank)
{
   
        _TCHAR path_shadow[_MAX_PATH];
        if(path == NULL) return;
        strncpy(path_shadow, path, _MAX_PATH - 1);
	emu->d88_file[drv].bank_num = 0;
	emu->d88_file[drv].cur_bank = -1;
	emu->d88_file[drv].bank[0].offset = 0;

	if(check_file_extension(path, ".d88") || check_file_extension(path, ".d77")) {
	
		FILE *fp = fopen(path, "rb");
		if(fp != NULL) {
			try {
				fseek(fp, 0, SEEK_END);
				int file_size = ftell(fp), file_offset = 0;
				while(file_offset + 0x2b0 <= file_size && emu->d88_file[drv].bank_num < MAX_D88_BANKS) {
					emu->d88_file[drv].bank[emu->d88_file[drv].bank_num].offset = file_offset;
					fseek(fp, file_offset, SEEK_SET);
//#ifdef _UNICODE
					char tmp[18];
					fread(tmp, 17, 1, fp);
					tmp[17] = 0;
					Convert_CP932_to_UTF8(emu->d88_file[drv].bank[emu->d88_file[drv].bank_num].name, tmp, 127);

//#else
//					fread(emu->d88_file[drv].bank[emu->d88_file[drv].bank_num].name, 17, 1, fp);
//					emu->d88_file[drv].bank[emu->d88_file[drv].bank_num].name[17] = 0;
//#endif
					fseek(fp, file_offset + 0x1c, SEEK_SET);
					file_offset += fgetc(fp);
					file_offset += fgetc(fp) << 8;
					file_offset += fgetc(fp) << 16;
					file_offset += fgetc(fp) << 24;
					emu->d88_file[drv].bank_num++;
				}
				strcpy(emu->d88_file[drv].path, path);
			        if(bank >= emu->d88_file[drv].bank_num) bank = emu->d88_file[drv].bank_num - 1;
			        if(bank < 0) bank = 0;
				emu->d88_file[drv].cur_bank = bank;
			}
			catch(...) {
				emu->d88_file[drv].bank_num = 0;
			}
		}
	}
	emu->open_disk(drv, path, emu->d88_file[drv].bank[bank].offset);
}

void close_disk(int drv)
{
  emu->LockVM();
  emu->close_disk(drv);
  emu->d88_file[drv].bank_num = 0;
  emu->d88_file[drv].cur_bank = -1;
  emu->UnlockVM();

}
#endif

#if defined(USE_QD1) || defined(USE_QD2)
#endif

