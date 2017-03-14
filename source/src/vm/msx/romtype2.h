/*
	Common Source Code Project
	MSX Series (experimental)

	Author : umaiboux
	Date   : 2016.03.xx-

	[ ROM type hash ]
*/

#ifndef _ROMTYPE2_H_

//#include <stdlib.h>
//#include <stdio.h>

static RomType hashRomType(uint8_t *rom, int size)
{
	int crc32 = get_crc32(rom, size);
	_TCHAR str_crc32[10];
	_TCHAR str_romtype[32];
	my_stprintf_s(str_crc32, 10, _T("%08X"), crc32);
	MyGetPrivateProfileString(_T("CRC32"), str_crc32, _T("NONE"), str_romtype, sizeof(str_romtype)/sizeof(_TCHAR), create_local_path(_T("romhash.ini")));
	int i;
	for(i=0;; i++) {
		if ((!_tcscmp(typestr[i].p, str_romtype)) || (typestr[i].type == ROMTYPE_NONE)) {
			return (RomType)(typestr[i].type);
		}
	}
	return (RomType)ROMTYPE_NONE;
}

#define _ROMTYPE2_H_

#endif

