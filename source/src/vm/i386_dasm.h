/*
	Skelton for retropc emulator

	Origin : MAME i386 core
	Author : Takeda.Toshiya
	Date   : 2020.02.02-

	[ i386 disassembler ]
*/

#ifndef _I386_DASM_H_
#define _I386_DASM_H_

#include "../common.h"

int i386_dasm(uint8_t *oprom, uint32_t eip, bool is_ia32, _TCHAR *buffer, size_t buffer_len);

#endif
