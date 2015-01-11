

#include <stdio.h>
#include <string>
#include <vector>
#include "common.h"
#include "emu.h"
#include "qt_main.h"

#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || 
    defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)

void open_disk(int drv, _TCHAR* path, int bank)
{
	emu->d88_file[drv].bank_num = 0;
	emu->d88_file[drv].cur_bank = -1;
	emu->d88_file[drv].bank[0].offset = 0;

	if(check_file_extension(path, ".d88") || check_file_extension(path, ".d77") ||
	   check_file_extension(path, ".D88") || check_file_extension(path, ".D77")) {
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
					Convert_CP932_to_UTF8(emu->d88_file[drv].bank[emu->d88_file[drv].bank_num].name, tmp, 18);

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
				emu->d88_file[drv].cur_bank = bank;
			}
			catch(...) {
				emu->d88_file[drv].bank_num = 0;
			}
		}
	}
	emu->open_disk(drv, path, emu->d88_file[drv].bank[bank].offset);
#ifdef USE_FD2
	if((drv & 1) == 0 && drv + 1 < MAX_FD && bank + 1 < emu->d88_file[drv].bank_num) {
		open_disk(drv + 1, path, bank + 1);
	}
#endif
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

