#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "..\..\src\vm\disk.h"

void main(int argc, char *argv[])
{
	int arg_offset = 0;
	bool open_as_1dd = false;
	bool open_as_256 = false;
	
	for(int i = 1; i < argc; i++) {
		if(_stricmp(argv[i], "-1dd") == 0) {
			open_as_1dd = true;
			arg_offset++;
		} else if(_stricmp(argv[i], "-256") == 0) {
			open_as_256 = true;
			arg_offset++;
		} else {
			break;
		}
	}
	if(argc < 2 + arg_offset) {
		printf("any2d88 : convert teledisk/imagedisk/cpdread/solid image file to d88 format\n");
		printf("\n");
		printf("usage : any2d88 [-1dd] [-256] file [output.d88]\n");
	} else if(check_file_extension(argv[arg_offset + 1], _T(".d88")) || check_file_extension(argv[arg_offset + 1], _T(".d77"))) {
		printf("the source file is d88 format image file\n");
	} else {
		DISK *disk = new DISK();
		disk->open_as_1dd = open_as_1dd;
		disk->open_as_256 = open_as_256;
		disk->open(argv[arg_offset + 1], 0);
		if(!disk->inserted) {
			printf("the source file cannot be converted\n");
		} else {
			if(argc < 3 + arg_offset) {
				char output[_MAX_PATH];
				if(disk->is_d8e()) {
					sprintf(output, "%s.d8e", argv[arg_offset + 1]);
				} else {
					sprintf(output, "%s.d88", argv[arg_offset + 1]);
				}
				disk->save_as_d88(output);
			} else {
				disk->save_as_d88(argv[arg_offset + 2]);
			}
		}
		delete disk;
	}
}
